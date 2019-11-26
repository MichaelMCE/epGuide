
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



static inline TTV_RENDER *tv_renderPreCalc (TTV *tv, TTV_ITEM *item, TLPOINTEX *pos, int *height);




void tvTreeFree (TTV *tv)
{
	if (ccLock(tv)){
		treeFree(tv->tree);
		tv->tree = NULL;
		tv->rootId = 0;
		ccUnlock(tv);
	}
}

int tvTreeCreate (TTV *tv, const char *name, const int rootId)
{
	if (ccLock(tv)){
		if (tv->tree) treeFree(tv->tree);
		tv->tree = treeCreate(name, rootId);
		tv->rootId = rootId;
		ccUnlock(tv);
	}

	return (tv->tree != NULL);
}

void tvTreeSetStorage (TTV *tv, const int id, void *data)
{
	if (ccLock(tv)){
		treeSetStorage(tv->tree, id, data);
		ccUnlock(tv);
	}
}

void *tvTreeGetStorage (TTV *tv, const int id)
{
	void *data = NULL;
	if (ccLock(tv)){
		data = treeGetStorage(tv->tree, id);
		ccUnlock(tv);
	}
	return data;
}

int tvTreeCountItems (TTV *tv, const int id)
{
	return treeCountItems(tv->tree, id);
}

void tvTreeFreeItem2 (TTV_ITEM *item)
{
	if (item){
		//my_free(item->name);
		if (item->type == TREE_TYPE_BRANCH)
			if (item->children) my_free(item->children);
		my_free(item);
	}
}

void tvTreeFreeItem (TTV_ITEM *item)
{
	if (item){
		my_free(item->name);
		if (item->type == TREE_TYPE_BRANCH)
			my_free(item->children);
		my_free(item);
	}
}

TTV_ITEM_DESC *tvTreeItemGetDesc (TTV_ITEM *item)
{
	return item->storage;
}

static inline TTV_ITEM *tvTreeGetItem2 (TTV *tv, const int id, void *fromEntry)
{
	
	//printf("tvTreeGetItem2 %i\n", id);
	TTV_ITEM *item = NULL;

	//if (ccLock(tv)){
		TTREEENTRY *entry;
		if (fromEntry)
			entry = treeEntryFind(fromEntry, id);
		else
			entry = treeEntryFind(tv->tree->root, id);
		
		if (entry){
			item = my_malloc(sizeof(TTV_ITEM));
			if (item){
				item->entry = entry;


				//item->name = my_strdup(entry->name);
				item->storage = entry->storage;
				item->id = entry->id;
				//item->parentId = entry->parentId;
				item->type = entry->type;
		
				TTV_ITEM_DESC *desc = item->storage;

				if (item->type == TREE_TYPE_BRANCH && desc && desc->expander.expanderState == TV_EXPANDER_OPEN){

					int ct = treeEntryGetSubItemCount(entry);
					if (ct <= 0) ct = 0;
					
					item->children = my_malloc((ct+1) * sizeof(int));
					if (item->children){
						int i = 0;
				 
						TLISTITEM *subitem = entry->head;
						while(subitem){
							item->children[i++] = treeListGetSubEntry(subitem)->id;
							subitem = subitem->next;
						}
						item->children[ct] = 0;
					}
				}else{
					item->children = NULL;
				}
			}
		}
		//ccUnlock(tv);
	//}
	return item;
}


TTV_ITEM *tvTreeGetItem (TTV *tv, const int id, void *fromEntry)
{
	
	//printf("tvTreeGetItem %i\n", id);
	
	TTV_ITEM *item = NULL;

	//if (ccLock(tv)){
		TTREEENTRY *entry;
		if (fromEntry)
			entry = treeEntryFind(fromEntry, id);
		else
			entry = treeEntryFind(tv->tree->root, id);

		if (entry){
			item = my_malloc(sizeof(TTV_ITEM));
			if (item){
				item->entry = entry;
				item->name = my_strdup(entry->name);
				item->storage = entry->storage;
				item->id = entry->id;
				item->parentId = entry->parentId;
				item->type = entry->type;
		
				if (item->type == TREE_TYPE_BRANCH){

					int ct = treeEntryGetSubItemCount(entry);
					if (ct <= 0) ct = 0;
					
					item->children = my_calloc(ct+1, sizeof(int));
					if (item->children){
						int i = 0;
				 
						TLISTITEM *subitem = entry->head;
						while(subitem){
							item->children[i++] = treeListGetSubEntry(subitem)->id;
							subitem = subitem->next;
						}
					}
				}
			}
		}
		//ccUnlock(tv);
	//}
	return item;
}

int tvTreeGetEx (TTV *tv, const int id, void **data, int *type, int **items)
{
	TTREEENTRY *entry = treeEntryFind(tv->tree->root, id);
	if (entry){
		*data = entry->storage;
		*type = entry->type;
		
		if (treeEntryIsBranch(entry)){
			if (*items)
				*items = my_realloc(*items, (listCount(entry->head)+1) * sizeof(int));
			else
				*items = my_malloc((listCount(entry->head)+1) * sizeof(int));
				
			if (*items){
				int i = 0;
				TLISTITEM *item = entry->head;
				while(item){
					TTREEENTRY *subentry = listGetStorage(item);
					*items[i++] = subentry->id;
					*items[i] = 0;
					item = item->next;
				}
			}
		}
		return entry->parentId;
	}
	return 0;
}

static inline int tvTreeAdd (TTV *tv, const int nodeId, char *name, const int id, TTV_ITEM_DESC *desc, const int type)
{
	int ret = 0;
	if (ccLock(tv)){
		TTREEENTRY *entry = treeAddItem(tv->tree, nodeId, name, id, type);
		if (entry){
			//printf("tvTreeAdd %i %p %p, '%s'\n", id, entry, desc, name);
			if (desc)
				treeEntrySetStorage(entry, desc);
			ret = 1;
		}
		ccUnlock(tv);
	}
	
	return ret;
}

int tvTreeAddNode (TTV *tv, const int nodeId, char *name, const int id, TTV_ITEM_DESC *desc)
{
	return tvTreeAdd(tv, nodeId, name, id, desc, TREE_TYPE_BRANCH);
}

int tvTreeAddItem (TTV *tv, const int nodeId, char *name, const int id, TTV_ITEM_DESC *desc)
{
	return tvTreeAdd(tv, nodeId, name, id, desc, TREE_TYPE_LEAF);
}

void tvTreeMove (TTV *tv, const int fromId, const int toId)
{
	if (ccLock(tv)){
		treeEntryMove(tv->tree, fromId, toId);
		ccUnlock(tv);
	}
}

void treeRenameItem (TTV *tv, const int id, const char *newName)
{
	if (ccLock(tv)){
		TTREEENTRY *entry = treeEntryFind(tv->tree->root, id);
  		if (entry){
  			my_free(entry->name);
  			entry->name = my_strdup(newName);
  		}
		ccUnlock(tv);
	}
}


static inline void tvItemDescFree (TTV_ITEM_DESC *desc)
{

	if (desc->objType == TV_TYPE_CCOBJECT){
			
	}else if (desc->objType == TV_TYPE_LABEL){
		//if (desc->label.text)
		//	my_free(desc->label.text);
		//desc->label.text = NULL;

	}else if (desc->objType == TV_TYPE_IMAGE){
		//if (desc->varImage.img)
		//	lDeleteFrame(desc->varImage.img);
		
		//if (desc->varImage.opaque)
		//	my_free(desc->varImage.opaque);

	}else if (desc->objType == TV_TYPE_INT32){
		//desc->varInt32 = 0;
		
	}else if (desc->objType == TV_TYPE_FLOAT){
		//desc->varFloat = 0.0;
	}

	if (desc->image){
		for (int i = 0; i < desc->imageTotal; i++){
			//if (desc->image[i].img)
				//lDeleteFrame(desc->image[i].img);
		
			if (desc->image[i].opaque)
				my_free(desc->image[i].opaque);
			//item->data.image[i].opaque = NULL;
		}
		my_free(desc->image);
	}
	
	if (desc->opaque)
		my_free(desc->opaque);
	my_free(desc);
}

static inline void tvTreeDestoryItem (TTREEENTRY *entry)
{
	if (!entry) return;
	
	TLISTITEM *subitem = entry->head;
	while(subitem){
		tvTreeDestoryItem(subitem->storage);
		subitem = subitem->next;
	}
	
	if (entry->storage){
		tvItemDescFree(entry->storage);
		entry->storage = NULL;
	}
}

