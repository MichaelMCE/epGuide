
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

//#pragma GCC optimize ("-O2")


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <ctype.h>
#include <wtypes.h>
#include <math.h>

#include "pngcommon.h"
#include "ufont.h"
#include "ufont_primitives.h"
#if ENABLE_CHARREFS
#include "entities.h"
#endif


// target display type
static int DWIDTH = 0;
static int DHEIGHT = 0;







static inline void *uf_malloc (size_t size/*, const uint8_t *func*/)
{
	return malloc(size);
}

static inline void *uf_calloc (size_t nelem, size_t elsize/*, const uint8_t *func*/)
{
	return calloc(nelem, elsize);
}

static inline void uf_free (void *ptr/*, const uint8_t *func*/)
{
	free(ptr);
}

static inline int UTF8ToUTF32 (uint8_t *buffer, uint32_t *wc)
{
	
	uint32_t n;

	// guess length of multi-char
	for (n = 0; n < 7; n++){
		if (!buffer[n])
			break;
	}

	uint8_t *s = (uint8_t*)buffer;
	const uint8_t c = s[0];
	const uint32_t invalidChar = '?';
	
	if (c < 0x80){
		*wc = c;
		return 1;
	}else if (c < 0xc2){
		*wc = invalidChar;
		return 1;

	}else if (c < 0xe0){
		if (n < 2){
			*wc = invalidChar;
			return 1;
		}
			
		if (!((s[1] ^ 0x80) < 0x40)){
			*wc = invalidChar;
			if (s[1]&0x80)
				return 2;
			else
				return 1;
		}
			
		*wc = ((uint32_t)(c & 0x1f) << 6) | (uint32_t)(s[1] ^ 0x80);
		return 2;
	}else if (c < 0xf0){
		if (n < 3){
			*wc = invalidChar;
			return 2;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0))){
			*wc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80)
					return 3;
				else
					return 2;
			}else{
				return 1;
			}
		}

		*wc = ((uint32_t)(c & 0x0f) << 12) | ((uint32_t)(s[1] ^ 0x80) << 6) | (uint32_t)(s[2] ^ 0x80);
		return 3;
	}else if (c < 0xf8){
		if (n < 4){
			*wc = invalidChar;
			return 3;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90))){
			*wc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80)
						return 4;
					else
						return 3;
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
			
		*wc = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(s[1] ^ 0x80) << 12) | ((uint32_t)(s[2] ^ 0x80) << 6) | (uint32_t)(s[3] ^ 0x80);
		return 4;
	}else if (c < 0xfc){
		if (n < 5){
			*wc = invalidChar;
			return 4;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88))){
			*wc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							return 5;
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*wc = ((uint32_t)(c & 0x03) << 24) | ((uint32_t)(s[1] ^ 0x80) << 18) | ((uint32_t)(s[2] ^ 0x80) << 12) | ((uint32_t)(s[3] ^ 0x80) << 6) | (uint32_t)(s[4] ^ 0x80);
		return 5;
	}else if (c < 0xfe){
		if (n < 6){
			*wc = invalidChar;
			return 5;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84))){
			*wc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							if (s[5]&0x80){
								return 6;
							}else{
								return 5;
							}
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*wc = ((uint32_t)(c & 0x01) << 30) | ((uint32_t)(s[1] ^ 0x80) << 24) | ((uint32_t)(s[2] ^ 0x80) << 18) | ((uint32_t)(s[3] ^ 0x80) << 12) | ((uint32_t)(s[4] ^ 0x80) << 6) | (uint32_t)(s[5] ^ 0x80);
		return 6;
	}else{
		*wc = invalidChar;
		return 1;
	}
}


#if 0
void clearFileCache (const uint8_t *drive)
{
	uint8_t *dosprefix = "\\\\.\\";
	uint8_t *path = (char*)uf_calloc(strlen(drive) + strlen(dosprefix) + 1, strlen(drive));
	if (!path) return;

	if (isalpha(drive[0]) && drive[1] == ':' && ((drive[2] == '\0' || drive[2] == '\\') && drive[3] == '\0')){
		strcat(path, dosprefix);
	}
	strcat(path, drive);
            
	HANDLE hFile = CreateFile(path, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE){
		CloseHandle(hFile);
	}else {
		int le = (int)GetLastError();
		if (le != ERROR_SHARING_VIOLATION && le != ERROR_ACCESS_DENIED){
			printf("Error %d clearing %s\n", le, drive);
		}
	}
	uf_free(path);
}
#endif



// clip rect to the target display space
static inline void bindRectangle (_ufont_surface_t *surface, int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 > surface->width-1)
		*x1 = surface->width-1;
	else if (*x1 < 0)
		*x1 = 0;
		
	if (*x2 > surface->width-1)
		*x2 = surface->width-1;
	else if (*x2 < 0)
		*x2 = 0;

	if (*y1 > surface->height-1)
		*y1 = surface->height-1;
	else if (*y1 < 0)
		*y1 = 0;
		
	if (*y2 > surface->height-1)
		*y2 = surface->height-1;
	else if (*y2 < 0)
		*y2 = 0;
}

// clip rect to display space
static inline void bindPoint (int *x, int *y)
{
	if (*x > DWIDTH-1)
		*x = DWIDTH-1;
	else if (*x < 0)
		*x = 0;

	if (*y > DHEIGHT-1)
		*y = DHEIGHT-1;
	else if (*y < 0)
		*y = 0;
}

void fontSetRenderColour (_ufont_surface_t *surface, const uint16_t colour)
{
	surface->colour = colour;
}

void fontSetRenderSurface (_ufont_t *font, _ufont_surface_t *surface)
{
	font->render.surface = surface;
}

static inline _ufont_surface_t *fontGetRenderSurface (_ufont_t *font)
{
	return font->render.surface;
}

void fontCleanSurface (_ufont_t *font, _ufont_surface_t *surface)
{
	if (font)
		surface = fontGetRenderSurface(font);
	if (surface)
		memset(surface->pixels, 0, surface->size);
}

void fontApplySurface (_ufont_t *font, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);

	bindRectangle(front, &startX, &startY, &endX, &endY);
	
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		if (*((uint64_t*)clip) != 0){		// sanity check
		
			if (startX < clip->x1){
				surfaceX += (clip->x1 - startX);
				if (endX < clip->x1) return;
				startX = clip->x1;
			}

			if (endX > clip->x2) endX = clip->x2;
	
			if (startY < clip->y1){
				surfaceY += (clip->y1 - startY);
				if (endY < clip->y1) return;
				startY = clip->y1;
			}	
	
			if (endY > clip->y2) endY = clip->y2;
		}
	}
	endX++; endY++;


	int y1 = surfaceY;
	for (int y = startY; y < endY; y++, y1++){
		int x1 = surfaceX;
		for (int x = startX; x < endX; x++, x1++){
			if (getPixel1(srcPixels, srcPitch, x1, y1))
				setPixel16_nb(front, x, y, surface->colour);
		}
	}
}

void fontApplySurfaceMask (_ufont_t *font, _ufont_surface_t *mask, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	uint8_t *mskPixels = mask->pixels;
	const int mskPitch = CALC_PITCH_1(mask->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;

	int y1 = 0;	
	for (int y = startY; y < endY; y++, y1++){
		int x1 = surfaceX;
		for (int x = startX; x < endX; x++, x1++){
			if (getPixel1(srcPixels, srcPitch, x1, y1) && getPixel1(mskPixels, mskPitch, x1, y1))
				setPixel16_nb(front, x, y, surface->colour);
		}
	}
}

void fontApplySurfaceOutline (_ufont_t *font, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);

	bindRectangle(front, &startX, &startY, &endX, &endY);
	//printf("%i %i %i %i\n", startX, startY, endX, endY);
	
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		if (*((uint64_t*)clip) != 0){		// sanity check
			if (startX < clip->x1-1){
				surfaceX += (clip->x1 - startX);
				if (endX < clip->x1-1) return;
				startX = clip->x1;
			}

			if (endX > clip->x2+1) endX = clip->x2;

			if (startY < clip->y1-1){
				surfaceY += (clip->y1 - startY);
				if (endY < clip->y1-1) return;
				startY = clip->y1;
			}	
	
			if (endY > clip->y2+1) endY = clip->y2;
		}
	}
	
	endX++; endY++;

	int32_t z = 1;
	int y1 = surfaceY;
	
	for (int y = startY; y < endY; y++, y1++){
		int setState = 0;
		int x1 = surfaceX;
		
		for (int x = startX; x < endX; x++, x1++){
			if (getPixel1(srcPixels, srcPitch, x1, y1)){
				if (!setState){
					setState = 0xFF;
					setPixel16_bc(front, x-z, y, surface->colour);
				}
			}else{
				if (setState){
					setState = 0;
					setPixel16_bc(front, x, y, surface->colour);
				}
			}
		} 
	}
	
	
	int x1 = surfaceX;
	for (int x = startX; x < endX; x++, x1++){
		int setState = 0;
		int y1 = surfaceY;
		
		for (int y = startY; y < endY; y++, y1++){
			if (getPixel1(srcPixels, srcPitch, x1, y1)){
				if (!setState){
					setState = 0xFF;
					setPixel16_bc(front, x, y-z, surface->colour);
				}
			}else{
				if (setState){
					setState = 0;
					setPixel16_bc(front, x, y, surface->colour);
				}
			}
		} 
	}
}

