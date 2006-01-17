/*
 *  kis_layerbox.h - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_LAYERBOX_H
#define KIS_LAYERBOX_H

#include <qframe.h>
#include <kdebug.h>
#include <qtimer.h>

#include "kis_types.h"
#include "kis_colorspace.h"

class WdgLayerBox;
class QButton;
class QPainter;
class QWidget;
class KIconLoader;
class KPopupMenu;
class KoDocumentEntry;
class KisCompositeOp;
class KisLayerList;
class LayerItem;

// XXX: Add layer locking, previews
class KisLayerBox : public QFrame {
        typedef QFrame super;
        Q_OBJECT

public:
    KisLayerBox(QWidget *parent = 0, const char *name = 0);
    virtual ~KisLayerBox();

    void clear();
    void setUpdatesAndSignalsEnabled(bool enable);
    void setImage(KisImageSP image);

public slots:
    // connect to KisImage signals
    void slotLayerActivated(KisLayerSP layer);
    void slotLayerAdded(KisLayerSP layer);
    void slotLayerRemoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAboveThis);
    void slotLayerMoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAboveThis);
    void slotLayerPropertiesChanged(KisLayerSP layer);
    void slotLayersChanged(KisGroupLayerSP rootLayer);
    void slotImageUpdated();
    void slotNonActiveLayersUpdated();

    void slotSetCompositeOp(const KisCompositeOp& compositeOp);
    void slotSetOpacity(int opacity);
    void slotSetColorSpace(const KisColorSpace * colorSpace);

signals:
    void sigRequestLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestGroupLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry);
    void sigRequestLayerProperties(KisLayerSP layer);

    void sigOpacityChanged(int opacity);
    void sigItemComposite(const KisCompositeOp&);

private:
    enum LayerTypes { PAINT_LAYER, GROUP_LAYER, ADJUSTMENT_LAYER, OBJECT_LAYER };

private slots:
    // connect to LayerList signals
    void slotLayerActivated(LayerItem* layer);
    void slotLayerDisplayNameChanged(LayerItem* layer, const QString& displayName);
    void slotLayerPropertyChanged(LayerItem* layer, const QString& name, bool on);
    void slotLayerMoved(LayerItem* layer, LayerItem* parent, LayerItem* after);
    void slotRequestNewLayer(LayerItem* parent, LayerItem* after);
    void slotRequestNewFolder(LayerItem* parent, LayerItem* after);
    void slotRequestNewAdjustmentLayer(LayerItem* parent, LayerItem* after);
    void slotRequestNewObjectLayer(LayerItem* parent, LayerItem* item, const KoDocumentEntry& entry);
    void slotRequestRemoveLayer(LayerItem* layer);
    void slotRequestLayerProperties(LayerItem* layer);

    void slotAboutToShow();
    void slotAddMenuActivated(int type = OBJECT_LAYER);
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotPropertiesClicked();

    void updateThumbnails();

private:

    void updateUI();
    QPixmap loadPixmap(const QString& filename, const KIconLoader& il, int size);
    KisLayerList* list() const;

    KPopupMenu *m_newLayerMenu;
    KoPartSelectAction *m_partLayerAction;
    KisImageSP m_image;
    QValueList<int> m_modified;
    QTimer m_thumbnailerTimer;
    WdgLayerBox *m_lst;
};

#endif // KIS_LAYERBOX_H

