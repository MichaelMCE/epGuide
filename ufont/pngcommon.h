
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


#ifndef _PNGCOMMON_H_
#define _PNGCOMMON_H_


#include "ufont_config.h"


//#define BPP_8		8
//#define BPP_12		12
//#define BPP_15		15
#define BPP_16		16
//#define BPP_24		24
//#define BPP_32		32



#include "libpng/png.h"

int32_t png_read (const char *filename, uint8_t *buffer, const int32_t bufferbpp, const int32_t ox, const int32_t oy);
int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp);


#if 0
int32_t png_read (const char *filename, uint8_t *buffer, const int32_t bufferbpp, const int32_t ox, const int32_t oy) {return 0;}
int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp) {return 0;}
#endif



#endif

