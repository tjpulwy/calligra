/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#include <qheader.h>
#include <qpoint.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>

#include "kexi.h"
#include "kexipart.h"
#include "kexipartinfo.h"
#include "kexipartitem.h"
#include "kexibrowser.h"
#include "kexibrowseritem.h"
#include "kexiproject.h"
#include "kexidialogbase.h"
#include "keximainwindow.h"


//KexiBrowser::KexiBrowser(KexiMainWindow *parent, QString mime, KexiPart::Info *part )
KexiBrowser::KexiBrowser(KexiMainWindow *parent )
 : KListView(parent,"KexiBrowser")
 , KexiActionProxy(parent, this)
{
	m_parent = parent;
//	m_ac = m_parent->actionCollection();
//	KexiActionProxy ap;
	plugAction("edit_remove",SLOT(slotRemove()));

	setCaption(i18n("Navigator"));
	setIcon(*parent->icon());

	header()->hide();
	addColumn("");
	setShowToolTips(true);
	setRootIsDecorated(true);
	setSorting(0);
	sort();
//	setResizeMode(QListView::LastColumn);

	connect(this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
		this, SLOT(slotContextMenu(KListView*, QListViewItem *, const QPoint&)));
	connect(this, SIGNAL(executed(QListViewItem*)), this, SLOT(slotExecuteItem(QListViewItem*)));
//js todo: ADD OPTION for enable this:
//connect(this, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(slotExecuteItem(QListViewItem*)));
	connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)));
	connect(this, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotClicked(QListViewItem*)));
	connect(this, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(slotExecuteItem(QListViewItem*)));

/*	if(part)
	{
//		connect(part, SIGNAL(itemListChanged(KexiProjectHandler *)), this, SLOT(slotItemListChanged(KexiProjectHandler *)));
//		slotItemListChanged(part);
	}
*/
}

void
KexiBrowser::addGroup(KexiPart::Info *info)
{
//	KexiBrowserItem *item = new KexiBrowserItem(this, info->mime(), info->groupName(), 0, info);
	KexiBrowserItem *item = new KexiBrowserItem(this, info);
//	item->setPixmap(0, SmallIcon(info->groupIcon()));
//	item->setOpen(true);
//	item->setSelectable(false);
	m_baseItems.insert(info->mime().lower(), item);

	kdDebug() << "KexiBrowser::addGroup()" << endl;
//js: now it's executed by hand from keximainwindow:	slotItemListChanged(info);
}

void
KexiBrowser::addItem(KexiPart::Item *item)
{
	if (!item)
		return;
	//part object for this item
	KexiBrowserItem *parent = m_baseItems.find(item->mime().lower());
	if (!parent) //TODO: add "Other" part group for that
		return;
	kdDebug() << "KexiBrowser::addItem() found parent:" << parent << endl;
//	KexiBrowserItem *bitem = new KexiBrowserItem(parent, item.mime(), item.name(), item.identifier());
	KexiBrowserItem *bitem = new KexiBrowserItem(parent, parent->info(), item);
//	bitem->setPixmap(0, SmallIcon(parent->info()->itemIcon()));

#if 0 //nonsense since we have no multi tab bar now
	if(m_mime == "kexi/db" && m_baseItems.find(item.mime()))
	{
		//part object
		KexiBrowserItem *parent = m_baseItems.find(item.mime());
		kdDebug() << "KexiBrowser::addItem() found parent:" << parent->name() << endl;
		KexiBrowserItem *bitem = new KexiBrowserItem(parent, item.mime(), item.name(), item.identifier());
		bitem->setPixmap(0, SmallIcon(parent->info()->itemIcon()));
	}
	else if(m_mime == item.mime())
	{
		//part objects group
		KexiBrowserItem *bitem = new KexiBrowserItem(this, item.mime(), item.name(), item.identifier(), 0);
		if(m_part)
			bitem->setPixmap(0, SmallIcon(m_part->itemIcon()));
	}
#endif
}

