/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_slider_dock.h"

//#include <QWidget>
#include <QVBoxLayout>
#include <QBitArray>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include "kis_config_notifier.h"
#include <kconfiggroup.h>

#include <kis_layer.h>
#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_image.h>
#include <kis_display_color_converter.h>

#include "kis_color_slider_widget.h"

ColorSliderDock::ColorSliderDock()
    : QDockWidget(i18n("Color Sliders"))
    , m_canvas(0)
    , m_view(0)
    , m_colorSliders(0)
{
}

void ColorSliderDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }
    if (m_view) {
        m_view->resourceProvider()->disconnect(m_colorSliders);
        m_view->resourceProvider()->disconnect(this);
        m_view->image()->disconnect(m_colorSliders);
    }

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    KisViewManager* view = kisCanvas->viewManager();

    if (m_colorSliders) {
        m_colorSliders->disconnect(); // explicit disconnect in case Qt gets confused.
        delete m_colorSliders;
    }
    QWidget *m_sliderdockerwidget = new QWidget;
    QVBoxLayout *m_layout = new QVBoxLayout(m_sliderdockerwidget);
    setWidget(m_sliderdockerwidget);
    //m_updateAllowed = true;
    
    //settings//
    QBitArray m_SlidersConfigArray(12);
    
    KConfigGroup cfg = KGlobal::config()->group("hsxColorSlider");
	
    m_SlidersConfigArray[0] =cfg.readEntry("hsvH", false);
    m_SlidersConfigArray[1] =cfg.readEntry("hsvS", false);
    m_SlidersConfigArray[2] =cfg.readEntry("hsvV", false);
    
    m_SlidersConfigArray[3] =cfg.readEntry("hslH", true);
    m_SlidersConfigArray[4] =cfg.readEntry("hslS", true);
    m_SlidersConfigArray[5] =cfg.readEntry("hslL", true);
    
    m_SlidersConfigArray[6] =cfg.readEntry("hsiH", false);
    m_SlidersConfigArray[7] =cfg.readEntry("hsiS", false);
    m_SlidersConfigArray[8] =cfg.readEntry("hsiI", false);
    
    m_SlidersConfigArray[9] =cfg.readEntry("hsyH", false);
    m_SlidersConfigArray[10]=cfg.readEntry("hsyS", false);
    m_SlidersConfigArray[11]=cfg.readEntry("hsyY", false);
    
    
    m_colorSliders = new KisColorSliderWidget(kisCanvas->displayColorConverter()->displayRendererInterface(), this, kisCanvas, m_SlidersConfigArray);
    m_layout->addWidget(m_colorSliders);
    connect(m_colorSliders, SIGNAL(colorChanged(const KoColor&)), view->resourceProvider(), SLOT(slotSetFGColor(const KoColor&)));
    connect(view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), m_colorSliders, SLOT(setColor(const KoColor&)));

    connect(view->resourceProvider(), SIGNAL(sigNodeChanged(const KisNodeSP)), this, SLOT(layerChanged(const KisNodeSP)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), m_colorSliders, SLOT(slotConfigChanged()));
    
    
    //connect(view->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_colorSliders, SLOT(setColorSpace(const KoColorSpace*)));

    m_canvas = kisCanvas;
    m_view = view;
    //b_canvas = canvas;
}


void ColorSliderDock::unsetCanvas()
{
    m_canvas = 0;
    m_view = 0;

    delete m_colorSliders;
    m_colorSliders = 0;
}

void ColorSliderDock::layerChanged(const KisNodeSP node)
{
    if (!node) return;
    if (!node->paintDevice()) return;
    if (!m_colorSliders) return;
    //if (!m_colorSliders->customColorSpaceUsed()) {
    //    const KoColorSpace *cs = node->paintDevice() ?
    //        node->paintDevice()->compositionSourceColorSpace() :
    //        node->colorSpace();
    //
    //    m_colorSliders->setColorSpace(cs);
    //}
    m_colorSliders->setColor(m_view->resourceProvider()->fgColor());
}





#include "kis_color_slider_dock.moc"
