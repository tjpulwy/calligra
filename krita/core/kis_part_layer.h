/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_PART_LAYER_
#define _KIS_PART_LAYER_

#include <qrect.h>

#include <koDocument.h>
#include <koDocumentChild.h>

#include "kis_layer.h"
#include "kis_types.h"
#include "kis_doc.h"

class KoFrame;
class KoDocument;


/**
 * The child document is responsible for saving and loading the embedded layers.
 */
class KisChildDoc : public KoDocumentChild
{

public:
    KisChildDoc ( KisDoc * kisDoc, const QRect& rect, KoDocument * childDoc );
    KisChildDoc ( KisDoc * kisDdoc );

    virtual ~KisChildDoc();
    
    KisDoc * parent() const { return m_doc; }
    
    void setPartLayer (KisPartLayerSP layer) { m_partLayer = layer; }
    
    KisPartLayerSP partLayer() const { return m_partLayer; }
    
    
protected:

    KisDoc * m_doc;
    KisPartLayerSP m_partLayer;

};
 


/**
 * A PartLayer is a layer that contains a KOffice Part like a KWord document
 * or a KSpread spreadsheet. Or whatever. A Karbon drawing.
 *
 * The part is rendered into an RBGA8 paint device so we can composite it with
 * the other layers.
 */
class KisPartLayer : public KisLayer {


    Q_OBJECT;

public:

    KisPartLayer(KisImageSP img, KisChildDoc * doc);
    virtual ~KisPartLayer();

    // Called when the layer is made active
    virtual void activate();

    // Called when another layer is made active
    virtual void deactivate();

private:
    KoFrame * m_frame; // The widget that holds the editable view of the embedded part
    KisChildDoc * m_doc; // The sub-document

};

#endif // _KIS_PART_LAYER_