#if 0
//js: moved to keximainwindow
void
KexiBrowser::slotItemListChanged(KexiPart::Info *parent)
{
	kdDebug() << "KexiBrowser::slotItemListChanged()" << endl;
	if(m_mime == "kexi/db")
	{
		KexiBrowserItem *group = m_baseItems.find(parent->mime());
		kdDebug() << "KexiBrowser::slotItemListChanged(): group=" << group  << " mime=" << parent->mime()<< endl;
		group->clearChildren();
	}
	else
	{
		clear();
	}

	KexiPart::ItemList list = m_parent->project()->items(parent);
	kdDebug() << "KexiBrowser::slotItemListChanged(): list count:" << list.count() << endl;
	for(KexiPart::ItemList::Iterator it = list.begin(); it != list.end(); ++it)
	{
		kdDebug() << "KexiBrowser::slotItemListChanged() adding item" << (*it).mime() << endl;
		addItem(*it);
	}
}
#endif

void
KexiBrowser::slotContextMenu(KListView *list, QListViewItem *item, const QPoint &)
{
	if(!item)
		return;
//	KexiBrowserItem *bit = static_cast<KexiBrowserItem*>(item);
//	if (bit

#if 0
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(item);
	if(!it)
		return;

	if(it->proxy() || it->item())
	{
		KexiPartPopupMenu *pg = 0;
		if(it->identifier().isNull())
		{
			// FIXME: Make this less hacky please :)
			pg = it->proxy()->groupContextMenu();
		}
		else
		{
			kdDebug() << "KexiBrowser::slotContextMenu() item @ " << it->item() << endl;
			//a littlebit hacky
			pg = it->item()->handler()->proxy(
			static_cast<KexiDialogBase*>(parent()->parent())->kexiView()
			)->itemContextMenu(it->identifier());
		}

		pg->setPartItemId(it->identifier());
		pg->exec(pos);
//		delete pg;
	}
#endif
}

void
KexiBrowser::slotExecuteItem(QListViewItem *vitem)
{
	kdDebug() << "KexiBrowser::slotExecuteItem()" << endl;
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(vitem);

	if (!it->item())
		return;
	emit executeItem( it->item() );

/*	if(m_parent->activateWindow(it->item().identifier()))
		return;

	if(!it || it->info())
		return;

	KexiPart::Item item;
	item.setName(it->name());
	item.setIdentifier(it->identifier());
	item.setMime(it->mime());

	kdDebug() << "KexiBrowser::slotExecuteItem() searching stuff for mime: " << it->mime() << endl;
//	KexiPart::Info *info = m_parent->project()->partManager()->info(it->mime());
//	if(!info)
//		return;

//	kdDebug() << "KexiBrowser::slotExecuteItem() info=" << info << endl;

	KexiPart::Part *part = Kexi::partManager().part(it->mime());
//	if(!info->instance())
	if (!part)
		return;

//	info->instance()->execute(m_parent, it->name());
	part->execute(m_parent, item);
	*/
}

void
KexiBrowser::slotSelectionChanged(QListViewItem* i)
{
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(i);
	bool gotitem = it && it->item();
	setAvailable("edit_remove",gotitem);
}

void
KexiBrowser::slotClicked(QListViewItem* i)
{
	//workaround for non-selectable item
	if (!i || !static_cast<KexiBrowserItem*>(i)->item())
		slotSelectionChanged(i);
}

/*bool KexiBrowser::actionAvailable(const char *name)
{
	if (qstrcmp(name,"edit_remove")==0)
		return selectedItem() && static_cast<KexiBrowserItem*>(selectedItem())->item();

	return false;
}
*/

void KexiBrowser::slotRemove()
{
	kdDebug() << "KexiBrowser::slotRemove()" << endl;
}

#include "kexibrowser.moc"

