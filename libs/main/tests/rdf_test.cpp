/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
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
#include "rdf_test.h"

#include <QtTest/QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>

#include <KoRdfSemanticItem.h>
#include <KoDocumentRdf.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextInlineRdf.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );

class TestSemanticItem : public KoRdfSemanticItem
{
public:
    const QString PREDBASE;

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf = 0)
        : KoRdfSemanticItem(const_cast<KoDocumentRdf*>(rdf), parent)
        , PREDBASE("http://calligra-suite.org/testrdf/")
    {
        updateTriple(m_uri, QUuid::createUuid().toString(), PREDBASE + "object");
        setRdfType(PREDBASE + "testitem");
    }

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
        : KoRdfSemanticItem(const_cast<KoDocumentRdf*>(rdf), it, parent)
    {
        m_uri = it.binding("object").toString();
        m_name = it.binding("name").toString();
    }

    virtual ~TestSemanticItem()
    {
    }

    virtual QWidget *createEditor(QWidget */*parent*/)
    {
        return 0;
    }

    virtual void updateFromEditorData()
    {
    }

    virtual void exportToFile(const QString &/*fileName*/ = QString()) const
    {
    }

    virtual void importFromData(const QByteArray &/*ba*/, KoDocumentRdf */*rdf*/ = 0, KoCanvasBase */*host*/ = 0)
    {
    }

    void setName(const QString &name)
    {
        updateTriple(m_name, name, PREDBASE + "name");
        if (documentRdf()) {
            const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(this);
        }
    }

    virtual QString name() const
    {
        return m_name;
    }

    virtual QString className() const
    {
        return "TestSemanticItem";
    }

    virtual QList<KoSemanticStylesheet*> stylesheets() const
    {
        QList<KoSemanticStylesheet*> sheets;
        return sheets;
    }

    Soprano::Node linkingSubject() const
    {
        return Soprano::Node::createResourceNode(m_uri);
    }

    static QList<TestSemanticItem*> allObjects(KoDocumentRdf* rdf, Soprano::Model *model = 0)
    {
        QList<TestSemanticItem*> result;
        const Soprano::Model* m = model ? model : rdf->model();

        QString query =
                "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                "prefix testrdf: <http://calligra-suite.org/testrdf/> \n"
                "select distinct ?name ?object \n"
                "where { \n"
                "    ?element rdf:type testrdf:testitem . \n"
                "    ?element testrdf:name ?name . \n"
                "    ?element testrdf:object ?object \n"
                "}\n"
                "    order by  DESC(?name) \n ";

        Soprano::QueryResultIterator it = m->executeQuery(query,
                                                         Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            TestSemanticItem *item = new TestSemanticItem(rdf, rdf, it);
            result << item;
        }
        return result;
    }

private:
    QString m_name;
    QString m_uri;
};

QString RdfTest::insertSemItem(KoTextEditor &editor,
                               KoDocumentRdf &rdfDoc,
                               QObject &parent,
                               const QString name)
{
    editor.insertTable(5,10);
    QTextTable *table = editor.cursor()->currentTable();
    table->setObjectName(name);

    TestSemanticItem *testItem = new TestSemanticItem(&parent, &rdfDoc);
    testItem->setName(table->objectName());

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);

    KoTextInlineRdf *inlineRdf(new KoTextInlineRdf(editor.document(), startmark));
    QString newId = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newId);

    startmark->setName(newId);
    startmark->setInlineRdf(inlineRdf);

    editor.setPosition(table->firstPosition());
    editor.movePosition(QTextCursor::PreviousCharacter);
    editor.insertInlineObject(startmark);

    Soprano::Statement st(
          testItem->linkingSubject(), // subject
          Soprano::Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")), // predicate
          Soprano::Node::createLiteralNode(newId), // object
          rdfDoc.manifestRdfNode()); // manifest datastore

    const_cast<Soprano::Model*>(rdfDoc.model())->addStatement(st);

    rdfDoc.rememberNewInlineRdfObject(inlineRdf);

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setName(newId);
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);

    editor.setPosition(table->lastPosition());
    editor.movePosition(QTextCursor::NextCharacter);
    editor.insertInlineObject(endmark);

    return newId;

}

void RdfTest::testCreateMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");

    // enter some more lorem
    editor.insertText(lorem);

    // verify that the markers are there
    QPair<int,int> position = rdfDoc.findExtent(newId);
    Q_ASSERT(position.first == 444);
    Q_ASSERT(position.second == 496);

    editor.setPosition(position.first + 1);

    QPair<int,int> position2 = rdfDoc.findExtent(&editor);
    Q_ASSERT(position == position2);

    QPair<int,int> position3 = rdfDoc.findExtent(*editor.cursor());
    Q_ASSERT(position2 == position3);

    // verify that we don't find markers where there aren't any
    editor.setPosition(10);
    QPair<int,int> position4 = rdfDoc.findExtent(&editor);
    Q_ASSERT(position4.first == 0);
    Q_ASSERT(position4.second == 0);

    QPair<int,int> position5 = rdfDoc.findExtent(*editor.cursor());
    Q_ASSERT(position5.first == 0);
    Q_ASSERT(position5.second == 0);

    // go back to the semitem
    editor.setPosition(position.first + 1);
    QCOMPARE(newId, rdfDoc.findXmlId(&editor));
    QCOMPARE(newId, rdfDoc.findXmlId(*editor.cursor()));
}

void RdfTest::testFindMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QStringList idList;
    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");
    idList << newId;

    editor.setPosition(0);

    // now use soprano to find the tables
    QList<TestSemanticItem*> semItems = TestSemanticItem::allObjects(&rdfDoc);
    Q_ASSERT(semItems.length() == 1);

    foreach(TestSemanticItem *semItem, semItems) {
        QStringList xmlidlist = semItem->xmlIdList();

        Q_ASSERT(xmlidlist.length() == 1);
        foreach(const QString xmlid, xmlidlist) {
            Q_ASSERT(idList.contains(xmlid));
            QPair<int, int> position = rdfDoc.findExtent(xmlid);
            Q_ASSERT(position.first == 444);
            Q_ASSERT(position.second == 496);
            editor.setPosition(position.first + 1);
            QTextTable *table = editor.cursor()->currentTable();
            Q_ASSERT(table);
            Q_ASSERT(table->objectName() == QString("test item1"));
        }
    }

    // add an extra semitem
    editor.movePosition(QTextCursor::End);
    QString newId2 = insertSemItem(editor, rdfDoc, parent, "test item2");
        idList << newId2;

    semItems = TestSemanticItem::allObjects(&rdfDoc);
    Q_ASSERT(semItems.length() == 2);

    foreach(TestSemanticItem *semItem, semItems) {
        QStringList xmlidlist = semItem->xmlIdList();
        Q_ASSERT(xmlidlist.length() == 1);
        foreach(const QString xmlid, xmlidlist) {
            Q_ASSERT(idList.contains(xmlid));
            QPair<int, int> position = rdfDoc.findExtent(xmlid);
            editor.setPosition(position.first + 1);
            QTextTable *table = editor.cursor()->currentTable();
            Q_ASSERT(table);
        }
    }
}

void RdfTest::testEditAndFindMarkers()
{
}

void RdfTest::testRemoveMarkers()
{
}

QTEST_MAIN(RdfTest)

#include "rdf_test.moc"
