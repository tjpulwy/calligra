/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KISGLOBAL_H_
#define KISGLOBAL_H_

#include <limits.h>
#include <qglobal.h>
#include <kglobal.h>
#include <koGlobal.h>
#include <koUnit.h>

#define DBG_AREA_TILES 40001
#define DBG_AREA_CORE 41000

/**
 * Mime type for this app - not same as file type, but file types
 * can be associated with a mime type and are opened with applications
 * associated with the same mime type
 */
#define APP_MIMETYPE "application/x-krita"

/**
 * Mime type for native file format
 */
#define NATIVE_MIMETYPE "application/x-kra"


/**
 * Default size for a tile,  Usually, all
 * tiles are sqr(TILE_SIZE).  Only tiles
 * on the edge of an canvas are exempt from
 * this rule.
 */
const int TILE_SIZE = 64;

/**
 * Default width of a tile.
 */
const int TILE_WIDTH = TILE_SIZE;

/**
 * Default height of a tile.
 */
const int TILE_HEIGHT = TILE_SIZE;

/**
 * Size of a quantum -- this could be 8, 16, 32 or 64 -- but for now, only 8 is possible.
 */
#if !defined(QUANTUM_DEPTH)
#define QUANTUM_DEPTH 8 // bits, i.e., one byte per channel
#endif

#if (QUANTUM_DEPTH == 8)
typedef Q_UINT8 QUANTUM;
const QUANTUM QUANTUM_MAX = UCHAR_MAX;
const QUANTUM OPACITY_TRANSPARENT = 0;
const QUANTUM OPACITY_OPAQUE = QUANTUM_MAX;

const QUANTUM SELECTED = 0;
const QUANTUM NOT_SELECTED = 255;
#endif

const QUANTUM MAXCHANNELS = 8; // Imagetype 'wet' needs 8 channels
const Q_INT32 IMG_DEFAULT_DEPTH = 4;
const Q_INT32 RENDER_HEIGHT = TILE_SIZE * 4;
const Q_INT32 RENDER_WIDTH = TILE_SIZE * 4;

enum enumInputDevice {
	INPUT_DEVICE_UNKNOWN,
	INPUT_DEVICE_MOUSE,	// Standard mouse
	INPUT_DEVICE_STYLUS,	// Wacom stylus
	INPUT_DEVICE_ERASER,	// Wacom eraser
	INPUT_DEVICE_PUCK	// Wacom puck
};

/*
 * Most wacom pads have 512 levels of pressure; Qt only supports 256, and even 
 * this is downscaled to 127 levels because the line would be too jittery, and
 * the amount of masks take too much memory otherwise.
 */
const Q_INT32 PRESSURE_LEVELS= 127;
const double PRESSURE_MIN = 0.0;
const double PRESSURE_MAX = 1.0;
const double PRESSURE_DEFAULT = (PRESSURE_MAX - PRESSURE_MIN) / 2;
const double PRESSURE_THRESHOLD = 5.0 / 255.0;

enum CompositeOp {
	COMPOSITE_OVER,
	COMPOSITE_IN,
	COMPOSITE_OUT,
	COMPOSITE_ATOP,
	COMPOSITE_XOR,
	COMPOSITE_PLUS,
	COMPOSITE_MINUS,
	COMPOSITE_ADD,
	COMPOSITE_SUBTRACT,
	COMPOSITE_DIFF,
	COMPOSITE_MULT,
	COMPOSITE_BUMPMAP,
	COMPOSITE_COPY,
	COMPOSITE_COPY_RED,
	COMPOSITE_COPY_GREEN,
	COMPOSITE_COPY_BLUE,
	COMPOSITE_COPY_OPACITY,
	COMPOSITE_CLEAR,
	COMPOSITE_DISSOLVE,
	COMPOSITE_DISPLACE,
#if 0
	COMPOSITE_MODULATE,
	COMPOSITE_THRESHOLD,
#endif 
	COMPOSITE_NO,
#if 0
	COMPOSITE_DARKEN,
	COMPOSITE_LIGHTEN,
	COMPOSITE_HUE,
	COMPOSITE_SATURATE,
	COMPOSITE_COLORIZE,
	COMPOSITE_LUMINIZE,
	COMPOSITE_SCREEN,
	COMPOSITE_OVERLAY,
#endif
	COMPOSITE_COPY_CYAN,
	COMPOSITE_COPY_MAGENTA,
	COMPOSITE_COPY_YELLOW,
	COMPOSITE_COPY_BLACK,
	COMPOSITE_ERASE,
	 