void tvTreeDelete (TTV *tv, const int id)
{
	if (ccLock(tv)){
		TTREEENTRY *entry = treeEntryFind(tv->tree->root, id);
		if (tv->rootId == id){
			if (entry)
				tvTreeDestoryItem(entry);
			tvTreeFree(tv);
		}else{
			if (entry){
				tvTreeDestoryItem(entry);
				treeDestoryItem(tv->tree, id);
			}
		}
		ccUnlock(tv);
	}
}

void tvTreeDeleteItems (TTV *tv, const int id)
{
	if (ccLock(tv)){
		TTREEENTRY *entry = treeEntryFind(tv->tree->root, id);	
		if (entry){
			if (treeEntryIsBranch(entry)){
				TLISTITEM *subitem = entry->head;
				while(subitem){
					tvTreeDestoryItem(subitem->storage);
					subitem = subitem->next;
				}
				treeEntryDestroyItems(entry);
			}
		}
		ccUnlock(tv);
	}
}

int tvGenImageCbId (TTV *tv)
{
	int id = 0;
	//if (ccLock(tv)){
		id = ++tv->cbIdSource;
	//	ccUnlock(tv);
	//}
	return id;
}

static inline int64_t tv_scrollbar_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	TCCOBJECT *obj = (TCCOBJECT*)object;
	if (msg == CC_MSG_RENDER) return 1;
	
	//printf("tv_scrollbar_cb. id:%i, objType:%i, msg:%i, data1:%i, data2:%i\n", obj->id, obj->type, msg, data1, data2);

	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	TTV *tv = (TTV*)ccGetUserData(scrollbar);

	switch (msg){
  	  case SCROLLBAR_MSG_VALCHANGED:
  	  	if (obj->type == CC_SCROLLBAR_VERTICAL)
  	  		tv->sbVertPosition = scrollbarGetFirstItem(scrollbar);
		break;

	  case CC_MSG_INPUT:
	  	ccSendMessage(tv, TV_MSG_SB_INPUT, data1, scrollbar->id, dataPtr);
		break;
  	}
  	
  	return 1;
}

#if 0
int tv_btn_cb (const TTOUCHCOORD *pos, TBUTTON *button, const int btn_id, const int flags, void *ptr)
{
	//printf("treeview_btn_cb: %i: %i,%i %i\n", btn_id, pos->x, pos->y, TV_BTN_PASTE);

	TTV *tv = (TTV*)buttonGetUserData(button);
	if (!tv) return 0;
	
	switch (btn_id){
	  case TV_BTN_PASTE:{
		/*if (!flags){
			if (tv->drag.post.item)
				printf("'%s'\n", tv->drag.post.item->data.label.text);
		}*/
		break;
	  }
	}
	return 0;
}
#endif

TTV_ITEM_DESC_IMAGE *tvTreeItemImageGet (TTV *tv, TTV_ITEM *item, const int imgNumber)
{
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	
	if (desc->image)
		return &desc->image[imgNumber-1];
	else
		return NULL;
}

static inline TTV_RENDER_ITEM *addPost (TTV *tv, const int x1, const int y1, const int x2, const int y2, const int action, const int id, void *opaque)
{
	TTV_RENDER_ITEM *post = &tv->postRender[tv->tPostItems++];
	post->pos.x1 = x1;
	post->pos.y1 = y1;
	post->pos.x2 = x2;
	post->pos.y2 = y2;
	post->id = id;
	post->action = action;
	post->opaque = opaque;
	return post;
}

static inline TTV_RENDER_ITEM * tvRenderImage (TTV *tv, TFRAME *frame, TTV_ITEM_DESC_IMAGE *image, const int id, TLPRINTR *rt)
{
	if (image->drawState == TV_DRAWIMAGE_DONTRENDER)
		return NULL;
		

	int width, height;
	void *am = ccGetImageManager(tv->cc, CC_IMAGEMANAGER_ART);
	artManagerImageGetMetrics(am, image->artId, &width, &height);
	width *= image->scale;
	height *= image->scale;


	const int x = rt->sx + image->offsetX;
	const int y = rt->sy + image->offsetY;

	int h = height-1;
	if (y + h > rt->by2) h = (rt->by2 - y);
	int y1 = 0;
	if (y < rt->by1) y1 = (rt->by1 - y);
	if (y + y1 > (y + y1 + h) - y1) return NULL;

	TTV_RENDER_ITEM *post = addPost(tv, x, y + y1, x + width-1, (y + y1 + h) - y1, TV_ACTION_IMAGE_SELECT, id, image);
	
	if (tv->renderFlags&TV_RENDER_IMAGES){
		TFRAME *img = artManagerImageAcquireScaled(am, image->artId, image->scale);
		if (img){
			//printf("tvRenderImage %X, %f, %i %i, %i %i %i\n", image->artId, image->scale, width, height, img->bpp, img->width, img->height);
			
			if (y1 || h != img->height-1 || img->bpp != LFRM_BPP_32)
				com_copyArea(img, frame, post->pos.x1, post->pos.y1, 0, y1, img->width-1, h);
			else
				fastFrameCopy(img, frame, post->pos.x1, post->pos.y1);
			
			artManagerImageRelease(am, image->artId);
		//}else{
			//printf("tvRenderImage failed %X, %f, %i %i\n", image->artId, image->scale, width, height);
		}
	}
	return post;
}

static inline void ldrawRectangleFilledConstrained (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int colour, const int miny, const int maxy)
{
	int startY = MAX(y1, miny);
	int endY = MIN(y2, maxy);
	for (int r = startY; r <= endY; r++)
		lDrawLine(frame, x1, r, x2, r, colour);
}

static inline TTV_RENDER_ITEM *tvRenderLabel (TTV *tv, TFRAME *frame, TTV_ITEM *item, TLPRINTR *rt)
{
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	
	char *font = desc->label.font;
	int flags = desc->label.flags;
	char *str = item->name;
	if (!str) return NULL;//str = "----";

	int w = (rt->bx2 - rt->sx)+1;


	uint32_t shash = com_getHash(str) ^ (tv->renderFlags<<24 ^ frame->hw->render->foreGround);
	TFRAME *img = strcFindString(tv->cc->strc, shash);
	if (!img){
		TMETRICS metrics = {1, 5, 0, 0};
		img = com_newStringEx(frame->hw, &metrics, LFRM_BPP_32A, flags, font, str, w, NSEX_LEFT);
		strcAddString(tv->cc->strc, img, shash);
	}

	if (img){
		rt->sy += desc->label.offsetY;
		rt->sx += desc->label.offsetX;
		
		int h = img->height-1;
		if (rt->sy + h > rt->by2) h = (rt->by2 - rt->sy);
		int y1 = 0;
		if (rt->sy < rt->by1) y1 = (rt->by1 - rt->sy);
		if (rt->sy + y1 > (rt->sy + y1 + h) - y1) return NULL;

		TTV_RENDER_ITEM *post = addPost(tv, rt->sx, rt->sy + y1, rt->sx + img->width-1, (rt->sy + y1 + h) - y1, TV_ACTION_TRACK_SELECT, item->id, NULL);
		if (tv->renderFlags&TV_RENDER_LABELS){
			if (tv->renderFlags&TV_RENDER_LABELROWHL){
				if (item->id&1)
					ldrawRectangleFilledConstrained(frame, post->pos.x1, post->pos.y1+1, post->pos.x1+img->width-1, post->pos.y1+h-y1-1, 25<<24|0x222222, 0, frame->height-1);
				else
					ldrawRectangleFilledConstrained(frame, post->pos.x1, post->pos.y1+1, post->pos.x1+img->width-1, post->pos.y1+h-y1-1, 25<<24|0x121262, 0, frame->height-1);
			}
			com_copyArea(img, frame, post->pos.x1, post->pos.y1, 0, y1, img->width-1, h);
		}

		rt->sy += 3;

		if (post->pos.y2 - post->pos.y1 > 2)
			return post;
		else
			if (--tv->tPostItems < 0) tv->tPostItems = 0;
	}
	return NULL;
}

