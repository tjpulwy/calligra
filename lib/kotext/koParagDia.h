/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef paragdia_h
#define paragdia_h

#include <kdialogbase.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <koRuler.h>
#include <koUnit.h>
#include <qdict.h>
#include <qlineedit.h>
#include "kotextparag.h"
#include <koparagcounter.h>

class KButtonBox;
class KColorButton;
class KoBorderPreview;
class KoNumPreview;
class KPagePreview2;
class KPagePreview;
class KoSpinBox;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QListBox;
class QPushButton;
class QRadioButton;
class QWidget;

/**
 * This is the base class for any widget [usually used in a tab]
 * that handles modifying a part of a KoParagLayout.
 * Used by the paragraph dialog (this file) and by the style editor.
 */
class KoParagLayoutWidget : public QWidget
{
    Q_OBJECT
public:
    // Constructor: parent widget, flag (PD_SOMETHING) and name
    KoParagLayoutWidget( int flag, QWidget * parent, const char * name = 0 )
        : QWidget( parent, name ), m_flag( flag )
    {
    }
    virtual ~KoParagLayoutWidget() {}

    // Display settings, from the paragLayout
    virtual void display( const KoParagLayout & lay ) = 0;

    // Save the settings, into the paragLayout
    // This is only used by the stylist, not by paragdia (which needs undo/redo, applying partially etc.)
    virtual void save( KoParagLayout & lay ) = 0;

    // Return true if the settings where modified
    // ## maybe too global, but how to do it differently? We'll see if we need this.
    //virtual bool isModified() = 0;

    /**  return the (i18n-ed) name of the tab */
    virtual QString tabName() = 0;

    // Return the part of the paraglayout that this widget cares about
    int flag() const { return m_flag; }

private:
    int m_flag;
};

/**
 * The widget for editing idents and spacings (tab 1)
 */
class KoIndentSpacingWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoIndentSpacingWidget( KoUnit::Unit unit, bool breakLine, double _frameWidth, QWidget * parent,const char * name = 0 );
    virtual ~KoIndentSpacingWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    double leftIndent() const;
    double rightIndent() const;
    double firstLineIndent() const;
    double spaceBeforeParag() const;
    double spaceAfterParag() const;
    double lineSpacing() const;
    int pageBreaking() const;

private slots:
    void leftChanged( const QString & );
    void rightChanged( const QString & );
    void firstChanged( const QString & );
    void spacingActivated( int );
    void spacingChanged( const QString & );
    void beforeChanged( const QString & );
    void afterChanged( const QString & );
private:
    QLineEdit *eLeft, *eRight, *eFirstLine, *eBefore, *eAfter, *eSpacing;
    QComboBox *cSpacing;
    QCheckBox *cKeepLinesTogether, *cHardBreakBefore, *cHardBreakAfter;
    KPagePreview *prev1;
    KoUnit::Unit m_unit;
};

/**
 * The widget for editing paragraph alignment (tab 2)
 */
class KoParagAlignWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagAlignWidget( QWidget * parent, const char * name = 0 );
    virtual ~KoParagAlignWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    int align() const;

protected slots:
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();

protected:
    void clearAligns();

private:
    QRadioButton *rLeft, *rCenter, *rRight, *rJustify;
    KPagePreview2 *prev2;
};

/**
 * The widget for editing paragraph borders (tab 3)
 */
class KoParagBorderWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagBorderWidget( QWidget * parent, const char * name = 0 );
    virtual ~KoParagBorderWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    KoBorder leftBorder() const { return m_leftBorder; }
    KoBorder rightBorder() const { return m_rightBorder; }
    KoBorder topBorder() const { return m_topBorder; }
    KoBorder bottomBorder() const { return m_bottomBorder; }

protected:
    void updateBorders();

protected slots:
    void brdLeftToggled( bool );
    void brdRightToggled( bool );
    void brdTopToggled( bool );
    void brdBottomToggled( bool );
    //void brdStyleChanged( const QString & );
    //void brdWidthChanged( const QString & );
    //void brdColorChanged( const QColor& );
    void slotPressEvent(QMouseEvent *_ev);

private:
    QComboBox *cWidth, *cStyle;
    QPushButton *bLeft, *bRight, *bTop, *bBottom;
    KColorButton *bColor;
    KoBorder m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;
    KoBorderPreview *prev3;
};

/**
 * The widget for editing counters (bullets & numbering) (tab 4)
 */
class KoParagCounterWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:

    class StyleRepresenter {
        public:
            StyleRepresenter (const QString name, KoParagCounter::Style style, bool listStyle=false) {
                m_name=name;
                m_style=style;
                m_listStyle=listStyle;
            }
            QString name() { return m_name; }
            KoParagCounter::Style style() { return m_style; }
            bool listStyle() { return m_listStyle; }

        private:
            QString m_name;
            KoParagCounter::Style m_style;
            bool m_listStyle;
    };

    KoParagCounterWidget( QWidget * parent, const char * name = 0 );
    virtual ~KoParagCounterWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    const KoParagCounter & counter() const { return m_counter; }

protected slots:
    void selectCustomBullet();
    void numStyleChanged(); // selected another style from the combobox
    void numTypeChanged( int );  // selected another type radiobutton.

    void changeKWSpinboxType();
    void suffixChanged(const QString & txt) {m_counter.setSuffix(txt); updatePreview(); }
    void prefixChanged(const QString & txt) {m_counter.setPrefix(txt);  updatePreview();}
    void startChanged(int i) {m_counter.setStartNumber(i);  updatePreview();}
    void depthChanged(int i) {m_counter.setDepth(i);  updatePreview();}

private:
    void updatePreview();
    QPtrList <StyleRepresenter> stylesList;
    void fillStyleCombo(KoParagCounter::Numbering type = KoParagCounter::NUM_LIST);

    QGroupBox *gStyle;
    QButtonGroup *gNumbering;
    QListBox *lstStyle;
    KoParagCounter m_counter;
    QLineEdit *sSuffix, *sPrefix;
    QPushButton *bCustom;
    KoSpinBox *spnStart;
    QSpinBox *spnDepth;
    QLabel *lStart;
    QLabel *lCustom;
    KoNumPreview *preview;

    unsigned int styleBuffer;
    bool noSignals;
};

/**
 *
 */
class KoTabulatorsLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    KoTabulatorsLineEdit ( QWidget * parent, const char * name=0 );

protected:
    virtual void keyPressEvent ( QKeyEvent * );
 signals:
    void keyReturnPressed();
};

/**
 * The widget for editing tabulators (tab 5)
 */
class KoParagTabulatorsWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagTabulatorsWidget( KoUnit::Unit unit, double _frameWidth, QWidget * parent, const char * name = 0 );
    virtual ~KoParagTabulatorsWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    virtual QString tabName();

    KoTabulatorList tabList() const { return m_tabList; }

    void setCurrentTab( double tabPos );

protected slots:
    void slotTabValueChanged( const QString &_text );
    void slotAlignCharChanged( const QString &_text );
    void newClicked();
    void deleteClicked();
    void setActiveItem(int selected);
    void updateAlign(int selected);

protected:

    void sortLists();
    QString tabToString(const KoTabulator *tab);

    QVBoxLayout* editLayout;

    QListBox* lstTabs;
    QGroupBox* gPosition;
    KoTabulatorsLineEdit* sTabPos;
    QButtonGroup* bgAlign;
    QRadioButton* rAlignLeft;
    QRadioButton* rAlignCentre;
    QRadioButton* rAlignRight;
    QRadioButton* rAlignVar;
    QLineEdit* sAlignChar;
    QGroupBox* gTabLeader;
    QComboBox* cFilling;
    QPushButton* bNew;
    QPushButton* bDelete;

    KoTabulatorList m_tabList;
    KoUnit::Unit m_unit;
    double m_toplimit;
    bool noSignals;
};

class KoShadowPreview : public QFrame
{
    Q_OBJECT

public:
    // constructor - destructor
    KoShadowPreview( QWidget* parent, const char* );
    ~KoShadowPreview() {}

    void setShadowDirection( short int sd ) { shadowDirection = sd; repaint( true ); }
    void setShadowDistance( int sd ) { shadowDistance = sd; repaint( true ); }
    void setShadowColor( QColor sc ) { shadowColor = sc; repaint( true ); }

protected:
    void drawContents( QPainter* );

    short int shadowDirection;
    int shadowDistance;
    QColor shadowColor;
};


class KoParagShadowWidget: public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagShadowWidget( QWidget * parent, const char * name=0 );
    virtual ~KoParagShadowWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    virtual QString tabName();

    void setShadowDirection( short int sd );
    void setShadowDistance( int sd );
    void setShadowColor( const QColor & sc );

    short int getShadowDirection() { return shadowDirection; }
    int getShadowDistance() { return shadowDistance; }
    QColor getShadowColor() { return shadowColor; }

protected:
    KoShadowPreview *m_shadowPreview;

    QSpinBox *distance;
    QPushButton *lu, *u, *ru, *r, *rb, *b, *lb, *l;
    KColorButton *color;
    QLabel *lcolor, *ldirection, *ldistance;

    short int shadowDirection;
    int shadowDistance;
    QColor shadowColor;

protected slots:
    void luChanged();
    void uChanged();
    void ruChanged();
    void rChanged();
    void rbChanged();
    void bChanged();
    void lbChanged();
    void lChanged();
    void colorChanged( const QColor& );
    void distanceChanged( int );

};

