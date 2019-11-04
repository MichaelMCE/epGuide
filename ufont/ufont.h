
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.

#ifndef _UFONT_H_
#define _UFONT_H_





#include "ufont_config.h"
#include "fio/fileio.h"


#if FONTS_LLATIN
#define UFDIR		"uf_Latin/"
#elif FONTS_DIGIITS
#define UFDIR		"uf_Digits/"
#elif FONTS_NOCJK
#define UFDIR		"uf_noCJK/"
#else
#define UFDIR		"uf/"
#endif






// RGB24 to RGB565
#define COLOUR_24TO16(c)		((uint16_t)(((((c)>>16)&0xF8)<<8) | ((((c)>>8)&0xFC)<<3) | (((c)&0xF8)>>3)))

// 4bit gray scale to RGB565
#define COLOUR_G4TO16(c)		((uint16_t)((((c)&0x0F)<<12) | (((c)&0x0F)<<7) | ((c)&0x0F)<<1))

// 8bit gray scale to RGB24
#define COLOUR_G8TO24(c)		((uint32_t)(((c)<<16) | ((c)<<8) | ((c)&0xFF)))

#define COLOUR_RED				RGB_16_RED
#define COLOUR_GREEN			RGB_16_GREEN
#define COLOUR_BLUE				RGB_16_BLUE
#define COLOUR_WHITE			RGB_16_WHITE
#define COLOUR_BLACK			RGB_16_BLACK
#define COLOUR_MAGENTA			RGB_16_MAGENTA
#define COLOUR_YELLOW			RGB_16_YELLOW
#define COLOUR_CYAN				RGB_16_CYAN
#define COLOUR_CREAM			COLOUR_24TO16(0xEEE7D0)
#define COLOUR_GREY				COLOUR_24TO16(0x111111 * 7)

#define BFONT_COLOUR_BOUNDRECT	COLOUR_RED
#define BFONT_COLOUR_GLYPHRECT	COLOUR_BLUE



#define BFONT_RENDER_GLYPHCLIP	0x0001			// render only if complete glyph is renderable within surface area, or clip at surface bound
#define BFONT_RENDER_ADVANCE_X	0x0002			// touch x
#define BFONT_RENDER_ADVANCE_Y	0x0004			// touch y
#define BFONT_RENDER_BOUNDRECT	0x0008			// draw box encompassing string area
#define BFONT_RENDER_GLYPHRECT	0x0010			// draw a box surrounding glyph(s)
#define BFONT_RENDER_FIXEDWIDTH	0x0020			// force monospacing for non monospaced fonts
#define BFONT_RENDER_NEWLINE	0x0040			// process \n (0x0A)
#define BFONT_RENDER_RETURN		0x0080			// process \r (0x0D)
#define BFONT_RENDER_COMBINED	0x0100			// render diacritical marks (combined chars) as combined or as individual glyphs
#define BFONT_RENDER_NODEFAULT	0x0200			// disable char replacement (missing)
#define BFONT_RENDER_CONDENSED	0x0400			// remove all space between glyphs. use this to set manual space and glyph padding
#define BFONT_RENDER_SKIPSPACE	0x0800			// advance pass leading white space. begin rendering at first now white space
#define BFONT_RENDER_METRICS	0x1000
#define BFONT_RENDER_CLIPFRONT	0x2000			// clip surface rendering to distination rect
#define BFONT_RENDER_WORDWRAP	0x4000
#define BFONT_RENDER_WWRETURN	0x8000			// wordwrap but newline to 0 on wrap-around

#define BFONT_RENDER_DEFAULT	(BFONT_RENDER_ADVANCE_Y)	// set compile time flag(s)
//#define BFONT_RENDER_SINGLELINE (~(BFONT_RENDER_NEWLINE|BFONT_RENDER_RETURN))




#define BFONT_PIXELOP_SET		1				// 1bit [glyph] pixel write mode
#define BFONT_PIXELOP_CLEAR		2
#define BFONT_PIXELOP_INVERT	3
#define BFONT_PIXELOP_DEFAULT	BFONT_PIXELOP_SET

#define BFONT_TAB_CHAR			L' '			// tab replacement character
#define BFONT_TAB_LENGTH		4				// replace tab character by this amount
#define BFONT_CHAR_SPACE		L' '			// [not implemented]. will define space replacement character. Unrelated to BFONT_TAB_CHAR


// header
#define BFONT_IDENT				0x1234			// version
#define BFONT_FLAGS_MONOSPACE	0x10			// is a fixed width font; 'i' is the same width as 'M'
#define BFONT_FLAGS_PROPSPACE	0x20			// glyphs are proportionaly spaced; 'i' is [usually] narrower than 'M'
#define BFONT_FLAGS_HASLOOKUP	0x40			// contains an encoding-to-file offset lookup table

