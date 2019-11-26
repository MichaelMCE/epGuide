
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
#include <process.h>


static volatile int activeThreadCount = 0;



static inline TPANEOBJ *paneItemIdToObject (TPANE *pane, const int itemId)
{
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && (obj->text.itemId == itemId || obj->image.itemId == itemId))
			return obj;
		item = listGetNext(item);
	}
	return NULL;
}

static inline TPANEOBJ *paneItemIdxToObject (TPANE *pane, const int idx)
{
	int ct = 0;
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj){
			if (ct++ == idx) return obj;
		}
		item = listGetNext(item);
	}
	return NULL;
}

static inline TPANEOBJ *paneIdToObject (TPANE *pane, const int id)
{
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && (obj->id == id))
			return obj;
		item = listGetNext(item);
	}
	return NULL;
}

static inline int paneGetInvalidated (TPANE *pane)
{
	return pane->layout.isInvalidated;
}

static inline void paneSetInvalidated (TPANE *pane)
{
	pane->layout.isInvalidated = 0xFF;
}

static inline void paneSetValidated (TPANE *pane)
{
	pane->layout.isInvalidated = 0;
}


void paneInvalidate (TPANE *pane)
{
	//if (ccLock(pane)){
		paneSetInvalidated(pane);
	//	ccUnlock(pane);
	//}
}


/*

set pagination off
set target-async on

*/
static inline uint32_t __stdcall panePreloadObjectsThread (void *data)
{
	img_readhead *cache = (img_readhead*)data;

	//const int tid = GetCurrentThreadId();
	//printf("readAhead %i %X\n", cache->total, tid);
	//double t0 = getTime(cache->vp);

	for (int i = 0; i < cache->total; i++){
		if (lockWait(cache->am->hLock, 10000)){
			int ret = artManagerImagePreload(cache->am, cache->imgIds[i]);

			// if image is already loaded; don't waste the lock but try another
			while (ret == 2 && ++i < cache->total){
				/*ret = */artManagerImagePreload(cache->am, cache->imgIds[i]);
				//printf("preload %i %X, %i\n", i, cache->imgIds[i], ret);
			}
			lockRelease(cache->am->hLock);
		}else{
			break;
		}
	}

	//double t1 = getTime(cache->vp);
	//printf("readAhead %i %.4f \n\n", cache->total, t1-t0);

	my_free(cache);
	//printf("readAhead Exit %X %i\n", tid, activeThreadCount);
	activeThreadCount--;
	_endthreadex(1);
	return 1;
}

static inline int isInList (img_readhead *cache, int count, const int id)
{
	while (count--){
		if (cache->imgIds[count] == id)
			return 1;
	}
	return 0;
}

static inline void panePreloadObjectsForward (TPANE *pane, TLISTITEM *item, const int preloadCount)
{
	if (activeThreadCount >= 2) return;

	img_readhead *cache = my_malloc(sizeof(img_readhead));
	if (!cache) return;

	cache->total = 0;
	cache->am = ccGetImageManager(pane->cc, CC_IMAGEMANAGER_ART);
	cache->ctx = pane->cc;

	//double t0 = getTime(cache->vp);

	for (int i = 0; i < preloadCount && item; i++){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) return;

		if (obj->image.imgcId && !isInList(cache, i, obj->image.imgcId)){
			TLABELIMGCA *img = labelItemGet(pane->base, obj->image.itemId);
			if (img && !img->drawableDrawn && cache->total < maxImgCacheItems)
				cache->imgIds[cache->total++] = obj->image.imgcId;
		}
		item = listGetNext(item);
	}

	//double t1 = getTime(cache->vp);
	//printf("panePreloadObjectsForward %i %.4f\n", cache->total, t1-t0);

	if (cache->total){
		uint32_t tid;
		activeThreadCount++;
		_beginthreadex(NULL, 0, panePreloadObjectsThread, cache, 0, &tid);
	}else{
		my_free(cache);
	}
}

static inline void panePreloadObjectsBack (TPANE *pane, TLISTITEM *item, const int preloadCount)
{
	if (activeThreadCount >= 2) return;

	img_readhead *cache = my_malloc(sizeof(img_readhead));
	if (!cache) return;


	cache->total = 0;
	cache->am = ccGetImageManager(pane->cc, CC_IMAGEMANAGER_ART);
	cache->ctx = pane->cc;

	//double t0 = getTime(cache->vp);

	for (int i = 0; i < preloadCount && item; i++){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) return;

		if (obj->image.imgcId && !isInList(cache, i, obj->image.imgcId)){
			TLABELIMGCA *img = labelItemGet(pane->base, obj->image.itemId);
			if (img && !img->drawableDrawn && cache->total < maxImgCacheItems)
				cache->imgIds[cache->total++] = obj->image.imgcId;
		}
		item = listGetPrev(item);
	}

	//double t1 = getTime(cache->vp);
	//printf("panePreloadObjectsBack %i %.4f\n", cache->total, t1-t0);

	if (cache->total){
		uint32_t tid;
		activeThreadCount++;
		_beginthreadex(NULL, 0, panePreloadObjectsThread, cache, 0, &tid);
	}else{
		my_free(cache);
	}
}

static inline void panePreloadObjects (TPANE *pane, TLISTITEM *backItem, TLISTITEM *forwardItem, const int preloadCount)
{
	if (pane->layout.direction == PANE_DIRECTION_UP || pane->layout.direction == PANE_DIRECTION_LEFT)
		panePreloadObjectsForward(pane, forwardItem, preloadCount);
	else if (pane->layout.direction == PANE_DIRECTION_DOWN || pane->layout.direction == PANE_DIRECTION_RIGHT)
		panePreloadObjectsBack(pane, backItem, preloadCount);
}

void panePreloadItems (TPANE *pane, const int count)
{
	//printf("panePreloadItems %i\n", count);

	if (ccLock(pane)){
		if (!pane->lastEnabledItem) pane->lastEnabledItem = pane->items;
		panePreloadObjects(pane, pane->firstEnabledItem, pane->lastEnabledItem, count);
		ccUnlock(pane);
	}
}

static inline int paneCalcDirection (TPANE *pane)
{
	const int deltaX = pane->previous.x - pane->offset.x;
	const int deltaY = pane->previous.y - pane->offset.y;

	if (deltaX > 0) pane->layout.direction = PANE_DIRECTION_LEFT;
	else if (deltaX < 0) pane->layout.direction = PANE_DIRECTION_RIGHT;
	else if (deltaY > 0) pane->layout.direction = PANE_DIRECTION_UP;
	else if (deltaY < 0) pane->layout.direction = PANE_DIRECTION_DOWN;
	else pane->layout.direction = PANE_DIRECTION_STOP;

	//printf("paneVert %i, %i %i %i\n", pane->pos.x, pane->offset.x, pane->previous.x, pane->layout.direction);
	pane->previous.x = pane->offset.x;
	pane->previous.y = pane->offset.y;

	return pane->layout.direction;
}

static inline void paneValidateImage (TPANE *pane, TPANEOBJ *obj, TMETRICS *metrics)
{
	labelArtcGetMetrics(pane->base, obj->image.itemId, &obj->image.metrics.width, &obj->image.metrics.height);

	//printf("paneValidateImage: %i %i %i\n", obj->image.itemId, obj->image.metrics.width, obj->image.metrics.height);

	switch (obj->image.dir){
	case PANE_IMAGE_CENTRE:
		obj->image.metrics.x = (metrics->width - obj->image.metrics.width)/2;
		obj->image.metrics.x += obj->image.offset.x;// + pane->metrics.x;
		obj->image.metrics.y = (metrics->height - obj->image.metrics.height)/2;
		obj->image.metrics.y += obj->image.offset.y;// + pane->metrics.y;
		break;
	case PANE_IMAGE_NORTH:
		obj->image.metrics.x = (metrics->width - obj->image.metrics.width)/2;
		obj->image.metrics.x += obj->image.offset.x;
		//obj->image.metrics.y = 0;
		obj->image.metrics.y = obj->image.offset.y;
		break;
	case PANE_IMAGE_SOUTH:
		obj->image.metrics.x = (metrics->width - obj->image.metrics.width)/2;
		obj->image.metrics.x += obj->image.offset.x;
		obj->image.metrics.y = (metrics->height - obj->image.metrics.height);
		obj->image.metrics.y += obj->image.offset.y;
		break;
	case PANE_IMAGE_WEST:
		//obj->image.metrics.x = 0;
		obj->image.metrics.x = obj->image.offset.x;
		obj->image.metrics.y = abs(metrics->height - obj->image.metrics.height)/2;
		obj->image.metrics.y += obj->image.offset.y;
		break;
	case PANE_IMAGE_EAST:
		obj->image.metrics.x = metrics->width - obj->image.metrics.width;
		obj->image.metrics.x += obj->image.offset.x;
		obj->image.metrics.y = abs(metrics->height - obj->image.metrics.height)/2;
		obj->image.metrics.y += obj->image.offset.y;
		//printf("%i %i\n", metrics->height, obj->image.metrics.height);
		break;
	case PANE_IMAGE_NW:
		//obj->image.metrics.x = 0;
		obj->image.metrics.x = obj->image.offset.x;
		//obj->image.metrics.y = 0;
		obj->image.metrics.y = obj->image.offset.y;
		break;
	case PANE_IMAGE_NE:
		obj->image.metrics.x = metrics->width - obj->image.metrics.width;
		obj->image.metrics.x += obj->image.offset.x;
		//obj->image.metrics.y = 0;
		obj->image.metrics.y = obj->image.offset.y;
		break;
	case PANE_IMAGE_SW:
		//obj->image.metrics.x = 0;
		obj->image.metrics.x = obj->image.offset.x;
		obj->image.metrics.y = (metrics->height - obj->image.metrics.height);
		obj->image.metrics.y += obj->image.offset.y;
		break;
	case PANE_IMAGE_SE:
		obj->image.metrics.x = metrics->width - obj->image.metrics.width;
		obj->image.metrics.x += obj->image.offset.x;
		obj->image.metrics.y = metrics->height - obj->image.metrics.height;
		obj->image.metrics.y += obj->image.offset.y;
		break;
	case PANE_IMAGE_POS:
	default:
		break;
	}

	if (obj->image.metrics.x < 0) obj->image.metrics.x = 0;
	if (obj->image.metrics.y < 0) obj->image.metrics.y = 0;
	labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
}

static inline int paneValidateVertMiddle (TPANE *pane)
{

	//printf("paneValidateVertMiddle %i %i\n", pane->offset.x, pane->offset.y);

	if (pane->offset.y > 0) pane->offset.y = 0;
	const int yOffset = -pane->offset.y;

	int ct = 0;
	const int x1 = (ccGetWidth(pane)/2); /*+ ccGetPositionX(pane);*/
	int y1 = pane->offset.y;
	int lineHeight = 0;
	int tItemWidth = 0;
	int tItemHeight = 0;
	const int vertLineHeight = pane->layout.metrics.vertLineHeight;
	const int baseRenderFlags = labelRenderFlagsGet(pane->base);
	TLISTITEM *firstEnabledItem = NULL;
	TLISTITEM *lastEnabledItem = NULL;
	int firstEnabledItemId = 0;
	const int imageTextSpace = 4;
	labelItemsDisable(pane->base);
	int aveLineHeight = 0;
	int aveLineHeightTotal = 0;


	//printf("pane paneValidateVertMiddle %i, %i %i, objGuess:%i\n", pane->layout.metrics.vertLineHeight, y1, yOffset, yOffset/pane->layout.metrics.vertLineHeight);


	TLISTITEM *item = pane->items;

#if 1
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj->type == PANE_OBJ_IMAGE){
			labelItemEnable(pane->base, obj->image.itemId);
			paneValidateImage(pane, obj, &pane->metrics);
			ct++;
			break;
		}
		item = listGetNext(item);
	}
	item = pane->items;

	// if line height is constant then we can jump directly to first visible item
	if (vertLineHeight){
		int objIdx = yOffset/vertLineHeight;
		if (objIdx > 5){
			y1 += (objIdx * vertLineHeight);

			while(objIdx >= 0 && item){
				objIdx -= 1;
				item = listGetNext(item);
			}
			y1 -= (objIdx * vertLineHeight) + vertLineHeight;
		}
	}
