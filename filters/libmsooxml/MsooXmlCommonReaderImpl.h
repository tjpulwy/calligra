/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef MSOOXMLCOMMONREADER_IMPL_H
#define MSOOXMLCOMMONREADER_IMPL_H

void MSOOXML_CURRENT_CLASS::initInternal()
{
    m_insideHdr = false;
    m_insideFtr = false;
    m_hasPosOffsetH = false;
    m_hasPosOffsetV = false;
    m_posOffsetH = 0;
    m_posOffsetV = 0;
    m_currentTextStyleProperties = 0;
}

void MSOOXML_CURRENT_CLASS::doneInternal()
{
    delete m_currentTextStyleProperties;
}

#undef CURRENT_EL
#define CURRENT_EL t
//! t handler (Text)
/*! ECMA-376, 17.3.3.31, p.389.
 Parent elements:
 - [done] r (§22.1.2.87) - Shared ML
 - [done] r (§17.3.2.25)
 No child elements.
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_t()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        if (isCharacters()) {
            body->addTextSpan(text().toString());
#ifdef PPTXXMLSLIDEREADER_CPP
            d->textBoxHasContent = true;
#endif
        }
//! @todo add ELSE_WRONG_FORMAT
        BREAK_IF_END_OF(CURRENT_EL)
    }
//kDebug() << "{1}";
    READ_EPILOGUE
}
#undef MSOOXML_CURRENT_NS
#define MSOOXML_CURRENT_NS "mc"

#undef CURRENT_EL
#define CURRENT_EL AlternateContent
//! Alternate content handler
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_AlternateContent()
{
    READ_PROLOGUE

    m_choiceAccepted = false;

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL)
        if (isStartElement()) {
            TRY_READ_IF(Choice)
            else if (!m_choiceAccepted && qualifiedName() == "mc:Fallback") {
                TRY_READ(Fallback)
            }
            SKIP_UNKNOWN
        }
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL Choice
//! Choice handler
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_Choice()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(Requires)

    if (Requires != "v") {
        skipCurrentElement();
        READ_EPILOGUE
    }
    m_choiceAccepted = true;

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL)
        if (isStartElement()) {
#ifdef PPTXXMLSLIDEREADER_CPP
            TRY_READ_IF_NS(p, oleObj)
#endif
        }
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL Fallback
//! Fallback handler
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_Fallback()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL)
        if (isStartElement()) {
#ifdef DOCXXMLDOCREADER_H
            TRY_READ_IF_NS(w, pict)
#endif
        }
    }

    READ_EPILOGUE
}

#endif