#define BFONT_CACHE_SIZE		32
#define BFONT_EXT				L".uf"



#define SURFACE_PALLOC			0x01		// alloc'd .pixels should be freed with deconstructor
#define SURFACE_PALETTE_DIR		0x02		// Sets how the palette is applied to the surface. if set then horizontal, otherwise vertical
#define SURFACE_TEXCLAMP_H		0x10
#define SURFACE_TEXCLAMP_V		0x20

#define SURFACE_PALETTE_DIR_V	0
#define SURFACE_PALETTE_DIR_H	1


#define SURFACE_BPP_1			(1)
#define SURFACE_BPP_8			(8)
#define SURFACE_BPP_16			(16)


#define CALC_PITCH_1(w)			(((w)>>3)+(((w)&0x07)!=0))	// 1bit packed, calculate number of storage bytes per row given width (of glyph)
#define CALC_PITCH_16(w)		((w)*sizeof(uint16_t))		// 16bit (8 bits per byte)

#define DEG2RAD					(0.0174532925195)



// RGB 332
#define RGB_8_RED			(0xE0)
#define RGB_8_GREEN			(0x1C)
#define RGB_8_BLUE			(0x03)
#define RGB_8_WHITE			(RGB_8_RED|RGB_8_GREEN|RGB_8_BLUE)
#define RGB_8_BLACK			(0x00)
#define RGB_8_MAGENTA		(RGB_8_RED|RGB_8_BLUE)
#define RGB_8_YELLOW		(RGB_8_RED|RGB_8_GREEN)
#define RGB_8_CYAN			(RGB_8_GREEN|RGB_8_BLUE)

// RGB 444
#define RGB_12_RED			(0xF00)
#define RGB_12_GREEN		(0x0F0)
#define RGB_12_BLUE			(0x00F)
#define RGB_12_WHITE		(RGB_12_RED|RGB_12_GREEN|RGB_12_BLUE)
#define RGB_12_BLACK		(0x000
#define RGB_12_MAGENTA		(RGB_12_RED|RGB_12_BLUE)
#define RGB_12_YELLOW		(RGB_12_RED|RGB_12_GREEN)
#define RGB_12_CYAN			(RGB_12_GREEN|RGB_12_BLUE)

// RGB 555
#define RGB_15_RED			(0x7C00)
#define RGB_15_GREEN		(0x03E0)
#define RGB_15_BLUE			(0x001F)
#define RGB_15_WHITE		(RGB_15_RED|RGB_15_GREEN|RGB_15_BLUE)
#define RGB_15_BLACK		(0x0000)
#define RGB_15_MAGENTA		(RGB_15_RED|RGB_15_BLUE)
#define RGB_15_YELLOW		(RGB_15_RED|RGB_15_GREEN)
#define RGB_15_CYAN			(RGB_15_GREEN|RGB_15_BLUE)


// RGB 565
#define RGB_16_RED			(0xF800)
#define RGB_16_GREEN		(0x07E0)
#define RGB_16_BLUE			(0x001F)
#define RGB_16_WHITE		(RGB_16_RED|RGB_16_GREEN|RGB_16_BLUE)
#define RGB_16_BLACK		(0x0000)
#define RGB_16_MAGENTA		(RGB_16_RED|RGB_16_BLUE)
#define RGB_16_YELLOW		(RGB_16_RED|RGB_16_GREEN)
#define RGB_16_CYAN			(RGB_16_GREEN|RGB_16_BLUE)


// RGB 888
#define RGB_24_RED			(0xFF0000)
#define RGB_24_GREEN		(0x00FF00)
#define RGB_24_BLUE			(0x0000FF)	
#define RGB_24_WHITE		(RGB_24_RED|RGB_24_GREEN|RGB_24_BLUE)
#define RGB_24_BLACK		(0x000000)
#define RGB_24_MAGENTA		(RGB_24_RED|RGB_24_BLUE)
#define RGB_24_YELLOW		(RGB_24_RED|RGB_24_GREEN)
#define RGB_24_CYAN			(RGB_24_GREEN|RGB_24_BLUE)






#ifdef max
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifdef min
#undef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define clipInt5(x)\
	if ((x) > 0x1F) (x) = 0x1F;\
	else if ((x) < 0) (x) = 0

#define clipInt6(x)\
	if ((x) > 0x3F) (x) = 0x3F;\
	else if ((x) < 0) (x) = 0



typedef struct __attribute__((packed)){
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
}_rect_t;
	