static inline void drawExpanderOpen (TFRAME *frame, const int x, const int y, const int colour, const int miny, const int maxy)
{
	//const int crossSize = 8;
	const int crossSize = EXPANDER_SIZE;
	
	const int crossSizeHalf = crossSize>>1;
	const int x1 = x;
	const int y1 = y + 5; 
	const int x2 = x1 + crossSize;
	const int y2 = y1 + crossSize;
	
	 // Hori stroke
	if (y1+crossSizeHalf >= miny && y1+crossSizeHalf <= maxy){
		for (int x = x1; x <= x2; x++)
			lSetPixel_NB(frame, x, y1+crossSizeHalf, colour);
	}
	
	// Vert stroke
	for (int y = y1; y <= y2; y++){
		if (y >= miny && y <= maxy)
			lSetPixel_NB(frame, x1+crossSizeHalf, y, colour);
	}
}

static inline void drawExpanderClosed (TFRAME *frame, const int x, const int y, const int colour, const int miny, const int maxy)
{
	//const int crossSize = 8;
	const int crossSize = EXPANDER_SIZE;
	
	const int crossSizeHalf = crossSize>>1;
	const int x1 = x;
	const int y1 = y + 5; 
	const int x2 = x1 + crossSize;
	
	 // Hori stroke
	if (y1+crossSizeHalf >= miny && y1+crossSizeHalf <= maxy){
		for (int x = x1; x <= x2; x++)
			lSetPixel_NB(frame, x, y1+crossSizeHalf, colour);
	}
}

static inline TTV_RENDER_ITEM * tvRenderExpander (TTV *tv, TFRAME *frame, TTV_ITEM *item, TLPRINTR *rt, TTV_RENDER_ITEM *post, const int colour)
{
	//const int expsize = 9;
	//int x = post->pos.x1 - 12;
	
	const int expsize = EXPANDER_SIZE+1;
	int x = post->pos.x1 - expsize - 3;
	int y = rt->sy + EXPANDER_OFFSET_Y; //post->pos.y1;
	
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	if (desc->expander.expanderState == TV_EXPANDER_OPEN){
		if (tv->renderFlags&TV_RENDER_EXPANDER)
			drawExpanderClosed(frame, x, y, colour, rt->by1, rt->by2);

		y += 4;
		int y2 = y + expsize+1;
		if (y2 > rt->by2) y2 = rt->by2;
		if (y < rt->by1) y = rt->by1;
		post = addPost(tv, x-1, y, x+expsize, y2, TV_ACTION_EXPANDER_CLOSE, item->id, NULL);
		
	}else if (desc->expander.expanderState == TV_EXPANDER_CLOSED){
		if (tv->renderFlags&TV_RENDER_EXPANDER)
			drawExpanderOpen(frame, x, y, colour, rt->by1, rt->by2);
			
		y += 4;
		int y2 = y + expsize+1;
		if (y2 > rt->by2) y2 = rt->by2;
		if (y < rt->by1) y = rt->by1;
		post = addPost(tv, x-1, y, x+expsize, y2, TV_ACTION_EXPANDER_OPEN, item->id, NULL);
	}
	return post;
}

static inline void drawRectangleConstrained (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int colour, const int miny, const int maxy)
{
	if (y1 >= miny && y1 <= maxy){
		for (int x = x1; x <= x2; x++)			// top
			lSetPixel_NB(frame, x, y1, colour);
	}
	if (y2 >= miny && y2 <= maxy){
		for (int x = x1; x <= x2; x++)			// bottom
			lSetPixel_NB(frame, x, y2, colour);
	}
	for (int y = y1+1; y < y2; y++){			// left
		if (y >= miny && y <= maxy)
			lSetPixel_NB(frame, x1, y, colour);
	}
	for (int y = y1+1; y < y2; y++){			// right
		if (y >= miny && y <= maxy)
			lSetPixel_NB(frame, x2, y, colour);
	}
}

static inline int isChecked (TTV_ITEM *item)
{
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	return (desc->checkbox.checkState == TV_CHECKBOX_CHECKED);
}

static inline TTV_RENDER_ITEM * tvDrawCheckbox (TTV *tv, TFRAME *frame, TTV_ITEM *item, TLPRINTR *rt, TTV_RENDER_ITEM *post, const int colour)
{
	//const int cbsize = 10;
	//int x = post->pos.x1 - 15;
	
	const int cbsize = CHECKBOX_SIZE;
	int x = post->pos.x1 - cbsize - 5;
	int y = rt->sy + 4 + CHECKBOX_OFFSET_Y; //post->pos.y1;
	
	if (tv->renderFlags&TV_RENDER_CHECKBOX){
		drawRectangleConstrained(frame, x, y, x+cbsize, y+cbsize, colour, rt->by1, rt->by2);

		if (isChecked(item)){
			const int cbsizehalf = cbsize>>1;
			for (int i = 2; i < cbsizehalf; i++)
				drawRectangleConstrained(frame, x+i, y+i, x+(cbsize-i), y+(cbsize-i), colour, rt->by1, rt->by2);
		}
	}

	y -= 1;
	int y2 = y + cbsize+2;
	if (y2 > rt->by2) y2 = rt->by2;
	if (y < rt->by1) y = rt->by1;
	post = addPost(tv, x-1, y, x+cbsize+1, y2, TV_ACTION_CHECKBOX_TOGGLE, item->id, NULL);
	return post;
}

void *tvItemOpaqueGet (TTV_ITEM *item)
{
	return tvTreeItemGetDesc(item)->opaque;
}

static inline int tvRenderItem (TTV *tv, TFRAME *frame, TTV_ITEM *item, TLPRINTR *rt, const int yOffset)
{
	const int id = item->id;
	
	TTV_RENDER_ITEM *post = NULL;
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	if (!desc->enabled) return -1;

	TTV_ITEM_DESC_IMAGE *image = tvTreeItemImageGet(tv, item, 1);

	if (image && image->drawImage == TV_DRAWIMAGE_ENABLED){
		post = tvRenderImage(tv, frame, image, id, rt);
		if (post) rt->sx = post->pos.x2 + TEXT_INDENT_GUESS;
	}

	if (yOffset <= tv->metrics.y + tv->metrics.height){
		int colour = lGetForegroundColour(frame->hw);
		
		if (desc->objType == TV_TYPE_LABEL){
			post = tvRenderLabel(tv, frame, item, rt);
			if (post){
				if (item->type == TREE_TYPE_BRANCH)
					post->action = TV_ACTION_EXPANDER_TOGGLE;
					
				if (desc->checkbox.drawCheckbox)
					post = tvDrawCheckbox(tv, frame, item, rt, post, colour);
			}
		//}else if (item->data.objType == TV_TYPE_IMAGE){
		}

		if (post && desc->expander.drawExpander == TV_EXPANDER_ENABLED){
			if (!desc->checkbox.drawCheckbox)
				post->pos.x1 -= 2;
		
			tvRenderExpander(tv, frame, item, rt, post, colour);
		}

		for (int i = 2; i <= desc->imageTotal; i++){
			image = tvTreeItemImageGet(tv, item, i);
	
			if (image && image->drawImage == TV_DRAWIMAGE_ENABLED){
				//if (post) rt.sy = post->pos.y2;
				post = tvRenderImage(tv, frame, image, id, rt);
				if (post) rt->sx = post->pos.x2 + TEXT_INDENT_GUESS;
			}
		}
		return 1;
	}else{
		return 0;
	}
}

static inline void ldrawRectangleFilled (TFRAME *frame, TMETRICS *met, const uint32_t colour)
{
	lDrawRectangleFilled(frame, met->x, met->y, met->x+met->width-1, met->y+met->height-1, colour);
}

