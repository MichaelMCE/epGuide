
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






static inline TBUTTON2LABEL *buttonLabelGet (TCCBUTTON2 *button, const int idx)
{
	return button->btnlbl[idx];
}

static inline void buttonLabelSet (TCCBUTTON2 *button, const int idx, TBUTTON2LABEL *bl)
{
	button->btnlbl[idx] = bl;
}

void button2DataSet (TCCBUTTON2 *btn, int64_t data)
{
	if (ccLock(btn)){
		btn->dataInt64 = data;
		ccUnlock(btn);
	}
}

int64_t button2DataGet (TCCBUTTON2 *btn)
{
	int64_t data = 0;
	if (ccLock(btn)){
		data = btn->dataInt64;
		ccUnlock(btn);
	}
	return data;
}

int ccButton2GetXWidth (TCCBUTTON2 *btn)
{
	return ccGetPositionX(btn) + ccGetWidth(btn);
}

int ccButton2GetXHeight (TCCBUTTON2 *btn)
{
	return ccGetPositionY(btn) + ccGetHeight(btn);
}

static inline int button2SetActive (TCCBUTTON2 *button, const int idx)
{
	//printf("button2SetActive %i, %i\n", button->id, idx);
	if (idx == BUTTON_PRI || idx == BUTTON_SEC){
		TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
		if (bl){
			if (bl->itemId){
				button->active = bl;
				//printf("active = %i\n", idx);
				return bl->itemId;

			}else if (bl->itemStrId){
				button->active = bl;
				//printf("active = %i\n", idx);
				return bl->itemStrId;
			}
		}
	}
	return 0;
}

int button2FaceActiveSet (TCCBUTTON2 *button, const int idx)
{
	//printf("button2FaceActiveSet %i, %i\n", button->id, idx);
	int ret = 0;
	if (ccLock(button)){
		ret = button2SetActive(button, idx);
		ccUnlock(button);
	}
	return ret;
}

int button2FaceActiveGet (TCCBUTTON2 *button)
{
	int ret = -1;
	if (ccLock(button)){
		if (button->active){
			if (button->active == buttonLabelGet(button, BUTTON_PRI))
				ret = BUTTON_PRI;
			else if (button->active == buttonLabelGet(button, BUTTON_SEC))
				ret = BUTTON_SEC;
		}
		ccUnlock(button);
	}
	return ret;
}

int button2FaceAutoSwapGet (TCCBUTTON2 *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.autoFaceSwap;
		ccUnlock(button);
	}
	return ret;
}

void button2FaceAutoSwapEnable (TCCBUTTON2 *button, const int mode)
{
	if (ccLock(button)){
		button->flags.autoFaceSwap = mode;
		ccUnlock(button);
	}
}

void button2FaceAutoSwapDisable (TCCBUTTON2 *button)
{
	if (ccLock(button)){
		button->flags.autoFaceSwap = 0;
		ccUnlock(button);
	}
}

void (CALLBACK timerBtn2SetPri)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	//printf("@@@ timerBtn2SetPri\n");

	TCCBUTTON2 *btn = (TCCBUTTON2*)dwUser;
	btn->timerId = 0;
	button2SetActive(btn, BUTTON_PRI);
	com_renderSignalUpdate(btn->cc);
}

