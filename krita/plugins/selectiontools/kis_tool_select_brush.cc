/*
 *  kis_tool_select_brush.cc - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qrect.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_cmb_composite.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_move_event.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_tool_select_brush.h"
#include "kis_types.h"
#include "kis_view.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"

KisToolSelectBrush::KisToolSelectBrush()
        : super(i18n("SelectBrush"))
{
    setName("tool_select_brush");
    m_optWidget = 0;
    setCursor(KisCursor::brushCursor());
}

KisToolSelectBrush::~KisToolSelectBrush()
{
}

void KisToolSelectBrush::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget -> slotActivated();
}

void KisToolSelectBrush::initPaint(KisEvent* /*e*/) 
{
    if (!m_currentImage || !m_currentImage -> activeDevice()) return;

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisLayerSP layer;
    if (m_currentImage && (layer = m_currentImage -> activeLayer())) {
        if (m_painter)
            delete m_painter;
        bool hasSelection = layer->hasSelection();
        m_transaction = new KisSelectedTransaction(i18n("Selection Brush"),layer.data());
        if(! hasSelection)
        {
            layer -> selection() -> clear();
            layer -> emitSelectionChanged();
        }
        KisSelectionSP selection = layer -> selection();
        
        m_painter = new KisPainter(selection.data());
        Q_CHECK_PTR(m_painter);
        m_painter -> setPaintColor(Qt::black);
        m_painter -> setBrush(m_subject -> currentBrush());
        m_painter -> setOpacity(MAX_SELECTED);
        m_painter -> setCompositeOp(COMPOSITE_OVER);
        KisPaintOp * op = KisPaintOpRegistry::instance() -> paintOp("paintbrush", painter());
        painter() -> setPaintOp(op); // And now the painter owns the op and will destroy it.
    }
    // Set the cursor -- ideally. this should be a mask created from the brush,
    // now that X11 can handle colored cursors.
#if 0
    // Setting cursors has no effect until the tool is selected again; this
    // should be fixed.
    setCursor(KisCursor::brushCursor());
#endif
}

void KisToolSelectBrush::endPaint() 
{
    m_mode = HOVER;
    if (m_currentImage && m_currentImage -> activeLayer()) {
        KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
        if (adapter && m_painter) {
            // If painting in mouse release, make sure painter
            // is destructed or end()ed
            adapter -> addCommand(m_transaction);
        }
        delete m_painter;
        m_painter = 0;
        notifyModified();
    }
}


void KisToolSelectBrush::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Selectionbrush"),
                        "tool_brush_selection", Qt::Key_B, this,
                        SLOT(activate()), collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action -> setToolTip(i18n("Paint a selection"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

QWidget* KisToolSelectBrush::createOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget -> setCaption(i18n("Selection Brush"));

    return m_optWidget;
}

QWidget* KisToolSelectBrush::optionWidget()
{
    return m_optWidget;
}

#include "kis_tool_select_brush.moc"
