
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



static inline TLABEL *listboxGetBase (TLISTBOX *listbox)
{
	return listbox->pane->base;
}

static inline int64_t scrollbarCalcRange (TLISTBOX *listbox)
{
	TPANE *pane = listbox->pane;
	const int lineHeight = listbox->flags.lineHeight;
	int64_t rmax = (pane->flags.total.text * lineHeight) + 1;//(pane->metrics.height-(lineHeight*2));//(lineHeight*5);
	if (rmax < lineHeight) rmax = lineHeight;

	//printf("scrollbarCalcRange aveHeight:%i  rmax:%i  total:%i, total:%i\n", lineHeight, (int)rmax, pane->flags.total.text, pane->totalHeight);

	scrollbarSetRange(listbox->scrollbar.vert, 0, rmax, listbox->scrollbar.vert->firstItem, pane->metrics.height);

	if (rmax <= listbox->metrics.height)
		ccDisable(listbox->scrollbar.vert);
	else if (listbox->enabled)
		ccEnable(listbox->scrollbar.vert);

	return rmax;
}

void listboxSetLineHeight (TLISTBOX *listbox, const int lineHeight)
{
	if (lineHeight){
		listbox->flags.lineHeight = lineHeight;
		listbox->pane->layout.metrics.vertLineHeight = listbox->flags.lineHeight;
	}
}

void listboxSetFont (TLISTBOX *listbox, const char *font)
{
	if (font)
		listbox->flags.font = (char*)font;
}

int listboxAddItem (TLISTBOX *listbox, const char *string, const int imgId, const uint32_t colour, const int64_t udataInt64)
{
	return paneTextAddEx(listbox->pane, imgId, listbox->flags.imgScale, string, listbox->flags.font, colour, 0, udataInt64);
}

int listboxUpdateItem (TLISTBOX *listbox, const int itemId, const char *string)
{	
	return paneTextReplace(listbox->pane, itemId, string);
}

int listboxUpdateItemEx (TLISTBOX *listbox, const int itemId, const char *string, const char *font, const int newImgId)
{	
	int ret = 0;
	
	if (ccLock(listbox)){
		if (font)
			labelRenderFontSet(listboxGetBase(listbox), itemId, font);
		if (string)
			ret += paneTextReplace(listbox->pane, itemId, string);
		if (newImgId)
			ret += paneImageReplace(listbox->pane, itemId+1, newImgId);

		ccUnlock(listbox);
	}

	return ret;
}

void listboxSetHighlightedColour (TLISTBOX *listbox, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	listbox->highlighted.colour.fore = fore;
	listbox->highlighted.colour.back = back;
	listbox->highlighted.colour.outline = outline;
}

void listboxSetHighlightedItem (TLISTBOX *listbox, const int itemId)
{
	
//	printf("listboxSetHighlightedItem %i (%i)\n", itemId, listbox->highlighted.itemId);
	
	if (listbox->highlighted.itemId == itemId) return;
	
	listboxSetItemColour(listbox, listbox->highlighted.itemId, listbox->highlighted.colourPre.fore, listbox->highlighted.colourPre.back, listbox->highlighted.colourPre.outline);
	if (itemId){
		listboxGetItemColour(listbox, itemId, &listbox->highlighted.colourPre.fore, &listbox->highlighted.colourPre.back, &listbox->highlighted.colourPre.outline);
		listboxSetItemColour(listbox, itemId, listbox->highlighted.colour.fore, listbox->highlighted.colour.back, listbox->highlighted.colour.outline);
	}
	listbox->highlighted.itemId = itemId;
}

void listboxSetItemColour (TLISTBOX *listbox, const int itemId, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	//printf("listboxSetItemColour %i %X\n", itemId, fore);
	
	if (itemId)
		labelRenderColourSet(listboxGetBase(listbox), itemId, fore, back, outline);
}

void listboxGetItemColour (TLISTBOX *listbox, const int itemId, uint32_t *fore, uint32_t *back, uint32_t *outline)
{
	//printf("listboxGetItemColour %i\n", itemId);
	
	if (itemId)
		labelRenderColourGet(listboxGetBase(listbox), itemId, fore, back, outline);
}

int listboxItemGet (TLISTBOX *listbox, const int itemId, char **str, int *imgId)
{
	return paneItemGetDetail(listbox->pane, itemId, str, imgId);
}

int listboxIndexToItemId (TLISTBOX *listbox, const int idx)
{
	return paneIndexToItemId(listbox->pane, idx);
}