int _tvRender (TTV *tv, TFRAME *frame, TTV_RENDER *restrict pre)
{
	if (tv->renderFlags&TV_RENDER_BLUR){
		lBlurArea(frame, tv->metrics.x+1, tv->metrics.y+1, tv->metrics.x+tv->metrics.width-1, tv->metrics.y+tv->metrics.height-2, 2);
		//lBlurImage(frame, lBLUR_STACKFAST, 2);
		//ldrawRectangleFilled(frame, &tv->metrics, 100<<24|COL_BLACK);
	}
	if (tv->renderFlags&TV_RENDER_UNDERLAY)
		ldrawRectangleFilled(frame, &tv->metrics, (60<<24)|COL_BLUE_SEA_TINT);
		
	if (tv->renderFlags&TV_RENDER_AREAFILLED)
		ldrawRectangleFilled(frame, &tv->metrics, (80<<24)|COL_BLACK);
	
	TLPRINTR rt;
	rt.bx1 = tv->metrics.x;
	rt.by1 = tv->metrics.y + 1;
	rt.bx2 = tv->metrics.x + tv->metrics.width-1;
	rt.by2 = tv->metrics.y + tv->metrics.height-2;

	if (tv->renderStartY < 0) tv->renderStartY = 0;
	tv->tPostItems = 0;


	//calc yoffset
	int y = tv->metrics.y - tv->renderStartY;
	
	while (pre && pre->rItem){
		if (y >= tv->metrics.y - MIN_IMAGE_HEIGHT-16){
			rt.sx = pre->rItem->pos.x1;
			rt.sy = y;
			const int id = pre->rItem->id;
	
			TTV_ITEM *item = tvTreeGetItem(tv, id, NULL);
			if (item){
				int ret = tvRenderItem(tv, frame, item, &rt, y);
				tvTreeFreeItem(item);
				
				if (!ret)
					break;
				else if (ret < 0)	// as item is disabled it is not rendered, so account with this
					y -= pre->rItem->height;
			}
		}
		y += pre->rItem->height;
		pre = pre->next;
	}
			
	if (tv->sbVert->rangeMax > tv->metrics.height){
		if (tv->renderFlags&TV_RENDER_SCROLLWIDGETS)
			ccRender(tv->sbVert, frame);
	}else if (tv->renderFlags&TV_RENDER_UNDERLAY){
		tv->sbVertPosition = 0;
		lDrawLine(frame, tv->metrics.x+tv->metrics.width, tv->metrics.y, tv->metrics.x+tv->metrics.width, tv->metrics.y+tv->metrics.height-1, (255<<24)|COL_BLUE_SEA_TINT);
	}
		
	if (tv->renderFlags&TV_RENDER_UNDERLAY){
		const int swidth = 0;
		int colour = (240<<24)|COL_BLUE_SEA_TINT;
		lDrawLine(frame, tv->metrics.x+tv->metrics.width, tv->metrics.y, tv->metrics.x+tv->metrics.width, tv->metrics.y+tv->metrics.height-1, colour);
		lDrawLine(frame, tv->metrics.x, tv->metrics.y, tv->metrics.x+tv->metrics.width-1-swidth, tv->metrics.y, colour);
		lDrawLine(frame, tv->metrics.x, tv->metrics.y+1, tv->metrics.x, tv->metrics.y+tv->metrics.height-1, colour);
		lDrawLine(frame, tv->metrics.x, tv->metrics.y+tv->metrics.height-1, tv->metrics.x+tv->metrics.width-1-swidth, tv->metrics.y+tv->metrics.height-1, colour);
		
		colour = (180<<24)|COL_BLACK;
		lDrawLine(frame, tv->metrics.x+tv->metrics.width+1, tv->metrics.y-1, tv->metrics.x+tv->metrics.width+1, tv->metrics.y+tv->metrics.height, colour);
		lDrawLine(frame, tv->metrics.x-1, tv->metrics.y-1, tv->metrics.x+tv->metrics.width-swidth, tv->metrics.y-1, colour);
		lDrawLine(frame, tv->metrics.x-1, tv->metrics.y, tv->metrics.x-1, tv->metrics.y+tv->metrics.height, colour);
		lDrawLine(frame, tv->metrics.x-1, tv->metrics.y+tv->metrics.height, tv->metrics.x+tv->metrics.width-swidth, tv->metrics.y+tv->metrics.height, colour);
		
		colour = (140<<24)|COL_WHITE;
		lDrawLine(frame, tv->metrics.x+tv->metrics.width+2, tv->metrics.y-2, tv->metrics.x+tv->metrics.width+2, tv->metrics.y+tv->metrics.height+1, colour);
		lDrawLine(frame, tv->metrics.x-2, tv->metrics.y-2, tv->metrics.x+tv->metrics.width-swidth+1, tv->metrics.y-2, colour);
		lDrawLine(frame, tv->metrics.x-2, tv->metrics.y-1, tv->metrics.x-2, tv->metrics.y+tv->metrics.height+1, colour);
		lDrawLine(frame, tv->metrics.x-2, tv->metrics.y+tv->metrics.height+1, tv->metrics.x+tv->metrics.width-swidth+1, tv->metrics.y+tv->metrics.height+1, colour);
	}
	
#if DRAWTOUCHRECTS
	lDrawRectangle(frame, tv->metrics.x, tv->metrics.y, tv->metrics.x+tv->metrics.width-1, tv->metrics.y+tv->metrics.height-1, DRAWTOUCHRECTCOL);
	tv->renderFlags |= TV_RENDER_BBOX;
#endif

	if (tv->renderFlags&TV_RENDER_BBOX){
		TTV_RENDER_ITEM *post = tv->postRender;
		for (int i = 0; i < tv->tPostItems; i++, post++)
			lDrawRectangle(frame, post->pos.x1, post->pos.y1, post->pos.x2, post->pos.y2, 0xFFFF0000);
	}
	return 1;
}

static inline int isOverlap (TTOUCHCOORD *pos, const TLPOINTEX *box)
{
	if (pos->x >= box->x1 && pos->x <= box->x2){
		if (pos->y >= box->y1 && pos->y <= box->y2)
			return 1;
	}
	return 0;
}


// return object touched, if any
static inline int tvInputGetTouchedPostId (TTV *tv, TTOUCHCOORD *pos, const int flags)
{
	TTV_RENDER_ITEM *post = tv->postRender;
	
	for (int i = 0; i < tv->tPostItems; i++, post++){
		if (isOverlap(pos, &post->pos)){
			//printf("post id %i %X\n", post->id, post->id);
			return post->id;
		}
	}
	return 0;
}

static inline TTV_RENDER_ITEM *tvInputGetTouchedPost (TTV *tv, TTOUCHCOORD *pos, const int flags)
{
	TTV_RENDER_ITEM *post = tv->postRender;
	
	for (int i = 0; i < tv->tPostItems; i++, post++){
		if (isOverlap(pos, &post->pos)){
			//printf("post id %i %X\n", post->id, post->id);
			return post;
		}
	}
	return NULL;
}

static inline int tvInputGetUnderCursor (TTV *tv, TTV_RENDER_ITEM *post, TTOUCHCOORD *pos)
{
	for (int i = 0; i < tv->tPostItems; i++, post++){
		if (isOverlap(pos, &post->pos))
			return post->id;
	}
	return 0;
}

void tvRenderSetNodeIdLocation (TTV *tv, const int x, const int y)
{
	tv->idt.pos.x = x;
	tv->idt.pos.y = y;
}

void tvRenderSetNodeIdFont (TTV *tv, const char *font)
{
	tv->idt.font = (char*)font;
}

void tvRenderSetNodeIdColour (TTV *tv, const int colour)
{
	tv->idt.colour = colour;
}

int tvRender (void *object, TFRAME *frame)
{
	TTV *tv = (TTV*)object;
	lSetCharacterEncoding(frame->hw, CMT_UTF8);
	int ret = 0;

	if (ccLock(tv)){
		// render the control
		ret = _tvRender(tv, frame, tv->preRender);
		
		
		// draw id of node under cursor -- this shouldn't be here
		if (tv->renderFlags&TV_RENDER_IDTEXT && tv->cc->cursor->isHooked && tv->inputEnabled){
			TTOUCHCOORD pos = {.x = tv->cc->cursor->dx, .y = tv->cc->cursor->dy};

			int id = tvInputGetUnderCursor(tv, tv->postRender, &pos);
			if (id)
				com_drawHex(frame, tv->idt.pos.x, tv->idt.pos.y, tv->idt.font, id, tv->idt.colour);
		}
		
		// if drag is in motion render its label
		if (tv->dragEnabled && tv->drag.state == 2){
			TLPRINTR rt;
			rt.bx1 = tv->metrics.x;
			rt.by1 = tv->metrics.y + 1;
			rt.bx2 = tv->metrics.x + tv->metrics.width-1;
			rt.by2 = tv->metrics.y + tv->metrics.height-2;
			rt.sx = tv->drag.post.pos.x1;
			rt.sy = tv->drag.post.pos.y1;


			// draw label of dragged item
			if (tv->renderFlags&TV_RENDER_DRAGITEM){
				TTV_ITEM *item = tvTreeGetItem(tv, tv->drag.post.id, NULL);
				if (item){
					lSetForegroundColour(frame->hw, COL_AQUA);	// todo: move colour selection elsewhere
				
					TTV_RENDER_ITEM *ritem = tvRenderLabel(tv, frame, item, &rt);
					tvTreeFreeItem(item);
					if (ritem) tv->tPostItems--;
				}
			}

			// draw destination location
			if (tv->renderFlags&TV_RENDER_PASTEDEST){
				TTV_ITEM *item = tvTreeGetItem(tv, tv->drag.dest.id, NULL);
				if (item){
					rt.by1 = tv->metrics.y - 40;
					rt.sx = 10;
					rt.sy = rt.by1;
					lSetForegroundColour(frame->hw, COL_GREEN_TINT);
					
					TTV_RENDER_ITEM *ritem = tvRenderLabel(tv, frame, item, &rt);
					tvTreeFreeItem(item);
					if (ritem) tv->tPostItems--;
				}
			}
			lSetForegroundColour(frame->hw, 255<<24 | 0xFFFFFF);
		}
		ccUnlock(tv);
	}
	return ret;
}