#if 0
void _fontApplySurfaceOutline (_ufont_t *font, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	int32_t srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	//bindPoint(&startX, &startY);
	

	
	int fromX = 0;
	int endX = surface->width;
	
	if (startX < 100){
		fromX += (100 - startX);
		if (endX < 100) return;
		startX = 100;
	}
		
	
	//printf("%i %i\n", startX, surface->width);
	
	int32_t z = 1;
	int32_t setState = 0;
	
	for (int y = 0; y < surface->height; y++){
		setState = 0;
		
		int x1 = fromX;
		for (int x = 0; x < endX; x++, x1++){
			if (getPixel1(srcPixels, srcPitch, x1, y)){
				if (!setState){
					setState = 0xFF;
					setPixel16_bc(front, startX+(x-z), startY+y, surface->colour);
				}
			}else{
				if (setState){
					setState = 0;
					setPixel16_bc(front, startX+x, startY+y, surface->colour);
				}
			}
		}
	}

	return;

#if 0
	for (int x = 0; x < surface->width; x += 32){
		uint32_t mask = 0;
		
		for (int y = 0; y < surface->height; y++){
			uint32_t pixels32 = getPixels32(srcPixels, srcPitch, x, y);
			int32_t diff = mask^pixels32;
			
			if (diff){
				for (int32_t p = 0; p < 32; p++){
					if (diff&0x01){
						z = (pixels32>>p)&0x01;
						setPixel16_bc(front, startX+x+p, startY+(y-z), surface->colour);
					}
					diff >>= 1;
				}
				mask = pixels32;
			}
		}
	}
	
#elif 0

	for (int32_t x = fromX; x < endX; x += 8){
		uint8_t mask = 0;
		
		for (int32_t y = 0; y < surface->height; y++){
			uint8_t pixels8 = getPixels8(srcPixels, srcPitch, x, y);
			int32_t diff = pixels8 ^ mask;
			
			if (diff){
				for (int32_t p = 0; p < 8; p++){
					if (diff&0x01){
						z = (pixels8>>p)&0x01;
						setPixel16_bc(front, startX+x+p, startY+(y-z), surface->colour);
					}
					diff >>= 1;
				}
				mask = pixels8;
			}
		}
	}
	
#elif 0	
//#else
	for (int32_t x = fromX; x < surface->width; x++){
		setState = 0;
		
		for (int32_t y = 0; y < surface->height; y++){
			if (getPixel1(srcPixels, srcPitch, x, y)){
				if (!setState){
					setState = 0xFF;
					setPixel16_bc(front, startX+x, startY+(y-z), surface->colour);
				}
			}else{
				if (setState){
					setState = 0;
					setPixel16_bc(front, startX+x, startY+y, surface->colour);
				}
			}
		}
	}
#endif
}
#endif



// cliping isn't complete
void fontApplySurfaceOutlineEx (_ufont_t *font, int startX, int startY, const int z)
{

	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1-1){
			surfaceX += (clip->x1-1 - startX);
			if (endX < clip->x1-1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2+1) endX = clip->x2+1;

		if (startY < clip->y1-1){
			surfaceY += (clip->y1-1 - startY);
			if (endY < clip->y1-1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2+1) endY = clip->y2+1;
	}
	endX++; endY++;

	_ufont_surface_t *surfaceTmp = fontCreateSurface(surface->width+z+z, surface->height+z+z, surface->colour, NULL);
	if (!surfaceTmp) return;
	
	uint8_t *srcPixelsTmp = surfaceTmp->pixels;
	const int pitchTmp = CALC_PITCH_1(surfaceTmp->width);
	const int o = z;

	for (int y = 0; y < surface->height; y++){
		for (int x = 0; x < surface->width; x++){
			if (getPixel1(srcPixels, srcPitch, x, y)){
				setPixel1_nb(surfaceTmp, pitchTmp, o+(x-z), o+(y-z));		// NW
				//setPixel1_bc(surfaceTmp, pitchTmp, o+x, o+(y-z));			// N
				setPixel1_nb(surfaceTmp, pitchTmp, o+(x+z), o+(y-z));		// NE
				
				//setPixel1_bc(surfaceTmp, pitchTmp, o+(x-z), o+y);			// W
				//setPixel1_bc(surfaceTmp, pitchTmp, o+x, o+y);				// C
				//setPixel1_bc(surfaceTmp, pitchTmp, o+(x+z), o+y);			// E

				setPixel1_nb(surfaceTmp, pitchTmp, o+(x-z), o+(y+z));		// SW
				//setPixel1_bc(surfaceTmp, pitchTmp, o+x, o+(y+z));			// S
				setPixel1_nb(surfaceTmp, pitchTmp, o+(x+z), o+(y+z));		// SE
			}
		}
	}

	int y1 = 0;
	for (int y = startY; y < endY; y++, y1++){
		int x1 = surfaceX;

		for (int x = startX; x < endX; x++, x1++){
			if (getPixel1(srcPixelsTmp, pitchTmp, x1+o, y1+o) && !getPixel1(srcPixels, srcPitch, x1, y1))
				setPixel16_nb(front, x, y, surface->colour);
		}
	}

	fontSurfaceFree(surfaceTmp);
}

void fontTextureSetClamp (_ufont_surface_t *surface, const uint16_t dir)
{
	surface->flags8 &= ~(SURFACE_TEXCLAMP_H|SURFACE_TEXCLAMP_V);
	surface->flags8 |= dir&(SURFACE_TEXCLAMP_H|SURFACE_TEXCLAMP_V);
}

void fontSetPaletteAxis (_ufont_surface_t *surface, const uint16_t dir)
{
	surface->flags8 &= ~SURFACE_PALETTE_DIR;
	surface->flags8 |= ((dir&0x01)<<1);
}

void fontPaletteSetColour (_ufont_palette_t *pal, const uint16_t idx, const uint16_t colour)
{
	if (idx < pal->total)
		pal->colours[idx] = colour;
}

uint16_t fontPaletteGetColour (_ufont_palette_t *pal, const uint16_t idx)
{
	if (idx < pal->total)
		return pal->colours[idx];
		
	return 0;
}

void fontPaletteFree (_ufont_palette_t *pal)
{
	if (pal){
		if (pal->flags&PALETTE_PALLOC)
			uf_free(pal->colours);
		uf_free(pal);
	}
}

_ufont_palette_t *fontPaletteCreate (void *pixels, const uint16_t total)
{
	_ufont_palette_t *pal = (_ufont_palette_t*)uf_malloc(sizeof(_ufont_palette_t));
	if (pal){
		pal->total = total;
		pal->colours = (uint16_t*)pixels;
		pal->flags = 0;
	}
	return pal;
}

_ufont_palette_t *fontPaletteLoad (const char *path)
{
	uint32_t width, height, bpc;
	if (png_metrics(path, &width, &height, &bpc)){
		uint8_t *pixels = (uint8_t*)uf_malloc(height * CALC_PITCH_16(width));
		if (!pixels)
			return NULL;

		if (!png_read(path, pixels, BPP_16, 0, 0))
			return NULL;
				
		int len = max(width, height);
		_ufont_palette_t *pal = (_ufont_palette_t*)uf_malloc(sizeof(_ufont_palette_t));
		if (pal){
			pal->total = len;
			pal->colours = (uint16_t*)uf_malloc(len * sizeof(uint16_t));
			if (!pal->colours){
				uf_free(pal);
				uf_free(pixels);
				return NULL;
			}
			pal->flags |= PALETTE_PALLOC;
		
			int pitch = CALC_PITCH_16(width);
			//uint16_t *pixels = (uint16_t*)tex->pixels;
		
			if (width > height){
				for (int i = 0; i < len; i++)
					pal->colours[i] = getPixel16(pixels, pitch, i, 0);
			}else{
				for (int i = 0; i < len; i++)
					pal->colours[i] = getPixel16(pixels, pitch, 0, i);
			}
			return pal;
		}	
	}

	return NULL;
}


void fontApplySurfacePalette (_ufont_t *font, _ufont_palette_t *pal, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;
	
	const int len = pal->total;
	int y1 = 0;
			
	if (surface->flags8&SURFACE_PALETTE_DIR){
		for (int y = startY; y < endY; y++, y1++){
			int x1 = surfaceX;
	
			for (int x = startX; x < endX; x++, x1++){
				if (getPixel1(srcPixels, srcPitch, x1, y1))
					setPixel16_nb(front, x, y, pal->colours[x%len]);
			}
		}
	}else{
		for (int y = startY; y < endY; y++, y1++){
			const uint16_t col = pal->colours[y%len];
			int x1 = surfaceX;
	
			for (int x = startX; x < endX; x++, x1++){
				if (getPixel1(srcPixels, srcPitch, x1, y1))
					setPixel16_nb(front, x, y, col);
			}
		}
	}
}

void fontApplySurfacePaletteEx (_ufont_t *font, _ufont_palette_t *pal, int bandStart, int bandLength, int startX, int startY)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;
	
	const int len = min (pal->total, bandLength);
	int bandX = 0;
	int y1 = 0;
			
	if (surface->flags8&SURFACE_PALETTE_DIR){
		for (int y = startY; y < endY; y++, y1++){
			int x1 = surfaceX;
			bandX = 0;
			
			for (int x = startX; x < endX; x++, x1++){
				if (getPixel1(srcPixels, srcPitch, x1, y1))
					setPixel16_nb(front, x, y, pal->colours[bandStart+bandX]);
				bandX = (bandX+1) % len;
			}
		}
	}else{
		for (int y = startY; y < endY; y++, y1++){
			bandX = (bandX+1) % len;
			const uint16_t col = pal->colours[bandStart+bandX];
			int x1 = surfaceX;
	
			for (int x = startX; x < endX; x++, x1++){
				if (getPixel1(srcPixels, srcPitch, x1, y1))
					setPixel16_nb(front, x, y, col);
			}
			
		}
	}
}

