/* This file is part of the KDE project
   Copyright (C) 2011 Benjamin Port <port.benjamin@gmail.com>
   Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KPrViewModeOutline.h"

#include "KoPAView.h"
#include "KoPACanvas.h"
#include "KoPAPageBase.h"
#include "KoPADocument.h"
#include "KPrPage.h"
#include "KoTextShapeData.h"

#include "KPrOutlineEditor.h"

#include <KDebug>
#include <QtGui/qtextformat.h>

KPrViewModeOutline::KPrViewModeOutline(KoPAView *view, KoPACanvas *canvas)
    : KoPAViewMode( view, canvas )
    , m_outlineEditor(new KPrOutlineEditor(this, view->parentWidget()))
{
    m_outlineEditor->hide();
}

void KPrViewModeOutline::paint(KoPACanvasBase* canvas, QPainter& painter, const QRectF &paintRect)
{
    Q_UNUSED(canvas);
    Q_UNUSED(painter);
    Q_UNUSED(paintRect);
}

void KPrViewModeOutline::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void KPrViewModeOutline::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void KPrViewModeOutline::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KPrViewModeOutline::activate(KoPAViewMode *previousViewMode)
{
    Q_UNUSED(previousViewMode);
    
    populate();
    
    KoPAView *view = dynamic_cast<KoPAView *>(m_view);
    if (view) {
        view->hide();
    }
    m_outlineEditor->show();
    m_outlineEditor->setFocus(Qt::ActiveWindowFocusReason);
}


void KPrViewModeOutline::deactivate()
{
    m_outlineEditor->hide();
     // Active the view as a basic but active one
    m_view->setActionEnabled(KoPAView::AllActions, true);
    m_view->doUpdateActivePage(m_view->activePage());
    KoPAView *view = dynamic_cast<KoPAView *>(m_view);
    if (view) {
        view->show();
    }
}

void KPrViewModeOutline::populate()
{
    m_outlineEditor->clear();
    QTextCursor currentCursor = m_outlineEditor->textCursor();
    
    // For each slides
    foreach (KoPAPageBase * pageBase, m_view->kopaDocument()->pages()) {
        if (KPrPage * page = dynamic_cast<KPrPage *>(pageBase)) {
            int pageNumber = m_view->kopaDocument()->pages().indexOf(pageBase);
            
            // Copy relevant content of the title of the page in the frame
            foreach (OutlinePair pair, page->placeholders().outlineData()) {
                QTextBlockFormat blockFormat;
                blockFormat.setBackground((pageNumber%2)?QBrush(Qt::gray):QBrush(Qt::white));
                // set the page property
                blockFormat.setProperty(0, pageNumber);
                // set the type property
                blockFormat.setProperty(1, pair.first);
                    
                currentCursor.insertBlock(blockFormat);
                currentCursor.insertText(pair.second->document()->toPlainText());
            }
        }
    }
    currentCursor.setPosition(0);
    m_outlineEditor->setTextCursor(currentCursor);
}