static inline void tvDeleteRender (TTV_RENDER *render)
{
	while(render){
		if (render->rItem)
			my_free(render->rItem);

		TTV_RENDER *next = render->next;

		my_free(render);
		render = next;
	}
}

static inline void tvDelete (void *object)
{
	//printf("tvDelete in %i\n", lockCt);
	
	TTV *tv = (TTV*)object;
	if (!tv) return;
	
	if (ccLock(tv)){
		tvDeleteRender(tv->preRender);

		if (tv->tree)
			tvTreeDelete(tv, tv->rootId);
			
		ccDelete(tv->sbVert);
		ccUnlock(tv);
	}
}

static inline void tvEnable (void *object)
{
	TTV *tv = (TTV*)object;
	tv->enabled = 1;
	tv->sbVert->enabled = 1;
}

static inline void tvDisable (void *object)
{	
	TTV *tv = (TTV*)object;
	tv->enabled = 0;
	tv->sbVert->enabled = 0;
}

void tvScrollUp (TTV *tv)
{
	int64_t value = scrollbarGetFirstItem(tv->sbVert) - (1 + tv->sbVertPositionRate);
	value = scrollbarSetFirstItem(tv->sbVert, value);
	tv->sbVertPosition = value;
}

void tvScrollDown (TTV *tv)
{
	int64_t value = scrollbarGetFirstItem(tv->sbVert) + (1 + tv->sbVertPositionRate);
	value = scrollbarSetFirstItem(tv->sbVert, value);
	tv->sbVertPosition = value;
}

static inline TTV_RENDER_ITEM *tvNewRenderItem ()
{
	return my_calloc(1, sizeof(TTV_RENDER_ITEM));
}

static inline TTV_RENDER *tvNewRender ()
{
	return my_malloc(sizeof(TTV_RENDER));
}

static inline int tvSetCheckNodeDeselectParents (TTV *tv, TTV_ITEM *item)
{
	int ct = 0;
	int parentId = item->parentId;
	
	while (parentId){
		TTV_ITEM *subItem = tvTreeGetItem(tv, parentId, NULL);
		if (subItem){
			TTV_ITEM_DESC *desc = tvTreeItemGetDesc(subItem);
			if (desc){
				desc->checkbox.checkState = TV_CHECKBOX_CLEAR;
				ct++;
			}

			parentId = subItem->parentId;
			tvTreeFreeItem(subItem);
		}
	}
	return ct;
}
/*
int tvExpandTo1 (TTV *tv, const int id)
{
	int stack[1024] = {0};
	int sp = 0;
	
	TTV_ITEM *item = tvTreeGetItem(tv, id, NULL);
	if (!item) return 0;

	int parentId = item->parentId;
	while (parentId){
		stack[sp++] = parentId;

		TTV_ITEM *subItem = tvTreeGetItem(tv, parentId, NULL);
		if (subItem){
			parentId = subItem->parentId;
			tvTreeFreeItem(subItem);
		}else{
			break;
		}
	}
	tvTreeFreeItem(item);
	
	for (int i = 0; i < sp; i++)
		printf("%i %X\n", i, stack[i]);
	
	return sp;
}
*/
static inline int getNextSiblingId (TTV *tv, TTV_ITEM *parent, const int id)
{
	int nextId = 0;
	int child = 0;
	// find next sibling below selected branch. remove all between
	while(parent->children[child]){
		if (parent->children[child] == id){
			nextId = parent->children[child+1];
			//printf("nextId %i (%i)\n", nextId, id);
			break;
		}
		child++;
	}
	return nextId;
}

static inline int tvCalcRenderedHeight (const TTV_RENDER *render)
{
	int height = 0;
	while(render){
		//if (render->rItem)
			height += render->rItem->height;
		render = render->next;
	}
	
	return height;
}
#if 0
static inline void tvDoActionOpen (TTV *tv, TTV_ITEM *item, const int id, TTV_ITEM_DESC *desc)
{

	TLPOINTEX pos;
	if (desc->expander.drawExpander == TV_EXPANDER_ENABLED)
		pos.x1 = TEXT_INDENT_GUESS+6;
	else
		pos.x1 = 4;
	pos.y1 = 0;
	pos.x2 = pos.x1 + ccGetWidth(tv) - 1;
	pos.y2 = pos.y1 + ccGetHeight(tv) - 1;
				
	TTV_RENDER *render = tv->preRender;
	while(render){
		if (render->rItem->id == id){
			pos.x1 += render->rItem->pos.x1;
			pos.y1 += render->rItem->pos.y1;
			break;
		}
		render = render->next;
	}

	int height = 0;
	TTV_RENDER *subRender = NULL;
	TTV_ITEM *sitem = tvTreeGetItem(tv, id, NULL);
	if (sitem){
		subRender = tv_renderPreCalc(tv, sitem, &pos, &height);
		tvTreeFreeItem(sitem);
	}

	if (subRender){
		render = tv->preRender;
		while(render){	
			if (render->rItem->id == id){	
				TTV_RENDER *right = render->next;
				render->next = subRender;
				
				while (subRender->next)		// get last entry
					subRender = subRender->next;
				subRender->next = right;

				int height = tvCalcRenderedHeight(tv->preRender);
				int first = scrollbarGetFirstItem(tv->sbVert);
				scrollbarSetRange(tv->sbVert, 0, height-2, first, tv->metrics.height-2);
				break;
			}
			render = render->next;
		}
	}
}
#endif
#if 0
static inline void tvDoActionClose (TTV *tv, TTV_ITEM *item, const int id, TTV_ITEM_DESC *desc)
{

	int x = 0;
	TTV_RENDER *pre = NULL;
	TTV_RENDER *head = NULL;
	TTV_RENDER *tail = NULL;
	TTV_RENDER *start = NULL;
	TTV_RENDER *render = tv->preRender;

	int nextId = 0;
	int parentId = item->parentId;
	int searchForId = item->id;
			
	// get rendered item (id) directly below the last item which we're closing
	while(!nextId){
		TTV_ITEM *parent = tvTreeGetItem(tv, parentId, NULL);
		if (parent){
			nextId = getNextSiblingId(tv, parent, searchForId);
			parentId = parent->parentId;
			searchForId = parent->id;
			tvTreeFreeItem(parent);
		}else{
			break;
		}
	}
	
	// find head and tail of chain being closed
	// then stitch together
	while(render){
		if (x){
			if (render->rItem->id == nextId){
				tail = pre;
				tail->next = NULL;
				head->next = render;
				break;
			}
		}else if (render->rItem->id == id){
			x = render->rItem->pos.x1;
			head = render;
			start = render->next;
		}
		pre = render;
		render = render->next;
	}

	if ((!tail || !nextId) && head)
		head->next = NULL;


	// free the now removed closed chain
	tvDeleteRender(start);
	
	int height = tvCalcRenderedHeight(tv->preRender);
	int first = scrollbarGetFirstItem(tv->sbVert);
	scrollbarSetRange(tv->sbVert, 0, height-2, first, tv->metrics.height-2);

}
#endif

