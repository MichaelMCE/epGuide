
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


extern volatile double UPDATERATE_BASE;



void buttonDataSet (TCCBUTTON *btn, int64_t data)
{
	if (ccLock(btn)){
		btn->dataInt64 = data;
		ccUnlock(btn);
	}
}

int64_t buttonDataGet (TCCBUTTON *btn)
{
	int64_t data = 0;
	if (ccLock(btn)){
		data = btn->dataInt64;
		ccUnlock(btn);
	}
	return data;
}

int ccButtonGetXWidth (TCCBUTTON *btn)
{
	return ccGetPositionX(btn) + ccGetWidth(btn);
}

int ccButtonGetXHeight (TCCBUTTON *btn)
{
	return ccGetPositionY(btn) + ccGetHeight(btn);
}

static inline int buttonSetActive (TCCBUTTON *button, const int idx)
{
	//printf("buttonSetActive %i, %i\n", button->id, idx);
	if (idx == BUTTON_PRI || idx == BUTTON_SEC){
		if (button->btnlbl[idx]){
			if (button->btnlbl[idx]->itemId){
				button->active = button->btnlbl[idx];
				//printf("active = %i\n", idx);
				return button->btnlbl[idx]->itemId;
			
			}else if (button->btnlbl[idx]->itemStrId){
				button->active = button->btnlbl[idx];
				//printf("active = %i\n", idx);
				return button->btnlbl[idx]->itemStrId;
			}
		}
		
	}
	return 0;
}

int buttonFaceActiveSet (TCCBUTTON *button, const int idx)
{
	//printf("buttonFaceActiveSet %i, %i\n", button->id, idx);
	int ret = 0;
	if (ccLock(button)){
		ret = buttonSetActive(button, idx);
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceActiveGet (TCCBUTTON *button)
{
	int ret = -1;
	if (ccLock(button)){
		if (button->active){
			if (button->active == button->btnlbl[BUTTON_PRI])
				ret = BUTTON_PRI;
			else if (button->active == button->btnlbl[BUTTON_SEC])
				ret = BUTTON_SEC;
		}
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceAutoSwapGet (TCCBUTTON *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.autoFaceSwap;
		ccUnlock(button);
	}
	return ret;
}

void buttonFaceAutoSwapEnable (TCCBUTTON *button, const int mode)
{
	if (ccLock(button)){
		button->flags.autoFaceSwap = mode;
		ccUnlock(button);
	}
}

void buttonFaceAutoSwapDisable (TCCBUTTON *button)
{
	if (ccLock(button)){
		button->flags.autoFaceSwap = 0;
		ccUnlock(button);
	}
}

void (CALLBACK timerBtnSetPri)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	//printf("@@@ timerBtnSetPri\n");

	TCCBUTTON *btn = (TCCBUTTON*)dwUser;
	btn->timerId = 0;
	buttonSetActive(btn, BUTTON_PRI);
	
	com_renderSignalUpdate(btn->cc);
}

static inline int64_t btn_label_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER/* || msg == CC_MSG_INPUT*/) return 1;
	
	TCCOBJECT *obj = (TCCOBJECT*)object;
	//printf("btn_label_cb. id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);
	

	TLABEL *label = (TLABEL*)obj;
	TCCBUTTON *btn = (TCCBUTTON*)ccGetUserData(label);
			
	switch (msg){
	  case CC_MSG_INPUT:
	  	ccSendMessage(btn, CC_MSG_INPUT, data1, data2, dataPtr);
	  	break;
	  
	  case LABEL_MSG_BASE_SELECTED_PRESS:
		if (btn->flags.disableRender)	// simulate a response if there is no child item or it can not be rendered (is disabled)
			ccSendMessage(btn, BUTTON_MSG_SELECTED_PRESS, data1, btn->active->itemId, dataPtr);
	  	break;
	  case LABEL_MSG_BASE_SELECTED_SLIDE:
	  	if (btn->flags.disableRender && (label->canDrag || btn->canDrag))
			ccSendMessage(btn, BUTTON_MSG_SELECTED_SLIDE, data1, btn->active->itemId, dataPtr);
	  	break;
	  case LABEL_MSG_BASE_SELECTED_RELEASE:
	  	if (btn->flags.disableRender)
			ccSendMessage(btn, BUTTON_MSG_SELECTED_RELEASE, data1, btn->active->itemId, dataPtr);
	  	break;

	  case LABEL_MSG_TEXT_SELECTED_PRESS:{
	  	//TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
	  	//if (pos->pen != 0 || pos->dt < 120) break;	// continue only if its a press/pen down and not a slide/drag
	  }
	  case LABEL_MSG_IMAGE_SELECTED_PRESS:{
  		if (btn->flags.canAnimate){
	  		btn->zoom.direction = 1;
  			btn->zoom.scaleBy = 1.0;
  			com_setTargetRate(btn->cc, btn->zoom.updateRate);
  		}

		if (btn->flags.autoFaceSwap == BUTTON_FACE_SWAP_SELECTED){
  			TBUTTONLABEL *bl = btn->btnlbl[BUTTON_SEC];
  			if (bl){
  				if (btn->timerId) timeKillEvent(btn->timerId);
  				buttonSetActive(btn, BUTTON_SEC);
  				//printf("btn_label_cb timeSetEvent %p %i\n", btn, btn->id);
  				btn->timerId = (int)timeSetEvent(HIGHLIGHTPERIOD, 20, timerBtnSetPri, (DWORD_PTR)btn, TIME_ONESHOT);
  			}
  		}

	  	//printf("btn_label_cb. id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);
	  	ccSendMessage(btn, BUTTON_MSG_SELECTED_PRESS, data1, data2, dataPtr);
	  	break;
	  }
	  case LABEL_MSG_IMAGE_SELECTED_SLIDE:
	  	if (btn->flags.acceptDrag)
	  		ccSendMessage(btn, BUTTON_MSG_SELECTED_SLIDE, data1, data2, dataPtr);
	  	break;
	  case LABEL_MSG_IMAGE_SELECTED_RELEASE:
	  	ccSendMessage(btn, BUTTON_MSG_SELECTED_RELEASE, data1, data2, dataPtr);
	  	break;
	}
	return 1;
}

