/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <klocale.h>

#include "vmcmd_stroke.h"
#include "vstroke.h"

VMCmdStroke::VMCmdStroke( KarbonPart* part, const VColor &color, float opacity )
	: VCommand( part, i18n( "Stroke Objects" ) ), m_color( color ), m_opacity( opacity )
{
	m_objects = m_part->selection();
	//m_part->deselectAllObjects();

	if( m_objects.count() == 1 )
		setName( i18n( "Stroke Object" ) );
}

void
VMCmdStroke::execute()
{
	VObjectListIterator itr( m_objects );
	for ( ; itr.current() ; ++itr )
	{
		if( m_opacity == -1 )
			m_color.setOpacity( itr.current()->stroke().color().opacity() );

		m_oldcolors.push_back( itr.current()->stroke() );
		VStroke stroke = itr.current()->stroke();
		stroke.setColor( m_color );
		itr.current()->setStroke( stroke );
	}
}

void
VMCmdStroke::unexecute()
{
	VObjectListIterator itr( m_objects );
	int i = 0;
	for ( ; itr.current() ; ++itr )
	{
		itr.current()->setStroke( m_oldcolors[ i++ ] );
	}
}