#endif

	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) break;//goto next;

		if (obj->type == PANE_OBJ_STRING){
			if (y1 >= pane->metrics.height){
				if (pane->flags.readAhead.enabled)
					panePreloadObjects(pane, firstEnabledItem, lastEnabledItem, pane->flags.readAhead.number);
				break;
			}
			int col = x1;

			if (!obj->text.metrics.height){
				obj->text.metrics.width = pane->metrics.width - obj->text.metrics.x;
				labelStringGetMetrics(pane->base, obj->text.itemId, NULL, NULL, &obj->text.metrics.width, &obj->text.metrics.height);
				//if (obj->text.metrics.width > maxW) obj->text.metrics.width = maxW;
				//printf("obj->text.metrics.height %i, %i: %i\n",ct, obj->text.itemId, obj->text.metrics.height);
			}

			obj->text.metrics.x = col - (obj->text.metrics.width/2);
			obj->text.metrics.y = y1;
			//labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);

			if (obj->text.hasIcon){
				TLABELIMGCA *img = labelItemGet(pane->base, obj->image.itemId);
				if (img){
					img->hasConstraints = 1;
					img->constraint.x1 = pane->metrics.x;
					img->constraint.y1 = pane->metrics.y;
					img->constraint.x2 = pane->metrics.x + pane->metrics.width-1;
					img->constraint.y2 = pane->metrics.y + pane->metrics.height-1;
				}

				if (!obj->image.metrics.height)
					labelArtcGetMetrics(pane->base, obj->image.itemId, &obj->image.metrics.width, &obj->image.metrics.height);

				if (!obj->text.overlapIcon){
					obj->text.metrics.x += obj->image.metrics.width/2;
					obj->image.metrics.x = obj->text.metrics.x - imageTextSpace - obj->image.metrics.width;
					//printf("obj->image.metrics.x a %i\n", obj->image.metrics.x);
				}else{
					//obj->image.metrics.x = obj->text.metrics.x;
					obj->text.metrics.x += obj->image.metrics.width/2;
					obj->image.metrics.x = obj->text.metrics.x - obj->image.metrics.width;
					obj->text.metrics.x = obj->image.metrics.x;
					//printf("obj->image.metrics.x b %i\n", obj->image.metrics.x);
				}
				obj->image.metrics.y = y1;
				//labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);

				//if (!obj->text.overlapIcon)
				//	col = x1 + obj->image.metrics.width;
			}

			// enable visible items only
			if ((obj->text.metrics.y > -obj->text.metrics.height) || (obj->text.hasIcon && obj->text.metrics.y > -obj->image.metrics.height)){
				if (baseRenderFlags&LABEL_RENDER_TEXT){
					labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);
					labelItemEnable(pane->base, obj->text.itemId);
				}
				if (obj->text.hasIcon){
					labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
					labelItemEnable(pane->base, obj->image.itemId);
				}
				if (!firstEnabledItem){
					pane->firstEnabledItem = firstEnabledItem = item;
					firstEnabledItemId = obj->text.itemId;
					if (!firstEnabledItemId) firstEnabledItemId = obj->image.itemId;
				}
				pane->lastEnabledItem = lastEnabledItem = item;
			}

			if (vertLineHeight){
				y1 += vertLineHeight;
				aveLineHeight += vertLineHeight;
			}else{
				int height = obj->text.metrics.height;
				//printf("height %i %i, %i\n", ct, height, pane->layout.metrics.vertLineHeight);

				if (height < 4 && obj->text.hasIcon) height = obj->image.metrics.height;
				lineHeight = height + 4;

				//printf("ct %i %i %i\n", ct, lineHeight, pane->layout.metrics.vertLineHeight);
				if (lineHeight < obj->image.metrics.height)
					lineHeight = obj->image.metrics.height+2;
				y1 += lineHeight;
				aveLineHeight += lineHeight;
			}

			if (!obj->text.overlapIcon){
				//printf("vert: metrics %i %i %i\n", tItemWidth, obj->text.metrics.width, obj->image.metrics.width);

				int itemWidth = 0;
				if (obj->text.hasIcon) itemWidth = obj->image.metrics.width;
				if (obj->text.metrics.width + itemWidth > tItemWidth)
					tItemWidth = obj->text.metrics.width + itemWidth;

			}else{
				tItemWidth = MAX(tItemWidth, MAX(obj->text.metrics.width,obj->image.metrics.width));
			}

			aveLineHeightTotal++;
			ct++;
#if 0
		}else if (obj->type == PANE_OBJ_IMAGE){
			labelItemEnable(pane->base, obj->image.itemId);
			paneValidateImage(pane, obj, &pane->metrics);
			//printf("vertMiddle pane->metrics %i: %i %i %i %i\n", obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y, obj->image.metrics.width, obj->image.metrics.height);

			ct++;
#endif
		}
//next:
		item = listGetNext(item);
	}

	//printf("endofLoop objs:%i, y1:%i\n", ct, y1);

	tItemHeight = y1;
	if (aveLineHeightTotal)
		aveLineHeight /= (double)aveLineHeightTotal;

	paneCalcDirection(pane);
	pane->layout.metrics.tItemWidth = tItemWidth;
	pane->layout.metrics.tItemHeight = tItemHeight;
	pane->layout.metrics.aveLineHeight = aveLineHeight;
	pane->pos.y = (y1 + yOffset) - aveLineHeight;//lineHeight;
	paneSetValidated(pane);
	return ct;
}

static inline int paneValidateVert (TPANE *pane)
{

	//printf("paneValidateVert %i %i\n", pane->offset.x, pane->offset.y);

	if (pane->offset.y > 0) pane->offset.y = 0;
	const int yOffset = -pane->offset.y;

	int ct = 0;
	int x1 = 0;
	int y1 = pane->offset.y;
	int lineHeight = 0;
	int tItemWidth = 0;
	int tItemHeight = 0;
	const int vertLineHeight = pane->layout.metrics.vertLineHeight;
	const int baseRenderFlags = labelRenderFlagsGet(pane->base);
	TLISTITEM *firstEnabledItem = NULL;
	TLISTITEM *lastEnabledItem = NULL;
	int firstEnabledItemId = 0;

	labelItemsDisable(pane->base);


	//printf("pane pane->layout.metrics.vertLineHeight %i\n", pane->layout.metrics.vertLineHeight);

	TLISTITEM *item = pane->items;

	// ensure PANE_OBJ_IMAGE is processed and not skipped over
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj->type == PANE_OBJ_IMAGE){
			labelItemEnable(pane->base, obj->image.itemId);
			paneValidateImage(pane, obj, &pane->metrics);
			ct++;
			break;
		}
		item = listGetNext(item);
	}
	item = pane->items;


	// if line height is constant then we can jump directly to first visible item(s)
	if (vertLineHeight){
		int objIdx = yOffset / vertLineHeight;
		if (objIdx > 5){
			y1 += objIdx * vertLineHeight;

			while(objIdx > 2 && item){
				objIdx -= 1;
				item = listGetNext(item);
			}
			y1 -= (objIdx * vertLineHeight) + vertLineHeight;
		}
	}


	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) break;//goto next;

		if (obj->type == PANE_OBJ_STRING){
			if (y1 >= pane->metrics.height){
				if (pane->flags.readAhead.enabled)
					panePreloadObjects(pane, firstEnabledItem, lastEnabledItem, pane->flags.readAhead.number);
				break;
			}
			int col = x1;

			if (obj->text.hasIcon){
				TLABELIMGCA *img = labelItemGet(pane->base, obj->image.itemId);
				if (img){
					img->hasConstraints = 1;
					img->constraint.x1 = pane->metrics.x;
					img->constraint.y1 = pane->metrics.y;
					img->constraint.x2 = pane->metrics.x + pane->metrics.width-1;
					img->constraint.y2 = pane->metrics.y + pane->metrics.height-1;
				}

				if (!obj->image.metrics.height)
					labelArtcGetMetrics(pane->base, obj->image.itemId, &obj->image.metrics.width, &obj->image.metrics.height);

				obj->image.metrics.x = col;
				obj->image.metrics.y = y1;
				//labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);

				if (!obj->text.overlapIcon)
					col = x1 + obj->image.metrics.width;

			}

			obj->text.metrics.x = col;
			obj->text.metrics.y = y1;
			//labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);

			if (!obj->text.metrics.height){
				obj->text.metrics.width = pane->metrics.width - obj->text.metrics.x;
				labelStringGetMetrics(pane->base, obj->text.itemId, NULL, NULL, &obj->text.metrics.width, &obj->text.metrics.height);
				//printf("obj->text.metrics.height %i, %i: %i\n",ct, obj->text.itemId, obj->text.metrics.height);
			}

			// don't enable invisible items
			if ((obj->text.metrics.y > -obj->text.metrics.height) || (obj->text.hasIcon && obj->text.metrics.y > -obj->image.metrics.height)){
				if (baseRenderFlags&LABEL_RENDER_TEXT){
					labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);
					labelItemEnable(pane->base, obj->text.itemId);
				}
				if (obj->text.hasIcon){
					labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
					labelItemEnable(pane->base, obj->image.itemId);
				}
				if (!firstEnabledItem){
					pane->firstEnabledItem = firstEnabledItem = item;
					firstEnabledItemId = obj->text.itemId;
					if (!firstEnabledItemId) firstEnabledItemId = obj->image.itemId;
				}
				pane->lastEnabledItem = lastEnabledItem = item;
			}

			int height = obj->text.metrics.height;
			//printf("height %i %i, %i\n", ct, height, pane->layout.metrics.vertLineHeight);

			if (height < 4 && obj->text.hasIcon) height = obj->image.metrics.height;
			lineHeight = height + 4;

			//printf("ct %i %i %i\n", ct, lineHeight, pane->layout.metrics.vertLineHeight);
			if (lineHeight < obj->image.metrics.height)
				lineHeight = obj->image.metrics.height+2;

			if (!vertLineHeight)
				y1 += lineHeight;
			else
				y1 += vertLineHeight;

			//if (y1 >= pane->metrics.height){
				//break;
			//}
			if (!obj->text.overlapIcon){
				//printf("vert: metrics %i %i %i\n", tItemWidth, obj->text.metrics.width, obj->image.metrics.width);

				int itemWidth = 0;
				if (obj->text.hasIcon) itemWidth = obj->image.metrics.width;
				if (obj->text.metrics.width + itemWidth > tItemWidth)
					tItemWidth = obj->text.metrics.width + itemWidth;

			}else{
				tItemWidth = MAX(tItemWidth, MAX(obj->text.metrics.width,obj->image.metrics.width));
			}
			ct++;
#if 0
		}else if (obj->type == PANE_OBJ_IMAGE){
			labelItemEnable(pane->base, obj->image.itemId);
			paneValidateImage(pane, obj, &pane->metrics);
			ct++;
#endif
		}
//next:
		item = listGetNext(item);
	}

	tItemHeight = y1;
	//printf("vert: metrics %i %i\n", tItemWidth, tItemHeight);

	paneCalcDirection(pane);
	pane->layout.metrics.tItemWidth = tItemWidth;
	pane->layout.metrics.tItemHeight = tItemHeight;
	pane->pos.y = (y1 + yOffset) - lineHeight;
	paneSetValidated(pane);
	return ct;
}

static inline void paneSendMessageItemStateChange (TPANE *pane, const int objType, const int itemId, TMETRICS *metrics, const int newState)
{
	if (newState){
		if (!labelItemGetEnabledStatus(pane->base, itemId))
			ccSendMessage(pane, PANE_MSG_ITEM_ENABLED, itemId, objType, metrics);
		labelItemEnable(pane->base, itemId);
	}else{
		if (labelItemGetEnabledStatus(pane->base, itemId))
			ccSendMessage(pane, PANE_MSG_ITEM_DISABLED, itemId, objType, metrics);
		//labelItemDisable(pane->base, itemId);	// should already have been disabled through 'labelItemsDisable(pane->base)'
	}
}

static inline int paneValidateTable (TPANE *pane)
{
	if (pane->offset.x > pane->base->metrics.width-1)
		pane->offset.x = pane->base->metrics.width-1;
		
	//const int xOffset = -pane->offset.x;
	const point_t offset = pane->offset;
	const TMETRICS metrics = pane->metrics;
	const float pixelsPerUnit = pane->layout.table.pixelsPerUnit;
	const int horiMin = pane->layout.table.horiMin;
	const int cellSpaceHori = pane->layout.table.cellSpaceHori;
	const int vertLineHeight = pane->layout.metrics.vertLineHeight;
	const int cellSpaceVert = pane->layout.table.cellSpaceVert;
	const int vcsize = vertLineHeight - cellSpaceVert;
	int ct = 0;
	
	labelItemsDisable(pane->base);
	
	ccLock(pane->base);
	
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) break;

		if (obj->type == PANE_OBJ_CELL){
			const int32_t row = obj->cell.row;
			const int32_t width = obj->cell.width;
			const int32_t position = obj->cell.position;

			float hpos = (position - horiMin) / pixelsPerUnit;
			int strX = offset.x + hpos;
			int strY = offset.y + (row * vertLineHeight);

			obj->cell.metrics.x = strX;
			obj->cell.metrics.y = strY;
			obj->cell.metrics.width = (width / pixelsPerUnit) - cellSpaceHori;
			obj->cell.metrics.height = vcsize;
			

			if (-obj->cell.metrics.x > obj->cell.metrics.width){
				
				//printf("0: %i: %i %i\n", obj->id, -obj->cell.metrics.x, obj->cell.metrics.width);
				
				obj->cell.isEnabled = 0;
				//paneSendMessageItemStateChange(pane, PANE_OBJ_CELL, obj->cell.item[i].id, &obj->cell.metrics, 0);
			}else if (obj->cell.metrics.x >= metrics.x + metrics.width){
				obj->cell.isEnabled = 0;
				//paneSendMessageItemStateChange(pane, PANE_OBJ_CELL, obj->cell.item[i].id, &obj->cell.metrics, 0);
			//}else if (obj->cell.metrics.y + (obj->cell.metrics.height*1.75) < pane->metrics.y){
			}else if (-obj->cell.metrics.y > obj->cell.metrics.height){
				obj->cell.isEnabled = 0;
				//printf("0: %i: %i %i\n", obj->id, -obj->cell.metrics.y, obj->cell.metrics.height);
				//paneSendMessageItemStateChange(pane, PANE_OBJ_CELL, obj->cell.item[i].id, &obj->cell.metrics, 0);
			}else if (obj->cell.metrics.y >= metrics.y + metrics.height){
				obj->cell.isEnabled = 0;
				//paneSendMessageItemStateChange(pane, PANE_OBJ_CELL, obj->cell.item[i].id, &obj->cell.metrics, 0);
			}else{
				obj->cell.isEnabled = 1;
				
				//printf("1: %i: %i %i\n", obj->id, obj->cell.metrics.x, obj->cell.metrics.width);
				
				for (int i = 0; i < PANE_CELL_MAXITEMS; i++){
					if (obj->cell.item[i].id){
						//printf("paneValidateTable: %i %i, %i %i\n", i, obj->cell.item[i].id, obj->cell.item[i].pos.x+strX, obj->cell.item[i].pos.y+strY);
						//labelItemPositionSet(pane->base, obj->cell.item[i].id, obj->cell.item[i].pos.x+strX, obj->cell.item[i].pos.y+strY);
						//labelItemEnable(pane->base, obj->cell.item[i].id);

						//const int baseRenderFlags = labelRenderFlagsGet(pane->base);
						//if (baseRenderFlags&LABEL_RENDER_TEXT){
							labelItemPositionSet_us(pane->base, obj->cell.item[i].id, obj->cell.item[i].pos.x+strX, obj->cell.item[i].pos.y+strY);
							//paneSendMessageItemStateChange(pane, PANE_OBJ_CELL, obj->cell.item[i].id, &obj->cell.metrics, 1);
							labelItemEnable_us(pane->base, obj->cell.item[i].id);
						//}
					}
				}
				ct++;
			}

			//drawRectangle(frame, pane->base->metrics.x+obj->cell.metrics.x, pane->base->metrics.y+obj->cell.metrics.y,
			//					  pane->base->metrics.x+obj->cell.metrics.x+obj->cell.metrics.width-1, pane->base->metrics.y+obj->cell.metrics.y+obj->cell.metrics.height-1,
			//					  0x001F);
			
		}

		item = listGetNext(item);
	}

	ccUnlock(pane->base);

	//printf("paneValidateTable: cells enabled: %i of %i\n", ct,  pane->flags.total.cell);
	paneCalcDirection(pane);
	pane->pos.x = offset.x + -offset.x;
	paneSetValidated(pane);

	return ct;
}