static inline void buttonSetTextColour (TCCBUTTON *button, const int idx, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (bl)
		labelRenderColourSet(bl->label, bl->itemStrId, fore, back, outline);
}

static inline int buttonUpdateText (TCCBUTTON *button, const int idx, const char *str)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (bl)
		return labelStringSet(bl->label, bl->itemStrId, str);
	return 0;
}

static inline int buttonTextMaxWidthSet (TCCBUTTON *button, const int idx, const int width)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (bl)
		return labelStringSetMaxWidth(bl->label, bl->itemStrId, width);
	return 0;
}

static inline void buttonTextSetFlags (TCCBUTTON *button, const int idx, const int flags)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (bl)
		labelStringRenderFlagsSet(bl->label, bl->itemStrId, flags);
}

static inline int buttonSetText (TCCBUTTON *button, const int idx, const char *str, const int flags, const char *font, const int x, const int y)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (bl){
		bl->itemStrId = labelTextCreate(bl->label, str, flags, font, x, y/*+2*/);
		labelRenderFlagsSet(bl->label, labelRenderFlagsGet(bl->label)|LABEL_RENDER_TEXT);
		return bl->itemStrId;
	}
	return 0;
}

static inline int buttonFaceSetConstraints (TCCBUTTON *button, const int x1, const int y1, const int x2, const int y2)
{
	int ret = 0;
	TBUTTONLABEL *bl = button->btnlbl[BUTTON_PRI];
	if (bl){
		if (!bl->imgId){
			TLABELIMAGE *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				//img->hasConstraints = 1;
				img->constraint.x1 = x1;
				img->constraint.y1 = y1;
				img->constraint.x2 = x2;
				img->constraint.y2 = y2;
				ret = 1;
			}
		}else{
			TLABELIMGCA *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				//img->hasConstraints = 1;
				img->constraint.x1 = x1;
				img->constraint.y1 = y1;
				img->constraint.x2 = x2;
				img->constraint.y2 = y2;
				ret = 1;
			}
		}
	}
	
	bl = button->btnlbl[BUTTON_SEC];
	if (bl){
		if (!bl->imgId){
			TLABELIMAGE *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				//img->hasConstraints = 1;
				img->constraint.x1 = x1;
				img->constraint.y1 = y1;
				img->constraint.x2 = x2;
				img->constraint.y2 = y2;
				ret++;
			}
		}else{
			TLABELIMGCA *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				//img->hasConstraints = 1;
				img->constraint.x1 = x1;
				img->constraint.y1 = y1;
				img->constraint.x2 = x2;
				img->constraint.y2 = y2;
				ret = 1;
			}
		}
		
	}
	return ret;
}

int buttonFaceConstraintsSet (TCCBUTTON *button, const int x1, const int y1, const int x2, const int y2)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonFaceSetConstraints(button, x1, y1, x2, y2);
		ccUnlock(button);
	}
	return ret;
}

static inline int buttonFaceConstraintsSetState (TCCBUTTON *button, const int state)
{
	int ret = 0;
	TBUTTONLABEL *bl = button->btnlbl[BUTTON_PRI];
	if (bl){
		if (!bl->imgId){
			TLABELIMAGE *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				img->hasConstraints = state;
				ret = 1;
			}
		}else{
			TLABELIMGCA *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				img->hasConstraints = state;
				ret = 1;
			}
		}
	}
	
	bl = button->btnlbl[BUTTON_SEC];
	if (bl){
		if (!bl->imgId){
			TLABELIMAGE *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				img->hasConstraints = state;
				ret++;
			}
		}else{
			TLABELIMGCA *img = labelItemGet(bl->label, bl->itemId);
			if (img){
				img->hasConstraints = state;
				ret = 1;
			}
		}
		
	}
	return ret;
}

