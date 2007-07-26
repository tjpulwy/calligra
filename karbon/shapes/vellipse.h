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

#ifndef __VELLIPSE_H__
#define __VELLIPSE_H__

#include "vcomposite.h"
#include <karbon_export.h>

#include <QPointF>

class KARBONBASE_EXPORT VEllipse : public VPath
{
public:
	enum VEllipseType
	{
		full,
		section,
		cut,
		arc
	};
	VEllipse( VObject* parent, VState state = edit );
	VEllipse( VObject* parent,
		const QPointF& topLeft, double width, double height,
		VEllipseType type = full, double startAngle = 0, double endAngle = 0 );

	virtual QString name() const;

	virtual void save( QDomElement& element ) const;
	virtual void saveOasis( KoStore *store, KoXmlWriter *docWriter, KoGenStyles &mainStyles, int &index ) const;
	virtual void load( const KoXmlElement& element );
	virtual bool loadOasis( const KoXmlElement &element, KoOasisLoadingContext &context );

	virtual VPath* clone() const;

protected:
	void init();

private:
	VEllipseType m_type;
	QPointF m_center;
	double m_rx;
	double m_ry;
	double m_startAngle;
	double m_endAngle;
};

#endif

