
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





#ifndef _ROTATE_H_
#define _ROTATE_H_

#define SCALE_BILINEAR		1
#define SCALE_BICUBIC		2
#define SCALE_NEIGHBOUR		3
#define SCALE_CLEANDES		(0x0100)

#define ROTATE_BILINEAR		1
#define ROTATE_BICUBIC		2
#define ROTATE_NEIGHBOUR	3


int transRotate (TFRAME *src, TFRAME *des, const int angle, const int type);
int transScale (TFRAME *src, TFRAME *des, const int width, const int height, const int desX, const int desY, const int type);

void transBrightness (TFRAME *src, const int brightness);
void transContrast (TFRAME *src, const int contrast);
void transPixelize (TFRAME *src, const int size);
void transSharpen (TFRAME *src, const float value);
void transGrayscale (TFRAME *src);
void transBlur (TFRAME *src, const int radius);



#define funcname		__FILE__
#define linenumber		(__LINE__)

MYLCD_EXPORT void * my_Malloc (size_t size, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void * my_Calloc (size_t nelem, size_t elsize, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void * my_Realloc (void *ptr, size_t size, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void my_Free (void *ptr, const char *func, const int line);
MYLCD_EXPORT char * my_Strdup (const char *str, const char *func, const int line)
	__attribute__((nonnull(1)));
MYLCD_EXPORT wchar_t * my_Wcsdup (const wchar_t *str, const char *func, const int line)
	__attribute__((nonnull(1)));
MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
	__attribute__((nonnull(1, 2)));
	

#endif