int buttonFaceConstraintsEnable (TCCBUTTON *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonFaceConstraintsSetState(button, 1);
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceConstraintsDisable (TCCBUTTON *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonFaceConstraintsSetState(button, 0);
		ccUnlock(button);
	}
	return ret;
}

TFRAME *buttonFaceImageGet (TCCBUTTON *button)
{
	TFRAME *ret = NULL;
	if (ccLock(button)){
		if (button->active){
			if (!button->active->imgId)
				ret = labelImageGetSrc(button->active->label, button->active->itemId);
		}
		ccUnlock(button);
	}

	//printf("@@ buttonFaceImageGet %i %p\n", button->id, ret);

	return ret;
}

void buttonBorderProfileSet (TCCBUTTON *button, const int set, const uint32_t *colour, const int total)
{
	if (ccLock(button)){
		if (button->active)
			labelBorderProfileSet(button->active->label, set, colour, total);
		ccUnlock(button);
	}
}

void buttonBaseColourSet (TCCBUTTON *button, const uint32_t colour)
{
	if (ccLock(button)){
		if (button->active)
			labelBaseColourSet(button->active->label, colour);
		ccUnlock(button);
	}
}

void buttonBaseEnable (TCCBUTTON *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) | LABEL_RENDER_BLUR | LABEL_RENDER_BASE);
		ccUnlock(button);
	}
}

void buttonBaseDisable (TCCBUTTON *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) &~LABEL_RENDER_BLUR &~LABEL_RENDER_BASE);
		ccUnlock(button);
	}
}

void buttonBorderEnable (TCCBUTTON *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label)|LABEL_RENDER_BORDER_POST);
		ccUnlock(button);
	}
}

void buttonBorderDisable (TCCBUTTON *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) &~LABEL_RENDER_BORDER_POST);
		ccUnlock(button);
	}
}

static inline int buttonSetImage (TCCBUTTON *button, const int idx, TFRAME *img, const int x, const int y)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (!bl) return 0;

	if (!bl->itemId)
		bl->itemId = labelImageCreate(bl->label, img, 1, x, y);
	else
		labelImageSet(bl->label, bl->itemId, img, 1);
	return bl->itemId;
}
							
static inline int buttonSetImageId (TCCBUTTON *button, const int idx, const int imgId, const int x, const int y)
{
	TBUTTONLABEL *bl = button->btnlbl[idx];
	if (!bl) return 0;

	if (!bl->itemId)
		bl->itemId = labelImgcCreate(bl->label, imgId, 1, x, y);
	else
		labelImgcSet(bl->label, bl->itemId, imgId, 1);

	bl->imgId = imgId;
	return bl->itemId;
}

int buttonFaceSet (TCCBUTTON *button, const int idx, TFRAME *img, const int x, const int y)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonSetImage(button, idx, img, x, y);
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceSetImgId (TCCBUTTON *button, const int idx, const int imgId, const int x, const int y)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonSetImageId(button, idx, imgId, x, y);
		ccUnlock(button);
	}
	return ret;
}

static inline int buttonCreateFace (TCCBUTTON *button, TFRAME *pri, TFRAME *sec, const int x, const int y)
{
	int ret = 0;
	if (pri)
		ret = buttonSetImage(button, BUTTON_PRI, pri, x, y) & 0xFFFF;
		
	if (sec)
		ret |= buttonSetImage(button, BUTTON_SEC, sec, x, y) << 16;
		
	if (ret){
		ccSetMetrics(button, -1, -1, pri->width, pri->height);
		buttonSetActive(button, BUTTON_PRI);
	}
	return ret;
}

static inline int buttonCreateFaceImgId (TCCBUTTON *button, const int priId, const int secId, const int x, const int y)
{
	int ret = 0;
	
	if (priId)
		ret = buttonSetImageId(button, BUTTON_PRI, priId, x, y) & 0xFFFF;
		
	if (secId)
		ret |= buttonSetImageId(button, BUTTON_SEC, secId, x, y) << 16;
		
	if (ret){
		int width = 0, height = 0;
		void *im = ccGetImageManager(button->cc, CC_IMAGEMANAGER_IMAGE);
		if (!imageManagerImageGetMetrics(im, priId, &width, &height)){
			if (secId && !imageManagerImageGetMetrics(im, secId, &width, &height))
				return 0;
		}
			
		if (width && height){
			ccSetMetrics(button, -1, -1, width, height);
			buttonSetActive(button, BUTTON_PRI);
		}
	}

	return ret;
}

int buttonFacePathSet (TCCBUTTON *btn, wchar_t *pri, wchar_t *sec, const int x, const int y)
{
	int ret = 0;
	
	if (ccLock(btn)){
		int priId = 0;
		int secId = 0;
		void *im = ccGetImageManager(btn->cc, CC_IMAGEMANAGER_IMAGE);
		
		if (pri)
			priId = imageManagerImageAdd(im, pri);

		if (sec)
			secId = imageManagerImageAdd(im, sec);

		if (priId || secId)
			ret = buttonCreateFaceImgId(btn, priId, secId, x, y);

		ccUnlock(btn);
	}
	return ret;
}