// cliping may not actually function
void fontApplySurfaceTextureEx (_ufont_t *font, const _ufont_surface_t *const tex, int startX, int startY, const int effect, uint32_t time1)
{

	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);

	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;
	
	const int texW = tex->width;
	const int texH = tex->height;
	const int texPitch = CALC_PITCH_16(tex->width);
	uint8_t *texPixels = tex->pixels;
	
	const float sheight = surface->height;
	const float swidth = surface->width;
	int32_t bright = 0;

	float u = 1.0f; float v = 1.0f;
	float a = 1.0f; float r = 1.0f;
					
	time1 += 10000;
    const uint32_t t0 = 0.05f * time1;
	int y1 = 0;

	
#define intro																		\
	for (int y = startY; y < endY; y++, y1++){										\
		float yy = -1.00f + 2.00f*(float)y/sheight;									\
		int x1 = surfaceX;															\
																					\
		for (int x = startX; x < endX; x++, x1++){									\
			if (getPixel1(srcPixels, srcPitch, x1, y1)){							\
				float xx = -1.00f + 2.00f*(float)x/swidth;							\


#define outtro																		\
				int xr = (t0 + (int)((float)texW * u)) % texW;						\
				int yr = (t0 + (int)((float)texH * v)) % texH;						\
																					\
				if (bright){														\
					uint16_t col = getPixel16(texPixels, texPitch, xr, yr);			\
																					\
					int r = (col>>11);												\
					r += bright>>1;													\
					clipInt5(r);													\
																					\
					int g = (col>>5)&0x3F;											\
					g += bright;													\
					clipInt6(g);													\
																					\
					int b = col&0x1F;												\
					b += bright>>1;													\
					clipInt5(b);													\
																					\
					setPixel16_nb(front, x, y, (r<<11)|(g<<5)|b);					\
				}else{																\
					uint16_t col = getPixel16(texPixels, texPitch, xr, yr);			\
					setPixel16_nb(front, x, y, col);								\
				}																	\
			}																		\
		}																			\
	}																				\


	float pipi = M_PI;
	
	switch (effect){
	  case 0:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);
			u = a*3.0f/pipi;
			v = 2.0f / (6.0f*r + 3.0f*xx);
			bright = 32.0f * -v;
		}outtro
		break;
		
	  case 1:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);
			u = cosf(a)/r;
			v = sinf(a)/r;
			bright = -10.0f * (2.0f/(6.0f*r + 3.0f*xx));
		}outtro
		break;
		
	  case 2:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			u = xx*cosf(2.0f*r) - yy*sinf(2.0f*r);
			v = yy*cosf(2.0f*r) + xx*sinf(2.0f*r);
		}outtro
		break;
		
	  case 3:
		intro{
			a = atan2f(yy, xx) * DEG2RAD;
			u = xx*cosf(a) - yy*sinf(a);
			v = xx*sinf(a) + yy*cosf(a);
		}outtro
		break;
		
	  case 4:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);
			u = 0.02f*yy+0.03f*cosf(a*3.0f)/r;
			v = 0.02f*xx+0.03f*sinf(a*3.0f)/r;
		}outtro
		break;
		
	  case 5:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			u = (0.1f * xx) / (0.11f + r*0.5f);
			v = (0.1f * yy) / (0.11f + r*0.5f);
		}outtro
		break;
		
	  case 6:
		intro{
			a = atan2f(yy, xx);
			u = 0.5f * a/pipi;
			r = sqrtf(xx*xx + yy*yy);
			v = sinf(7.0f*r);
		}outtro
		break;
		
	  case 7:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);
			u = r * cosf(a+r);
			v = r * sinf(a+r);
		}outtro
		break;
		
	  case 8:
		intro{
			a = atan2f(yy, xx);
			u = 0.5f * a/pipi;
			r = sqrtf(xx*xx + yy*yy);
			v = sinf(2.0f * r);
		}outtro
		break;
		
	  case 9:	
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);	
			u = 1.0f/(r+0.5f+0.5f*sinf(5.0f*a));
			v = a*3.0f/pipi;
		}outtro
		break;
		
	  case 10:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			a = atan2f(yy, xx);
			v = powf(r,0.1f);
			u = (1.0f*a/pipi)+r;
		}outtro
		break;
		
	  case 11:
		intro{
			r = sqrtf(xx*xx + yy*yy);
			v = xx*(3.0f-sqrtf(4.0f-5.0f*r*r))/(r*r+1.0f);
			u = yy*(3.0f-sqrtf(4.0f-5.0f*r*r))/(r*r+1.0f);
			bright = 2.0f * -18.7f * (xx+yy+r*r-(xx+yy-1.0f)*sqrtf(4.0f-5.0f*r*r)/3.0f)/(r*r+1.0f);
		}outtro
		break;
	
	  /*default:
		v = xx;
		u = yy;*/
	}	
}


static inline void drawSprite (_ufont_surface_t *surface, _ufont_surface_t *sprite, const int cx, const int cy, const uint16_t mask)
{
	int dx = cx - (sprite->width/2);
	int dy = cy - (sprite->height/2);
	const int texPitch = CALC_PITCH_16(sprite->width);
	
	for (int y = 0; y < sprite->height; y++, dy++){
		int _dx = dx;
		
		for (int x = 0; x < sprite->width; x++, _dx++){
			uint16_t col = getPixel16(sprite->pixels, texPitch, x, y);
			if (col != mask)
				setPixel16_bc(surface, _dx, dy, col);
		}
	}
}

void fontApplySurfaceSprite (_ufont_t *font, _ufont_surface_t *tex, const uint16_t mask, uint8_t strideX, uint8_t strideY, int startX, int startY)
{

	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	if (!strideX) strideX = 1;
	if (!strideY) strideY = 1;
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);


	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;

	int y1 = 0;
	for (int y = startY; y < endY; y+=strideY, y1+=strideY){
		int x1 = surfaceX;

		for (int x = startX; x < endX; x+=strideX, x1+=strideX){
			if (getPixel1(srcPixels, srcPitch, x1, y1))
				drawSprite(front, tex, x, y, mask);
		}
	}
}

void fontApplySurfaceTexture (_ufont_t *font, const _ufont_surface_t *const tex, int startX, int startY)
{

	_ufont_surface_t *surface = fontGetRenderSurface(font);
	
	uint8_t *srcPixels = surface->pixels;
	const int srcPitch = CALC_PITCH_1(surface->width);
	_ufont_surface_t *front = fontGetDisplayBuffer(font);
	
	int endX = startX + min(front->width, surface->width)-1;
	if (startX + surface->width-1 > front->width-1)
		endX -= front->width - (startX + surface->width);
	int endY = startY + min(front->height, surface->height)-1;
	if (startY + surface->height-1 > front->height-1)
		endY -= front->height - (startY + surface->height);


	int surfaceX = 0;
	if (startX < 0) surfaceX = abs(startX);
	int surfaceY = 0;
	if (startY < 0) surfaceY = abs(startY);
	
	bindRectangle(front, &startX, &startY, &endX, &endY);
	if (font->render.flags&BFONT_RENDER_CLIPFRONT){
		_rect_t *clip = &font->display.clip;
		
		if (startX < clip->x1){
			surfaceX += (clip->x1 - startX);
			if (endX < clip->x1) return;
			startX = clip->x1;
		}

		if (endX > clip->x2) endX = clip->x2;

		if (startY < clip->y1){
			surfaceY += (clip->y1 - startY);
			if (endY < clip->y1) return;
			startY = clip->y1;
		}	
	
		if (endY > clip->y2) endY = clip->y2;
	}
	endX++; endY++;

	int tx = 0, ty = 0;
	if (surface->flags8&SURFACE_TEXCLAMP_H) tx = startX;
	if (surface->flags8&SURFACE_TEXCLAMP_V) ty = startY;

	const int texW = tex->width;
	const int texH = tex->height;
	const int texPitch = CALC_PITCH_16(tex->width);
	uint8_t *texPixels = tex->pixels;

	int y1 = 0;
	for (int y = startY; y < endY; y++, y1++){
		int x1 = surfaceX;
		int dx = tx + startX;
		int dy = ty + y;
		
		for (int x = startX; x < endX; x++, x1++, dx++){
			if (getPixel1(srcPixels, srcPitch, x1, y1)){	
				uint16_t col = getPixel16(texPixels, texPitch, dx%texW, dy%texH);
				setPixel16_nb(front, x, y, col);
			}
		}
	}
}

