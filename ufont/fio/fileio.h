
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



#ifndef _FILEIO_H_
#define _FILEIO_H_




#ifdef __WIN32__
typedef FILE fileio_t;
#else

#if 0
typedef struct File File;
typedef File fileio_t;
#else

#define fileClass File		// old lib
//#define fileClass FsFile	// new lib
//#define fileClass ExFile
//#define fileClass FatFile



typedef struct fileClass fileClass;
typedef fileClass fileio_t;
#endif


#endif


#define FIO_READ		((uint8_t)0x01)
#define FIO_WRITE		((uint8_t)0x02)
#define FIO_NEW			((uint8_t)0x04)



/*
#ifdef __cplusplus
extern "C" {
#endif

*/

int32_t fio_init ();

fileio_t *fio_open (const uint8_t *filename, const uint8_t fio_flag);
void fio_close (fileio_t *fio);

size_t fio_read (fileio_t *fio, void *buffer, const size_t len);
size_t fio_write (fileio_t *fio, void *buffer, const size_t len);


size_t fio_length (fileio_t *fio);
void fio_advance (fileio_t *fio, const size_t amount);
size_t fio_seek (fileio_t *fio, const size_t pos);
size_t fio_setpos (fileio_t *fio, const size_t pos);
size_t fio_getpos (fileio_t *fio);
void fio_rewind (fileio_t *fp);
void fio_setDir (const uint8_t *dir);



/*
#ifdef __cplusplus
}
#endif*/











#endif