static inline int paneValidateHoriMiddle (TPANE *pane)
{
	//printf("paneValidateHori Middle %i %i\n", pane->offset.x, pane->offset.y);

	if (pane->offset.x > pane->base->metrics.width-1)
		pane->offset.x = pane->base->metrics.width-1;
	const int xOffset = -pane->offset.x;

	int x1 = pane->offset.x;
	int width = 0;
	const int paneHeight = ccGetHeight(pane);
	const int colGap = pane->layout.horiColumnSpace;			// space between columns
	int ct = 0;
	TLABEL *base = pane->base;
	const int panePitch = pane->metrics.x + (pane->metrics.width * 2);
	const int baseRenderFlags = labelRenderFlagsGet(base);
	TLISTITEM *firstEnabledItem = NULL;
	TLISTITEM *lastEnabledItem = NULL;
	int firstEnabledStrId = 0;
	int firstEnabledImgId = 0;
	int firstEnabledStrIdx = 0;
	int firstEnabledImgIdx = 0;

	labelItemsDisable(pane->base);
	const int basePosX = pane->base->metrics.x;


	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) break;//goto next;

		if (obj->type == PANE_OBJ_STRING){
			if (x1 > panePitch){
				if (pane->flags.readAhead.enabled)
					panePreloadObjects(pane, firstEnabledItem, lastEnabledItem, pane->flags.readAhead.number);
				break;
			}

			x1 += colGap;
			pane->pos.x = x1;

			if (!obj->text.metrics.height)
				labelStringGetMetrics(base, obj->text.itemId, NULL, NULL, &obj->text.metrics.width, &obj->text.metrics.height);

			int strY = (paneHeight - obj->text.metrics.height)/2;
			int imgY = strY;
			int strX = x1;
			int imgX = x1;

			if (obj->text.hasIcon){
				if (!obj->image.metrics.height)
					labelArtcGetMetrics(base, obj->image.itemId, &obj->image.metrics.width, &obj->image.metrics.height);

				if (obj->image.metrics.width > obj->text.metrics.width){
					imgX = x1;
					strX = imgX + ((obj->image.metrics.width - obj->text.metrics.width)/2);
					width += colGap + obj->image.metrics.width;
				}else{
					imgX += (obj->text.metrics.width - obj->image.metrics.width)/2;
					width += colGap + obj->text.metrics.width;
				}

				strY += (obj->image.metrics.height / 2);
				imgY -= (obj->image.metrics.height / 2);

				labelItemPositionSet(base, obj->image.itemId, imgX, imgY);
				obj->image.metrics.x = imgX;
				obj->image.metrics.y = imgY;

				if (basePosX + obj->image.metrics.x + obj->image.metrics.width < pane->metrics.x){
					paneSendMessageItemStateChange(pane, PANE_OBJ_IMAGE, obj->image.itemId, &obj->image.metrics, 0);
					//printf("a %i %i, %i %i\n", obj->image.itemId, imgX, obj->image.metrics.width, pane->metrics.x);

				}else if (basePosX + obj->image.metrics.x >= pane->metrics.x + pane->metrics.width){
					paneSendMessageItemStateChange(pane, PANE_OBJ_IMAGE, obj->image.itemId, &obj->image.metrics, 0);
					//printf("b %i %i, %i %i\n", obj->image.itemId, imgX, obj->image.metrics.width, pane->metrics.x);

				}else{
					paneSendMessageItemStateChange(pane, PANE_OBJ_IMAGE, obj->image.itemId, &obj->image.metrics, 1);
					//printf("c %i %i, %i %i\n", obj->image.itemId, imgX, obj->image.metrics.width, pane->metrics.x);

					if (!firstEnabledItem)
						pane->firstEnabledItem = firstEnabledItem = item;
					pane->lastEnabledItem = lastEnabledItem = item;
					if (!firstEnabledImgId){
						firstEnabledImgId = obj->image.itemId;
						firstEnabledImgIdx = ct;
					}
				}

				x1 = MAX((strX+obj->text.metrics.width), (imgX+obj->image.metrics.width));
			}else{
				x1 = strX + obj->text.metrics.width;
				width += colGap + obj->text.metrics.width;
			}

			labelItemPositionSet(base, obj->text.itemId, strX, strY);
			obj->text.metrics.x = strX;
			obj->text.metrics.y = strY;

			if (obj->text.metrics.x + obj->text.metrics.width < pane->metrics.x){
				paneSendMessageItemStateChange(pane, PANE_OBJ_STRING, obj->text.itemId, &obj->text.metrics, 0);
			}else if (obj->text.metrics.x >= pane->metrics.x + pane->metrics.width){
				paneSendMessageItemStateChange(pane, PANE_OBJ_STRING, obj->text.itemId, &obj->text.metrics, 0);
			}else{
				if (baseRenderFlags&LABEL_RENDER_TEXT)
					paneSendMessageItemStateChange(pane, PANE_OBJ_STRING, obj->text.itemId, &obj->text.metrics, 1);
				if (!firstEnabledItem)
					pane->firstEnabledItem = firstEnabledItem = item;
				pane->lastEnabledItem = lastEnabledItem = item;
				if (!firstEnabledStrId){
					firstEnabledStrId = obj->text.itemId;
					firstEnabledStrIdx = ct;
				}
			}

			ct++;
		}else if (obj->type == PANE_OBJ_IMAGE){
			paneValidateImage(pane, obj, &pane->metrics);
			paneSendMessageItemStateChange(pane, PANE_OBJ_IMAGE, obj->image.itemId, &obj->image.metrics, 1);
			ct++;
		}
//next:
		item = listGetNext(item);
	}

	paneCalcDirection(pane);
	pane->layout.metrics.tItemWidth = width + colGap;
	pane->pos.x += xOffset;
	pane->firstEnabledImgId = firstEnabledImgId;
	pane->firstEnabledStrId = firstEnabledStrId;
	pane->firstEnabledImgIdx = firstEnabledImgIdx;
	pane->firstEnabledStrIdx = firstEnabledStrIdx;
	paneSetValidated(pane);

	//printf("pane->pos.x %i %i\n", pane->pos.x, pane->layout.metrics.tItemWidth);

	return ct;
}

/*
"C:\Program Files (x86)\SeacrhMyFiles\SearchMyFiles.exe" /BaseFolder "K:\search headers\"  /FindFiles 1 /FileContains 1 /FileContainsText ""
*/
static inline int paneValidateHori (TPANE *pane)
{
	//printf("paneValidateHori %i, %i %i\n", FilesFilesFiles->id, pane->offset.x, pane->pos.y);

	if (pane->offset.x > 0) pane->offset.x = 0;
	const int xOffset = -pane->offset.x;

	int ct = 0;
	int x1 = pane->offset.x;
	int y1 = 0;
	int maxX = pane->offset.x;
	int col = x1;
	int tItemWidth = 0;
	const int horiItemSpace = pane->layout.horiColumnSpace;
	const int panePitch = pane->metrics.x + (pane->metrics.width * 2);
	const int maxW = pane->metrics.width + 256;
	const int baseRenderFlags = labelRenderFlagsGet(pane->base);
	const int globalOffsetX = pane->flags.text.globalOffset.x;
	const int globalOffsetY = pane->flags.text.globalOffset.y;
	TLISTITEM *firstEnabledItem = NULL;
	TLISTITEM *lastEnabledItem = NULL;
	int firstEnabledItemIdx = 0;

	// disable everything then only enable whats visible
	labelItemsDisable(pane->base);

	TLISTITEM *item = pane->items;

	/*
	int n = 0;
	while(item && n-- > 0)
		item = listGetNext(item);
	*/

	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (!obj) break;//goto next;

		if (obj->type == PANE_OBJ_STRING){
			if (x1 > panePitch){
				if (pane->flags.readAhead.enabled)
					panePreloadObjects(pane, firstEnabledItem, lastEnabledItem, pane->flags.readAhead.number);
				break;
			}
			col = x1;

			if (!obj->text.metrics.height){
				labelStringGetMetrics(pane->base, obj->text.itemId, NULL, NULL, &obj->text.metrics.width, &obj->text.metrics.height);
				if (obj->text.metrics.width > maxW) obj->text.metrics.width = maxW;
				if (obj->text.metrics.height < 16 && obj->text.metrics.width > 1 && obj->text.hasIcon){	// fix zero height strings, eg; a single space
					//printf("paneValidateHori %i: %i %i\n", ct, obj->text.metrics.width, obj->text.metrics.height);
					obj->text.metrics.height = 32;
				}
			}

			if (y1 + obj->text.metrics.height > pane->metrics.height){
				x1 = maxX + horiItemSpace;
				col = x1;
				y1 = 0;
			}

			pane->pos.x = x1;

			if (obj->text.hasIcon){
				TLABELIMGCA *img = labelItemGet(pane->base, obj->image.itemId);
				if (img){
					img->hasConstraints = 1;
					img->constraint.x1 = pane->metrics.x;
					img->constraint.y1 = pane->metrics.y;
					img->constraint.x2 = pane->metrics.x + pane->metrics.width-1;
					img->constraint.y2 = pane->metrics.y + pane->metrics.height-1;
				}

				if (!obj->image.metrics.height)
					labelArtcGetMetrics(pane->base, obj->image.itemId, &obj->image.metrics.width, &obj->image.metrics.height);

				//labelItemPositionSet(pane->base, obj->image.itemId, col, y1);
				obj->image.metrics.x = col;
				obj->image.metrics.y = y1;
				if (!obj->text.overlapIcon)
					col = x1 + obj->image.metrics.width;

				if (pane->metrics.x+obj->image.metrics.x + obj->image.metrics.width < pane->metrics.x){
					//labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
					//printf("hori a: %i %i\n", obj->image.itemId, obj->image.metrics.x);
				}else if (pane->metrics.x+obj->image.metrics.x >= pane->metrics.x + pane->metrics.width){
					//labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
					//printf("hori b: %i %i\n", obj->image.itemId, obj->image.metrics.x);
				}else{
					labelItemPositionSet(pane->base, obj->image.itemId, obj->image.metrics.x, obj->image.metrics.y);
					labelItemEnable(pane->base, obj->image.itemId);
					if (!firstEnabledItem){
						pane->firstEnabledItem = firstEnabledItem = item;
						pane->firstEnabledImgId = obj->image.itemId;
						firstEnabledItemIdx = ct;
					}
					pane->lastEnabledItem = lastEnabledItem = item;
				}
			}

			//labelItemPositionSet(pane->base, obj->text.itemId, col, y1);

			if (obj->text.hasIcon)
				obj->text.metrics.y = y1 + globalOffsetY + (abs(obj->image.metrics.height - obj->text.metrics.height)/2)-1;
			else
				obj->text.metrics.y = y1 + globalOffsetY;
			obj->text.metrics.x = col + globalOffsetX;
			//printf("%i: %i %i %i\n", ct, obj->text.itemId, col, y1);

			if (pane->metrics.x+obj->text.metrics.x + obj->text.metrics.width < pane->metrics.x){
				//labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);
				//labelItemDisable(pane->base, obj->text.itemId);

			}else if (pane->metrics.x+obj->text.metrics.x >= pane->metrics.x + pane->metrics.width){
				//labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);
				//labelItemDisable(pane->base, obj->text.itemId);

			}else if (baseRenderFlags&LABEL_RENDER_TEXT){
				labelItemPositionSet(pane->base, obj->text.itemId, obj->text.metrics.x, obj->text.metrics.y);
				labelStringSetMaxWidth(pane->base, obj->text.itemId, maxW);
				labelItemEnable(pane->base, obj->text.itemId);
				if (!firstEnabledItem){
					pane->firstEnabledItem = firstEnabledItem = item;
					pane->firstEnabledStrId = obj->text.itemId;
					firstEnabledItemIdx = ct;
				}
				pane->lastEnabledItem = lastEnabledItem = item;
			}

			if (maxX < col+obj->text.metrics.width)
				maxX = col + obj->text.metrics.width;
			if (obj->text.hasIcon && maxX < col+obj->image.metrics.width)
				maxX = col + obj->image.metrics.width+2;

			int height;
			if (obj->text.hasIcon)
				height = MAX(obj->text.metrics.height, obj->image.metrics.height) + 2;
			else
				height = obj->text.metrics.height + 4;

			y1 += height;
			//printf("y1 %i %i, %i %i\n", ct, y1, pane->metrics.y, pane->metrics.height);
			if (y1+height-4 >= pane->metrics.height){
				x1 = maxX + horiItemSpace;
				y1 = 0;
			}

			int itemWidth = 0;
			if (obj->text.hasIcon) itemWidth = obj->image.metrics.width;
			if (obj->text.metrics.width + itemWidth > tItemWidth)
				tItemWidth = obj->text.metrics.width + itemWidth;
			ct++;
		}else if (obj->type == PANE_OBJ_IMAGE){
			paneValidateImage(pane, obj, &pane->metrics);
			labelItemEnable(pane->base, obj->image.itemId); // should really be enabled
			ct++;
		}
//next:
		item = listGetNext(item);
#if 0
		if (!item){
			//printf("validateHori %i %i %i\n", x1, col, pane->pos.x);
			break;
		}
#endif
	}

	//printf("hor: firstEnabledItemIdx %i\n", firstEnabledItemIdx);

	paneCalcDirection(pane);
	pane->firstEnabledImgIdx = firstEnabledItemIdx;
	pane->layout.metrics.tItemWidth = tItemWidth;
	pane->pos.x += xOffset;
	paneSetValidated(pane);

	return ct;
}