int buttonFaceTextSet (TCCBUTTON *button, const int idx, const char *str, const int flags, const char *font, const int x, const int y)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonSetText(button, idx, str, flags, font, x, y);
		
		if (buttonFaceActiveGet(button) == -1)
			buttonFaceActiveSet(button, idx);
		
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceTextUpdate (TCCBUTTON *button, const int idx, const char *str)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonUpdateText(button, idx, str);
		ccUnlock(button);
	}
	return ret;
}

void buttonFaceTextFlagsSet (TCCBUTTON *button, const int idx, const int flags)
{
	if (ccLock(button)){
		buttonTextSetFlags(button, idx, flags);
		ccUnlock(button);
	}
}

int buttonFaceTextMetricsGet (TCCBUTTON *button, const int face, int *x, int *y, int *width, int *height)
{
	int ret = 0;
	if (ccLock(button)){
		TBUTTONLABEL *active = button->btnlbl[face];
		if (active)
			ret = labelStringGetMetrics(active->label, active->itemStrId, x, y, width, height);
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceTextWidthSet (TCCBUTTON *button, const int idx, const int width)
{
	int ret = 0;
	if (ccLock(button)){
		ret = buttonTextMaxWidthSet(button, idx, width);
		ccUnlock(button);
	}
	return ret;
}


void buttonFaceTextColourSet (TCCBUTTON *button, const int face, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	if (ccLock(button)){
		buttonSetTextColour(button, face, fore, back, outline);
		ccUnlock(button);
	}
}

int buttonFaceImageSet (TCCBUTTON *button, TFRAME *pri, TFRAME *sec, const int x, const int y)
{
	int ret = 0;
	if (ccLock(button)){
		TBUTTONLABEL *bl = button->btnlbl[BUTTON_PRI];	// if face already exists then update/overwrite it, else great a (new) face
		if (bl && bl->itemId){
			if (pri)
				ret = buttonSetImage(button, BUTTON_PRI, pri, x, y);
			if (sec)
				ret += buttonSetImage(button, BUTTON_SEC, sec, x, y);
		}else{
			ret = buttonCreateFace(button, pri, sec, x, y);
		}
		ccUnlock(button);
	}
	return ret;
}

int buttonAnimateGet (TCCBUTTON *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canAnimate;
		ccUnlock(button);
	}
	return ret;
}

int buttonAnimateSet (TCCBUTTON *button, const int state)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canAnimate;
		button->flags.canAnimate = state&0x01;
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceHoverGet (TCCBUTTON *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canHover;
		ccUnlock(button);
	}
	return ret;
}

int buttonFaceHoverSet (TCCBUTTON *button, const int state, const uint32_t colour, const double alpha)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canHover;
		button->flags.canHover = state&0x01;
		button->hover.colour = colour;
		button->hover.alpha = alpha;
		
		TBUTTONLABEL *bl = button->btnlbl[BUTTON_PRI];
		if (bl){
			if (bl->imgId)
				labelImgcHoverSet(bl->label, bl->itemId, colour, alpha);
			else
				labelImageHoverSet(bl->label, bl->itemId, colour, alpha);
				
			int flags = labelRenderFlagsGet(bl->label);
			if (state)
				flags |= LABEL_RENDER_HOVER_OBJ;
			else
				flags &= ~LABEL_RENDER_HOVER_OBJ;
			labelRenderFlagsSet(bl->label, flags);
			
		}

		bl = button->btnlbl[BUTTON_SEC];
		if (bl){
			if (bl->imgId)
				labelImgcHoverSet(bl->label, bl->itemId, colour, alpha);
			else
				labelImageHoverSet(bl->label, bl->itemId, colour, alpha);
				
			int flags = labelRenderFlagsGet(bl->label);
			if (state)
				flags |= LABEL_RENDER_HOVER_OBJ;
			else
				flags &= ~LABEL_RENDER_HOVER_OBJ;
			labelRenderFlagsSet(bl->label, flags);
		}
		ccUnlock(button);
	}
	return ret;
}

TCCBUTTON *buttonsCreateButton (TCCBUTTONS *btns, wchar_t *pri, wchar_t *sec, const int id, const int initialState, const int canAnimate, const int x, const int y)
{
	TCCBUTTON *btn = buttonsButtonGet(btns, id);
	buttonFacePathSet(btn, pri, sec, 0, 0);
	buttonAnimateSet(btn, canAnimate);
	buttonFaceHoverSet(btn, 1, COL_HOVER, 0.8);
	ccSetMetrics(btn, x, y, -1, -1);
	
	if (initialState) ccEnable(btn);
	return btn;
}

