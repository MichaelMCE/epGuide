
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




#include "../common.h"



#define artIdToBranchId(n) (((n)>>16)&0xFFFF)





static inline int abLock (TART_BRANCH *ab)
{
	return lockWait(ab->hLock, INFINITE);
}

static inline void abUnlock (TART_BRANCH *ab)
{
	lockRelease(ab->hLock);
}

static inline int amLock (TARTMANAGER *am)
{
	return lockWait(am->hLock, INFINITE);
}

static inline void amUnlock (TARTMANAGER *am)
{
	lockRelease(am->hLock);
}

static inline int hashTableFind (TARTMANAGER *am, const uint32_t hash)
{
	for (int i = 0; am->hashTable[i].hash && i < am->hashTableSize; i++){
		if (am->hashTable[i].hash == hash)
			return am->hashTable[i].id;
	}
	return 0;
}

static inline int hashTableGrow (TARTMANAGER *am, const int growBy)
{
	const int newSize = am->hashTableSize + growBy;
	am->hashTable = my_realloc(am->hashTable, newSize * sizeof(TART_HASHTABLE));
	if (am->hashTable){
		for (int i = am->hashTableSize; i < newSize; i++){
			am->hashTable[i].hash = 0;
			am->hashTable[i].id = 0;
		}
		return am->hashTableSize = newSize;
	}
	return 0;
}

static inline void hashTableAdd (TARTMANAGER *am, const uint32_t hash, const int id)
{
	for (int i = 0; i < am->hashTableSize; i++){
		if (!am->hashTable[i].hash){
			am->hashTable[i].hash = hash;
			am->hashTable[i].id = id;
			return;
		}
	}
	
	if (hashTableGrow(am, 16))
		hashTableAdd(am, hash, id);
}

static inline TART_BRANCH *amGetBranch (TARTMANAGER *am, const int idx)
{
	if (idx < am->branchTotal)
		return am->buckets[idx];
	//else
		//printf("amGetBranch FAILED: idx:%i, buckets:%i\n", idx, am->branchTotal);
	return NULL;
}

static inline TLISTITEM *amGetList (TARTMANAGER *am, const int id)
{
//	printf("amGetList %X\n", id);
	return amGetBranch(am, artIdToBranchId(id))->items;
}

static inline TART_ITEM *_artManagerIdToItem (TART_BRANCH *ab, const int id)
{
	TLISTITEM *item = ab->items;
	while(item){
		TART_ITEM *ai = listGetStorage(item);
		if (ai && ai->id == id) return ai;
		item = listGetNext(item);
	}
	
	//printf("_artManagerIdToItem: id not found %i %i\n", id>>16, id&0xFFFF);
	return NULL;
}

static inline TART_ITEM *artManagerIdToItem (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = NULL;
	TART_BRANCH *ab = amGetBranch(am, artIdToBranchId(id));
	if (abLock(ab)){
		ai = _artManagerIdToItem(ab, id);
		abUnlock(ab);
	}
	return ai;
}

static inline void artManagerItemDeleteSurfaceData (TART_ITEM *ai)
{
	lDeleteFrame(ai->surface);
	ai->surface = NULL;
}

static inline void artManagerItemDeleteScaledData (TART_ITEM *ai)
{
	if (ai->state&ARTC_STATE_HASSCALED){
		ai->state &= ~ARTC_STATE_HASSCALED;
		
		for (int i = 0; i < ARTC_SCALED_MAX; i++){
			TART_SCALED *as = ai->scaled[i];
			if (as){
				ai->scaled[i] = NULL;
				lDeleteFrame(as->surface);
				my_free(as);
			}
		}
	}
}

static inline int extMatch (const wchar_t *string, const wchar_t *ext)
{
	const int len = wcslen(string)-4;

	for (int i = len; i < len+1 && string[i]; i++){
		if (towlower(string[i]) == towlower(ext[0])){
			if (towlower(string[i+1]) == towlower(ext[1])){
				if (towlower(string[i+2]) == towlower(ext[2])){
					if (towlower(string[i+3]) == towlower(ext[3]))
						return 1;
				}
			}
		}
	}
	return 0;
}

