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

#ifndef _kotoolbutton_h_
#define _kotoolbutton_h_

#include <ktoolbarbutton.h>
#include <qmap.h>
#include <qpoint.h>

class KoColorPanel : public QWidget
{
    Q_OBJECT
public:
    // The position of the 16x16 tiles in "tile steps"
    struct Position {
        Position() : x( -1 ), y( -1 ) {}
        Position( short x_, short y_ ) : x( x_ ), y( y_ ) {}

        short x;
        short y;
    };

    KoColorPanel( QWidget* parent = 0, const char* name = 0 );
    virtual ~KoColorPanel();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    void clear();
    void insertColor( const QColor& color );
    void insertColor( const QColor& color, const QString& toolTip );
    void insertDefaultColors();

signals:
    void colorSelected( const QColor& color );

protected:
    virtual void mousePressEvent( QMouseEvent* e );
    virtual void mouseMoveEvent( QMouseEvent* e );
    virtual void paintEvent( QPaintEvent* e );
    virtual void keyPressEvent( QKeyEvent* e );
    virtual void focusInEvent( QFocusEvent* e );
    virtual void dragEnterEvent( QDragEnterEvent* e );
    virtual void dropEvent( QDropEvent* e );

private:
    void finalizeInsertion( const Position& pos );
    bool insertColor( const QColor& color, bool checking );
    bool insertColor( const QColor& color, const QString& toolTip, bool checking );
    bool isAvailable( const QColor& color );

    Position mapToPosition( const QPoint& point ) const;
    QColor mapToColor( const QPoint& point ) const;
    QRect mapFromPosition( const Position& position ) const;
    Position validPosition( const Position& position );

    int lines() const;
    void paintArea( const QRect& rect, int& startRow, int& endRow, int& startCol, int& endCol ) const;
    void updateFocusPosition( const Position& newPosition );
    void paint( const Position& position );
    void init();

    Position m_nextPosition, m_focusPosition;
    QMap<Position, QColor> m_colorMap;
    QPoint m_pressedPos;
    bool m_defaultsAdded;
};

// Needed for the use of KoColorPanel::Position in QMap
bool operator<( const KoColorPanel::Position& lhs, const KoColorPanel::Position& rhs );


class KoToolButton : public KToolBarButton
{
    Q_OBJECT
public:
    /**
     * Construct a button with an icon loaded by the button itself.
     * This will trust the button to load the correct icon with the
     * correct size.
     *
     * @param icon   Name of icon to load (may be absolute or relative)
     * @param id     Id of this button
     * @param parent This button's parent
     * @param name   This button's internal name
     * @param txt    This button's text (in a tooltip or otherwise)
     */
    KoToolButton( const QString& icon, int id, QWidget* parent,
                  const char* name = 0L, const QString& txt = QString::null,
                  KInstance* _instance = KGlobal::instance() );

    /**
     * Construct a button with an existing pixmap.  It is not
     * recommended that you use this as the internal icon loading code
     * will almost always get it "right".
     *
     * @param icon   Name of icon to load (may be absolute or relative)
     * @param id     Id of this button
     * @param parent This button's parent
     * @param name   This button's internal name
     * @param txt    This button's text (in a tooltip or otherwise)
     */
    KoToolButton( const QPixmap& pixmap, int id, QWidget* parent,
                  const char* name = 0L, const QString& txt = QString::null );

    virtual ~KoToolButton();
};

#endif // _kotoolbutton_h_
