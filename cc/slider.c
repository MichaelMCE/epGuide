
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


void sliderHoverEnable (TSLIDER *slider, const int colour, const double alpha)
{
	labelRenderFlagsSet(slider->base, labelRenderFlagsGet(slider->base) | LABEL_RENDER_HOVER);
	labelImgcHoverSet(slider->base, slider->faceIds[SLIDER_FACE_TIP], colour, alpha);
}
	
void sliderHoverDisable (TSLIDER *slider)
{
	labelRenderFlagsSet(slider->base, labelRenderFlagsGet(slider->base) &~ LABEL_RENDER_HOVER);
}

static inline int sliderSetTipPositionHori (TSLIDER *slider, const double pos)
{
	double w = labelImageGetWidth(slider->base, slider->faceIds[SLIDER_FACE_MID]);
	double value = w * pos;
	double tipWidth = (labelImgcGetWidth(slider->base, slider->faceIds[SLIDER_FACE_TIP])/2.0)-0.5;
	
	// centre tip on control
	int x = 0; int y = 0;
	labelItemPositionGet(slider->base, slider->faceIds[SLIDER_FACE_TIP], NULL, &y);
	labelItemPositionGet(slider->base, slider->faceIds[SLIDER_FACE_MID], &x, NULL);
	double xt = x + value - tipWidth;
	labelItemPositionSet(slider->base, slider->faceIds[SLIDER_FACE_TIP], xt, y);

	return x;
}

static inline int sliderSetTipPositionVert (TSLIDER *slider, const double pos)
{
	double h = labelImageGetHeight(slider->base, slider->faceIds[SLIDER_FACE_MID]);
	double value = h * pos;
	double tipHeight = (labelImgcGetHeight(slider->base, slider->faceIds[SLIDER_FACE_TIP])/2.0)-0.5;
	
	// centre tip on control
	int x = 0; int y = 0;
	labelItemPositionGet(slider->base, slider->faceIds[SLIDER_FACE_TIP], &x, NULL);
	labelItemPositionGet(slider->base, slider->faceIds[SLIDER_FACE_MID], NULL, &y);
	double yt = y + value - tipHeight;
	labelItemPositionSet(slider->base, slider->faceIds[SLIDER_FACE_TIP], x, yt);
	return y;
}


int sliderSetTipPosition (TSLIDER *slider, const double pos)
{
	if (slider->type == CC_SLIDER_HORIZONTAL)
		return sliderSetTipPositionHori(slider, pos);
	else if (slider->type == CC_SLIDER_VERTICAL)
		return sliderSetTipPositionVert(slider, pos);
	else
		return -1;
}

void sliderSetRange (TSLIDER *slider, const int64_t min, const int64_t max)
{
	slider->rangeMin = min;
	slider->rangeMax = max;
	slider->scalePos = 0.0;
	slider->value = 0;
}

void sliderGetRange (TSLIDER *slider, int64_t *min, int64_t *max)
{
	if (max) *max = slider->rangeMax;
	if (min) *min = slider->rangeMin;
}

int64_t sliderSetValueInt (TSLIDER *slider, const int64_t value)
{
	//printf("%I64i %I64i %I64i\n", value, slider->rangeMin, slider->rangeMax);

	if (value < slider->rangeMin)
		slider->value = slider->rangeMin;
	//else if ((int64_t)value == (int64_t)-1)
	//	slider->value = slider->rangeMin;
	else if (value > slider->rangeMax)
		slider->value = slider->rangeMax;
	else
		slider->value = value;
	return slider->value;
}

double sliderSetValueFloat (TSLIDER *slider, const double value)
{
	if (value > 1.0)
		slider->scalePos = 1.0;
	else if (value < 0.0)
		slider->scalePos = 0.0;
	else
		slider->scalePos = value;
	sliderSetTipPosition(slider, slider->scalePos/*value*/);
	return slider->scalePos;
}

int64_t sliderSetValue (TSLIDER *slider, const int64_t value)
{
	int64_t ivalue = sliderSetValueInt(slider, value);
	double fvalue = (ivalue - slider->rangeMin) / (double)(slider->rangeMax - slider->rangeMin);
	sliderSetValueFloat(slider, fvalue);

	ccSendMessage(slider, SLIDER_MSG_VALSET, slider->id, ivalue, NULL);
	return ivalue;
}

int64_t sliderGetValueLookup (TSLIDER *slider, const int mx)
{
	double w = labelImageGetWidth(slider->base, slider->faceIds[SLIDER_FACE_MID]);
	double value = (slider->rangeMax - slider->rangeMin) / w;
	return (int64_t)mx * value;
}

