/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoList.h"
#include "KoTextDocument.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextBlockData.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"

#include <KDebug>

#include <QTextCursor>
#include <QPointer>
#include <QBitArray>

class KoList::Private
{
public:
    Private(const QTextDocument *document)
    : style(0), textLists(10), document(document)
    {
    }

    ~Private()
    {
    }

    static void invalidate(const QTextBlock &block)
    {
        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(block.userData()))
            userData->setCounterWidth(-1.0);
    }

    KoListStyle *style;
    QVector<QPointer<QTextList> > textLists;
    const QTextDocument *document;
    QMap<int, QVariant> properties;
};

KoList::KoList(const QTextDocument *document, KoListStyle *style)
    : QObject(const_cast<QTextDocument *>(document)), d(new Private(document))
{
    Q_ASSERT(document);
    KoStyleManager *styleManager = KoTextDocument(document).styleManager();
    Q_ASSERT(styleManager);
    d->style = style ? style->clone(this) : styleManager->defaultListStyle();
    KoTextDocument(document).addList(this);
}

KoList::~KoList()
{
    KoTextDocument(d->document).removeList(this);
    delete d;
}

QVector<QPointer<QTextList> > KoList::textLists() const
{
    return d->textLists;
}

KoList *KoList::applyStyle(const QTextBlock &block, KoListStyle *style, int level)
{
    Q_ASSERT(style);
    KoTextDocument document(block.document());
    KoList *list = document.list(block);
    if (list && *list->style() == *style) {
        list->add(block, level);
        return list;
    }

    if (list)
        list->remove(block);

    list = document.list(block.previous());
    if (!list || *list->style() != *style) {
        list = document.list(block.next());
        if (!list || *list->style() != *style) {
            list = new KoList(block.document(), style);
        }
    }
    list->add(block, level);
    return list;
}

void KoList::add(const QTextBlock &block, int level)
{
    if (level == 0) { // fetch the first proper level we have
        level = 1; // if nothing works...
        for (int i = 1; i <= 10; i++) {
            if (d->style->hasLevelProperties(i)) {
                level = i;
                break;
            }
        }
    }

    remove(block);

    QTextList *textList = d->textLists.value(level-1);
    if (!textList) {
        QTextCursor cursor(block);
        QTextListFormat format = d->style->listFormat(level);
        if (continueNumbering(level))
            format.setProperty(KoListStyle::ContinueNumbering, true);
        QTextList *list = cursor.createList(format);
        d->textLists[level-1] = list;
    } else {
        textList->add(block);
    }

    QTextCursor cursor(block);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    if (d->style->styleId()) {
        blockFormat.setProperty(KoParagraphStyle::ListStyleId, d->style->styleId());
    } else {
        blockFormat.clearProperty(KoParagraphStyle::ListStyleId);
    }
    cursor.setBlockFormat(blockFormat);

    d->invalidate(block);
}

void KoList::remove(const QTextBlock &block)
{
    QTextList *textList = block.textList();
    if (textList)
        textList->remove(block);
    Private::invalidate(block);
}

void KoList::setStyle(KoListStyle *style)
{
    delete d->style;
    d->style = style->clone(this);
    for (int i = 0; i < d->textLists.count(); i++) {
        QTextList *textList = d->textLists[i];
        if (!textList)
            continue;
        KoListLevelProperties properties = d->style->levelProperties(i);
        QTextListFormat format;
        properties.applyStyle(format);
        textList->setFormat(format);
        // invalidate the block data too?
    }
}

KoListStyle *KoList::style() const
{
    return d->style;
}

bool KoList::contains(QTextList *list) const
{
    return list && d->textLists.contains(list);
}

void KoList::increaseLevel(const QTextBlock &block)
{
    Q_ASSERT(KoTextDocument(block.document()).list(block) == this);
    QTextList *textList = block.textList();
    int currentLevel = d->textLists.indexOf(textList) + 1;
    Q_ASSERT(currentLevel != 0);
    add(block, currentLevel + 1);
}

void KoList::decreaseLevel(const QTextBlock &block)
{
    Q_ASSERT(KoTextDocument(block.document()).list(block) == this);
}

void KoList::setContinueNumbering(int level, bool enable)
{
    Q_ASSERT(level > 0 && level <= 10);
    level = qMax(qMin(level, 10), 1);

    QBitArray bitArray = d->properties[ContinueNumbering].toBitArray();
    if (bitArray.isEmpty())
        bitArray.resize(10);
    bitArray.setBit(level-1, enable);
    d->properties[ContinueNumbering] = bitArray;

    QTextList *textList = d->textLists[level-1];
    if (!textList)
        return;
    QTextListFormat format = textList->format();
    if (enable) {
        format.setProperty(KoListStyle::ContinueNumbering, true);
    } else {
        format.clearProperty(KoListStyle::ContinueNumbering);
    }
    textList->setFormat(format);
}

bool KoList::continueNumbering(int level) const
{
    Q_ASSERT(level > 0 && level <= 10);
    level = qMax(qMin(level, 10), 1);

    QBitArray bitArray = d->properties.value(ContinueNumbering).toBitArray();
    if (bitArray.isEmpty())
        return false;
    return bitArray.testBit(level-1);
}