static inline int64_t btn_label_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER/* || msg == CC_MSG_INPUT*/) return 1;

	TCCOBJECT *obj = (TCCOBJECT*)object;

	//printf("btn_label_cb. id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);


	TLABEL *label = (TLABEL*)obj;
	TCCBUTTON2 *btn = (TCCBUTTON2*)ccGetUserData(label);

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
	  //	TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
	  	//if (pos->pen != 0 || pos->dt <= 120) break;	// continue only if its a press/pen down and not a slide/drag
	  }
	  case LABEL_MSG_IMAGE_SELECTED_PRESS:{
  		if (btn->flags.canAnimate){
	  		btn->zoom.direction = 1;
  			btn->zoom.scaleBy = 1.0;
  			com_setTargetRate(btn->cc, btn->zoom.updateRate);
  		}

		if (btn->flags.autoFaceSwap == BUTTON_FACE_SWAP_SELECTED){
  			TBUTTON2LABEL *bl = buttonLabelGet(btn, BUTTON_SEC);
  			if (bl){
  				if (btn->timerId) timeKillEvent(btn->timerId);
  				button2SetActive(btn, BUTTON_SEC);
  				//printf("btn_label_cb timeSetEvent %p %i\n", btn, btn->id);
  				btn->timerId = (int)timeSetEvent(HIGHLIGHTPERIOD, 20, timerBtn2SetPri, (DWORD_PTR)btn, TIME_ONESHOT);
  			}
  		}

	  	//printf("btn_label_cb. id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);
	  	ccSendMessage(btn, BUTTON_MSG_SELECTED_PRESS, data1, data2, dataPtr);
	  	break;
	  }
	  case LABEL_MSG_IMAGE_SELECTED_SLIDE:
	  	if (btn->flags.acceptDrag){
	  		ccSendMessage(btn, BUTTON_MSG_SELECTED_SLIDE, data1, data2, dataPtr);
	  	}
	  	break;
	  case LABEL_MSG_IMAGE_SELECTED_RELEASE:
	  	ccSendMessage(btn, BUTTON_MSG_SELECTED_RELEASE, data1, data2, dataPtr);
	  	break;
	}
	return 1;
}

static inline TBUTTON2LABEL *buttonLabelCreate (TCCBUTTON2 *button)
{
	TBUTTON2LABEL *bl = my_calloc(1, sizeof(TBUTTON2LABEL));
	if (bl){
		TLABEL *label = ccCreate(button->cc, button->pageOwner, CC_LABEL, btn_label_cb, NULL, button->metrics.width, button->metrics.height);
		if (!label) abort();

		label->isChild = 1;
		labelRenderFlagsSet(label, LABEL_RENDER_IMAGE);
		ccSetUserData(label, button);
		ccSetPosition(label, button->metrics.x, button->metrics.y);
		ccDisable(label);

		bl->label = label;
		bl->itemId = 0;
		bl->itemStrId = 0;
		bl->imgId = 0;
	}
	return bl;
}

static inline void button2SetTextColour (TCCBUTTON2 *button, const int idx, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
	if (bl)
		labelRenderColourSet(bl->label, bl->itemStrId, fore, back, outline);
}

static inline int button2UpdateText (TCCBUTTON2 *button, const int idx, const char *str)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
	if (bl)
		return labelStringSet(bl->label, bl->itemStrId, str);
	return 0;
}

static inline int button2TextMaxWidthSet (TCCBUTTON2 *button, const int idx, const int width)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
	if (bl)
		return labelStringSetMaxWidth(bl->label, bl->itemStrId, width);
	return 0;
}

static inline void button2TextSetFlags (TCCBUTTON2 *button, const int idx, const int flags)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
	if (bl)
		labelStringRenderFlagsSet(bl->label, bl->itemStrId, flags);
}

static inline int button2SetText (TCCBUTTON2 *button, const int idx, const char *str, const int flags, const char *font, const int x, const int y)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);
	if (bl){
		bl->itemStrId = labelTextCreate(bl->label, str, flags, font, x, y/*+2*/);
		labelRenderFlagsSet(bl->label, labelRenderFlagsGet(bl->label)|LABEL_RENDER_TEXT);
		//labelRenderFilterSet(bl->label, bl->itemStrId, 2);
		return bl->itemStrId;
	}
	return 0;
}

static inline int button2FaceSetConstraints (TCCBUTTON2 *button, const int x1, const int y1, const int x2, const int y2)
{
	int ret = 0;
	TBUTTON2LABEL *bl = buttonLabelGet(button, BUTTON_PRI);
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

	bl = buttonLabelGet(button, BUTTON_SEC);
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

int button2FaceConstraintsSet (TCCBUTTON2 *button, const int x1, const int y1, const int x2, const int y2)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2FaceSetConstraints(button, x1, y1, x2, y2);
		ccUnlock(button);
	}
	return ret;
}

static inline int button2FaceConstraintsSetState (TCCBUTTON2 *button, const int state)
{
	int ret = 0;
	TBUTTON2LABEL *bl = buttonLabelGet(button, BUTTON_PRI);
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

	bl = buttonLabelGet(button, BUTTON_SEC);
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

int button2FaceConstraintsEnable (TCCBUTTON2 *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2FaceConstraintsSetState(button, 1);
		ccUnlock(button);
	}
	return ret;
}