static inline void paneObjInsertLast (TPANE *pane, TLISTITEM *ptr)
{
	void *pre = NULL;
	if (pane->itemsLast)
		pre = pane->itemsLast->prev;

	if (pane->itemsLast){
		listInsert(pane->itemsLast, NULL, ptr);
		//ptr->prev = pane->itemsLast;
	}else{
		pane->items = ptr;
	}

	if (pane->itemsLast)
		pane->itemsLast->prev = pre;

	ptr->prev = pane->itemsLast;
	pane->itemsLast = ptr;
}


// note: if a pane requires more than 65k entries then perhaps the data requires reorganising
static inline int paneGenId (TPANE *pane)
{
	if (++pane->idSrc < 2147483647)
		return pane->idSrc;
	else{
		//printf("paneGenId: oh oh\n");
		pane->idSrc = 101;
		return pane->idSrc;
	}
}

static inline int paneTextGetData (TPANE *pane, const int itemId, int64_t *data64a, int64_t *data64b)
{
	int ret = 0;
	
	if (data64a || data64b){
		TPANEOBJ *obj = paneItemIdToObject(pane, itemId);
		if (obj){
			if (data64a) *data64a =  obj->data.i64[0];
			if (data64b) *data64b =  obj->data.i64[1];
			ret = 1;
		}
	}
	return ret;
}

static inline int paneTextGetDetail (TPANE *pane, const int itemId, char **str, int *imgId)
{
	int ret = 0;
	TPANEOBJ *obj = paneItemIdToObject(pane, itemId);
	if (obj){
		if (str){
			*str = NULL;
			if (obj->text.itemId){
				*str = labelStringGet(pane->base, obj->text.itemId);
				ret += (*str != NULL);
			}
		}
		if (imgId){
			*imgId = obj->image.imgcId;
			ret += (*imgId != 0);
		}
	}
	return ret;
}

static inline int paneObjSetText (TPANE *pane, const int itemId, const char *string)
{
	int ret = labelStringSet(pane->base, itemId, string);
	if (ret){
		TPANEOBJ *obj = paneItemIdToObject(pane, itemId);
		if (obj)
			obj->text.metrics.height = 0;
	}
	return ret;
}

int paneTextReplace (TPANE *pane, const int itemId, const char *string)
{
	int ret = 0;

	if (ccLock(pane)){
		/*int rFlags = pane->flags.text.renderFlags;
		if (!pane->flags.text.singleLine){
			rFlags |= PF_CLIPWRAP;
			if (pane->flags.text.wordWrap)
				rFlags |= PF_WORDWRAP;
		}
		labelStringRenderFlagsSet(pane->base, itemId, rFlags);*/

		ret = paneObjSetText(pane, itemId, string);
		if (ret) paneSetInvalidated(pane);
		ccUnlock(pane);
	}
	return ret;
}

int paneImageReplace (TPANE *pane, const int imageId, const int newImageId)
{
	int ret = 0;

	if (ccLock(pane)){
		TPANEOBJ *obj = paneItemIdToObject(pane, imageId);
		if (obj && obj->type == PANE_OBJ_STRING){
			if (obj->text.hasIcon){
				obj->image.imgcId = newImageId;
				labelImgcSet(pane->base, imageId, newImageId, 0);
			}
		}
		ccUnlock(pane);
	}
	return ret;
}

static inline TPANEOBJ *paneObjNew (TPANE *pane, const int type)
{
	TPANEOBJ *obj = my_calloc(1, sizeof(TPANEOBJ));
	if (obj){
		obj->id = paneGenId(pane);
		obj->type = type;
	}
	return obj;
}

static inline void paneObjDelete (TPANEOBJ *obj)
{
	if (obj->cc.object)
		ccDelete(obj->cc.object);

	my_free(obj);
}

static inline int paneObjAddImage (TPANE *pane, const int imgcId, const double scale, const int dir, const int x, const int y, const int64_t data64a)
{
	int id = 0;

	TPANEOBJ *obj = paneObjNew(pane, PANE_OBJ_IMAGE);
	if (obj){
		obj->data.i64[0] = data64a;

		obj->image.imgcId = imgcId;
		obj->image.itemId = labelArtcCreateEx(pane->base, imgcId, scale, 0, x, y, 0, 0, 0.0, data64a);
		//labelItemDisable(pane->base, obj->image.itemId);
		//labelImageScaleSet(pane->base, obj->image.itemId, 0.5);
		//labelImgcHoverSet(pane->base, obj->image.itemId, COL_RED, 1.0);
		obj->image.dir = dir;
		obj->image.metrics.x = x;
		obj->image.metrics.y = y;
		obj->image.offset.x = x;
		obj->image.offset.y = y;

		id = obj->image.itemId;
		pane->flags.total.img++;

		paneObjInsertLast(pane, listNewItem(obj));
		paneSetInvalidated(pane);
	}
	return id;
}

static inline int paneObjAddText (TPANE *pane, int imgcId, const double scale, const char *string, const char *font, const uint32_t colour, const int64_t data64a, const int64_t data64b)
{
	int id = 0;

	TPANEOBJ *obj = paneObjNew(pane, PANE_OBJ_STRING);
	if (obj){
		obj->data.i64[0] = data64a;
		obj->data.i64[1] = data64b;
		
		if (imgcId){
			if (imgcId < 0){
				imgcId = abs(imgcId);
				obj->text.overlapIcon = 1;
			}
			obj->text.hasIcon = 1;
			obj->image.imgcId = imgcId;
			obj->image.itemId = labelArtcCreateEx(pane->base, imgcId, scale, 0, 0, 0, 1, COL_WHITE, 1.0, data64a);
		}

		//obj->text.string = my_strdup(string);
		int rFlags = pane->flags.text.renderFlags;
		if (!pane->flags.text.singleLine){
			rFlags |= BFONT_RENDER_NEWLINE | BFONT_RENDER_RETURN;
			//rFlags |= PF_CLIPWRAP;
			//if (pane->flags.text.wordWrap)
			//	rFlags |= PF_WORDWRAP;
		}

		obj->text.itemId = labelTextCreateEx(pane->base, string, rFlags, font, 0, 0, 0, data64a);
		//if (rFlags&PF_CLIPWRAP)
		if (!pane->flags.text.singleLine)
			labelStringSetMaxWidth(pane->base, obj->text.itemId, pane->metrics.width);

#if 1
		//labelRenderBlurRadiusSet(pane->base, obj->text.itemId, 2);
		//labelRenderFilterSet(pane->base, obj->text.itemId, 1);
		//if (colour != (uint32_t)(255<<24|COL_WHITE))
			labelRenderColourSet(pane->base, obj->text.itemId, colour, 255<<24|COL_BLACK, 177<<24|COL_BLUE_SEA_TINT);

#endif

		id = obj->text.itemId;
		pane->flags.total.text++;

		paneObjInsertLast(pane, listNewItem(obj));
		paneSetInvalidated(pane);
	}
	return id;
}

int paneTextAddEx (TPANE *pane, const int imgcId, const double imgScale, const char *string, const char *font, const uint32_t colour, const int64_t data64a, const int64_t data64b)
{
	int id = 0;
	if (ccLock(pane)){
		id = paneObjAddText(pane, imgcId, imgScale, string, font, colour, data64a, data64b);
		ccUnlock(pane);
	}
	return id;
}

int paneTextAdd (TPANE *pane, const int imgcId, const double scale, const char *string, const char *font, const int64_t data64a)
{
	return paneTextAddEx(pane, imgcId, scale, string, font, PANE_COLOUR_TEXT, data64a, 0);
}


void paneTableSetLayoutMetrics (TPANE *pane, pane_tablelayout_t *layout)
{
	if (ccLock(pane)){
		pane->layout.table.horiMin = layout->horiMin;
		pane->layout.table.horiMax = layout->horiMax;
		pane->layout.table.vertMin = layout->vertMin;
		pane->layout.table.vertMax = layout->vertMax;
		pane->layout.table.cellSpaceHori = layout->cellSpaceHori;
		pane->layout.table.cellSpaceVert = layout->cellSpaceVert;
		pane->layout.table.pixelsPerUnit = layout->pixelsPerUnit;
		pane->layout.metrics.vertLineHeight = layout->cellHeight;
		ccUnlock(pane);
	}
}
	
static inline int paneObjAddCell (TPANE *pane, const uint32_t colour, const int32_t row, const int32_t position, const int32_t width, const int64_t data64a)
{
	TPANEOBJ *obj = paneObjNew(pane, PANE_OBJ_CELL);
	if (obj){
		obj->data.i64[0] = data64a;
		obj->data.i64[1] = 0;

		obj->cell.colour = colour;
		obj->cell.row = row;
		obj->cell.position = position;
		obj->cell.width = width;

		pane->flags.total.cell++;

		paneObjInsertLast(pane, listNewItem(obj));
		paneSetInvalidated(pane);
		
		return obj->id;
	}
	return 0;
}


static inline int paneObjCellIsOverlapped (TPANE *pane, TPANEOBJ *obj, const int x, const int y)
{
	TMETRICS *off = &pane->metrics;
	TMETRICS *met = &obj->cell.metrics;
	
	int x1 = off->x + met->x;
	int y1 = off->y + met->y;
	int x2 = off->x + met->x + met->width-1;
	int y2 = off->y + met->y + met->height-1;

	if (y >= y1 && y < y2){
		if (x >= x1 && x < x2)
			return 1;
	}
	return 0;
}

/*
#define COL_ORANGE				(0xFF7F11)
#define COL_BLUE_SEA_TINT		(0x508DC5)	
#define COL_GREEN_TINT			(0x00FF1E)	
#define COL_PURPLE_GLOW			(0xFF10CF)	
#define COL_HOVER				(0xB328C672)
#define COL_TASKBARFR			(0x141414)
#define COL_TASKBARBK			(0xD4CAC8)
#define COL_SOFTBLUE			(0X7296D3)
*/

static inline void paneCellRender (TPANE *pane, TPANEOBJ *obj, TFRAME *frame)
{
	TMETRICS *off = &pane->metrics;
	TMETRICS *met = &obj->cell.metrics;

	int x1 = off->x + met->x;
	int y1 = off->y + met->y;
	int x2 = off->x + met->x + met->width-1;
	int y2 = off->y + met->y + met->height-1;
	
	x1 = max(x1, pane->metrics.x);
	y1 = max(y1, pane->metrics.y);
	x2 = min(x2, (pane->metrics.x+pane->metrics.width-1));
	y2 = min(y2, (pane->metrics.y+pane->metrics.height-1));
	if ((y2 <= y1) || (x2 <= x1)) return;

	drawRectangleFilled(frame, x1, y1, x2, y2, COLOUR_24TO16(obj->cell.colour));
	drawRectangle(frame, x1, y1, x2, y2, COLOUR_24TO16(COL_BLUE_SEA_TINT));
	
	if (pane->processInput && pane->isHovered){
		int isHovered = paneObjCellIsOverlapped(pane, obj, pane->cc->cursor->dx, pane->cc->cursor->dy);
		if (isHovered){
			uint32_t colour = COLOUR_24TO16(COL_BLUE_SEA_TINT);
			drawRectangle(frame, x1-1, y1-1, x2+1, y2+1, colour);
			drawRectangle(frame, x1-2, y1-2, x2+2, y2+2, colour);
		}
	}
}
	
static inline void paneCellsRender (TPANE *pane, TFRAME *frame)
{
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && obj->type == PANE_OBJ_CELL && obj->cell.isEnabled)
			paneCellRender(pane, obj, frame);
		item = listGetNext(item);
	}
}

static inline TPANEOBJ *paneFindData (TPANE *pane, const int64_t data64, const int dIdx, const uint64_t mask)
{
	TLISTITEM *item = pane->items;
	
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && obj->type == PANE_OBJ_CELL){
			if ((obj->data.i64[dIdx]&mask) == data64)
				return obj;
		}
		item = listGetNext(item);
	}
	
	return NULL;
}

static inline void paneObjCellSetColour (TPANE *pane, const int cellId, const uint32_t colour)
{
	TPANEOBJ *obj = paneIdToObject(pane, cellId);
	if (obj)
		obj->cell.colour = colour;
}

void paneCellColourSet (TPANE *pane, const int cellId, const uint32_t colour)
{
	if (cellId && ccLock(pane)){
		paneObjCellSetColour(pane, cellId, colour);
		ccUnlock(pane);
	}
}

static inline uint32_t paneObjCellGetColour (TPANE *pane, const int cellId)
{
	TPANEOBJ *obj = paneIdToObject(pane, cellId);
	if (obj)
		return obj->cell.colour;
	return 0;
}

uint32_t paneCellColourGet (TPANE *pane, const int cellId)
{
	uint32_t ret = 0;

	if (cellId && ccLock(pane)){
		ret = paneObjCellGetColour(pane, cellId);
		ccUnlock(pane);
	}
	return ret;
}

TPANEOBJ *paneCellFind (TPANE *pane, const int64_t data64a, const uint64_t mask)
{
	TPANEOBJ *obj = NULL;
	
	if (ccLock(pane)){
		obj = paneFindData(pane, data64a, 0, mask);
		ccUnlock(pane);
	}
	return obj;
}

int paneCellCreate (TPANE *pane, const uint32_t colour, const int32_t row, const int32_t position, const int32_t width, const int64_t data64a)
{
	int id = 0;
	if (ccLock(pane)){
		id = paneObjAddCell(pane, colour, row, position, width, data64a);
		ccUnlock(pane);
	}
	return id;
}

