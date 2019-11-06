
//  Copyright (c) Michael McElligott
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





// magic stuff happens here


#include "ufont_config.h"


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <ctype.h>

#include "pngcommon.h"
#include "fio/fileio.h"



/*
typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}__attribute__ ((packed))TRGB;		// 888


typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}__attribute__ ((packed))TRGBA;		// 8888
*/




#if 0
uint16_t blend16 (uint16_t fg, uint16_t bg, uint8_t alpha)
{
    // Split foreground into components
    uint32_t fg_r = fg >> 11;
    uint32_t fg_g = (fg >> 5) & ((1u << 6) - 1);
    uint32_t fg_b = fg & ((1u << 5) - 1);

    // Split background into components
    uint32_t bg_r = bg >> 11;
    uint32_t bg_g = (bg >> 5) & ((1u << 6) - 1);
    uint32_t bg_b = bg & ((1u << 5) - 1);

#if 1
	// Alpha blend components
	uint32_t out_r = fg_r * alpha + bg_r * (255 - alpha);
	uint32_t out_g = fg_g * alpha + bg_g * (255 - alpha);
	uint32_t out_b = fg_b * alpha + bg_b * (255 - alpha);
	
	out_r = (out_r + 1 + (out_r >> 8)) >> 8;
	out_g = (out_g + 1 + (out_g >> 8)) >> 8;
	out_b = (out_b + 1 + (out_b >> 8)) >> 8;
#else
    // Alpha blend components
    uint16_t out_r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
    uint16_t out_g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
    uint16_t out_b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;
#endif

    // Pack result
    return (uint16_t )((out_r << 11) | (out_g << 5) | out_b);
}
#endif
/*
inline void setPixel8 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//uint16_t *pixels = (uint16_t*)buffer;	
	
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint8_t*)(buffer+(y*pitch) + x) = colour;
}


// non packed 12bit
inline void setPixel12 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint16_t*)(buffer+(y*pitch) + (x<<1)) = colour; //&0x0FFF
}
*/
static inline void setPixel16 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint16_t*)(buffer+(y*pitch) + (x<<1)) = colour;
}
/*
static inline void setPixel24 (uint8_t *buffer, const int pitch, const int x, const int y, uint32_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;

	TRGB *addr = (TRGB*)(buffer+((y*pitch)+(x*3)));
	*addr = *(TRGB*)&colour;
}
*/
static inline intptr_t *getPixelAddress32 (uint8_t *frame, const int pitch, const int x, const int y)
{
	return (intptr_t*)(frame+((y*pitch)+(x<<2)));
}

static inline void readPng32To16_565 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<8;	// 5
			int g = (line[i++]&0xFC)<<3;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

/*
static inline void readPng32To16_555 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
	
	while(passCount--){	// interlacing requires multiple passes
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<7;	// 5
			int g = (line[i++]&0xF8)<<2;	// 5
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

static inline void readPng32To24 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width*3;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++, i += 4){
			setPixel24(frame, pitch, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]);
		}
	}
	}
}


static inline void readPng32To32A_cpy (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<2;
	
	while(passCount--)
	for (int y = oy; y < (const int)(oy+height); y++){
		png_read_row(*png_ptr, line, NULL);
		TRGBA *p = (TRGBA*)getPixelAddress32(frame, pitch, ox, y);
		if (!p) continue;

		int i = 0;
		for (int x = 0; x < width; x++){
			p[x].b = line[i++];
			p[x].g = line[i++];
			p[x].r = line[i++];
			p[x].a = line[i++];
		}
	}
}

inline void readPng32To8 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width;
		
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xE0);
			int g = (line[i++]&0xE0)>>3;
			int b = (line[i++]&0xC0)>>6;
			setPixel8(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}

inline void readPng32To12 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
		
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF0)<<4;
			int g = (line[i++]&0xF0);
			int b = (line[i++]&0xF0)>>4;
			setPixel12(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}
*/



static fileio_t *png_fio = NULL;


static inline int readPngToFrame (uint8_t *frame, png_structp *png_ptr, int bpp, int width, int height, int ox, int oy, int passCount)
{
	uint8_t line[(width * 4) + 4];		// enough for a single RGBA line

	int ret = 1;
	switch (bpp){
	  //case BPP_8:  readPng32To8      (frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  //case BPP_12: readPng32To12     (frame, png_ptr, width, height, line, ox, oy, passCount); break; // non packed 12bit
	  //case BPP_15: readPng32To16_555 (frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case BPP_16: readPng32To16_565(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  //case BPP_24: readPng32To24     (frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  //case BPP_32: readPng32To32A_cpy(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  //default: printf("libpng: format %i not implemented\r\n", bpp); ret = 0;
	};

	return ret;
}

void png_read_dataCb (png_structp png_ptr, png_bytep data, png_size_t length)
{

	//printf("png_read_dataCb %p %i \r\n", data, length);
	//File *file = (File*)png_ptr->io_ptr;

	png_size_t check = (png_size_t)fio_read(png_fio, data, length);
	if (check != length)
		printf("png read error, length:%i %i\r\n", check, length);
}

static int32_t loadPng (uint8_t *frame, const int flags, const char *filename, const int32_t ox, const int32_t oy, uint32_t *width, uint32_t *height, uint32_t *bpp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;


	png_fio = fio_open((uint8_t*)filename, FIO_READ);
	if (!png_fio) return 0;

	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fio_close(png_fio);
		return 0;
	}

	png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fio_close(png_fio);
		return 0;
	}

   if (setjmp(png_jmpbuf(png_ptr))){
      // Free all of the memory associated with the png_ptr and info_ptr 
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  fio_close(png_fio);
      // if we get here, we had a problem reading the file
      return 0;
   }

	//png_init_io(png_ptr, png_fio); 
	png_set_read_fn(png_ptr, (png_voidp)&png_fio, png_read_dataCb);
	 
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);

	int passCount = 1;
	int bit_depth = 0, color_type = 0, interlace_type = 0;
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)width, (png_uint_32*)height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	if (bpp) *bpp = bit_depth;
	
	if (frame != NULL){
		if (interlace_type == PNG_INTERLACE_ADAM7)
			passCount = png_set_interlace_handling(png_ptr);
		//printf("interlace_type %i, ct:%i, %i %i\n", interlace_type, passCount, *width, *height);

	
		if (bit_depth == 16)
			png_set_strip_16(png_ptr);
		png_set_packing(png_ptr);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_ptr);

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		
		/*if (flags&LOAD_RESIZE){
			if (!_resizeFrame(frame, *width, *height, 0))
				return 0;
		}*/
	}

	int ret = 1;
	if (frame)
		ret = readPngToFrame(frame, &png_ptr, flags, *width, *height, ox, oy, passCount);
	
	if (!ret)
		png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	fio_close(png_fio);
	return ret;
}


int32_t png_read (const char *filename, uint8_t *buffer, const int32_t bufferbpp, const int32_t ox, const int32_t oy)
{
	uint32_t width = 0;
	uint32_t height = 0;
	
	return loadPng(buffer, bufferbpp, filename, ox, oy, &width, &height, NULL);
}

int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp)
{
	*width = 0;
	*height = 0;
	*filebpp = 0;

	return loadPng(NULL, 0, filename, 0, 0, width, height, filebpp);
}