static inline int renderGlyph (_ufont_t *font, _glyph_t *glyph, uint8_t *pixels, glyph_render_t *render)
{
	const int pitch1 = CALC_PITCH_1(glyph->w);
	int posX2 = 0;
	int xOffset = 0;
	int boxWidth = 0;


	if (font->header.flags&BFONT_FLAGS_MONOSPACE || font->render.flags&BFONT_RENDER_FIXEDWIDTH){
		boxWidth = 0;
		xOffset = max((font->header.fbb.xOffset - glyph->xOffset), 0);
	}else{
		boxWidth = font->header.fbb.width;
		xOffset = boxWidth + glyph->xOffset;
	}

	if (font->header.flags&BFONT_FLAGS_PROPSPACE || font->header.flags&BFONT_FLAGS_MONOSPACE)
		posX2 = render->start.x + xOffset;
	else
		posX2 = render->start.x + font->header.fbb.xOffset - glyph->xOffset;


	int maxWidth = DWIDTH;
	int maxHeight = DHEIGHT;
	
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		maxWidth = surface->width;
		maxHeight = surface->height;
	}
	
	int h = glyph->h;
	int oy = render->start.y + (font->header.verticalPitch - glyph->h - glyph->yOffset);

	if (oy + h >= maxHeight)
		h = max(glyph->h - ((oy + h) - maxHeight), 0);
	
	int w = glyph->w;
	if ((posX2-boxWidth) + w >= maxWidth)
		w = max(glyph->w - (((posX2-boxWidth)+w) - maxWidth), 0);


	int deltaY = 0;
	if (oy < 0) deltaY = -oy;

	int deltaX = 0;
	if (posX2-boxWidth < 0) deltaX = -(posX2-boxWidth);

	render->begin.x = posX2;
	render->end.x = (render->begin.x + w-1) - boxWidth;
	render->begin.y = oy + deltaY;
	render->end.y = oy + (h-1);
	//render->metrics.width = w;
	//render->metrics.height = h;

	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		if (font->render.flags&BFONT_RENDER_GLYPHRECT)
			surfaceDrawRectangle(&font->display.surface, render->begin.x-boxWidth, render->begin.y, render->end.x, render->end.y, BFONT_COLOUR_GLYPHRECT);
		
		const int32_t pitch = CALC_PITCH_1(surface->width);
		posX2 -= boxWidth;
		
		if (font->render.pixelOp == BFONT_PIXELOP_SET){
			for (int y = deltaY; y < h; y++){
				uint8_t *row = surface->pixels + (pitch * (oy + y));
			 	
				for (int x = deltaX; x < w; x++){
					if (getPixel1(pixels, pitch1, x, y))
						setPixel1(row, posX2+x);
				}
			}
		}else if (font->render.pixelOp == BFONT_PIXELOP_CLEAR){
			for (int y = deltaY; y < h; y++){
				uint8_t *row = surface->pixels + (pitch * (oy + y));
			 	
				for (int x = deltaX; x < w; x++){
					if (getPixel1(pixels, pitch1, x, y))
						unsetPixel1(row, posX2+x);
				}
			}
		}else if (font->render.pixelOp == BFONT_PIXELOP_INVERT){
			for (int y = deltaY; y < h; y++){
				uint8_t *row = surface->pixels + (pitch * (oy + y));
			 	
				for (int x = deltaX; x < w; x++){
					if (getPixel1(pixels, pitch1, x, y)){
						if (1 ^ getPixel1(row, 0, posX2+x, 0))
							setPixel1(row, posX2+x);
						else
							unsetPixel1(row, posX2+x);
					}
				}
			}
		}
	}


	if (font->render.flags&BFONT_RENDER_FIXEDWIDTH){
		return font->header.fbb.width;

	}else if (font->header.flags&BFONT_FLAGS_PROPSPACE){
		if (font->render.flags&BFONT_RENDER_CONDENSED){
			int advance = ((render->end.x+boxWidth) - (font->header.fbb.width + render->start.x));
			return advance;
		}else{
			return glyph->dwidth;
		}
	}else{
		return glyph->dwidth + xOffset;
	}
}

#if ENABLE_COMBINED
// tests for diacritical marks and other combined characters
static inline int isCombinedMark (const uint16_t ch)
{

	if (ch < 768)
		return 0;
	else if (ch < 880) 		// 768 to 879		latin
		return 1;
	else if (ch == 1161)
		return 1;
	else if (ch < 1456)
		return 0;
	else if (ch < 1475)
		return 1;			// 1456 to 1474		hebrew
	else if (ch < 1611)
		return 0;
	else if (ch < 1649)		
		return 1;			// 1611 to 1648		arabic
	else if (ch < 2305)
		return 0;
	else if (ch < 2308)
		return 1;			// 2305 to 2307		devanagari
	else if (ch < 2364)
		return 0;
	else if (ch < 2387)
		return 1;			// 2364 to 2386		devanagari
	else if ((ch == 3632)
		  || (ch == 3633))	// 3632 and 3633	thai
		return 1;
	else if (ch < 3636)
		return 0;
	else if (ch < 3643)
		return 1;			// 3636 to 3642		thai
	else if (ch < 3655) 
		return 0;
	else if (ch < 3663)
		return 1;			// 3655 to 3662		thai
	else if (ch < 7616)
		return 0;
	else if (ch < 7680)
		return 1;			// 7616 to 7679		marks supplement
	else if (ch < 8400)
		return 0;
	else if (ch < 8448)
		return 1;			// 8400 to 8447		marks for symbols
	else if (ch < 65056)
		return 0;
	else if (ch < 65072)
		return 1;			// 65056 to 65071	half marks
	else
		return 0;
}
#endif

static inline int drawGlyphStd (_ufont_t *font, _glyph_t *glyph, uint8_t *pixels, glyph_render_t *render)
{
	if (font->render.flags&BFONT_RENDER_GLYPHCLIP){
		if (render->start.y + glyph->h <= DHEIGHT-1){
			if (render->start.x + glyph->w <= DWIDTH-1){
#if ENABLE_COMBINED
				render->previous.x = render->start.x;
				render->previous.y = render->start.y;
#endif
				render->start.x += renderGlyph(font, glyph, pixels, render);
				render->start.x += font->render.glyphPadding;

				if (render->start.x >= DWIDTH) return 0;
				return 1;
			}
		}
		return 0;
	}else{
#if ENABLE_COMBINED
		render->previous.x = render->start.x;
		render->previous.y = render->start.y;
#endif
		render->start.x += renderGlyph(font, glyph, pixels, render);
		render->start.x += font->render.glyphPadding;

		if (render->start.x >= DWIDTH) return 0;
		return 1;
	}
}

#if ENABLE_COMBINED
static inline int drawGlyphCombined (_ufont_t *font, _glyph_t *glyph, uint8_t *pixels, glyph_render_t *render)
{
	if (font->render.flags&BFONT_RENDER_GLYPHCLIP){
		if (render->start.y + glyph->h <= DHEIGHT-1){
			render->start.x = render->previous.x;
			render->start.y = render->previous.y;
			
			if (render->start.x + glyph->w <= DWIDTH-1){
				render->start.x += renderGlyph(font, glyph, pixels, render);
				render->start.x += font->render.glyphPadding;
				if (render->start.x >= DWIDTH) return 0;
				return 1;
			}
		}
		return 0;
	}else{
		render->start.x = render->previous.x;
		render->start.y = render->previous.y;
		
		render->start.x += renderGlyph(font, glyph, pixels, render);
		render->start.x += font->render.glyphPadding;
		if (render->start.x >= DWIDTH) return 0;
		return 1;
	}
}
#endif

static inline int drawGlyph (_ufont_t *font, _glyph_t *glyph, uint8_t *pixels, glyph_render_t *render, const uint16_t codepoint)
{
#if ENABLE_COMBINED
	if (font->render.flags&BFONT_RENDER_COMBINED && isCombinedMark(codepoint))
		return drawGlyphCombined(font, glyph, pixels, render);
	else
#endif
		return drawGlyphStd(font, glyph, pixels, render);
}

static inline int readGlyphHeader (_ufont_t *font, _glyph_t *glyph, const uint32_t offset)
{
	
	fio_seek(font->fio, offset);
	int ret = fio_read(font->fio, glyph, sizeof(*glyph));

#if 0
	//printf("encoding %i\n", table->encoding);
	printf("length %i\n", glyph->length);
	printf("dwidth %i\n", glyph->dwidth);
	printf("w %i\n", glyph->w);
	printf("h %i\n", glyph->h);
	printf("xoffset %i\n", glyph->xOffset);
	printf("yoffset %i\n", glyph->yOffset);
	printf("pixels length %i\n", glyph->length);
#endif

	return ret;
}

static inline uint8_t *readGlyphPixels (_ufont_t *font, _glyph_t *glyph, const uint32_t offset)
{
	
	//printf("readGlyphPixels %i %i\n", table->encoding, table->offset);

	uint8_t *pixels = (uint8_t*)uf_malloc(glyph->length);
	if (!pixels) return NULL;
	
	fio_seek(font->fio, offset+sizeof(_glyph_t));
	if (!fio_read(font->fio, pixels, glyph->length)){
		uf_free(pixels);
		return NULL;
	}
	return pixels;
}

static inline uint8_t *readGlyph (_ufont_t *font, _glyph_t *glyph, const uint32_t offset)
{
	if (readGlyphHeader(font, glyph, offset))
		return readGlyphPixels(font, glyph, offset);

	return NULL;
}

static inline _cache_glyph_t *cacheGetGlyph (_ufont_t *font, const uint16_t encoding)
{
	for (int i = 0; i < BFONT_CACHE_SIZE; i++){
		//printf("search %i %i %i\n", i, font->glyphs.list[i].encoding, encoding);
		if (font->glyphs.list[i].encoding == encoding){
			if (font->glyphs.list[i].pixels)
				return &font->glyphs.list[i];
			else
				return NULL;
		}
	}
	return NULL;
}

static inline _cache_glyph_t *cacheAddGlyph (_ufont_t *font, const uint16_t encoding, _glyph_t *glyph, uint8_t *pixels)
{
	// add here, if not empty then free
	_cache_glyph_t *cglyph = &font->glyphs.list[font->glyphs.cachePosition];
	if (cglyph->encoding){
		if (cglyph->pixels) uf_free(cglyph->pixels);
	}


	cglyph->encoding = encoding;
	memcpy(&cglyph->glyph, glyph, sizeof(_glyph_t));
	cglyph->pixels = pixels;

	//printf("cachePosition %i %i\n", encoding, font->glyphs.cachePosition);
	
	if (++font->glyphs.cachePosition >= BFONT_CACHE_SIZE)
		font->glyphs.cachePosition = 0;	
		
	return cglyph;
}

