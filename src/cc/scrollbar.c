
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


void scrollbarBaseEnable (TSCROLLBAR *scrollbar)
{
	if (ccLock(scrollbar)){
		buttonsStateSet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE, 1);
		ccUnlock(scrollbar);
	}
}

void scrollbarBaseDisable (TSCROLLBAR *scrollbar)
{
	if (ccLock(scrollbar)){
		buttonsStateSet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE, 0);
		ccUnlock(scrollbar);
	}
}

static inline int _getSBarThumbPos (const TTOUCHCOORD *pos, const double sbarHeight, const int tObjects, const double tLines)
{
	const double tabH = (sbarHeight / (double)((tObjects/tLines)))/2.0;
	return (tObjects / sbarHeight) * (double)(pos->y - tabH);
}

int64_t scrollbarGetFirstItem (TSCROLLBAR *scrollbar)
{
	int64_t ret = 0;
	if (ccLock(scrollbar)){
		ret = scrollbar->firstItem;
		ccUnlock(scrollbar);
	}
	return ret;	
}

int64_t scrollbarSetFirstItem (TSCROLLBAR *scrollbar, int64_t item)
{
	//printf("scrollbarSetFirstItem %I64d\n", item);
	
	if (item < scrollbar->rangeMin)
		item = scrollbar->rangeMin;
	else if (item > scrollbar->rangeMax - scrollbar->displayedItems)
		item = scrollbar->rangeMax - scrollbar->displayedItems;
	scrollbar->firstItem = item;
	
	//double fvalue = (item - scrollbar->rangeMin) / (double)(scrollbar->rangeMax - scrollbar->rangeMin);
	//scrollbarSetValueFloat(scrollbar, fvalue);
	
	return item;
}		


static inline int64_t ccbtn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER || msg == CC_MSG_INPUT/* || msg == CC_MSG_SETPOSITION*/) return 1;
		
	TCCBUTTON *btn = (TCCBUTTON*)object;
	//printf("sb ccbtn_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", btn->id, btn->type, msg, (int)data1, (int)data2, dataPtr);
	//const int id = (int)data2;

	if (msg == BUTTON_MSG_SELECTED_PRESS || msg == BUTTON_MSG_SELECTED_SLIDE){
		TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		if (pos){
			TSCROLLBAR *scrollbar = ccGetUserData(btn);
			int64_t item = _getSBarThumbPos(pos, scrollbar->metrics.height, scrollbar->rangeMax, scrollbar->metrics.height);
			scrollbarSetFirstItem(scrollbar, item);
	  		ccSendMessage(scrollbar, SCROLLBAR_MSG_VALCHANGED, scrollbar->id, (pos->x&0xFFFF)<<16|(pos->y&0xFFFF), &item);
	  	}
	}
	return 1;
}

int scrollbarGetCursorPosition (TSCROLLBAR *scrollbar, int *x, int *y)
{
	if (x) *x = scrollbar->cursor.x;
	if (y) *y = scrollbar->cursor.y;
	return (scrollbar->cursor.x<<16)|(scrollbar->cursor.y&0xFFFF);
}

void scrollbarSetCursorPosition (TSCROLLBAR *scrollbar, int x, int y)
{
	if (x <= 0) x = 0;
	else if (x > ccGetWidth(scrollbar)-1)
		x = ccGetWidth(scrollbar)-1;
	if (y <= 0) y = 0;
	else if (y > ccGetHeight(scrollbar)-1)
		y = ccGetHeight(scrollbar)-1;
				
	scrollbar->cursor.x = x;
	scrollbar->cursor.y = y;
}

void _scrollbarEnable (void *object)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	scrollbar->enabled = 1;
}

void _scrollbarDisable (void *object)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	scrollbar->enabled = 0;
}

