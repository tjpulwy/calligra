/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kwanchor.h"
#include "kwtextdocument.h"
#include "kwtextframeset.h"

KWAnchor::KWAnchor( KWTextDocument *textdoc )
    : QTextCustomItem( textdoc )
{
}

QPoint KWAnchor::origin()
{
    // KWTextFrameSet * fs = static_cast<KWTextDocument *>(parent)->textFrameSet();
    // internalToContents ... argl how do I find my coordinates ?
    return QPoint(0,0); // #########
}

void KWAnchor::draw( QPainter*, int, int, int, int, int, int, const QColorGroup& )
{

}