static inline int tvInputDoAction (TTV *tv, const int id, const int action, TTOUCHCOORD *pos, void *opaque)
{
	/*TTV_ITEM *item = tvTreeGetItem(tv, id);
	if (!item) return 0;
	TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
	*/

	//printf("tvInputDoAction %p\n", opaque);
	
	int freeItem = (opaque == NULL);
	if (freeItem)
		opaque = tvTreeGetItem(tv, id, NULL);
		
	
	if (action == TV_ACTION_EXPANDER_OPEN){
		//printf("action == TV_ACTION_EXPANDER_OPEN %i\n", id);
		
		TTV_ITEM *item = (TTV_ITEM*)opaque;
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);

		if (desc->expander.expanderState != TV_EXPANDER_OPEN){
			desc->expander.expanderState = TV_EXPANDER_OPEN;
			ccSendMessage(tv, TV_MSG_EXPANDERSTATE, desc->expander.expanderState, item->id, item);
			//tvDoActionOpen(tv, item, item->id, desc);
			//tvBuildPrerender(tv, tv->rootId);
		}
	}else if (action == TV_ACTION_EXPANDER_CLOSE){
		//printf("action == TV_ACTION_EXPANDER_CLOSE %i\n", id);
		
		TTV_ITEM *item = (TTV_ITEM*)opaque;
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
		
		if (desc->expander.expanderState != TV_EXPANDER_CLOSED){
			desc->expander.expanderState = TV_EXPANDER_CLOSED;
			ccSendMessage(tv, TV_MSG_EXPANDERSTATE, desc->expander.expanderState, item->id, item);
			//tvDoActionClose(tv, item, item->id, desc);
			//tvBuildPrerender(tv, tv->rootId);
		}
	}else if (action == TV_ACTION_EXPANDER_TOGGLE){
		//printf("action == TV_ACTION_EXPANDER_TOGGLE %i\n", id);
		
		TTV_ITEM *item = (TTV_ITEM*)opaque;
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
		
		if (desc->expander.expanderState == TV_EXPANDER_OPEN)
			desc->expander.expanderState = TV_EXPANDER_CLOSED;
		else
			desc->expander.expanderState = TV_EXPANDER_OPEN;
		
		ccSendMessage(tv, TV_MSG_EXPANDERSTATE, desc->expander.expanderState, item->id, item);
		
		tvBuildPrerender(tv, tv->rootId);
		/*if (desc->expander.expanderState == TV_EXPANDER_OPEN)
			tvDoActionOpen(tv, item, item->id, desc);
		else
			tvDoActionClose(tv, item, item->id, desc);*/

	}else if (action == TV_ACTION_CHECKBOX_TOGGLE){
		//printf("action == TV_ACTION_CHECKBOX_TOGGLE %i\n", id);
		
		TTV_ITEM *item = (TTV_ITEM*)opaque;
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
		
		if (isChecked(item))
			desc->checkbox.checkState = TV_CHECKBOX_CLEAR;
		else
			desc->checkbox.checkState = TV_CHECKBOX_CHECKED;

		if (item->type == TREE_TYPE_BRANCH)
			tvSetCheckNode(tv, item, desc->checkbox.checkState);

		if (desc->checkbox.checkState == TV_CHECKBOX_CLEAR){
			// deselect parent branches
			tvSetCheckNodeDeselectParents(tv, item);
		}
		ccSendMessage(tv, TV_MSG_CHECKBOXSTATE, desc->checkbox.checkState, item->id, item);
		
	}else if (action == TV_ACTION_TRACK_SELECT){
		ccSendMessage(tv, TV_MSG_ITEMSELECT, 0, id, opaque);
		
	}else if (action == TV_ACTION_IMAGE_SELECT){
		//printf("action == TV_ACTION_IMAGE_SELECT %i, %p\n", id, opaque);
		
		TTV_ITEM *item = tvTreeGetItem(tv, id, NULL);
		if (!item) return 0;
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);

		for (int i = 1; i <= desc->imageTotal; i++){
			if (opaque == tvTreeItemImageGet(tv, item, i)){
				ccSendMessage(tv, TV_MSG_IMAGESELECT, i, item->id, opaque);
				break;
			}
		}
		tvTreeFreeItem(item);
	}
	
	if (freeItem)
		tvTreeFreeItem(opaque);
	return 1;
}

static inline void tvItemDisable (TTV *tv, const int id)
{
	TTV_ITEM *item = tvTreeGetItem2(tv, id, NULL);
	if (item){
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
		if (desc) desc->enabled = 0;
		tvTreeFreeItem2(item);
	}
}

void tvItemEnable (TTV *tv, const int id)
{
	TTV_ITEM *item = tvTreeGetItem2(tv, id, NULL);
	if (item){
		TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
		if (desc) desc->enabled = 1;
		tvTreeFreeItem2(item);
	}
}

static inline void tvSetExpandOpenAll (TTV *tv, TTV_ITEM *item, const int state)
{
	if (item->type != TREE_TYPE_BRANCH)
		return;

	int child = 0;
	while(item->children[child]){
		TTV_ITEM *subItem = tvTreeGetItem(tv, item->children[child], item->entry);
		if (subItem){
			if (subItem->type == TREE_TYPE_BRANCH){
				TTV_ITEM_DESC *desc = tvTreeItemGetDesc(subItem);
				
				if (desc->expander.expanderState == TV_EXPANDER_CLOSED){
					if (state == TV_EXPANDER_OPEN)
						tvInputDoAction(tv, item->children[child], TV_ACTION_EXPANDER_OPEN, NULL, subItem);
					else if (state == TV_EXPANDER_CLOSED)
						tvInputDoAction(tv, item->children[child], TV_ACTION_EXPANDER_CLOSE, NULL, subItem);
					else
						ccSendMessage(tv, TV_MSG_EXPANDERSTATE, desc->expander.expanderState, item->children[child], subItem);
										
					//desc->expander.expanderState = state;
					tvSetExpandOpenAll(tv, subItem, state);
				}
			}
			tvTreeFreeItem(subItem);
		}
		child++;
	};
}

static inline void tvSetExpandCloseAll (TTV *tv, TTV_ITEM *item, const int state)
{
	if (item->type != TREE_TYPE_BRANCH)
		return;

	int child = 0;
	while(item->children[child]){
		TTV_ITEM *subItem = tvTreeGetItem(tv, item->children[child], item->entry);
		if (subItem){
			if (subItem->type == TREE_TYPE_BRANCH){
				TTV_ITEM_DESC *desc = tvTreeItemGetDesc(subItem);
				if (desc->expander.expanderState == TV_EXPANDER_OPEN){

					/*if (state == TV_EXPANDER_OPEN)
						tvInputDoAction(tv, item->children[child], TV_ACTION_EXPANDER_OPEN, NULL, subItem);
					else*/ if (state == TV_EXPANDER_CLOSED)
						tvInputDoAction(tv, item->children[child], TV_ACTION_EXPANDER_CLOSE, NULL, subItem);
					else
						ccSendMessage(tv, TV_MSG_EXPANDERSTATE, desc->expander.expanderState, item->children[child], subItem);
				
					//desc->expander.expanderState = state;
					tvSetExpandCloseAll(tv, subItem, state);
				}
			}

			tvTreeFreeItem(subItem);
		}
		child++;
	};
}

void tvExpandAll (TTV *tv)
{
	if (ccLock(tv)){
		TTV_ITEM *item = tvTreeGetItem(tv, tv->rootId, NULL);
		if (item){
			tvSetExpandOpenAll(tv, item, TV_EXPANDER_OPEN);
			tvTreeFreeItem(item);
		}
		ccUnlock(tv);
	}
}

void tvCollapseAll (TTV *tv)
{
	if (ccLock(tv)){
		TTV_ITEM *item = tvTreeGetItem(tv, tv->rootId, NULL);
		if (item){
			tvSetExpandCloseAll(tv, item, TV_EXPANDER_CLOSED);
			tvTreeFreeItem(item);
		}
		ccUnlock(tv);
	}
}

void tvSetCheckNode (TTV *tv, TTV_ITEM *item, const int state)
{
	if (item->type != TREE_TYPE_BRANCH)
		return;

	int child = 0;
	while(item->children[child]){
		TTV_ITEM *subItem = tvTreeGetItem(tv, item->children[child], item->entry);
		if (subItem){
			TTV_ITEM_DESC *desc = tvTreeItemGetDesc(subItem);
		
			desc->checkbox.checkState = state;
			if (subItem->type == TREE_TYPE_BRANCH)
				tvSetCheckNode(tv, subItem, state);

			tvTreeFreeItem(subItem);
		}
		child++;
	};
}

