/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2014 Denis Kuplyakov <dener.kup@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CAUSECTIONSEMANTICITEMFACTORY_H
#define CAUSECTIONSEMANTICITEMFACTORY_H

#include <KoRdfSemanticItemFactoryBase.h>

/**
 * Factory for CAuSectionRdf
 * TODO: maybe we can implement one templated factory for all CAuSemanticItemBase derived items
 */
class CAuSectionSemanticItemFactory : public KoRdfSemanticItemFactoryBase
{
public:
    CAuSectionSemanticItemFactory();

public: // KoRdfSemanticItemFactoryBase API
    virtual QString className() const;
    virtual QString classDisplayName() const;

    virtual void updateSemanticItems(QList<hKoRdfBasicSemanticItem> &semanticItems, const KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> m);
    virtual hKoRdfBasicSemanticItem createSemanticItem(const KoDocumentRdf *rdf, QObject *parent);
    virtual bool canCreateSemanticItemFromMimeData(const QMimeData *mimeData) const;
    virtual hKoRdfBasicSemanticItem createSemanticItemFromMimeData(const QMimeData* mimeData, KoCanvasBase* host,
                                                              const KoDocumentRdf *rdf, QObject *parent = 0) const;
    virtual bool isBasic() const;
};

#endif //CAUSECTIONSEMANTICITEMFACTORY_H