static inline int artManagerItemReadMetrics (TART_ITEM *ai)
{
	if (ai->state&ARTC_STATE_HASPATH){
		int gotMetrics = 0;
		//wprintf(L"artManagerItemReadMetrics %X %i %i '%s'\n", ai->id, ai->state&ARTC_STATE_HASPATH, ai->state&ARTC_STATE_HASMETRICS, ai->path);

		if (extMatch(ai->path, L".png")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_PNG, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics){
				ai->image.type = IMG_PNG;
				ai->image.hasAlpha = 1;
			}
		}else if (extMatch(ai->path, L".jpg") || extMatch(ai->path, L".jpeg") || extMatch(ai->path, L".jpe")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_JPG, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_JPG;
			
		}else if (extMatch(ai->path, L".bmp")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_BMP, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_BMP;

		}else if (extMatch(ai->path, L".exe") || extMatch(ai->path, L".ico") || extMatch(ai->path, L".dll")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_ICO, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics){
				ai->image.type = IMG_ICO;
				ai->image.hasAlpha = 1;
			}
		}else if (extMatch(ai->path, L".tga")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_TGA, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_TGA;

		}else if (extMatch(ai->path, L".gif")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_GIF, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_GIF;

		}else if (extMatch(ai->path, L".psd")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_PSD, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_PSD;

		}else if (extMatch(ai->path, L".pgm")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_PGM, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_PGM;
			
		}else if (extMatch(ai->path, L".ppm")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_PPM, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_PPM;

		}else if (extMatch(ai->path, L".pbm")){
			gotMetrics = lImageGetMetrics(ai->path, IMG_PBM, &ai->image.width, &ai->image.height, NULL);
			if (gotMetrics) ai->image.type = IMG_PBM;
			
		}else{
			if ((gotMetrics=lImageGetMetrics(ai->path, IMG_JPG, &ai->image.width, &ai->image.height, NULL))){
				ai->image.type = IMG_JPG;
			}else if ((gotMetrics=lImageGetMetrics(ai->path, IMG_PNG, &ai->image.width, &ai->image.height, NULL))){
				ai->image.type = IMG_PNG;
				ai->image.hasAlpha = 1;
			}
		}
			
		if (gotMetrics){
			ai->state |= ARTC_STATE_HASMETRICS;
			ai->state &= ~ARTC_STATE_CANNOTREAD;
			return 1;
		}else{
			ai->state |= ARTC_STATE_CANNOTREAD;
		}
	}
	return 0;
}


static inline int artManagerItemReadPixels (TARTMANAGER *am, TART_ITEM *ai)
{

	if (ai->state&ARTC_STATE_CANNOTREAD)
		return 0;

	if (ai->state&ARTC_STATE_HASPATH /*&& ai->image.type != IMG_BMP*/){
		//wprintf(L"@@ amReadPixels %i %i, '%s'\n", ai->id>>16, ai->id&0xFFF, ai->path);
		//wprintf(L"am Read %i, '%s'\n", ai->id&0xFFF, ai->path);
		//wprintf(L"@@ artManagerItemReadPixels %i '%s'\n",  ai->id, ai->path);
		
		TFRAME *surface = lNewImage(am->hSurfaceLib, ai->path, ai->image.bpp);
		if (surface){
			if (ai->state&ARTC_STATE_ISRESIZED){
				TFRAME *tmp = lCloneFrame(surface);
				if (tmp){
					if (lResizeFrame(surface, ai->resize.width, ai->resize.height, 0) == 1){
						com_copyAreaScaled(tmp, surface, 0, 0, tmp->width, tmp->height, 0, 0, ai->resize.width, ai->resize.height);
						ai->resize.width = surface->width;
						ai->resize.height = surface->height;
					}
					lDeleteFrame(tmp);
				}
			}
			
			if (ai->state&ARTC_STATE_HASSURFACE){
				artManagerItemDeleteSurfaceData(ai);
				artManagerItemDeleteScaledData(ai);
			}

			if (ai->state&ARTC_STATE_HASCUSTMETRICS){
				if (!(ai->state&ARTC_STATE_HASMETRICS))
					artManagerItemReadMetrics(ai);

				com_imageBestFit(ai->bestfit.width, ai->bestfit.height, surface->width, surface->height, &ai->image.width, &ai->image.height);
				
				TFRAME *img = lNewFrame(am->hSurfaceLib, ai->image.width, ai->image.height, ai->image.bpp);
				if (img){
					//printf("ai->image.type %i %i\n", ai->image.type, IMG_PNG);
					if (ai->image.hasAlpha)
						com_copyAreaScaled(surface, img, 0, 0, surface->width, surface->height, 0, 0, img->width, img->height);
					else
						transScale(surface, img, img->width, img->height, 0, 0, SCALE_BILINEAR);

					lDeleteFrame(surface);
					ai->surface = img;
				}else{	// unlikely
					ai->surface = surface;
				}
			}else{
				ai->surface = surface;
			}

			ai->state |= ARTC_STATE_HASSURFACE;
			ai->state &= ~ARTC_STATE_CANNOTREAD;

			ai->image.width = ai->surface->width;
			ai->image.height = ai->surface->height;
			ai->state |= ARTC_STATE_HASMETRICS;

			am->lastReadId = ai->id;
			return surface->width * surface->height;
		
		}else{
			ai->state |= ARTC_STATE_CANNOTREAD;
		}
	}
	return 0;
}