static inline void paneObjRemoveCell (TPANE *pane, const int cellId, const int removeAllCells)
{
	TLISTITEM *item = pane->items;
	while(item){
		TLISTITEM *next = listGetNext(item);
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && obj->type == PANE_OBJ_CELL){
			if (removeAllCells || (cellId && (obj->id == cellId))){
				ccSendMessage(pane, PANE_MSG_CELL_DELETE, 0, obj->id, &obj->data.i64[0]);
			
				for (int i = 0; i < PANE_CELL_MAXITEMS; i++){
					if (obj->cell.item[i].id)
						labelItemDelete(pane->base, obj->cell.item[i].id);
				}
				pane->flags.total.cell--;
				paneObjDelete(obj);
				
				if (pane->items == item) pane->items = item->next;
				if (pane->itemsLast == item) pane->itemsLast = item->prev;
				if (pane->firstEnabledItem == item) pane->firstEnabledItem = pane->items;
				if (pane->lastEnabledItem == item) pane->lastEnabledItem = NULL;

				listRemove(item);
				listDestroy(item);
				if (!removeAllCells){
					paneSetInvalidated(pane);
					return;
				}
			}
		}
		item = next; //listGetNext(item);
	}
	paneSetInvalidated(pane);
}

void paneCellRemoveAll (TPANE *pane)
{
	if (ccLock(pane)){
		paneObjRemoveCell(pane, 0, 1);
		ccUnlock(pane);
	}
}

void paneCellRemove (TPANE *pane, const int cellId)
{
	if (ccLock(pane)){
		paneObjRemoveCell(pane, cellId, 0);
		ccUnlock(pane);
	}
}

int paneCellSetStringData (TPANE *pane, const int cellId, const int idx, int64_t data64)
{
	int ret = 0;
	if (ccLock(pane)){
		TPANEOBJ *obj = paneIdToObject(pane, cellId);
		if (obj){
			obj->cell.item[idx].data64 = data64;
			ret = 1;
		}
		ccUnlock(pane);
	}
	return ret;
}

int paneCellGetStringData (TPANE *pane, const int cellId, const int idx, int64_t *data64)
{
	int ret = 0;	
	if (ccLock(pane)){
		TPANEOBJ *obj = paneIdToObject(pane, cellId);
		if (obj){
			*data64 = obj->cell.item[idx].data64;
			ret = 1;
		}
		ccUnlock(pane);
	}
	return ret;
}


int paneCellAddString (TPANE *pane, const int cellId, const char *string, const char *font, const uint32_t colour, const int flags, const int x, const int y, const int64_t data64)
{
	int itemId = 0;
	
	if (ccLock(pane)){
		TPANEOBJ *obj = paneIdToObject(pane, cellId);
		if (obj){
			int rFlags = flags | pane->flags.text.renderFlags;

			for (int i = 0; i < PANE_CELL_MAXITEMS; i++){
				if (!obj->cell.item[i].id){
					itemId = labelTextCreateEx(pane->base, string, rFlags, font, 0, 0, 0, data64);
					if (itemId){
						obj->cell.item[i].id = itemId;
						obj->cell.item[i].data64 = data64;
						
						int len = obj->cell.width/pane->layout.table.pixelsPerUnit;
						int _x = min(len-2, x);
						_x = max(0, _x);
						
						//printf("addString: %i: %i %i %i: '%s'\n", itemId, len, _x, obj->cell.item[i].pos.y, string);

						obj->cell.item[i].pos.x = _x;
						obj->cell.item[i].pos.y = max(0, min(pane->layout.metrics.vertLineHeight-pane->layout.table.cellSpaceVert, y));
						
						len -= obj->cell.item[i].pos.x;
						int height = pane->layout.metrics.vertLineHeight - obj->cell.item[i].pos.y;
						labelStringSetMetricLimit(pane->base, itemId, len-2, height);
						labelRenderColourSet(pane->base, itemId, colour, 255<<24|COL_BLUE, 255<<24|COL_GRAY);
					}
					break;
				}
			}
		}
		ccUnlock(pane);
	}
	return itemId;
}

int paneImageAdd (TPANE *pane, const int imgcId, const double scale, const int dir, const int x, const int y, const int64_t data64)
{
	int id = 0;
	if (ccLock(pane)){
		id = paneObjAddImage(pane, imgcId, scale, dir, x, y, data64);
		ccUnlock(pane);
	}
	return id;
}

int paneItemGetData (TPANE *pane, const int itemId, int64_t *data64a)
{
	int ret = 0;
	if (ccLock(pane)){
		ret = paneTextGetData(pane, itemId, data64a, NULL);
		ccUnlock(pane);
	}
	return ret;
}

int paneItemGetDataEx (TPANE *pane, const int itemId, int64_t *data64a, int64_t *data64b)
{
	int ret = 0;
	if (ccLock(pane)){
		ret = paneTextGetData(pane, itemId, data64a, data64b);
		ccUnlock(pane);
	}
	return ret;
}

int paneItemGetDetail (TPANE *pane, const int itemId, char **str, int *imgId)
{
	int ret = 0;
	if (ccLock(pane)){
		ret = paneTextGetDetail(pane, itemId, str, imgId);
		ccUnlock(pane);
	}
	return ret;
}

int paneIndexToItemId (TPANE *pane, const int idx)
{
	int itemId = 0;

	if (ccLock(pane)){
		TPANEOBJ *obj = paneItemIdxToObject(pane, idx);
		if (obj){
			if (obj->text.itemId)
				itemId = obj->text.itemId;
			else
				itemId = obj->image.itemId;
		}
		ccUnlock(pane);
	}

	return itemId;
}

void paneTextMultilineEnable (TPANE *pane)
{
	pane->flags.text.singleLine = 0;
}

void paneTextMultilineDisable (TPANE *pane)
{
	pane->flags.text.singleLine = 1;
}

void paneTextWordwrapEnable (TPANE *pane)
{
	pane->flags.text.wordWrap = 1;
}

void paneTextWordwrapDisable (TPANE *pane)
{
	pane->flags.text.wordWrap = 0;
}

void paneSwipeEnable (TPANE *pane)
{
	pane->input.slideEnabled = 1;
}

void paneSwipeDisable (TPANE *pane)
{
	pane->input.slideEnabled = 0;
}

void paneDragEnable (TPANE *pane)
{
	pane->input.drag.state = PANE_SLIDE_NONE;
}

void paneDragDisable (TPANE *pane)
{
	pane->input.drag.state = PANE_SLIDE_DISABLED;
}

void paneSetAcceleration (TPANE *pane, double x, double y)
{
	if (ccLock(pane)){
		if (x < 1.0) x = 1.0;
		if (y < 1.0) y = 1.0;
		pane->input.acceleration.x = x;
		pane->input.acceleration.y = y;

		//printf("pane->input.acceleration.x %i %f %f\n", pane->id, pane->input.acceleration.x, pane->input.acceleration.y);
		ccUnlock(pane);
	}
}

void paneSetLayout (TPANE *pane, const int layoutMode)
{
	if (ccLock(pane)){
		pane->layout.type = layoutMode;
		pane->base->isChild = (layoutMode != PANE_LAYOUT_TABLE);
		
		paneSetInvalidated(pane);
		ccUnlock(pane);
	}
}

static inline int _paneScrollXY (TPANE *pane, const int xPixels, const int yPixels)
{
	if (pane->layout.type == PANE_LAYOUT_HORI || pane->layout.type == PANE_LAYOUT_HORICENTER){
		pane->offset.x += xPixels;
		if (-pane->offset.x >= pane->pos.x) pane->offset.x = -pane->pos.x;
	
	}else if (pane->layout.type == PANE_LAYOUT_VERT || pane->layout.type == PANE_LAYOUT_VERTCENTER){
		pane->offset.y += yPixels;
		if (-pane->offset.y >= pane->pos.y) pane->offset.y = -pane->pos.y;

	}else if (pane->layout.type == PANE_LAYOUT_TABLE){
		pane->offset.x += xPixels;
		//if (-pane->offset.x >= pane->pos.x) pane->offset.x = -pane->pos.x;
		pane->offset.y += yPixels;
		//if (-pane->offset.y >= pane->pos.y) pane->offset.y = -pane->pos.y;
	}

	//printf("paneSCroll %i %i\n", pane->pos.y, pane->offset.y);
	paneSetInvalidated(pane);

	return 1;
}

static inline int _paneScrollN (TPANE *pane, const int nPixels)
{
	if (pane->layout.type == PANE_LAYOUT_HORI || pane->layout.type == PANE_LAYOUT_HORICENTER){
		pane->offset.x += nPixels;
		if (-pane->offset.x >= pane->pos.x) pane->offset.x = -pane->pos.x;
	
	}else if (pane->layout.type == PANE_LAYOUT_VERT || pane->layout.type == PANE_LAYOUT_VERTCENTER){
		pane->offset.y += nPixels;
		if (-pane->offset.y >= pane->pos.y) pane->offset.y = -pane->pos.y;
		
	}else if (pane->layout.type == PANE_LAYOUT_TABLE){
		printf("pane.c::_paneScrollN():: fixme\n");
		pane->offset.x += nPixels;
		pane->offset.y += nPixels;
		if (-pane->offset.x >= pane->pos.x) pane->offset.x = -pane->pos.x;
		if (-pane->offset.y >= pane->pos.y) pane->offset.y = -pane->pos.y;
	}

	//printf("paneSCroll %i %i\n", pane->pos.y, pane->offset.y);
	paneSetInvalidated(pane);

	return 1;
}

int paneScrollGet (TPANE *pane, int *x, int *y)
{
	int ret = 0;
	
	if (ccLock(pane)){
		ret = 1;
		if (x) *x = pane->offset.x;
		if (y) *y = pane->offset.y;
		ccUnlock(pane);
	}
	return ret;
}

int paneScrollSet (TPANE *pane, const int x, const int y, const int invalidate)
{
	int ret = 0;
	
	if (ccLock(pane)){
		ret = 1;
		pane->offset.x = x;
		pane->offset.y = y;
		//printf("paneScrollSet %i,%i\n", pane->offset.x, pane->offset.y, invalidate);

		if (invalidate) paneSetInvalidated(pane);
		ccUnlock(pane);
	}
	return ret;
}

int paneScroll (TPANE *pane, const int nPixels)
{
	int ret = 0;
	
	if (ccLock(pane)){
		ret = 1;
		_paneScrollN(pane, nPixels);
		//printf("paneScroll %i  %i,%i\n", nPixels, pane->offset.x, pane->offset.y);
		ccUnlock(pane);
	}
	return ret;
}

int paneScrollReset (TPANE *pane)
{
	int ret = 0;
	
	if (ccLock(pane)){
		ret = 1;
		pane->offset.x = 0;
		pane->offset.y = 0;
		pane->layout.direction = PANE_DIRECTION_STOP;
		pane->previous.x = 0;
		pane->previous.y = 0;
		paneSetInvalidated(pane);
		ccUnlock(pane);
	}
	return ret;
}

static inline int paneValidateLayout (TPANE *pane)
{
	//const double t0 = com_getTime(pane->cc);

	int count = 0;
	if (pane->layout.type == PANE_LAYOUT_HORI)
		count = paneValidateHori(pane);
	else if (pane->layout.type == PANE_LAYOUT_VERT)
		count = paneValidateVert(pane);
	else if (pane->layout.type == PANE_LAYOUT_HORICENTER)
		count = paneValidateHoriMiddle(pane);
	else if (pane->layout.type == PANE_LAYOUT_VERTCENTER)
		count = paneValidateVertMiddle(pane);
	else if (pane->layout.type == PANE_LAYOUT_TABLE)
		count = paneValidateTable(pane);
	else
		return 0;

	//const double t1 = com_getTime(pane->cc);
	//printf("paneValidate: time: %.2fms [%i of %i]\n", t1-t0, count, pane->flags.total.cell);

	ccSendMessage(pane, PANE_MSG_VALIDATED, count, 0, 0);
	return count;
}

static inline int paneFocusSetHoriMiddle (TPANE *pane, const int itemId)
{
	int x = 0;
	if (labelItemPositionGet(pane->base, itemId, &x, NULL)){
		//printf("paneFocusSetHoriMiddle: offset %i %i\n", pane->offset.x, x);
		x += abs(pane->offset.x);
		//x -= (pane->metrics.width - labelImgcGetWidth(pane->base, itemId))/2;	// will center item

		pane->offset.x = -x;
		return 1;
	}
	return 0;
}

// TODO: add other focus views
int paneFocusSet (TPANE *pane, const int itemId)
{
	printf("paneFocusSet in %X\n", itemId);

	if (ccLock(pane)){
		if (paneValidateLayout(pane)){
			if (pane->layout.type == PANE_LAYOUT_HORICENTER || pane->layout.type == PANE_LAYOUT_HORI){
				paneFocusSetHoriMiddle(pane, itemId);

			}else if (pane->layout.type == PANE_LAYOUT_TABLE){
				printf("pane.c::paneFocusSet(): fixme\n");
			}
		
			
			/*if (pane->layout.type == PANE_LAYOUT_HORI){
				TPANEOBJ *obj = paneItemIdxToObject(pane, itemId);
				printf("paneItemIdxToObject %i %i\n", obj->text.itemId, obj->image.itemId);
				if (obj){
					int x = 0;
					if (obj->text.itemId)
						labelItemPositionGet(pane->base, obj->text.itemId, &x, NULL);

					if (!x)
						labelItemPositionGet(pane->base, obj->image.itemId, &x, NULL);

					if (x){
						printf("focusSet x:%i\n", x);
						x += abs(pane->offset.x);
						pane->offset.x = -x;
					}
				}
			}*/
			paneSetInvalidated(pane);
		}
		ccUnlock(pane);
	}

	//printf("paneFocusSet out\n");
	return 1;
}