int button2FaceConstraintsDisable (TCCBUTTON2 *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2FaceConstraintsSetState(button, 0);
		ccUnlock(button);
	}
	return ret;
}

void button2BorderProfileSet (TCCBUTTON2 *button, const int set, const uint32_t *colour, const int total)
{
	if (ccLock(button)){
		if (button->active)
			labelBorderProfileSet(button->active->label, set, colour, total);
		ccUnlock(button);
	}
}

void button2BaseColourSet (TCCBUTTON2 *button, const uint32_t colour)
{
	if (ccLock(button)){
		if (button->active)
			labelBaseColourSet(button->active->label, colour);
		ccUnlock(button);
	}
}

void button2BaseEnable (TCCBUTTON2 *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) | LABEL_RENDER_BLUR | LABEL_RENDER_BASE);
		ccUnlock(button);
	}
}

void button2BaseDisable (TCCBUTTON2 *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) &~LABEL_RENDER_BLUR &~LABEL_RENDER_BASE);
		ccUnlock(button);
	}
}

void button2BorderEnable (TCCBUTTON2 *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label)|LABEL_RENDER_BORDER_POST);
		ccUnlock(button);
	}
}

void button2BorderDisable (TCCBUTTON2 *button)
{
	if (ccLock(button)){
		if (button->active)
			labelRenderFlagsSet(button->active->label, labelRenderFlagsGet(button->active->label) &~LABEL_RENDER_BORDER_POST);
		ccUnlock(button);
	}
}

static inline int button2SetImageId (TCCBUTTON2 *button, const int idx, const int imgId, const double scale, const int x, const int y)
{
	TBUTTON2LABEL *bl = buttonLabelGet(button, idx);

	//printf("button2SetImageId %p\n", bl);

	if (!bl){
		bl = buttonLabelCreate(button);
		if (!bl) return 0;
		buttonLabelSet(button, idx, bl);
		if (idx == BUTTON_PRI) button2SetActive(button, idx);
	}

	if (!bl->itemId)
		bl->itemId = labelArtcCreate(bl->label, imgId, scale, 1, x, y);
	else
		labelArtcSet(bl->label, bl->itemId, imgId, 1);

	bl->imgId = imgId;
	return bl->itemId;
}

static inline int button2CreateFaceImgId (TCCBUTTON2 *button, const int priId, const int secId, double scale, const int x, const int y)
{
	int ret = 0;
	if (scale < 0.0 || scale > 1.0) scale = 1.0;

	if (priId)
		ret = button2SetImageId(button, BUTTON_PRI, priId, scale, x, y);

	if (secId)
		ret |= (button2SetImageId(button, BUTTON_SEC, secId, scale, x, y) << 16);


	//printf("button2CreateFaceImgId %i %i %f\n", ret, priId, scale);

	if (ret){
		int width = 0, height = 0;
		void *am = ccGetImageManager(button->cc, CC_IMAGEMANAGER_ART);
		if (!artManagerImageGetMetrics(am, priId, &width, &height)){
			printf("can not access image %X\n", priId);
			if (!artManagerImageGetMetrics(am, secId, &width, &height)){
				printf("can not access image %X\n", secId);
				return 0;
			}
		}

		if (width && height){
			if (scale < 0.001) scale = 1.0;
			ccSetMetrics(button, -1, -1, width * scale, height * scale);
			button2SetActive(button, BUTTON_PRI);
		}
	}

	return ret;
}

int button2FaceImgcSet (TCCBUTTON2 *btn, const int priId, const int secId, const double scale, const int x, const int y)
{
	int ret = 0;

	if (ccLock(btn)){
			ret = button2CreateFaceImgId(btn, priId, secId, scale, x, y);
		ccUnlock(btn);
	}
	return ret;
}

int button2FaceTextSet (TCCBUTTON2 *button, const int idx, const char *str, const int flags, const char *font, const int x, const int y)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2SetText(button, idx, str, flags, font, x, y);

		if (button2FaceActiveGet(button) == -1)
			button2FaceActiveSet(button, idx);

		ccUnlock(button);
	}
	return ret;
}

