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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qwidget.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <kcolorbtn.h>
#include <qlayout.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "wdgselectionoptions.h"
#include "kis_selection_options.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_device.h"

KisSelectionOptions::KisSelectionOptions(QWidget *parent, KisCanvasSubject * subject)
	: super(parent),
	  m_subject(subject)
{
	m_page = new WdgSelectionOptions(this);
	QVBoxLayout * l = new QVBoxLayout(this);
	l -> addWidget(m_page);

	connect(m_page -> radioInvert, SIGNAL(toggled(bool)), this, SLOT(slotSetMaskInversion(bool)));
	connect(m_page -> bnMaskColor, SIGNAL(changed(const QColor &)), this, SLOT(slotSetMaskColor(const QColor &)));
	connect(m_page -> cmbAction, SIGNAL(activated(int)), this, SIGNAL(actionChanged(int)));
}

KisSelectionOptions::~KisSelectionOptions()
{
}

int KisSelectionOptions::action()
{
	return m_page -> cmbAction -> currentItem();
}

void KisSelectionOptions::slotSetMaskColor(const QColor & c)
{
	if (!m_subject) return;
	KisImageSP img = m_subject -> currentImg();
	if (!img) return;
	KisLayerSP l = img -> activeLayer();
	if (!l) return;

	if (l -> hasSelection()) {
		l -> selection() -> setMaskColor(c);
		m_subject -> canvasController() -> updateCanvas();
	}
}



void KisSelectionOptions::slotSetMaskInversion(bool i)
{
	if (!m_subject) return;
	KisImageSP img = m_subject -> currentImg();
	if (!img) return;
	KisLayerSP l = img -> activeLayer();
	if (!l) return;
	if (l -> hasSelection()) {
		l -> selection() -> setInverted(i);
		m_subject -> canvasController() -> updateCanvas();
	}	
}


#include "kis_selection_options.moc"