int paneRender (void *object, TFRAME *frame)
{
	//printf("paneRender\n");

	TPANE *pane = (TPANE*)object;

	if (paneGetInvalidated(pane)){
		if (!paneValidateLayout(pane))
			return 0;
	}

	if (paneHasCells(pane))
		paneCellsRender(pane, frame);

	//double t0 = com_getTime(pane->cc);
	ccRender(pane->base, frame);
	//double t1 = com_getTime(pane->cc);
	//printf("paneRender: %.2fms\n", t1-t0);

	point_t cur;
	ccInputGetPosition(pane, &cur);
	TPANEINPUT *input = &pane->input;

	if (input->slideMode == PANE_SLIDEMODE_ITEM/* && input->drag.state == PANE_SLIDE_SLIDE*/){
		if (ccGetState(input->drag.label)){
			int x = cur.x - input->drag.dx;
			if (x < 0) x = 0;
			int y = cur.y - input->drag.dy;
			if (y < 0) y = 0;

			ccSetPosition(input->drag.label, x, y);
			ccRender(input->drag.label, frame);
		}
	}

	return 1;
}

void paneEnable (void *object)
{
	TPANE *pane = (TPANE*)object;

	pane->enabled = 1;
	ccEnable(pane->base);
}

void paneDisable (void *object)
{
	TPANE *pane = (TPANE*)object;

	pane->enabled = 0;
	ccDisable(pane->base);
	ccDisable(pane->input.drag.label);
}

static inline int paneObjCellInput (TPANE *pane, TPANEOBJ *obj, TTOUCHCOORD *pos, const int flags)
{
	//printf("paneCellInput %i %i\n", flags, TOUCH_VINPUT);


	const int x1 = ccGetPositionX(pane);
	//const int x2 = x1 + ccGetWidth(pane)-1;
	const int y1 = ccGetPositionY(pane);
	//const int y2 = y1 + ccGetHeight(pane)-1;
	
		
	//TTOUCHCOORD dpos;
	const int x = abs(x1 - pos->x);
	const int y = abs(y1 - pos->y);
	/*my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
	dpos.x = abs(x1 - pos->x);
	dpos.y = abs(y1 - pos->y);*/
		
	int64_t i64 = obj->data.i64[0];
		
	int ret = 0;
	if (!flags)												// press down
		ret = ccSendMessage(pane, PANE_MSG_CELL_SELECTED_PRESS,   ((x&0xFFFF)<<16)|(y&0xFFFF), obj->id, &i64);
	else if (flags == 1 && pos->id == pane->touchInputId)	// drag
		ret = ccSendMessage(pane, PANE_MSG_CELL_SELECTED_SLIDE,   ((x&0xFFFF)<<16)|(y&0xFFFF), obj->id, &i64);
	else if (flags == 3)									// up/release
		ret = ccSendMessage(pane, PANE_MSG_CELL_SELECTED_RELEASE, ((x&0xFFFF)<<16)|(y&0xFFFF), obj->id, &i64);

	//printf("paneCellInput ret = %i\n", ret);

	return (ret != -1);
}

int paneCellIsHovered (TPANE *pane, const int x, const int y)
{
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj){
		 	if (obj->type == PANE_OBJ_CELL && obj->cell.isEnabled){
				if (paneObjCellIsOverlapped(pane, obj, x, y)){
					//printf("paneInput: cell: %i\n", obj->id);
					return 1;
				}
			}
		}
		item = listGetNext(item);
	}
	return 0;
}

static inline TPANEOBJ *paneObjCellIsHovered (TPANE *pane, const int x, const int y)
{
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj){
		 	if (obj->type == PANE_OBJ_CELL && obj->cell.isEnabled){
				if (paneObjCellIsOverlapped(pane, obj, x, y)){
					//printf("paneInput: cell: %i\n", obj->id);
					return obj;
				}
			}
		}
		item = listGetNext(item);
	}
	return NULL;
}

int paneInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TPANE *pane = (TPANE*)object;
	//printf("__ paneInput %i %i %i\n", pos->x, pos->y, flags);

#if 1
	if (pane->processInput && paneHasCells(pane)){
		if (pane->input.slideEnabled || pane->input.drag.state != PANE_SLIDE_DISABLED){
			if (!pos->pen && ((flags&0xFF) == 1)){		// drag/slide
				if (pos->id != pane->touchInputId)
					return -1;
			}else if (!pos->pen && !(flags&0xFF)){
				if (pos->id == pane->touchInputId){
					return -1;
				}else{
					pane->touchInputId = pos->id;
				}
			}
		}
		TPANEOBJ *obj = paneObjCellIsHovered(pane, pos->x, pos->y);
		if (obj){
			//printf("paneInput: cell: %i\n", obj->id);
			return paneObjCellInput(pane, obj, pos, flags&0xFFF);
		}
	}
#endif
	return ccHandleInput(pane->base, pos, flags);
}

int paneSetPosition (void *object, const int x, const int y)
{
	TPANE *pane = (TPANE*)object;

	pane->metrics.x = x;
	pane->metrics.y = y;

	_rect_t clip = {x, y, x+pane->metrics.width-1, y+pane->metrics.height-1};
	labelRenderSetClipRegion(pane->base, &clip);
	int ret = ccSetPosition(pane->base, x, y);
	paneSetInvalidated(pane);
	return ret;
}

int paneSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TPANE *pane = (TPANE*)object;

	pane->metrics.width = width;
	pane->metrics.height = height;
	paneSetPosition(pane, x, y);

	return ccSetMetrics(pane->base, x, y, width, height);
}

static inline void paneItemDelete (TPANE *pane, const int itemId)
{
	if (!itemId) return;
	
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj && (obj->text.itemId == itemId || obj->image.itemId == itemId)){
			if (obj->text.itemId){
				labelItemDelete(pane->base, obj->text.itemId);
				pane->flags.total.text--;
			}
			if (obj->image.itemId){
				labelItemDelete(pane->base, obj->image.itemId);
				if (obj->type == PANE_OBJ_IMAGE)
					pane->flags.total.img--;
			}

			paneObjDelete(obj);

			//printf("paneItemDelete %i %p %p\n", itemId, item->prev, item->next);

			if (pane->items == item) pane->items = item->next;
			if (pane->itemsLast == item) pane->itemsLast = item->prev;
			if (pane->firstEnabledItem == item) pane->firstEnabledItem = pane->items;
			if (pane->lastEnabledItem == item) pane->lastEnabledItem = NULL;

			listRemove(item);
			listDestroy(item);
			paneSetInvalidated(pane);

			return;
		}
		item = listGetNext(item);
	}
}

void paneRemoveItem (TPANE *pane, const int itemId)
{
	if (ccLock(pane)){
		paneItemDelete(pane, itemId);
		ccUnlock(pane);
	}
}

void paneRemoveAll (TPANE *pane)
{
	if (ccLock(pane)){
		pane->firstEnabledItem = NULL;
		pane->lastEnabledItem = NULL;

		TLISTITEM *item = pane->items;
		while(item){
			TPANEOBJ *obj = listGetStorage(item);
			if (obj) paneObjDelete(obj);
			item = listGetNext(item);
		}
		listDestroyAll(pane->items);

		pane->items = NULL;
		pane->itemsLast = NULL;

		labelItemsDelete(pane->base);

		pane->firstEnabledImgId = 0;
		pane->firstEnabledStrId = 0;
		pane->firstEnabledImgIdx = 0;
		pane->firstEnabledStrIdx = 0;

		/*printf("\noverall: %.5f for pane:%i/label:%i items\n", t3-t0, totala, totalb);
		printf("paneObjDelete: %.5f\n", t1-t0);
		printf("listDestroyAll: %.5f\n", t2-t1);
		printf("labelItemsDelete: %.5f\n", t3-t2);
		printf("%f per item\n\n", (t3-t2)/(double)totalb);*/

#if 0
		pane->offset.x = 0;
		pane->offset.y = 0;
#endif

		pane->flags.total.text = 0;
		pane->flags.total.img = 0;
		pane->idSrc = 100;
		ccUnlock(pane);
	}
}

void paneDelete (void *object)
{
	TPANE *pane = (TPANE*)object;

	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj) paneObjDelete(obj);
		item = listGetNext(item);
	}
	if (pane->items)
		listDestroyAll(pane->items);

	ccDelete(pane->input.drag.label);
	ccDelete(pane->base);
}

static inline int paneSendPageSelectedHover (TPANE *pane, const int msg, const int64_t id, void *ptr)
{
	/*int state;
	if (msg == PANE_SLIDE_PRESS)
		state = PAGE_OBJ_HOVER_HELD;
	else if (msg == PANE_SLIDE_SLIDE)
		state = PAGE_OBJ_HOVER_SLIDE;
	else if (msg == PANE_SLIDE_RELEASE)
		state = PAGE_OBJ_HOVER_RELEASE;
	else if (msg == PANE_SLIDE_NONE)
		state = PAGE_OBJ_HOVER_NONE;
	else
		return 0;


	return pageSendMessage(pane->cc->vp->pages, pane->pageOwner, PAGE_MSG_OBJ_HOVER, state, id, ptr);
	*/
	//printf("pane.c::paneSendPageSelectedHover: finish me\n");
	return 0;
}

#if 0
static inline int paneDoSelected (TPANE *pane, int msg, const int dx, const int dy, const int id, const int64_t dataInt64, TTOUCHCOORD *pos)
{
	const double dt = com_getTimegetTime(pane->cc) - pane->input.start.time;

	if (0 || (dx < 9 && dy < 9)){
		pos->dt = dt;

		//printf("pane press time %i %i - %.2f\n", msg, id, dt);

		if (dt < 220.0){
			ccSendMessage(pane, msg, id, dataInt64, pos);
			//renderSignalUpdate(pane->cc->vp);
			pane->isHovered = 0;
			com_renderSignalUpdate(pane->cc);
			return 1;

		}else if (dt > 1600.0 && dt < 4000.0){
			printf("pane held %i %i - %.2f\n", msg, id, dt);

			if (msg == PANE_MSG_IMAGE_SELECTED)
				msg = PANE_MSG_IMAGE_SELECT_HELD;
			else if (msg == PANE_MSG_TEXT_SELECTED)
				msg = PANE_MSG_TEXT_SELECT_HELD;
			ccSendMessage(pane, msg, id, dataInt64, pos);

			//renderSignalUpdate(pane->cc->vp);
			pane->isHovered = 0;
			com_renderSignalUpdate(pane->cc);
			return 1;
		}
	}
	return 0;
}
#endif

static inline int64_t cclblDrag_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	TLABEL *label = (TLABEL*)object;

	//if (msg != CC_MSG_RENDER && msg != CC_MSG_SETPOSITION)
	//	printf("cclbldrag_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", label->id, label->type, msg, (int)data1, (int)data2, dataPtr);


	if (msg == CC_MSG_ENABLED){
		TPANE *pane = ccGetUserData(label);
		TPANEINPUT *input = &pane->input;

		if (input->drag.state != PANE_SLIDE_DISABLED)
			ccInputSlideHoverEnable(label);

	}else if (msg == CC_MSG_DISABLED){
		TPANE *pane = ccGetUserData(label);
		TPANEINPUT *input = &pane->input;

		ccInputSlideHoverDisable(label);

		if (input->drag.state != PANE_SLIDE_DISABLED){
			input->drag.state = PANE_SLIDE_NONE;
			paneSendPageSelectedHover(pane, input->drag.state, input->drag.heldId, dataPtr);
		}
	}else if (msg == LABEL_MSG_TEXT_SELECTED_RELEASE || msg == LABEL_MSG_BASE_SELECTED_RELEASE){
		TPANE *pane = ccGetUserData(label);
		TPANEINPUT *input = &pane->input;

		//printf("# ccdrag input->drag.state %i\n", input->drag.state);

		if (input->drag.state != PANE_SLIDE_DISABLED){
			TPANE *pane = ccGetUserData(label);
			TPANEINPUT *input = &pane->input;
			TTOUCHCOORD *pos = dataPtr;

			if (input->drag.state == PANE_SLIDE_PRESS || input->drag.state == PANE_SLIDE_SLIDE)
				ccSendMessage(pane, PANE_MSG_SLIDE_RELEASE, input->drag.heldId, labelItemDataGet(pane->base, input->drag.heldId), pos);

			input->drag.state = PANE_SLIDE_NONE;
	  		if (ccGetState(label))
				ccDisable(label);
		}
	}else if (msg == CC_MSG_CREATE){
		TPANE *pane = ccGetUserData(label);
		TPANEINPUT *input = &pane->input;

		input->drag.label = label;
		input->drag.state = PANE_SLIDE_DISABLED;
		input->drag.itemId = labelTextCreateEx(label, " ", LABEL_ALLIGN_MIDDLE, PANE_FONT, 0, 0, 0, /*udata*/0);
		labelRenderFlagsSet(label, LABEL_RENDER_TEXT|LABEL_RENDER_BORDER_PRE/*|LABEL_RENDER_BORDER_POST*/|LABEL_RENDER_BASE|LABEL_RENDER_BLUR);

		const uint32_t colourPre[] = {190<<24 | COL_WHITE, 155<<24 | COL_BLUE_SEA_TINT};
		labelBorderProfileSet(label, LABEL_BORDER_SET_PRE, colourPre, 2);
		labelBaseColourSet(label, 40<<24|COL_BLUE_SEA_TINT);
		ccInputDisable(label);
		paneDragDisable(pane);
	}
	return 1;
}

static inline int paneSelectPress (TPANE *pane, const int x, const int y, TTOUCHCOORD *pos)
{
	//printf("paneSelectPress %i %i\n", x, y);

	TPANEINPUT *input = &pane->input;

	input->state = PANE_INPUTSTATE_PRESSED;
	input->start.time = com_getTime(pane->cc);
	input->start.id = pos->id;
	input->start.x = x;			// start location
	input->start.y = y;
	input->last.x = x;			// previous location
	input->last.y = y;
	input->delta.x = 0;			// delta movement from previous event to this event
	input->delta.y = 0;
	input->travelled.x = 0;		// total points travelled in either direction
	input->travelled.y = 0;

	ccSendMessage(pane, PANE_MSG_BASE_SELECTED_PRESS, x<<16|y, input->start.time, pos);
	
	
	pos->x += pane->metrics.x;
	pos->y += pane->metrics.y;
	
		TLISTITEM *item = pane->items;
		while(item){
			TPANEOBJ *obj = listGetStorage(item);
			if (obj){
			 	if (obj->type == PANE_OBJ_CELL && obj->cell.isEnabled){
				
					if (paneObjCellIsOverlapped(pane, obj, pos->x, pos->y)){
						//printf("paneInput: cell: %i\n", obj->id);
						return paneObjCellInput(pane, obj, pos, 0);
					}
					
				}
			}
			item = listGetNext(item);
		}
	
	
	return 1;
}

