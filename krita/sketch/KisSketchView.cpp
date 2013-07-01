/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
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
#include "KisSketchView.h"

#include <QTimer>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QClipboard>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>

#include <kdebug.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoProgressUpdater.h>
#include <KoToolProxy.h>
#include <KoCanvasController.h>
#include <KoFilterManager.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoUnit.h>
#include <KoShapeController.h>
#include <KoDocumentResourceManager.h>
#include <KoCanvasResourceManager.h>
#include <KoShapeManager.h>

#include <kundo2stack.h>

#include "ProgressProxy.h"

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_doc2.h"
#include "kis_canvas2.h"
#include <kis_canvas_controller.h>
#include <kis_qpainter_canvas.h>
#include "kis_config.h"
#include "kis_view2.h"
#include "kis_image.h"
#include <kis_image_signal_router.h>
#include "kis_clipboard.h"
#include <input/kis_input_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_zoom_manager.h>
#include <kis_selection_manager.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_qpainter_canvas.h>
#include <kis_part2.h>
#include <kis_canvas_decoration.h>

#include "KisSketchPart.h"
#include "KisSelectionExtras.h"
#include "Settings.h"
#include "cpuid.h"
#include "DocumentManager.h"
#include "SketchDeclarativeView.h"

class KisSketchView::Private
{
public:
    Private( KisSketchView* qq)
        : q(qq)
        , doc(0)
        , view(0)
        , canvas(0)
        , canvasWidget(0)
        , selectionExtras(0)
        , undoAction(0)
        , redoAction(0)
    { }
    ~Private() {
        delete selectionExtras;
    }

    void imageUpdated(const QRect &updated);
    void documentOffsetMoved();
    void zoomChanged();
    void resetDocumentPosition();
    void removeNodeAsync(KisNodeSP removedNode);

    KisSketchView* q;

    KisDoc2* doc;
    KisView2* view;
    KisCanvas2* canvas;
    KUndo2Stack* undoStack;

    QWidget *canvasWidget;

    QString file;

    KisSelectionExtras *selectionExtras;

    QTimer *timer;

    QTimer *loadedTimer;
    QTimer *savedTimer;
    QAction* undoAction;
    QAction* redoAction;
};

KisSketchView::KisSketchView(QDeclarativeItem* parent)
    : QDeclarativeItem(parent)
    , d(new Private(this))
{
    // this is just an interaction overlay, the contents are painted on the sceneview background
    setFlag(QGraphicsItem::ItemHasNoContents, true);
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);

    grabGesture(Qt::PanGesture);
    //grabGesture(Qt::PinchGesture);

    KoZoomMode::setMinimumZoom(0.1);
    KoZoomMode::setMaximumZoom(16.0);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(resetDocumentPosition()));

    d->loadedTimer = new QTimer(this);
    d->loadedTimer->setSingleShot(true);
    d->loadedTimer->setInterval(100);
    connect(d->loadedTimer, SIGNAL(timeout()), SIGNAL(loadingFinished()));

    d->savedTimer = new QTimer(this);
    d->savedTimer->setSingleShot(true);
    d->savedTimer->setInterval(100);
    connect(d->savedTimer, SIGNAL(timeout()), SIGNAL(savingFinished()));

    connect(DocumentManager::instance(), SIGNAL(aboutToDeleteDocument()), SLOT(documentAboutToBeDeleted()));
    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(documentChanged()));
    connect(DocumentManager::instance()->progressProxy(), SIGNAL(valueChanged(int)), SIGNAL(progress(int)));
    connect(DocumentManager::instance(), SIGNAL(documentSaved()), d->savedTimer, SLOT(start()));

    if(DocumentManager::instance()->document())
        documentChanged();
}

KisSketchView::~KisSketchView()
{
    if(d->doc) {
        DocumentManager::instance()->closeDocument();
    }
    if (d->canvasWidget) {
        SketchDeclarativeView *v = qobject_cast<SketchDeclarativeView*>(scene()->views().at(0));
        if (v) {
            v->setCanvasWidget(0);
            v->setDrawCanvas(false);
        }
    }

    delete d;
}