static inline void artManagerItemSetCustomMetrics (TART_ITEM *ai, const int width, const int height)
{
	ai->state |= ARTC_STATE_HASCUSTMETRICS;
	ai->bestfit.width = width;
	ai->bestfit.height = height;
}

int artManagerSetCustomMetrics (TARTMANAGER *am, const int id, const int width, const int height)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai)
		artManagerItemSetCustomMetrics(ai, width, height);
	return (ai != NULL);
}

static inline int artManagerItemGetMetrics (TART_ITEM *ai, int *width, int *height)
{
	if (width) *width = 0;
	if (height) *height = 0;

	//wprintf(L"%X %i %i, %i %i\n", ai->id, ai->image.width, ai->image.height, ai->state&ARTC_STATE_HASMETRICS, ai->state&ARTC_STATE_HASPATH);

	if (!(ai->state&ARTC_STATE_HASMETRICS)){
		if (artManagerItemReadMetrics(ai)){
			if (ai->state&ARTC_STATE_HASCUSTMETRICS){
				int w, h;
				com_imageBestFit(ai->bestfit.width, ai->bestfit.height, ai->image.width, ai->image.height, &w, &h);
				ai->image.width = w;
				ai->image.height = h;
			}
		}
	}

	if (ai->state&ARTC_STATE_HASMETRICS){
		if (width) *width = ai->image.width;
		if (height) *height = ai->image.height;
		return ai->id;
	}
	
	//if (ai->image.type != IMG_ICO)
		__mingw_wprintf(L"artManagerImageGetMetrics failed 0x%X: %i %i, '%ls'\n", ai->id, ai->image.width, ai->image.height, ai->path);

	return 0;
}

int artManagerImageResize (TARTMANAGER *am, const int id, const int width, const int height)
{
	int ret = 0;

	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (artManagerImageAcquire(am, id)){
			if (ai->acquireRefCt != 1){
				//printf("artManagerImageResize: can not resize %X, surface in use (ct:%i/%i)\n", id, ai->acquireRefCt, ai->multiRefCt);
				artManagerImageRelease(am, id);
				return ret;
			}
			artManagerItemDeleteScaledData(ai);
		
			if (ai->state&ARTC_STATE_HASCUSTMETRICS)
				ai->state &= ~ARTC_STATE_HASCUSTMETRICS;
		
			TFRAME *tmp = lCloneFrame(ai->surface);
			if (tmp){
				if (lResizeFrame(ai->surface, width, height, 0) == 1){
					com_copyAreaScaled(tmp, ai->surface, 0, 0, tmp->width, tmp->height, 0, 0, width, height);
					ai->state |= ARTC_STATE_ISRESIZED;
					ai->resize.width = ai->surface->width;
					ai->resize.height = ai->surface->height;
				}
				lDeleteFrame(tmp);

				ai->image.width = ai->surface->width;
				ai->image.height = ai->surface->height; 
				//printf("%i %i\n", ai->image.width, ai->image.height);
				ret = 1;
			}
			artManagerImageRelease(am, id);
		}
	}
	return ret;
}

TFRAME *artManagerImageClone (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (!(ai->state&ARTC_STATE_HASSURFACE))
			artManagerItemReadPixels(am, ai);

		if (ai->state&ARTC_STATE_HASSURFACE)
			return lCloneFrame(ai->surface);
	}
	return NULL;
}

int artManagerImagePreload (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	//wprintf(L"artManagerImageAcquire %X %p %i %i '%s'\n", id, ai, ai->state&ARTC_STATE_HASSURFACE, ai->state&ARTC_STATE_HASPATH, ai->path);
	if (ai){
		if (!(ai->state&ARTC_STATE_HASSURFACE))
			artManagerItemReadPixels(am, ai);
		else
			return 1;
		return ai->state&ARTC_STATE_HASSURFACE;
	}
	return 0;
}

TFRAME *artManagerImageAcquire (TARTMANAGER *am, const int id)
{
	TART_BRANCH *ab = amGetBranch(am, artIdToBranchId(id));
	if (!ab) return 0;

	if (abLock(ab)){
		TART_ITEM *ai = artManagerIdToItem(am, id);
		//wprintf(L"artManagerImageAcquire %X %p %i %i '%s'\n", id, ai, ai->state&ARTC_STATE_HASSURFACE, ai->state&ARTC_STATE_HASPATH, ai->path);
		if (ai){
			if (!(ai->state&ARTC_STATE_HASMETRICS))
				artManagerItemGetMetrics(ai, NULL, NULL);
			
			if (!(ai->state&ARTC_STATE_HASSURFACE))
				artManagerItemReadPixels(am, ai);
    	
			if (ai->state&ARTC_STATE_HASSURFACE){
				ai->acquireRefCt++;
				abUnlock(ab);
				return ai->surface;
			}
		}
		abUnlock(ab);
	}
	//printf("artManagerImageAcquire %X NULL\n", id);
	return NULL;
}