int button2FaceTextUpdate (TCCBUTTON2 *button, const int idx, const char *str)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2UpdateText(button, idx, str);
		ccUnlock(button);
	}
	return ret;
}

void button2FaceTextFlagsSet (TCCBUTTON2 *button, const int idx, const int flags)
{
	if (ccLock(button)){
		button2TextSetFlags(button, idx, flags);
		ccUnlock(button);
	}
}

int button2FaceTextMetricsGet (TCCBUTTON2 *button, const int face, int *x, int *y, int *width, int *height)
{
	int ret = 0;
	if (ccLock(button)){
		TBUTTON2LABEL *active = buttonLabelGet(button, face);
		if (active)
			ret = labelStringGetMetrics(active->label, active->itemStrId, x, y, width, height);
		ccUnlock(button);
	}
	return ret;
}

int button2FaceTextWidthSet (TCCBUTTON2 *button, const int idx, const int width)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button2TextMaxWidthSet(button, idx, width);
		ccUnlock(button);
	}
	return ret;
}


void button2FaceTextColourSet (TCCBUTTON2 *button, const int face, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	if (ccLock(button)){
		button2SetTextColour(button, face, fore, back, outline);
		ccUnlock(button);
	}
}

int button2AnimateGet (TCCBUTTON2 *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canAnimate;
		ccUnlock(button);
	}
	return ret;
}

int button2AnimateSet (TCCBUTTON2 *button, const int state)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canAnimate;
		button->flags.canAnimate = state&0x01;
		ccUnlock(button);
	}
	return ret;
}

int button2FaceHoverGet (TCCBUTTON2 *button)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canHover;
		ccUnlock(button);
	}
	return ret;
}

int button2FaceHoverSet (TCCBUTTON2 *button, const int state, const uint32_t colour, const double alpha)
{
	int ret = 0;
	if (ccLock(button)){
		ret = button->flags.canHover;
		button->flags.canHover = state&0x01;
		button->hover.colour = colour;
		button->hover.alpha = alpha;

		TBUTTON2LABEL *bl = buttonLabelGet(button, BUTTON_PRI);
		if (bl){
			if (bl->imgId)
				labelArtcHoverSet(bl->label, bl->itemId, colour, alpha);
			//else
			//	labelImageHoverSet(bl->label, bl->itemId, colour, alpha);
			
			int flags = labelRenderFlagsGet(bl->label);
			labelRenderFlagsSet(bl->label, flags|LABEL_RENDER_HOVER_OBJ/*|LABEL_RENDER_HOVER*/);
		}

		bl = buttonLabelGet(button, BUTTON_SEC);
		if (bl){
			if (bl->imgId)
				labelArtcHoverSet(bl->label, bl->itemId, colour, alpha);
			//else
			//	labelImageHoverSet(bl->label, bl->itemId, colour, alpha);
			
			int flags = labelRenderFlagsGet(bl->label);
			labelRenderFlagsSet(bl->label, flags|LABEL_RENDER_HOVER_OBJ/*|LABEL_RENDER_HOVER*/);
		}
		ccUnlock(button);
	}
	return ret;
}

/*
#############################################################################################
#############################################################################################
#############################################################################################
#############################################################################################
#############################################################################################
*/

static inline int ccbutton2SetPosition (void *object, const int x, const int y)
{
	//printf("treeviewSetPosition %i %i\n", x, y);

	TCCBUTTON2 *button = (TCCBUTTON2*)object;
	button->metrics.x = x;
	button->metrics.y = y;

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl){
			if (bl->itemId || bl->itemStrId)
				ccSetPosition(bl->label, x, y);
		}
	}

	return 1;
}

static inline int button2HandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{

	TCCBUTTON2 *btn = (TCCBUTTON2*)object;

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
		return ret;
	}
	return 0;
}

static inline int button2SetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	//printf("tvSetMetrics %p, %i %i %i %i\n",object, x, y, width, height);

	TCCBUTTON2 *button = (TCCBUTTON2*)object;

	ccSetPosition(button, x, y);
	button->metrics.width = width;
	button->metrics.height = height;

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl){
			if (bl->itemId || bl->itemStrId)
				ccSetMetrics(bl->label, x, y, width, height);
		}
	}

	return 1;
}

