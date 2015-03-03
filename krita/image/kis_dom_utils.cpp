/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dom_utils.h"

#include <QTransform>
#include <QDebug>

#include "kis_debug.h"

namespace KisDomUtils {

inline int stringToInt(const QString &str) {
    bool ok = false;
    int value = 0;

    QLocale c(QLocale::German);

    value = str.toInt(&ok);
    if (!ok) {
        value = c.toInt(str, &ok);
    }

    if (!ok) {
        qWarning() << "WARNING: KisDomUtils::stringToInt failed:" << ppVar(str);
        value = 0;
    }

    return value;
}

inline double stringToDouble(const QString &str) {
    bool ok = false;
    double value = 0;

    QLocale c(QLocale::German);

    /**
     * A special workaround to handle ','/'.' decimal point
     * in different locales. Added for backward compatibility,
     * because we used to save qreals directly using
     *
     * e.setAttribute("w", (qreal)value),
     *
     * which did local-aware conversion.
     */

    value = str.toDouble(&ok);
    if (!ok) {
        value = c.toDouble(str, &ok);
    }

    if (!ok) {
        qWarning() << "WARNING: KisDomUtils::stringToDouble failed:" << ppVar(str);
        value = 0;
    }

    return value;
}

void saveValue(QDomElement *parent, const QString &tag, const QSize &size)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "size");

    e.setAttribute("w", Private::numberToString(size.width()));
    e.setAttribute("h", Private::numberToString(size.height()));
}

void saveValue(QDomElement *parent, const QString &tag, const QRect &rc)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "rect");

    e.setAttribute("x", Private::numberToString(rc.x()));
    e.setAttribute("y", Private::numberToString(rc.y()));
    e.setAttribute("w", Private::numberToString(rc.width()));
    e.setAttribute("h", Private::numberToString(rc.height()));
}

void saveValue(QDomElement *parent, const QString &tag, const QPointF &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "pointf");

    e.setAttribute("x", Private::numberToString(pt.x()));
    e.setAttribute("y", Private::numberToString(pt.y()));
}

void saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "vector3d");

    e.setAttribute("x", Private::numberToString(pt.x()));
    e.setAttribute("y", Private::numberToString(pt.y()));
    e.setAttribute("z", Private::numberToString(pt.z()));
}

void saveValue(QDomElement *parent, const QString &tag, const QTransform &t)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "transform");

    e.setAttribute("m11", Private::numberToString(t.m11()));
    e.setAttribute("m12", Private::numberToString(t.m12()));
    e.setAttribute("m13", Private::numberToString(t.m13()));

    e.setAttribute("m21", Private::numberToString(t.m21()));
    e.setAttribute("m22", Private::numberToString(t.m22()));
    e.setAttribute("m23", Private::numberToString(t.m23()));

    e.setAttribute("m31", Private::numberToString(t.m31()));
    e.setAttribute("m32", Private::numberToString(t.m32()));
    e.setAttribute("m33", Private::numberToString(t.m33()));
}

bool findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages)
{
    QDomNodeList list = parent.elementsByTagName(tag);
    if (list.size() != 1 || !list.at(0).isElement()) {
        QString msg = i18n("Could not find \"%1\" XML tag in \"%2\"", tag, parent.tagName());
        if (errorMessages) {
            *errorMessages << msg;
        } else {
            qWarning() << msg;
        }

        return false;
    }

    *el = list.at(0).toElement();
    return true;
}

namespace Private {
    bool checkType(const QDomElement &e, const QString &expectedType)
    {
        QString type = e.attribute("type", "unknown-type");
        if (type != expectedType) {
            qWarning() << i18n("Error: incorrect type (%2) for value %1. Expected %3", e.tagName(), type, expectedType);
            return false;
        }

        return true;
    }
}

bool loadValue(const QDomElement &e, float *v)
{
    if (!Private::checkType(e, "value")) return false;
    *v = stringToDouble(e.attribute("value", "0"));
    return true;
}

bool loadValue(const QDomElement &e, double *v)
{
    if (!Private::checkType(e, "value")) return false;
    *v = stringToDouble(e.attribute("value", "0"));
    return true;
}

bool loadValue(const QDomElement &e, QSize *size)
{
    if (!Private::checkType(e, "size")) return false;

    size->setWidth(stringToInt(e.attribute("w", "0")));
    size->setHeight(stringToInt(e.attribute("h", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QRect *rc)
{
    if (!Private::checkType(e, "rect")) return false;

    rc->setX(stringToInt(e.attribute("x", "0")));
    rc->setY(stringToInt(e.attribute("y", "0")));
    rc->setWidth(stringToInt(e.attribute("w", "0")));
    rc->setHeight(stringToInt(e.attribute("h", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QPointF *pt)
{
    if (!Private::checkType(e, "pointf")) return false;

    pt->setX(stringToDouble(e.attribute("x", "0")));
    pt->setY(stringToDouble(e.attribute("y", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QVector3D *pt)
{
    if (!Private::checkType(e, "vector3d")) return false;

    pt->setX(stringToDouble(e.attribute("x", "0")));
    pt->setY(stringToDouble(e.attribute("y", "0")));
    pt->setZ(stringToDouble(e.attribute("z", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QTransform *t)
{
    if (!Private::checkType(e, "transform")) return false;

    qreal m11 = stringToDouble(e.attribute("m11", "1.0"));
    qreal m12 = stringToDouble(e.attribute("m12", "0.0"));
    qreal m13 = stringToDouble(e.attribute("m13", "0.0"));

    qreal m21 = stringToDouble(e.attribute("m21", "0.0"));
    qreal m22 = stringToDouble(e.attribute("m22", "1.0"));
    qreal m23 = stringToDouble(e.attribute("m23", "0.0"));

    qreal m31 = stringToDouble(e.attribute("m31", "0.0"));
    qreal m32 = stringToDouble(e.attribute("m32", "0.0"));
    qreal m33 = stringToDouble(e.attribute("m33", "1.0"));

    t->setMatrix(
        m11, m12, m13,
        m21, m22, m23,
        m31, m32, m33);

    return true;
}


}