static inline TFRAME *_artManagerImageAcquireEx (TARTMANAGER *am, const int id, const double scale, const int opacity)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	//wprintf(L"imgManagerImageAcquireEx %X %p %i %i %i '%s'\n", id, ai, ai->state&ARTC_STATE_HASMETRICS, ai->state&ARTC_STATE_HASSURFACE, ai->state&ARTC_STATE_HASPATH, ai->path);
	if (!ai) return NULL;
		
	if (!(ai->state&ARTC_STATE_HASMETRICS))
		artManagerItemGetMetrics(ai, NULL, NULL);
	
	if (!(ai->state&ARTC_STATE_HASSURFACE))
		artManagerItemReadPixels(am, ai);

	if (ai->state&ARTC_STATE_HASSURFACE){
		ai->acquireRefCt++;
			
		const int w = ai->image.width * scale;
		const int h = ai->image.height * scale;

		// return original surface when scale of 1.0 is requested
		if (w == ai->image.width && h == ai->image.height)
			return ai->surface;
			
		// check if we have this scale already, if so return it
		if (ai->state&ARTC_STATE_HASSCALED){
			for (int i = 0; i < ARTC_SCALED_MAX; i++){
				TART_SCALED *as = ai->scaled[i];
				if (as){
					if (as->width == w && as->height == h && as->opacity == opacity){
						//printf("amImageAcquireScaled %X, %i %i %i, %.3f, %i %i %i\n", id, i, w, h, scale, as->width, as->height, opacity);
						return as->surface;
					}
				}
			}
		} 

		// create a new surface for this scale then return it
		for (int i = 0; i < ARTC_SCALED_MAX; i++){
			TART_SCALED *as = ai->scaled[i];
			if (as == NULL){
				as = my_calloc(1, sizeof(TART_SCALED));
				if (as){
					as->surface = lNewFrame(am->hSurfaceLib, w, h, /*ai->surface->bpp*/LFRM_BPP_32A);
					if (as->surface){
						if (opacity == 100 || opacity <= 0){
							//printf("ai->image.type %i %i\n", ai->image.type, IMG_PNG);
							if (ai->image.hasAlpha)
								com_copyAreaScaled(ai->surface, as->surface, 0, 0, ai->surface->width, ai->surface->height, 0, 0, w, h);
							else
								transScale(ai->surface, as->surface, w, h, 0, 0, SCALE_BILINEAR);								
						}else{
							const float opacityA = (float)opacity/100.0f;
							com_drawImageScaledOpacity(ai->surface, as->surface, 0, 0, ai->surface->width, ai->surface->height, 0, 0, w, h, opacityA, opacityA*0.65f);
						}

						as->width = w;
						as->height = h;
						as->scale = scale;
						as->opacity = opacity;
						ai->state |= ARTC_STATE_HASSCALED;
						ai->scaled[i] = as;
						return as->surface;
					}
					my_free(as);
				}
			}
		}
			
		// we've failed...
		ai->acquireRefCt--;
	}

	//printf("artManagerImageAcquire %X NULL\n", id);
	return NULL;
}

TFRAME *artManagerImageAcquireEx (TARTMANAGER *am, const int id, const double scale, const int opacity)
{
	TFRAME *ret = NULL;
	TART_BRANCH *ab = amGetBranch(am, artIdToBranchId(id));

	if (ab && abLock(ab)){
		ret = _artManagerImageAcquireEx(am, id, scale, opacity);
		abUnlock(ab);
	}
	return ret;
}

TFRAME *artManagerImageAcquireScaled (TARTMANAGER *am, const int id, const double scale)
{
	return artManagerImageAcquireEx(am, id, scale, 100);
}

int artManagerImageRelease (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (ai->acquireRefCt > 0){
			ai->acquireRefCt--;
			return 1;
		}
	}
	return 0;
}

static inline int artManagerImageItemFlush (TART_BRANCH *ab, TART_ITEM *ai)
{
	if (ai->state&ARTC_STATE_HASSURFACE){
		if (ai->acquireRefCt < 1){
			ai->acquireRefCt = 0;
			ai->state &= ~ARTC_STATE_HASSURFACE;
			
			//wprintf(L"## imgManagerImageItemFlush %i '%s'\n",  ai->id, ai->path);
			//wprintf(L"## amFlush %i %i, '%s'\n", ai->id>>16, ai->id&0xFFF, ai->path);
			
			artManagerItemDeleteSurfaceData(ai);
			artManagerItemDeleteScaledData(ai);

			//wprintf(L"am Flush %i '%s'\n", ai->id&0xFFF, ai->path);
			return 1;
		}
	}
	return 0;
}

