
// libmylcd - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#include "../common.h"


size_t strcGetStoredSize (TFRAMESTRINGCACHE *strc)
{
	size_t sumSize = 0;
	for (int i = 0; i < strc->totalAdded; i++){
		if (strc->cache[i].hash && strc->cache[i].img)
			sumSize += strc->cache[i].img->frameSize;
	}
	return sumSize;
}


void strcFlush (TFRAMESTRINGCACHE *strc)
{
	for (int i = 0; i < strc->totalAdded; i++){
		strc->cache[i].hash = 0;
		if (strc->cache[i].img)
			lDeleteFrame(strc->cache[i].img);
		strc->cache[i].img = NULL;
	}
	strc->totalAdded = 0;
}

TFRAME *strcFindString (TFRAMESTRINGCACHE *strc, const unsigned int hash)
{
	if (!hash) return 0;
	
	for (int i = strc->totalAdded-1; i >= 0; i--){
		if (strc->cache[i].hash == hash){
			if (strc->cache[i].img){
				return strc->cache[i].img;
			}else{
				strc->cache[i].hash = 0;
				return NULL;
			}
		}
	}
	return NULL;
}

int strcAddString (TFRAMESTRINGCACHE *strc, TFRAME *img, const unsigned int hash)
{
	if (!img || !hash) return -1;
	
	if (strc->totalAdded >= STRINGIMAGECACHESIZE){
		if (strc->cache[0].img){
			lDeleteFrame(strc->cache[0].img);
			strc->cache[0].img = NULL;
		}
		strc->cache[0].hash = 0;

		for (int i = 1; i < strc->totalAdded; i++)
			my_memcpy(&strc->cache[i-1], &strc->cache[i], sizeof(TFRAMESTRINGITEM));
	}else{
		strc->totalAdded++;
	}

	strc->cache[strc->totalAdded-1].img = img;
	strc->cache[strc->totalAdded-1].hash = hash;
	//strc->cache[strc->totalAdded-1].ftime = 0.0;

	return strc->totalAdded;
}

void strcFree (TFRAMESTRINGCACHE *strc)
{
	strcFlush(strc);
	my_free(strc);
}

TFRAMESTRINGCACHE *strcNew (void *hSurfaceLib)
{
	TFRAMESTRINGCACHE *strc = my_calloc(1, sizeof(TFRAMESTRINGCACHE));
	if (strc) strc->hSurfaceLib = hSurfaceLib;
	return strc;
}