TCCBUTTONS *buttonsCreate (TCC *cc, const int pageOwner, const int total, const TCommonCrtlCbMsg_t btn_cb)
{
	TCCBUTTONS *btns = my_malloc(sizeof(TCCBUTTONS));
	if (btns){
		btns->list = my_calloc(total, sizeof(TCCBUTTON*));
		if (btns->list){
			btns->total = total;
			btns->pageId = pageOwner;
			btns->t0 = 0;

			for (int i = 0; i < total; i++){
				btns->list[i] = ccCreate(cc, pageOwner, CC_BUTTON, btn_cb, NULL, 0, 0);
				if (btns->list[i]){
					//printf("buttonsCreate %i: %i %i %i\n", i, pageOwner, btns->list[i]->type, btns->list[i]->id);
					ccSetUserData(btns->list[i], btns);
					ccSetUserDataInt(btns->list[i], i);
				}
			}
			return btns;
		}
		my_free(btns);
	}
	return NULL;
}

int buttonsTotalSet (TCCBUTTONS *btns, const int total, const TCommonCrtlCbMsg_t btn_cb)
{
	if (!total || total == btns->total){
		return total;
		
	}else if (total < btns->total){
		for (int i = total; i < btns->total; i++){
			TCCBUTTON *btn = buttonsButtonGet(btns, i);
			if (btn) ccDelete(btn);
		}
		
		btns->list = my_realloc(btns->list, total * sizeof(TCCBUTTONS*));
		btns->total = total;
		return total;
		
	}else if (total > btns->total){
		TCCBUTTON *btn = buttonsButtonGet(btns, 0);
		if (!btn) return 0;

		btns->list = my_realloc(btns->list, total * sizeof(TCCBUTTONS*));
		for (int i = btns->total; i < total; i++){
			btns->list[i] = ccCreate(btn->cc, btn->pageOwner, CC_BUTTON, btn_cb, NULL, 0, 0);
			if (btns->list[i])
				ccSetUserDataInt(btns->list[i], i);
		}
		
		btns->total = total;
		return total;
	}else if (total < 0){
		return -1;
	}
	
	return 0;
}

TCCBUTTON *buttonsButtonGet (TCCBUTTONS *btns, const int id)
{
	if (id < btns->total)
		return btns->list[id];
	else
		return NULL;
}

int buttonsWidthGet (TCCBUTTONS *btns, const int id)
{
	return ccGetWidth(buttonsButtonGet(btns, id));
}

int buttonsHeightGet (TCCBUTTONS *btns, const int id)
{
	return ccGetHeight(buttonsButtonGet(btns, id));
}

int buttonsPosXGet (TCCBUTTONS *btns, const int id)
{
	return ccGetPositionX(buttonsButtonGet(btns, id));
}

int buttonsPosYGet (TCCBUTTONS *btns, const int id)
{
	return ccGetPositionY(buttonsButtonGet(btns, id));
}

void buttonsPosSet (TCCBUTTONS *btns, const int id, const int x, const int y)
{
	ccSetPosition(buttonsButtonGet(btns, id), x, y);
}

void buttonsMetricsSet (TCCBUTTONS *btns, const int id, const int x, const int y, const int width, const int height)
{
	ccSetMetrics(buttonsButtonGet(btns, id), x, y, width, height);
}

int buttonsMetricsGet (TCCBUTTONS *btns, const int id, int *x, int *y, int *width, int *height)
{
	if (!btns) return 0;

	int ret = 0;
	TCCBUTTON *btn = buttonsButtonGet(btns, id);
	if (btn){
		if (ccLock(btn)){
			if (x) *x = btn->metrics.x;
			if (y) *y = btn->metrics.y;
			if (width) *width = btn->metrics.width;
			if (height) *height = btn->metrics.height;
			ret = 1;
			ccUnlock(btn);
		}
	}
	return ret;
}

int buttonsTotalGet (TCCBUTTONS *btns)
{
	if (btns)
		return btns->total;
	else
		return 0;
}

int buttonsButtonToIdx (TCCBUTTONS *btns, TCCBUTTON *btn)
{
	if (!btns || !btn) return -1;

	const int total = buttonsTotalGet(btns);
	for (int i = 0; i < total; i++){
		if (buttonsButtonGet(btns, i) == btn)
			return i;
	}
	return -1;
}

void buttonsStateSet (TCCBUTTONS *btns, const int id, const int state)
{
	if (!btns) return;
	
	//printf("buttonSetState %i %i\n", id, state);
	if (state)
		ccEnable(buttonsButtonGet(btns, id));
	else
		ccDisable(buttonsButtonGet(btns, id));
}

int buttonsStateGet (TCCBUTTONS *btns, const int id)
{
	if (!btns) return -1;
	
	return ccGetState(buttonsButtonGet(btns, id));
}

void buttonsSetState (TCCBUTTONS *btns, const int state)
{
	if (!btns) return;
	
	if (state){
		for (int i = 0; i < btns->total; i++)
			ccEnable(buttonsButtonGet(btns, i));
	}else{
		for (int i = 0; i < btns->total; i++)
			ccDisable(buttonsButtonGet(btns, i));
	}
}

