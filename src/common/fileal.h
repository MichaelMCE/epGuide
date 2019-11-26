
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





#ifndef _FILEAL_H_
#define _FILEAL_H_


typedef struct{
	uint32_t tlines;
	unsigned char **line;
	unsigned char *data;
}TASCIILINE;


TASCIILINE *readFileW (const wchar_t *filename);
TASCIILINE *readFileA (const char *filename);
void freeASCIILINE (TASCIILINE *al);


#endif