typedef struct __attribute__((packed)){
	uint16_t ident;						// version ident

	struct __attribute__((packed)){
		uint16_t glyphs;				// glyphs in table, size of table == (.glyphs * sizeof(_lookup_t))
		uint32_t offset;				// location of table within file
	}lookup;
	
	uint16_t flags;						// general font flags (mono, proportional and the like)
	int16_t verticalPitch;				// BDF 'PIXEL_SIZE' field describing maximum rows, ie; height
	int16_t fontAscent;					// BDF 'FONT_ASCENT' field
	int16_t fontDescent;				// BDF 'FONT_DESCENT' field
	uint16_t defaultGlyph;				// replacement glyph when requested glyph is unavailable (unicode encoding).
	uint16_t totalGlyphs;				// number of glyphs (described) in font. is currently unused
	
	struct __attribute__((packed)){		// FontBoundingBox
		uint16_t width;					// quad width = max possible width of all char's
		uint16_t height;				// quad height = max possible height of all char's
		int8_t xOffset;	
		int8_t yOffset;
	}fbb;
}_ufont_header_t;

typedef struct __attribute__((packed)){
	uint8_t w;				// [bbx] width of this glyph		// this limits glyphs to a max width/height of 255/255
	uint8_t h;				// [bbx] height of this glyph
	int8_t xOffset;			// [bbx] x offset of glyph within frame
	int8_t yOffset;			// [bbx] y offset of glyph within frame
	int16_t dwidth;			// bdf 'DWIDTH' field (horizontal advance)
	int16_t length;			// length of pixel data (which follows this)
}_glyph_t;

typedef struct __attribute__((packed)){
	_glyph_t glyph;
	uint8_t *pixels;
	uint16_t encoding;
}_cache_glyph_t;

typedef struct __attribute__((packed)){
	uint32_t offset;		// location within file
	uint16_t encoding;		// unicode codepoint
}_lookup_t;


typedef	struct  __attribute__((packed)){
	uint8_t blockLoaded;		// current block in memory
	uint8_t blockTotal;
	uint16_t entriesPerBlock;
	
	uint32_t memSize;			// blockSize == maximum memory afforded to the lookup table
								// maximum indexes/entries = blockSize/sizeof(_lookup_t)

	_lookup_t *list;
	uint16_t listSize;			// size of list in bytes, as passed to malloc()

	uint16_t from;				// when read to memory, this table references complete lookup table index 'from'
	uint16_t to;				// to index 'to'
	uint16_t total;				// actual number of entries in table
}_table_t;


typedef struct /*__attribute__((packed))*/{
	uint8_t *pixels;
	uint32_t size;
	
	uint16_t width;
	uint16_t height;
	
	uint16_t colour;
	uint8_t flags8;
	uint8_t bpp;
}_ufont_surface_t;

#define PALETTE_PALLOC				0x0001		// alloc'd .colours should be freed within deconstructor

typedef struct {
	uint16_t *colours;
	uint16_t total;
	uint16_t flags;
}_ufont_palette_t;


//typedef void (*pvid_t) (void *ptr, int x, int y);

typedef struct __attribute__((packed)){
	_ufont_header_t header;

	struct {
		_cache_glyph_t list[BFONT_CACHE_SIZE];		// glyph cache
		uint16_t cachePosition;
		uint16_t flags;
		uint32_t defaultGlyphPosition;
	}glyphs;

	_table_t table;
	
	struct {
		int8_t spaceGlyphPadding;	// width of space character (which are often poorly defined)
		int8_t glyphPadding;		// extra (horizontal) padding added to every glyph
		int8_t linePadding;			// vertical padding added to line wrap around
		int8_t padding;				// use 8bit to keep allignment
		
		uint16_t flags;				// 
		uint16_t pixelOp;			//	whether to set (default), clear or XOR target pixel

		_ufont_surface_t *surface;
	}render;
	
	struct {
		_ufont_surface_t surface;
		_rect_t clip;				// clip display surface rendering to within this rect
	}display;
	
	
#if NO_FIO
	FILE *fp;
#else
	fileio_t *fio;
#endif
}_ufont_t;


typedef struct {
	struct {			// initiate rendering here
		int16_t x;		// this is where we want to print our character
		int16_t y;		// upper left
	}start;
#if ENABLE_COMBINED
	struct {			// 
		int16_t x;		// 
		int16_t y;		// 
	}previous;
#endif	
	struct {			// begin actual render here
		int16_t x;		// this is where the character was actually printed when factoring offsets
		int16_t y;		// upper left
	}begin;
	
	struct {			// rendering completed here:
		int16_t x;		// lower right
		int16_t y;
	}end;
	
	/*struct {
		int16_t width;
		int16_t height;
	}metrics;*/
}glyph_render_t;


int fontOpen (_ufont_t *font, const char *file);
void fontClose (_ufont_t *font);
int fontReadHeader (_ufont_t *font, _ufont_header_t *header);
void fontCleanCache (_ufont_t *font);

void fontSetDisplayBuffer (_ufont_t *font, void *pixels, const int width, const int height);
_ufont_surface_t *fontGetDisplayBuffer (_ufont_t *font);

