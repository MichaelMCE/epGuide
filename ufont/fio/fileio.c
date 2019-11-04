
//#pragma GCC optimize ("-Ofast")

#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __WIN32__
#include <Arduino.h>
//#include "SdFat.h"
#include "SdFs.h"
#endif

#include "fileio.h"



#ifdef __WIN32__


int32_t fio_init ()
{
	// do nothing
	return 1;
}


fileio_t *fio_open (const uint8_t *filename, const uint8_t fio_flag)
{
	fileio_t *file = NULL;
	
	if (fio_flag&FIO_NEW)
		file = fopen((char*)filename, "wb");
	else if (fio_flag&FIO_WRITE)
		file = fopen((char*)filename, "r+b");
	else if (fio_flag&FIO_READ)
		file = fopen((char*)filename, "rb");
	return file;
}


void fio_close (fileio_t *fp)
{
	fclose(fp);
}

size_t fio_read (fileio_t *fp, void *buffer, const size_t len)
{
	return fread(buffer, 1, len, fp);
}

size_t fio_write (fileio_t *fp, void *buffer, const size_t len)
{
	return fwrite(buffer, 1, len, fp);
}

size_t fio_length (fileio_t *fp)
{
	fpos_t pos;
	
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	size_t fl = ftell(fp);
	fsetpos(fp, &pos);
	
	return fl;
}

void fio_rewind (fileio_t *fp)
{
	rewind(fp);
}

size_t fio_seek (fileio_t *fp, const size_t pos)
{
	return fseek(fp, pos, SEEK_SET);
}

size_t fio_getpos (fileio_t *fp)
{
	fpos_t pos;
	fgetpos(fp, &pos);
	
	return (size_t)pos;
}

size_t fio_setpos (fileio_t *fp, const size_t pos)
{
	return fio_seek(fp, pos);
}

void fio_advance (fileio_t *fp, const size_t amount)
{
	fpos_t pos;
	fgetpos(fp, &pos);
	pos += (fpos_t)amount;
	fsetpos(fp, &pos);
}


#else

static SdFatSdioEX sd;
static File file;

//static SdFs sd;
//static FsFile file;

//static SdExFat sd;
//static ExFile file;

//static SdFat sd;
//static FatFile file;

// only one file may be open at any given time
static bool fileIsOpen = 0;

int32_t fio_init ()
{
	if (!sd.begin()){
	//if (!sd.begin(SdioConfig(FIFO_SDIO))){
		Serial.println("sd.begin(): failed");
		return false;
	}

	if (fileIsOpen){
		fio_rewind(&file);
		fio_close(&file);
	}
	sd.chvol();
	
	return true;
}

fileio_t *fio_open (const uint8_t *filename, const uint8_t fio_flag)
{
	if (fileIsOpen){
		Serial.println("fio_open(): file already open");
		return NULL;
	}
			
	fileIsOpen = file.open((const char*)filename, O_RDONLY);
	if (fileIsOpen){
		return &file;
	}else{
		return NULL;
	}
}

void fio_close (fileio_t *fp)
{
	fp->close();
	fileIsOpen = 0;
}

void fio_rewind (fileio_t *fp)
{
	fp->rewind();
}

size_t fio_read (fileio_t *fp, void *buffer, const size_t len)
{
	return (size_t)fp->read(buffer, len) == len;
}

size_t fio_length (fileio_t *fp)
{
	return fp->fileSize();
}

size_t fio_seek (fileio_t *fp, size_t pos)
{
	//FatPos_t fatpos;
	//fatpos.position = pos;
	//fp->setpos(&fatpos);
	
	fp->seekSet(pos);
	
	return 0;
}

size_t fio_getpos (fileio_t *fp)
{
	return fp->curPosition();
}

size_t fio_setpos (fileio_t *fp, const size_t pos)
{
	return fio_seek(fp, pos);
}

void fio_advance (fileio_t *fp, const size_t amount)
{
	fp->seekCur(amount);
}

void fio_setDir (const uint8_t *dir)
{
	sd.chdir((const char*)dir);
}


#endif