static inline void _drawScrollBar (TSCROLLBAR *scrollbar, TFRAME *des, const int tRows, const int tObjects, const int firstObject)
{
	double tabH = scrollbar->metrics.height / (double)((tObjects/(double)tRows));
	if (tabH < 6.0)
		tabH = 6.0;
	else if (tabH > scrollbar->metrics.height-1)
		tabH = scrollbar->metrics.height-1;

	double tabY;	
	if (firstObject > 0)
		tabY = scrollbar->metrics.y + ((scrollbar->metrics.height / (double)tObjects) * firstObject);
	else
		tabY = scrollbar->metrics.y;

	tabH -= 0.75;
	if (tabY+tabH > scrollbar->metrics.y + scrollbar->metrics.height)
		tabY -= abs((tabY+tabH) - (scrollbar->metrics.y+scrollbar->metrics.height));
		
	lDrawRectangleFilled(des, scrollbar->metrics.x+1, tabY+1.0, scrollbar->metrics.x+scrollbar->metrics.width-1, (tabY+tabH)-1.0, (190<<24)|COL_BLUE_SEA_TINT);
	lDrawRectangle(des, scrollbar->metrics.x, tabY, scrollbar->metrics.x+scrollbar->metrics.width, tabY+tabH, (160<<24)|0xFFFFFF);
	lDrawRectangle(des, scrollbar->metrics.x+1, tabY+1, scrollbar->metrics.x+scrollbar->metrics.width-1, tabY+tabH-1, (160<<24)|0x000000);
	
	if (tabH > 8.0){
		lDrawLine(des, scrollbar->metrics.x+2, tabY+(tabH/2.0)-1, scrollbar->metrics.x+scrollbar->metrics.width-2, tabY+(tabH/2.0)-1, (127<<24)|0xFFFFFF);
		lDrawLine(des, scrollbar->metrics.x+2, tabY+(tabH/2.0)+1, scrollbar->metrics.x+scrollbar->metrics.width-2, tabY+(tabH/2.0)+1, (127<<24)|0xFFFFFF);
	}else{
		lDrawLine(des, scrollbar->metrics.x+2, tabY+(tabH/2.0), scrollbar->metrics.x+scrollbar->metrics.width-2, tabY+(tabH/2.0), (127<<24)|0xFFFFFF);
	}



#if (DRAWTOUCHRECTS)
	lDrawRectangle(des, scrollbar->metrics.x, tabY, scrollbar->metrics.x+scrollbar->metrics.width, tabY+tabH, DRAWTOUCHRECTCOL);
#endif
}

int _scrollbarHandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	//printf("_scrollbarHandleInput %i %i %i, %i %i %i\n", pos->x, pos->y, ccGetPositionY(scrollbar), flags, scrollbar->inputId, pos->id);

	int ret = 0;
	
	if (scrollbar->enabled){
		if (!flags || (flags && scrollbar->inputId == pos->id)){
			TCCBUTTON *btn = buttonsButtonGet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE);
			ret = ccHandleInput(btn, pos, flags);
			if (ret > 0){
				if (!flags) scrollbar->inputId = pos->id;
				scrollbarSetCursorPosition(scrollbar, pos->x - ccGetPositionX(scrollbar), pos->y - ccGetPositionY(scrollbar));
			}
		}
	}
	return ret;
}

void scrollbarSetRange (TSCROLLBAR *scrollbar, const int64_t min, const int64_t max, const int64_t first, const int64_t viewable)
{
	//printf("scrollbarSetRange min:%I64d max:%I64d first:%I64d\n", min, max, first);
	
	scrollbar->rangeMin = min;
	scrollbar->rangeMax = max;
	scrollbar->firstItem = first;
	scrollbar->displayedItems = viewable;
	scrollbar->scalePos = 0.0;
//	printf("%I64d %I64d %I64d %I64d\n", min, max, first, viewable);
}
/*
double scrollbarSetValueFloat (TSCROLLBAR *scrollbar, const double value)
{
	if (value > 1.0)
		scrollbar->scalePos = 1.0;
	else if (value < 0.0)
		scrollbar->scalePos = 0.0;
	else
		scrollbar->scalePos = value;
	
	int64_t item = (double)(scrollbar->rangeMax - scrollbar->rangeMin) * value;
	item += scrollbar->rangeMin;
	
	if (item < scrollbar->rangeMin)
		item = scrollbar->rangeMin;
	else if (item > scrollbar->rangeMax - scrollbar->displayedItems)
		item = scrollbar->rangeMax - scrollbar->displayedItems;
	printf("scrollbarSetValueFloat %f %I64d\n", value, item);
	//scrollbar->firstItem = item;
	return scrollbar->scalePos;
}
*/
int _scrollbarSetPosition (void *object, const int x, const int y)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	scrollbar->metrics.x = x;
	scrollbar->metrics.y = y;
	buttonsPosSet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE, x, y);
	return 1;
}

int _scrollbarSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;

	TCCBUTTON *button = buttonsButtonGet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE);

	TFRAME *img = buttonFaceImageGet(button);
	lResizeFrame(img, width, height, 0);
	ccSetMetrics(button, x, y, width, height);
		
	scrollbar->metrics.width = width;
	scrollbar->metrics.height = height;

	return 1;
}