static inline int tableLoadBlock (_ufont_t *font, const int block)
{
	
	if (font->table.blockLoaded == block)
		return 1;

	//static int loadTotal = 0;
	int glyphsPerBlocks = (font->table.listSize / sizeof(_lookup_t));
	font->table.blockTotal = font->header.lookup.glyphs / glyphsPerBlocks;
	
	if (font->header.lookup.glyphs % glyphsPerBlocks)
		font->table.blockTotal++;
		
	//printf("tableLoadBlock: %i, %i %i, %i %i\n", font->header.lookup.glyphs, font->table.blockTotal, block, glyphsPerBlocks, font->table.listSize);
	
	size_t blockPosition = block * font->table.listSize;				// position in complete table, block 0 is position 0
	size_t blockOffset = font->header.lookup.offset + blockPosition;	// position offset of this block within file
	
	int entriesInthisblock = glyphsPerBlocks;
	if (block * glyphsPerBlocks > font->header.lookup.glyphs)
		entriesInthisblock = (block * glyphsPerBlocks) - font->header.lookup.glyphs;
	
	font->table.from = block * glyphsPerBlocks;
	font->table.to = font->table.from + entriesInthisblock-1;
	font->table.total = entriesInthisblock;
	font->table.entriesPerBlock = glyphsPerBlocks;

#if 0
	printf("glyphsPerBlocks %i\n", glyphsPerBlocks);
	printf("numBlocks %i\n", font->table.blockTotal);
	printf("blockPosition %i\n", blockPosition);
	printf("blockOffset %i\n", blockOffset);
	printf("font->table.from %i\n", font->table.from);
	printf("font->table.to %i\n", font->table.to);
	printf("entriesInthisblock %i\n\n", entriesInthisblock);
#endif

	//printf("block %i offset %i %i\n", block, blockOffset, font->header.lookup.offset);
	//printf("entriesInthisblock %i, %i %i,%i %i\n", entriesInthisblock, font->table.total, sizeof(_lookup_t), font->table.total*sizeof(_lookup_t), font->table.listSize);

	fio_seek(font->fio, blockOffset);
	//printf("table.list Read: %i\n", sizeof(_lookup_t)*font->table.total);
	
	if (fio_read(font->fio, font->table.list, sizeof(_lookup_t)*font->table.total))
		font->table.blockLoaded = block;

	//printf("load table block [%i]: %i of %i  (%i -> %i)\n", ++loadTotal, block, font->table.blockTotal, font->table.from, font->table.to);
	
	return 1;
}

/*
static inline int codeLookupIdx (_table_t *table, const uint16_t code)
{
	for (int i = 0; i < table->total; i++){
		if (table->list[i].encoding == code)
			return i;
	}
	return -1;
}
*/

static inline _lookup_t *codeLookup (_table_t *table, const uint16_t code)
{
	for (int i = 0; i < table->total; i++){
		if (table->list[i].encoding == code)
			return &table->list[i];
	}
	return NULL;
}

static inline _lookup_t *tableSearchBlocks (_ufont_t *font, const int from, const int to, const uint16_t codepoint)
{
	for (int i = from; i <= to; i++){
		//printf("%.5i: ", codepoint);
		if (tableLoadBlock(font, i)){
			//printf("%i \n", i);
			_lookup_t *tableEntry = codeLookup(&font->table, codepoint);
			if (tableEntry) return tableEntry;
		}
	}
	return NULL;
}

static inline _lookup_t *tableFindEntry (_ufont_t *font, _table_t *table, const uint16_t codepoint)
{
	_lookup_t *tableEntry = NULL;
	// ansi/ascii characters are probably in the first block, try there first
	if (codepoint < table->entriesPerBlock && table->blockLoaded){
		table->blockLoaded = table->blockTotal + 1;
	}else{
		tableEntry = codeLookup(table, codepoint);
	}
	
	//printf("\n");
	//printf("tableFindEntry: (%i) %p %i, %i\n", codepoint, tableEntry, table->blockLoaded, table->entriesPerBlock);
	
	//tableEntry = codeLookup(table, codepoint);
	if (!tableEntry){
		int from = table->blockLoaded+1;
		if (from >= table->blockTotal)
			from = 0;
		int to = table->blockTotal-1;

		tableEntry = tableSearchBlocks(font, from, to, codepoint);
		//printf("tableFindEntry: search: %p %i %i\n", tableEntry, from, to);
		
		if (!tableEntry && (from > 0))
			tableEntry = tableSearchBlocks(font, 0, from-1, codepoint);
#if 0
		if (!tableEntry){
			tableEntry = codeLookup(table, font->header.defaultGlyph);
			//if (!tableEntry) return NULL;
		}
#endif
	}
	return tableEntry;
}


static inline _cache_glyph_t *loadGlyph (_ufont_t *font, const uint16_t codepoint)
{
	uint32_t goffset = 0;
		
	//printf("\n");
		
	_cache_glyph_t *cglyph = cacheGetGlyph(font, codepoint);
	//printf("loadGlyph: cglyph %p\n", cglyph);
	
	if (!cglyph){
		_lookup_t *tableEntry = tableFindEntry(font, &font->table, codepoint);
	//	printf("loadGlyph: tableEntry %p\n", tableEntry);
		
		if (!tableEntry){
			if (font->glyphs.defaultGlyphPosition && !(font->render.flags&BFONT_RENDER_NODEFAULT))
				goffset = font->glyphs.defaultGlyphPosition;
			else
				return NULL;
		}else{
			goffset = tableEntry->offset;
		}
		
		//printf("loadGlyph: goffset %u\n", goffset);
		
		_glyph_t glyph;
		uint8_t *pixels = readGlyph(font, &glyph, goffset);
		if (!pixels) return NULL;
		
		// workaround/fix zero-width space bug	
		if (codepoint == ' '){
			glyph.w = max(glyph.w, glyph.dwidth);
		}
			
		cglyph = cacheAddGlyph(font, codepoint, &glyph, pixels);
		if (!cglyph){
			uf_free(pixels);
			return NULL;
		}
	}
	return cglyph;
}

uint32_t fontGetGlyphHeader (_ufont_t *font, const uint16_t codepoint, _glyph_t *glyph)
{
	_cache_glyph_t *cglyph = loadGlyph(font, codepoint);
	if (!cglyph) return 0;

	if (glyph) *glyph = cglyph->glyph; //memcpy(glyph, &cglyph->glyph, sizeof(cglyph->glyph));

	uint32_t offset = 0;
	_lookup_t *tableEntry = codeLookup(&font->table, codepoint);
	if (tableEntry) offset = tableEntry->offset;
	
	if ((cglyph->encoding == codepoint) && offset)
		return offset;
	return 0;
}

int metricsGetGlyphHeader (_ufont_t *font, const uint16_t codepoint, _glyph_t *glyph)
{
	_cache_glyph_t *cglyph = cacheGetGlyph(font, codepoint);
	if (cglyph){
		*glyph = cglyph->glyph;
	
	}else{
		uint32_t goffset = 0;
		_lookup_t *tableEntry = tableFindEntry(font, &font->table, codepoint);
		if (!tableEntry){
			if (font->glyphs.defaultGlyphPosition && !(font->render.flags&BFONT_RENDER_NODEFAULT))
				goffset = font->glyphs.defaultGlyphPosition;
			else
				return 0;
		}else{
			goffset = tableEntry->offset;
		}

		if (!readGlyphHeader(font, glyph, goffset))
			return 0;
	}
	return 1;
}

uint8_t *fontGetGlyphPixels (_ufont_t *font, const uint16_t codepoint)
{
	_cache_glyph_t *cglyph = loadGlyph(font, codepoint);
	if (!cglyph) return 0;

	uint32_t offset = 0;
	_lookup_t *tableEntry = codeLookup(&font->table, codepoint);
	if (tableEntry) offset = tableEntry->offset;
	
	if ((cglyph->encoding == codepoint) && offset)
		return cglyph->pixels;
	
	return NULL;
}

static inline int drawCodepoint (_ufont_t *font, glyph_render_t *render, const uint16_t codepoint)
{
	
	_cache_glyph_t *cglyph = loadGlyph(font, codepoint);
	if (!cglyph) return 0;

	if (cglyph->pixels){
		int ret = drawGlyph(font, &cglyph->glyph, cglyph->pixels, render, cglyph->encoding);
		if (!ret) return 0;
		if (codepoint == ' ' && font->header.flags&BFONT_FLAGS_PROPSPACE){
			render->start.x += font->render.spaceGlyphPadding;
		}
		return 1;
	}
	return 0;
}

#if ENABLE_CHARREFS
static inline const uint8_t atoi2 (const uint8_t *str, uint16_t *value)
{
	uint8_t c = 0;
	*value = 0;
	
	while (*str) {
		if ((*str > 47) && (*str < 58)){
			*value *= 10;
			*value += (*str-'0');
			c++;
		}else{
			return c;
		}
		str++;
	}
	return c;
}
#endif

static inline uint8_t isCharRef (const uint8_t *str, uint16_t *wc)
{
#if ENABLE_CHARREFS
	if (*(str++) == '&'){
		if (*str == '#'){
			uint8_t chars = 0;
			if ((chars=atoi2(++str, wc))){
				if (*(str+chars) == L';')
					return (chars+2);
			}
			return 0;
		}
		
		for (int i = 0; entities[i].len; i++){
			if (str[0] == (uint8_t)entities[i].entity[0]){
				if (!strncmp((char*)str, entities[i].entity, entities[i].len)){
					*wc = entities[i].encoding;
					return entities[i].len;
				}
			}
		}
		
	}
#endif
	return 0;
}

