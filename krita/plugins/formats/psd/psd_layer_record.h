/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef PSD_LAYER_RECORD_H
#define PSD_LAYER_RECORD_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QBitArray>

#include <kis_types.h>
#include <kis_paint_device.h>


#include "psd.h"
#include "psd_header.h"

#include "compression.h"

class QIODevice;

struct ChannelInfo {
    qint16 channelId; // 0 red, 1 green, 2 blue, -1 transparency, -2 user-supplied layer mask
    Compression::CompressionType compressionType;
    quint64 channelDataStart;
    quint64 channelDataLength;
    QVector<quint32> rleRowLengths;
};

class PSDLayerRecord
{
public:

    PSDLayerRecord(const PSDHeader &header);

    ~PSDLayerRecord()
    {
        qDeleteAll(channelInfoRecords);
    }

    bool read(QIODevice* io);
    bool write(QIODevice* io);
    bool valid();

    QString error;

    quint32 top;
    quint32 left;
    quint32 bottom;
    quint32 right;

    quint16 nChannels;

    QVector<ChannelInfo*> channelInfoRecords;

    QString blendModeKey;

    quint8 opacity;
    quint8 clipping;
    bool   transparencyProtected;
    bool   visible;
    bool   irrelevant;

    struct LayerMaskData {
        quint32 top;
        quint32 left;
        quint32 bottom;
        quint32 right;
        quint8 defaultColor;
        bool positionedRelativeToLayer;
        bool disabled;
        bool invertLayerMaskWhenBlending;
    };

    LayerMaskData layerMask;

    struct LayerBlendingRanges {

        QByteArray data;

        quint8 blackValues[2];
        quint8 whiteValues[2];
        quint32 compositeGrayBlendDestinationRange;
        QVector<QPair<quint32, quint32> > sourceDestinationRanges;
    };

    LayerBlendingRanges blendingRanges;

    QString layerName; // pascal, not unicode!

    struct LayerInfoBlock {
        QByteArray data;
    };

    QMap<QString, LayerInfoBlock*> infoBlocks;

    bool readChannels(QIODevice* io, KisPaintDeviceSP device);

private:

    bool doRGB(KisPaintDeviceSP dev ,QIODevice *io);
    bool doCMYK(KisPaintDeviceSP dev ,QIODevice *io);
    bool doLAB(KisPaintDeviceSP dev ,QIODevice *io);

    const PSDHeader m_header;
    quint16 m_compression;
    quint64 m_channelDataLength;
    quint32 m_channelSize;

    QVector<ChannelInfo> m_channelInfoRecords;
    QVector<int> m_channelOffsets; // this doesn't need to be global
};

QDebug operator<<(QDebug dbg, const PSDLayerRecord& layer);
QDebug operator<<(QDebug dbg, const ChannelInfo& layer);

#endif // PSD_LAYER_RECORD_H