void buttonsDeleteAll (TCCBUTTONS *btns)
{
	if (!btns) return;
	
	for (int i = 0; i < btns->total; i++){
		TCCBUTTON *btn = buttonsButtonGet(btns, i);
		if (btn) ccDelete(btn);
	}
	
	my_free(btns->list);
	my_free(btns);
}

uint32_t buttonsRenderAll (TCCBUTTONS *btns, TFRAME *frame, const int flags)
{
	if (!btns || !btns->total) return 0;

	TCCBUTTON *_btn = NULL;
	int ct = 0;
	
	for (int i = 0; i < btns->total; i++){
		TCCBUTTON *btn = buttonsButtonGet(btns, i);
		if (btn){
			_btn = btn;
			
			const int canAnimate = btn->flags.canAnimate;
			const int canHover = btn->flags.canHover;
			
			if (!(flags&BUTTONS_RENDER_ANIMATE))
				btn->flags.canAnimate = 0;

			if (!(flags&BUTTONS_RENDER_HOVER))
				btn->flags.canHover = 0;
			
			ccRender(btn, frame);
			//printf("btnrenderall: %i, %i %i %i %i\n", btn->enabled, btn->metrics.x, btn->metrics.y, btn->metrics.width, btn->metrics.height);
			
			btn->flags.canAnimate = canAnimate;
			btn->flags.canHover = canHover;
			
			ct++;
#if DRAWBUTTONRECTS
			if (ccGetState(btn))
				lDrawRectangle(frame, btn->metrics.x, btn->metrics.y, btn->metrics.x+btn->metrics.width-1, btn->metrics.y+btn->metrics.height-1, DRAWBUTTONRECTCOL);
#endif
		}
	}
	if (!ct) return 0;
	
	const int64_t dt = com_getTickCount() - btns->t0;
	if (dt < UPDATERATE_LENGTH){
		com_setTargetRate(_btn->cc, 25);
	}else{
		//TPAGE2 *page = pageRenderGetTopMostPage(_btn->cc->vp->pages);		
		//if (page && page->updateRate)
		//	setTargetRate(_btn->cc, page->updateRate);
		//else
			com_setTargetRate(_btn->cc, UPDATERATE_BASE);

	}
		
	return dt;
}

uint32_t buttonsRenderAllEx (TCCBUTTONS *btns, TFRAME *frame, const int flags, const int cx, const int cy)
{
	if (!btns || !btns->total) return 0;

	TCCBUTTON *_btn = NULL;
	int ct = 0;
	for (int i = 0; i < btns->total; i++){
		TCCBUTTON *btn = buttonsButtonGet(btns, i);
		if (btn){
			_btn = btn;
			
			const int canAnimate = btn->flags.canAnimate;
			const int canHover = btn->flags.canHover;
			
			if (!(flags&BUTTONS_RENDER_ANIMATE))
				btn->flags.canAnimate = 0;

			if (!(flags&BUTTONS_RENDER_HOVER))
				btn->flags.canHover = 0;
			
			ccRenderEx(btn, frame, cx, cy);
			//printf("btnrenderall: %i %i %i %i\n", btn->metrics.x, btn->metrics.y, btn->metrics.width, btn->metrics.height);
			
			btn->flags.canAnimate = canAnimate;
			btn->flags.canHover = canHover;
			
			ct++;
#if DRAWBUTTONRECTS
			if (ccGetState(btn))
				lDrawRectangle(frame, btn->metrics.x, btn->metrics.y, btn->metrics.x+btn->metrics.width-1, btn->metrics.y+btn->metrics.height-1, DRAWBUTTONRECTCOL);
#endif
		}
	}
	if (!ct) return 0;

	const int64_t dt = com_getTickCount() - btns->t0;
	if (dt < UPDATERATE_LENGTH)
		com_setTargetRate(_btn->cc, 25);
	else
		com_setTargetRate(_btn->cc, UPDATERATE_BASE);

	return dt;
}

/*
#############################################################################################
#############################################################################################
#############################################################################################
#############################################################################################
#############################################################################################
*/

static inline int ccbuttonSetPosition (void *object, const int x, const int y)
{
	//printf("treeviewSetPosition %i %i\n", x, y);
	
	TCCBUTTON *button = (TCCBUTTON*)object;
	button->metrics.x = x;
	button->metrics.y = y;

	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			if (button->btnlbl[i]->itemId || button->btnlbl[i]->itemStrId)
				ccSetPosition(button->btnlbl[i]->label, x, y);
		}
	}

	return 1;
}

