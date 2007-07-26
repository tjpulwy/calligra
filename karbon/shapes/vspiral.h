/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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

#ifndef __VSPIRAL_H__
#define __VSPIRAL_H__

#include "vcomposite.h"
#include <karbon_export.h>

#include <QPointF>

class KARBONBASE_EXPORT VSpiral : public VPath
{
public:
	enum VSpiralType
	{
		round,
		rectangular
	};
	VSpiral( VObject* parent, VState state = edit );
	VSpiral( VObject* parent,
		const QPointF& center, double radius, uint segments,
		double fade, bool clockwise, double angle = 0.0, VSpiralType type = round );

	virtual QString name() const;

	virtual void save( QDomElement& element ) const;
	virtual void load( const KoXmlElement& element );

	virtual VPath* clone() const;

protected:
	void init();

private:
	QPointF m_center;
	double m_radius;
	double m_fade;
	uint m_segments;
	bool m_clockwise;
	double m_angle;
	VSpiralType m_type;
};

#endif

