/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef IDENTIFIERELEMENT_H
#define IDENTIFIERELEMENT_H

#include "tokenelement.h"

KFORMULA_NAMESPACE_BEGIN

class IdentifierElement : public TokenElement {
    typedef TokenElement inherited;
public:
    IdentifierElement( BasicElement* parent = 0 );

    virtual void calcSizes( const ContextStyle& style,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );
};

KFORMULA_NAMESPACE_END

#endif // IDENTIFIERELEMENT_H