void tvCheckSetAll (TTV *tv)
{
	//printf("tvCheckSetAll\n");
	
	if (ccLock(tv)){
		TTV_ITEM *item = tvTreeGetItem(tv, tv->rootId, NULL);
		if (item){
			tvSetCheckNode(tv, item, TV_CHECKBOX_CHECKED);
			tvTreeFreeItem(item);
		}
		tvBuildPrerender(tv, tv->rootId);
		ccUnlock(tv);
	}
}

void tvCheckClearAll (TTV *tv)
{
	//printf("tvCheckClearAll\n");
	
	if (ccLock(tv)){
		TTV_ITEM *item = tvTreeGetItem(tv, tv->rootId, NULL);
		if (item){
			tvSetCheckNode(tv, item, TV_CHECKBOX_CLEAR);
			tvTreeFreeItem(item);
		}
		
		tvBuildPrerender(tv, tv->rootId);
		ccUnlock(tv);
	}
}

void tvInputDisable (TTV *tv)
{
	tv->inputEnabled = 0;
}

void tvInputEnable (TTV *tv)
{
	tv->inputEnabled = 1;
}

int tvRenameGetState (TTV *tv)
{
	return tv->renameEnabled;
}

void tvRenameSetState (TTV *tv, const int state)
{
	//printf("tvRenameSetState %i\n", state);
	tv->renameEnabled = state;
}

void tvRenameEnable (TTV *tv)
{
	tvRenameSetState(tv, 1);
}

void tvRenameDisable (TTV *tv)
{
	tvRenameSetState(tv, 0);
}

static inline int _tvHandleInput (TTV *tv, TTOUCHCOORD *pos, const int flags)
{
	TTV_RENDER_ITEM *post = tv->postRender;
	
	for (int i = 0; i < tv->tPostItems; i++, post++){
		if (isOverlap(pos, &post->pos)){
			//printf("# down: post id %X, %i: %i %i, %i %i\n", post->id, tv->dragEnabled, pos->dt, (int)pos->time, flags, pos->pen);
			
			if (!flags){
				TTV_ITEM *item = tvTreeGetItem(tv, post->id, NULL);
				if (item){
					TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
					if (desc){
						desc->time = pos->time;
						desc->tState = 1;
					}
					tvTreeFreeItem(item);
				}
			}
			
			if (tv->dragEnabled){
				tv->drag.post = *post;
				//my_memcpy(&tv->drag.post, post, sizeof(TTV_RENDER_ITEM));
				
				tv->drag.ox = pos->x - tv->drag.post.pos.x1;
				tv->drag.oy = pos->y - tv->drag.post.pos.y1;
				tv->drag.sx = tv->drag.post.pos.x1;
				tv->drag.sy = tv->drag.post.pos.y1;
			}
			tvInputDoAction(tv, post->id, post->action, pos, post->opaque);
			
			if (post->action == TV_ACTION_EXPANDER_OPEN || post->action == TV_ACTION_EXPANDER_CLOSE)
				tvBuildPrerender(tv, tv->rootId);
			return 1;
		}
	}
	return 0;
}

int tvHandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TTV *tv = (TTV*)object;
	if (!tv->inputEnabled) return 0;
	
	int ret = 0;
	if (ccLock(tv)){
		if (tv->renameEnabled && !flags){		// we're renaming an item, find then setup a keypad callback
			// enable and setup virtual keyboard, when done post message to treeview owner
			//TVLCPLAYER *vp = tv->cc->vp;
			
			int postId = tvInputGetTouchedPostId(tv, pos, flags);
			TTV_ITEM *item = tvTreeGetItem(tv, postId, NULL);
			if (item){
				//if (!page2RenderGetState(vp->pages, PAGE_VKEYBOARD)){
				// is keypad on display?
				printf("tv.c::tvHandleInput: handle me\n");
				/*
					TKEYBOARD *vkey = pageGetPtr(vp, PAGE_VKEYBOARD);
	  				TKEYPAD *kp = vkey->kp;
	  				
	  				keypadListenerAdd(kp, tv->id, KP_INPUT_OPENED|KP_INPUT_CLOSED|KP_INPUT_COMPLETE8, item->id);
	  				keypadEditboxSetBuffer8(&kp->editbox, item->name);
	  				keypadEditboxSetUndoBuffer8(&kp->editbox, item->name);
	  				keypadEditboxSetUserData(&kp->editbox, item->id);

					page2Set(vkey->com->pages, PAGE_RENDER_CONCURRENT|PAGE_VKEYBOARD, 0);
					ccEnable(kp);
				*/
				//}
				tvTreeFreeItem(item);
			}
		}else if (tv->dragEnabled){				// we're in drag mode
			if (flags == 0x03){					// mouse up
				tv->drag.state = 0;
				
				TTV_INPUT_DROP *tvid = my_calloc(1, sizeof(TTV_INPUT_DROP));
				if (tvid){
					//printf("mouse up %X %i\n", tv->drag.post.id, tv->drag.state);
					tvItemEnable(tv, tv->drag.post.id);
					
					tvid->from = tv->drag.post.id;
					tvid->to = tv->drag.dest.id;
					tvid->action = tv->drag.action;
					tv->drag.dest.id = 0;
					tv->drag.post.id = 0;
				
					if (tvid->from && tvid->to && tvid->from != tvid->to && tvid->action)
						ccSendMessage(tv, TV_MSG_ITEMDROP, tv->drag.action, 0, tvid);
					my_free(tvid);
				}else{
					tv->drag.dest.id = 0;
					tv->drag.post.id = 0;
				}
				tv->drag.action = 0;
				
			}else if (flags == 0x01){				// mouse drag
				tv->drag.post.pos.x1 = pos->x - tv->drag.ox;
				tv->drag.post.pos.y1 = pos->y - tv->drag.oy;

				if (tv->drag.state == 1 && tv->drag.post.id){
					if (abs(tv->drag.post.pos.x1 - tv->drag.sx) > TV_DRAGDELTA || abs(tv->drag.post.pos.y1 - tv->drag.sy) > TV_DRAGDELTA){
						tv->drag.state = 2;
						tv->drag.dest.id = 0;
						tv->drag.action = 0;
						
						tvInputDoAction(tv, tv->drag.post.id, TV_ACTION_EXPANDER_CLOSE, NULL, NULL);
						tvItemDisable(tv, tv->drag.post.id);
						tvBuildPrerender(tv, tv->rootId);
					}
				}
				if (tv->drag.state == 2){
					if (pos->y <= tv->metrics.y+8){
						tv->sbVertPositionRate = 8;
						tvScrollUp(tv);
					}else if (pos->y >= (tv->metrics.y+tv->metrics.height)-8){
						tv->sbVertPositionRate = 8;
						tvScrollDown(tv);
					}
				}
			}else if (!flags){						// mouse down
				//printf("mouse down %X %i\n", tv->drag.post.id, tv->drag.state);
			
				tv->drag.dest.id = 0;
				tv->drag.state = 1;
				tv->drag.state = ret = _tvHandleInput(tv, pos, flags);
				if (!ret) tv->drag.post.id = 0;
			}
		}else if (!flags){
			ret = _tvHandleInput(tv, pos, flags);
#if 0								
		}else if (flags == 1){		// detect and show keypad if being requested and allow editing of that item
			int postId = tvInputGetTouchedPostId(tv, pos, flags);
			if (postId){
				//printf("state %i, %i %i\n", flags, tv->drag.state, pos->dt);

				TTV_ITEM *item = tvTreeGetItem(tv, postId, NULL);
				if (item){
					TTV_ITEM_DESC *desc = tvTreeItemGetDesc(item);
					if (desc->tState == 1){

						const int dt = pos->time - desc->time;
						//printf("move: post id %X, %i: %i %i %i\n", postId, tv->dragEnabled, (int)desc->time, (int)pos->time, dt);
						if (dt >= 700){
							desc->tState = 0;
							if (dt <= 1250){
								TVLCPLAYER *vp = tv->cc->vp;
								if (!page2RenderGetState(vp->pages, PAGE_VKEYBOARD)){
									TKEYBOARD *vkey = pageGetPtr(vp, PAGE_VKEYBOARD);
	  								TKEYPAD *kp = vkey->kp;

									ccEnable(kp);	// ensure pad is built by enabling it before use
	  								keypadListenerAdd(kp, tv->id, KP_INPUT_OPENED|KP_INPUT_CLOSED|KP_INPUT_COMPLETE8, item->id);
	  								keypadEditboxSetBuffer8(&kp->editbox, item->name);
	  								keypadEditboxSetUndoBuffer8(&kp->editbox, item->name);
	  								keypadEditboxSetUserData(&kp->editbox, item->id);
									page2Set(vp->pages, PAGE_VKEYBOARD, 1);
								}
							}else{
								desc->time = 0;
							}
						}

					}
					tvTreeFreeItem(item);
				}
			}
#endif
		}
		ccUnlock(tv);
	}
								
	return ret;
}