void labelSetItemFont (TLISTBOX *listbox, const int itemId, const char *font)
{
	if (itemId)
		labelRenderFontSet(listboxGetBase(listbox), itemId, font);
}

void listboxRemoveItem (TLISTBOX *listbox, const int itemId)
{
	if (itemId)
		paneRemoveItem(listbox->pane, itemId);
	scrollbarCalcRange(listbox);
}

void listboxRemoveAll (TLISTBOX *listbox)
{
	paneRemoveAll(listbox->pane);
	scrollbarCalcRange(listbox);
}

static inline int listboxGetTotalItems (TLISTBOX *listbox)
{
	return listbox->pane->flags.total.text;
}

int listboxGetTotal (TLISTBOX *listbox)
{
	return listboxGetTotalItems(listbox);
}

void listboxEnable (void *object)
{
	//printf("@@ listboxEnable\n");
	TLISTBOX *listbox = (TLISTBOX*)object;

	listbox->enabled = 1;
	ccEnable(listbox->pane);
	scrollbarCalcRange(listbox);
}

void listboxDisable (void *object)
{
	//printf("@@ listboxDisable\n");
	
	TLISTBOX *listbox = (TLISTBOX*)object;

	listbox->enabled = 0;
	ccDisable(listbox->scrollbar.vert);
	ccDisable(listbox->pane);
}

int listboxSetUnderlsy (TLISTBOX *listbox, const int artId, const double scale, const int opacity, const int offsetX, const int offsetY, const int shadowIdx)
{
	if (artId){
		listbox->underlay.imgId = paneImageAdd(listbox->pane, artId, scale, PANE_IMAGE_CENTRE, -offsetX, -offsetY, 0);
		labelArtcOpacitySet(listboxGetBase(listbox), listbox->underlay.imgId, opacity);
		listbox->underlay.offset.x = offsetX;
		listbox->underlay.offset.y = offsetY;
		
		if (shadowIdx&0x03){
			listbox->underlay.colour = shadowIdx;
			listbox->underlay.drawShadow = 1;
		}else{
			listbox->underlay.drawShadow = 0;
		}
	}else{
		listbox->underlay.drawShadow = 0;
		listbox->underlay.imgId = 0;
	}

	return listbox->underlay.imgId;
}

int listboxRender (void *object, TFRAME *frame)
{
	TLISTBOX *listbox = (TLISTBOX*)object;

	if (listbox->underlay.imgId && listbox->underlay.drawShadow){
		TMETRICS metrics;
		labelArtcGetMetrics(listboxGetBase(listbox), listbox->underlay.imgId, &metrics.width, &metrics.height);
		metrics.x = (listbox->metrics.x + ((listbox->metrics.width  - metrics.width) /2)) - listbox->underlay.offset.x;
		metrics.y = (listbox->metrics.y + ((listbox->metrics.height - metrics.height)/2)) - listbox->underlay.offset.y;
		com_drawShadowUnderlay(listbox->cc, frame, metrics.x-1, metrics.y-1, metrics.width+2, metrics.height+2, listbox->underlay.colour);
	}

	ccRender(listbox->pane, frame);

	if (listbox->scrollbar.draw)
		ccRender(listbox->scrollbar.vert, frame);
	
	//lDrawRectangle(frame, listbox->metrics.x, listbox->metrics.y, listbox->metrics.x+listbox->metrics.width-1, listbox->metrics.y+listbox->metrics.height-1, DRAWTOUCHRECTCOL);
	return 1;
}

int listboxHandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TLISTBOX *listbox = (TLISTBOX*)object;
	if (!listbox->enabled) return 0;

	int ret = ccHandleInput(listbox->scrollbar.vert, pos, flags);
	if (!ret) ccHandleInput(listbox->pane, pos, flags);
	
	//printf("listboxHandleInput sb %i, %i %i %i %i\n", ret, listbox->metrics.x, listbox->metrics.y, listbox->metrics.width, listbox->metrics.height);

	return ret;
}

int listboxSetPosition (void *object, const int x, const int y)
{
	TLISTBOX *listbox = (TLISTBOX*)object;
	listbox->metrics.x = x;
	listbox->metrics.y = y;

	ccSetPosition(listbox->pane, x, y);
	
	int sbX = (listbox->metrics.x + listbox->metrics.width - listbox->scrollbar.width) - listbox->scrollbar.offset;
	int sbY = listbox->metrics.y;
	ccSetMetrics(listbox->scrollbar.vert, sbX, sbY, listbox->scrollbar.width, -1);
	scrollbarCalcRange(listbox);

	return 1;
}

int listboxSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TLISTBOX *listbox = (TLISTBOX*)object;
	

	listbox->metrics.x = x;
	listbox->metrics.y = y;
	listbox->metrics.width = width;
	listbox->metrics.height = height;

	ccSetMetrics(listbox->pane, -1, -1, width, height);
	ccSetMetrics(listbox->scrollbar.vert, -1, -1, -1, height-2);
	ccSetPosition(listbox, x, y);
	scrollbarCalcRange(listbox);

	return 1;
}

int listboxSetScrollbarWidth (TLISTBOX *listbox, const int swidth)
{
	listbox->scrollbar.width = swidth;
	ccSetMetrics(listbox->scrollbar.vert, -1, -1, swidth, -1);
	return ccSetMetrics(listbox, -1, -1, swidth, -1);
}

int listboxScroll (TLISTBOX *listbox, const int nPixels)
{
	int ret = paneScroll(listbox->pane, nPixels);
	scrollbarCalcRange(listbox);
	return ret;
}


int listboxScrollUp (TLISTBOX *listbox)
{
	//printf("listboxScrollUp %i\n", listbox->flags.scrollDelta);
	
	paneScroll(listbox->pane, listbox->flags.scrollDelta);
	scrollbarCalcRange(listbox);
	return 1;
}

int listboxScrollDown (TLISTBOX *listbox)
{
	//printf("listboxScrollDown %i\n", listbox->flags.scrollDelta);
	
	paneScroll(listbox->pane, -listbox->flags.scrollDelta);
	scrollbarCalcRange(listbox);
	return 1;
}

int listboxGetFocus (TLISTBOX *listbox)
{
	int xOffset, yOffset = 0;
	paneScrollGet(listbox->pane, &xOffset, &yOffset);
	return yOffset;
}

int listboxSetFocus (TLISTBOX *listbox, const int nPixels)
{
	int xOffset;
	paneScrollGet(listbox->pane, &xOffset, NULL);
	paneScrollSet(listbox->pane, xOffset, nPixels, PANE_INVALIDATE);
	scrollbarSetFirstItem(listbox->scrollbar.vert, abs(nPixels));
	scrollbarCalcRange(listbox);
	
	//printf("listboxSetFocus %i\n", nPixels);
	
	return 1;
}

void listboxDelete (void *object)
{
	TLISTBOX *listbox = (TLISTBOX*)object;

	ccDelete(listbox->scrollbar.vert);
	ccDelete(listbox->pane);
}

static inline int64_t listbox_pane_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg == CC_MSG_INPUT || msg == CC_MSG_HOVER || msg == CC_MSG_RENDER) return 1;
	
	TPANE *pane = (TPANE*)object;
	//printf("listbox_pane_cb in %i %I64d %I64X %p\n", msg, data1, data2, dataPtr);

	if (msg == PANE_MSG_TEXT_SELECTED){
		TLISTBOX *listbox = (TLISTBOX*)ccGetUserData(pane);
		ccSendMessage(listbox, LISTBOX_MSG_ITEMSELECTED, data1, data2, NULL);
	
	}else if (msg == PANE_MSG_IMAGE_SELECTED){
		TLISTBOX *listbox = (TLISTBOX*)ccGetUserData(pane);
		ccSendMessage(listbox, LISTBOX_MSG_ITEMSELECTED, data1+1, data2, NULL);
			
	}else if (msg == PANE_MSG_VALIDATED){
		TLISTBOX *listbox = (TLISTBOX*)ccGetUserData(pane);
		scrollbarSetFirstItem(listbox->scrollbar.vert, abs(pane->offset.y));
		scrollbarCalcRange(listbox);
		
		const int sbState = ccGetState(listbox->scrollbar.vert);
		//printf("listbox PANE_MSG_VALIDATED %i\n", sbState);
		if (!sbState)
			paneSwipeDisable(listbox->pane);
		else
			paneSwipeEnable(listbox->pane);
			
		//ccSendMessage(listbox, LISTBOX_MSG_VALIDATED, listboxGetTotalItems(listbox), sbState, NULL);
	}
	
	return 1;
}