int artManagerImageFlush (TARTMANAGER *am, const int id)
{
	TART_BRANCH *ab = amGetBranch(am, artIdToBranchId(id));
	if (!ab) return 0;

	if (abLock(ab)){
		TART_ITEM *ai = artManagerIdToItem(am, id);
		if (ai) artManagerImageItemFlush(ab, ai);
		abUnlock(ab);
	}
	return 1;
}

int artManagerFlush (TARTMANAGER *am)
{
	int ct = 0;
	
	if (amLock(am)){
		for (int i  = 0; i <= am->branchInsertPos; i++){
			TART_BRANCH *ab = amGetBranch(am, i);
			if (abLock(ab)){
				TLISTITEM *item = ab->items;
				while(item){
					TART_ITEM *ai = listGetStorage(item);
					if (ai && ai->id != am->lastReadId)
						ct += artManagerImageItemFlush(ab, ai);
					item = listGetNext(item);
				}
				abUnlock(ab);
			}
		}
		amUnlock(am);
	}
	return ct;
}

int artManagerImageGetMetrics (TARTMANAGER *am, const int id, int *width, int *height)
{
	if (width) *width = 0;
	if (height) *height = 0;
			
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai)
		return artManagerItemGetMetrics(ai, width, height);

	return 0;
}

static inline int artManagerGenerateId (TARTMANAGER *am)
{
	return (++am->idSrc)&0xFFFF;
}

static inline void artManagerItemDeleteData (TART_ITEM *ai)
{
	if (ai->state&ARTC_STATE_HASPATH)
		my_free(ai->path);
	if (ai->state&ARTC_STATE_HASSURFACE)
		artManagerItemDeleteSurfaceData(ai);
	artManagerItemDeleteScaledData(ai);

	my_free(ai);
}

static inline void hashRemove (TARTMANAGER *am, const wchar_t *path)
{
	const uint32_t hash = com_getHashW(path);
	for (int i = 0; am->hashTable[i].hash && i < am->hashTableSize; i++){
		if (am->hashTable[i].hash == hash){
			am->hashTable[i].id = 0;
			am->hashTable[i].hash = 0;
			return;
		}
	}	
}


// fix this.
// consider whether to delete bucket or delete image data only while leaving paths' and metrics in place
#if 0
static inline int artManagerItemDelete (TARTMANAGER *am, const int id, const int force)
{
	printf("artManagerImageDelete %i %X\n", artIdToBranchId(id), id);
	
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		ai->id = 0;
		ai->state = ARTC_STATE_CANNOTREAD;

		uint32_t hash = com_getHashW(ai->path);
		for (int i = 0; am->hashTable[i].hash && i < am->hashTableSize; i++){
			if (am->hashTable[i].hash == hash){
				am->hashTable[i].id = 0;
				am->hashTable[i].hash = 0;
			}
		}
		
	}

	return 1;
}

#else
static inline int artManagerItemDelete (TARTMANAGER *am, const int id, const int force)
{
	TLISTITEM *item = amGetList(am, id);
	while(item){
		TART_ITEM *ai = listGetStorage(item);
		if (ai && ai->id == id){
			//printf("%X: %i %i\n", id, ai->multiRefCt, ai->acquireRefCt);
			
			if (ai->state&ARTC_STATE_HASPATH){
				if (!ai->multiRefCt && !ai->acquireRefCt)
					hashRemove(am, ai->path);
			}

			if ((!ai->multiRefCt && !ai->acquireRefCt) || force){
				if (item == amGetList(am, id)){
					artManagerItemDeleteData(ai);
					
					TLISTITEM *next = item->next;
					listRemove(item);
					listDestroy(item);
					amGetBranch(am, artIdToBranchId(id))->items = next;
					return 1;
				}else{
					artManagerItemDeleteData(ai);
					listRemove(item);
					listDestroy(item);
					return 1;
				}
			//}else if (ai->acquireRefCt && !force){
			//	printf("am Delete item not released id:%i acquireRefCt:%i multiRefCt:%i\n", ai->id&0xFFF, ai->acquireRefCt, ai->multiRefCt);
			}
			
			if (ai->multiRefCt && !ai->acquireRefCt)
				ai->multiRefCt--;
			return 0;
		}
		item = listGetNext(item);
	}
	
	return 0;
}
#endif