int64_t sliderGetValueInt (TSLIDER *slider)
{
	return slider->value;
}

double sliderGetValueFloat (TSLIDER *slider)
{
	return slider->scalePos;
}

int64_t sliderGetValue (TSLIDER *slider)
{
	return sliderGetValueInt(slider);
}

int sliderGetSliderRect (TSLIDER *slider, TLPOINTEX *rt)
{
	if (ccLock(slider)){
		TLABEL *label = slider->base;

		labelItemPositionGet(label, slider->faceIds[SLIDER_FACE_MID], &rt->x1, &rt->y1);
	
		rt->x1 += ccGetPositionX(slider);
		rt->y1 += ccGetPositionY(slider);
	
		rt->x2 = rt->x1 + labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]) - 1;
		rt->y2 = rt->y1 + labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]) - 1;
		
		ccUnlock(slider);
	}
	return 1;
}


static inline void sliderLabelAlignFacesHori (TSLIDER *slider, TLABEL *label)
{
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_LEFT], slider->pad.left, slider->pad.top);
	
	int w = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_LEFT]);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_MID], slider->pad.left+w, slider->pad.top);
	
	w += labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_RIGHT], slider->pad.left+w, slider->pad.top);

	int x = 0;
	labelItemPositionGet(label, slider->faceIds[SLIDER_FACE_TIP], &x, NULL);
	int midH = labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]);
	int tipH = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TIP]);
	int y = slider->pad.top + ((midH - tipH)/2);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_TIP], x, y);
}

static inline void sliderLabelAlignFacesVert (TSLIDER *slider, TLABEL *label)
{
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_TOP], 0, 0);
	
	int h = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TOP]);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_MID], 0, h);
	
	h += labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_BTM], 0, h);
	
	int y = 0;
	labelItemPositionGet(label, slider->faceIds[SLIDER_FACE_TIP], NULL, &y);
	int midW = labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
	int tipW = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_TIP]);
	int x = slider->pad.left + ((midW - tipW)/2);
	labelItemPositionSet(label, slider->faceIds[SLIDER_FACE_TIP], x, y);
}

static inline void sliderAlignFaces (TSLIDER *slider)
{
	if (slider->type == CC_SLIDER_HORIZONTAL)
		sliderLabelAlignFacesHori(slider, slider->base);
	else if (slider->type == CC_SLIDER_VERTICAL)
		sliderLabelAlignFacesVert(slider, slider->base);
}