static inline void ccbutton2Enable (void *object)
{
	TCCBUTTON2 *button = (TCCBUTTON2*)object;
	button->enabled = 1;

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl){
			if (bl->itemId || bl->itemStrId)
				ccEnable(bl->label);
		}
	}
}

static inline void ccbutton2Disable (void *object)
{
	TCCBUTTON2 *button = (TCCBUTTON2*)object;
	button->enabled = 0;

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl){
			if (bl->itemId || bl->itemStrId)
				ccDisable(bl->label);
		}
	}
}

static inline int button2RenderLabel (TCCBUTTON2 *btn, TLABEL *label, TFRAME *frame)
{
	//printf("button2RenderLabel, %i %i %i\n", btn->isHovered, label->isHovered, btn->flags.canHover);

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

static inline int ccbutton2Render (void *object, TFRAME *frame)
{
	TCCBUTTON2 *btn = (TCCBUTTON2*)object;

	if (!btn->active)
		return 0;
	else if (btn->flags.disableRender)
		return 1;

	//int activeFace = buttonFaceActiveGet(btn);
	/*int activeFace = -1;

	if (btn->isHovered && !buttonFaceAutoSwapGet(btn)){
		activeFace = buttonFaceActiveGet(btn);
		if (activeFace == BUTTON_SEC)
			button2FaceActiveSet(btn, BUTTON_PRI);
		else if (activeFace == BUTTON_PRI)
			button2FaceActiveSet(btn, BUTTON_SEC);
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
			}
		}
		com_setTargetRate(btn->cc, btn->zoom.updateRate);
		labelArtcScaleSet(btn->active->label, btn->active->itemId, btn->zoom.scaleBy);
	}

	return button2RenderLabel(btn, label, frame);
}

static inline void ccbutton2Delete (void *object)
{
	TCCBUTTON2 *button = (TCCBUTTON2*)object;

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl){
			if (bl->label){
				ccDelete(bl->label);
				bl->label = NULL;
			}
		}
	}

	for (int i = 0; i < 2; i++){
		TBUTTON2LABEL *bl = buttonLabelGet(button, i);
		if (bl) my_free(bl);
	}
}

static inline int64_t btn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER/* || msg == CC_MSG_INPUT*/) return 1;

	TCCBUTTON2 *btn = (TCCBUTTON2*)object;

	if (msg == CC_MSG_HOVER && data2 == 1){			// hover begun
		if (btn->flags.autoFaceSwap == 1 && btn->flags.canHover){
			//printf("face set %i, %i\n", btn->id, btn->flags.autoFaceSwap);
			button2FaceActiveSet(btn, BUTTON_SEC);
		}
	}else if (msg == CC_MSG_HOVER && data2 == 0){	// hover ended
		if (btn->flags.autoFaceSwap == 1 && btn->flags.canHover){
			//printf("face reset %i, %i\n", btn->id, btn->flags.autoFaceSwap);
			button2FaceActiveSet(btn, BUTTON_PRI);
		}
	}

	return btn->clientMsgCb(btn, msg, data1, data2, dataPtr);
}

int ccbutton2New (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t button_cb, int *id, const int x, const int y)
{
	TCCBUTTON2 *button = (TCCBUTTON2*)object;

	button->pageOwner = pageOwner;
	if (id) *id = button->id;
	button->type = type;

	button->cb.msg = btn_cb;
	button->cb.create = ccbutton2New;
	button->cb.free = ccbutton2Delete;
	button->cb.render = ccbutton2Render;
	button->cb.enable = ccbutton2Enable;
	button->cb.disable = ccbutton2Disable;
	button->cb.input = button2HandleInput;
	button->cb.setPosition = ccbutton2SetPosition;
	button->cb.setMetrics = button2SetMetrics;
	button->clientMsgCb = button_cb;

	const int width = 8;
	const int height = 8;

	//button->imageManager = imageManager;
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

	button2FaceAutoSwapDisable(button);
	ccSetPosition(button, x, y);

	//printf("ccbutton2New %p %i (%i)\n", button, button->id, button->pageOwner);

	return 1;
}