static inline int buttonHandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	
	TCCBUTTON *btn = (TCCBUTTON*)object;

	//printf("%p %i %i %i\n",btn->active, btn->active->itemId, btn->active->itemStrId, btn->flags.disableInput);

	if (btn->active && (btn->active->itemId || btn->active->itemStrId) && !btn->flags.disableInput){
		/*if (btn->flags.disableRender){
			const int x1 = ccGetPositionX(btn);
			const int x2 = x1 + ccGetWidth(btn)-1;
			const int y1 = ccGetPositionY(btn);
			const int y2 = y1 + ccGetHeight(btn)-1;

			printf("disabled %i, %i %i %i %i, %i %i\n", btn->id, x1, y1, x2, y2, pos->x, pos->y);
			
			if (pos->y >= y1 && pos->y <= y2){
				if (pos->x >= x1 && pos->x <= x2){
					return ccSendMessage(btn, BUTTON_MSG_SELECTED_PRESS, ((pos->x&0xFFFF)<<16)|(pos->y&0xFFFF), btn->id, pos);
				}
			}
		}*/

		int ret = ccHandleInput(btn->active->label, pos, flags);
		//printf("ishovered %i, %i %i, %p %p %p\n", btn->id, btn->isHovered, btn->active->label->isHovered, btn->active, btn->btnlbl[0], btn->btnlbl[1]);
		//printf("button ret = %i\n", ret);
		return ret;
	}
	return 0;
}

static inline int buttonSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	//printf("tvSetMetrics %p, %i %i %i %i\n",object, x, y, width, height);
	
	TCCBUTTON *button = (TCCBUTTON*)object;
	
	ccSetPosition(button, x, y);
	button->metrics.width = width;
	button->metrics.height = height;

	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			if (button->btnlbl[i]->itemId || button->btnlbl[i]->itemStrId)
				ccSetMetrics(button->btnlbl[i]->label, x, y, width, height);
		}
	}
	
	return 1;
}

static inline void ccbuttonEnable (void *object)
{
	TCCBUTTON *button = (TCCBUTTON*)object;
	button->enabled = 1;
	
	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			if (button->btnlbl[i]->itemId || button->btnlbl[i]->itemStrId)
				ccEnable(button->btnlbl[i]->label);
		}
	}
}

static inline void ccbuttonDisable (void *object)
{	
	TCCBUTTON *button = (TCCBUTTON*)object;
	button->enabled = 0;
	
	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			if (button->btnlbl[i]->itemId || button->btnlbl[i]->itemStrId)
				ccDisable(button->btnlbl[i]->label);
		}
	}
}

static inline int buttonRenderLabel (TCCBUTTON *btn, TLABEL *label, TFRAME *frame)
{
	//printf("buttonRenderLabel, %i %i %i\n", btn->isHovered, label->isHovered, btn->flags.canHover);
	
	if (btn->isHovered && btn->flags.canHover){
		uint32_t flags = labelRenderFlagsGet(label);
		labelRenderFlagsSet(label, flags|LABEL_RENDER_HOVER);
		int ret = ccRender(label, frame);
		labelRenderFlagsSet(label, flags);
		return ret;
	}else{
		return ccRender(label, frame);
	}
}

static inline int ccbuttonRender (void *object, TFRAME *frame)
{
	TCCBUTTON *btn = (TCCBUTTON*)object;

	if (!btn->active)
		return 0;
	else if (btn->flags.disableRender)
		return 1;
		
	//int activeFace = buttonFaceActiveGet(btn);	
	/*int activeFace = -1;
	
	if (btn->isHovered && !buttonFaceAutoSwapGet(btn)){
		activeFace = buttonFaceActiveGet(btn);
		if (activeFace == BUTTON_SEC)
			buttonFaceActiveSet(btn, BUTTON_PRI);
		else if (activeFace == BUTTON_PRI)
			buttonFaceActiveSet(btn, BUTTON_SEC);
	}*/

	TLABEL *label = btn->active->label;

	if (btn->flags.canAnimate && btn->zoom.direction){
		if (btn->zoom.direction == 1){
			btn->zoom.scaleBy -= btn->zoom.scaleRate;
			if (btn->zoom.scaleBy < 0.5)
				btn->zoom.direction = 2;
				
		}else if (btn->zoom.direction == 2){
			btn->zoom.scaleBy += btn->zoom.scaleRate;
			if (btn->zoom.scaleBy >= 1.0){
				btn->zoom.scaleBy = 1.0;		// 1.0 will disable scaling
				btn->zoom.direction = 0;
				
				//TBUTTONLABEL *bl = btn->btnlbl[BUTTON_SEC];
				//printf("buttonSetActive zoom %i\n", btn->id);
	  			//if (bl) buttonSetActive(btn, BUTTON_PRI);
			}
		}
		com_setTargetRate(btn->cc, btn->zoom.updateRate);
		if (btn->active->imgId)
			labelImgcScaleSet(btn->active->label, btn->active->itemId, btn->zoom.scaleBy);
		else
			labelImageScaleSet(btn->active->label, btn->active->itemId, btn->zoom.scaleBy);
	}

#if 1

	int ret = buttonRenderLabel(btn, label, frame);

#else
	//return buttonRenderLabel(btn, label, frame);

	TLABEL *labelP = btn->btnlbl[BUTTON_PRI]->label;

	if (!btn->btnlbl[BUTTON_SEC] || !btn->btnlbl[BUTTON_SEC]->itemId || btn->active->label == labelP || !btn->btnlbl[BUTTON_PRI]->itemStrId){
		int ret = buttonRenderLabel(btn, label, frame);
		if (activeFace != -1)
			buttonFaceActiveSet(btn, activeFace);
		return ret;
	}

	TLABEL *labelS = btn->btnlbl[BUTTON_SEC]->label;
	uint32_t rflags = labelP->renderflags;
	labelP->renderflags = LABEL_RENDER_TEXT;

	buttonRenderLabel(btn, labelS, frame);
	int ret = buttonRenderLabel(btn, labelP, frame);
	labelP->renderflags = rflags;
	
	//if (activeFace != -1)
	//	buttonFaceActiveSet(btn, activeFace);
#endif
	return ret;

}

