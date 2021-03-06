/* This file is part of the KDE project
   Copyright (C) 2003-2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
   Contact: Manikandaprasad Chandrasekar <manikandaprasad.chandrasekar@nokia.com>
   Copyright (c) 2010 Carlos Licea <carlos@kdab.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "ImportUtils.h"

#include <NumberFormatParser.h>

namespace XlsUtils {

QString removeEscaped(const QString &text, bool removeOnlyEscapeChar)
{
    QString s(text);
    int pos = 0;
    while (true) {
        pos = s.indexOf('\\', pos);
        if (pos < 0)
            break;
        if (removeOnlyEscapeChar) {
            s = s.left(pos) + s.mid(pos + 1);
            pos++;
        } else {
            s = s.left(pos) + s.mid(pos + 2);
        }
    }
    return s;
}

QString extractLocale(QString &time)
{
    QString locale;
    if (time.startsWith("[$-")) {
        int pos = time.indexOf(']');
        if (pos > 3) {
            locale = time.mid(3, pos - 3);
            time.remove(0, pos + 1);
            pos = time.lastIndexOf(';');
            if (pos >= 0) {
                time = time.left(pos);
            }
        }
    }
    return locale;
}

bool isPercentageFormat(const QString& valueFormat)
{
    int length = valueFormat.length();
    if (length < 1) return false;
    return valueFormat[length - 1] == QChar('%');
}

bool isTimeFormat(const QString& valueFormat)
{
    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);
    vf = removeEscaped(vf);

    // if there is still a time formatting picture item that was not escaped
    // and therefore removed above, then we have a time format here.
    QRegExp ex("(h|H|m|s)");
    return ex.indexIn(vf) >= 0;
}

bool isFractionFormat(const QString& valueFormat)
{
    QRegExp ex("^#[?]+/[0-9?]+$");
    QString vf = removeEscaped(valueFormat);
    return ex.indexIn(vf) >= 0;
}

bool isDateFormat(const QString& valueFormat)
{
    return NumberFormatParser::isDateFormat(valueFormat);
}



CellFormatKey::CellFormatKey(const Swinder::Format* format, const QString& formula)
    : format(format), isGeneral(format->valueFormat() == "General"), decimalCount(-1)
{
    if (!isGeneral) {
        if (formula.startsWith(QLatin1String("msoxl:="))) { // special cases
            QRegExp roundRegExp( "^msoxl:=ROUND[A-Z]*\\(.*;[\\s]*([0-9]+)[\\s]*\\)$" );
            if (roundRegExp.indexIn(formula) >= 0) {
                bool ok = false;
                int decimals = roundRegExp.cap(1).trimmed().toInt(&ok);
                if (ok) {
                    decimalCount = decimals;
                }
            }
        } else if (formula.startsWith(QLatin1String("msoxl:=RAND("))) {
            decimalCount = 9;
        }
    }
}

bool CellFormatKey::operator==(const CellFormatKey& b) const {
    return format == b.format && isGeneral == b.isGeneral && decimalCount == b.decimalCount;
}

}