static inline int paneSelectSlide (TPANE *pane, const int x, const int y, TTOUCHCOORD *pos)
{
	//printf("paneSelectSlide %i %i\n", x ,y);
	
	TPANEINPUT *input = &pane->input;

	if (input->state == PANE_INPUTSTATE_RELEASED)
		return -1;

	const float dt = com_getTime(pane->cc) - pane->input.start.time;
	if (input->state == PANE_INPUTSTATE_SLIDE && dt < 50.0f) return 0;

	//printf("*** paneSelectSlide: state:%i %i, dt:%.1f, %i/%i, %i/%i\n", input->state, input->drag.state, dt, x, y, pos->x, pos->y);

	input->state = PANE_INPUTSTATE_SLIDE;
	input->delta.x = (x - input->last.x) * input->acceleration.x;
	input->delta.y = (y - input->last.y) * input->acceleration.y;
	input->delta.time = com_getTime(pane->cc) - pane->input.start.time;
	input->travelled.x += abs(input->delta.x);
	input->travelled.y += abs(input->delta.y);
	input->last.x = x;
	input->last.y = y;

	if (!pane->input.slideEnabled)
		return -1;

	//const int area = abs(input->travelled.x) * abs(input->travelled.y);
	//printf("paneSelectSlide: area %i %i %i\n", area, input->travelled.x, input->travelled.y);

	int ret = -1;
	if (input->slideMode == PANE_SLIDEMODE_PANE){
		point_t delta = {input->delta.x, input->delta.y};
		if (ccSendMessage(pane, PANE_MSG_SLIDE, pane->offset.x, pane->offset.y, &delta)){
			//_paneScrollXY(pane, input->delta.x, input->delta.y);
			_paneScrollXY(pane, delta.x, delta.y);
		}

	}else if (input->slideMode == PANE_SLIDEMODE_ITEM){
		ret = 1;
		if (input->drag.state == PANE_SLIDE_PRESS){
			int dx = abs(input->travelled.x);
			if (dx < 1) dx = 1;
			int dy = abs(input->travelled.y);
			if (dy < 1) dy = 1;
			const int area =  dx * dy;
			//printf("paneDragSelectSlide area %i\n", area);

			if (area >= PANE_GRAB_AREA){
				input->drag.state = PANE_SLIDE_SLIDE;

				if (!ccGetState(input->drag.label)){
					ccEnable(input->drag.label);
				}
			}
		}
	}
	
	pos->x += pane->metrics.x;
	pos->y += pane->metrics.y;
	TLISTITEM *item = pane->items;
	while(item){
		TPANEOBJ *obj = listGetStorage(item);
		if (obj){
		 	if (obj->type == PANE_OBJ_CELL && obj->cell.isEnabled){
			
				if (paneObjCellIsOverlapped(pane, obj, pos->x, pos->y)){
					//printf("paneInput: cell slide: %i\n", obj->id);
				return paneObjCellInput(pane, obj, pos, 2);
				}
			}
		}
		item = listGetNext(item);
	}
	

	//printf("paneSelectSlide delta %i %i\n", input->delta.x, input->delta.y);
	return ret;
}

static inline int paneSelectRelease (TPANE *pane, const int x, const int y, TTOUCHCOORD *pos)
{
	//printf("paneSelectReleasse %i %i\n", x, y);

	TPANEINPUT *input = &pane->input;

	// tmp
	if (input->slideMode == PANE_SLIDEMODE_ITEM){
		if (input->drag.state == PANE_SLIDE_SLIDE){
			input->drag.state = PANE_SLIDE_RELEASE;
			if (ccGetState(input->drag.label))
				ccDisable(input->drag.label);
		}
	}

	double t1 = com_getTime(pane->cc);
	input->state = PANE_INPUTSTATE_RELEASED;
	input->released.dt = t1 - input->start.time;
	input->released.id = pos->id;
	input->released.x = x;
	input->released.y = y;
	input->released.travelled.x = input->travelled.x;
	input->released.travelled.y = input->travelled.y;

	int dx = abs(input->travelled.x);
	if (dx < 1) dx = 1;
	int dy = abs(input->travelled.y);
	if (dy < 1) dy = 1;
	input->released.travelled.area =  dx * dy;


	//printf("id check: %i %i\n", input->start.id, input->released.id);	

	if (input->start.id == input->released.id){
		ccSendMessage(pane, PANE_MSG_BASE_SELECTED_RELEASE, ((int64_t)input->released.travelled.area<<32)|(x<<16|y), input->released.dt, pos);

		if (input->released.dt < 400 && input->released.travelled.area <= 64){
			//printf("input->drag.state %i: %i %i\n", input->drag.state, input->drag.id,  pos->id);
			if (input->released.id != input->drag.id)
				ccSendMessage(pane, PANE_MSG_BASE_SELECTED, x<<16|y, t1, pos);
		}
		
		
		pos->x += pane->metrics.x;
		pos->y += pane->metrics.y;
		TLISTITEM *item = pane->items;
		while(item){
			TPANEOBJ *obj = listGetStorage(item);
			if (obj){
			 	if (obj->type == PANE_OBJ_CELL && obj->cell.isEnabled){
				
					if (paneObjCellIsOverlapped(pane, obj, pos->x, pos->y)){
						//printf("paneInput: cell Release: %i\n", obj->id);
						return paneObjCellInput(pane, obj, pos, 3);
					}
					
				}
			}
			item = listGetNext(item);
		}
	}

	//printf("release delta %.0f %i,%i %i,%i, area:%i\n", input->released.dt, input->start.x, input->start.y, dx, dy, input->released.travelled.area);
	if (input->released.dt < 400.0 && input->released.travelled.area <= 64)
		return 1;
	else
		return -1;	// don't forward event
}

static inline int paneSelectItemSelected (TPANE *pane, const int whichEvent, const int itemId, const int64_t itemIdVar, TTOUCHCOORD *pos)
{
	//pane->isHovered = 0;
	ccSendMessage(pane, whichEvent, itemId, itemIdVar, pos);
	com_renderSignalUpdate(pane->cc);
	return 1;
}

void paneDragSelectPress (TPANE *pane, TLABEL *lbl, const int itemId, const int itemType, const int x, const int y, TTOUCHCOORD *pos)
{
	TPANEINPUT *input = &pane->input;
	//printf("@@@ paneDragSelectPress %i %i %i,%i %i,%i\n", itemId, itemType, x, y, input->start.x, input->start.y);

	char *str = labelStringGet(lbl, itemId);
	if (str){
		TMETRICS metrics;
		labelStringGetMetrics(lbl, itemId, &metrics.x, &metrics.y, &metrics.width, &metrics.height);
		ccSetMetrics(input->drag.label, -1, -1, metrics.width, metrics.height);
		input->drag.id = pos->id;
		input->drag.dx = x;
		input->drag.dy = y;
		input->drag.heldId = itemId;
		input->drag.heldType = itemType; //(labelItemDataGet(lbl,itemId)&0xF0000)>>16;

		//printf("held %i '%s'\n", (int)input->drag.heldType, str);

		labelStringRenderFlagsSet(input->drag.label, input->drag.itemId, LABEL_ALLIGN_MIDDLE);
		labelStringSet(input->drag.label, input->drag.itemId, str);
		ccSetUserDataInt(input->drag.label, itemId);
		ccInputDisable(input->drag.label);

		input->drag.state = PANE_SLIDE_PRESS;

		if (pane->input.slideMode == PANE_SLIDEMODE_ITEM){
			ccSendMessage(pane, PANE_MSG_SLIDE_HELD, itemId, labelItemDataGet(lbl,itemId), pos);
			paneSendPageSelectedHover(pane, PANE_SLIDE_PRESS, (uint64_t)itemId<<32|input->drag.heldId, lbl);
		}

		//if (!ccGetState(input->drag.label))
		//	ccEnable(input->drag.label);
		my_free(str);
	}
}

void paneDragSelectSlide (TPANE *pane, TLABEL *lbl, const int itemId, const int itemType, const int x, const int y)
{
	TPANEINPUT *input = &pane->input;
	//printf("### paneDragSelectSlide %i %i %i,%i %i,%i\n", itemId, itemType, x, y, input->start.x, input->start.y);

	if (input->drag.state == PANE_SLIDE_PRESS){
		int dx = abs(input->travelled.x);
		if (dx < 1) dx = 1;
		int dy = abs(input->travelled.y);
		if (dy < 1) dy = 1;
		const int area = dx * dy;

		//printf("paneDragSelectSlide area %i\n", area);
		if (area >= PANE_GRAB_AREA){
			input->drag.state = PANE_SLIDE_SLIDE;

			if (!ccGetState(input->drag.label))
				ccEnable(input->drag.label);
		}

		//ccSendMessage(pane, PANE_MSG_SLIDE_HOVER, itemId<<32|input->drag.heldId, labelItemDataGet(lbl,itemId), pos);
	}
	paneSendPageSelectedHover(pane, PANE_SLIDE_SLIDE, (uint64_t)itemId<<32|input->drag.heldId, lbl);
}

void paneDragSelectRelease (TPANE *pane, TLABEL *lbl, const int itemId, const int itemType, const int x, const int y)
{
	TPANEINPUT *input = &pane->input;
	//printf("@@@ paneDragSelectRelease %i %i %i,%i %i,%i\n", itemId, itemType, x, y, input->start.x, input->start.y);

	if (input->drag.state == PANE_SLIDE_PRESS || input->drag.state == PANE_SLIDE_SLIDE){
		//input->drag.state = PANE_SLIDE_RELEASE;

		if (ccGetState(input->drag.label))
			ccDisable(input->drag.label);
	}
	input->drag.state = PANE_SLIDE_RELEASE;
	//ccSendMessage(pane, PANE_MSG_SLIDE_HOVER, itemId<<32|input->drag.heldId, labelItemDataGet(lbl,itemId), pos);
	paneSendPageSelectedHover(pane, PANE_SLIDE_RELEASE, (uint64_t)itemId<<32|input->drag.heldId, lbl);
}

static inline int64_t cclbl_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_ENABLED || msg == CC_MSG_DISABLED || msg == CC_MSG_HOVER) return 1;

	TLABEL *lbl = (TLABEL*)object;
	TPANE *pane = ccGetUserData(lbl);

	//if (msg != CC_MSG_RENDER && msg != LABEL_MSG_DELETE)
		//printf("cclbl_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", lbl->id, lbl->type, msg, (int)data1, (int)data2, dataPtr);

	switch (msg){
	case LABEL_MSG_BASE_SELECTED_PRESS:
		return paneSelectPress(pane, (data1>>16)&0xFFFF, data1&0xFFFF, (TTOUCHCOORD*)dataPtr);

	case LABEL_MSG_BASE_SELECTED_SLIDE:
		return paneSelectSlide(pane, (data1>>16)&0xFFFF, data1&0xFFFF, (TTOUCHCOORD*)dataPtr);

	case LABEL_MSG_BASE_SELECTED_RELEASE:
		//printf("pane_cb: LABEL_MSG_BASE_SELECTED_RELEASE\n");
		return paneSelectRelease(pane, (data1>>16)&0xFFFF, data1&0xFFFF, (TTOUCHCOORD*)dataPtr);

	case LABEL_MSG_TEXT_SELECTED_PRESS:
		//printf("LABEL_MSG_TEXT_SELECTED_PRESS\n");
		paneDragSelectPress(pane, lbl, data2, labelItemGetType(lbl,data2), (data1>>16)&0xFFFF, data1&0xFFFF, (TTOUCHCOORD*)dataPtr);
		break;
	case LABEL_MSG_IMAGE_SELECTED_PRESS:{
		//printf("LABEL_MSG_IMAGE_SELECTED_PRESS\n");
		TPANEINPUT *input = &pane->input;
		TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		input->start.id = pos->id;
		input->drag.id = pos->id;
		break;
	}
	case LABEL_MSG_TEXT_SELECTED_SLIDE:
		//printf("LABEL_MSG_TEXT_SELECTED_SLIDE\n");
		paneDragSelectSlide(pane, lbl, data2, labelItemGetType(lbl,data2), (data1>>16)&0xFFFF, data1&0xFFFF);
		break;

	case LABEL_MSG_IMAGE_SELECTED_SLIDE:{
		//printf("LABEL_MSG_IMAGE_SELECTED_SLIDE\n");
		/*TPANEINPUT *input = &pane->input;
		TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		input->drag.id = pos->id;*/
		break;
	}
	case LABEL_MSG_TEXT_SELECTED_RELEASE:
		//TPANEINPUT *input = &pane->input;
		//printf("LABEL_MSG_TEXT_SELECTED_RELEASE\n");
		paneDragSelectRelease(pane, lbl, data2, labelItemGetType(lbl,data2), (data1>>16)&0xFFFF, data1&0xFFFF);

		return paneSelectItemSelected(pane, PANE_MSG_TEXT_SELECTED, data2, labelItemDataGet(lbl,data2), dataPtr);

	case LABEL_MSG_IMAGE_SELECTED_RELEASE:
		//printf("LABEL_MSG_IMAGE_SELECTED_RELEASE\n");
		return paneSelectItemSelected(pane, PANE_MSG_IMAGE_SELECTED, data2, labelItemDataGet(lbl,data2), dataPtr);
	}
	return 1;
}