static inline void ccbuttonDelete (void *object)
{
	TCCBUTTON *button = (TCCBUTTON*)object;

	//printf("ccbuttonDelete in %p %i, %p %p\n", button, button->id, button->btnlbl[0], button->btnlbl[1]);
	
	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			if (button->btnlbl[i]->label){
				//printf("::: ccbuttonDelete %i %p %i\n", i, button->btnlbl[i]->label, button->btnlbl[i]->label->id);
				ccDelete(button->btnlbl[i]->label);
				button->btnlbl[i]->label = NULL;
			}
		}
	}

	for (int i = 0; i < 2; i++){
		if (button->btnlbl[i]){
			my_free(button->btnlbl[i]);
			button->btnlbl[i] = NULL;
		}
	}
}

static inline int64_t btn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER/* || msg == CC_MSG_INPUT*/) return 1;
	
	TCCBUTTON *btn = (TCCBUTTON*)object;
	
	if (msg == CC_MSG_HOVER && data2 == 1){			// hover begun
		if (btn->flags.autoFaceSwap == 1 && btn->flags.canHover){
			//printf("face set %i, %i\n", btn->id, btn->flags.autoFaceSwap);
			buttonFaceActiveSet(btn, BUTTON_SEC);
		}
	}else if (msg == CC_MSG_HOVER && data2 == 0){	// hover ended
		if (btn->flags.autoFaceSwap == 1 && btn->flags.canHover){
			//printf("face reset %i, %i\n", btn->id, btn->flags.autoFaceSwap);
			buttonFaceActiveSet(btn, BUTTON_PRI);
		}
	}

	return btn->clientMsgCb(btn, msg, data1, data2, dataPtr);
}

int ccbuttonNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t button_cb, int *id, const int x, const int y)
{
	TCCBUTTON *button = (TCCBUTTON*)object;

	button->pageOwner = pageOwner;
	if (id) *id = button->id;
	button->type = type;

	button->cb.msg = btn_cb;
	button->cb.create = ccbuttonNew;
	button->cb.free = ccbuttonDelete;
	button->cb.render = ccbuttonRender;
	button->cb.enable = ccbuttonEnable;
	button->cb.disable = ccbuttonDisable;
	button->cb.input = buttonHandleInput;
	button->cb.setPosition = ccbuttonSetPosition;
	button->cb.setMetrics = buttonSetMetrics;
	button->clientMsgCb = button_cb;
	
	const int width = 8;
	const int height = 8;

	button->canDrag = 0;
	button->metrics.x = x;
	button->metrics.y = y;
	button->metrics.width = width;
	button->metrics.height = height;

	button->flags.canAnimate = 0;
	button->flags.highlightDisable = 1;
	button->flags.acceptDrag = 0;
	button->flags.canHover = 0;
	button->flags.disableInput = 0;
	button->flags.disableRender = 0;
	button->flags.autoFaceSwap = 0;
	
	button->zoom.direction = 0;
	button->zoom.scaleBy = 1.0;
	button->zoom.scaleRate = 0.16;
	button->zoom.updateRate = 25;

	for (int i = 0; i < 2; i++){
		button->btnlbl[i] = my_calloc(1, sizeof(TBUTTONLABEL));
		if (button->btnlbl[i]){

			int id;
			TLABEL *label = ccCreate(button->cc, pageOwner, CC_LABEL, btn_label_cb, &id, width, height);
			if (!label) abort();

			label->isChild = 1;
			labelRenderFlagsSet(label, LABEL_RENDER_IMAGE/*|LABEL_RENDER_HOVER_OBJ|LABEL_RENDER_HOVER*/);
			ccSetUserData(label, button);
			ccSetPosition(label, x, y);
			ccDisable(label);

			button->btnlbl[i]->label = label;
			button->btnlbl[i]->itemId = 0;
			button->btnlbl[i]->imgId = 0;
		}
	}
	
	buttonFaceAutoSwapDisable(button);
	buttonSetActive(button, BUTTON_PRI);
	ccSetPosition(button, x, y);

	//printf("ccbuttonNew %p %i (%i)\n", button, button->id, button->pageOwner);

	return 1;
}