static inline int metricsCalc (_ufont_t *font, const uint16_t wc, int *totalWidth, int *totalHeight)
{
	_glyph_t glyph;
	if (!metricsGetGlyphHeader(font, wc, &glyph))
		return 0;

	int h = (font->header.verticalPitch - glyph.h - glyph.yOffset) + glyph.h;
	if (h > *totalHeight) *totalHeight = h;

		
	if (font->render.flags&BFONT_RENDER_FIXEDWIDTH)
		*totalWidth += font->header.fbb.width;
	else if (font->header.flags&BFONT_FLAGS_PROPSPACE)
		*totalWidth += glyph.dwidth;
	else
		*totalWidth += glyph.dwidth + max((font->header.fbb.xOffset - glyph.xOffset), 0);
		
	if (wc == L' ' && font->header.flags&BFONT_FLAGS_PROPSPACE)
		*totalWidth += font->render.spaceGlyphPadding;
	
	//if (wc == L'\t' && font->header.flags&BFONT_FLAGS_PROPSPACE)
	//	*totalWidth += (font->render.spaceGlyphPadding * BFONT_TAB_LENGTH);

	//if (str[c+1])	// don't apply padding after the last char
	//	totalWidth += font->render.glyphPadding;

	return 1;
}

static inline uint8_t *advanceWhiteSpace (const uint8_t *text)
{
	uint8_t *str = (uint8_t*)text;
	while(*str == ' ' || *str == '\t')
		str++;
		
	return str;
}

static inline uint16_t *advanceWhiteSpaceW (const uint16_t *text)
{
	uint16_t *str = (uint16_t*)text;
	while(*str == ' ' || *str == '\t')
		str++;
		
	return str;
}

static inline void breakStringFree (uint8_t **txt)
{
	if (!txt) return;
	
	for (int i = 0; txt[i]; i++)
		free(txt[i]);
	free(txt);
}

static inline int isWordBreakW (const uint16_t ch)
{
	if (ch == L' ' || ch == L',' || ch == L'.' || ch == L'-' || ch == L'|' || ch == L'\\' || ch == L'\"' || ch == L':' || ch == L';' || ch == L'/' || ch == L'+' /*|| ch == L'(' || ch == L')' || ch == 12288*/)
		return 1;
	else
		return 0;
}

static inline int isWordBreak (const uint8_t ch)
{
	if (ch == ' '  || ch == ',' || ch == '.' || ch == '\r' || ch == '\n' || ch == '-' || ch == '|' || ch == '\\' || ch == '\"' || ch == ';' || ch == ':' || ch == '/' || ch == '+' /*|| ch == '(' || ch == ')' || ch == 12288*/)
		return 1;
	else
		return 0;
}

static inline uint8_t **breakString (const uint8_t *text)
{
	if (!text || !text[0]) return NULL;
		
	int strTotal = 8;
	int x1 = 0;
	int x2 = 0;
	int wIdx = 0;
	uint8_t **wrapped = calloc(1+strTotal, sizeof(uint8_t*));


	while (1){
		if (text[x2] && isWordBreak(text[x2])){
			int len = (x2 - x1) + 1;
	
			wrapped[wIdx] = calloc(len+1, sizeof(uint8_t));
			strncpy((char*)wrapped[wIdx], (char*)&text[x1], len);
			wrapped[wIdx][len] = 0;

			if (++wIdx >= strTotal-1){
				strTotal <<= 1;
				wrapped = realloc(wrapped, (1+strTotal) * sizeof(uint8_t*));
			}
			x1 = x2 + 1;
		}
		x2++;

		if (!text[x2]){
			int len = (x2 - x1) + 1;
			if (!len) break;
			wrapped[wIdx] = calloc(len+1, sizeof(uint8_t));
			strncpy((char*)wrapped[wIdx], (char*)&text[x1], len);
			wrapped[wIdx++][len] = 0;
			break;
		}
	}

	if (!wIdx){
		free(wrapped);
		return NULL;
	}else{
		wrapped[wIdx] = NULL;
		return wrapped;
	}
}

static inline int printWordwrap (_ufont_t *font, int *desX, int *desY, const uint8_t *str)
{
	
	int ct = 0;
	int x = *desX;
	int y = *desY;

	const uint16_t userFlags = fontGetRenderFlags(font);
	//const uint16_t wrapFlags = (userFlags | BFONT_RENDER_ADVANCE_X) &~(BFONT_RENDER_ADVANCE_Y|BFONT_RENDER_WORDWRAP);
	const uint16_t wrapFlags = BFONT_RENDER_ADVANCE_X | BFONT_RENDER_RETURN|BFONT_RENDER_NEWLINE;
	fontSetRenderFlags(font, wrapFlags);

	int maxX = DWIDTH;
	int maxY = DHEIGHT;
	

	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		_ufont_surface_t *surface = fontGetRenderSurface(font);
		maxX = surface->width;
		maxY = surface->height;
	}
	
	int returnCol = x;
	if (font->render.flags&BFONT_RENDER_WWRETURN)
		returnCol = 0;
	
	int maxRenderedX = 0;
	int maxRenderedY = 0;
	
	uint8_t **txt = breakString(str);
	if (txt){
		int maxH = 0;
		
		for (int i = 0; txt[i]; i++){
			if (*txt[i] != '\n'){
				int width = 0, height = 0;
				fontGetMetrics(font, txt[i], &width, &height);
				//height -= font->render.linePadding;
				
				if (height > maxH) maxH = height;
				if ((x+width)-1 > maxX-1){
					x = returnCol;
					y += maxH - font->render.linePadding;
					maxH = 0;
				}
			}else{
				x = returnCol;
				y += maxH + font->render.linePadding;
				maxH = 0;
			}
			if (y > maxY-1) break;
			
			//printf("%i: %i,%i '%s' %i\n", i, x, y, txt[i], strlen( txt[i]));
			int ret = fontPrint(font, &x, &y, txt[i]);
			//printf("ret %i\n", ret);
			if (x > maxRenderedX) maxRenderedX = x;
			if (y > maxRenderedY) maxRenderedY = y;
			if (ret < 1) break;
			ct += ret;
		}
		breakStringFree(txt);
	}

	*desX = maxRenderedX;
	*desY = maxRenderedY;
	fontSetRenderFlags(font, userFlags);
	return ct;
}

static inline int printWordwrap8 (_ufont_t *font, int *desX, int *desY, const uint8_t *str)
{
	
	int ct = 0;
	int x = *desX;
	int y = *desY;

	const uint16_t userFlags = fontGetRenderFlags(font);
	//const uint16_t wrapFlags = (userFlags | BFONT_RENDER_ADVANCE_X) &~(BFONT_RENDER_ADVANCE_Y|BFONT_RENDER_WORDWRAP);
	const uint16_t wrapFlags = BFONT_RENDER_ADVANCE_X | BFONT_RENDER_RETURN|BFONT_RENDER_NEWLINE;
	fontSetRenderFlags(font, wrapFlags);

	int maxX = DWIDTH;
	int maxY = DHEIGHT;
	

	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		_ufont_surface_t *surface = fontGetRenderSurface(font);
		maxX = surface->width;
		maxY = surface->height;
	}
	
	int returnCol = x;
	if (font->render.flags&BFONT_RENDER_WWRETURN)
		returnCol = 0;
	
	int maxRenderedX = 0;
	int maxRenderedY = 0;
	
	uint8_t **txt = breakString(str);
	if (txt){
		int maxH = 0;
		
		for (int i = 0; txt[i]; i++){
			if (*txt[i] != '\n'){
				int width = 0, height = 0;
				fontGetMetrics8(font, txt[i], &width, &height);
				//height -= font->render.linePadding;
				
				if (height > maxH) maxH = height;
				if ((x+width)-1 > maxX-1){
					x = returnCol;
					y += maxH - font->render.linePadding;
					maxH = 0;
				}
			}else{
				x = returnCol;
				y += maxH + font->render.linePadding;
				maxH = 0;
			}
			if (y > maxY-1) break;
			
			//printf("%i: %i,%i '%s' %i\n", i, x, y, txt[i], strlen( txt[i]));
			int ret = fontPrint8(font, &x, &y, txt[i]);
			//printf("ret %i\n", ret);
			if (x > maxRenderedX) maxRenderedX = x;
			if (y > maxRenderedY) maxRenderedY = y;
			if (ret < 1) break;
			ct += ret;
		}
		breakStringFree(txt);
	}

	*desX = maxRenderedX;
	*desY = maxRenderedY;
	fontSetRenderFlags(font, userFlags);
	return ct;
}

int fontPrint (_ufont_t *font, int *desX, int *desY, const uint8_t *str)
{
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		if (*desY >= DHEIGHT-1 || *desX >= DWIDTH-1)
			return 0;
	}

	if (font->render.flags&BFONT_RENDER_SKIPSPACE)
		str = advanceWhiteSpace(str);
		
#if 0
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		int mw, mh;
		metricsGetString(font, str, &mw, &mh);
		surfaceDrawRectangle(&font->display.surface, *desX+2, *desY+2, *desX+mw+2, *desY+mh+2, COLOUR_GREEN);
	}