	COMPOSITE_UNDEF,
};

#if 0
enum enumImgType {
#endif
// The following enum is keep to allow the loading of old document in KisDoc::loadImage
enum enumImgTypeDeprecated {
	IMAGE_TYPE_UNKNOWN,
	IMAGE_TYPE_INDEXED,
	IMAGE_TYPE_INDEXEDA,
	IMAGE_TYPE_GREY,
	IMAGE_TYPE_GREYA,
	IMAGE_TYPE_RGB,
	IMAGE_TYPE_RGBA,
	IMAGE_TYPE_CMYK,
	IMAGE_TYPE_CMYKA,
	IMAGE_TYPE_LAB,
	IMAGE_TYPE_LABA,
	IMAGE_TYPE_YUV,
	IMAGE_TYPE_YUVA,
	IMAGE_TYPE_WET,
	IMAGE_TYPE_SELECTION_MASK
};

enum enumPaintOp {
	PAINTOP_BRUSH,
	PAINTOP_PEN,
	PAINTOP_ERASE,
	PAINTOP_AIRBRUSH,
	PAINTOP_CONVOLVE,
	PAINTOP_DUPLICATE };

enum enumPaintStyles {
	PAINTSTYLE_HARD,
	PAINTSTYLE_SOFT };
		
// If QUANTUM changes size, this should change, too.
typedef Q_UINT8 CHANNELTYPE;
typedef Q_UINT8 PIXELTYPE;

const CHANNELTYPE REDCHANNEL = 0;
const CHANNELTYPE GREENCHANNEL = 1;
const CHANNELTYPE BLUECHANNEL = 2;
const CHANNELTYPE GRAYCHANNEL = 3;
const CHANNELTYPE INDEXEDCHANNEL = 4;
const CHANNELTYPE ALPHACHANNEL = 5;

const PIXELTYPE PIXEL_UNDEF = 255;

const PIXELTYPE PIXEL_GRAY = 0;
const PIXELTYPE PIXEL_GRAY_ALPHA = 1;

const PIXELTYPE PIXEL_BLUE = 0;
const PIXELTYPE PIXEL_GREEN = 1;
const PIXELTYPE PIXEL_RED = 2;
const PIXELTYPE PIXEL_ALPHA = 3;

const PIXELTYPE PIXEL_WET_RED = 0;
const PIXELTYPE PIXEL_WET_MYTH_RED = 1;
const PIXELTYPE PIXEL_WET_GREEN = 2;
const PIXELTYPE PIXEL_WET_MYTH_GREEN = 3;
const PIXELTYPE PIXEL_WET_BLUE = 4;
const PIXELTYPE PIXEL_WET_MYTH_BLUE = 5;
const PIXELTYPE PIXEL_WET_WATER = 6;
const PIXELTYPE PIXEL_WET_HEIGHT = 7;

const PIXELTYPE PIXEL_CYAN = 0;
const PIXELTYPE PIXEL_MAGENTA = 1;
const PIXELTYPE PIXEL_YELLOW = 2;
const PIXELTYPE PIXEL_BLACK = 3;
const PIXELTYPE PIXEL_CMYK_ALPHA = 4;

const PIXELTYPE PIXEL_INDEXED = 0;
const PIXELTYPE PIXEL_INDEXED_ALPHA = 1;

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((QUANTUM) (257UL*(value)))

#if 0
Q_INT32 imgTypeDepth(const enumImgType& type);
bool imgTypeHasAlpha(const enumImgType& type);
#endif

#define USEITERATORSFORPO 1

#endif // KISGLOBAL_H_