int tvSetPosition (void *object, const int x, const int y)
{
	//printf("treeviewSetPosition %i %i\n", x, y);
	
	TTV *tv = (TTV*)object;
	tv->metrics.x = x;
	tv->metrics.y = y;

	int swidth = tv->sbVert->metrics.width;
	ccSetMetrics(tv->sbVert, ((x + tv->metrics.width-1) - swidth) - 8, y+1, swidth, tv->metrics.height-2);
	return 1;
}

int tvSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	//printf("tvSetMetrics %p, %i %i %i %i\n",object, x, y, width, height);
	
	TTV *tv = (TTV*)object;
	tv->metrics.width = width;
	tv->metrics.height = height;
	
	ccSetPosition(tv, x, y);

	tv->metrics.width = width;
	tv->metrics.height = height;
	return 1;
}

int tvScrollbarGetWidth (TTV *tv)
{
	return ccGetWidth(tv->sbVert);
}

int tvScrollbarSetWidth (TTV *tv, const int width)
{
	return ccSetMetrics(tv->sbVert, -1, -1, width, ccGetHeight(tv->sbVert));
}

static inline TTV_RENDER *tv_renderPreCalc (TTV *tv, TTV_ITEM *item, TLPOINTEX *pos, int *height)
{
	if (item->type != TREE_TYPE_BRANCH) return NULL;

	TTV_RENDER *parentRender = NULL;
	TTV_RENDER *root = NULL;
	TTV_RENDER *next = NULL;
	
	int child = 0;
	while(item->children[child]){
		//printf("tvTreeGetItem2 %X %i %p\n", item->children[child], item->children[child], item->entry);
		
		TTV_ITEM *subItem = tvTreeGetItem2(tv, item->children[child], item->entry);
		if (subItem){
			TTV_ITEM_DESC *desc = subItem->storage;//tvTreeItemGetDesc(subItem);

			//printf("preCalc %i %i %p %p '%s', %i\n", child, item->children[child], subItem, desc, subItem->name, desc->expander.expanderState);

			//if (desc->enabled){
				TTV_RENDER *render = tvNewRender();
				render->rItem = tvNewRenderItem();
				render->rItem->pos.x1 = (desc->metrics.x = pos->x1);
				render->rItem->pos.x2 = pos->x1 + desc->metrics.width-1;
				render->rItem->width = (render->rItem->pos.x2 - render->rItem->pos.x1);
				render->rItem->pos.y1 = (desc->metrics.y = pos->y1);
				render->rItem->pos.y2 = pos->y1 += desc->metrics.height;
				render->rItem->height = (render->rItem->pos.y2 - render->rItem->pos.y1);
				*height += render->rItem->height;
				render->rItem->id = subItem->id;

				// printf("%i, %i %i\n", subItem->id, render->rItem->pos.y1, render->rItem->height);

				TTV_RENDER *subRender = NULL;

				if (subItem->type == TREE_TYPE_BRANCH){
					int indent;
					if (desc->expander.drawExpander == TV_EXPANDER_ENABLED)
						indent = TEXT_INDENT_GUESS+6;
					else
						indent = 4;

					if (desc->expander.expanderState == TV_EXPANDER_OPEN){
						pos->x1 += indent;
						subRender = tv_renderPreCalc(tv, subItem, pos, height);
						pos->x1 -= indent;
					}
				}
				render->next = subRender;
			
				if (subRender){
					while(subRender->next)
						subRender = subRender->next;
					next = subRender;					
				}else{
					next = render;
				}

				if (!root){
					root = render;
					parentRender = next;
				}else{
					parentRender->next = render;
					parentRender = next;
				}
			//}

			tvTreeFreeItem2(subItem);
		}
		child++;
	}

	return root;
}

static inline TTV_RENDER *tv_PreRender (TTV *tv, const int id, TLPOINTEX *pos, const int indent, int *height)
{
	
	TLPOINTEX box;
	box.x1 = ccGetPositionX(tv) + indent;
	box.y1 = ccGetPositionY(tv);
	box.x2 = box.x1 + ccGetWidth(tv) - 1;
	box.y2 = box.y1 + ccGetHeight(tv) - 1;
	
	//*pos = *(TLPOINTEX*)(&box);
	my_memcpy(pos, &box, sizeof(box));

	TTV_ITEM *item = tvTreeGetItem(tv, id, NULL);
	if (item){
		TTV_RENDER *render = tv_renderPreCalc(tv, item, pos, height);
		tvTreeFreeItem(item);

		pos->x2 = pos->x1;
		pos->y2 = pos->y1;
		pos->x1 = box.x1;
		pos->y1 = box.y1;
	
		return render;
	}
	return NULL;
}

void tvBuildPrerender (TTV *tv, const int id)
{
	//printf("tvBuildPrerender %i\n", id);
	
	TLPOINTEX pos;
	int height = 0;
	TTV_RENDER *render = tv_PreRender(tv, id, &pos, TV_RENDER_INDENT, &height);
	tvDeleteRender(tv->preRender);
	tv->preRender = render;


	//printf("height %i %i %f\n", height, h, t1-t0);

	int first = scrollbarGetFirstItem(tv->sbVert);
	scrollbarSetRange(tv->sbVert, 0, height-2, first, tv->metrics.height-2);
	scrollbarSetFirstItem(tv->sbVert, first);
}

void tvTreeEntryMove (TTV *tv, const int fromId, const int toId)
{
	if (ccLock(tv)){
		treeEntryMove(tv->tree, fromId, toId);
		ccUnlock(tv);
	}
}

void tvTreeEntryMoveToTail (TTV *tv, const int fromId, const int toId)
{
	if (ccLock(tv)){
		treeEntryMoveLast(tv->tree, fromId, toId);
		ccUnlock(tv);
	}
}

int tvNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t tv_cb, int *id, const int unused1, const int unused2)
{
	TTV *tv = (TTV*)object;

	tv->pageOwner = pageOwner;
	if (id) *id = tv->id;
	tv->type = type;
	
	tv->cb.msg = tv_cb;
	tv->cb.render = tvRender;
	tv->cb.create = tvNew;
	tv->cb.free = tvDelete;
	tv->cb.enable = tvEnable;
	tv->cb.disable = tvDisable;
	tv->cb.input = tvHandleInput;
	tv->cb.setPosition = tvSetPosition;
	tv->cb.setMetrics = tvSetMetrics;
	
	
	tv->canDrag = 0;
	tv->metrics.x = -1;
	tv->metrics.y = -1;
	tv->metrics.width = 64;
	tv->metrics.height = 64;
	tv->sbVertPositionRate = 0;
	tv->inputEnabled = 1;
	tv->dragEnabled = 0;
	tv->cbIdSource = 0;

	tv->idt.colour = COL_WHITE;
	tv->idt.font = UPDATERATE_FONT;
	tv->idt.pos.x = 0;
	tv->idt.pos.y = 0;
	
	tv->tree = NULL;
	tv->rootId = 0;

	tv->preRender = NULL;
	tv->renderStartY = 0;

	tv->sbVert = ccCreate(tv->cc, pageOwner, CC_SCROLLBAR_VERTICAL, tv_scrollbar_cb, NULL, 0, 0);
	ccSetUserData(tv->sbVert, tv);

	scrollbarSetRange(tv->sbVert, 0, tv->metrics.height-1, 0, 1);
	tv->sbVert->flags.drawBlur = 0;
	tv->sbVert->flags.drawBase = 0;
	tv->sbVert->flags.drawFrame = 0;
	ccDisable(tv->sbVert);

	return 1;
}