int _scrollbarRender (void *object, TFRAME *frame)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	//buttonsRender(scrollbar->buttons, frame);
	
	if (scrollbar->flags.drawBlur)
		lBlurArea(frame, scrollbar->metrics.x-1, scrollbar->metrics.y-1, scrollbar->metrics.x+scrollbar->metrics.width+1, scrollbar->metrics.y+scrollbar->metrics.height+1, 2);
		
	if (scrollbar->flags.drawBase)
		lDrawRectangleFilled(frame, scrollbar->metrics.x-2, scrollbar->metrics.y-2, scrollbar->metrics.x+scrollbar->metrics.width+2, scrollbar->metrics.y+scrollbar->metrics.height+2, (60<<24)|COL_BLUE_SEA_TINT);
		
	if (scrollbar->flags.drawFrame)
		lDrawRectangle(frame, scrollbar->metrics.x-1, scrollbar->metrics.y-1, scrollbar->metrics.x+scrollbar->metrics.width+1, scrollbar->metrics.y+scrollbar->metrics.height/*+1*/, (255<<24)|COL_BLUE_SEA_TINT);
		
	_drawScrollBar(scrollbar, frame,
		  scrollbar->displayedItems,	// number of displayable items that have been rendered
		  scrollbar->rangeMax,			// number of items available to render
		  scrollbar->firstItem);		// first item to render
	
	
	buttonsRenderAll(scrollbar->ccbtns, frame, 0);

#if (DRAWTOUCHRECTS)
	TCCBUTTON *btn = buttonsButtonGet(scrollbar->ccbtns, 0);
	lDrawRectangle(frame, btn->metrics.x, btn->metrics.y, btn->metrics.x+btn->metrics.width, btn->metrics.y+btn->metrics.height, DRAWTOUCHRECTCOL);
	lDrawRectangle(frame, scrollbar->metrics.x-1, scrollbar->metrics.y-1, scrollbar->metrics.x+scrollbar->metrics.width+1, scrollbar->metrics.y+scrollbar->metrics.height, DRAWTOUCHRECTCOL);
#endif	
	return 1;
}

void _scrollbarDelete (void *object)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;
	if (scrollbar->ccbtns)
		buttonsDeleteAll(scrollbar->ccbtns);
	scrollbar->ccbtns = NULL;
}

int scrollbarNew (TCCOBJECT *object, void *unused, const int pageOwner, const int scrollbarType, const TCommonCrtlCbMsg_t scrollbar_cb, int *id, const int width, const int height)
{
	TSCROLLBAR *scrollbar = (TSCROLLBAR*)object;

	scrollbar->pageOwner = pageOwner;
	if (id) *id = scrollbar->id;
	scrollbar->type = scrollbarType;
	
	scrollbar->cb.msg = scrollbar_cb;
	scrollbar->cb.render = _scrollbarRender;
	scrollbar->cb.create = scrollbarNew;
	scrollbar->cb.free = _scrollbarDelete;
	scrollbar->cb.enable = _scrollbarEnable;
	scrollbar->cb.disable = _scrollbarDisable;
	scrollbar->cb.input = _scrollbarHandleInput;
	scrollbar->cb.setPosition = _scrollbarSetPosition;
	scrollbar->cb.setMetrics = _scrollbarSetMetrics;
		
	scrollbar->canDrag = 1;
	scrollbar->rangeMin = 0;
	scrollbar->rangeMax = height;
	scrollbar->firstItem = 0;
	scrollbar->displayedItems = 1;
	scrollbar->cursor.x = -1;
	scrollbar->cursor.y = -1;
	scrollbar->inputId = -1;
	scrollbar->metrics.x = -1;
	scrollbar->metrics.y = -1;
	scrollbar->metrics.width = width;
	scrollbar->metrics.height = height;
	
	scrollbar->flags.drawBlur = 1;
	scrollbar->flags.drawBase = 1;
	scrollbar->flags.drawFrame = 1;
	
	scrollbar->scalePos = 0.0;
	scrollbar->ccbtns = buttonsCreate(scrollbar->cc, scrollbar->pageOwner, SCROLLBAR_BTN_TOTAL, ccbtn_cb);
	TCCBUTTON *btn = buttonsButtonGet(scrollbar->ccbtns, SCROLLBAR_BTN_BASE);
	
	ccSetUserData(btn, scrollbar);
	btn->flags.disableRender = 1;
	btn->isChild = 1;
	btn->canDrag = 1;
		
	// to enable input callbacks a renderless button face must be applied
	TFRAME *img = lNewFrame(scrollbar->cc->hSurfaceLib, width, height, LFRM_BPP_24);
	if (img){
		buttonFaceImageSet(btn, img, NULL, 0, 0);
		lDeleteFrame(img);
	}
	
	buttonFaceActiveSet(btn, BUTTON_PRI);	
	ccSetMetrics(btn, 0, 0, width, height);		
	ccEnable(btn);

	return 1;
}
