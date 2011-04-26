/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SimpleRootAreaProvider.h"

#include "TextShape.h"

#include <KoTextLayoutRootArea.h>
#include <KoTextLayoutObstruction.h>

SimpleRootAreaProvider::SimpleRootAreaProvider(KoTextShapeData *data, TextShape *textshape)
    : m_textShape(textshape)
    , m_area(0)
    , m_textShapeData(data)

{
}

KoTextLayoutRootArea *SimpleRootAreaProvider::provide(KoTextDocumentLayout *documentLayout, QString mastePageName)
{
    Q_UNUSED(mastePageName);

    if(m_area == 0) {
        m_area = new KoTextLayoutRootArea(documentLayout);
        m_area->setAssociatedShape(m_textShape);
        m_textShapeData->setRootArea(m_area);

        return m_area;
    }
    return 0;
}

void SimpleRootAreaProvider::releaseAllAfter(KoTextLayoutRootArea *afterThis)
{
}

void SimpleRootAreaProvider::doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea)
{
    Q_UNUSED(isNewRootArea);

    QRectF updateRect = rootArea->associatedShape()->outlineRect();
    rootArea->associatedShape()->update(updateRect);

    QSizeF newSize = rootArea->associatedShape()->size();
    if (m_textShapeData->verticalAlignment() & Qt::AlignBottom) {
    }
    if (m_textShapeData->verticalAlignment() & Qt::AlignVCenter) {
    }
    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
        ||m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowHeight) {
        newSize.setHeight(rootArea->bottom() - rootArea->top());
    }
    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
        ||m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidth) {
        newSize.setWidth(rootArea->right() - rootArea->left());
    }

    qreal newBottom = rootArea->top() + newSize.height();
    KoFlake::Position sizeAnchor= KoFlake::TopLeftCorner;

    if (m_textShapeData->verticalAlignment() & Qt::AlignBottom) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset(newBottom - rootArea->bottom());
            sizeAnchor= KoFlake::BottomLeftCorner;
        }
    }
    if (m_textShapeData->verticalAlignment() & Qt::AlignVCenter) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset((newBottom - rootArea->bottom()) / 2);
            sizeAnchor = KoFlake::CenteredPosition;
        }
    }

    if (newSize != rootArea->associatedShape()->size()) {
        QSizeF tmpSize = rootArea->associatedShape()->size();
        tmpSize.setWidth(newSize.width());
        QPointF centerpos = rootArea->associatedShape()->absolutePosition(KoFlake::CenteredPosition);
        rootArea->associatedShape()->setSize(tmpSize);
        rootArea->associatedShape()->setAbsolutePosition(centerpos, KoFlake::CenteredPosition);
        centerpos = rootArea->associatedShape()->absolutePosition(sizeAnchor);
        rootArea->associatedShape()->setSize(newSize);
        rootArea->associatedShape()->setAbsolutePosition(centerpos, sizeAnchor);
    }


    updateRect |= rootArea->associatedShape()->outlineRect();
    rootArea->associatedShape()->update(rootArea->associatedShape()->outlineRect());
}

QSizeF SimpleRootAreaProvider::suggestSize(KoTextLayoutRootArea *rootArea)
{
    QSizeF size = m_textShape->size();
    // In simple cases we always set height way too high so that we have no breaking
    // If the shape grows afterwards or not is handled in doPostLayout()
    size.setHeight(1E6);

    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
        ||m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidth) {
        rootArea->setNoWrap(1E6);
    }
    return size;
}

QList<KoTextLayoutObstruction *> SimpleRootAreaProvider::relevantObstructions(const QRectF &rect, const QList<KoTextLayoutObstruction *> &excludingThese)
{
    QList<KoTextLayoutObstruction*> obstructions;
    QRectF canvasRect = rect;
    canvasRect.moveTop(rect.top() - m_area->top());
    QTransform transform = m_textShape->absoluteTransformation(0);
    canvasRect = transform.mapRect(canvasRect);

    QList<KoShape *> shapes;
    //shapes = manager->shapesAt(canvasRect):
    return obstructions;
}