int artManagerImageDelete (TARTMANAGER *am, const int id)
{
	//printf("am delete %X\n", id);
	
	return artManagerItemDelete(am, id, 0);
}

static inline TART_ITEM *artManagerItemNew (TARTMANAGER *am)
{
	TART_ITEM *ai = my_calloc(1, sizeof(TART_ITEM));
	if (ai){
		ai->id = artManagerGenerateId(am);
		ai->state = ARTC_STATE_INVALID;
		//ai->acquireRefCt = 0;
		//ai->multiRefCt = 0;
		ai->image.bpp = SKINFILEBPP;
		ai->image.type = IMG_JPG;
		//ai->image.hasAlpha = 0;
		//ai->scaled[0] = NULL;
	}
	return ai;
}

wchar_t *artManagerImageGetPath (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (ai->state&ARTC_STATE_HASPATH)
			return my_wcsdup(ai->path);
	}
	return NULL;
}

char *artManagerImageGetPath8 (TARTMANAGER *am, const int id)
{
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (ai->state&ARTC_STATE_HASPATH)
			return com_convertto8(ai->path);
	}
	return NULL;
}

int artManagerImageSetPath (TARTMANAGER *am, const int id, const wchar_t *path)
{
	//printf("artManagerImageSetPath %X\n", id);
	
	TART_ITEM *ai = artManagerIdToItem(am, id);
	if (ai){
		if (ai->state&ARTC_STATE_HASPATH)
			my_free(ai->path);
			
		if (am->pathPrefix){
			wchar_t buffer[MAX_PATH+1];
			//_snwprintf(buffer, MAX_PATH, L"%s/%s/%s", SKINDROOT, am->pathPrefix, path);
			__mingw_snwprintf(buffer, MAX_PATH, L"%ls/%ls", am->pathPrefix, path);
			ai->path = my_wcsdup(buffer);
		}else{
			ai->path = my_wcsdup(path);
		}
		
		//wprintf(L"artManagerImageSetPath @%s@\n", ai->path);
		
		ai->state |= ARTC_STATE_HASPATH;
		ai->state &= ~ARTC_STATE_HASMETRICS;
		ai->state &= ~ARTC_STATE_CANNOTREAD;

		if (ai->state&ARTC_STATE_HASSURFACE){
			ai->state &= ~ARTC_STATE_HASSURFACE;
			artManagerItemDeleteSurfaceData(ai);
			artManagerItemDeleteScaledData(ai);
		}

		return 1;
	}
	return 0;
}

static inline int artManagerFindHash (TARTMANAGER *am, const uint32_t hash)
{
	return hashTableFind(am, hash);
}

static inline TART_BRANCH *artManagerBranchNew ()
{
	TART_BRANCH *ab = my_calloc(1, sizeof(TART_BRANCH));
	if (ab){
		ab->bucketSize = ARTC_BUCKET_SIZE;
		ab->hLock = lockCreate("artManagerBranchNew");
		ab->items = NULL;
	}
	return ab;
}

static inline int artManagerBranchCount (TART_BRANCH *ab)
{
	return listCount(ab->items);
}

static inline void artManagerBranchAdd (TART_BRANCH *ab, void *ptr)
{
	listAdd(ab->items, ptr);
}

static inline int artManagerBranchGrow (TARTMANAGER *am, const int growBy)
{
	const int newSize = am->branchTotal + growBy;
		
	am->buckets = (TART_BRANCH**)my_realloc(am->buckets, newSize * sizeof(TART_BRANCH**));
	for (int i = am->branchTotal; i < newSize; i++)
		am->buckets[i] = artManagerBranchNew();
	return  am->branchTotal = newSize;
}

static inline int isImageAvailable (const wchar_t *path)
{
	if (extMatch(path, L".exe") || extMatch(path, L".dll")){
		int ret = lImageGetMetrics(path, IMG_ICO, NULL, NULL, NULL) > 0;
		//wprintf(L"isImageAvailable %i '%s'\n", ret, path);
		return ret;
	}
	return 1;
}