#if 0
static inline int64_t cclbl_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg == CC_MSG_RENDER /*|| msg == CC_MSG_INPUT*/ || msg == CC_MSG_SETPOSITION) return 1;
	if (msg == CC_MSG_ENABLED || msg == CC_MSG_DISABLED) return 1;

	TLABEL *lbl = (TLABEL*)object;
	TPANE *pane = ccGetUserData(lbl);
	TPANEINPUT *input = &pane->input;

	//if (msg != CC_MSG_RENDER)
	//	printf("ccpane_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", lbl->id, lbl->type, msg, (int)data1, (int)data2, dataPtr);

	int paneMsg;

	switch(msg){
	case LABEL_MSG_TEXT_SELECTED_PRESS:
	case LABEL_MSG_IMAGE_SELECTED_PRESS:{
		TTOUCHCOORD *pos = dataPtr;

		//printf("pane press %i %i, %i %i %i\n", input->state, pos->pen, input->start.x, input->start.y, input->slideEnabled);

		if (input->state == 3 && !pos->pen){
			if (input->slideEnabled) input->state = 1;

			input->start.x = (pane->offset.x - (pos->x*input->acceleration.x));
			input->start.y = (pane->offset.y - (pos->y*input->acceleration.y));
			input->start.time = com_getTime(pane->cc);
			input->delta.x = pos->x;
			input->delta.y = pos->y;

			if (input->drag.state == PANE_SLIDE_DISABLED)
				break;

			const int srcItemId = data2;
			if (labelItemGetType(lbl, srcItemId) == LABEL_OBJTYPE_TEXT){
				//printf("PANE_SLIDE_PRESS %I64d \n", data2);

				char *str = labelStringGet(lbl, srcItemId);
				if (str){
					TMETRICS metrics;
					labelStringGetMetrics(lbl, srcItemId, &metrics.x, &metrics.y, &metrics.width, &metrics.height);
					ccSetMetrics(input->drag.label, -1, -1, metrics.width+30, metrics.height+6);
					input->drag.dx = abs(pos->x - metrics.x);
					input->drag.dy = abs(pos->y - metrics.y);
					input->drag.heldId = srcItemId;
					input->drag.heldType = (labelItemDataGet(lbl,input->drag.heldId)&0xF0000)>>16;

					//printf("held %i\n", (int)input->drag.heldType);

					labelStringRenderFlagsSet(input->drag.label, pane->input.drag.itemId, LABEL_ALLIGN_MIDDLE);
					labelStringSet(input->drag.label, pane->input.drag.itemId, str);
					ccSetUserDataInt(input->drag.label, srcItemId);
					ccInputDisable(input->drag.label);

					my_free(str);
				}
			}
		}
		break;
	}
	case LABEL_MSG_TEXT_SELECTED_SLIDE:
	case LABEL_MSG_IMAGE_SELECTED_SLIDE:
	  {
	  //TTOUCHCOORD *pos = dataPtr;
	  //printf("pane slide %i %i, %i %i %i\n", input->state, pos->pen, input->start.x, input->start.y, input->slideEnabled);
	  }

	  if (input->slideEnabled == 1){
		TTOUCHCOORD *pos = dataPtr;

		if (input->state == 1 && !pos->pen){
			pane->offset.x = (input->start.x + (pos->x*input->acceleration.x));
			if (pane->offset.x < -pane->pos.x) pane->offset.x = -pane->pos.x;

			pane->offset.y = (input->start.y + (pos->y*input->acceleration.y));
			if (pane->offset.y < -pane->pos.y) pane->offset.y = -pane->pos.y;

			paneSetInvalidated(pane);
			ccSendMessage(pane, PANE_MSG_SLIDE, pane->offset.x, pane->offset.y, pos);

			if (input->drag.state == PANE_SLIDE_DISABLED)
				break;

			int dx = abs(input->delta.x - pos->x);
			int dy = abs(input->delta.y - pos->y);
			if (dy > 10 && dx < 25){
				input->state = 3;
				input->drag.state = PANE_SLIDE_PRESS;

			}else if (dx >= 25){
				if (input->drag.state == PANE_SLIDE_PRESS || input->drag.state == PANE_SLIDE_SLIDE){
					input->drag.state = PANE_SLIDE_RELEASE;
					ccSendMessage(pane, PANE_MSG_SLIDE_RELEASE, data2<<32|input->drag.heldId, labelItemDataGet(lbl,input->drag.heldId), pos);
					//paneSendPageSelectedHover(pane, PANE_SLIDE_RELEASE, data2<<32|input->drag.heldId, lbl);
					if (ccGetState(input->drag.label))
						ccDisable(input->drag.label);
				}
				break;
			}

			if (input->drag.state == PANE_SLIDE_PRESS){
				input->drag.state = PANE_SLIDE_SLIDE;
				//input->drag.heldId = data2;
				//printf("PANE_SLIDE_SLIDE %I64d %i\n", data2, input->drag.heldId);

				ccSendMessage(pane, PANE_MSG_SLIDE_HELD, data2, labelItemDataGet(lbl,data2), pos);
				paneSendPageSelectedHover(pane, PANE_SLIDE_PRESS, data2<<32|input->drag.heldId, lbl);

				if (!ccGetState(input->drag.label))
					ccEnable(input->drag.label);

			}else if (input->drag.state == PANE_SLIDE_SLIDE){
				ccSendMessage(pane, PANE_MSG_SLIDE_HOVER, data2<<32|input->drag.heldId, labelItemDataGet(lbl,data2), pos);
				paneSendPageSelectedHover(pane, PANE_SLIDE_SLIDE, data2<<32|input->drag.heldId, lbl);
			}
		}else if (input->drag.state == PANE_SLIDE_SLIDE){
			int dx = abs(input->delta.x - pos->x);
			int dy = abs(input->delta.y - pos->y);
			if (dy > 10 && dx < 25) input->state = 3;

			ccSendMessage(pane, PANE_MSG_SLIDE_HOVER, data2<<32|input->drag.heldId, labelItemDataGet(lbl,data2), pos);
		}
	  }
	  break;

	case LABEL_MSG_TEXT_SELECTED_RELEASE:
		paneMsg = PANE_MSG_TEXT_SELECTED;
		goto skipMe;
	case LABEL_MSG_IMAGE_SELECTED_RELEASE:{
		paneMsg = PANE_MSG_IMAGE_SELECTED;
skipMe:
		//printf("pane release %i %i %i\n", input->state, input->start.x, input->start.y);

		if (input->drag.state != PANE_SLIDE_DISABLED){
			if (input->drag.state == PANE_SLIDE_SLIDE){
				TTOUCHCOORD *pos = dataPtr;
				ccSendMessage(pane, PANE_MSG_SLIDE_RELEASE, data2<<32|input->drag.heldId, labelItemDataGet(lbl,input->drag.heldId), pos);
				paneSendPageSelectedHover(pane, PANE_SLIDE_RELEASE, data2<<32|input->drag.heldId, lbl);

				if (ccGetState(input->drag.label))
					ccDisable(input->drag.label);
			}
			input->drag.state = PANE_SLIDE_RELEASE;
		}

		TTOUCHCOORD *pos = dataPtr;
		input->delta.x = abs(pos->x - input->delta.x);
		input->delta.y = abs(pos->y - input->delta.y);

		//paneDoSelected(pane, PANE_MSG_TEXT_SELECTED, input->delta.x, input->delta.y, data2, labelItemDataGet(lbl, data2), pos);
		paneDoSelected(pane, paneMsg, input->delta.x, input->delta.y, data2, labelItemDataGet(lbl, data2), pos);

		input->state = 3;
		input->start.x = 0;
		input->start.y = 0;
		input->delta.x = 0;
		input->delta.y = 0;

		break;
	}
	case LABEL_MSG_BASE_SELECTED_PRESS:{
	  //if (input->slideEnabled){
		TTOUCHCOORD *pos = dataPtr;
		//int penState = data2;

		//if (input->state == 3 && !penState){			// pen down
			input->state = 1;
			input->start.x = (pane->offset.x - (pos->x*input->acceleration.x));
			input->start.y = (pane->offset.y - (pos->y*input->acceleration.y));
			input->start.time = com_getTime(pane->cc);
			input->delta.x = pos->x;
			input->delta.y = pos->y;
			ccSendMessage(pane, PANE_MSG_BASE_SELECTED_PRESS, data1, data2, dataPtr);
		//}
		break;
	}
	case LABEL_MSG_BASE_SELECTED_SLIDE:	{
			TTOUCHCOORD *pos = dataPtr;
		//if (input->state == 1 && penState == 1){	// slide/drag
			pane->offset.x = (input->start.x + (pos->x*input->acceleration.x));
			if (pane->offset.x < -pane->pos.x) pane->offset.x = -pane->pos.x;
			pane->offset.y = (input->start.y + (pos->y*input->acceleration.y));
			if (pane->offset.y < -pane->pos.y) pane->offset.y = -pane->pos.y;

			paneSetInvalidated(pane);
			ccSendMessage(pane, PANE_MSG_SLIDE, pane->offset.x, pane->offset.y, pos);
		break;
	}
	case LABEL_MSG_BASE_SELECTED_RELEASE:{
		TTOUCHCOORD *pos = dataPtr;
		//if (penState == 3){						// pen up
			if (input->drag.state != PANE_SLIDE_DISABLED){
				if (input->drag.state == PANE_SLIDE_PRESS || input->drag.state == PANE_SLIDE_SLIDE){
					ccSendMessage(pane, PANE_MSG_SLIDE_RELEASE, input->drag.heldId, labelItemDataGet(lbl, input->drag.heldId), pos);
					paneSendPageSelectedHover(pane, PANE_SLIDE_RELEASE, input->drag.heldId, lbl);
				}
		  		input->drag.state = PANE_SLIDE_NONE;
		  		if (ccGetState(input->drag.label))
					ccDisable(input->drag.label);
			}

			input->delta.x = abs(pos->x - input->delta.x);
			input->delta.y = abs(pos->y - input->delta.y);

			input->state = 3;
			input->start.x = 0;
			input->start.y = 0;
			input->delta.x = 0;
			input->delta.y = 0;

			ccSendMessage(pane, PANE_MSG_BASE_SELECTED_RELEASE, data1, data2, dataPtr);
		//}
	 // }//else{	// input->slideEnabled)
		/*int penState = data2;
		if (input->state == 3 && !penState){
	  		input->state = 1;
			ccSendMessage(pane, PANE_MSG_BASE_SELECTED_PRESS, data1, data2, dataPtr);
		}else if (penState == 3){
			input->state = 3;
			ccSendMessage(pane, PANE_MSG_BASE_SELECTED_RELEASE, data1, data2, dataPtr);
		}*/

	 // }
	  break;
	}
	case CC_MSG_HOVER:{
		if (input->drag.state != PANE_SLIDE_DISABLED){
			int hoverState = data2&0xF;
			if (!hoverState)
				ccInputEnable(input->drag.label);
			else
				ccInputDisable(input->drag.label);
		}

		input->state = 3;
		input->start.x = 0;
		input->start.y = 0;
		break;
	  }
	};


	return 1;
}
#endif

int paneNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t pane_cb, int *id, const int width, const int height)
{
	TPANE *pane = (TPANE*)object;

	pane->pageOwner = pageOwner;
	if (id) *id = pane->id;
	pane->type = type;

	pane->cb.msg = pane_cb;
	pane->cb.render = paneRender;
	pane->cb.create = paneNew;
	pane->cb.free = paneDelete;
	pane->cb.enable = paneEnable;
	pane->cb.disable = paneDisable;
	pane->cb.input = paneInput;
	pane->cb.setPosition = paneSetPosition;
	pane->cb.setMetrics = paneSetMetrics;

	pane->metrics.width = width;
	pane->metrics.height = height;

	pane->base = ccCreateEx(pane->cc, pageOwner, CC_LABEL, cclbl_cb, NULL, width, height, pane);
	labelRenderFlagsSet(pane->base, LABEL_RENDER_CLIP|/*LABEL_RENDER_HOVER_OBJ|*/LABEL_RENDER_IMAGE|LABEL_RENDER_TEXT/*|LABEL_RENDER_BORDER_POST*/);
	pane->base->isChild = 1;
	pane->canDrag = 1;

	pane->flags.text.renderFlags = LABEL_ALLIGN_LEFT;
	pane->flags.readAhead.enabled = 0;
	pane->flags.readAhead.number = 6;

	pane->offset.x = 0;
	pane->offset.y = 0;
	pane->pos.x = 0;
	pane->pos.y = 0;
	pane->offset.x = 0;
	pane->offset.y = 0;
	pane->previous.x = 0;
	pane->previous.y = 0;

	pane->layout.metrics.tItemWidth = 0;

	pane->input.state = PANE_INPUTSTATE_RELEASED;
	pane->input.slideMode = PANE_SLIDEMODE_PANE;
	//pane->input.slideMode = PANE_SLIDEMODE_ITEM;
	pane->input.acceleration.x = 1.0;
	pane->input.acceleration.y = 1.0;
	pane->input.slideEnabled = 1;

	pane->flags.text.singleLine = 1;
	pane->flags.text.wordWrap = 0;
	pane->layout.horiColumnSpace = 1;
	pane->layout.metrics.aveLineHeight = 0;
	pane->layout.metrics.vertLineHeight = 0;
	pane->layout.table.horiMin = 0;
	pane->layout.table.horiMax = 1000;
	pane->layout.table.vertMin = 0;
	pane->layout.table.vertMax = 8;
	pane->layout.table.pixelsPerUnit = 1.0f;

	ccDisable(ccCreateEx(pane->cc, pageOwner, CC_LABEL, cclblDrag_cb, NULL, 64, 32, pane));

	pane->layout.type = PANE_LAYOUT_HORI;
	pane->layout.direction = PANE_DIRECTION_STOP;
	pane->layout.isInvalidated = 1;				// set when item list is adjusted. indicates item list objects may require repositioning
	pane->items = NULL;
	pane->idSrc = 100;

	_rect_t clip = {0, 0, width-1, height-1};
	labelRenderSetClipRegion(pane->base, &clip);

	return 1;
}
