// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qstring.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qfile.h>

#include <klocale.h>
#include <kdebug.h>

#include <KWEFUtil.h>
#include <KWEFBaseWorker.h>

#include "ExportFilter.h"
#include "ExportCss.h"

QString HtmlCssWorker::escapeCssIdentifier(const QString& strText) const
{
    // Reference: section 4.1.3 of the CSS2 recommendation
    // NOTE: when we need to escape, we choose the numerical CSS escape as it is encoding neutral.

    QString strReturn;

    for (uint i=0; i<strText.length(); i++)
    {
        const QChar ch=strText[i];
        if (((ch>='a') && (ch<='z'))
            || ((ch>='A') && (ch<='Z')))
        {
            strReturn+=ch;
        }
        else if (((ch>='0') && (ch<='9'))
            || (ch=='-'))
        {
            if (!i)
            {
                // A digit or a hyphen is not allowed as first character of an identifier
                //  therefore we must escape it
                strReturn+='\\'; // start escape
                strReturn+=QString::number(ch.unicode(),16);
                strReturn+=' '; // end escape (the space is not part of the following text!)
            }
            else
            {
                strReturn+=ch;
            }
        }
        else if ((ch>=QChar(161)) && (getCodec()->canEncode(ch)))
        {
            // Any Unicode character greater or egual to 161 is allowed too, even at start.
            // Except if the encoding cannot write the character
            strReturn+=ch;
        }
        else
        {
            // We have a non-acceptable character, so escape it!
            strReturn+='\\'; // start escape
            strReturn+=QString::number(ch.unicode(),16);
            strReturn+=' '; // end escape (the space is not part of the following text!)
        }
    }
    return strReturn;
}

QString HtmlCssWorker::textFormatToCss(const TextFormatting& formatOrigin,
    const TextFormatting& formatData, const bool force) const
{
    // TODO: as this method comes from the AbiWord filter,
    // TODO:   verify that it is working for HTML

    // TODO: rename variable formatData
    QString strElement; // TODO: rename this variable

    // Font name
    QString fontName = formatData.fontName;
    if (!fontName.isEmpty()
        && (force || (formatOrigin.fontName!=formatData.fontName)))
    {
        strElement+="font-family: ";
        strElement+=fontName; // TODO: add alternative font names
        strElement+="; ";
    }

    if (force || (formatOrigin.italic!=formatData.italic))
    {
        // Font style
        strElement+="font-style: ";
        if ( formatData.italic )
        {
            strElement+="italic";
        }
        else
        {
            strElement+="normal";
        }
        strElement+="; ";
    }

    if (force || ((formatOrigin.weight>=75)!=(formatData.weight>=75)))
    {
        strElement+="font-weight: ";
        if ( formatData.weight >= 75 )
        {
            strElement+="bold";
        }
        else
        {
            strElement+="normal";
        }
        strElement+="; ";
    }

    if (force || (formatOrigin.fontSize!=formatData.fontSize))
    {
        const int size=formatData.fontSize;
        if (size>0)
        {
            // We use absolute font sizes.
            strElement+="font-size: ";
            strElement+=QString::number(size,10);
            strElement+="pt; ";
        }
    }

    if (force || (formatOrigin.fgColor!=formatData.fgColor))
    {
        if ( formatData.fgColor.isValid() )
        {
            // Give colour
            strElement+="color: ";
            strElement+=formatData.fgColor.name();
            strElement+="; ";
        }
    }

    if (force || (formatOrigin.bgColor!=formatData.bgColor))
    {
        if ( formatData.bgColor.isValid() )
        {
            // Give background colour
            strElement+="bgcolor: ";
            strElement+=formatData.bgColor.name();
            strElement+="; ";
        }
    }

    if (force || (formatOrigin.underline!=formatData.underline)
        || (formatOrigin.strikeout!=formatData.strikeout))
    {
        strElement+="text-decoration: ";
        if ( formatData.underline )
        {
            strElement+="underline";
        }
        else if ( formatData.strikeout )
        {
            strElement+="line-through";
        }
        else
        {
            strElement+="none";
        }
        strElement+="; ";
    }

    // TODO: As this is the last property, do not put a semi-colon

    return strElement;
}