int artManagerImageAddEx (TARTMANAGER *am, const wchar_t *path, const int width, const int height)
{
	
	//wprintf(L"added '%s'\n", path);
	
	if (!isImageAvailable(path))
		return 0;

	const uint32_t hash = com_getHashW(path);
	
	// check if the path has previously been added, if so reuse that handle (return id and increase reference count)
	if (!amLock(am))
		return 0;
	
	int id = artManagerFindHash(am, hash);
	if (id){
		TART_ITEM *ai = artManagerIdToItem(am, id);
		ai->multiRefCt++;
		amUnlock(am);
		return id;
	}

	//amUnlock(am);

	// create new instance and add to a bucket
	//if (amLock(am)){
		TART_ITEM *ai = artManagerItemNew(am);
		if (ai){
			if (am->pathPrefix){
				wchar_t buffer[MAX_PATH+1];
				__mingw_snwprintf(buffer, MAX_PATH, L"%ls/%ls", am->pathPrefix, path);
				ai->path = my_wcsdup(buffer);
			}else{
				ai->path = my_wcsdup(path);
			}

			ai->image.type = IMG_JPG;
			ai->state |= ARTC_STATE_HASPATH;

			TART_BRANCH *ab = amGetBranch(am, am->branchInsertPos);
			if (abLock(ab)){
				if (artManagerBranchCount(ab) >= ab->bucketSize){
					if (++am->branchInsertPos >= am->branchTotal)
						artManagerBranchGrow(am, 2);		// growBy * bucketSize = added capacity. (2*32 = 64)
					abUnlock(ab);
					ab = amGetBranch(am, am->branchInsertPos);
					abLock(ab);
				}

				artManagerBranchAdd(ab, listNewItem(ai));
				ai->id |= (am->branchInsertPos << 16);
				hashTableAdd(am, hash, ai->id);
				id = ai->id;
	
				if (width && height)
					artManagerItemSetCustomMetrics(ai, width, height);
				
				//wprintf(L"added %i %i '%s' ### '%s'\n", ai->id>>16, ai->id&0xFFFF, ai->path, path);
				abUnlock(ab);
			}
		}
		amUnlock(am);
	//}
	return id;
}

int artManagerImageAdd (TARTMANAGER *am, const wchar_t *path)
{
	return artManagerImageAddEx(am, path, 0, 0);
}

int artManagerSetPathPrefix (TARTMANAGER *am, const wchar_t *prefix)
{
	if (am->pathPrefix)
		my_free(am->pathPrefix);
	am->pathPrefix = my_wcsdup(prefix);
	return (am->pathPrefix != NULL);
}

static inline void artManagerBranchDelete (TART_BRANCH *ab)
{
	TLISTITEM *item = ab->items;

	while(item){
		TART_ITEM *ai = listGetStorage(item);
		if (ai) artManagerItemDeleteData(ai);
		item = listGetNext(item);
	}
	listDestroyAll(ab->items);
		
	lockClose(ab->hLock);
	my_free(ab);
}

#if 1

int artManagerCount (TARTMANAGER *am)
{
	int ct = 0;
	if (amLock(am)){
		for (int i  = 0; i <= am->branchInsertPos; i++){
			TART_BRANCH *ab = am->buckets[i];
			if (abLock(ab)){
				if (ab->items)
					ct += listCount(ab->items);
				abUnlock(ab);
			}
		}
		amUnlock(am);
	}
	return ct;
}

int artManagerUnreleasedCount (TARTMANAGER *am)
{
	int ct = 0;

	if (amLock(am)){
		for (int i  = 0; i <= am->branchInsertPos; i++){
			TART_BRANCH *ab = am->buckets[i];
			if (abLock(ab)){
				TLISTITEM *item = ab->items;
				while(item){
					TART_ITEM *ai = listGetStorage(item);
					if (ai->acquireRefCt){
						//wprintf(L"artManagerUnreleased %i %i '%s'\n", ai->id&0xFFF, ai->acquireRefCt, ai->path);
						ct++;
					}
					item = listGetNext(item);
				}
				abUnlock(ab);
			}
		}
		amUnlock(am);
	}
	return ct;
}

int artManagerSurfaceCount (TARTMANAGER *am)
{
	int ct = 0;

	if (amLock(am)){
		for (int i  = 0; i <= am->branchInsertPos; i++){
			TART_BRANCH *ab = am->buckets[i];
			abLock(ab);
			
			TLISTITEM *item = ab->items;
			while(item){
				TART_ITEM *ai = listGetStorage(item);
				ct += (ai->surface != NULL);
				if (ai->scaled){
					for (int i = 0; i < ARTC_SCALED_MAX; i++){
						if (ai->scaled[i])
							ct += (ai->scaled[i]->surface != NULL);
					}
				}
				item = listGetNext(item);
			}
			abUnlock(ab);
		}
		amUnlock(am);
	}
	return ct;
}

