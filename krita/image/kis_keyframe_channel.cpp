/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_keyframe_channel.h"
#include "KoID.h"
#include "kis_node.h"
#include "kis_time_range.h"

#include <QMap>

const KoID KisKeyframeChannel::Content = KoID("content", i18n("Content"));

struct KisKeyframeChannel::Private
{
    QMap<int, KisKeyframe*> keys;
    KisNodeWSP node;
    KoID id;
};

KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisNodeWSP node)
    : m_d(new Private)
{
    m_d->id = id;
    m_d->node = node;
}

KisKeyframeChannel::~KisKeyframeChannel()
{}

QString KisKeyframeChannel::id() const
{
    return m_d->id.id();
}

QString KisKeyframeChannel::name() const
{
    return m_d->id.name();
}

KisNodeWSP KisKeyframeChannel::node() const
{
    return m_d->node;
}

KisKeyframe *KisKeyframeChannel::addKeyframe(int time)
{
    return insertKeyframe(time, 0);
}

bool KisKeyframeChannel::deleteKeyframe(KisKeyframe *keyframe)
{
    if (canDeleteKeyframe(keyframe)) {
        QRect rect = affectedRect(keyframe);
        KisTimeRange range = affectedFrames(keyframe->time());

        emit sigKeyframeAboutToBeRemoved(keyframe);
        m_d->keys.remove(keyframe->time());
        destroyKeyframe(keyframe);
        emit sigKeyframeRemoved(keyframe);

        requestUpdate(range, rect);

        delete keyframe;

        return true;
    }

    return false;
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframe *keyframe, int newTime)
{
    KisKeyframe *other = keyframeAt(newTime);
    if (other) return false;

    KisTimeRange rangeSrc = affectedFrames(keyframe->time());
    QRect rectSrc = affectedRect(keyframe);

    emit sigKeyframeAboutToBeMoved(keyframe, newTime);

    m_d->keys.remove(keyframe->time());
    int oldTime = keyframe->time();
    keyframe->setTime(newTime);
    m_d->keys.insert(newTime, keyframe);

    emit sigKeyframeMoved(keyframe, oldTime);

    KisTimeRange rangeDst = affectedFrames(keyframe->time());
    QRect rectDst = affectedRect(keyframe);

    requestUpdate(rangeSrc, rectSrc);
    requestUpdate(rangeDst, rectDst);

    return true;
}

KisKeyframe *KisKeyframeChannel::copyKeyframe(const KisKeyframe *keyframe, int newTime)
{
    return insertKeyframe(newTime, keyframe);
}

KisKeyframe *KisKeyframeChannel::keyframeAt(int time)
{
    QMap<int, KisKeyframe*>::iterator i = m_d->keys.find(time);
    if (i != m_d->keys.end()) {
        return i.value();
    }

    return 0;
}

KisKeyframe *KisKeyframeChannel::activeKeyframeAt(int time) const
{
    return activeKeyIterator(time).value();
}

KisKeyframe *KisKeyframeChannel::nextKeyframeAfter(int time) const
{
    const QMap<int, KisKeyframe *> keys = constKeys();
    QMap<int, KisKeyframe*>::const_iterator i = keys.upperBound(time);

    if (i == keys.end()) return 0;

    return i.value();
}

KisTimeRange KisKeyframeChannel::affectedFrames(int time) const
{
    return identicalFrames(time);
}

KisTimeRange KisKeyframeChannel::identicalFrames(int time) const
{
    QMap<int, KisKeyframe*>::const_iterator active = activeKeyIterator(time);
    QMap<int, KisKeyframe*>::const_iterator next = active + 1;

    int from;

    if (active == m_d->keys.begin()) {
        // First key affects even the frames before it
        from = 0;
    } else {
        from = active.key();
    }

    if (next == m_d->keys.end()) {
        return KisTimeRange::infinite(from);
    } else {
        return KisTimeRange::fromTime(from, next.key() - 1);
    }
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
}

QList<KisKeyframe*> KisKeyframeChannel::keyframes() const
{
    return m_d->keys.values();
}

QDomElement KisKeyframeChannel::toXML(QDomDocument doc, const QString &layerFilename)
{
    QDomElement channelElement = doc.createElement("channel");

    channelElement.setAttribute("name", id());

    foreach (KisKeyframe *keyframe, m_d->keys.values()) {
        QDomElement keyframeElement = doc.createElement("keyframe");
        keyframeElement.setAttribute("time", keyframe->time());

        saveKeyframe(keyframe, keyframeElement, layerFilename);

        channelElement.appendChild(keyframeElement);
    }

    return channelElement;
}

void KisKeyframeChannel::loadXML(const QDomElement &channelNode)
{
    // Make sure we're starting from scratch
    KisKeyframe *keyframe;
    foreach (keyframe, m_d->keys) {
        destroyKeyframe(keyframe);
    }
    m_d->keys.clear();

    for (QDomElement keyframeNode = channelNode.firstChildElement(); !keyframeNode.isNull(); keyframeNode = keyframeNode.nextSiblingElement()) {
        if (keyframeNode.nodeName().toUpper() != "KEYFRAME") continue;

        KisKeyframe *keyframe = loadKeyframe(keyframeNode);

        m_d->keys.insert(keyframe->time(), keyframe);
    }
}

QMap<int, KisKeyframe *>& KisKeyframeChannel::keys()
{
    return m_d->keys;
}

const QMap<int, KisKeyframe *>& KisKeyframeChannel::constKeys() const
{
    return m_d->keys;
}

KisKeyframe * KisKeyframeChannel::insertKeyframe(int time, const KisKeyframe *copySrc)
{
    KisKeyframe *keyframe = keyframeAt(time);
    if (keyframe) return keyframe;

    keyframe = createKeyframe(time, copySrc);

    emit sigKeyframeAboutToBeAdded(keyframe);
    m_d->keys.insert(time, keyframe);
    emit sigKeyframeAdded(keyframe);

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(time);
    requestUpdate(range, rect);

    return keyframe;
}

QMap<int, KisKeyframe*>::const_iterator KisKeyframeChannel::activeKeyIterator(int time) const
{
    QMap<int, KisKeyframe*>::const_iterator i = m_d->keys.upperBound(time);

    if (i != m_d->keys.begin()) i--;

    return i;
}

void KisKeyframeChannel::destroyKeyframe(KisKeyframe *key)
{
     Q_UNUSED(key);
}

void KisKeyframeChannel::requestUpdate(const KisTimeRange &range, const QRect &rect)
{
    if (m_d->node) {
        m_d->node->invalidateFrames(range, rect);
    }
}

#include "kis_keyframe_channel.moc"