/* This file is part of the KDE project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>

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

#include <qapplication.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qstyle.h>

#include <kotoolbutton.h>
#include <kcolordrag.h>
#include <klocale.h>
#include <kdebug.h>

namespace {
    const int COLS = 15;
    const int TILESIZE = 16;
}

KoColorPanel::KoColorPanel( QWidget* parent, const char* name ) :
    QWidget( parent, name, WStaticContents | WRepaintNoErase | WResizeNoErase )
{
    setMouseTracking( true );
    setAcceptDrops( true );
    init();
}

KoColorPanel::~KoColorPanel()
{
}

QSize KoColorPanel::sizeHint() const
{
    return minimumSizeHint();
}

QSize KoColorPanel::minimumSizeHint() const
{
    return QSize( COLS << 4, lines() << 4 );
}

void KoColorPanel::clear()
{
    if ( m_colorMap.isEmpty() )
        return;

    QSize area( minimumSizeHint() );
    m_colorMap.clear();
    init();
    updateGeometry();
    erase( 0, 0, area.width(), area.height() );
}

void KoColorPanel::insertColor( const QColor& color )
{
    Position pos = m_nextPosition;
    if ( insertColor( color, true ) ) // we want checking for external users
        finalizeInsertion( pos );
}

void KoColorPanel::insertColor( const QColor& color, const QString& toolTip )
{
    Position pos = m_nextPosition;
    if ( insertColor( color, toolTip, true ) ) // we want checking for external users
        finalizeInsertion( pos );
}

void KoColorPanel::insertDefaultColors()
{
    if ( m_defaultsAdded )
        return;
    m_defaultsAdded = true;

    int currentRow = m_nextPosition.y; // we have to repaint this row below

    // Note: No checking for duplicates, so take care when you modify that list
    insertColor(qRgb( 255 ,     0 ,     0 ), i18n( "Color", "Red" ), false);
    insertColor(qRgb( 255 ,   165 ,     0 ), i18n( "Color", "Orange" ), false);
    insertColor(qRgb( 255 ,     0 ,   255 ), i18n( "Color", "Magenta" ), false);
    insertColor(qRgb(   0 ,     0 ,   255 ), i18n( "Color", "Blue" ), false);
    insertColor(qRgb(   0 ,   255 ,   255 ), i18n( "Color", "Cyan" ), false);
    insertColor(qRgb(   0 ,   255 ,     0 ), i18n( "Color", "Green" ), false);
    insertColor(qRgb( 255 ,   255 ,     0 ), i18n( "Color", "Yellow" ), false);
    insertColor(qRgb( 165 ,    42 ,    42 ), i18n( "Color", "Brown" ), false);
    insertColor(qRgb( 139 ,     0 ,     0 ), i18n( "Color", "Darkred" ), false);
    insertColor(qRgb( 255 ,   140 ,     0 ), i18n( "Color", "Dark Orange" ), false);
    insertColor(qRgb( 139 ,     0 ,   139 ), i18n( "Color", "Dark Magenta" ), false);
    insertColor(qRgb(   0 ,     0 ,   139 ), i18n( "Color", "Dark Blue" ), false);
    insertColor(qRgb(   0 ,   139 ,   139 ), i18n( "Color", "Dark Cyan" ), false);
    insertColor(qRgb(   0 ,   100 ,     0 ), i18n( "Color", "Dark Green" ), false);
    insertColor(qRgb( 130 ,   127 ,     0 ), i18n( "Color", "Dark Yellow" ), false);
    insertColor(qRgb( 255 ,   255 ,   255 ), i18n( "Color", "White" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 229 ,   229 ,   229 ), i18n( "Color", "Grey 90%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 204 ,   204 ,   204 ), i18n( "Color", "Grey 80%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 178 ,   178 ,   178 ), i18n( "Color", "Grey 70%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 153 ,   153 ,   153 ), i18n( "Color", "Grey 60%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 127 ,   127 ,   127 ), i18n( "Color", "Grey 50%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb( 102 ,   102 ,   102 ), i18n( "Color", "Grey 40%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb(  76 ,    76 ,    76 ), i18n( "Color", "Grey 30%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb(  51 ,    51 ,    51 ), i18n( "Color", "Grey 20%" ), false);
    // xgettext:no-c-format
    insertColor(qRgb(  25 ,    25 ,    25 ), i18n( "Color", "Grey 10%" ), false);
    insertColor(qRgb(   0 ,     0 ,     0 ), i18n( "Color", "Black" ), false);
    insertColor(qRgb( 255 ,   255 ,   240 ), i18n( "Color", "Ivory" ), false);
    insertColor(qRgb( 255 ,   250 ,   250 ), i18n( "Color", "Snow" ), false);
    insertColor(qRgb( 245 ,   255 ,   250 ), i18n( "Color", "Mint Cream" ), false);
    insertColor(qRgb( 255 ,   250 ,   240 ), i18n( "Color", "Floral White" ), false);
    insertColor(qRgb( 255 ,   255 ,   224 ), i18n( "Color", "Light Yellow" ), false);
    insertColor(qRgb( 240 ,   255 ,   255 ), i18n( "Color", "Azure" ), false);
    insertColor(qRgb( 248 ,   248 ,   255 ), i18n( "Color", "Ghost White" ), false);
    insertColor(qRgb( 240 ,   255 ,   240 ), i18n( "Color", "Honeydew" ), false);
    insertColor(qRgb( 255 ,   245 ,   238 ), i18n( "Color", "Seashell" ), false);
    insertColor(qRgb( 240 ,   248 ,   255 ), i18n( "Color", "Alice Blue" ), false);
    insertColor(qRgb( 255 ,   248 ,   220 ), i18n( "Color", "Cornsilk" ), false);
    insertColor(qRgb( 255 ,   240 ,   245 ), i18n( "Color", "Lavender Blush" ), false);
    insertColor(qRgb( 253 ,   245 ,   230 ), i18n( "Color", "Old Lace" ), false);
    insertColor(qRgb( 245 ,   245 ,   245 ), i18n( "Color", "White Smoke" ), false);
    insertColor(qRgb( 255 ,   250 ,   205 ), i18n( "Color", "Lemon Chiffon" ), false);
    insertColor(qRgb( 224 ,   255 ,   255 ), i18n( "Color", "Light Cyan" ), false);
    insertColor(qRgb( 250 ,   250 ,   210 ), i18n( "Color", "Light Goldenrod Yellow" ), false);
    insertColor(qRgb( 250 ,   240 ,   230 ), i18n( "Color", "Linen" ), false);
    insertColor(qRgb( 245 ,   245 ,   220 ), i18n( "Color", "Beige" ), false);
    insertColor(qRgb( 255 ,   239 ,   213 ), i18n( "Color", "Papaya Whip" ), false);
    insertColor(qRgb( 255 ,   235 ,   205 ), i18n( "Color", "Blanched Almond" ), false);
    insertColor(qRgb( 250 ,   235 ,   215 ), i18n( "Color", "Antique White" ), false);
    insertColor(qRgb( 255 ,   228 ,   225 ), i18n( "Color", "Misty Rose" ), false);
    insertColor(qRgb( 230 ,   230 ,   250 ), i18n( "Color", "Lavender" ), false);
    insertColor(qRgb( 255 ,   228 ,   196 ), i18n( "Color", "Bisque" ), false);
    insertColor(qRgb( 255 ,   228 ,   181 ), i18n( "Color", "Moccasin" ), false);
    insertColor(qRgb( 255 ,   222 ,   173 ), i18n( "Color", "Navajo White" ), false);
    insertColor(qRgb( 255 ,   218 ,   185 ), i18n( "Color", "Peach Puff" ), false);
    insertColor(qRgb( 238 ,   232 ,   170 ), i18n( "Color", "Pale Goldenrod" ), false);
    insertColor(qRgb( 245 ,   222 ,   179 ), i18n( "Color", "Wheat" ), false);
    insertColor(qRgb( 220 ,   220 ,   220 ), i18n( "Color", "Gainsboro" ), false);
    insertColor(qRgb( 240 ,   230 ,   140 ), i18n( "Color", "Khaki" ), false);
    insertColor(qRgb( 175 ,   238 ,   238 ), i18n( "Color", "Pale Turquoise" ), false);
    insertColor(qRgb( 255 ,   192 ,   203 ), i18n( "Color", "Pink" ), false);
    insertColor(qRgb( 238 ,   221 ,   130 ), i18n( "Color", "Light Goldenrod" ), false);
    insertColor(qRgb( 211 ,   211 ,   211 ), i18n( "Color", "Light Grey" ), false);
    insertColor(qRgb( 255 ,   182 ,   193 ), i18n( "Color", "Light Pink" ), false);
    insertColor(qRgb( 176 ,   224 ,   230 ), i18n( "Color", "Powder Blue" ), false);
    insertColor(qRgb( 127 ,   255 ,   212 ), i18n( "Color", "Aquamarine" ), false);
    insertColor(qRgb( 216 ,   191 ,   216 ), i18n( "Color", "Thistle" ), false);
    insertColor(qRgb( 173 ,   216 ,   230 ), i18n( "Color", "Light Blue" ), false);
    insertColor(qRgb( 152 ,   251 ,   152 ), i18n( "Color", "Pale Green" ), false);
    insertColor(qRgb( 255 ,   215 ,     0 ), i18n( "Color", "Gold" ), false);
    insertColor(qRgb( 173 ,   255 ,    47 ), i18n( "Color", "Green Yellow" ), false);
    insertColor(qRgb( 176 ,   196 ,   222 ), i18n( "Color", "Light Steel Blue" ), false);
    insertColor(qRgb( 144 ,   238 ,   144 ), i18n( "Color", "Light Green" ), false);
    insertColor(qRgb( 221 ,   160 ,   221 ), i18n( "Color", "Plum" ), false);
    insertColor(qRgb( 190 ,   190 ,   190 ), i18n( "Color", "Gray" ), false);
    insertColor(qRgb( 222 ,   184 ,   135 ), i18n( "Color", "Burly Wood" ), false);
    insertColor(qRgb( 135 ,   206 ,   250 ), i18n( "Color", "Light Skyblue" ), false);
    insertColor(qRgb( 255 ,   160 ,   122 ), i18n( "Color", "Light Salmon" ), false);
    insertColor(qRgb( 135 ,   206 ,   235 ), i18n( "Color", "Sky Blue" ), false);
    insertColor(qRgb( 210 ,   180 ,   140 ), i18n( "Color", "Tan" ), false);
    insertColor(qRgb( 238 ,   130 ,   238 ), i18n( "Color", "Violet" ), false);
    insertColor(qRgb( 244 ,   164 ,    96 ), i18n( "Color", "Sandy Brown" ), false);
    insertColor(qRgb( 233 ,   150 ,   122 ), i18n( "Color", "Dark Salmon" ), false);
    insertColor(qRgb( 189 ,   183 ,   107 ), i18n( "Color", "Dark khaki" ), false);
    insertColor(qRgb( 127 ,   255 ,     0 ), i18n( "Color", "Chartreuse" ), false);
    insertColor(qRgb( 169 ,   169 ,   169 ), i18n( "Color", "Dark Gray" ), false);
    insertColor(qRgb( 124 ,   252 ,     0 ), i18n( "Color", "Lawn Green" ), false);
    insertColor(qRgb( 255 ,   105 ,   180 ), i18n( "Color", "Hot Pink" ), false);
    insertColor(qRgb( 250 ,   128 ,   114 ), i18n( "Color", "Salmon" ), false);
    insertColor(qRgb( 240 ,   128 ,   128 ), i18n( "Color", "Light Coral" ), false);
    insertColor(qRgb(  64 ,   224 ,   208 ), i18n( "Color", "Turquoise" ), false);
    insertColor(qRgb( 143 ,   188 ,   143 ), i18n( "Color", "Dark Seagreen" ), false);
    insertColor(qRgb( 218 ,   112 ,   214 ), i18n( "Color", "Orchid" ), false);
    insertColor(qRgb( 102 ,   205 ,   170 ), i18n( "Color", "Medium Aquamarine" ), false);
    insertColor(qRgb( 255 ,   127 ,    80 ), i18n( "Color", "Coral" ), false);
    insertColor(qRgb( 154 ,   205 ,    50 ), i18n( "Color", "Yellow Green" ), false);
    insertColor(qRgb( 218 ,   165 ,    32 ), i18n( "Color", "Goldenrod" ), false);
    insertColor(qRgb(  72 ,   209 ,   204 ), i18n( "Color", "Medium Turquoise" ), false);
    insertColor(qRgb( 188 ,   143 ,   143 ), i18n( "Color", "Rosy Brown" ), false);
    insertColor(qRgb( 219 ,   112 ,   147 ), i18n( "Color", "Pale VioletRed" ), false);
    insertColor(qRgb(   0 ,   250 ,   154 ), i18n( "Color", "Medium Spring Green" ), false);
    insertColor(qRgb( 255 ,    99 ,    71 ), i18n( "Color", "Tomato" ), false);
    insertColor(qRgb( 0   ,   255 ,   127 ), i18n( "Color", "Spring Green" ), false);
    insertColor(qRgb( 205 ,   133 ,    63 ), i18n( "Color", "Peru" ), false);
    insertColor(qRgb( 100 ,   149 ,   237 ), i18n( "Color", "Cornflower Blue" ), false);
    insertColor(qRgb( 132 ,   112 ,   255 ), i18n( "Color", "Light Slate Blue" ), false);
    insertColor(qRgb( 147 ,   112 ,   219 ), i18n( "Color", "Medium Purple" ), false);
    insertColor(qRgb( 186 ,    85 ,   211 ), i18n( "Color", "Medium Orchid" ), false);
    insertColor(qRgb(  95 ,   158 ,   160 ), i18n( "Color", "Cadet Blue" ), false);
    insertColor(qRgb(   0 ,   206 ,   209 ), i18n( "Color", "Dark Turquoise" ), false);
    insertColor(qRgb(   0 ,   191 ,   255 ), i18n( "Color", "Deep Skyblue" ), false);
    insertColor(qRgb( 119 ,   136 ,   153 ), i18n( "Color", "Light Slate Grey" ), false);
    insertColor(qRgb( 184 ,   134 ,    11 ), i18n( "Color", "Dark Goldenrod" ), false);
    insertColor(qRgb( 123 ,   104 ,   238 ), i18n( "Color", "MediumSlate Blue" ), false);
    insertColor(qRgb( 205 ,    92 ,    92 ), i18n( "Color", "IndianRed" ), false);
    insertColor(qRgb( 210 ,   105 ,    30 ), i18n( "Color", "Chocolate" ), false);
    insertColor(qRgb(  60 ,   179 ,   113 ), i18n( "Color", "Medium Sea Green" ), false);
    insertColor(qRgb(  50 ,   205 ,    50 ), i18n( "Color", "Lime Ggreen" ), false);
    insertColor(qRgb(  32 ,   178 ,   170 ), i18n( "Color", "Light Sea Green" ), false);
    insertColor(qRgb( 112 ,   128 ,   144 ), i18n( "Color", "Slate Gray" ), false);
    insertColor(qRgb(  30 ,   144 ,   255 ), i18n( "Color", "Dodger Blue" ), false);
    insertColor(qRgb( 255 ,    69 ,     0 ), i18n( "Color", "Orange Red" ), false);
    insertColor(qRgb( 255 ,    20 ,   147 ), i18n( "Color", "Deep Pink" ), false);
    insertColor(qRgb(  70 ,   130 ,   180 ), i18n( "Color", "Steel Blue" ), false);
    insertColor(qRgb( 106 ,    90 ,   205 ), i18n( "Color", "Slate Blue" ), false);
    insertColor(qRgb( 107 ,   142 ,    35 ), i18n( "Color", "Olive Drab" ), false);
    insertColor(qRgb(  65 ,   105 ,   225 ), i18n( "Color", "Royal Blue" ), false);
    insertColor(qRgb( 208 ,    32 ,   144 ), i18n( "Color", "Violet Red" ), false);
    insertColor(qRgb( 153 ,    50 ,   204 ), i18n( "Color", "Dark Orchid" ), false);
    insertColor(qRgb( 160 ,    32 ,   240 ), i18n( "Color", "Purple" ), false);
    insertColor(qRgb( 105 ,   105 ,   105 ), i18n( "Color", "Dim Gray" ), false);
    insertColor(qRgb( 138 ,    43 ,   226 ), i18n( "Color", "Blue Violet" ), false);
    insertColor(qRgb( 160 ,    82 ,    45 ), i18n( "Color", "Sienna" ), false);
    insertColor(qRgb( 199 ,    21 ,   133 ), i18n( "Color", "Medium Violet Red" ), false);
    insertColor(qRgb( 176 ,    48 ,    96 ), i18n( "Color", "Maroon" ), false);
    insertColor(qRgb(  46 ,   139 ,    87 ), i18n( "Color", "Sea Green" ), false);
    insertColor(qRgb(  85 ,   107 ,    47 ), i18n( "Color", "Dark Olive Green" ), false);
    insertColor(qRgb(  34 ,   139 ,    34 ), i18n( "Color", "Forest Green" ), false);
    insertColor(qRgb( 139 ,    69 ,    19 ), i18n( "Color", "Saddle Brown" ), false);
    insertColor(qRgb( 148 ,     0 ,   211 ), i18n( "Color", "Darkviolet" ), false);
    insertColor(qRgb( 178 ,    34 ,    34 ), i18n( "Color", "Fire Brick" ), false);
    insertColor(qRgb(  72 ,    61 ,   139 ), i18n( "Color", "Dark Slate Blue" ), false);
    insertColor(qRgb(  47 ,    79 ,    79 ), i18n( "Color", "Dark Slate Gray" ), false);
    insertColor(qRgb(  25 ,    25 ,   112 ), i18n( "Color", "Midnight Blue" ), false);
    insertColor(qRgb(   0 ,     0 ,   205 ), i18n( "Color", "Medium Blue" ), false);
    insertColor(qRgb(   0 ,     0 ,   128 ), i18n( "Color", "Navy" ), false);

    finalizeInsertion( m_nextPosition );  // with a no-op paint() call as we repaint anyway
    updateGeometry();
    // we have to repaint the "old" current row explicitly due
    // to WStaticContents
    repaint( 0, currentRow << 4, COLS << 4, 16 );
}

void KoColorPanel::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
        m_pressedPos = e->pos();
}

void KoColorPanel::mouseMoveEvent( QMouseEvent* e )
{
    if ( e->state() & Qt::LeftButton ) {
        QPoint p = m_pressedPos - e->pos();
        if ( p.manhattanLength() > QApplication::startDragDistance() ) {
            QColor color( mapToColor( m_pressedPos ) );
            if ( color.isValid() ) {
                KColorDrag *drag = new KColorDrag( color, this, name() );
                drag->dragCopy();
            }
        }
    }
    else
        updateFocusPosition( mapToPosition( e->pos() ) );
}

void KoColorPanel::paintEvent( QPaintEvent* e )
{
    int lns = lines();
    int startRow, endRow, startCol, endCol;
    paintArea( e->rect(), startRow, endRow, startCol, endCol );

    QPainter p( this );

    // First clear all the areas we won't paint on later (only if the widget isn't erased)
    if ( !e->erased() ) {
        // vertical rects
        int tmp = TILESIZE * lns;
        if ( startCol == 0 )
            erase( 0, 0, 2, tmp );
        if ( endCol == COLS )
            erase( width() - 2, 0, 2, tmp );
        else
            erase( ( endCol << 4 ) - 2, 0, 2, tmp );
        int i = startCol == 0 ? 1 : startCol;
        for ( ; i < endCol; ++i )
            erase( ( i << 4 ) - 2, 0, 4, tmp );

        // horizontal rects
        tmp = TILESIZE * COLS;
        if ( startRow == 0 )
            erase( 0, 0, tmp, 2 );
        if ( endRow == lns )
            erase( 0, height() - 2, tmp, 2 );
        else
            erase( 0, ( endRow << 4 ) - 2, tmp, 2 );
        i = startRow == 0 ? 1 : startRow;
        for ( ; i < endRow; ++i )
            erase( 0, ( i << 4 ) - 2, tmp, 4 );
    }

    // The "active" element (if there is one)
    if ( hasFocus() && m_focusPosition.x != -1 && m_focusPosition.y != -1 &&
         mapFromPosition( m_focusPosition ).intersects( e->rect() ) )
        style().drawPrimitive( QStyle::PE_Panel, &p, QRect( m_focusPosition.x << 4, m_focusPosition.y << 4, TILESIZE, TILESIZE ),
                               colorGroup(), QStyle::Style_Sunken | QStyle::Style_Enabled );

    --lns;  // Attention: We just avoid some lns - 1 statements

    // ...all color tiles
    if ( !m_colorMap.isEmpty() ) {
        int currentRow = startRow, currentCol = startCol;
        while ( currentRow < endRow && currentCol < endCol ) {
            QMap<Position, QColor>::ConstIterator it = m_colorMap.find( Position( currentCol, currentRow ) );
            if( it != m_colorMap.end() )
                p.fillRect( ( currentCol << 4 ) + 2, ( currentRow << 4 ) + 2, 12, 12, it.data() );

            // position of the next cell
            ++currentCol;
            if ( currentCol == endCol ) {
                ++currentRow;
                currentCol = startCol;
            }
        }
    }

    // clean up the last line (it's most likely that it's not totally filled)
    if ( !e->erased() && endRow > lns ) {
        int fields = m_colorMap.count() % COLS;
        erase( fields << 4, lns * TILESIZE, ( COLS - fields ) << 4, 16 );
    }
}

void KoColorPanel::keyPressEvent( QKeyEvent* e )
{
    Position newPos( validPosition( m_focusPosition ) );
    if ( e->key() == Qt::Key_Up ) {
        if ( newPos.y == 0 )
            e->ignore();
        else
            --newPos.y;
    }
    else if ( e->key() == Qt::Key_Down ) {
        if ( newPos < Position( m_colorMap.count() % COLS, lines() - 2 ) )
            ++newPos.y;
        else
            e->ignore();
    }
    else if ( e->key() == Qt::Key_Left ) {
        if ( newPos.x == 0 )
            e->ignore();
        else
            --newPos.x;
    }
    else if ( e->key() == Qt::Key_Right ) {
        if ( newPos.x < COLS - 1 && newPos < Position( m_colorMap.count() % COLS - 1, lines() - 1 ) )
            ++newPos.x;
        else
            e->ignore();
    }
    else if ( e->key() == Qt::Key_Return )
        kdDebug() << "KoColorPanel::keyPressEvent -- return pressed" << endl;
    updateFocusPosition( newPos );
}

void KoColorPanel::focusInEvent( QFocusEvent* e )
{
    if ( !m_colorMap.isEmpty() && m_focusPosition.x == -1 && m_focusPosition.y == -1 ) {
        m_focusPosition.x = 0;
        m_focusPosition.y = 0;
    }
    QWidget::focusInEvent( e );
}

void KoColorPanel::dragEnterEvent( QDragEnterEvent* e )
{
    e->accept( KColorDrag::canDecode( e ) );
}

void KoColorPanel::dropEvent( QDropEvent* e )
{
    QColor color;
    if ( KColorDrag::decode( e, color ) )
        insertColor( color );
}

void KoColorPanel::finalizeInsertion( const Position& pos )
{
    paint( pos );

    if ( !isFocusEnabled() )
        setFocusPolicy( QWidget::StrongFocus );
    // Did we start a new row?
    if ( m_nextPosition.x == 1 )
        updateGeometry();
}

bool KoColorPanel::insertColor( const QColor& color, bool checking )
{
    if ( checking && isAvailable( color ) )
        return false;

    m_colorMap.insert( m_nextPosition, color );

    ++m_nextPosition.x;
    if ( m_nextPosition.x == COLS ) {
        m_nextPosition.x = 0;
        ++m_nextPosition.y;
    }
    return true;
}

bool KoColorPanel::insertColor( const QColor& color, const QString& toolTip, bool checking )
{
    if ( checking && isAvailable( color ) )
        return false;

    // Remember the "old" m_nextPosition -- this is the place where the newly
    // inserted color will be located
    QRect rect( mapFromPosition( m_nextPosition ) );
    insertColor( color, false ); // check only once ;)
    QToolTip::add( this, rect, toolTip );
    return true;
}

bool KoColorPanel::isAvailable( const QColor& color )
{
    // O(n) checking on insert, but this is better than O(n) checking
    // on every mouse move...
    QMap<Position, QColor>::ConstIterator it = m_colorMap.begin();
    QMap<Position, QColor>::ConstIterator end = m_colorMap.end();
    for ( ; it != end; ++it )
        if ( it.data() == color )
            return true;
    return false;
}

KoColorPanel::Position KoColorPanel::mapToPosition( const QPoint& point ) const
{
    return Position( point.x() >> 4, point.y() >> 4 );
}

QColor KoColorPanel::mapToColor( const QPoint& point ) const
{
    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( mapToPosition( point ) );
    if ( it != m_colorMap.end() )
        return it.data();
    return QColor();
}

QRect KoColorPanel::mapFromPosition( const KoColorPanel::Position& position ) const
{
    return QRect( position.x << 4, position.y << 4, TILESIZE, TILESIZE );
}

KoColorPanel::Position KoColorPanel::validPosition( const Position& position )
{
    Position pos( position );
    int lns = lines() - 1;
    int lastLineLen = m_colorMap.count() % COLS - 1;

    // ensure the position is within the valid grid area
    // note: special handling of the last line
    if ( pos.x < 0 )
        pos.x = 0;
    else if ( pos.y == lns && pos.x > lastLineLen )
        pos.x = lastLineLen;
    else if ( pos.x >= COLS )
        pos.x = COLS - 1;

    if ( pos.y < 0 )
        pos.y = 0;
    else if ( pos.x > lastLineLen && pos.y > lns - 1 )
        pos.y = lns - 1;
    else if ( pos.y > lns )
        pos.y = lns;
    return pos;
}

int KoColorPanel::lines() const
{
    if ( m_colorMap.isEmpty() )
        return 1;
    return ( m_colorMap.count() - 1 ) / COLS + 1;
}

void KoColorPanel::paintArea( const QRect& rect, int& startRow, int& endRow, int& startCol, int& endCol ) const
{
    startRow = rect.top() >> 4;
    endRow = ( rect.bottom() >> 4 ) + 1;
    startCol = rect.left() >> 4;
    endCol = ( rect.right() >> 4 ) + 1;
}

void KoColorPanel::updateFocusPosition( const Position& newPosition )
{
    QPainter p( this );

    // restore the old tile where we had the focus before
    if ( m_focusPosition.x != -1 && m_focusPosition.y != -1 )
        paint( m_focusPosition );

    m_focusPosition = newPosition;

    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( m_focusPosition );
    if ( it != m_colorMap.end() ) {
        // draw at the new focus position
        style().drawPrimitive( QStyle::PE_Panel, &p, QRect( m_focusPosition.x << 4, m_focusPosition.y << 4, TILESIZE, TILESIZE ),
                               colorGroup(), QStyle::Style_Sunken | QStyle::Style_Enabled );
        p.fillRect( ( m_focusPosition.x << 4 ) + 2, ( m_focusPosition.y << 4 ) + 2, 12, 12, it.data() );
    }

}

void KoColorPanel::paint( const Position& position )
{
    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( position );
    if ( it == m_colorMap.end() )
        return;

    erase( mapFromPosition( position ) );
    QPainter p( this );
    p.fillRect( ( position.x << 4 ) + 2, ( position.y << 4 ) + 2, 12, 12, it.data() );
}

void KoColorPanel::init()
{
    setFocusPolicy( QWidget::NoFocus ); // it's empty
    m_nextPosition.x = 0;
    m_nextPosition.y = 0;
    m_focusPosition.x = -1;
    m_focusPosition.y = -1;
    m_defaultsAdded = false;
}

bool operator<( const KoColorPanel::Position& lhs, const KoColorPanel::Position& rhs )
{
    return ( lhs.y * COLS + lhs.x ) < ( rhs.y * COLS + rhs.x );
}


KoToolButton::KoToolButton( const QString& icon, int id, QWidget* parent,
                            const char* name, const QString& txt, KInstance* _instance ) :
    KToolBarButton( icon, id, parent, name, txt, _instance )
{
}

KoToolButton::KoToolButton( const QPixmap& pixmap, int id, QWidget* parent,
                            const char* name, const QString& txt ) :
    KToolBarButton( pixmap, id, parent, name, txt )
{
}

KoToolButton::~KoToolButton()
{
}

#include <kotoolbutton.moc>