size_t artManagerMemUsage (TARTMANAGER *am)
{
	amLock(am);
	size_t mem = sizeof(TART_HASHTABLE) * am->hashTableSize;

	for (int i  = 0; i <= am->branchInsertPos; i++){
		TART_BRANCH *ab = am->buckets[i];
		abLock(ab);
		
		mem += sizeof(TART_BRANCH) + sizeof(TMLOCK) + (listCount(ab->items) * sizeof(TLISTITEM));
		
		TLISTITEM *item = ab->items;
		while(item){
			mem += sizeof(TART_ITEM);
			
			TART_ITEM *ai = listGetStorage(item);
			if (ai->path)
				mem += ((1+wcslen(ai->path)) * sizeof(wchar_t));

			if (ai->surface)
				mem += (ai->surface->frameSize + sizeof(TFRAME) + sizeof(TPIXELPRIMITVES));
			
			if (ai->scaled){
				for (int i = 0; i < ARTC_SCALED_MAX; i++){
					if (ai->scaled[i] && ai->scaled[i]->surface)
						mem += (ai->scaled[i]->surface->frameSize + sizeof(TFRAME) + sizeof(TPIXELPRIMITVES) + sizeof(TART_SCALED));
				}
			}
			item = listGetNext(item);
		}
		abUnlock(ab);
	}
	
	amUnlock(am);
	return mem;
}

#if 0
void artManagerDumpStats (TARTMANAGER *am)
{
	printf("\n");
	printf("count %i\n", artManagerCount(am));
	printf("surfaceCount %i\n", artManagerSurfaceCount(am));
	printf("mem used %.0f\n", artManagerMemUsage(am)/1024.0);
	printf("acquiredCount %i\n", artManagerUnreleasedCount(am));
	printf("branchTotal %i\n", am->branchTotal);
	printf("hashTableSize %i\n", am->hashTableSize);
}

int artManagerDumpIds (TARTMANAGER *am)
{
	int ct = 0;
	
	if (amLock(am)){
		for (int i  = 0; i <= am->branchInsertPos; i++){
			TART_BRANCH *ab = amGetBranch(am, i);
			if (abLock(ab)){
				int j = 0;
				TLISTITEM *item = ab->items;
				while(item){
					TART_ITEM *ai = listGetStorage(item);
					if (ai && ai->id){
						printf("artManagerDumpIds %i %i: %X\n", i, j, ai->id);
						ct++;
					}
					j++;
					item = listGetNext(item);
				}
				abUnlock(ab);
			}
		}
		amUnlock(am);
	}
	return ct;
}
#endif

int *artManagerGetIds (TARTMANAGER *am, int *count)
{
	*count = 0;
	int *list = NULL;
	
	if (amLock(am)){
		const int total = artManagerCount(am);
		if (total){
			list = my_malloc((2+total)*sizeof(int));
			if (list){
				for (int i = 0; i <= am->branchInsertPos; i++){
					TART_BRANCH *ab = amGetBranch(am, i);
        	
					if (abLock(ab)){
						TLISTITEM *item = ab->items;
						while(item && *count < total){
							TART_ITEM *ai = listGetStorage(item);
							if (ai && ai->id){
								//printf("artManagerGetIds %i: %X, %i %i\n", i, ai->id, ai->bestfit.width, ai->resize.height);
								int area = ai->bestfit.width * ai->bestfit.height;
								if (area > 6400){	// don't give me ui/skin data
									list[*count] = ai->id;
									(*count)++;
								}
							}
							item = listGetNext(item);
						}
						abUnlock(ab);
					}
				}
				list[total] = 0;
			}
		}
		amUnlock(am);
	}
	return list;
}
#endif

// refactor: add an artbranch delete method
void artManagerDelete (TARTMANAGER *am)
{
	amLock(am);
	
	//printf("am Delete added remaining:%i\n", artManagerCount(am));
	//printf("am Delete unreleased:%i\n", artManagerUnreleasedCount(am));
	//printf("am Delete ", am->branchTotal, am->branchTotal);
	
	for (int i  = 0; i < am->branchTotal; i++){
		TART_BRANCH *ab = amGetBranch(am, i);
		if (ab){
			abLock(ab);
			artManagerBranchDelete(ab);
		}
	}
	
	my_free(am->buckets);
	my_free(am->hashTable);
	if (am->pathPrefix) my_free(am->pathPrefix);
	lockClose(am->hLock);
	my_free(am);
}

TARTMANAGER *artManagerNew (void *hSurfaceLib)
{
	TARTMANAGER *am = my_calloc(1, sizeof(TARTMANAGER));
	if (am){
		am->hSurfaceLib = hSurfaceLib;
		am->hLock = lockCreate("artManagerNew");
		am->idSrc = 100;
		am->branchInsertPos = 0;
		am->branchTotal = 2;		// branch total
		
		am->buckets = (TART_BRANCH**)my_malloc(am->branchTotal * sizeof(TART_BRANCH**));
		for (int i = 0; i < am->branchTotal; i++)
			am->buckets[i] = artManagerBranchNew();

		am->hashTableSize = 32;
		am->hashTable = my_calloc(am->hashTableSize, sizeof(TART_HASHTABLE));
		
	}
	return am;
}