QString HtmlCssWorker::getStartOfListOpeningTag(const CounterData::Style typeList, bool& ordered)
{
    QString strResult;
    switch (typeList)
    {
    case CounterData::STYLE_CUSTOMBULLET: // We cannot keep the custom type/style
    default:
        {
            m_orderedList=false;
            strResult="<ul>\n";
            break;
        }
    case CounterData::STYLE_NONE:
        {
            m_orderedList=false;
            strResult="<ul style=\"list-style-type:none\">\n";
            break;
        }
    case CounterData::STYLE_CIRCLEBULLET:
        {
            m_orderedList=false;
            strResult="<ul style=\"list-style-type:circle\">\n";
            break;
        }
    case CounterData::STYLE_SQUAREBULLET:
        {
            m_orderedList=false;
            strResult="<ul style=\"list-style-type:square\">\n";
            break;
        }
    case CounterData::STYLE_DISCBULLET:
        {
            m_orderedList=false;
            strResult="<ul style=\"list-style-type:disc\">\n";
            break;
        }
    case CounterData::STYLE_NUM:
        {
            m_orderedList=true;
            strResult="<ol style=\"list-style-type:decimal\">\n";
            break;
        }
    case CounterData::STYLE_ALPHAB_L:
        {
            m_orderedList=true;
            strResult="<ol style=\"list-style-type:lower-alpha\">\n";
            break;
        }
    case CounterData::STYLE_ALPHAB_U:
        {
            m_orderedList=true;
            strResult="<ol style=\"list-style-type:upper-alpha\">\n";
            break;
        }
    case CounterData::STYLE_ROM_NUM_L:
        {
            m_orderedList=true;
            strResult="<ol style=\"list-style-type:lower-roman\">\n";
            break;
        }
    case CounterData::STYLE_ROM_NUM_U:
        {
            m_orderedList=true;
            strResult="<ol style=\"list-style-type:upper-roman\">\n";
            break;
        }
    case CounterData::STYLE_CUSTOM:
        {
            // We cannot keep the custom type/style
            m_orderedList=true;
            strResult="<ol>\n";
            break;
        }
    }
    return strResult;
}

