/* This file is part of the KDE project
   Copyright (C) 2002 Bo Thorsen  bo@suse.de

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

#include "kptnodeitem.h"
#include "kptnode.h"


KPTNodeItem::KPTNodeItem(KListView *lv, KPTNode &n)
    : KListViewItem(lv, n.name(), n.leader(), n.description()), node(n)
{
}

KPTNodeItem::KPTNodeItem(KPTNodeItem *p, KPTNode &n)
    : KListViewItem(p, n.name(), n.leader(), n.description()), node(n)
{
}

KPTNodeItem::KPTNodeItem(KPTNodeItem *p, KPTNode *n)
    : KListViewItem(p, n->name(), n->leader(), n->description()), node(*n)
{
}

KPTNodeItem::KPTNodeItem(KPTNodeItem *p, QListViewItem *after, KPTNode *n)
    : KListViewItem(p, after, n->name(), n->leader(), n->description()), node(*n)
{
}

void KPTNodeItem::openDialog()
{
    if (node.openDialog())
    {
        setText(0,node.name());
        setText(1,node.leader());
        setText(2,node.description());
        repaint();
    }
}

QListViewItem *KPTNodeItem::lastChild()
{
    QListViewItem *i = firstChild();
    if (i) {
        while (i->nextSibling())
            i = i->nextSibling();
    }
    return i;
}
