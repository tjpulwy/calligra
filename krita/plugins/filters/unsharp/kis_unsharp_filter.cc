/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_unsharp_filter.h"
#include <QBitArray>


#include <kcombobox.h>
#include <knuminput.h>

#include <kis_mask_generator.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_iterators_pixel.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>
#include <kis_paint_device.h>

#include "kis_wdg_unsharp.h"
#include "ui_wdgunsharp.h"

KisUnsharpFilter::KisUnsharpFilter() : KisFilter(id(), CategoryEnhance, i18n("&Unsharp Mask..."))
{
        setSupportsPainting(true);
        setSupportsPreview(true);
        setSupportsIncrementalPainting(false);
        setSupportsAdjustmentLayers(false);
        setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisFilterConfigWidget * KisUnsharpFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageSP ) const
{
    return new KisWdgUnsharp(parent);
}

KisFilterConfiguration* KisUnsharpFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("halfSize", 5 );
    config->setProperty("amount", 0.5 );
    config->setProperty("threshold", 10 );
    return config;
}

void KisUnsharpFilter::process(KisConstProcessingInformation src,
                 KisProcessingInformation dst,
                 const QSize& areaSize,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
        ) const
{
    
    KoUpdater* filterUpdater = 0;
    KoUpdater* convolutionUpdater = 0;
    if (progressUpdater){
    	KoProgressUpdater updater(progressUpdater);
    	updater.start();
    	// Two sub-sub tasks that each go from 0 to 100.
    	convolutionUpdater = new KoUpdater( updater.startSubtask() );
    	filterUpdater = new KoUpdater(updater.startSubtask() );
    }

    if(!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    uint halfSize = (config->getProperty("halfSize", value)) ? value.toUInt() : 5;
    uint brushsize = 2 * halfSize + 1;
    double amount = (config->getProperty("amount", value)) ? value.toDouble() : 0.5;
    uint threshold = (config->getProperty("threshold", value)) ? value.toUInt() : 10;

//     dbgKrita <<" brush size =" << size <<"" << halfSize;
    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(brushsize, brushsize, halfSize, halfSize);


    KisConvolutionKernelSP kernel = KisConvolutionKernel::kernelFromMaskGenerator(kas);

    KisPaintDeviceSP interm = KisPaintDeviceSP(new KisPaintDevice(*src.paintDevice()));
    KoColorSpace * cs = interm->colorSpace();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();
    QBitArray channelFlags = cs->channelFlags(); // Only convolve color channels

    KisConvolutionPainter painter( interm );
    if (progressUpdater){
    	painter.setProgress( convolutionUpdater );
    }

    painter.setChannelFlags( channelFlags );
    painter.beginTransaction("convolution step");
    painter.applyMatrix(kernel, src.topLeft().x(), src.topLeft().y(), areaSize.width(), areaSize.height(), BORDER_REPEAT);

    if (progressUpdater && progressUpdater->interrupted()) {
        return;
    }

    KisHLineIteratorPixel dstIt = dst.paintDevice()->createHLineIterator(dst.topLeft().x(), dst.topLeft().y(), areaSize.width());
    KisHLineConstIteratorPixel srcIt = src.paintDevice()->createHLineConstIterator(src.topLeft().x(), src.topLeft().y(), areaSize.width());
    KisHLineConstIteratorPixel intermIt = interm->createHLineConstIterator(src.topLeft().x(), src.topLeft().y(), areaSize.width());

    int cdepth = cs -> pixelSize();
    quint8 *colors[2];
    colors[0] = new quint8[cdepth];
    colors[1] = new quint8[cdepth];

    int pixelsProcessed = 0;
    qint32 weights[2];
    qint32 factor = 128;

    // XXX: Added static cast to avoid warning
    weights[0] = static_cast<qint32>(factor * ( 1. + amount));
    weights[1] = static_cast<qint32>(-factor * amount);

    int steps = 100 / areaSize.width() * areaSize.height();

    for( int j = 0; j < areaSize.height(); j++)
    {
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                quint8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if( diff > threshold)
                {
                    memcpy(colors[0],srcIt.rawData(), cdepth);
                    memcpy(colors[1],intermIt.rawData(), cdepth);
                    convolutionOp->convolveColors(colors, weights, dstIt.rawData(),  factor, 0, 2, channelFlags );
                }
            }
            ++pixelsProcessed;
            if (progressUpdater) filterUpdater->setProgress(steps * pixelsProcessed);
            ++srcIt;
            ++dstIt;
            ++intermIt;
        }
        
        if (progressUpdater && progressUpdater->interrupted()) {
            return;
        }
        srcIt.nextRow();
        dstIt.nextRow();
        intermIt.nextRow();
    }
    delete colors[0];
    delete colors[1];
    delete filterUpdater;
    delete convolutionUpdater;


    if (progressUpdater) progressUpdater->setProgress(100);
}
