
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



#ifndef _ARTC_H_
#define _ARTC_H_




#define ARTC_STATE_INVALID			0x00
#define ARTC_STATE_HASPATH			0x01
#define ARTC_STATE_HASSURFACE		0x02
#define ARTC_STATE_HASMETRICS		0x04
#define ARTC_STATE_HASSCALED		0x08
#define ARTC_STATE_CANNOTREAD		0x10
#define ARTC_STATE_HASCUSTMETRICS	0x20	/* best fit custom metrics */
#define ARTC_STATE_ISRESIZED		0x40


#define ARTC_SCALED_MAX				8
#define ARTC_BUCKET_SIZE			32



typedef struct{
	uint32_t hash;
	int id;
}TART_HASHTABLE;

typedef struct{
	TFRAME *surface;
	double scale;
	int opacity;	// 0 to 100, -1 is default
	int width;		// width after scaling. should equal ::surface->width
	int height;		// height after scaling. should equal ::surface->height
}TART_SCALED;

typedef struct {
	int id;
	uint32_t state;		// hasPath, hasSurface, hasMetrics, canNotRead
	//uint32_t hash;
	
	int acquireRefCt;		// refCt inc'd per surface request, dec'd per release
	int multiRefCt;
	
	TFRAME *surface;		// original image as read from file
	TART_SCALED *scaled[ARTC_SCALED_MAX];	// scaled copies of ::surface
	
	struct{
		int width;
		int height;
	}bestfit;
	
	struct{
		int width;
		int height;
	}resize;
	
	struct{
		int width;
		int height;
		int bpp;
		int type;			// libmylcd image type IMG_XXX
		int hasAlpha;
	}image;

	int imageType;
	
	wchar_t *path;
}TART_ITEM;

typedef struct {
	TLISTITEM *items;
	int bucketSize;		// number of artworks per bucket
	TMLOCK *hLock;		// per branch/bucket lock
}TART_BRANCH;

typedef struct {
	TART_BRANCH **buckets;		// branch/bucket
	int branchTotal;			// number of art buckets
	int branchInsertPos;		// index to current bucket [list] where additions are placed

	TART_HASHTABLE *hashTable;	// hash to id lookup table
	int hashTableSize;
	
	TMLOCK *hLock;
	
	int lastReadId;
	uint32_t idSrc;
	wchar_t *pathPrefix;		// prefix added to each/every path
	
	void *hSurfaceLib;			// handle to surface/frame library (mylcd::hw)
}TARTMANAGER;


TARTMANAGER *artManagerNew (void *hSurfaceLib);		// hLib: surface library
void artManagerDelete (TARTMANAGER *am);

// set a global art path prefix, prepended to every path
int artManagerSetPathPrefix (TARTMANAGER *am, const wchar_t *prefix);


// remove non acquired image data from memory
// data in acquired state will not be flushed
int artManagerFlush (TARTMANAGER *am);


// change the path to something else
int artManagerImageSetPath (TARTMANAGER *am, const int id, const wchar_t *path);
wchar_t *artManagerImageGetPath (TARTMANAGER *am, const int id);
char *artManagerImageGetPath8 (TARTMANAGER *am, const int id);

// adds an art to the cache, returns a reference Id
int artManagerImageAdd (TARTMANAGER *am, const wchar_t *path);
int artManagerImageAddEx (TARTMANAGER *am, const wchar_t *path, const int width, const int height);	// with best fit enabled
int artManagerImageDelete (TARTMANAGER *am, const int id);

 
TFRAME *artManagerImageAcquire (TARTMANAGER *am, const int id);
TFRAME *artManagerImageAcquireEx (TARTMANAGER *am, const int id, const double scale, const int opacity);
TFRAME *artManagerImageAcquireScaled (TARTMANAGER *am, const int id, const double scale);
int artManagerImageRelease (TARTMANAGER *am, const int id);

// ensure an image is loaded before acquiring (does not require releasing)
int artManagerImagePreload (TARTMANAGER *am, const int id);

TFRAME *artManagerImageClone (TARTMANAGER *am, const int id);	// free with lDeleteFrame()

int artManagerImageGetMetrics (TARTMANAGER *am, const int id, int *width, int *height);
int artManagerImageFlush (TARTMANAGER *am, const int id);
int artManagerImageResize (TARTMANAGER *am, const int id, const int width, const int height);
int artManagerSetCustomMetrics (TARTMANAGER *am, const int id, const int width, const int height);

void artManagerDumpStats (TARTMANAGER *am);
int artManagerDumpIds (TARTMANAGER *am);
int *artManagerGetIds (TARTMANAGER *am, int *count);


size_t artManagerMemUsage (TARTMANAGER *am);
int artManagerSurfaceCount (TARTMANAGER *am);
int artManagerCount (TARTMANAGER *am);
int artManagerUnreleasedCount (TARTMANAGER *am);

#endif


