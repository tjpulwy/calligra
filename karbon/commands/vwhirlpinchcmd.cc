/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klocale.h>

#include "vselection.h"
#include "vwhirlpinch.h"
#include "vwhirlpinchcmd.h"


VWhirlPinchCmd::VWhirlPinchCmd( VDocument* doc,
	double angle, double pinch, double radius )
		: VCommand( doc, i18n( "Whirl Pinch" ) )
{
	m_selection = m_doc->selection()->clone();

	m_angle = angle;
	m_pinch = pinch;
	m_radius = radius;
	m_center = m_selection->boundingBox().center();
}

VWhirlPinchCmd::~VWhirlPinchCmd()
{
	delete( m_selection );
}

void
VWhirlPinchCmd::execute()
{
	VWhirlPinch op( m_center, m_angle, m_pinch, m_radius );

	VObjectListIterator itr( m_selection->objects() );
	for ( ; itr.current() ; ++itr )
		op.visit( *itr.current() );
}

void
VWhirlPinchCmd::unexecute()
{
}