static inline int64_t slider_label_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	TCCOBJECT *obj = (TCCOBJECT*)object;
	if (msg == CC_MSG_RENDER || msg == CC_MSG_INPUT) return 1;
	
	TLABEL *label = (TLABEL*)obj;
	TSLIDER *slider = (TSLIDER*)ccGetUserData(label);	
	
	if (slider)
		slider->touchInputId = label->touchInputId;
		
	//printf("slider_label_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);

	if (msg == LABEL_MSG_BASE_SELECTED_PRESS/* || msg == LABEL_MSG_BASE_SELECTED_SLIDE*/){	// base callback so adjust for image offset(s) then simulate a press
		TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		if (pos->pen) return 1;
		
		int x = (data1>>16)&0xFFFF;
		int y = data1&0xFFFF;
		
		//printf("slider_label_cb, id:%i %i %i\n", slider->id, pos->pen, pos->dt);
		
		if (slider->type == CC_SLIDER_HORIZONTAL){
			int w = labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
			
			int xLocal, yLocal;
			labelItemPositionGet(label, slider->faceIds[SLIDER_FACE_MID], &xLocal, &yLocal);
			if (x < xLocal){
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_LEFT], dataPtr);
				
			}else if (x >= xLocal && x < xLocal+w){
				x -= labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_LEFT]);
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_MID], dataPtr);
				
			}else{
				x -= (w + labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_LEFT]));
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_RIGHT], dataPtr);
			}
		}else if (slider->type == CC_SLIDER_VERTICAL){
			int h = labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]);
			
			int xLocal, yLocal;
			labelItemPositionGet(label, slider->faceIds[SLIDER_FACE_MID], &xLocal, &yLocal);
			if (y < yLocal){
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_TOP], dataPtr);
			}else if (y >= yLocal && y < yLocal+h){
				y -= labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TOP]);
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_MID], dataPtr);
			}else{
				y -= (h + labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TOP]));
				ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (x<<16)|y, slider->faceIds[SLIDER_FACE_BTM], dataPtr);
			}
		}
	}else if (msg == LABEL_MSG_IMAGE_SELECTED_PRESS || msg == LABEL_MSG_IMAGE_SELECTED_SLIDE){
		const int id = data2;
		//TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		//printf("slider_label_cb, %i id:%i %i %i, \n", pageGet(obj->cc->cc->vp), id, slider->touchInputId, pos->id);
	
	
		if (id == slider->faceIds[SLIDER_FACE_TIP]) return 0;

		int x = (data1>>16)&0xFFFF;
		int y = data1&0xFFFF;

		int64_t value = 0;
		
		if (id == slider->faceIds[SLIDER_FACE_LEFT]){
			value = slider->rangeMin;
			sliderSetValue(slider, value);
	  		ccSendMessage(slider, SLIDER_MSG_VALCHANGED, (0<<16)|y, value, NULL);
			
		}else if (id == slider->faceIds[SLIDER_FACE_RIGHT]){
			value = slider->rangeMax;
			sliderSetValue(slider, value);
			int xRight = labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
	  		ccSendMessage(slider, SLIDER_MSG_VALCHANGED, (xRight<<16)|y, value, NULL);
			
		}else if (id == slider->faceIds[SLIDER_FACE_MID]){
			if (slider->type == CC_SLIDER_HORIZONTAL){
				int w = labelImageGetWidth(slider->base, slider->faceIds[SLIDER_FACE_MID]);
				double mul = (double)(slider->rangeMax - slider->rangeMin) / (double)w;
				value = ((double)x * mul) + slider->rangeMin;
			}else if (slider->type == CC_SLIDER_VERTICAL){
				int h = labelImageGetHeight(slider->base, slider->faceIds[SLIDER_FACE_MID]);
				double mul = (double)(slider->rangeMax - slider->rangeMin) / (double)h;
				value = ((double)y * mul) + slider->rangeMin;				
			}
			
			sliderSetValue(slider, value);
	  		ccSendMessage(slider, SLIDER_MSG_VALCHANGED, (x<<16)|y, value, NULL);
		}
		
		//printf("%i: %i %i, %I64d\n", id, x, y, slider->value);
	}
	return 1;
}

static inline int sliderRender (void *object, TFRAME *frame)
{
	TSLIDER *slider = (TSLIDER*)object;
	return ccRender(slider->base, frame);
}

static inline int sliderSetPosition (void *object, const int x, const int y)
{
	TSLIDER *slider = (TSLIDER*)object;
	return ccSetPosition(slider->base, x, y);
}

static inline void sliderEnable (void *object)
{
	TSLIDER *slider = (TSLIDER*)object;
	slider->enabled = 1;
	ccEnable(slider->base);
}

static inline void sliderDisable (void *object)
{
	TSLIDER *slider = (TSLIDER*)object;
	ccDisable(slider->base);
	slider->enabled = 0;
}

static inline int sliderHandleInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	// input is handled via the base label control
	
	int ret = 0;
	/*
	TSLIDER *slider = (TSLIDER*)object;
	if (slider->enabled){
		if (!flags || (flags && slider->inputId == pos->id)){
			ret = (slider->buttons, slider->cc->vp, pos, flags);
			if (ret){
				if (!flags) slider->inputId = pos->id;
				sliderSetCursorPosition(slider, pos->x - ccGetPositionX(slider), pos->y - ccGetPositionY(slider));
			}
		}
	}
	*/
	return ret;
}

static inline void sliderBuildMidHori (TSLIDER *slider, TLABEL *label, const int width, const int height)
{
	
	int midWidth = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_LEFT]) + labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_RIGHT]);
	midWidth = width - midWidth;
	int midHeight = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_LEFT]);
	
	TFRAME *src = labelImageGetSrc(label, slider->faceIds[SLIDER_FACE_MIDSRC]);
	TFRAME *img = lNewFrame(slider->cc->hSurfaceLib, midWidth, midHeight, src->bpp);
	const int x2 = src->width-1;
	
	for (int x = 0; x < img->width-1; x += src->width){
		if (x+x2 < img->width)
			com_copyAreaNoBlend(src, img, x, 0, 0, 0, x2, src->height-1);
		else
			com_copyAreaNoBlend(src, img, x, 0, 0, 0, (img->width%src->width)-1, src->height-1);
	}

	labelImageSet(label, slider->faceIds[SLIDER_FACE_MID], img, 0);
	lDeleteFrame(img);
}

