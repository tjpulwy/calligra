/* This file is part of the KDE project
  Copyright (C) 2007 Dag Andersen <danders@get2net>
  Copyright (C) 2011, 2012 Dag Andersen <danders@get2net.dk>

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

#include "kptaccountseditor.h"

#include "kptcommand.h"
#include "kptcalendar.h"
#include "kptduration.h"
#include "kptnode.h"
#include "kptproject.h"
#include "kpttask.h"
#include "kptaccount.h"
#include "kptdatetime.h"
#include "kptdebug.h"

#include <KoDocument.h>
#include <KoPageLayoutWidget.h>
#include <KoIcon.h>

#include <QList>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include <KLocalizedString>
#include <kactioncollection.h>


namespace KPlato
{


AccountseditorConfigDialog::AccountseditorConfigDialog( ViewBase *view, AccountTreeView *treeview, QWidget *p)
    : KPageDialog(p),
    m_view( view ),
    m_treeview( treeview )
{
    setWindowTitle( i18n("Settings") );

    QTabWidget *tab = new QTabWidget();

    QWidget *w = ViewBase::createPageLayoutWidget( view );
    tab->addTab( w, w->windowTitle() );
    m_pagelayout = w->findChild<KoPageLayoutWidget*>();
    Q_ASSERT( m_pagelayout );

    m_headerfooter = ViewBase::createHeaderFooterWidget( view );
    m_headerfooter->setOptions( view->printingOptions() );
    tab->addTab( m_headerfooter, m_headerfooter->windowTitle() );

    KPageWidgetItem *page = addPage( tab, i18n( "Printing" ) );
    page->setHeader( i18n( "Printing Options" ) );

    connect( this, SIGNAL(accepted()), this, SLOT(slotOk()));
}

void AccountseditorConfigDialog::slotOk()
{
    debugPlan;
    m_view->setPageLayout( m_pagelayout->pageLayout() );
    m_view->setPrintingOptions( m_headerfooter->options() );
}


//--------------------
AccountTreeView::AccountTreeView( QWidget *parent )
    : TreeViewBase( parent )
{
    header()->setContextMenuPolicy( Qt::CustomContextMenu );
    setModel( new AccountItemModel( this ) );
    setSelectionModel( new QItemSelectionModel( model() ) );
    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    setAcceptDrops( false );
    setDropIndicatorShown( false );
    
    connect( header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(headerContextMenuRequested(QPoint)) );
}

void AccountTreeView::headerContextMenuRequested( const QPoint &pos )
{
    debugPlan<<header()->logicalIndexAt(pos)<<" at"<<pos;
}

void AccountTreeView::contextMenuEvent ( QContextMenuEvent *event )
{
    debugPlan;
    emit contextMenuRequested( indexAt(event->pos()), event->globalPos() );
}

void AccountTreeView::selectionChanged( const QItemSelection &sel, const QItemSelection &desel )
{
    debugPlan<<sel.indexes().count();
    foreach( const QModelIndex &i, selectionModel()->selectedIndexes() ) {
        debugPlan<<i.row()<<","<<i.column();
    }
    QTreeView::selectionChanged( sel, desel );
    emit selectionChanged( selectionModel()->selectedIndexes() );
}

void AccountTreeView::currentChanged( const QModelIndex & current, const QModelIndex & previous )
{
    debugPlan;
    QTreeView::currentChanged( current, previous );
    emit currentChanged( current );
    // possible bug in qt: in QAbstractItemView::SingleSelection you can select multiple items/rows
    selectionModel()->select( current, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

Account *AccountTreeView::currentAccount() const
{
    return model()->account( currentIndex() );
}

Account *AccountTreeView::selectedAccount() const
{
    QModelIndexList lst = selectionModel()->selectedRows();
    if ( lst.count() == 1 ) {
        return model()->account( lst.first() );
    }
    return 0;
}

QList<Account*> AccountTreeView::selectedAccounts() const
{
    QList<Account*> lst;
    foreach ( const QModelIndex &i, selectionModel()->selectedRows() ) {
        Account *a = model()->account( i );
        if ( a ) {
            lst << a;
        }
    }
    return lst;
}


//-----------------------------------
AccountsEditor::AccountsEditor(KoPart *part, KoDocument *doc, QWidget *parent)
    : ViewBase(part, doc, parent)
{
    setupGui();
    
    QVBoxLayout * l = new QVBoxLayout( this );
    l->setMargin( 0 );
    m_view = new AccountTreeView( this );
    l->addWidget( m_view );
    m_view->setEditTriggers( m_view->editTriggers() | QAbstractItemView::EditKeyPressed );

    connect( model(), SIGNAL(executeCommand(KUndo2Command*)), doc, SLOT(addCommand(KUndo2Command*)) );
    
    connect( m_view, SIGNAL(currentChanged(QModelIndex)), this, SLOT(slotCurrentChanged(QModelIndex)) );

    connect( m_view, SIGNAL(selectionChanged(QModelIndexList)), this, SLOT(slotSelectionChanged(QModelIndexList)) );
    
    connect( m_view, SIGNAL(contextMenuRequested(QModelIndex,QPoint)), this, SLOT(slotContextMenuRequested(QModelIndex,QPoint)) );
    connect( m_view, SIGNAL(headerContextMenuRequested(QPoint)), SLOT(slotHeaderContextMenuRequested(QPoint)) );
}

void AccountsEditor::updateReadWrite( bool readwrite )
{
    m_view->setReadWrite( readwrite );
}

void AccountsEditor::draw( Project &project )
{
    m_view->setProject( &project );
}

void AccountsEditor::draw()
{
}

void AccountsEditor::setGuiActive( bool activate )
{
    debugPlan<<activate;
    updateActionsEnabled( true );
    ViewBase::setGuiActive( activate );
    if ( activate ) {
        if ( !m_view->currentIndex().isValid() ) {
            m_view->selectionModel()->setCurrentIndex(m_view->model()->index( 0, 0 ), QItemSelectionModel::NoUpdate);
        }
        slotSelectionChanged( m_view->selectionModel()->selectedRows() );
    }
}

void AccountsEditor::slotContextMenuRequested( const QModelIndex &index, const QPoint& pos )
{
    debugPlan<<index.row()<<","<<index.column()<<":"<<pos;
    slotHeaderContextMenuRequested( pos );
}

void AccountsEditor::slotHeaderContextMenuRequested( const QPoint &pos )
{
    debugPlan;
    QList<QAction*> lst = contextActionList();
    if ( ! lst.isEmpty() ) {
        QMenu::exec( lst, pos,  lst.first() );
    }
}

Account *AccountsEditor::currentAccount() const
{
    return m_view->currentAccount();
}

void AccountsEditor::slotCurrentChanged(  const QModelIndex &curr )
{
    debugPlan<<curr.row()<<","<<curr.column();
    //slotEnableActions( curr.isValid() );
}

void AccountsEditor::slotSelectionChanged( const QModelIndexList& list)
{
    debugPlan<<list.count();
    updateActionsEnabled( true );
}

void AccountsEditor::slotEnableActions( bool on )
{
    updateActionsEnabled( on );
}

void AccountsEditor::updateActionsEnabled(  bool on )
{
    QList<Account*> lst = m_view->selectedAccounts();
    bool one = lst.count() == 1;
    bool more = lst.count() > 1;
    actionAddAccount->setEnabled( on && !more );
    actionAddSubAccount->setEnabled( on && one );

    bool baselined = project() ? project()->isBaselined() : false;
    actionDeleteSelection->setEnabled( on && one && ! baselined );
}

void AccountsEditor::setupGui()
{
    QString name = "accountseditor_edit_list";
    
    actionAddAccount  = new QAction(koIcon("document-new"), i18n("Add Account"), this);
    actionCollection()->addAction("add_account", actionAddAccount );
    actionCollection()->setDefaultShortcut(actionAddAccount, Qt::CTRL + Qt::Key_I);
    connect( actionAddAccount, SIGNAL(triggered(bool)), SLOT(slotAddAccount()) );
    addAction( name, actionAddAccount );

    actionAddSubAccount  = new QAction(koIcon("document-new"), i18n("Add Subaccount"), this);
    actionCollection()->addAction("add_subaccount", actionAddSubAccount );
    actionCollection()->setDefaultShortcut(actionAddSubAccount, Qt::SHIFT + Qt::CTRL + Qt::Key_I);
    connect( actionAddSubAccount, SIGNAL(triggered(bool)), SLOT(slotAddSubAccount()) );
    addAction( name, actionAddSubAccount );

    actionDeleteSelection  = new QAction(koIcon("edit-delete"), i18nc("@action", "Delete"), this);
    actionCollection()->addAction("delete_selection", actionDeleteSelection );
    actionCollection()->setDefaultShortcut(actionDeleteSelection, Qt::Key_Delete);
    connect( actionDeleteSelection, SIGNAL(triggered(bool)), SLOT(slotDeleteSelection()) );
    addAction( name, actionDeleteSelection );

    createOptionAction();
}

void AccountsEditor::slotOptions()
{
    debugPlan;
    AccountseditorConfigDialog *dlg = new AccountseditorConfigDialog( this, m_view, this );
    connect(dlg, SIGNAL(finished(int)), SLOT(slotOptionsFinished(int)));
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

void AccountsEditor::slotAddAccount()
{
    debugPlan;
    int row = -1;
    Account *parent = m_view->selectedAccount(); // sibling
    if ( parent ) {
        row = parent->parent() ? parent->parent()->indexOf( parent ) : project()->accounts().indexOf( parent );
        if ( row >= 0 ) {
            ++row;
        }
        parent = parent->parent();
    }
    insertAccount( new Account(), parent, row );
}

void AccountsEditor::slotAddSubAccount()
{
    debugPlan;
    insertAccount( new Account(), m_view->selectedAccount(), -1 );
}

void AccountsEditor::insertAccount( Account *account, Account *parent, int row )
{
    m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
    QModelIndex i = m_view->model()->insertAccount( account, parent, row );
    if ( i.isValid() ) {
        QModelIndex p = m_view->model()->parent( i );
        if (parent) debugPlan<<" parent="<<parent->name()<<":"<<p.row()<<","<<p.column();
        debugPlan<<i.row()<<","<<i.column();
        if ( p.isValid() ) {
            m_view->setExpanded( p, true );
        }
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
        m_view->edit( i );
    }
}

void AccountsEditor::slotDeleteSelection()
{
    debugPlan;
    m_view->model()->removeAccounts( m_view->selectedAccounts() );
}

void AccountsEditor::slotAccountsOk()
{
     debugPlan<<"Account Editor : slotAccountsOk";
     //QModelList
     

     //QModelIndex i = m_view->model()->insertGroup( g );
     
}

KoPrintJob *AccountsEditor::createPrintJob()
{
    return m_view->createPrintJob( this );
}

} // namespace KPlato
