/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org
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
#ifndef KIS_CHANGE_PROFILE_VISITOR_H_
#define KIS_CHANGE_PROFILE_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"
#include "kis_external_layer_iface.h"

/**
 * The Change Profile visitor walks over all layers and if the current
 * layer has the specified colorspace AND the specified old profile, sets
 * the colorspace to the same colorspace with the NEW profile, without doing
 * conversions. This is essential if you have loaded an image that didn't
 * have an embedded profile to which you want to attach the right profile.
 */
class KisChangeProfileVisitor :public KisNodeVisitor {
public:
    KisChangeProfileVisitor(const KoColorSpace *oldColorSpace, const KoColorSpace *dstColorSpace);
    virtual ~KisChangeProfileVisitor();

public:
    bool visit( KisExternalLayer * )
        {
            return true;
        }

    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);

private:
    const KoColorSpace *m_oldColorSpace;
    const KoColorSpace *m_dstColorSpace;
};

KisChangeProfileVisitor::KisChangeProfileVisitor(const KoColorSpace * oldColorSpace,
                                                 const KoColorSpace *dstColorSpace) :
    KisNodeVisitor(),
    m_oldColorSpace(oldColorSpace),
    m_dstColorSpace(dstColorSpace)
{
}

KisChangeProfileVisitor::~KisChangeProfileVisitor()
{
}

bool KisChangeProfileVisitor::visit(KisGroupLayer * layer)
{
    // Clear the projection, we will have to re-render everything.
    layer->resetProjection();

    KisLayerSP child = dynamic_cast<KisLayer*>( layer->firstChild().data() );
    while (child) {
        child->accept(*this);
        child = dynamic_cast<KisLayer*>( child->nextSibling().data() );
    }
    layer->setDirty();
    return true;
}

bool KisChangeProfileVisitor::visit(KisPaintLayer *layer)
{
    if (!layer) return false;
    if (!layer->paintDevice()) return false;
    if (!layer->paintDevice()->colorSpace()) return false;

    const KoColorSpace * cs = layer->paintDevice()->colorSpace();

    if (*cs == *m_oldColorSpace) {

        layer->paintDevice()->setProfile(m_dstColorSpace->profile());

        layer->setDirty();
    }
    return true;
}


bool KisChangeProfileVisitor::visit(KisAdjustmentLayer * layer)
{
    layer->resetCache();
    layer->setDirty();
    return true;
}

#endif // KIS_CHANGE_PROFILE_VISITOR_H_