/**
 * The complete(*) dialog for changing attributes of a paragraph
 *
 * (*) the flags (to only show parts of it) have been kept just in case
 * but are not currently used.
 */
class KoParagDia : public KDialogBase
{
    Q_OBJECT

public:
    enum { PD_SPACING = 1, PD_ALIGN = 2, PD_BORDERS = 4, PD_NUMBERING = 8, PD_TABS = 16, PD_SHADOW=32 };

    /**
     * breakLine : kpresenter didn't used this attibute, kword use it.
     */
    KoParagDia( QWidget*, const char*, int flags, KoUnit::Unit unit, double _frameWidth=-1,bool breakLine=true);
    ~KoParagDia();

    int getFlags() { return m_flags; }

    /** Make a given page the current one - @p page is a flag (PD_something) value */
    void setCurrentPage( int page );

    /** Set the values to be displayed */
    void setParagLayout( const KoParagLayout & lay );

    // Get values (in pt) - tab 1
    double leftIndent() const { return m_indentSpacingWidget->leftIndent(); }
    double rightIndent() const { return m_indentSpacingWidget->rightIndent(); }
    double firstLineIndent() const { return m_indentSpacingWidget->firstLineIndent(); }
    double spaceBeforeParag() const { return m_indentSpacingWidget->spaceBeforeParag(); }
    double spaceAfterParag() const { return m_indentSpacingWidget->spaceAfterParag(); }
    double lineSpacing() const { return m_indentSpacingWidget->lineSpacing(); }
    int pageBreaking() const { return m_indentSpacingWidget->pageBreaking(); }

    // tab 2
    int align() const { return m_alignWidget->align(); }

    // tab 3
    KoBorder leftBorder() const { return m_borderWidget->leftBorder(); }
    KoBorder rightBorder() const { return m_borderWidget->rightBorder(); }
    KoBorder topBorder() const { return m_borderWidget->topBorder(); }
    KoBorder bottomBorder() const { return m_borderWidget->bottomBorder(); }

    // tab 4
    const KoParagCounter & counter() const { return m_counterWidget->counter(); }

    // tab 5
    KoTabulatorList tabListTabulator() const { return m_tabulatorsWidget->tabList(); }
    KoParagTabulatorsWidget * tabulatorsWidget() const { return m_tabulatorsWidget; }

    // tab 6
    short int shadowDirection()const {return  m_shadowWidget->getShadowDirection();}
    int shadowDistance() const { return m_shadowWidget->getShadowDistance(); }
    QColor shadowColor() const { return m_shadowWidget->getShadowColor(); }

    // Support for "what has changed?"
    bool isAlignChanged() const {return oldLayout.alignment!=align();}
    bool isLineSpacingChanged() const {return oldLayout.lineSpacing!=lineSpacing();}
    bool isLeftMarginChanged() const { return oldLayout.margins[QStyleSheetItem::MarginLeft]!=leftIndent(); }
    bool isRightMarginChanged() const { return oldLayout.margins[QStyleSheetItem::MarginRight]!=rightIndent();}
    bool isFirstLineChanged() const {return oldLayout.margins[ QStyleSheetItem::MarginFirstLine]!=firstLineIndent();}
    bool isSpaceBeforeChanged() const { return oldLayout.margins[QStyleSheetItem::MarginTop]!=spaceBeforeParag();}
    bool isSpaceAfterChanged() const {return oldLayout.margins[QStyleSheetItem::MarginBottom]!=spaceAfterParag();}
    bool isPageBreakingChanged() const { return oldLayout.pageBreaking!=pageBreaking(); }
    bool isCounterChanged() const;

    bool isBorderChanged() const { return (oldLayout.leftBorder!=leftBorder() ||
                                           oldLayout.rightBorder!=rightBorder() ||
                                           oldLayout.topBorder!=topBorder() ||
                                           oldLayout.bottomBorder!=bottomBorder() ); }
    bool listTabulatorChanged() const {return oldLayout.tabList()!=tabListTabulator();}

    bool isShadowChanged() const { return (oldLayout.shadowDistance!=shadowDistance() || 
					  oldLayout.shadowColor !=shadowColor() 
					  ||  oldLayout.shadowDirection!=shadowDirection());}

private:
    KoIndentSpacingWidget * m_indentSpacingWidget;
    KoParagAlignWidget * m_alignWidget;
    KoParagBorderWidget * m_borderWidget;
    KoParagCounterWidget * m_counterWidget;
    KoParagTabulatorsWidget * m_tabulatorsWidget;
    KoParagShadowWidget *m_shadowWidget;

    int m_flags;
    KoParagLayout oldLayout;
};

#endif