static inline void sliderBuildMidVert (TSLIDER *slider, TLABEL *label, const int width, const int height)
{
	
	int midHeight = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TOP]) + labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_BTM]);
	midHeight = height - midHeight;
	int midWidth = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_TOP]);
	
	TFRAME *src = labelImageGetSrc(label, slider->faceIds[SLIDER_FACE_MIDSRC]);
	TFRAME *img = lNewFrame(slider->cc->hSurfaceLib, midWidth, midHeight, src->bpp);
	int y2 = src->height-1;
	
				
	for (int y = 0; y < img->height; y += src->height){
		if (y+y2 < img->height)
			com_copyAreaNoBlend(src, img, 0, y, 0, 0, src->width-1, y2);
		else
			com_copyAreaNoBlend(src, img, 0, y, 0, 0, src->width-1, (img->height%src->height)-1);
	}

	labelImageSet(label, slider->faceIds[SLIDER_FACE_MID], img, 0);
	lDeleteFrame(img);
}

static inline void sliderBuildMid (TSLIDER *slider, const int width, const int height)
{
	if (slider->type == CC_SLIDER_HORIZONTAL)
		sliderBuildMidHori(slider, slider->base, width, height);
	else if (slider->type == CC_SLIDER_VERTICAL)
		sliderBuildMidVert(slider, slider->base, width, height);
}

int sliderFaceSet (TSLIDER *slider, int face, wchar_t *path)
{
	int ret = 0;
	
	if (ccLock(slider)){
		if (face == SLIDER_FACE_MID || face == SLIDER_FACE_MIDSRC){
			face = SLIDER_FACE_MIDSRC;
	
			wchar_t buffer[MAX_PATH+1];
			TFRAME *img = com_newImage(slider->cc, com_buildSkinD(slider->cc, buffer, path), SKINFILEBPP);
			if (img){
				ret = labelImageSet(slider->base, slider->faceIds[face], img, 0);
				if (ret){
					//if (face == SLIDER_FACE_MIDSRC)
						sliderBuildMid(slider, ccGetWidth(slider), ccGetHeight(slider));
					//sliderAlignFaces(slider);
				}
				lDeleteFrame(img);
			}
		}else{
			void *im = ccGetImageManager(slider->cc, CC_IMAGEMANAGER_IMAGE);
			slider->imgIds[face] = imageManagerImageAdd(im, path);
			ret = labelImgcSet(slider->base, slider->faceIds[face], slider->imgIds[face], 0);
			//int w, h;
			//labelImgcGetMetrics(slider->base, slider->faceIds[face], &w, &h);
			//printf("%i %i %i %i %i\n", slider->id, face, slider->faceIds[face], w, h);
		}

		ccUnlock(slider);
	}
	return ret;
}

static inline void sliderFacesApplyMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TSLIDER *slider = (TSLIDER*)object;
	
	//printf("sliderFacesApplyMetrics %i: %i %i %i %i\n", slider->id, x, y, width, height);

	ccSetMetrics(slider->base, x, y, width, height);
	sliderBuildMid(slider, width - (slider->pad.left+slider->pad.right), height - (slider->pad.top + slider->pad.btm));
	sliderAlignFaces(slider);
	
	slider->metrics.width = slider->base->metrics.width;
	slider->metrics.height = slider->base->metrics.height;
}

static inline int sliderSetMetrics (void *object, const int x, const int y, int width, int height)
{
	TSLIDER *slider = (TSLIDER*)object;
	
	
	//printf("sliderSetMetrics %i %i %i %i:: %i\n", x, y, width, height, slider->base->metrics.height);
	ccSetPosition(slider, x, y);
	slider->metrics.width = width;
	slider->metrics.height = height;
	
	sliderFacesApplyMetrics(slider, x, y, width, height);
	sliderFacesApply(slider);

	return 1;
}

static inline void sliderFacesApplyHori (TSLIDER *slider, TLABEL *label)
{

	int width = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_LEFT]);
	width += labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
	width += labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_RIGHT]);

	int tWidth = slider->pad.left + width + slider->pad.right;
	int tHeight = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_LEFT]);

	int height = labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]);
	if (height > tHeight) tHeight = height;
	height = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_RIGHT]);
	if (height > tHeight) tHeight = height;
	tHeight = slider->pad.top + tHeight + slider->pad.btm;
	
	sliderFacesApplyMetrics(slider, -1, -1, tWidth, tHeight);
}