static inline int64_t listbox_scrollbar_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg == CC_MSG_INPUT || msg == CC_MSG_HOVER || msg == CC_MSG_RENDER) return 1;

	
	//printf("search_scrollbar_cb. msg:%i, data1:%I64d, data2:%I64d,  ptr:%p\n", msg, data1, data2, dataPtr);
	
	if (msg == SCROLLBAR_MSG_VALCHANGED){
		TPANE *pane = ccGetUserData((void*)object);

		int64_t val = *(int64_t*)dataPtr;
		paneScrollSet(pane, 0, -val, PANE_INVALIDATE);
	}
	return 1;
}

int listboxNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t lb_cb, int *id, const int width, const int height)
{
	TLISTBOX *listbox = (TLISTBOX*)object;
	

	if (id) *id = listbox->id;
	listbox->type = type;

	listbox->cb.msg = lb_cb;
	listbox->cb.render = listboxRender;
	listbox->cb.create = listboxNew;
	listbox->cb.free = listboxDelete;
	listbox->cb.enable = listboxEnable;
	listbox->cb.disable = listboxDisable;
	listbox->cb.input = listboxHandleInput;
	listbox->cb.setPosition = listboxSetPosition;
	listbox->cb.setMetrics = listboxSetMetrics;

	listbox->metrics.x = 0;
	listbox->metrics.y = 0;
	listbox->metrics.width = width;
	listbox->metrics.height = height;
	

	listbox->flags.font = (char*)LISTBOX_FONT_DEFAULT;
	listbox->flags.lineHeight = LISTBOX_LINEHEIGHT_DEFAULT;
	listbox->flags.artMaxHeight = listbox->cc->dheight/1.083;
	listbox->flags.imgScale = (double)listbox->flags.lineHeight / (double)listbox->flags.artMaxHeight;
	listbox->flags.scrollDelta = listbox->flags.lineHeight * 1;
	
	listbox->underlay.imgId = 0;
	listbox->underlay.drawShadow = 0;
	listbox->underlay.colour = SHADOW_BLACK;
	listbox->underlay.offset.x = 0;
	listbox->underlay.offset.y = 20;

	TSCROLLBAR *scrollbar = ccCreate(listbox->cc, pageOwner, CC_SCROLLBAR_VERTICAL, listbox_scrollbar_cb, NULL, SCROLLBAR_VERTWIDTH_DEFAULT, height-2);
	listbox->scrollbar.vert = scrollbar;
	scrollbar->isChild = 0;
	scrollbar->flags.drawBlur = 0;
	scrollbar->flags.drawBase = 0;
	scrollbar->flags.drawFrame = 0;
	listbox->scrollbar.location = 0;
	listbox->scrollbar.offset = 25;
	listbox->scrollbar.draw = 1;
	listbox->scrollbar.width = SCROLLBAR_VERTWIDTH_DEFAULT;
	ccSetMetrics(scrollbar, 0, 0, listbox->scrollbar.width, height-2);
	ccEnable(scrollbar);


	TPANE *pane = ccCreateEx(listbox->cc, pageOwner, CC_PANE, listbox_pane_cb, NULL, width, height, listbox);
	listbox->pane = pane;
	ccSetUserData(scrollbar, pane);
	paneSetLayout(pane, PANE_LAYOUT_VERTCENTER);
	paneSetAcceleration(pane, 0.0, PANE_ACCELERATION_Y);
	paneTextMulityLineDisable(pane);
	paneTextWordwrapDisable(pane);
	paneDragDisable(pane);
	labelRenderFlagsSet(listboxGetBase(listbox), LABEL_RENDER_HOVER_OBJ|LABEL_RENDER_IMAGE|LABEL_RENDER_TEXT);
	
	pane->flags.readAhead.enabled = 0;
	pane->flags.readAhead.number = 3 * 4;
	pane->layout.horiColumnSpace = 8;
	pane->layout.metrics.vertLineHeight = listbox->flags.lineHeight;
	
	listbox->highlighted.itemId = 0;
	listbox->highlighted.colourPre.fore = 255<<24|COL_WHITE;
	listbox->highlighted.colourPre.back = 255<<24|COL_BLACK;
	listbox->highlighted.colourPre.outline = 177<<24|COL_BLUE_SEA_TINT;
	listbox->highlighted.colour.fore = (0xFF<<24)|COL_GREEN_TINT;
	listbox->highlighted.colour.back = 255<<24|COL_BLACK;
	listbox->highlighted.colour.outline = 177<<24|COL_BLUE_SEA_TINT;

	return 1;
}