QObject* KisSketchView::selectionManager() const
{
    if(!d->view)
        return 0;
    return d->view->selectionManager();
}

QObject* KisSketchView::selectionExtras() const
{
    if (!d->selectionExtras) {
        d->selectionExtras = new KisSelectionExtras(d->view);
    }
    return d->selectionExtras;
}

QObject* KisSketchView::doc() const
{
    return d->doc;
}

QObject* KisSketchView::view() const
{
    return d->view;
}

QString KisSketchView::file() const
{
    return d->file;
}

QString KisSketchView::fileTitle() const
{
    QFileInfo file(d->file);
    return file.fileName();
}

bool KisSketchView::isModified() const
{
    return d->doc->isModified();
}

void KisSketchView::setFile(const QString& file)
{
    if (!file.isEmpty() && file != d->file) {
        d->file = file;
        emit fileChanged();

        if(!file.startsWith("temp://")) {
            DocumentManager::instance()->openDocument(file);
        }
    }
}

void KisSketchView::componentComplete()
{
}

bool KisSketchView::canUndo() const
{
    if(d->undoAction)
        return d->undoAction->isEnabled();
    return false;
}

bool KisSketchView::canRedo() const
{
    if(d->redoAction)
        return d->redoAction->isEnabled();
    return false;
}

void KisSketchView::undo()
{
    d->undoAction->trigger();
}

void KisSketchView::redo()
{
    d->redoAction->trigger();
}

void KisSketchView::zoomIn()
{
    d->view->actionCollection()->action("zoom_in")->trigger();
}

void KisSketchView::zoomOut()
{
    d->view->actionCollection()->action("zoom_out")->trigger();
}

void KisSketchView::save()
{
    DocumentManager::instance()->save();
}

void KisSketchView::saveAs(const QString& fileName, const QString& mimeType)
{
    DocumentManager::instance()->saveAs(fileName, mimeType);
}

void KisSketchView::documentAboutToBeDeleted()
{
    if(d->undoAction)
        d->undoAction->disconnect(this);

    if(d->redoAction)
        d->redoAction->disconnect(this);

    KisView2 *oldView = d->view;
    disconnect(d->view, SIGNAL(floatingMessageRequested(QString,QString)), this, SIGNAL(floatingMessageRequested(QString,QString)));
    d->view = 0;
    emit viewChanged();

    delete oldView;

    d->canvas = 0;
    d->canvasWidget = 0;
}

void KisSketchView::documentChanged()
{
    d->doc = DocumentManager::instance()->document();

    connect(d->doc, SIGNAL(modified(bool)), SIGNAL(modifiedChanged()));

    d->view = qobject_cast<KisView2*>(DocumentManager::instance()->part()->createView(QApplication::activeWindow()));
    connect(d->view, SIGNAL(floatingMessageRequested(QString,QString)), this, SIGNAL(floatingMessageRequested(QString,QString)));
    emit viewChanged();

    d->view->canvasControllerWidget()->setGeometry(x(), y(), width(), height());
    d->view->hide();
    d->canvas = d->view->canvasBase();

    d->undoStack = d->doc->undoStack();
    d->undoAction = d->view->actionCollection()->action("edit_undo");
    connect(d->undoAction, SIGNAL(changed()), this, SIGNAL(canUndoChanged()));

    d->redoAction = d->view->actionCollection()->action("edit_redo");
    connect(d->redoAction, SIGNAL(changed()), this, SIGNAL(canRedoChanged()));

    KoToolManager::instance()->switchToolRequested( "KritaShape/KisToolBrush" );

    d->canvasWidget = d->canvas->canvasWidget();

    connect(d->doc->image(), SIGNAL(sigImageUpdated(QRect)), SLOT(imageUpdated(QRect)));
    connect(d->view->canvasControllerWidget()->proxyObject, SIGNAL(moveDocumentOffset(QPoint)), SLOT(documentOffsetMoved()));
    connect(d->view->zoomController(), SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)), SLOT(zoomChanged()));
    connect(d->canvas, SIGNAL(updateCanvasRequested(QRect)), SLOT(imageUpdated(QRect)));
    connect(d->doc->image()->signalRouter(), SIGNAL(sigRemoveNodeAsync(KisNodeSP)), SLOT(removeNodeAsync(KisNodeSP)));

    SketchDeclarativeView *v = qobject_cast<SketchDeclarativeView*>(scene()->views().at(0));
    if (v) {
        v->setCanvasWidget(d->canvasWidget);
        v->setDrawCanvas(true);
    }

    d->imageUpdated(d->canvas->image()->bounds());

    static_cast<KoZoomHandler*>(d->canvas->viewConverter())->setResolution(d->doc->image()->xRes(), d->doc->image()->yRes());
    d->view->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);
    d->view->canvasControllerWidget()->setScrollBarValue(QPoint(0, 0));
    d->view->canvasControllerWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->view->canvasControllerWidget()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    geometryChanged(QRectF(x(), y(), width(), height()), QRectF());

    d->loadedTimer->start(100);

    d->view->actionCollection()->action("zoom_to_100pct")->trigger();
    d->resetDocumentPosition();
}