static inline void sliderFacesApplyVert (TSLIDER *slider, TLABEL *label)
{
	int height = labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_TOP]);
	height += labelImageGetHeight(label, slider->faceIds[SLIDER_FACE_MID]);
	height += labelImgcGetHeight(label, slider->faceIds[SLIDER_FACE_BTM]);
	int tHeight = slider->pad.top + height + slider->pad.btm ;

	int tWidth = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_TOP]);
	int width = labelImageGetWidth(label, slider->faceIds[SLIDER_FACE_MID]);
	if (width > tWidth) tWidth = width;
	width = labelImgcGetWidth(label, slider->faceIds[SLIDER_FACE_BTM]);
	if (width > tWidth) tWidth = width;
	tWidth = slider->pad.left + tWidth + slider->pad.right;
	
	sliderFacesApplyMetrics(slider, -1, -1, tWidth, tHeight);
}

void sliderFacesApply (TSLIDER *slider)
{
	if (ccLock(slider)){
		sliderAlignFaces(slider);

		if (slider->type == CC_SLIDER_HORIZONTAL)
			sliderFacesApplyHori(slider, slider->base);
		else if (slider->type == CC_SLIDER_VERTICAL)	
			sliderFacesApplyVert(slider, slider->base);
	
		sliderAlignFaces(slider);
		ccUnlock(slider);
	}
	
	//ccSendMessage(slider->base, LABEL_MSG_IMAGE_SELECTED_PRESS, 0, slider->faceIds[SLIDER_FACE_LEFT], 0);
}

static inline void sliderDelete (void *object)
{
	TSLIDER *slider = (TSLIDER*)object;
	
	ccDelete(slider->base);
}

int sliderNew (TCCOBJECT *object, void *unused, const int pageOwner, const int sliderType, const TCommonCrtlCbMsg_t slider_cb, int *id, const int width, const int height)
{
	TSLIDER *slider = (TSLIDER*)object;

	slider->pageOwner = pageOwner;
	if (id) *id = slider->id;
	slider->type = sliderType;
		
	slider->cb.msg = slider_cb;
	slider->cb.render = sliderRender;
	slider->cb.create = sliderNew;
	slider->cb.free = sliderDelete;
	slider->cb.enable = sliderEnable;
	slider->cb.disable = sliderDisable;
	slider->cb.input = sliderHandleInput;
	slider->cb.setPosition = sliderSetPosition;
	slider->cb.setMetrics = sliderSetMetrics;
		
	slider->canDrag = 1;
	slider->inputId = -1;

	slider->rangeMin = 0;
	slider->rangeMax = 100;
	slider->value = 0;
	slider->scalePos = 0.0;

	slider->metrics.x = -1;
	slider->metrics.y = -1;
	slider->metrics.width = width;
	slider->metrics.height = height;
	
	slider->pad.left = 0;
	slider->pad.right = 4;
	slider->pad.top = 0;
	slider->pad.btm = 0;

	int lblid;
	slider->base = ccCreate(slider->cc, pageOwner, CC_LABEL, slider_label_cb, &lblid, width, height);
	slider->base->canDrag = slider->canDrag;
	ccSetUserData(slider->base, slider);
	labelRenderFlagsSet(slider->base, LABEL_RENDER_IMAGE /*|LABEL_RENDER_BORDER_POST*/);
	
	
	TFRAME *img = lNewFrame(slider->cc->hSurfaceLib, 8, 8, SKINFILEBPP);
	if (img){
		slider->faceIds[SLIDER_FACE_MIDSRC] = labelImageCreate(slider->base, img, 0, 0, 0);
		slider->faceIds[SLIDER_FACE_MID] = labelImageCreate(slider->base, img, 0, 0, 0);
		lDeleteFrame(img);
		
		void *im = ccGetImageManager(slider->cc, CC_IMAGEMANAGER_IMAGE);
		const int imgStubId = imageManagerImageAdd(im, L"cc\\stub.png");
		slider->faceIds[SLIDER_FACE_LEFT] = labelImgcCreate(slider->base, imgStubId, 0, 0, 0);
		slider->faceIds[SLIDER_FACE_RIGHT] = labelImgcCreate(slider->base, imgStubId, 0, 0, 0);
		slider->faceIds[SLIDER_FACE_TIP] = labelImgcCreate(slider->base, imgStubId, 0, 0, 0);
						
		labelItemDisable(slider->base, slider->faceIds[SLIDER_FACE_MIDSRC]);
		sliderHoverEnable(slider, COL_HOVER, 0.8);
	}

	sliderAlignFaces(slider);
	
	return 1;
}

