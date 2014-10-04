/* This file is part of the KDE project

   Copyright (C) 2013-2014 Inge Wallin       <inge@lysator.liu.se>

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
   Boston, MA 02110-1301, USA.
*/


// Own
#include "OdfChartReaderBackend.h"

// Calligra
#include <KoXmlReader.h>

// Odftraverse library
#include "OdfParser.h"


// ================================================================
//             class OdfChartReaderBackend::Private


class OdfChartReaderBackend::Private
{
 public:
    Private();
    ~Private();

    bool dummy;                 // We don't have any actual content in
                                // this class yet but it's still
                                // needed for forward binary compatibility.
};

OdfChartReaderBackend::Private::Private()
{
}

OdfChartReaderBackend::Private::~Private()
{
}


// ================================================================
//                 class OdfChartReaderBackend


OdfChartReaderBackend::OdfChartReaderBackend()
    : d(new OdfChartReaderBackend::Private)
{
}

OdfChartReaderBackend::~OdfChartReaderBackend()
{
    delete d;
}


// ----------------------------------------------------------------


void OdfChartReaderBackend::elementOfficeChart(KoXmlStreamReader &reader,
					       OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}

void OdfChartReaderBackend::elementChartChart(KoXmlStreamReader &reader,
					      OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}

void OdfChartReaderBackend::elementChartFooter(KoXmlStreamReader &reader,
					       OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}

void OdfChartReaderBackend::elementChartSubtitle(KoXmlStreamReader &reader,
						 OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}

void OdfChartReaderBackend::elementChartTitle(KoXmlStreamReader &reader,
					      OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}

void OdfChartReaderBackend::elementChartLegend(KoXmlStreamReader &reader,
					       OdfReaderContext *context)
{
    Q_UNUSED(reader);
    Q_UNUSED(context);
}