bool KisSketchView::sceneEvent(QEvent* event)
{
    if (d->canvas) {
        switch(event->type()) {
        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseButtonPress, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseMove, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            update();
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseButtonRelease, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneWheel: {
            QGraphicsSceneWheelEvent *gswevent = static_cast<QGraphicsSceneWheelEvent*>(event);
            QWheelEvent wevent(gswevent->pos().toPoint(), gswevent->delta(), gswevent->buttons(), gswevent->modifiers(), gswevent->orientation());
            QApplication::sendEvent(d->canvasWidget, &wevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::TouchBegin: {
            QApplication::sendEvent(d->canvasWidget, event);
            event->accept();
            emit interactionStarted();
            return true;
        }
        default:
            if(QApplication::sendEvent(d->canvasWidget, event)) {
            emit interactionStarted();
                return true;
        }
        }
    }
    return QDeclarativeItem::sceneEvent(event);
}

void KisSketchView::geometryChanged(const QRectF& newGeometry, const QRectF& /*oldGeometry*/)
{
    if (d->canvasWidget && !newGeometry.isEmpty()) {
        d->view->resize(newGeometry.toRect().size());
        d->timer->start(100);
    }
}

void KisSketchView::Private::imageUpdated(const QRect &updated)
{
    if (q->scene()) {
        q->scene()->views().at(0)->update(updated);
        q->scene()->invalidate( 0, 0, q->width(), q->height() );
    }
}

void KisSketchView::Private::documentOffsetMoved()
{
    if (q->scene()) {
        q->scene()->views().at(0)->update();
        q->scene()->invalidate( 0, 0, q->width(), q->height() );
    }
}

void KisSketchView::Private::resetDocumentPosition()
{
    view->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);

    QPoint pos;
    QScrollBar *sb = view->canvasControllerWidget()->horizontalScrollBar();

    pos.rx() = sb->minimum() + (sb->maximum() - sb->minimum()) / 2;

    sb = view->canvasControllerWidget()->verticalScrollBar();
    pos.ry() = sb->minimum() + (sb->maximum() - sb->minimum()) / 2;

    view->canvasControllerWidget()->setScrollBarValue(pos);
}


void KisSketchView::Private::removeNodeAsync(KisNodeSP removedNode)
{
    if(removedNode) {
        imageUpdated(removedNode->extent());
    }
}

void KisSketchView::Private::zoomChanged()
{
    if (q->scene()) {
        q->scene()->views().at(0)->update();
        q->scene()->invalidate( 0, 0, q->width(), q->height() );
    }
}

#include "KisSketchView.moc"