#endif

	if (font->render.flags&BFONT_RENDER_WORDWRAP)
		return printWordwrap(font, desX, desY, str);
		
	int x = *desX;
	int yy = *desY;
	int maxH = 0;
	int minY = 0xFFFF;
	int ct = 0;
	uint8_t tchars = 0;
	uint16_t wc = 0;
	int maxX = 0;

	*desY += font->header.fbb.yOffset;
	//*desY -= font->header.fontDescent;

	glyph_render_t render = {0};
	render.start.x = render.end.x = x;
	render.start.y = render.end.y = *desY;

	//const int slen = strlen(str);	
	for (int i = 0; str[i]; i++){
		if ((tchars=isCharRef((const uint8_t*)&str[i], &wc))){
			i += tchars;
		}else{
			wc = (uint16_t)str[i];
		}

		render.start.y = yy;

		if (wc == L'\t'){
			int num = BFONT_TAB_LENGTH;
			while (num--){
				if (!drawCodepoint(font, &render, BFONT_TAB_CHAR))
					break;
			}
			maxX = max(maxX, render.end.x);
			ct++;
			continue;
		}
	
		if (wc == L'\r'){
			if (font->render.flags&BFONT_RENDER_RETURN){
				render.start.x = render.end.x = *desX;
				ct++;
			}
			continue;
		}
	
		if (wc != L'\n'){
			//drawRectangle(&font->display.surface, render.start.x, *desY, render.start.x, *desY+400, COLOUR_MAGENTA);
			
			if (drawCodepoint(font, &render, wc)){
				int y = render.begin.y;
				if (y >= 0) minY = min(y, minY);
				maxH = max(maxH, render.end.y);
				ct++;
			}
			
			//drawRectangle(&font->display.surface, render.start.x, *desY, render.start.x, *desY+400, COLOUR_MAGENTA);
			//drawRectangle(&font->display.surface, render.end.x, *desY, render.end.x, *desY+400, COLOUR_GREEN);
			//printf("render.end.x %i\n", render.end.x);
			maxX = max(maxX, (render.end.x));
			
		}else if (font->render.flags&BFONT_RENDER_NEWLINE){
			yy = render.end.y;
			yy += font->render.linePadding;
			ct++;
		}
	}

	//printf("drawString: %i %i %i\n", maxX, render.end.x, render.begin.x);


	if (ct && minY != 0xFFFF){
		*desY = /*minY +*/ maxH;	// vertical line pitch
		if (font->render.flags&BFONT_RENDER_BOUNDRECT)
			surfaceDrawRectangle(&font->display.surface, x, minY, maxX, *desY, BFONT_COLOUR_BOUNDRECT);
		*desY -= font->header.fbb.yOffset;
	}

	if (font->render.flags&BFONT_RENDER_ADVANCE_X)
		*desX = maxX; //render.end.x;

	if (font->render.flags&BFONT_RENDER_ADVANCE_Y){
		*desY -= abs(font->header.fontDescent);
		*desY += font->render.linePadding;
	}else{
		*desY = yy;
	}
	
	*desX += 1;
	*desX += font->render.glyphPadding;

	return ct;
}

// BFONT_RENDER_WORDWRAP not implemented
int fontPrintList (_ufont_t *font, int *desX, int *desY, const uint16_t *listw)
{
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		if (*desY >= DHEIGHT-1 || *desX >= DWIDTH-1)
			return 0;
	}
	
	
	uint16_t *list = (uint16_t*)listw;
	if (font->render.flags&BFONT_RENDER_SKIPSPACE)
		list = advanceWhiteSpaceW(listw);
	
#if 0
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		int mw, mh;
		metricsGetList(font, list, &mw, &mh);
		surfaceDrawRectangle(&font->display.surface, *desX+2, *desY+2, *desX+mw+2, *desY+mh+2, COLOUR_RED);
	}
#endif
	
	int x = *desX;
	int yy = *desY;
	int maxH = 0;
	int maxX = 0;
	int minY = 0xFFFF;

	*desY += font->header.fbb.yOffset;	
	glyph_render_t render = {0};
	render.start.x = render.end.x = x;
	render.start.y = render.end.y = *desY;
		
	int ct = 0;
	for (int i = 0; list[i]; i++){
		render.start.y = yy;

		if (list[i] == L'\t'){
			int num = BFONT_TAB_LENGTH;
			while (num--){
				if (!drawCodepoint(font, &render, BFONT_TAB_CHAR))
					break;
			}
			maxX = max(maxX, render.end.x);
			ct++;
			continue;
		}
		
		if (list[i] == L'\r'){
			if (font->render.flags&BFONT_RENDER_RETURN){
				render.start.x = render.end.x = *desX;
				ct++;
			}
			continue;
		}
		
		if (list[i] != L'\n'){
			if (drawCodepoint(font, &render, list[i])){
				int y = render.begin.y;
				if (y >= 0) minY = min(y, minY);
				maxH = max(maxH, render.end.y);
				ct++;
			}
			maxX = max(maxX, render.end.x);
		}else if (font->render.flags&BFONT_RENDER_NEWLINE){
			yy = render.end.y;
			yy += font->render.linePadding;
			ct++;
		}
	}
	
	if (ct && minY != 0xFFFF){
		*desY = /*minY +*/ maxH;	// vertical line pitch
		if (font->render.flags&BFONT_RENDER_BOUNDRECT)
			surfaceDrawRectangle(&font->display.surface, x, minY, maxX, *desY, BFONT_COLOUR_BOUNDRECT);
		*desY -= font->header.fbb.yOffset;
	}
	
	if (font->render.flags&BFONT_RENDER_ADVANCE_X)
		*desX = maxX;

	if (font->render.flags&BFONT_RENDER_ADVANCE_Y){
		*desY -= abs(font->header.fontDescent);
		*desY += font->render.linePadding;
	}else{
		*desY = yy;
	}
	return ct;
}

int fontPrint8 (_ufont_t *font, int *desX, int *desY, const uint8_t *str)
{
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		if (*desY >= DHEIGHT-1 || *desX >= DWIDTH-1)
			return 0;
	}

	if (font->render.flags&BFONT_RENDER_SKIPSPACE)
		str = advanceWhiteSpace(str);

#if 0
	if (!(font->render.flags&BFONT_RENDER_METRICS)){
		int mw, mh;
		metricsGetStringUtf8(font, str, &mw, &mh);
		surfaceDrawRectangle(&font->display.surface, *desX+2, *desY+2, *desX+mw+2, *desY+mh+2, COLOUR_RED);
	}
#endif

	if (font->render.flags&BFONT_RENDER_WORDWRAP)
		return printWordwrap8(font, desX, desY, str);

	int x = *desX;
	int yy = *desY;
	int maxH = 0;
	int maxX = 0;
	int minY = 0xFFFF;
	
	*desY += font->header.fbb.yOffset;
	glyph_render_t render = {0};
	render.start.x = render.end.x = x;
	render.start.y = render.end.y = *desY;
			
	uint16_t wc16 = 0;
	int ct = 0;
	//const int slen = strlen(str);

	for (int i = 0; str[i]; i++){
		int tchars = isCharRef((const uint8_t*)&str[i], &wc16);
		if (!tchars){
			uint32_t wc32;
			tchars = UTF8ToUTF32((uint8_t*)&str[i], &wc32);
			wc16 = wc32;
		}else{
			i++;
		}

		if (!tchars) continue;
		i += tchars-1;
		render.start.y = yy;
		
		if (wc16 == L'\t'){
			int num = BFONT_TAB_LENGTH;
			while (num--){
				if (!drawCodepoint(font, &render, BFONT_TAB_CHAR))
					break;
			}
			maxX = max(maxX, render.end.x);
			ct++;
			continue;
		}
		
		if (wc16 == L'\r'){
			if (font->render.flags&BFONT_RENDER_RETURN){
				render.start.x = render.end.x = *desX;
				ct++;
			}
			continue;
		}		

		if (wc16 != L'\n'){
			if (drawCodepoint(font, &render, wc16)){
				int y = render.begin.y;
				if (y >= 0) minY = min(y, minY);
				maxH = max(maxH, render.end.y);
				ct++;
			}
			maxX = max(maxX, render.end.x);
		}else if (font->render.flags&BFONT_RENDER_NEWLINE){
			yy = render.end.y;
			yy += font->render.linePadding;
			ct++;
		}
	}

	if (ct && minY != 0xFFFF){
		*desY = /*minY +*/ maxH;	// vertical line pitch
		if (font->render.flags&BFONT_RENDER_BOUNDRECT)
			surfaceDrawRectangle(&font->display.surface, x, minY, maxX, *desY, BFONT_COLOUR_BOUNDRECT);
		*desY -= font->header.fbb.yOffset;
	}
	
	if (font->render.flags&BFONT_RENDER_ADVANCE_X)
		*desX = maxX;

	if (font->render.flags&BFONT_RENDER_ADVANCE_Y){
		*desY -= abs(font->header.fontDescent);
		*desY += font->render.linePadding;
	}else{
		*desY = yy;
	}
	
	*desX += 1;
	*desX += font->render.glyphPadding;
	
	return ct;
}

int fontGetMetricsList (_ufont_t *font, const uint16_t *list, int *width, int *height)
{
	int tmpNull;
	if (!width) width = &tmpNull;
	if (!height) height = &tmpNull;
	
	*width = 0;
	*height = 0;
	
	int old = fontSetRenderFlags(font, BFONT_RENDER_METRICS | BFONT_RENDER_ADVANCE_X | BFONT_RENDER_ADVANCE_Y);
	int ret = fontPrintList(font, width, height, list);
	fontSetRenderFlags(font, old);
	//*width += 2;
	*height += 2;
	return ret;
}

