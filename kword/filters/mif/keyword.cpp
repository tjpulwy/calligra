/* $Id$
 *
 * This file is part of QtMFC, the MFC API for the Qt toolkit.
 *
 * Copyright (C) 1998 by Matthias Kalle Dalheimer <kalle@dalheimer.de>
 */

#include "treebuild.h"
#include "y.tab.h"

#include <stdio.h>
#include <string.h>

/*
 * keyword: Take a text string and determine if it is, in fact a valid 
 * keyword. If it is, return the value of the keyword; if not, return
 * zero. N.B.: The token values must be nonzero.
 */

extern "C" {
	int keyword( char* string );
};

static struct keyword {
	char* name;              /* text string */
	int   value;             /* token */
	int   length;            /* length of name */
} keywords[] = 
{
	"AFrame", AFRAME, 6,
	"AFrames", AFRAMES, 7,
	"ATbl", ATBL, 4,
	"AnchorAlign", ANCHORALIGN, 11,
	"Angle", ANGLE, 5,
	"ArrowStyle", ARROWSTYLE, 10,
	"AutoNumSeries", AUTONUMSERIES, 13,
	"BitMapDpi", BITMAPDPI, 9,
	"BLOffset", BLOFFSET, 8,
	"BRect", BRECT, 5,
	"BaseAngle", BASEANGLE, 9,
	"BookComponent", BOOKCOMPONENT, 13,
	"CColor", CCOLOR, 6,
	"CSeparation", CSEPARATION, 11,
	"CState", CSTATE, 6,
	"CStyle", CSTYLE, 6,
	"CTag", CTAG, 4,
	"Cell", CELL, 4,
	"CellBRuling", CELLBRULING, 11,
	"CellColor", CELLCOLOR, 9,
	"CellColumns", CELLCOLUMNS, 11,
	"CellContent", CELLCONTENT, 11,
	"CellFill", CELLFILL, 8,
	"CellLRuling", CELLLRULING, 11,
	"CellRows", CELLROWS, 8,
	"CellRRuling", CELLRRULING, 11,
	"CellTRuling", CELLTRULING, 11,
	"CellSeparation", CELLSEPARATION, 14,
	"Char", CHAR, 4,
	"CharUnits", CHARUNITS, 9,
	"Color", COLOR, 5,
	"ColorAttribute", COLORATTRIBUTE, 14,
	"ColorBlack", COLORBLACK, 10,
	"ColorCatalog", COLORCATALOG, 12,
	"ColorCyan", COLORCYAN, 9,
	"ColorLibraryFamilyName", COLORLIBRARYFAMILYNAME, 22,
	"ColorLibraryInkName", COLORLIBRARYINKNAME, 19,
	"ColorMagenta", COLORMAGENTA, 12,
	"ColorTag", COLORTAG, 8,
	"ColorYellow", COLORYELLOW, 11,
	"Colors", COLORS, 6,
	"CombinedFontCatalog", COMBINEDFONTCATALOG, 19,
	"Condition", CONDITION, 9,
	"ConditionCatalog", CONDITIONCATALOG, 16,
	"Cropped", CROPPED, 7,
	"DAcrobatBookmarksIncludeTagNames", DACROBATBOOKMARKSINCLUDETAGNAMES, 32,
	"DAcrobatParagraphBookmarks", DACROBATPARAGRAPHBOOKMARKS, 26, 
	"DAutoChBars", DAUTOCHBARS, 11,
	"DBordersOn", DBORDERSON, 10, 
	"DChBarColor", DCHBARCOLOR, 11,
	"DChBarGap", DCHBARGAP, 9,
	"DChBarPosition", DCHBARPOSITION, 14,
	"DChBarWidth", DCHBARWIDTH, 11,
	"DCurrentView", DCURRENTVIEW, 12,
	"DDisplayOverrides", DDISPLAYOVERRIDES, 17,
	"DFNoteAnchorPos", DFNOTEANCHORPOS, 15,
	"DFNoteAnchorPrefix", DFNOTEANCHORPREFIX, 18, 
	"DFNoteAnchorSuffix", DFNOTEANCHORSUFFIX, 18,
	"DFNoteLabels", DFNOTELABELS, 12,
	"DFNoteMaxH", DFNOTEMAXH, 10, 
	"DFNoteNumStyle", DFNOTENUMSTYLE, 14,
	"DFNoteNumberPos", DFNOTENUMBERPOS, 15,
	"DFNoteNumberPrefix", DFNOTENUMBERPREFIX, 18, 
	"DFNoteNumberSuffix", DFNOTENUMBERSUFFIX, 18,
	"DFNoteRestart", DFNOTERESTART, 13,
	"DFNoteTag", DFNOTETAG, 9,
	"DFluid", DFLUID, 6,
	"DFluidSideheads", DFLUIDSIDEHEADS, 15,
	"DFrozenPages", DFROZENPAGES, 12,
	"DFullRulers", DFULLRULERS, 11,
	"DGenerateAcrobatInfo", DGENERATEACROBATINFO, 20,
	"DGraphicsOff", DGRAPHICSOFF, 12,
	"DGridOn", DGRIDON, 7,
	"DLanguage", DLANGUAGE, 9,
	"DLinebreakChars", DLINEBREAKCHARS, 15,
	"DLinkBoundariesOn", DLINKBOUNDARIESON, 17,
	"DMathAlphaCharFontFamily", DMATHALPHACHARFONTFAMILY, 24,
	"DMathCatalog", DMATHCATALOG, 12,
	"DMathFunctions", DMATHFUNCTIONS, 14,
	"DMathGreek", DMATHGREEK, 10,
	"DMathLargeHoriz", DMATHLARGEHORIZ, 15,
	"DMathLargeIntegral", DMATHLARGEINTEGRAL, 18,
	"DMathLargeLevel1", DMATHLARGELEVEL1, 16,
	"DMathLargeLevel2", DMATHLARGELEVEL2, 16,
	"DMathLargeLevel3", DMATHLARGELEVEL3, 16,
	"DMathLargeSigma", DMATHLARGESIGMA, 15,
	"DMathLargeVert", DMATHLARGEVERT, 14,
	"DMathMediumHoriz", DMATHMEDIUMHORIZ, 16,
	"DMathMediumIntegral", DMATHMEDIUMINTEGRAL, 19,
	"DMathMediumLevel1", DMATHMEDIUMLEVEL1, 17,
	"DMathMediumLevel2", DMATHMEDIUMLEVEL2, 17,
	"DMathMediumLevel3", DMATHMEDIUMLEVEL3, 17,
	"DMathMediumSigma", DMATHMEDIUMSIGMA, 16,
	"DMathMediumVert", DMATHMEDIUMVERT, 15,
	"DMathNumbers", DMATHNUMBERS, 12, 
	"DMathShowCustom", DMATHSHOWCUSTOM, 15, 
	"DMathSmallHoriz", DMATHSMALLHORIZ, 15,
	"DMathSmallIntegral", DMATHSMALLINTEGRAL, 18,
	"DMathSmallLevel1", DMATHSMALLLEVEL1, 16,
	"DMathSmallLevel2", DMATHSMALLLEVEL2, 16,
	"DMathSmallLevel3", DMATHSMALLLEVEL3, 16,
	"DMathSmallSigma", DMATHSMALLSIGMA, 15,
	"DMathSmallVert", DMATHSMALLVERT, 14,
	"DMathStrings", DMATHSTRINGS, 12,
	"DMathVariables", DMATHVARIABLES, 14,
	"DMenuBar", DMENUBAR, 8,
	"DNarrowRubiSpaceForJapanese", DNARROWRUBISPACEFORJAPANESE, 27,
	"DNarrowRubiSpaceForOther", DNARROWRUBISPACEFOROTHER, 24,
	"DNextUnique", DNEXTUNIQUE, 11,
	"DNoPrintSepColor", DNOPRINTSEPCOLOR, 16,
	"DPageGrid", DPAGEGRID, 9,
	"DPageNumStyle", DPAGENUMSTYLE, 13,
	"DPagePointStyle", DPAGEPOINTSTYLE, 15,
	"DPageRounding", DPAGEROUNDING, 13,
	"DPageScrolling", DPAGESCROLLING, 14,
	"DPageSize", DPAGESIZE, 9,
	"DParity", DPARITY, 7,
	"DPrintSeparations", DPRINTSEPARATIONS, 17, 
	"DPrintSkipBlankPages", DPRINTSKIPBLANKPAGES, 20,
	"DPunctuationChars", DPUNCTUATIONCHARS, 17,
	"DRubiAlignAtLineBounds", DRUBIALIGNATLINEBOUNDS, 22,
	"DRubiOverhang", DRUBIOVERHANG, 13,
	"DRubiSize", DRUBISIZE, 9,
	"DRulersOn", DRULERSON, 9,
	"DShowAllConditions", DSHOWALLCONDITIONS, 18,
	"DSmallCapsSize", DSMALLCAPSSIZE, 14,
	"DSmallCapsStretch", DSMALLCAPSSTRETCH, 17,
	"DSmartQuotesOn", DSMARTQUOTESON, 14, 
	"DSmartSpacesOn", DSMARTSPACESON, 14,
	"DSnapGrid", DSNAPGRID, 9,
	"DSnapRotation", DSNAPROTATION, 13,
	"DStartPage", DSTARTPAGE, 10, 
	"DSubscriptShift", DSUBSCRIPTSHIFT, 15,
	"DSubscriptSize", DSUBSCRIPTSIZE, 14,
	"DSubscriptStretch", DSUBSCRIPTSTRETCH, 17,
	"DSuperscriptShift", DSUPERSCRIPTSHIFT, 17,
	"DSuperscriptSize", DSUPERSCRIPTSIZE, 16,
	"DSuperscriptStretch", DSUPERSCRIPTSTRETCH, 19, 
	"DSymbolsOn", DSYMBOLSON, 10,
	"DTblFNoteAnchorPos", DTBLFNOTEANCHORPOS, 18,
	"DTblFNoteAnchorPrefix", DTBLFNOTEANCHORPREFIX, 21,
	"DTblFNoteAnchorSuffix", DTBLFNOTEANCHORSUFFIX, 21,
	"DTblFNoteLabels", DTBLFNOTELABELS, 15,
	"DTblFNoteNumStyle", DTBLFNOTENUMSTYLE, 17,
	"DTblFNoteNumberPos", DTBLFNOTENUMBERPOS, 18,
	"DTblFNoteNumberPrefix", DTBLFNOTENUMBERPREFIX, 21, 
	"DTblFNoteNumberSuffix", DTBLFNOTENUMBERSUFFIX, 21,
	"DTblFNoteTag", DTBLFNOTETAG, 12,
	"DTwoSides", DTWOSIDES, 9,
	"DUpdateTextInsetsOnOpen", DUPDATETEXTINSETSONOPEN, 23,
	"DUpdateXRefsOnOpen", DUPDATEXREFSONOPEN, 18,
	"DViewOnly", DVIEWONLY, 9,
	"DViewOnlySelect", DVIEWONLYSELECT, 15,
	"DViewOnlyWinBorders", DVIEWONLYWINBORDERS, 19,
	"DViewOnlyWinMenubar", DVIEWONLYWINMENUBAR, 19,
	"DViewOnlyWinPalette", DVIEWONLYWINPALETTE, 19,
	"DViewOnlyWinPopup", DVIEWONLYWINPOPUP, 17,
	"DViewOnlyXRef", DVIEWONLYXREF, 13,
	"DViewRect", DVIEWRECT, 9,
	"DViewScale", DVIEWSCALE, 10,
	"DVoMenuBar", DVOMENUBAR, 10,
	"DWideRubiSpaceForJapanese", DWIDERUBISPACEFORJAPANESE, 25,
	"DWideRubiSpaceForOther", DWIDERUBISPACEFOROTHER, 22,
	"DWindowRect", DWINDOWRECT, 11,
	"DashedPattern", DASHEDPATTERN, 13,
	"DashedStyle", DASHEDSTYLE, 11,
	"DashSegment", DASHSEGMENT, 11,
	"DeriveLinks", DERIVELINKS, 11, 
	"DeriveTag", DERIVETAG, 9,
	"DeriveType", DERIVETYPE, 10,
	"Dictionary", DICTIONARY, 10,
	"Document", DOCUMENT, 8,
	"Ellipse", ELLIPSE, 7,
	"FAngle", FANGLE, 6,
	"FCase", FCASE, 5,
	"FChangeBar", FCHANGEBAR, 10,
	"FColor", FCOLOR, 6,
	"FDW", FDW, 3,
	"FDX", FDX, 3,
	"FDY", FDY, 3,
	"FEncoding", FENCODING, 9,
	"FFamily", FFAMILY, 7,
	"FLanguage", FLANGUAGE, 9,
	"FLocked", FLOCKED, 7,
	"FNote", FNOTE, 5,
	"FNoteStartNum", FNOTESTARTNUM, 13,
	"FOutline", FOUTLINE, 8,
	"FOverline", FOVERLINE, 9,
	"FPairKern", FPAIRKERN, 9,
	"FPlatformName", FPLATFORMNAME, 13,
	"FPosition", FPOSITION, 9,
	"FPostScriptName", FPOSTSCRIPTNAME, 15,
	"FSeparation", FSEPARATION, 11,
	"FShadow", FSHADOW, 7,
	"FSize", FSIZE, 5,
	"FStretch", FSTRETCH, 8,
	"FStrike", FSTRIKE, 7,
	"FTag", FTAG, 4,
	"FTsume", FTSUME, 6,
	"FUnderlining", FUNDERLINING, 12,
	"FVar", FVAR, 4,
	"FWeight", FWEIGHT, 7,
	"FileName", FILENAME, 8,
	"FileNameSuffix", FILENAMESUFFIX, 14,
	"Fill", FILL, 4,
	"FlipLR", FLIPLR, 6,
	"FlowTag", FLOWTAG, 7,
	"Font", FONT, 4,
	"FontCatalog", FONTCATALOG, 11,
	"Frame", FRAME, 5,
	"FrameType", FRAMETYPE, 9,
	"Group", GROUP, 5,
	"GroupID", GROUPID, 7,
	"HeadCap", HEADCAP, 7,
	"HeadType", HEADTYPE, 8,
	"HyphenMaxLines", HYPHENMAXLINES, 14,
	"HyphenMinPrefix", HYPHENMINPREFIX, 15,
	"HyphenMinSuffix", HYPHENMINSUFFIX, 15,
	"HyphenMinWord", HYPHENMINWORD, 13,
	"ID", FRAMEID, 2,
	"ImportObFile", IMPORTOBFILE, 12,
	"ImportObject", IMPORTOBJECT, 12,
	"InitialAutoNums", INITIALAUTONUMS, 15,
	"KumihanCatalog", KUMIHANCATALOG, 14,
	"Length", LENGTH, 6,
	"MarkerTypeCatalog", MARKERTYPECATALOG, 17,
	"Math", MATH, 4,
	"MathAlignment", MATHALIGNMENT, 13,
	"MathFullForm", MATHFULLFORM, 12,
	"MathLineBreak", MATHLINEBREAK, 13,
	"MathOrigin", MATHORIGIN, 10,
	"MathSize", MATHSIZE, 8,
	"MCurrPage", MCURRPAGE, 9,
	"MIFFile", MIFFILE, 7,
	"MText", MTEXT, 5,
	"MType", MTYPE, 5,
	"MTypeName", MTYPENAME, 9,
	"Marker", MARKER, 6,
	"NSOffset", NSOFFSET, 8,
	"NativeOrigin", NATIVEORIGIN, 12,
	"Notes", NOTES, 5,
	"NumCounter", NUMCOUNTER, 10,
	"NumPoints", NUMPOINTS, 9,
	"NumSegments", NUMSEGMENTS, 11,
	"OKWord", OKWORD, 6,
	"ObColor", OBCOLOR, 7,
	"Page", PAGE, 4,
	"PageAngle", PAGEANGLE, 9,
	"PageBackground", PAGEBACKGROUND, 14,
	"PageNum", PAGENUM, 7,
	"PageOrientation", PAGEORIENTATION, 15,
	"PageSize", PAGESIZE, 8,
	"PageTag", PAGETAG, 7,
	"PageType", PAGETYPE, 8,
	"Para", PARA, 4,
	"ParaLine", PARALINE, 8,
	"Pen", PEN, 3,
	"PenWidth", PENWIDTH, 8,
	"Pgf", PGF, 3,
	"PgfAcrobatLevel", PGFACROBATLEVEL, 15,
	"PgfAlignment", PGFALIGNMENT, 12,
	"PgfAutoNum", PGFAUTONUM, 10,
	"PgfBlockSize", PGFBLOCKSIZE, 12,
	"PgfBotSepAtIndent", PGFBOTSEPATINDENT, 17,
	"PgfBotSepOffset", PGFBOTSEPOFFSET, 15,
	"PgfBotSeparator", PGFBOTSEPARATOR, 15,
	"PgfCatalog", PGFCATALOG, 10,
	"PgfCellAlignment", PGFCELLALIGNMENT, 16,
	"PgfCellBMarginFixed", PGFCELLLMARGINFIXED, 19,
	"PgfCellLMarginFixed", PGFCELLLMARGINFIXED, 19,
	"PgfCellMargins", PGFCELLMARGINS, 14,
	"PgfCellRMarginFixed", PGFCELLLMARGINFIXED, 19,
	"PgfCellTMarginFixed", PGFCELLLMARGINFIXED, 19,
	"PgfFIndent", PGFFINDENT, 10,
	"PgfFIndentOffset", PGFFINDENTOFFSET, 16,
	"PgfFIndentRelative", PGFFINDENTRELATIVE, 18,
	"PgfFont", PGFFONT, 7,
	"PgfHyphenate", PGFHYPHENATE, 12,
	"PgfLIndent", PGFLINDENT, 10,
	"PgfLanguage", PGFLANGUAGE, 11,
	"PgfLeading", PGFLEADING, 10,
	"PgfLetterSpace", PGFLETTERSPACE, 14,
	"PgfLineSpacing", PGFLINESPACING, 14,
	"PgfLocked", PGFLOCKED, 9,
	"PgfMaxJLetterSpace", PGFMAXJLETTERSPACE, 18,
	"PgfMaxJRomanLetterSpace", PGFMAXJROMANLETTERSPACE, 23,
	"PgfMaxWordSpace", PGFMAXWORDSPACE, 15,
	"PgfMinJLetterSpace", PGFMINJLETTERSPACE, 18,
	"PgfMinJRomanLetterSpace", PGFMINJROMANLETTERSPACE, 23,
	"PgfMinWordSpace", PGFMINWORDSPACE, 15,
	"PgfNextTag", PGFNEXTTAG, 10,
	"PgfNumAtEnd", PGFNUMATEND, 11,
	"PgfNumFormat", PGFNUMFORMAT, 12,
	"PgfNumString", PGFNUMSTRING, 12,
	"PgfNumTabs", PGFNUMTABS, 10,
	"PgfNumberFont", PGFNUMBERFONT, 13,
	"PgfOptJLetterSpace", PGFOPTJLETTERSPACE, 18,
	"PgfOptJRomanLetterSpace", PGFOPTJROMANLETTERSPACE, 23,
	"PgfOptWordSpace", PGFOPTWORDSPACE, 15,
	"PgfPlacement", PGFPLACEMENT, 12,
	"PgfPlacementStyle", PGFPLACEMENTSTYLE, 17,
	"PgfRIndent", PGFRINDENT, 10,
	"PgfRunInDefaultPunct", PGFRUNINDEFAULTPUNCT, 20,
	"PgfSpAfter", PGFSPAFTER, 10,
	"PgfSpBefore", PGFSPBEFORE, 11,
	"PgfTag", PGFTAG, 6,
	"PgfTopSepAtIndent", PGFTOPSEPATINDENT, 17,
	"PgfTopSepOffset", PGFTOPSEPOFFSET, 15,
	"PgfTopSeparator", PGFTOPSEPARATOR, 15,
	"PgfUseNextTag", PGFUSENEXTTAG, 13,
	"PgfWithNext", PGFWITHNEXT, 11,
	"PgfWithPrev", PGFWITHPREV, 11,
	"PgfYakumonoType", PGFYAKUMONOTYPE, 15,
	"Point", POINT, 5,
	"Polygon", POLYGON, 7,
	"PolyLine", POLYLINE, 8,
	"Rectangle", RECTANGLE, 9,
	"Row", ROW, 3,
	"RowHeight", ROWHEIGHT, 9,
	"RowMaxHeight", ROWMAXHEIGHT, 12,
	"RowMinHeight", ROWMINHEIGHT, 12,
	"Ruling", RULING, 6,
	"RulingCatalog", RULINGCATALOG, 13,
	"RulingColor", RULINGCOLOR, 11,
	"RulingGap", RULINGGAP, 9,
	"RulingLines", RULINGLINES, 11,
	"RulingPen", RULINGPEN, 9,
	"RulingPenWidth", RULINGPENWIDTH, 14,
	"RulingSeparation", RULINGSEPARATION, 16,
	"RulingTag", RULINGTAG, 9,
	"RunaroundGap", RUNAROUNDGAP, 12,
	"RunaroundType", RUNAROUNDTYPE, 13,
	"ScaleFactor", SCALEFACTOR, 11,
	"ScaleHead", SCALEHEAD, 9,
	"Separation", SEPARATION, 10,
	"Series", SERIES, 6,
	"ShapeRect", SHAPERECT, 9,
	"String", STRING, 6,
	"TFAutoConnect", TFAUTOCONNECT, 13,
	"TFLineSpacing", TFLINESPACING, 13,
	"TFMinHangHeight", TFMINHANGHEIGHT, 15,
	"TFSynchronized", TFSYNCHRONIZED, 14,
	"TFTag", TFTAG, 5,
	"TLAlignment", TLALIGNMENT, 11,
	"TLOrigin", TLORIGIN, 8,
	"TRColumnBalance", TRCOLUMNBALANCE, 15,
	"TRColumnGap", TRCOLUMNGAP, 11,
	"TRNext", TRNEXT, 6,
	"TRNumColumns", TRNUMCOLUMNS, 12,
	"TRSideheadGap", TRSIDEHEADGAP, 13,
	"TRSideheadPlacement", TRSIDEHEADPLACEMENT, 19,
	"TRSideheadWidth", TRSIDEHEADWIDTH, 15,
	"TSDecimalChar", TSDECIMALCHAR, 13,
	"TSLeaderStr", TSLEADERSTR, 11,
	"TSType", TSTYPE, 6,
	"TSX", TSX, 3,
	"TabStop", TABSTOP, 7,
	"Tag", TAG, 3,
	"TailCap", TAILCAP, 7,
	"Tbl", TBL, 3,
	"TblAlignment", TBLALIGNMENT, 12,
	"TblAltShadePeriod", TBLALTSHADEPERIOD, 17,
	"TblBRuling", TBLBRULING, 10,
	"TblBlockSize", TBLBLOCKSIZE, 12,
	"TblBody", TBLBODY, 7,
	"TblBodyColor", TBLBODYCOLOR, 12,
	"TblBodyFill", TBLBODYFILL, 11,
	"TblBodyRowRuling", TBLBODYROWRULING, 16,
	"TblBodySeparation", TBLBODYSEPARATION, 17,
	"TblCatalog", TBLCATALOG, 10,
	"TblCellMargins", TBLCELLMARGINS, 14,
	"TblColumn", TBLCOLUMN, 9,
	"TblColumnBody", TBLCOLUMNBODY, 13,
	"TblColumnF", TBLCOLUMNF, 10,
	"TblColumnH", TBLCOLUMNH, 10,
	"TblColumnNum", TBLCOLUMNNUM, 12,
	"TblColumnRuling", TBLCOLUMNRULING, 15,
	"TblColumnWidth", TBLCOLUMNWIDTH, 14,
	"TblFormat", TBLFORMAT, 9,
	"TblH", TBLH, 4,
	"TblHFColor", TBLHFCOLOR, 10,
	"TblHFFill", TBLHFFILL, 9,
	"TblHFRowRuling", TBLHFROWRULING, 14,
	"TblHFSeparation", TBLHFSEPARATION, 15,
	"TblID", TBLID, 5,
	"TblInitNumBodyRows", TBLINITNUMBODYROWS, 18,
	"TblInitNumColumns", TBLINITNUMCOLUMNS, 17,
	"TblInitNumFRows", TBLINITNUMFROWS, 15,
	"TblInitNumHRows", TBLINITNUMHROWS, 15,
	"TblLIndent", TBLLINDENT, 10,
	"TblLRuling", TBLLRULING, 10,
	"TblLastBRuling", TBLLASTBRULING, 14,
	"TblLocked", TBLLOCKED, 9,
	"TblNumByColumn", TBLNUMBYCOLUMN, 14,
	"TblNumColumns", TBLNUMCOLUMNS, 13,
	"TblPlacement", TBLPLACEMENT, 12,
	"TblRIndent", TBLRINDENT, 10,
	"TblRRuling", TBLRRULING, 10,
	"TblRulingPeriod", TBLRULINGPERIOD, 15,
	"TblSeparatorRuling", TBLSEPARATORRULING, 18,
	"TblShadeByColumn", TBLSHADEBYCOLUMN, 16,
	"TblShadePeriod", TBLSHADEPERIOD, 14,
	"TblSpAfter", TBLSPAFTER, 10,
	"TblSpBefore", TBLSPBEFORE, 11,
	"TblTRuling", TBLTRULING, 10,
	"TblTag", TBLTAG, 6,
	"TblTitle", TBLTITLE, 8,
	"TblTitleContent", TBLTITLECONTENT, 15,
	"TblTitleGap", TBLTITLEGAP, 11,
	"TblTitlePgf1", TBLTITLEPGF1, 12,
	"TblTitlePlacement", TBLTITLEPLACEMENT, 17,
	"TblXColor", TBLXCOLOR, 9,
	"TblXColumnNum", TBLXCOLUMNNUM, 13,
	"TblXColumnRuling", TBLXCOLUMNRULING, 16,
	"TblXFill", TBLXFILL, 8,
	"TblXRowRuling", TBLXROWRULING, 13,
	"TblXSeparation", TBLXSEPARATION, 14,
	"Tbls", TBLS, 4,
	"TextFlow", TEXTFLOW, 8,
	"TextLine", TEXTLINE, 8,
	"TextRect", TEXTRECT, 8,
	"TextRectID", TEXTRECTID, 10,
	"TipAngle", TIPANGLE, 8,
	"Unique", UNIQUE, 6,
	"Unique", UNIQUE, 6,
	"Units", UNITS, 5,
	"Variable", VARIABLE, 8,
	"VariableDef", VARIABLEDEF, 11,
	"VariableFormat", VARIABLEFORMAT, 14,
	"VariableFormats", VARIABLEFORMATS, 15,
	"VariableName", VARIABLENAME, 12,
	"View", VIEW, 4,
	"ViewCutout", VIEWCUTOUT, 10,
	"ViewInvisible", VIEWINVISIBLE, 13,
	"ViewNumber", VIEWNUMBER, 10,
	"Views", VIEWS, 5,
	"XRefDef", XREFDEF, 7,
	"XRefFormat", XREFFORMAT, 10,
	"XRefFormats", XREFFORMATS, 11,
	"XRefName", XREFNAME, 8,
	"XRef", XREF, 4,
	"XRefSrcText", XREFSRCTEXT, 11,
	"XRefSrcIsElem", XREFSRCISELEM, 13,
	"XRefSrcFile", XREFSRCFILE, 11,
	"XRefEnd", XREFEND, 7,
	"XRefLastUpdate", XREFLASTUPDATE, 14,
	"pc", UNIT_PC, 2,
	"pt", UNIT_PT, 2,
	(char*)0,   0,          0,
};

int keyword( char* string )
{
	struct keyword* ptr = keywords;
	int len = strlen( string );
	extern char* progname;

#ifdef KEY_CHECK
	fprintf( stderr, "%s: checking keywords\n", progname );
	while( ptr->length != 0 ) {
		if( strlen( ptr->name ) != ptr->length ) {
			fprintf( stderr, 
					 "keyword %s, length is %d should be %d\n",
					 ptr->name, strlen( ptr->name ),
					 ptr->length );
		}
		ptr++;
	}
	fprintf( stderr, "%s: done checking keywords, ok\n",
			 progname );
	ptr = keywords;
#endif /* KEY_CHECK */

	while( ptr->length != 0 )
		if( len == ptr->length &&
			( strcmp( ptr->name, string ) == 0 ) ) {
#ifdef PARSERDEBUG
			fprintf( stderr, "%s: returning keyword # %d\n",
					 progname, ptr->value );
#endif
			return ptr->value;
		} else {
#ifdef PARSERDEBUG
// 			fprintf( stderr, "%s: %s (%d) != %s (%d)\n",
// 					 progname, ptr->name, ptr->length,
// 					 string, len );
#endif
			ptr++;
		}

	return 0; /* no match */
}


			
		