QString HtmlCssWorker::layoutToCss(const LayoutData& layoutOrigin,
    const LayoutData& layout, const bool force) const
{
    QString strLayout;

    if (force || (layoutOrigin.alignment!=layout.alignment))
    {
        if ( (layout.alignment=="left") || (layout.alignment== "right")
            || (layout.alignment=="center") || (layout.alignment=="justify"))
        {
            strLayout += QString("text-align:%1; ").arg(layout.alignment);
        }
        else
        {
            kdWarning(30503) << "Unknown alignment: " << layout.alignment << endl;
        }
    }

    if ((layout.indentLeft>=0.0)
        && (force || (layoutOrigin.indentLeft!=layout.indentLeft)))
    {
            strLayout += QString("margin-left:%1pt; ").arg(layout.indentLeft);
    }

    if ((layout.indentRight>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
        strLayout += QString("margin-right:%1pt; ").arg(layout.indentRight);
    }

    if (force || (layoutOrigin.indentLeft!=layout.indentLeft))
    {
        strLayout += QString("text-indent:%1pt; ").arg(layout.indentFirst);
    }

    if ((layout.marginBottom>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
       strLayout += QString("margin-bottom:%1pt; ").arg(layout.marginBottom);
    }

    if ((layout.marginTop>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
       strLayout += QString("margin-top:%1pt; ").arg(layout.marginTop);
    }

    // TODO: Konqueror/KHTML does not support "line-height"
    if (!force
        && (layoutOrigin.lineSpacingType==layoutOrigin.lineSpacingType)
        && (layoutOrigin.lineSpacing==layoutOrigin.lineSpacing))
    {
        // Do nothing!
    }
    else if ( !layout.lineSpacingType )
    {
        // We have a custom line spacing (in points)
        strLayout += QString("line-height:%1pt; ").arg(layout.lineSpacing);
    }
    else if ( 15==layout.lineSpacingType  )
    {
        strLayout += "line-height:1.5; "; // One-and-half
    }
    else if ( 20==layout.lineSpacingType  )
    {
        strLayout += "line-height:2.0; "; // Two
    }
    else if ( layout.lineSpacingType!=10  )
    {
        kdWarning(30503) << "Curious lineSpacingType: " << layout.lineSpacingType << " (Ignoring!)" << endl;
    }

    // TODO: Konqueror/KHTML does not support "text-shadow"
    if (!force
        && (layoutOrigin.shadowDirection==layoutOrigin.shadowDirection)
        && (layoutOrigin.shadowDistance==layoutOrigin.shadowDistance))
    {
        // Do nothing!
    }
    else if ((!layout.shadowDirection) || (!layout.shadowDistance))
    {
        strLayout += "text-shadow:";
        strLayout+="none; ";
    }
    else
    {
        double xDistance,yDistance;
        const double distance=layout.shadowDistance;
        switch (layout.shadowDirection)
        {
        case 1: // SD_LEFT_UP
            {
                xDistance= (-distance);
                yDistance= (-distance);
                break;
            }
        case 2: // SD_UP
            {
                xDistance= 0;
                yDistance= (-distance);
                break;
            }
        case 3: // SD_RIGHT_UP
            {
                xDistance= (distance);
                yDistance= (-distance);
                break;
            }
        case 4: // SD_RIGHT
            {
                xDistance= (distance);
                yDistance= 0;
                break;
            }
        case 5: // SD_RIGHT_BOTTOM
            {
                xDistance= (distance);
                yDistance= (distance);
                break;
            }
        case 6: // SD_BOTTOM
            {
                xDistance= 0;
                yDistance= (distance);
                break;
            }
        case 7: // SD_LEFT_BOTTOM
            {
                xDistance= (-distance);
                yDistance= (distance);
                break;
            }
        case 8: // SD_LEFT
            {
                xDistance= (distance);
                yDistance= 0;
                break;
            }
        default:
            {
                xDistance=0;
                yDistance=0;
                break;
            }
        }
        if ( (!xDistance) && (!yDistance) )
        {
            strLayout += "text-shadow:";
            strLayout+="none; ";
        }
        else
        {
            strLayout += "text-shadow:";
            strLayout+=QString("%1 %2pt %3pt; ").arg(layout.shadowColor.name())
                .arg(xDistance,0,'f',0).arg(yDistance,0,'f',0);
                // We do not want any scientific notation or any decimal
        }
    }

    // TODO: borders

    // This must remain last, as the last property does not have a semi-colon
    strLayout+=textFormatToCss(layoutOrigin.formatData.text,
        layout.formatData.text,force);

    return strLayout;
}

void HtmlCssWorker::openParagraph(const QString& strTag,
    const LayoutData& layout)
{
    const LayoutData& styleLayout=m_styleMap[layout.styleName];

    *m_streamOut << '<' << strTag;

    // Opening elements
    *m_streamOut << " class=\"" << escapeCssIdentifier(layout.styleName);
    *m_streamOut << "\"";

    QString strStyle=layoutToCss(styleLayout,layout,false);
    if (!strStyle.isEmpty())
    {
        *m_streamOut << " style=\"" << strStyle << "\"";
    }
    
    *m_streamOut << ">";

    if ( 1==layout.formatData.text.verticalAlignment )
    {
        *m_streamOut << "<sub>"; //Subscript
    }
    else if ( 2==layout.formatData.text.verticalAlignment )
    {
        *m_streamOut << "<sup>"; //Superscript
    }
}

void HtmlCssWorker::closeParagraph(const QString& strTag,
    const LayoutData& layout)
{
    if ( 2==layout.formatData.text.verticalAlignment )
    {
        *m_streamOut << "</sup>"; //Superscript
    }
    else if ( 1==layout.formatData.text.verticalAlignment )
    {
        *m_streamOut << "</sub>"; //Subscript
    }

    *m_streamOut << "</" << strTag << ">\n";
}

void HtmlCssWorker::openSpan(const FormatData& formatOrigin, const FormatData& format)
{
    *m_streamOut << "<span style=\"";
    *m_streamOut << textFormatToCss(formatOrigin.text,format.text,false);
    *m_streamOut << "\">"; // close span opening tag

    if ( 1==format.text.verticalAlignment )
    {
        *m_streamOut << "<sub>"; //Subscript
    }
    else if ( 2==format.text.verticalAlignment )
    {
        *m_streamOut << "<sup>"; //Superscript
    }
}

void HtmlCssWorker::closeSpan(const FormatData& formatOrigin, const FormatData& format)
{
    if ( 2==format.text.verticalAlignment )
    {
        *m_streamOut << "</sup>"; //Superscript
    }
    else if ( 1==format.text.verticalAlignment )
    {
        *m_streamOut << "</sub>"; //Subscript
    }

    *m_streamOut << "</span>";
}

bool HtmlCssWorker::doFullPaperFormat(const int format,
            const double width, const double height, const int orientation)
{
    QString strWidth, strHeight, strUnits;
    KWEFUtil::GetNativePaperFormat(format, strWidth, strHeight, strUnits);

    if ((strWidth.isEmpty())||(strHeight.isEmpty())||(strUnits.isEmpty()))
    {
        // page format is unknown, so we need the size information
        strUnits="pt";
        strWidth=QString::number(width);
        strHeight=QString::number(height);
    }
    if (orientation==1)
    {
        // Landscape, so we must swap the sizes
        QString strTemp(strWidth);
        strWidth=strHeight;
        strHeight=strTemp;
    }

    m_strPageSize="size: ";
    m_strPageSize+=strWidth;
    m_strPageSize+=strUnits;
    m_strPageSize+=" ";
    m_strPageSize+=strHeight;
    m_strPageSize+=strUnits;
    m_strPageSize+=";";
    return true;
}

bool HtmlCssWorker::doOpenStyles(void)
{
    *m_streamOut << "<style type=\"text/css\">\n";
    if (!isXML())
    {
        // Put the style under comment to increase the compatibility with old browsers
        // However in XHTML 1.0, you cannot put the style definition into HTML comments
        *m_streamOut << "<!--\n";
    }
    // TODO: does KWord gives a paper colour?
    *m_streamOut << "BODY\n{\n  background-color: #FFFFFF\n}\n";

    return true;
}

bool HtmlCssWorker::doFullDefineStyle(LayoutData& layout)
{
    //Register style in the style map
    m_styleMap[layout.styleName]=layout;
    
    // We do not limit (anymore) any style to <h1> ... <h6>, because
    //   the style could be forced on <p> by the layout.

    *m_streamOut << "." << escapeCssIdentifier(layout.styleName);
    *m_streamOut << "\n{\n  " << layoutToCss(layout,layout,true) << "\n}\n";

    return true;
}

bool HtmlCssWorker::doCloseStyles(void)
{
    if (!m_strPageSize.isEmpty())
    {
        *m_streamOut << "@page\n{\n  ";
        *m_streamOut << m_strPageSize;
        // TODO: margins
        *m_streamOut << "\n}\n";
    }

    if (!isXML())
    {
        // Put the style under comment to increase the compatibility with old browsers
        // However in XHTML 1.0, you cannot put the style definition into HTML comments
        *m_streamOut << "-->\n";
    }
    *m_streamOut << "</style>\n";

    return true;
}