int fontGetMetrics (_ufont_t *font, const uint8_t *text, int *width, int *height)
{
	int tmpNull;
	if (!width) width = &tmpNull;
	if (!height) height = &tmpNull;
	
	*width = 0;
	*height = 0;
	
	int old = fontGetRenderFlags(font);	
	int newline = (old&BFONT_RENDER_RETURN) | (old&BFONT_RENDER_NEWLINE);
	fontSetRenderFlags(font, newline|BFONT_RENDER_METRICS | BFONT_RENDER_ADVANCE_X | BFONT_RENDER_ADVANCE_Y);
	int ret = fontPrint(font, width, height, text);
	fontSetRenderFlags(font, old);
	if (ret){
		//*width += 2;
		*height += 2;
	}
	return ret;
}

int fontGetMetrics8 (_ufont_t *font, const uint8_t *text, int *width, int *height)
{
	int tmpNull;
	if (!width) width = &tmpNull;
	if (!height) height = &tmpNull;
	
	*width = 0;
	*height = 0;
	
	int old = fontGetRenderFlags(font);	
	int newline = (old&BFONT_RENDER_RETURN) | (old&BFONT_RENDER_NEWLINE);
	fontSetRenderFlags(font, newline|BFONT_RENDER_METRICS | BFONT_RENDER_ADVANCE_X | BFONT_RENDER_ADVANCE_Y);
	int ret = fontPrint8(font, width, height, text);
	fontSetRenderFlags(font, old);
	if (ret){
		//*width += 2;
		*height += 2;
	}
	return ret;
}

static inline void freeLookupTable (_ufont_t *font)
{
	if (font->table.list)
		uf_free(font->table.list);
	font->table.list = NULL;
	font->table.blockLoaded = 254;
}

static inline void freeGlyphCache (_ufont_t *font)
{
	for (int i = 0; i < BFONT_CACHE_SIZE; i++){
		if (font->glyphs.list[i].encoding){
			font->glyphs.list[i].encoding = 0;
			if (font->glyphs.list[i].pixels){
				uf_free(font->glyphs.list[i].pixels);
				font->glyphs.list[i].pixels = NULL;
			}
		}
	}
}

void fontCleanCache (_ufont_t *font)
{
	freeGlyphCache(font);
}

void fontClose (_ufont_t *font)
{
	//printf("fontClose: %p\n", font);
	
	if (font->fio)
		fio_close(font->fio);
	
	freeGlyphCache(font);
	freeLookupTable(font);
	
	memset(font, 0, sizeof(*font));
}

uint16_t fontGetRenderFlags (_ufont_t *font)
{
	return font->render.flags;
}

uint16_t fontSetRenderFlags (_ufont_t *font, const uint16_t flags)
{
	uint16_t old = font->render.flags;
	font->render.flags = flags;
	return old;
}

int8_t fontSetWriteOp (_ufont_t *font, const int8_t pixelOp)
{
	int8_t old = font->render.pixelOp;
	font->render.pixelOp = pixelOp&0x03;
	return old;
}

int8_t fontGetWriteOp (_ufont_t *font)
{
	return font->render.pixelOp;
}

int8_t fontSetLineSpace (_ufont_t *font, const int8_t pixels)
{
	int8_t old = font->render.linePadding;
	font->render.linePadding = pixels;
	return old;
}

int8_t fontGetLineSpace (_ufont_t *font)
{
	return font->render.linePadding;
}

int8_t fontSetGlyphPadding (_ufont_t *font, const int8_t pixels)
{
	int8_t old = font->render.glyphPadding;
	font->render.glyphPadding = pixels;
	return old;
}

int8_t fontGetGlyphPadding (_ufont_t *font)
{
	return font->render.glyphPadding;
}

void fontSurfaceFree (_ufont_surface_t *surface)
{
	//if (surface){
		if (surface->flags8&SURFACE_PALLOC) uf_free(surface->pixels);
		uf_free(surface);
	//}
}

static void fontSurfaceFill (_ufont_surface_t *surface, const int width, const int height, const uint16_t colour, const uint8_t *pixels)
{
	surface->width = width;
	surface->height = height;
	surface->size = height * CALC_PITCH_1(width);
	surface->colour = colour;
	surface->bpp = SURFACE_BPP_1;
	surface->pixels = (uint8_t*)pixels;
	surface->flags8 = SURFACE_PALETTE_DIR<<SURFACE_PALETTE_DIR_H;
}

_ufont_surface_t *fontCreateSurface (const int width, const int height, const uint16_t colour, uint8_t *pixels)
{
	_ufont_surface_t *surface = (_ufont_surface_t*)uf_calloc(1, sizeof(_ufont_surface_t));
	if (surface){
		fontSurfaceFill(surface, width, height, colour, pixels);
		if (!pixels){
			surface->flags8 |= SURFACE_PALLOC;
			surface->bpp = SURFACE_BPP_1;
			surface->size = height * CALC_PITCH_1(width);
			surface->pixels = (uint8_t*)uf_calloc(1, surface->size);
			if (!surface->pixels){
				uf_free(surface);
				return NULL;
			}
		}
	}
	return surface;
}

_ufont_surface_t *fontGetDisplayBuffer (_ufont_t *font)
{
	return &font->display.surface;
}

void fontSetDisplayBuffer (_ufont_t *font, void *pixels, const int width, const int height)
{
	DWIDTH = width;
	DHEIGHT = height;

	fontSurfaceFill(&font->display.surface, width, height, COLOUR_BLACK, (const uint8_t*)pixels);
	font->display.surface.bpp = SURFACE_BPP_16;
	font->display.surface.size = height * CALC_PITCH_16(width);
	
}

_ufont_surface_t *fontCreateTexture (const char *path)
{
	uint32_t width, height, bpc;
	if (png_metrics(path, &width, &height, &bpc)){
		uint8_t *pixels = (uint8_t*)uf_malloc(height * CALC_PITCH_16(width));
		if (!pixels)
			return NULL;

		if (!png_read(path, pixels, BPP_16, 0, 0)){
			uf_free(pixels);
			return NULL;
		}
		
		_ufont_surface_t *surface = (_ufont_surface_t*)uf_calloc(1, sizeof(_ufont_surface_t));
		if (surface){
			surface->size = height * CALC_PITCH_16(width);
			if (pixels){
				fontSurfaceFill(surface, width, height, 0, pixels);
				surface->flags8 |= SURFACE_PALLOC;
				surface->bpp = SURFACE_BPP_16;		
				return surface;
			}
		}
	}

	return NULL;
}

// textures are 16bit only
_ufont_surface_t *fontGenerateTexture (const int width, const int height, const void *pixels)
{
	_ufont_surface_t *surface = (_ufont_surface_t*)uf_calloc(1, sizeof(_ufont_surface_t));
	if (surface){
		fontSurfaceFill(surface, width, height, 0, (const uint8_t*)pixels);
		surface->bpp = SURFACE_BPP_16;
		surface->size = height * CALC_PITCH_16(width);
	}

	return surface;
}

void fontTextureFree (_ufont_surface_t *surface)
{
	fontSurfaceFree(surface);
}

#if 0
int fontReadHeader (const uint8_t *file, _ufont_header_t *header)
{
	memset(header, 0, sizeof(*header));
	fileio_t *fio = fio_open((uint8_t*)file, FIO_READ);
	if (fio){
		fio_seek(fio, 0);
		int ret = fio_read(fio, header, sizeof(*header));
		fio_close(fio);
		return ret;
	}
		
	return 0;
}
#else

int fontReadHeader (_ufont_t *font, _ufont_header_t *header)
{
	memcpy(header, &font->header, sizeof(*header));
	return font->fio != NULL;
}

#endif

int fontOpen (_ufont_t *font, const char *file)
{
	memset(font, 0, sizeof(*font));
	if (!file) return -1;
	
	if (!DWIDTH) DWIDTH = 65535;
	if (!DHEIGHT) DHEIGHT = 65535;

	font->glyphs.cachePosition = 0;
	
	font->render.spaceGlyphPadding = 0;
	font->render.glyphPadding = 0;
	font->render.linePadding = 2;
	font->render.pixelOp = BFONT_PIXELOP_DEFAULT;
	fontSetRenderFlags(font, BFONT_RENDER_DEFAULT);
		
	font->table.blockLoaded = 254;
	font->table.memSize = 2048*8;		// equates to allowable max memory (bytes)
	font->table.memSize -= (font->table.memSize%sizeof(_lookup_t));


	font->fio = fio_open((uint8_t*)file, FIO_READ);
	if (!font->fio) return 0;
	
	fio_seek(font->fio, 0);
	int headerRead = fio_read(font->fio, &font->header, sizeof(font->header));
	if (headerRead != sizeof(font->header)){
		fio_close(font->fio);
 		font->fio = NULL;
 		return 0;
	}
	
	if (font->header.ident != BFONT_IDENT){
		fio_close(font->fio);
 		font->fio = NULL;
 		return 0;
	}
	
	font->table.listSize = font->header.lookup.glyphs*sizeof(_lookup_t);
	if (font->table.listSize > font->table.memSize)
		font->table.listSize = font->table.memSize;

	//printf("fontOpen: listSize: %i\n",  font->table.listSize);
	//printf("lookup table size: %i\n", font->header.lookup.glyphs*sizeof(_lookup_t));

	font->table.list = (_lookup_t*)uf_calloc(font->table.listSize+sizeof(_lookup_t), 1);
	if (!font->table.list) return 0;

 	if (!tableLoadBlock(font, 0)){
 		uf_free(font->table.list);
		font->table.list = NULL;
 		fio_close(font->fio);
 		font->fio = NULL;
 		return 0;
 	}

 	_lookup_t *entry = codeLookup(&font->table, font->header.defaultGlyph);
 	if (entry)
 		font->glyphs.defaultGlyphPosition = entry->offset;

	return 1;
}