_ufont_surface_t *fontCreateSurface (const int width, const int height, const uint16_t colour, uint8_t *pixels);
void fontSurfaceFree (_ufont_surface_t *surface);

_ufont_surface_t *fontGenerateTexture (const int width, const int height, const void *pixels);
_ufont_surface_t *fontCreateTexture (const char *path);
void fontTextureFree (_ufont_surface_t *surface);
void fontTextureSetClamp (_ufont_surface_t *surface, const uint16_t dir);


_ufont_palette_t *fontPaletteCreate (void *pixels, const uint16_t total);
_ufont_palette_t *fontPaletteLoad (const char *path);
void fontPaletteFree (_ufont_palette_t *pal);
void fontPaletteSetColour (_ufont_palette_t *pal, const uint16_t idx, const uint16_t colour);
uint16_t fontPaletteGetColour (_ufont_palette_t *pal, const uint16_t idx);


void fontApplySurface (_ufont_t *font, int startX, int startY);
void fontApplySurfaceOutline (_ufont_t *font, int startX, int startY);
void fontApplySurfaceOutlineEx (_ufont_t *font, int startX, int startY, const int thickness);
void fontApplySurfacePalette (_ufont_t *font, _ufont_palette_t *pal, int startX, int startY);
void fontApplySurfacePaletteEx (_ufont_t *font, _ufont_palette_t *pal, int bandStart, int bandLength, int startX, int startY);
void fontApplySurfaceTexture (_ufont_t *font, const _ufont_surface_t *const tex, int startX, int startY);
void fontApplySurfaceTextureEx (_ufont_t *font, const _ufont_surface_t *const tex, int startX, int startY, const int effect, uint32_t time1);
void fontApplySurfaceSprite (_ufont_t *font, _ufont_surface_t *tex, const uint16_t maskCol, uint8_t texStrideX, uint8_t texStrideY, int startX, int startY);
void fontApplySurfaceMask (_ufont_t *font, _ufont_surface_t *mask, int startX, int startY);



void fontSetRenderColour (_ufont_surface_t *surface, const uint16_t colour);
void fontSetPaletteAxis (_ufont_surface_t *surface, const uint16_t dir);
void fontSetRenderSurface (_ufont_t *font, _ufont_surface_t *surface);

// clear render surface attached to font, or if NULL, clean passed surface
void fontCleanSurface (_ufont_t *font, _ufont_surface_t *surface);


int fontPrint (_ufont_t *font, int *desX, int *desY, const uint8_t *str);
int fontPrint8 (_ufont_t *font, int *desX, int *desY, const uint8_t *str);
int fontPrintList (_ufont_t *font, int *desX, int *desY, const uint16_t *list);
#define fontPrintW(a,b,c,d) fontPrintList((a),(b),(c),(const uint16_t*)(d))


int fontGetMetrics (_ufont_t *font, const uint8_t *str, int *width, int *height);
int fontGetMetrics8 (_ufont_t *font, const uint8_t *str, int *width, int *height);
int fontGetMetricsList (_ufont_t *font, const uint16_t *list, int *width, int *height);
#define fontGetMetricsW(a,b,c,d) fontGetMetricsList((a),(const uint16_t*)(b),(c),(d))

uint16_t fontSetRenderFlags (_ufont_t *font, const uint16_t flags);
uint16_t fontGetRenderFlags (_ufont_t *font);


// set how the glyph is rendered to the buffer
int8_t fontSetWriteOp (_ufont_t *font, const int8_t pixelOp);
int8_t fontGetWriteOp (_ufont_t *font);

// space between rows of glyphs
int8_t fontGetLineSpace (_ufont_t *font);
int8_t fontSetLineSpace (_ufont_t *font, const int8_t pixels);

// horizontal space between glyphs
int8_t fontGetGlyphPadding (_ufont_t *font);
int8_t fontSetGlyphPadding (_ufont_t *font, const int8_t pixels);


// get copy of glyph header
// if not already glyph is added to cache
// returns 0 on failure/glyph not found, otherwise returns glyph fileOffset
uint32_t fontGetGlyphHeader (_ufont_t *font, const uint16_t codepoint, _glyph_t *glyph);

// similar to fontGetGlyphHeader() except cache is untouched.
// returns 1 on success, 0 otherwise
int metricsGetGlyphHeader (_ufont_t *font, const uint16_t codepoint, _glyph_t *glyph);


// as with fontGetGlyphHeader
// returns internal pointer to pixel data (cache) of size glyph.length
// pointer is invalidated upon fontCleanCache()
uint8_t *fontGetGlyphPixels (_ufont_t *font, const uint16_t codepoint);





#define fontGetWidth(a)		(((_ufont_surface_t*)a)->width)
#define fontGetHeight(a)	(((_ufont_surface_t*)a)->height)

#endif
