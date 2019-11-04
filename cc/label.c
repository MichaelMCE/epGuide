
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





#define DRAWMISCDETAIL 		0
#define DRAWTOUCHRECTS		0


#define DRAWBUTTONRECTCOL		(0xFF0000FF)	// blue
#define DRAWTOUCHRECTCOL		(0xFFFFFF00)	// yellow
#define DRAWCCOBJCOL			(0x00FFFF00)



/*
##################################################################################################################
##################################################################################################################
##################################################################################################################
##################################################################################################################
*/

int ccLabelFlushAll (TCC *cc)
{
	TCCOBJ *list = cc->objs;
	int ct = 0;

	do{
		//printf("%i %i\n", list->obj->type, list->obj->id);

		TCCOBJECT *ccObj = list->obj;

		if (ccObj /*&& ccObj->enabled == 1*/){
			if (ccObj->type == CC_LABEL){
				TLABEL *lbl = (TLABEL*)ccObj;
				if (ccLock(lbl)){

					TLISTITEM *item = lbl->objs->head;
					while(item){
						TLABELOBJ *obj = listGetStorage(item);
						if (obj/* && obj->enabled*/){
							if  (obj->objType == LABEL_OBJTYPE_ARTCACHEID || obj->objType == LABEL_OBJTYPE_IMGCACHEID){
								TLABELARTCA *img = obj->u.artc;
								if (img->drawable){
									img->drawableDrawn = 0;
									lDeleteFrame(img->drawable);
									img->drawable = NULL;
									ct++;
								}
							}
						}
						item = listGetNext(item);
					}
					
					ccUnlock(lbl);
				}
			}
		}

	}while((list=list->next));

	return ct;
}


static inline TLISTITEM *labelObjsIdToItem (TLABELOBJLIST *objs, const int id)
{
	TLISTITEM *item = objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj && obj->id == id) return item;
		item = listGetNext(item);
	}

	return NULL;
}

static inline void labelSetEnabledStateAll (TLABELOBJLIST *objs, const int state)
{
	//printf("labelSetEnabledStateAll %i\n", id);

	TLISTITEM *item = objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){
			obj->enabled = state;
			
			if (obj->objType == LABEL_OBJTYPE_CCOBJECT){
				TCCOBJECT *cc = obj->u.ccObj->cc;

				if (cc->type == CC_LABEL){
					TLABEL *label = (TLABEL*)cc;
					if (label->objs != objs)
						labelSetEnabledStateAll(label->objs, state);
				}
			}
		}
		item = listGetNext(item);
	}
}

static inline TLABELOBJ *labelObjsIdToObj (TLABELOBJLIST *objs, const int id)
{
	if (id < 1) return NULL;

	//printf("labelObjsIdToObj %i\n", id);

	TLISTITEM *item = objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){

			if (obj->id == id) return obj;
			if (obj->objType == LABEL_OBJTYPE_CCOBJECT){
				TCCOBJECT *cc = obj->u.ccObj->cc;

				if (cc->type == CC_LABEL){
					TLABEL *label = (TLABEL*)cc;
					if (label->objs != objs){
						obj = labelObjsIdToObj(label->objs, id);
						if (obj) return obj;
					}
				}
			}
		}
		item = listGetNext(item);
	}

	return NULL;
}

#if 1
static inline void *labelGetItem (TLABEL *label, const int id)
{
	TLABELOBJ *obj = labelObjsIdToObj(label->objs, id);
	if (obj)
		return obj->u.obj;
	else
		return NULL;
}
#else
static inline void *labelGetItem (TLABEL *label, const int id)
{
	void *ret = NULL;

	TLABELOBJ *obj = labelObjsIdToObj(label->objs, id);
	if (obj){
		switch (obj->objType){
		  case LABEL_OBJTYPE_IMGCACHEID:
		  case LABEL_OBJTYPE_ARTCACHEID:
		  	ret = obj->u.artc;
		  	break;
   		  case LABEL_OBJTYPE_TEXT:
  			ret = obj->u.text;
			break;
   		  case LABEL_OBJTYPE_IMAGE:
   			ret = obj->u.image;
			break;
   		  case LABEL_OBJTYPE_CCOBJECT:
   			ret = obj->u.ccObj;
			break;
		};
	}
	return ret;
}
#endif

void *labelItemGet (TLABEL *label, const int id)
{
	void *ptr = NULL;
	if (ccLock(label)){
		ptr = labelGetItem(label, id);
		ccUnlock(label);
	}
	return ptr;
}

static inline TLABELOBJ *labelGetObj (TLABEL *label, const int id)
{
	return labelObjsIdToObj(label->objs, id);
}

static inline int labelEnableObj (TLABEL *label, const int id)
{
	TLABELOBJ *obj = labelGetObj(label, id);
	if (obj) obj->enabled = 1;

	return (obj != NULL);
}

static inline int labelDisableObj (TLABEL *label, const int id)
{
	TLABELOBJ *obj = labelGetObj(label, id);
	if (obj) obj->enabled = 0;

	return (obj != NULL);
}

static inline int labelGetEnabledStatus (TLABEL *label, const int id)
{
	TLABELOBJ *obj = labelGetObj(label, id);
	if (obj)
		return obj->enabled;
	else
		return -1;
}

static inline void labelBaseSetColour (TLABELBASE *base, const int colour)
{
	base->colour = colour;
}

static inline void labelBaseRender (TLABELBASE *base, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	lDrawRectangleFilled(frame, x1, y1, x2, y2, base->colour);
}

static inline void labelBorderSetProfile (TLABELBORDER *border, const int set, const uint32_t *colour, const int total)
{
	border->thickness = MIN(total, 16);
	for (int i = 0; i < border->thickness; i++)
		border->colour[set][i] = colour[i];
}

static inline void labelBorderRender (TLABELBORDER *bdr, TFRAME *frame, const int set, int x1, int y1, int x2, int y2)
{
	x1--; y1--;
	x2++; y2++;

	for (int i = 0; i < bdr->thickness; i++)
		lDrawRectangle(frame, x1-i, y1-i, x2+i, y2+i, bdr->colour[set][i]);
}

static inline void labelTextFree (TLABELTEXT *text)
{

	//printf("labelTextFree: %i '%s'\n", text->maxW, text->str);

	if (text->str)
		my_free(text->str);
	if (text->uf.surface)
		fontSurfaceFree(text->uf.surface);

	my_free(text);
}

static inline void labelArtcFree (TLABELARTCA *image, const int dontFreeDrawable)
{
	if (!dontFreeDrawable && image->drawable){
		//printf("labelArtcFree %i: %X %i\n", image->groupId, image->imgId, image->drawable->groupId);
		lDeleteFrame(image->drawable);
	}

	my_free(image);
}

static inline void labelImageFree (TLABELIMAGE *image)
{
	//printf("labelImageFree %p %p\n", image->img, image->working);
		
	if (image->img) lDeleteFrame(image->img);
	if (image->working) lDeleteFrame(image->working);
	my_free(image);
}

static inline void labelCCObjFree (TLABELCCOBJ *ccObj)
{
	if (ccObj->cc)
		ccDelete(ccObj->cc);
	my_free(ccObj);
}

static inline char *labelTextGetString (TLABELTEXT *text)
{
	if (text->str)
		return my_strdup(text->str);
	else
		return NULL;
}

static inline void labelTextSetColour (TLABELTEXT *text, const uint32_t fore, const uint32_t back, const int outline)
{
	text->colInk = fore;
	text->colBack = back;
	text->colOutline = outline;
}

static inline void labelTextGetColour (TLABELTEXT *text, uint32_t *fore, uint32_t *back, uint32_t *outline)
{
	if (fore) *fore = text->colInk;
	if (back) *back = text->colBack;
	if (outline) *outline = text->colOutline;
}

static inline int labelTextGetMetrics (TLABELTEXT *text, int *x, int *y, int *width, int *height)
{
	if (text->uf.surface){
		if (x) *x = text->offset.x;
		if (y) *y = text->offset.y;
		if (width) *width = fontGetWidth(text->uf.surface);
		if (height) *height = fontGetHeight(text->uf.surface);
		return 1;		
	}
	return 0;
}

static inline int labelTextUpdateText (TLABELTEXT *text, const char *str)
{
	
	//printf("labelTextUpdateText: %p '%s'\n", text->uf.surface, str);
	
	//if (str)
	//	strncpy(text->string, str, MAX_PATH_UTF8);

	if (str){
		if (text->str) my_free(text->str);
		text->str = my_strdup(str);
	}else{
		text->str = NULL;
	}

	if (text->uf.surface){
		fontSurfaceFree(text->uf.surface);
		text->uf.surface = NULL;
	}

/*
	if (text->frame){
		lDeleteFrame(text->frame);
		text->frame = NULL;
	}
	if (text->blur){
		lDeleteFrame(text->blur);
		text->blur = NULL;
	}
*/
	return (str != NULL);
}

static inline void labelTextSetFont (TLABELTEXT *text, const char *font)
{
	//if (text->uf.file && !strcmp(font, text->uf.file))
	//	return;

	text->uf.fontIdx = ccFontAdd(text->parent->cc, font);

	//if (text->uf.file) my_free(text->uf.file);
	//text->uf.file = my_strdup(font);
	
	//if (text->uf.hLib){				// force font rebuild
	//	fontClose(text->uf.hLib);
	//	text->uf.hLib = NULL;
	//}

}

static inline int labelTextGetFont (TLABELTEXT *text)
{
	return text->uf.fontIdx;
}

static inline void labelTextSetRenderFlags (TLABELTEXT *text, const int flags)
{
	text->uflags = flags;
}

static inline int labelTextGetRenderFlags (TLABELTEXT *text)
{
	return text->uflags;
}

static inline int labelTextGetFilterType (TLABELTEXT *text)
{
	return text->filterType;
}

static inline void labelTextSetFilterType (TLABELTEXT *text, const int type)
{
	text->filterType = type;
}

static inline int labelTextGetBlurRadius (TLABELTEXT *text)
{
	return text->blurRadius;
}

static inline void labelTextSetBlurRadius (TLABELTEXT *text, const int radius)
{
	text->blurRadius = radius;
}

static inline double labelTextGetScale (TLABELTEXT *text)
{
	return text->scale;
}

static inline void labelTextSetScale (TLABELTEXT *text, const double scale)
{
	text->scale = scale;
	//printf("labelTextSetScale %f\n", text->scale);
}

static inline void labelTextPositionGet (TLABELTEXT *text, int *xLocal, int *yLocal)
{
	if (xLocal) *xLocal = text->pos.x;
	if (yLocal) *yLocal = text->pos.y;
}

static inline void labelTextSetMaxWidth (TLABELTEXT *text, const int width)
{
	text->maxW = max(width, 4);
}

static inline void labelTextSetMaxHeight (TLABELTEXT *text, const int height)
{
	text->maxH = max(height, 4);
}

static inline int labelTextGetMaxWidth (TLABELTEXT *text)
{
	return text->maxW;
}

static inline int labelTextGetMaxHeight (TLABELTEXT *text)
{
	return text->maxH;
}

// fix&test me
static inline void labelObjsTextResize (TLABELTEXT *text, const int width, const int height)
{
	//labelTextSetMaxWidth(text, abs(text->pos.x - width));
	//labelTextSetMaxHeight(text, abs(text->pos.y - height));
	//printf("labelObjsTextResize %i %i, %i\n", text->pos.x, text->maxW, width);
	

	if (text->uf.surface){
		// force prerendered string buffers to recreate
		char *str = labelTextGetString(text);
		if (str){
			labelTextUpdateText(text, str);
			my_free(str);
		}
	}
}

static inline void labelTextPositionSet (TLABEL *label, TLABELTEXT *text, const int xLocal, const int yLocal)
{
	//printf("labelTextPositionSet: %i %i\n", xLocal, yLocal);
	
	text->pos.x = xLocal;
	text->pos.y = yLocal;
	text->offset.x = text->pos.x;
	text->offset.y = text->pos.y;

	//labelObjsTextResize(text, ccGetWidth(label) - xLocal, ccGetHeight(label) - yLocal);
}

static inline int64_t labelObjSetData (TLABELOBJ *obj, const int64_t data)
{
	int64_t old = obj->dataInt64;
	obj->dataInt64 = data;
	return old;
}

static inline int64_t labelObjGetData (TLABELOBJ *obj)
{
	return obj->dataInt64;
}

static inline int labelGetObjType (TLABEL *label, const int id)
{
	TLABELOBJ *obj = labelObjsIdToObj(label->objs, id);
	if (obj)
		return obj->objType;
	else
		return 0;
}

static inline void labelCCObjSet (TLABELCCOBJ *ccObj, void *object)
{
	ccObj->cc = object;
}

static inline void *labelCCObjGet (TLABELCCOBJ *ccObj)
{
	return ccObj->cc;
}

static inline int labelImageCreateByFrame (TLABEL *label, TLABELIMAGE *image, TFRAME *img, const int resize, const int xLocal, const int yLocal)
{
	if (!img) return 0;

	image->img = lCloneFrame(img);
	image->working = lCloneFrame(img);
	image->scaleBy = 1.0;
	image->scaleImage = 0;
	image->opacity = 100;

	if (resize){
		ccSetMetrics(label, -1, -1, img->width, img->height);
		image->pos.x = 0;
		image->pos.y = 0;
	}else{
		image->pos.x = xLocal;
		image->pos.y = yLocal;

		int w = MIN(img->width, ccGetWidth(label) - image->pos.x);
		int h = MIN(img->height, ccGetHeight(label) - image->pos.y);

		lResizeFrame(image->working, w, h, 1);
	}

	//image->path = NULL;
	image->hasConstraints = 0;

	return 1;
}

static inline void labelImageSetHover (TLABELIMAGE *image, const uint32_t colour, const double alpha)
{
	image->canHover = 1;
	image->hoverColour = colour;
	image->hoverAlpha = alpha;
}

void labelImageHoverSet (TLABEL *label, const int id, const uint32_t colour, const double alpha)
{
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image)
			labelImageSetHover(image, colour, alpha);
		ccUnlock(label);
	}
}

static inline void labelArtcSetHover (TLABELARTCA *image, const uint32_t colour, const double alpha)
{
	image->canHover = 7;
	image->hoverColour = colour;
	image->hoverAlpha = alpha;
}

void labelArtcHoverSet (TLABEL *label, const int id, const uint32_t colour, const double alpha)
{
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image)
			labelArtcSetHover(image, colour, alpha);
		ccUnlock(label);
	}
}

static inline int labelArtcCreateByAmId (TLABEL *label, TLABELARTCA *image, const int artId, const double scale, const int resize, const int xLocal, const int yLocal, const int hoverState, const uint32_t hoverColour, const double hoverAlpha)
{

	image->imgId = artId;
	image->scaleAcquired = scale;
	image->scaleBy = 1.0;
	image->scaleImage = 0;
	image->opacity = 100;
	image->groupId = label->id;
	
	int width, height;
	if (artManagerImageGetMetrics(image->im, image->imgId, &width, &height)){
		if (image->scaleAcquired > 0.0){
			width *= image->scaleAcquired;
			height *= image->scaleAcquired;
		}
	}else{
		//printf("labelArtcCreateByAmId: can not read %X\n", artId);
		return 0;
	}

	if (width < 8) width = 8;
	if (height < 8) height = 8;

	image->drawable = lNewFrame(label->cc->hSurfaceLib, width, height, SKINFILEBPP);
	image->drawable->groupId = image->groupId;
	
	if (resize){
		ccSetMetrics(label, -1, -1, width, height);
		image->pos.x = 0;
		image->pos.y = 0;
	}else{
		image->pos.x = xLocal;
		image->pos.y = yLocal;

		int w = MIN(width, ccGetWidth(label) - image->pos.x);
		int h = MIN(height, ccGetHeight(label) - image->pos.y);
		lResizeFrame(image->drawable, w, h, 1);
	}

	image->hasConstraints = 0;
	image->drawableDrawn = 0;

	if (hoverState)
		labelArtcSetHover(image, hoverColour, hoverAlpha);

	//printf("labelArtcCreateByAmId: %i %i %i %i \n", width, height, image->pos.x, image->pos.y);

	return 1;
}

static inline TFRAME *labelImageGetImage (TLABEL *label, TLABELIMAGE *image)
{
	return lCloneFrame(image->img);
}

static inline TFRAME *labelImageGetImageSrc (TLABEL *label, TLABELIMAGE *image)
{
	return image->img;
}

static inline int labelArtcGetImage (TLABEL *label, TLABELARTCA *image)
{
	return image->imgId;
}

static inline int labelArtcGetImageSrc (TLABEL *label, TLABELARTCA *image)
{
	return image->imgId;
}

static inline void labelArtcSetImage (TLABEL *label, TLABELARTCA *image, const int artId, const int resize)
{
	//printf("labelArtcSetImage %i\n", imgId);

	if (image->imgId != artId){
		//artManagerImageDelete(image->im, image->imgId);
		//artManagerImageFlush(image->im, image->imgId);
	}
	image->imgId = artId;

	if (image->drawable)
		lDeleteFrame(image->drawable);
	image->drawable = NULL;

	//TFRAME *img = imageManagerImageAcquire(image->im, imgId);
	//if (!img) return;

	int width, height;
	artManagerImageGetMetrics(image->im, image->imgId, &width, &height);
	if (image->scaleAcquired > 0.0){
		width *= image->scaleAcquired;
		height *= image->scaleAcquired;
	}

	//printf("labelArtcSetImage %i, %i %i\n", artId, width, height);

	image->drawable = lNewFrame(label->cc->hSurfaceLib, width, height, SKINFILEBPP);
	image->drawable->groupId = image->groupId;
	image->drawableDrawn = 0;

	if (resize){	// resize label to image
		ccSetMetrics(label, -1, -1, width, height);
		image->pos.x = 0;
		image->pos.y = 0;
	}else{			// resize image to label
		int w = MIN(width, ccGetWidth(label) - image->pos.x);
		int h = MIN(height, ccGetHeight(label) - image->pos.y);
		lResizeFrame(image->drawable, w, h, 1);
	}

	//imageManagerImageRelease(image->im, image->imgId);

}

static inline void labelImageSetImage (TLABEL *label, TLABELIMAGE *image, TFRAME *img, const int resize)
{
	if (image->img)
		lDeleteFrame(image->img);
	image->img = NULL;

	if (image->working)
		lDeleteFrame(image->working);
	image->working = NULL;


	if (img){
		image->img = lCloneFrame(img);
		image->working = lCloneFrame(img);

		if (resize){	// reisze label to image
			ccSetMetrics(label, -1, -1, img->width, img->height);
			image->pos.x = 0;
			image->pos.y = 0;
		}else{			// resize image to label
			int w = MIN(img->width, ccGetWidth(label) - image->pos.x);
			int h = MIN(img->height, ccGetHeight(label) - image->pos.y);
			lResizeFrame(image->working, w, h, 1);
		}
	}
}

static inline void labelImageRenderScaled (TFRAME *src, TFRAME *des, const int x, const int y, const double s)
{
	com_copyAreaScaled(src, des, 0, 0, src->width, src->height, x, y, src->width*s, src->height*s);
}

static inline void labelImageSetScale (TLABELIMAGE *image, const double scale)
{
	if (scale > 0.01 && scale < 1.0){
		image->scaleBy = scale;
		image->scaleImage = 1;

		memset(image->working->pixels, 0, image->working->frameSize);
		labelImageRenderScaled(image->img, image->working, 0, 0, scale);
		//lSaveImage(image->working, L"setscale.png", IMG_PNG, 0, 0);

	}else{
		image->scaleBy = 1.0;
		image->scaleImage = 0;

		if (image->working->width == image->img->width && image->working->height == image->img->height){
			my_memcpy(image->working->pixels, image->img->pixels, image->img->frameSize);
		}else{
			memset(image->working->pixels, 0, image->working->frameSize);
			labelImageRenderScaled(image->img, image->working, 0, 0, 1.0);
		}
	}
}

static inline void labelArtcSetScale (TLABELARTCA *image, const double scale)
{

	//printf("labelArtcSetScale %i %p %i\n", image->drawableDrawn, image->drawable, image->imgId);
	TFRAME *img;
	if (image->scaleAcquired >= 1.0 || image->scaleAcquired <= 0.0)
		img = artManagerImageAcquire(image->im, image->imgId);
	else
		img = artManagerImageAcquireEx(image->im, image->imgId, image->scaleAcquired, image->opacity);

	if (!img) return;

	//printf("labelArtcSetScale %i %p %i, %f, %i %i, bpp %i\n", image->drawableDrawn, image->drawable, image->imgId, image->scaleAcquired, img->width, img->height, img->bpp);


	if (image->drawable == NULL){
		image->drawable = lNewFrame(img->hw, img->width, img->height, img->bpp);
		image->drawable->groupId = image->groupId;
	}	
	if (scale > 0.01 && scale < 1.0){
		image->scaleBy = scale;
		image->scaleImage = 5;

		memset(image->drawable->pixels, 0, image->drawable->frameSize);
		labelImageRenderScaled(img, image->drawable, 0, 0, scale);
		//lSaveImage(image->working, L"setscale.png", IMG_PNG, 0, 0);
	}else{
		image->scaleBy = 1.0;
		image->scaleImage = 0;

		if (image->drawable->width == img->width && image->drawable->height == img->height){
			//if (image->drawable->frameSize == img->frameSize)
				my_memcpy(image->drawable->pixels, img->pixels, img->frameSize);
		}else{
			//printf("labelArtcSetScale: image->drawable %p %p %i %i %i\n", image->drawable, image->drawable->pixels, image->drawable->frameSize, image->drawable->width, image->drawable->height);
			memset(image->drawable->pixels, 0, image->drawable->frameSize);
			com_copyAreaNoBlend(img, image->drawable, 0, 0, 0, 0, img->width-1, img->height-1);
		}
	}
	artManagerImageRelease(image->im, image->imgId);
}

static inline void labelImageSetPosition (TLABEL *label, TLABELIMAGE *image, const int xLocal, const int yLocal)
{
	image->pos.x = xLocal;
	image->pos.y = yLocal;

	TFRAME *img = lCloneFrame(image->img);
	if (img){
		labelImageSetImage(label, image, img, 0);
		lDeleteFrame(img);

		if (image->scaleImage)
			labelImageSetScale(image, image->scaleBy);
	}
}

static inline void labelArtcSetPosition (TLABEL *label, TLABELARTCA *image, const int xLocal, const int yLocal)
{
	image->pos.x = xLocal;
	image->pos.y = yLocal;

	//labelArtcSetImage(label, image, image->imgId, 0);

	if (image->scaleImage)
		labelArtcSetScale(image, image->scaleBy);
}


/*
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
*/

static inline int labelCCObjCreate (TLABEL *label, TLABELCCOBJ *cc, void *ccObject, const int xLocal, const int yLocal)
{
	cc->cc = ccObject;
	cc->pos.x = xLocal;
	cc->pos.y = yLocal;
	return 1;
}
 
static inline int labelObjTextCreateDefault (TLABEL *label, TLABELTEXT *text, const char *str, const int flags, const char *font, const int xLocal, const int yLocal, const int renderType)
{
	//printf("labelObjTextCreateDefault %i '%s'\n", renderType, str);


	//text->hw = label->cc->vp->ml->hw;
	text->parent = label;
	//strncpy(text->string, str, MAX_PATH_UTF8);
	text->str = my_strdup(str);
	//text->string[1][0] = 0;
	text->charRenderOffset = 0;
	//text->charRenderOffset[1] = 0;
	
	/*if ((flags&PF_CLIPWRAP) | (flags&PF_WORDWRAP))
		text->wrapText = (flags&PF_CLIPWRAP)|(flags&PF_WORDWRAP);
	else
		text->wrapText = 0;
	
	if (flags)
		labelTextSetRenderFlags(text, flags);
	else
		labelTextSetRenderFlags(text, PF_MIDDLEJUSTIFY);*/

	labelTextSetFont(text, font);
	labelTextSetRenderFlags(text, flags/*LABEL_ALLIGN_LEFT*/);
	labelTextPositionSet(label, text, xLocal, yLocal);
	labelTextSetMaxWidth(text, abs(xLocal - ccGetWidth(label)));
	labelTextSetMaxHeight(text, abs(yLocal - ccGetHeight(label)));

	labelTextSetColour(text, 255<<24|COL_WHITE, 255<<24|COL_BLACK, 177<<24|COL_BLUE_SEA_TINT);
	labelTextSetBlurRadius(text, 2);
	labelTextSetFilterType(text, renderType);
	return 1;

}

char *labelStringGet (TLABEL *label, const int id)
{
	char *txt = NULL;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			txt = labelTextGetString(text);
		ccUnlock(label);
	}
	return txt;
}

int labelItemGetType (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelGetObjType(label, id);
		ccUnlock(label);
	}
	return ret;
}

int64_t labelItemDataSet (TLABEL *label, const int id, const int64_t data)
{
	int64_t ret = 0;
	if (ccLock(label)){
		TLABELOBJ *obj = labelGetObj(label, id);
		if (obj)
			ret = labelObjSetData(obj, data);
		ccUnlock(label);
	}
	return ret;
}

int64_t labelItemDataGet (TLABEL *label, const int id)
{
	int64_t ret = 0;
	if (ccLock(label)){
		TLABELOBJ *obj = labelGetObj(label, id);
		if (obj)
			ret = labelObjGetData(obj);
		ccUnlock(label);
	}
	return ret;
}

int labelStringPositionGet (TLABEL *label, const int id, int *xLocal, int *yLocal)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			labelTextPositionGet(text, xLocal, yLocal);
			ret = 1;
		}
		ccUnlock(label);
	}
	return ret;
}

void labelRenderFontSet (TLABEL *label, const int id, const char *font)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			labelTextSetFont(text, font);
		ccUnlock(label);
	}
}

int abelRenderFontGet (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = labelTextGetFont(text);
		ccUnlock(label);
	}
	return ret;
}

void labelStringRenderFlagsSet (TLABEL *label, const int id, const int flags)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			labelTextSetRenderFlags(text, flags);
		ccUnlock(label);
	}
}

int labelStringRenderFlagsGet (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = labelTextGetRenderFlags(text);
		ccUnlock(label);
	}
	return ret;
}

void labelRenderColourSet (TLABEL *label, const int id, const uint32_t fore, const uint32_t back, const uint32_t outline)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			labelTextSetColour(text, fore, back, outline);
			
			if (text->uf.surface){
				// force prerendered string buffers to recreate
				char *str = labelTextGetString(text);
				if (str){
					labelTextUpdateText(text, str);
					my_free(str);
				}
			}
		}
		ccUnlock(label);
	}
}

int labelRenderColourGet (TLABEL *label, const int id, uint32_t *fore, uint32_t *back, uint32_t *outline)
{

	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			labelTextGetColour(text, fore, back, outline);
			ccUnlock(label);
			return 1;
		}
		ccUnlock(label);
	}
	return 0;
}

void labelRenderFilterSet (TLABEL *label, const int id, const int type)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			labelTextSetFilterType(text, type);
		ccUnlock(label);
	}
}

int labelRenderFilterGet (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = labelTextGetFilterType(text);
		ccUnlock(label);
	}
	return ret;
}

void labelRenderBlurRadiusSet (TLABEL *label, const int id, const int radius)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			labelTextSetBlurRadius(text, radius);
		ccUnlock(label);
	}
}

int labelRenderBlurRadiusGet (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = labelTextGetBlurRadius(text);
		ccUnlock(label);
	}
	return ret;
}

void labelRenderScaleSet (TLABEL *label, const int id, const double scale)
{
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			labelTextSetScale(text, scale);
			text->isScaled = !((scale > 0.0) && (scale < 0.0));
		}
		ccUnlock(label);
	}
}

double labelRenderScaleGet (TLABEL *label, const int id)
{
	float ret = 0.0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = labelTextGetScale(text);
		ccUnlock(label);
	}
	return ret;
}

void labelBaseColourSet (TLABEL *label, const uint32_t colour)
{
	if (ccLock(label)){
		labelBaseSetColour(&label->base, colour);
		ccUnlock(label);
	}
}

void labelBorderProfileSet (TLABEL *label, const int set, const uint32_t *colour, const int total)
{
	if (ccLock(label)){
		if (set < 2)
			labelBorderSetProfile(&label->border, set, colour, total);
		ccUnlock(label);
	}
}

void labelRenderFlagsSet (TLABEL *label, const uint32_t flags)
{
	if (ccLock(label)){
		label->renderflags = flags;
		if (flags&LABEL_RENDER_HOVER_OBJ)
			label->renderflags |= LABEL_RENDER_HOVER;
		if (flags&LABEL_RENDER_HOVER_OBJ2)
			label->renderflags |= LABEL_RENDER_HOVER | LABEL_RENDER_HOVER_OBJ | LABEL_RENDER_HOVER_OBJ2;
		ccUnlock(label);
	}
}

uint32_t labelRenderFlagsGet (TLABEL *label)
{
	uint32_t ret = 0;
	if (ccLock(label)){
		ret = label->renderflags;
		ccUnlock(label);
	}
	return ret;
}

int labelItemGetEnabledStatus (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelGetEnabledStatus(label, id);
		ccUnlock(label);
	}
	return ret;
}

int labelItemEnable (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelEnableObj(label, id);
		ccUnlock(label);
	}
	return ret;
}

int labelItemEnable_us (TLABEL *label, const int id)
{
	return labelEnableObj(label, id);
}

void labelItemsEnable (TLABEL *label)
{
	if (ccLock(label)){
		labelSetEnabledStateAll(label->objs, 1);
		ccUnlock(label);
	}
}

void labelItemsDisable (TLABEL *label)
{
	if (ccLock(label)){
		labelSetEnabledStateAll(label->objs, 0);
		ccUnlock(label);
	}
}

int labelItemDisable (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelDisableObj(label, id);
		ccUnlock(label);
	}
	return ret;
}

static inline void labelCCObjSetPosition (TLABEL *label, TLABELCCOBJ *ccObj, const int xLocal, const int yLocal)
{
	TCCOBJECT *cc = ccObj->cc;
	int x = ccGetPositionX(cc) - ccObj->pos.x;
	int y = ccGetPositionY(cc) - ccObj->pos.y;

	ccObj->pos.x = xLocal;
	ccObj->pos.y = yLocal;

	x += ccObj->pos.x;
	y += ccObj->pos.y;

	ccSetPosition(ccObj->cc, x, y);
}

static inline int labelObjSetPosition (TLABEL *label, const int id, const int xLocal, const int yLocal)
{
	TLABELOBJ *obj = labelGetObj(label, id);
	if (!obj) return 0;

	void *item = labelGetItem(label, id);
	if (!item) return 0;

	switch (obj->objType){
	  case LABEL_OBJTYPE_IMGCACHEID:
	  case LABEL_OBJTYPE_ARTCACHEID:
	  	labelArtcSetPosition(label, item, xLocal, yLocal);
	  	break;

	  case LABEL_OBJTYPE_TEXT:
	  	//printf("labelObjSetPosition %i %i %i\n", id, xLocal, yLocal);
		labelTextPositionSet(label, item, xLocal, yLocal);
		break;

	  case LABEL_OBJTYPE_IMAGE:
	  	labelImageSetPosition(label, item, xLocal, yLocal);
		break;

	  case LABEL_OBJTYPE_CCOBJECT:{
	  	labelCCObjSetPosition(label, item, xLocal, yLocal);

	  	TCCOBJECT *cc = obj->u.ccObj->cc;
		if (cc->type == CC_LABEL)
			labelObjSetPosition((TLABEL*)cc, id, xLocal, yLocal);
		break;
	  }
	};

	return 1;
}

static inline int labelObjGetPosition (TLABEL *label, const int id, int *xLocal, int *yLocal)
{
	TLABELOBJ *obj = labelGetObj(label, id);
	if (!obj) return 0;

	void *item = labelGetItem(label, id);
	if (!item) return 0;

	switch (obj->objType){
	  case LABEL_OBJTYPE_IMGCACHEID:
	  case LABEL_OBJTYPE_ARTCACHEID:{
	  	TLABELARTCA *image = (TLABELARTCA*)item;
	  	if (xLocal) *xLocal = image->pos.x;
		if (yLocal) *yLocal = image->pos.y;
		break;
	  }
	  case LABEL_OBJTYPE_TEXT:{
	  	TLABELTEXT *text = (TLABELTEXT*)item;
		if (xLocal) *xLocal = text->offset.x;
		if (yLocal) *yLocal = text->offset.y;
		break;
	  }
	  case LABEL_OBJTYPE_IMAGE:{
	  	TLABELIMAGE *image = (TLABELIMAGE*)item;
	  	if (xLocal) *xLocal = image->pos.x;
		if (yLocal) *yLocal = image->pos.y;
		break;
	  }
	  case LABEL_OBJTYPE_CCOBJECT:{
	  	TLABELCCOBJ *cc = (TLABELCCOBJ*)item;
	  	if (xLocal) *xLocal = cc->pos.x;
		if (yLocal) *yLocal = cc->pos.y;
		break;
	  }
	};

	return 1;
}

static inline _ufont_t *labelGetFontHandle (TLABEL *label, const int ccFontIdx)
{
	int ret = ccFontOpen(label->cc, ccFontIdx);
	if (ret < 1){
		//printf("labelGetFontHandle(): font open failed ret:%i, idx:%i\n", ret, ccFontIdx);
		return NULL;
	}
	return ccFontGetHandle(label->cc, ccFontIdx);
}

static inline int calcTextHoriAllignment (TLABELTEXT *text, const int x, const int maxW)
{
	if (text->uflags&LABEL_ALLIGN_MIDDLE)
		return x + (abs(/*text->*/maxW - text->uf.surface->width) / 2);
	else if (text->uflags&LABEL_ALLIGN_RIGHT)
		return x + (/*text->*/maxW - text->uf.surface->width);
	else
		return x + LABEL_TEXT_BORDERSIZE;
}

static inline int calcTextVertAllignment (TLABELTEXT *text, const int y)
{
	if (text->uflags&LABEL_ALLIGN_CENTER)
		return y + (abs(text->maxH - text->uf.surface->height) / 2);	// center
	else if (text->uflags&LABEL_ALLIGN_BOTTOM)
		return y + (text->maxH - text->uf.surface->height);			// bottom
	else
		return y  ;		// top
}

static inline int labelTextRender_uf (TLABELTEXT *text, TFRAME *frame, int x, int y, const int isHovered, const int lWidth)
{
	if (!text->str) return 0;
	if (text->maxW < 4 || text->maxH < 4) return 0;

	_ufont_t *hLib = NULL;
	int maxW = text->maxW;
	
	if (!text->uf.surface){

		//printf("labelTextRender_uf: %i '%s' %i\n", text->maxH, text->str, isHovered);
		
		int _x = 0, _y = 0;
		int width, height;

 		hLib = labelGetFontHandle(text->parent, text->uf.fontIdx);
		//fontSetRenderFlags(hLib, fontGetRenderFlags(hLib) | BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y| BFONT_RENDER_RETURN|BFONT_RENDER_NEWLINE);


		const uint16_t userFlags = hLib->render.flags;
		if (text->uflags&LABEL_WORDWRAP){
			hLib->render.flags  = BFONT_RENDER_WORDWRAP|BFONT_RENDER_WWRETURN;
			hLib->render.flags |= BFONT_RENDER_RETURN|BFONT_RENDER_NEWLINE;
		}

		if (!fontGetMetrics8(hLib, (uint8_t*)text->str, &width, &height))
			return 0;
			
		width++;
			
		// fixup until fontGetMetrics accounts for linewrap
		if (text->uflags&LABEL_WORDWRAP)
			height *= 2;
			
		//printf("WxH %i %i %i\n", width, height, text->maxH);

		//allign text within surface
		if (text->uflags&LABEL_ALLIGN_MIDDLE){
			if (width > /*text->*/maxW) _x -= abs(/*text->*/maxW - width) / 2;	// middle
		}else if (text->uflags&LABEL_ALLIGN_RIGHT){
			/*text->*/maxW -= LABEL_TEXT_BORDERSIZE;
			if (width > /*text->*/maxW) _x -= (width - /*text->*/maxW);			// right
		}else{
			/*text->*/maxW -= LABEL_TEXT_BORDERSIZE;
		}
		
		width = MIN(width, /*text->*/maxW);
		height = MIN(height, text->maxH);
		
		text->uf.surface = fontCreateSurface(width, height, COLOUR_24TO16(text->colInk), NULL);
		//printf("text->uf.surface %p %p\n", text, text->uf.surface);
		if (!text->uf.surface) abort();

		fontSetRenderSurface(hLib, text->uf.surface);
		fontPrint8(hLib, &_x, &_y, (uint8_t*)text->str);
		hLib->render.flags = userFlags;
		//fontDrawRectangle(hLib, 0, 0, width-1, height-1, 1);

		//printf("_uf: %i %i '%s'\n", text->maxW, width, text->str);

	}else{
		hLib = ccFontGetHandle(text->parent->cc, text->uf.fontIdx);
		fontSetRenderSurface(hLib, text->uf.surface);
	}

	if (text->parent->renderflags|LABEL_RENDER_CLIP){
		hLib->render.flags |= BFONT_RENDER_CLIPFRONT;
		my_memcpy(&hLib->display.clip, &text->parent->base.clip, sizeof(text->parent->base.clip));
	}

	// allign surface within label
	text->offset.x = calcTextHoriAllignment(text, x, maxW);
	text->offset.y = calcTextVertAllignment(text, y);

	//printf("labelRender: %i %i, %i %i, %i %i\n", x, y, text->offset.x, text->offset.y, text->parent->metrics.x, text->parent->metrics.y);

	fontSetDisplayBuffer(hLib, frame->pixels, frame->width, frame->height);
	
	if (isHovered){
		fontSetRenderColour(text->uf.surface, COLOUR_24TO16(text->colOutline));
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x-1, text->pos.y+text->offset.y);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x, text->pos.y+text->offset.y-1);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x+1, text->pos.y+text->offset.y);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x, text->pos.y+text->offset.y+1);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x-1, text->pos.y+text->offset.y-1);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x-1, text->pos.y+text->offset.y+1);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x+1, text->pos.y+text->offset.y-1);
		fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x+1, text->pos.y+text->offset.y+1);
		fontSetRenderColour(text->uf.surface, COLOUR_24TO16(text->colInk));
	}else{
		fontSetRenderColour(text->uf.surface, COLOUR_24TO16(text->colOutline));
	}

	fontApplySurfaceOutline(hLib, text->pos.x+text->offset.x, text->pos.y+text->offset.y);	
	fontSetRenderColour(text->uf.surface, COLOUR_24TO16(text->colInk));
	fontApplySurface(hLib, text->pos.x+text->offset.x, text->pos.y+text->offset.y);

	
	return 1;
}

static inline int labelTextRender (TLABELTEXT *text, TFRAME *frame, int x, int y, const int isHovered, const int lWidth)
{
	//printf("labelTextRender %p %i %i\n", text, x, y);
	
	return labelTextRender_uf(text, frame, x, y, isHovered, lWidth);
}

void labelRenderSetClipRegion (TLABEL *label, _rect_t *clip)
{
	if (ccLock(label)){
		my_memcpy(&label->base.clip, clip, sizeof(*clip));
		ccUnlock(label);
	}
}

static inline void labelCCObjRender (TLABELCCOBJ *ccObj, TFRAME *frame, int x, int y)
{
	if (!ccObj) return;

	x += ccObj->pos.x;
	y += ccObj->pos.y;

	if (ccGetPositionX(ccObj->cc) != x || ccGetPositionY(ccObj->cc) != y)
		ccSetPosition(ccObj->cc, x, y);

	ccRender(ccObj->cc, frame);
}

static inline void labelArtcRenderSurface (TLABELARTCA *image, const int imgWidth, const int imgHeight, TFRAME *working, TFRAME *frame, int x, int y, const int renderHover)
{

	//printf("labelArtcRenderSurface %p %p\n", image, working);

	if (!image->hasConstraints){
		if (image->scaleImage){
			int w = ((imgWidth)  * image->scaleBy);
			int h = ((imgHeight) * image->scaleBy);
			if (w > working->width) w = working->width;
			if (h > working->height) h = working->height;

			x += abs(w - working->width)/2;
			y += abs(h - working->height)/2;

			image->actual.x2 = x + w-1;
			image->actual.y2 = y + h-1;
		}else{
			image->actual.x2 = x + (working->width-1);
			image->actual.y2 = y + (working->height-1);
		}

		image->actual.x1 = x;
		image->actual.y1 = y;

		if (renderHover && image->canHover){
			//printf("labelArtcRenderSurface hover %p %p\n", working, frame);
			
			com_drawShadowedImageAlpha(working, frame, x, y, image->hoverColour, 2, 1, 1, image->hoverAlpha);
			com_drawShadowedImageAlpha(working, frame, x, y, image->hoverColour, 2, -1, -1, image->hoverAlpha);
		}else{
			//printf("labelArtcRenderSurface %i %i %p %p\n", x, image->imgId, working, frame);
			com_drawImage(working, frame, x, y, working->width-1, working->height-1);
		}
	}else{
		rect_t pos = {x, y, x+imgWidth-1, y+imgHeight-1};
		int x = 0;
		int y = 0;

		if (image->scaleImage){
			int w = (pos.x2 - pos.x1) * image->scaleBy;
			pos.x1 += abs(w - working->width)/2;

			int h = (pos.y2 - pos.y1) * image->scaleBy;
			pos.y1 += abs(h - working->height)/2;
		}

		if (pos.x1 < image->constraint.x1){
			x = abs(image->constraint.x1 - pos.x1);
			pos.x1 += abs(image->constraint.x1 - pos.x1);
		}
		if (pos.y1 < image->constraint.y1){
			y = abs(image->constraint.y1 - pos.y1);
			pos.y1 += abs(image->constraint.y1 - pos.y1);
		}
		if (pos.x2 > image->constraint.x2)
			pos.x2 -= abs(image->constraint.x2 - pos.x2);

		if (pos.y2 > image->constraint.y2)
			pos.y2 -= abs(image->constraint.y2 - pos.y2);

		const int w = (pos.x2 - pos.x1)+1;
		const int h = (pos.y2 - pos.y1)+1;

		if (renderHover && image->canHover){
			TFRAME *tmp = lNewFrame(frame->hw, w, h, frame->bpp);
			if (tmp){
				//printf("labelRenderHover %i %X\n", pos.x1, image->imgId);
				
				com_copyAreaNoBlend(working, tmp, 0, 0, x, y, x+w-1, y+h-1);
				com_drawShadowedImageAlpha(tmp, frame, pos.x1, pos.y1, image->hoverColour, 4, 3, 3, image->hoverAlpha);
				com_drawShadowedImageAlpha(tmp, frame, pos.x1, pos.y1, image->hoverColour, 4, -3, -3, image->hoverAlpha);
				lDeleteFrame(tmp);
			}
		}else{
			//printf("labelRender %i %i %i %X\n", pos.x1, pos.x1+x+w-1, x+w-1, image->imgId);
			com_copyArea(working, frame, pos.x1, pos.y1, x, y, x+w-1, y+h-1);
		}

		image->actual.x1 = pos.x1;
		image->actual.y1 = pos.y1;
		image->actual.x2 = pos.x2;
		image->actual.y2 = pos.y2;
	}

#if DRAWMISCDETAIL
	lDrawRectangle(frame, image->actual.x1, image->actual.y1, image->actual.x2, image->actual.y2, 255<<24 | 0xFF00FF);
#endif
}

static inline void labelArtcRender (TLABELARTCA *image, TFRAME *frame, const int x, const int y, const int renderHover)
{
	//printf("labelArtcRender %i\n", image->imgId);

	if (!image) return;

	if (!image->drawableDrawn){
		image->drawableDrawn = 3;
		labelArtcSetScale(image, image->scaleBy);
	}

	//TFRAME *img = imageManagerImageAcquire(image->im, image->imgId);
	//if (img){
		int width, height;
		artManagerImageGetMetrics(image->im, image->imgId, &width, &height);
		if (image->scaleAcquired > 0.0){
			width *= image->scaleAcquired;
			height *= image->scaleAcquired;
		}

		//printf("labelArtcRender %i %i\n", width, height);

		if (image->drawable)
			labelArtcRenderSurface(image, width, height, image->drawable, frame, x+image->pos.x, y+image->pos.y, renderHover);
		//imageManagerImageRelease(image->im, image->imgId);
	//}
}

static inline void labelImageRenderSurface (TLABELIMAGE *image, TFRAME *img, TFRAME *working, TFRAME *frame, int x, int y, const int renderHover)
{

	//printf("labelImageRender %p %p\n", img, working);

	if (!image->hasConstraints){
		if (image->scaleImage){
			int w = ((img->width)  * image->scaleBy);
			int h = ((img->height) * image->scaleBy);
			if (w > working->width) w = working->width;
			if (h > working->height) h = working->height;

			x += abs(w - working->width)/2;
			y += abs(h - working->height)/2;

			image->actual.x2 = x + w-1;
			image->actual.y2 = y + h-1;
		}else{
			image->actual.x2 = x + (working->width-1);
			image->actual.y2 = y + (working->height-1);
		}

		image->actual.x1 = x;
		image->actual.y1 = y;

		if (renderHover && image->canHover){
			com_drawShadowedImageAlpha(working, frame, x, y, image->hoverColour, 2, 1, 1, image->hoverAlpha);
			com_drawShadowedImageAlpha(working, frame, x, y, image->hoverColour, 2, -1, -1, image->hoverAlpha);
		}else{
			com_drawImage(working, frame, x, y, working->width-1, working->height-1);
		}
	}else{
		rect_t pos = {x, y, x+img->width-1, y+img->height-1};
		int x = 0;
		int y = 0;

		if (image->scaleImage){
			int w = (pos.x2 - pos.x1) * image->scaleBy;
			pos.x1 += abs(w - working->width)/2;

			int h = (pos.y2 - pos.y1) * image->scaleBy;
			pos.y1 += abs(h - working->height)/2;
		}

		if (pos.x1 < image->constraint.x1){
			x = abs(image->constraint.x1 - pos.x1);
			pos.x1 += abs(image->constraint.x1 - pos.x1);
		}
		if (pos.y1 < image->constraint.y1){
			y = abs(image->constraint.y1 - pos.y1);
			pos.y1 += abs(image->constraint.y1 - pos.y1);
		}
		if (pos.x2 > image->constraint.x2)
			pos.x2 -= abs(image->constraint.x2 - pos.x2);

		if (pos.y2 > image->constraint.y2)
			pos.y2 -= abs(image->constraint.y2 - pos.y2);

		const int w = (pos.x2 - pos.x1)+1;
		const int h = (pos.y2 - pos.y1)+1;

		if (renderHover && image->canHover){
			TFRAME *img = lNewFrame(frame->hw, w, h, frame->bpp);
			if (img){
				com_copyAreaNoBlend(working, img, 0, 0, x, y, x+w-1, y+h-1);
				com_drawShadowedImageAlpha(img, frame, pos.x1, pos.y1, image->hoverColour, 4, 3, 3, image->hoverAlpha);
				com_drawShadowedImageAlpha(img, frame, pos.x1, pos.y1, image->hoverColour, 4, -3, -3, image->hoverAlpha);
				lDeleteFrame(img);
			}
		}else{
			com_copyArea(working, frame, pos.x1, pos.y1, x, y, x+w-1, y+h-1);
		}

		image->actual.x1 = pos.x1;
		image->actual.y1 = pos.y1;
		image->actual.x2 = pos.x2;
		image->actual.y2 = pos.y2;
	}

#if DRAWMISCDETAIL
	lDrawRectangle(frame, image->actual.x1, image->actual.y1, image->actual.x2, image->actual.y2, 255<<24 | 0xFFFFFF);
#endif
}

static inline void labelImageRender (TLABELIMAGE *image, TFRAME *frame, const int x, const int y, const int renderHover)
{
	if (!image) return;
	labelImageRenderSurface(image, image->img, image->working, frame, x+image->pos.x, y+image->pos.y, renderHover);
}

double labelImageScaleGet (TLABEL *label, const int id)
{
	double ret = 1.0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image)
			ret =  image->scaleBy;
		ccUnlock(label);
	}
	return ret;
}

int labelImageScaleSet (TLABEL *label, const int id, const double scale)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image){
			labelImageSetScale(image, scale);
			ret = image->scaleImage;
		}
		ccUnlock(label);
	}
	return ret;
}

double labelArtcScaleGet (TLABEL *label, const int id)
{
	double ret = 1.0;
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image)
			ret =  image->scaleBy;
		ccUnlock(label);
	}
	return ret;
}

int labelArtcScaleSet (TLABEL *label, const int id, const double scale)
{
	//printf("labelArtcScaleSet %f\n", scale);

	int ret = 0;
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image){
			labelArtcSetScale(image, scale);
			ret = image->scaleImage;
		}
		ccUnlock(label);
	}
	return ret;
}


int labelArtcOpacityGet (TLABEL *label, const int id)
{
	int ret = 100;
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image)
			ret =  image->opacity;
		ccUnlock(label);
	}
	return ret;
}

int labelArtcOpacitySet (TLABEL *label, const int id, const int opacity)
{
	//printf("labelArtcScaleSet %f\n", scale);

	int ret = 0;
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image)
			ret = (image->opacity = opacity);
		ccUnlock(label);
	}
	return ret;
}

static inline int labelObjsRender (TLABEL *label, TLABELOBJLIST *objs, TFRAME *frame, const int renderFlags, const int x, const int y)
{
	int ct = 0;
	point_t cur;
	ccInputGetPosition(label, &cur);
	TLISTITEM *item = objs->head;

	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj && obj->enabled){
			//printf("labelObjsRender %i: %i %p, %i\n", ct, obj->id, item, label->isHovered);

			switch (obj->objType){
	   		  case LABEL_OBJTYPE_IMGCACHEID:
	   		  case LABEL_OBJTYPE_ARTCACHEID:
				if (renderFlags&LABEL_RENDER_IMAGE){
	   		  		int isHovered = label->isHovered && (renderFlags&LABEL_RENDER_HOVER);
	   		  		if (isHovered && (renderFlags&LABEL_RENDER_HOVER_OBJ)){
	   		  			if (obj->u.artc->drawable){
							int x1 = x + obj->u.artc->pos.x;
							int y1 = y + obj->u.artc->pos.y;
							int x2 = x1 + obj->u.artc->drawable->width-1;
							int y2 = y1 + obj->u.artc->drawable->height-1;

	   		  				isHovered = 0;
							if (cur.x >= x1 && cur.x <= x2){
								if (cur.y >= y1 && cur.y <= y2)
									isHovered = 1;
							}
#if (DRAWMISCDETAIL)
	   		  				if (isHovered)
			   		 			lDrawRectangle(frame, x1, y1, x2, y2, 0xFF00FF00);
	   		  				else
	   		  					lDrawRectangle(frame, x1, y1, x2, y2, 0xFFFF0000);
#endif
	   		  			}
	   		  		}
	   		  		labelArtcRender(obj->u.artc, frame, x, y, isHovered);
	   		  		ct++;
	   		  	}
				break;
	   		  case LABEL_OBJTYPE_TEXT:
	   		  	if (renderFlags&LABEL_RENDER_TEXT){
	   		  		if (obj->u.text){
	   		  			int isHovered = label->isHovered && (renderFlags&LABEL_RENDER_HOVER);
	   		  			if (isHovered && (renderFlags&LABEL_RENDER_HOVER_OBJ)){
	   		  				if (obj->u.text->uf.surface){
	   		  					int x1, y1, x2, y2;
	   		  					if (renderFlags&LABEL_RENDER_HOVER_OBJ2){
									x1 = obj->u.text->pos.x + ccGetPositionX(label)-1;
									y1 = obj->u.text->pos.y + ccGetPositionY(label)-1;
									x2 = x1 + obj->u.text->maxW;
									y2 = y1 + obj->u.text->maxH-1;
								}else{
									x1 = obj->u.text->pos.x + obj->u.text->offset.x;
									y1 = obj->u.text->pos.y + obj->u.text->offset.y;
									x2 = x1 + fontGetWidth(obj->u.text->uf.surface)-1;
									y2 = y1 + fontGetHeight(obj->u.text->uf.surface)-1;
								}

	   		  					isHovered = 0;
								if (cur.x >= x1 && cur.x <= x2){
									if (cur.y >= y1 && cur.y <= y2)
										isHovered = 1;
								}
#if (DRAWMISCDETAIL)
	   		  					if (isHovered)
			   		  				lDrawRectangle(frame, x1, y1, x2, y2, 0xFF0000FF);
	   		  					//else
	   		  					//	lDrawRectangle(frame, x1, y1, x2, y2, 0xFFFF0000);
#endif
	   		  				}
	   		  			}
	   		  			
						//printf("labelObjsRender %i: %i, %i %i %i %i, %i %i\n", label->id, obj->id, label->metrics.x, label->metrics.y, label->metrics.width, label->metrics.height, obj->u.text->pos.x, obj->u.text->pos.y);
						//printf("LabelobjRender: %i 0x%I64X, %i %i\n", obj->id, obj->dataInt64, obj->u.text->pos.x, obj->u.text->pos.y);
						/*int ret =*/ labelTextRender(obj->u.text, frame, x, y, isHovered, label->metrics.width);
					}
					ct++;
				}
				break;
	   		  case LABEL_OBJTYPE_IMAGE:
				if (renderFlags&LABEL_RENDER_IMAGE){
	   		  		labelImageRender(obj->u.image, frame, x, y, label->isHovered && (renderFlags&LABEL_RENDER_HOVER));
	   		  		ct++;
	   		  	}
				break;
	   		  case LABEL_OBJTYPE_CCOBJECT:
				if (renderFlags&LABEL_RENDER_CCOBJ){
					labelCCObjRender(obj->u.ccObj, frame, x, y);
					ct++;
				}
				break;
			};
		}
		item = listGetNext(item);
	}
	return ct;
}

static inline int labelRender (void *object, TFRAME *frame)
{
	TLABEL *label = (TLABEL*)object;

	const int x = ccGetPositionX(label);
	const int y = ccGetPositionY(label);
	const int w = ccGetWidth(label);
	const int h = ccGetHeight(label);


	int pad = 0;
	if (label->renderflags&LABEL_RENDER_BORDER_PRE){
		labelBorderRender(&label->border, frame, 0, x, y, x+w-1, y+h-1);
		pad = label->border.thickness;
	}

	if (label->renderflags&LABEL_RENDER_BLUR)
		lBlurArea(frame, x-(1+pad), y-(1+pad), x+w+pad, y+h+pad, 3);

	if (label->renderflags&LABEL_RENDER_BASE)
		labelBaseRender(&label->base, frame, x, y, x+w-1, y+h-1);

	labelObjsRender(label, label->objs, frame, label->renderflags, x, y);


	if (label->renderflags&LABEL_RENDER_BORDER_POST)
		labelBorderRender(&label->border, frame, 1, x, y, x+w-1, y+h-1);

#if DRAWTOUCHRECTS
	lDrawRectangle(frame, x, y, x+w-1, y+h-1, DRAWTOUCHRECTCOL);
#endif
	return 1;
}

static inline void labelObjsFreeObj (TLABEL *label, TLABELOBJ *obj, const int dontFreeDrawable)
{
	switch (obj->objType){
	  case LABEL_OBJTYPE_TEXT:
		ccSendMessage(label, LABEL_MSG_DELETE, obj->objType, obj->id, obj);
		labelTextFree(obj->u.text);
		return;

	  case LABEL_OBJTYPE_IMGCACHEID:
	  case LABEL_OBJTYPE_ARTCACHEID:
	  	ccSendMessage(label, LABEL_MSG_DELETE, LABEL_OBJTYPE_ARTCACHEID, obj->id, obj);
		labelArtcFree(obj->u.artc, dontFreeDrawable);
	  	return;

	  case LABEL_OBJTYPE_IMAGE:
		ccSendMessage(label, LABEL_MSG_DELETE, obj->objType, obj->id, obj);
		labelImageFree(obj->u.image);
		return;

	  case LABEL_OBJTYPE_CCOBJECT:
		ccSendMessage(label, LABEL_MSG_DELETE, obj->objType, obj->id, obj);
		labelCCObjFree(obj->u.ccObj);
		return;
	};

	//obj->u.obj = NULL;
}

static inline int labelObjDeleteObj (TLABEL *label, TLABELOBJLIST *objs, const int id)
{
	TLISTITEM *item = labelObjsIdToItem(objs, id);
	if (item){
		TLABELOBJ *obj = listGetStorage(item);

		labelObjsFreeObj(label, obj, 0);

		//printf("count %i\n", listCount());

		TLISTITEM *next = item->next;
		TLISTITEM *prev = item->prev;

		listRemove(item);
		listDestroy(item);

		if (objs->head == item)
			objs->head = next;
		if (objs->tail == item)
			objs->tail = prev;

	}
	return (item != NULL);
}

static inline void labelObjsFreeObjs (TLABEL *label, TLABELOBJLIST *objs, const int dontFreeDrawable)
{
	//printf("labelObjsFreeObjs  in %p\n", objs);

	TLISTITEM *item = objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){
			labelObjsFreeObj(label, obj, dontFreeDrawable);
			my_free(obj);
		}
		item = listGetNext(item);
	}
}

static inline void labelObjsFreeList (TLABELOBJLIST *objs)
{
	if (objs->head)
		listDestroyAll(objs->head);
	my_free(objs);
}

static inline void labelObjsFree (TLABEL *label)
{
	if (!label->objs) return;

	lDeleteFrames(label->cc->hSurfaceLib, label->id);
	labelObjsFreeObjs(label, label->objs, 1);
	labelObjsFreeList(label->objs);
	label->objs = NULL;
}

static inline TLABELOBJLIST *labelObjsAlloc ()
{
	return my_calloc(1, sizeof(TLABELOBJLIST));
}

void labelItemsDelete (TLABEL *label)
{
	if (ccLock(label)){
		labelObjsFree(label);
		label->objs	= labelObjsAlloc();
		label->idSrc = 101;
		ccUnlock(label);
	}
}

static inline TLABELOBJ *labelObjAlloc (const int type)
{
	TLABELOBJ *obj = my_calloc(1, sizeof(TLABELOBJ));
	if (!obj) return NULL;

	obj->enabled = 1;
	obj->objType = type;

	switch (type){
	  case LABEL_OBJTYPE_IMGCACHEID:
	  case LABEL_OBJTYPE_ARTCACHEID:
	  	obj->u.artc = my_calloc(1, sizeof(TLABELARTCA));
	  	break;
	   case LABEL_OBJTYPE_TEXT:
	  	obj->u.text = my_calloc(1, sizeof(TLABELTEXT));
	  	//obj->pos = &obj->u.text->pos;
		break;
	   case LABEL_OBJTYPE_IMAGE:
	   	obj->u.image = my_calloc(1, sizeof(TLABELIMAGE));
	   	//obj->pos = &obj->image->pos;
		break;
	   case LABEL_OBJTYPE_CCOBJECT:
	   	obj->u.ccObj = my_calloc(1, sizeof(TLABELCCOBJ));
	   	//obj->pos = &obj->u.ccObj->pos;
		break;
	};

	return obj;
}

static inline int labelObjsInsertLast (TLABELOBJLIST *objs, TLABELOBJ *obj)
{
	if (!objs->head){
		objs->head = objs->tail = listNewItem(obj);
		return 1;
	}

	if (objs->head){
		TLISTITEM *item = listNewItem(obj);
		listInsert(item, objs->tail, NULL);
		objs->tail = item;
		return 1;
	}
	return 0;
}

static inline int labelObjAdd (TLABEL *label, TLABELOBJ *obj)
{
	return labelObjsInsertLast(label->objs, obj);
}

static int labelGenerateItemId (TLABEL *label)
{
	if (++label->idSrc < 2147483647){
		return label->idSrc;
	}else{
		//printf("labelGenerateItemId %i\n", label->idSrc);
		return (label->idSrc = 101);
	}
}

static inline void *labelObjNew (TLABEL *label, const int type, int *id)
{
	TLABELOBJ *obj = labelObjAlloc(type);
	if (obj){
		obj->id = labelGenerateItemId(label);
		if (labelObjAdd(label, obj)){
			if (id) *id = obj->id;
			return obj;
		}
		//else
		//	printf("labelObjNew obj not added %i %i\n", type, obj->id);
	}
	if (id) *id = 0;
	return NULL;
}

static inline int labelObjsTextCreate (TLABEL *label, const char *str, const int renderflags, const char *font, const int xLocal, const int yLocal, const int renderType, const int64_t udata)
{
	int id = 0;
	TLABELOBJ *obj = labelObjNew(label, LABEL_OBJTYPE_TEXT, &id);
	if (id){
		TLABELTEXT *text = obj->u.obj; //labelGetItem(label, id);
		if (text){
			obj->dataInt64 = udata;
			//printf("labelObjsTextCreate %i '%s'\n", id, str);
			labelObjTextCreateDefault(label, text, str, renderflags, font, xLocal, yLocal, renderType);
		}
	}
	return id;
}

static inline int labelObjImageCreate (TLABEL *label, TFRAME *img, const int resize, const int xLocal, const int yLocal)
{
	int id = 0;
	TLABELOBJ *obj = labelObjNew(label, LABEL_OBJTYPE_IMAGE, &id);
	if (id){
		TLABELIMAGE *image = obj->u.obj; //labelGetItem(label, id);
		if (image){
			labelImageCreateByFrame(label, image, img, resize, xLocal, yLocal);
			//image->path = my_wcsdup(path);
		}
	}
	return id;
}

static inline int labelObjImgcCreate (TLABEL *label, TARTMANAGER *manager, const int imgId, const int resize, const int xLocal, const int yLocal)
{
	int id = 0;
	TLABELOBJ *obj = labelObjNew(label, LABEL_OBJTYPE_IMGCACHEID, &id);
	if (obj){
		TLABELIMGCA *image = obj->u.obj; //labelGetItem(label, id);
		if (image){
			image->im = manager;
			if (!labelArtcCreateByAmId(label, image, imgId, 0.0, resize, xLocal, yLocal, 0, 0, 0.0))
				return 0;
		}
	}
	return id;
}

static inline int labelObjArtcCreate (TLABEL *label, TARTMANAGER *manager, const int artId, const double scale, const int resize, const int xLocal, const int yLocal, const int hoverState, const uint32_t hoverColour, const double hoverAlpha, const int64_t udata)
{
	int id = 0;
	TLABELOBJ *obj = labelObjNew(label, LABEL_OBJTYPE_ARTCACHEID, &id);
	if (obj){
		//TLABELOBJ *obj = labelObjsIdToObj(label->objs, id);
		TLABELARTCA *image = obj->u.obj; //labelGetItem(label, id);
		if (image){
			image->im = manager;
			obj->dataInt64 = udata;
			if (labelArtcCreateByAmId(label, image, artId, scale, resize, xLocal, yLocal, hoverState, hoverColour, hoverAlpha))
				return id;
		}
	}
	return 0;
}

static inline int labelObjCCObjectCreate (TLABEL *label, void *ccObject, const int xLocal, const int yLocal)
{
	int id = 0;
	TLABELOBJ *obj = labelObjNew(label, LABEL_OBJTYPE_CCOBJECT, &id);
	if (id){
		TLABELCCOBJ *cc = obj->u.obj; //labelGetItem(label, id);
		if (cc)
			labelCCObjCreate(label, cc, ccObject, xLocal, yLocal);
	}
	return id;
}

static inline int isOverlap (TTOUCHCOORD *pos, const point_t *obj, const int x, const int y, const int w, const int h)
{
	if (pos->y >= y+obj->y && pos->y < y+obj->y+h){
		if (pos->x >= x+obj->x && pos->x < x+obj->x+w)
			return 1;
	}
	return 0;
}

static inline int isOverlap4 (TTOUCHCOORD *pos, const rect_t *obj, const int x, const int y, const int w, const int h)
{
	if (pos->y >= obj->y1-y && pos->y <= (obj->y2-y)+h){
		if (pos->x >= obj->x1-x && pos->x < (obj->x2-x)+w)
			return 1;
	}
	return 0;
}

static inline int labelObjsHandleInput (TLABEL *label, TLABELOBJLIST *objs, TTOUCHCOORD *pos, const int flags)
{
	const int x = ccGetPositionX(label);
	const int y = ccGetPositionY(label);

	//printf("labelObjsHandleInput %i: %i %i\n", label->id, pos->x, pos->y);


	int ret = 0;
	TLISTITEM *item = objs->tail;
	//TLISTITEM *item = objs->head;

	while(item && !ret){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj && obj->enabled){
			switch (obj->objType){
	   		  case LABEL_OBJTYPE_IMGCACHEID:
	   		  case LABEL_OBJTYPE_ARTCACHEID:
	   		  	//printf("image: %i %i, %i %i\n", obj->image->actual.x1 - x, obj->image->actual.y1 - y, pos->x, pos->y);

	   		  	if (isOverlap4(pos, &obj->u.artc->actual, x, y, 0, 0)){
	   		  		const int cx = abs(obj->u.artc->actual.x1 - x - pos->x);
	   		  		const int cy = abs(obj->u.artc->actual.y1 - y - pos->y);

	   		  		if (!flags){				// press down
	   		  			//printf("label img down %i\n", obj->id);
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 1 && pos->id == label->touchInputId){	// drag
	   		  			//printf("label img slide %i\n", obj->id);
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_SLIDE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 3){	// up/release
	   		  			//printf("label img up %i\n", obj->id);
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_RELEASE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}

	   		  		//printf("imgc ccHandleInput %i\n", obj->id);
	   		  		//ret = 1;
				}
				break;

	   		  case LABEL_OBJTYPE_TEXT:
	   		  	//printf("text: %i %i, %i %i, %i\n", obj->u.text->offset.x, obj->u.text->offset.y, pos->x, pos->y, flags);
	   		  	//if (obj->u.text->string[0])
	   		  	//	printf(":0: '%s'\n",obj->u.text->string[0]);
	   		  	//if (obj->u.text->string[1])
	   		  	//	printf(":1: '%s'\n",obj->u.text->string[1]);

	   		  	//if (flags) break;

				//if ((obj->u.text->frame && isOverlap(pos, &obj->u.text->offset, 0, 0, obj->u.text->frame->width, obj->u.text->frame->height))
				if (obj->u.text->uf.surface && isOverlap(pos, &obj->u.text->offset, 0, 0, fontGetWidth(obj->u.text->uf.surface), fontGetHeight(obj->u.text->uf.surface))){
					int cx = abs(obj->u.text->offset.x - pos->x);
					int cy = abs(obj->u.text->offset.y - pos->y);

					//printf("text ccHandleInput %i %i %i %i\n", obj->id, x, y, flags);

					//ret = ccSendMessage(label, LABEL_MSG_TEXT_SELECTED, ((cx&0xFFFF)<<16)|(cy&0xFFFF), obj->id, pos);
					//printf("text ccHandleInput %i\n", obj->id);
					//ret = 1;

					if (!flags){				// press down
						//printf("label txt down %i\n", obj->id);
	   		  			ret = ccSendMessage(label, LABEL_MSG_TEXT_SELECTED_PRESS, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 1 && pos->id == label->touchInputId){	// drag
	   		  			//printf("label txt slide %i, %i\n", obj->id, pos->pen);
	   		  			ret = ccSendMessage(label, LABEL_MSG_TEXT_SELECTED_SLIDE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 3){	// up/release
	   		  			//printf("label txt up %i\n", obj->id);
	   		  			ret = ccSendMessage(label, LABEL_MSG_TEXT_SELECTED_RELEASE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}
				}
				break;

	   		  case LABEL_OBJTYPE_IMAGE:
	   		  	//printf("image: %i %i, %i %i\n", obj->image->actual.x1 - x, obj->image->actual.y1 - y, pos->x, pos->y);

	   		  	if (isOverlap4(pos, &obj->u.image->actual, x, y, 0, 0)){
	   		  		const int cx = abs(obj->u.image->actual.x1 - x - pos->x);
	   		  		const int cy = abs(obj->u.image->actual.y1 - y - pos->y);

	   		  		if (!flags){				// press down
	   		  			//printf("label down\n");
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_PRESS, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 1 && pos->id == label->touchInputId){	// drag
	   		  			//printf("label slide\n");
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_SLIDE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}else if (flags == 3){	// up/release
	   		  			//printf("label up\n");
	   		  			ret = ccSendMessage(label, LABEL_MSG_IMAGE_SELECTED_RELEASE, (cx<<16)|(cy&0xFFFF), obj->id, pos);
	   		  		}

	   		  		//printf("image ccHandleInput %i\n", obj->id);
	   		  		//ret = 1;
				}
				break;

	   		  case LABEL_OBJTYPE_CCOBJECT:
	   		  	//printf("object: %i %i, %i %i, %i %i\n", obj->u.ccObj->pos.x, obj->u.ccObj->pos.y, x, y, pos->x, pos->y);
				if (isOverlap(pos, &obj->u.ccObj->pos, 0, 0, ccGetWidth(obj->u.ccObj->cc), ccGetHeight(obj->u.ccObj->cc))){
					int cx = abs(obj->u.ccObj->pos.x - pos->x);
					int cy = abs(obj->u.ccObj->pos.y - pos->y);
					ret = ccSendMessage(label, LABEL_MSG_CCOBJ_SELECTED, ((cx&0xFFFF)<<16)|(cy&0xFFFF), obj->id, pos);

					TTOUCHCOORD dpos;
					my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
					dpos.x = abs(obj->u.ccObj->pos.x - (pos->x + ccGetPositionX(obj->u.ccObj->cc)));
					dpos.y = abs(obj->u.ccObj->pos.y - (pos->y + ccGetPositionY(obj->u.ccObj->cc)));
					ret = ccHandleInput(obj->u.ccObj->cc, &dpos, flags);

					//printf("ccObj ccHandleInput %i %i %i, %i %i\n", obj->id, x, y, dpos.x, dpos.y);
					//ret = 1;
				}
				break;
			};
		}
		item = item->prev;
		//item = item->next;
	}

	return ret;
}

int labelStringSetLeftOffsetIndex (TLABEL *label, const int id, const int offset)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			if (offset < 0)
				text->charRenderOffset += abs(offset);
			else
				text->charRenderOffset = offset;
			ret = text->charRenderOffset;
		}
		ccUnlock(label);
	}
	return ret;
}

int labelStringGetLeftOffsetIndex (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text)
			ret = text->charRenderOffset;
		ccUnlock(label);
	}
	return ret;
}


int labelStringGetMetrics (TLABEL *label, const int id, int *x, int *y, int *width, int *height)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			ret = labelTextGetMetrics(text, x, y, width, height);
		}
		ccUnlock(label);
	}
	return ret;
}

int labelStringSetMaxWidth (TLABEL *label, const int id, const int width)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			ret = labelTextGetMaxWidth(text);
			
			//printf("labelStringSetMaxWidth: %i: %i -> %i\n", id, text->maxW, width);
			
			labelTextSetMaxWidth(text, width);
		}
		ccUnlock(label);
	}
	return ret;
}

int labelStringSetMaxHeight (TLABEL *label, const int id, const int height)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			ret = labelTextGetMaxHeight(text);
			labelTextSetMaxHeight(text, height);
		}
		ccUnlock(label);
	}
	return ret;
}

int labelStringSetMetricLimit (TLABEL *label, const int id, const int width, const int height)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			//printf("labelStringSetMetricLimit: %i: %i -> %i\n", id, text->maxW, width);

			labelTextSetMaxWidth(text, width);
			labelTextSetMaxHeight(text, height);
		}
		ccUnlock(label);
	}
	return ret;	
}

int labelStringSet (TLABEL *label, const int id, const char *str)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELTEXT *text = labelGetItem(label, id);
		if (text){
			text->offset.x = text->pos.x;
			text->offset.y = text->pos.y;
			
			if (text->uf.surface)
				ret = labelTextUpdateText(text, str);
			else
				ret = 1;
		}
		ccUnlock(label);
	}
	return ret;
}

int labelImageSet (TLABEL *label, const int id, TFRAME *img, const int resize)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image){
			labelImageSetImage(label, image, img, resize);
			if (image->scaleImage)
				labelImageSetScale(image, image->scaleBy);
			ret = 1;
		}
		ccUnlock(label);
	}
	return ret;
}


TFRAME *labelImageGet (TLABEL *label, const int id)
{
	TFRAME *ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image)
			ret = labelImageGetImage(label, image);
		ccUnlock(label);
	}
	return ret;
}

int labelArtcSet (TLABEL *label, const int id, const int imgId, const int resize)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image){
			labelArtcSetImage(label, image, imgId, resize);
			if (image->scaleImage)
				labelArtcSetScale(image, image->scaleBy);
			ret = 1;
		}
		ccUnlock(label);
	}
	return ret;
}

int labelArtcGet (TLABEL *label, const int id)
{
	int artId = 0;

	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image)
			artId = labelArtcGetImage(label, image);
		ccUnlock(label);
	}
	return artId;
}

int labelArtcGetSrc (TLABEL *label, const int id)
{
	return labelArtcGet(label, id);
}

TFRAME *labelImageGetSrc (TLABEL *label, const int id)
{
	TFRAME *ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image)
			ret = labelImageGetImageSrc(label, image);
		ccUnlock(label);
	}
	return ret;
}

int labelArtcGetMetrics (TLABEL *label, const int id, int *width, int *height)
{
	int ret = 0;

	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image){
			//int artId = labelArtcGetImageSrc(label, image);
			ret = artManagerImageGetMetrics(image->im, image->imgId, width, height);
			if (image->scaleAcquired > 0.0){
				if (width) *width *= image->scaleAcquired;
				if (height) *height *= image->scaleAcquired;
			}

			//printf("labelArtcGetMetrics %i %i\n", *width, *height);
		}
		ccUnlock(label);
	}
	return ret;
}

int labelArtcGetWidth (TLABEL *label, const int id)
{
	int width = 0;

	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image){
			//int artId = labelArtcGetImageSrc(label, image);
			artManagerImageGetMetrics(image->im, image->imgId, &width, NULL);
			if (image->scaleAcquired > 0.0)
				width *= image->scaleAcquired;

			//printf("labelArtcGetWidth %i\n", width);
		}
		ccUnlock(label);
	}
	return width;
}

int labelArtcGetHeight (TLABEL *label, const int id)
{
	int height = 0;

	if (ccLock(label)){
		TLABELARTCA *image = labelGetItem(label, id);
		if (image){
			//int artId = labelArtcGetImageSrc(label, image);
			artManagerImageGetMetrics(image->im, image->imgId, NULL, &height);
			if (image->scaleAcquired > 0.0)
				height *= image->scaleAcquired;

			//printf("labelArtcGetMetrics %i\n", height);
		}
		ccUnlock(label);
	}
	return height;
}

int labelImageGetWidth (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image){
			TFRAME *img = labelImageGetImageSrc(label, image);
			ret = img->width;
		}
		ccUnlock(label);
	}
	return ret;
}

int labelImageGetHeight (TLABEL *label, const int id)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELIMAGE *image = labelGetItem(label, id);
		if (image){
			TFRAME *img = labelImageGetImageSrc(label, image);
			ret = img->height;
		}
		ccUnlock(label);
	}
	return ret;
}

int labelCCSet (TLABEL *label, const int id, void *object)
{
	int ret = 0;
	if (ccLock(label)){
		TLABELCCOBJ *ccObj = labelGetItem(label, id);
		if (ccObj){
			labelCCObjSet(ccObj, object);
			ret = 1;
		}
		ccUnlock(label);
	}
	return ret;
}

void *labelCCGet (TLABEL *label, const int id)
{
	void *ret = 0;
	if (ccLock(label)){
		TLABELCCOBJ *ccObj = labelGetItem(label, id);
		if (ccObj)
			ret = labelCCObjGet(ccObj);
		ccUnlock(label);
	}
	return ret;
}

int labelTextCreateEx (TLABEL *label, const char *str, const int renderFlags, const char *font, const int xLocal, const int yLocal, const int renderType, const int64_t udata)
{
	int id = 0;
	if (ccLock(label)){
		id = labelObjsTextCreate(label, str, renderFlags, font, xLocal, yLocal, renderType, udata);
		ccUnlock(label);
	}
	return id;
}

int labelTextCreate (TLABEL *label, const char *str, const int flags, const char *font, const int xLocal, const int yLocal)
{
	int id = 0;
	if (ccLock(label)){
		id = labelObjsTextCreate(label, str, flags, font, xLocal, yLocal, 0, 0);
		ccUnlock(label);
	}
	return id;
}

int labelImageCreate (TLABEL *label, TFRAME *img, const int resize, const int xLocal, const int yLocal)
{
	int id = 0;
	if (ccLock(label)){
		id = labelObjImageCreate(label, img, resize, xLocal, yLocal);
		ccUnlock(label);
	}
	return id;
}

int labelImgcCreate (TLABEL *label, const int imgId, const int resize, const int xLocal, const int yLocal)
{
	int id = 0;
	if (ccLock(label)){
		void *im = ccGetImageManager(label->cc, CC_IMAGEMANAGER_IMAGE);
		id = labelObjImgcCreate(label, im, imgId, resize, xLocal, yLocal);
		ccUnlock(label);
	}
	return id;
}

int labelArtcCreateEx (TLABEL *label, const int artId, const double scale, const int resize, const int xLocal, const int yLocal, const int hoverState, const uint32_t hoverColour, const double hoverAlpha, const int64_t udata)
{
	int id = 0;
	if (ccLock(label)){
		void *am = ccGetImageManager(label->cc, CC_IMAGEMANAGER_ART);
		id = labelObjArtcCreate(label, am, artId, scale, resize, xLocal, yLocal, hoverState, hoverColour, hoverAlpha, udata);
		ccUnlock(label);
	}
	return id;
}

int labelArtcCreate (TLABEL *label, const int artId, const double scale, const int resize, const int xLocal, const int yLocal)
{
	int id = 0;
	if (ccLock(label)){
		void *am = ccGetImageManager(label->cc, CC_IMAGEMANAGER_ART);
		id = labelObjArtcCreate(label, am, artId, scale, resize, xLocal, yLocal, 0, 0, 0.0, 0);
		ccUnlock(label);
	}
	return id;
}

int labelCCCreate (TLABEL *label, void *ccObject, const int xLocal, const int yLocal)
{
	int id = 0;
	if (ccLock(label)){
		id = labelObjCCObjectCreate(label, ccObject, xLocal, yLocal);
		ccUnlock(label);
	}
	return id;
}

int labelItemDelete (TLABEL *label, const int id)
{
	//printf("labelItemDelete %i\n", id);
	
	int ret = 0;
	if (ccLock(label)){
		ret = labelObjDeleteObj(label, label->objs, id);
		ccUnlock(label);
	}
	return ret;
}

int labelItemPositionSet (TLABEL *label, const int id, const int xLocal, const int yLocal)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelObjSetPosition(label, id, xLocal, yLocal);
		ccUnlock(label);
	}
	return ret;
}

int labelItemPositionSet_us (TLABEL *label, const int id, const int xLocal, const int yLocal)
{
	return labelObjSetPosition(label, id, xLocal, yLocal);
}

int labelItemPositionGet (TLABEL *label, const int id, int *xLocal, int *yLocal)
{
	int ret = 0;
	if (ccLock(label)){
		ret = labelObjGetPosition(label, id, xLocal, yLocal);
		ccUnlock(label);
	}
	return ret;
}

static inline int labelInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TLABEL *label = (TLABEL*)object;
	//printf("labelInput: %i %i %i\n", pos->x, pos->y, flags);

	const int x1 = ccGetPositionX(label);
	const int x2 = x1 + ccGetWidth(label)-1;
	const int y1 = ccGetPositionY(label);
	const int y2 = y1 + ccGetHeight(label)-1;


	if (pos->y >= y1 && pos->y <= y2){
		if (pos->x >= x1 && pos->x <= x2){
			TTOUCHCOORD dpos;
			//my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
			//dpos.x = abs(x1 - pos->x);
			//dpos.y = abs(y1 - pos->y);

			//if (ret == 0){
				const int x = abs(x1 - pos->x);
				const int y = abs(y1 - pos->y);
				my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
				dpos.x = abs(x1 - pos->x);
				dpos.y = abs(y1 - pos->y);
				//printf("labelInput pen %i %i\n", dpos.pen, flags);
#if 0
				ccSendMessage(label, LABEL_MSG_BASE_SELECTED, ((x&0xFFFF)<<16)|(y&0xFFFF), flags, &dpos);
				my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
				dpos.x = abs(x1 - pos->x);
				dpos.y = abs(y1 - pos->y);
#endif
				int ret = 0;
				if (!flags)												// press down
					ret = ccSendMessage(label, LABEL_MSG_BASE_SELECTED_PRESS, ((x&0xFFFF)<<16)|(y&0xFFFF), flags, &dpos);
				else if (flags == 1 && pos->id == label->touchInputId)	// drag
					ret = ccSendMessage(label, LABEL_MSG_BASE_SELECTED_SLIDE, ((x&0xFFFF)<<16)|(y&0xFFFF), flags, &dpos);
				else if (flags == 3){									// up/release
					ret = ccSendMessage(label, LABEL_MSG_BASE_SELECTED_RELEASE, ((x&0xFFFF)<<16)|(y&0xFFFF), flags, &dpos);
					//printf("labelInput: Up ret %i\n", ret);
				}
			//}

			if (ret != -1){
				my_memcpy(&dpos, pos, sizeof(TTOUCHCOORD));
				dpos.x = abs(x1 - pos->x);
				dpos.y = abs(y1 - pos->y);
				/*int ret = */labelObjsHandleInput(label, label->objs, &dpos, flags);
				//printf("labelObjsHandleInput ret = %i, %i\n", ret, label->id);
			}
			return 1;
		}
	}
	return 0;
}

static inline int labelSetPosition (void *object, const int x, const int y)
{
	//printf("treeviewSetPosition %i %i\n", x, y);

	TLABEL *label = (TLABEL*)object;
	label->metrics.x = x;
	label->metrics.y = y;

	return 1;
}

static inline void labelObjsArtcResize (TLABELARTCA *image, int width, int height)
{
	if (image){
		if (image->drawable){

			int iwidth, iheight;
			artManagerImageGetMetrics(image->im, image->imgId, &iwidth, &iheight);
			/*if (image->scaleAcquired > 0.0){
				width *= image->scaleAcquired;
				height *= image->scaleAcquired;
			}*/

			int w = MIN(iwidth, width);
			int h = MIN(iheight, height);
			lResizeFrame(image->drawable, w, h, 1);
		}

		image->drawableDrawn = 0;
	}
}

static inline void labelObjsImageResize (TLABELIMAGE *image, const int width, const int height)
{
	if (image){
		lDeleteFrame(image->working);
		image->working = lCloneFrame(image->img);

		int w = MIN(image->img->width, width);
		int h = MIN(image->img->height, height);
		lResizeFrame(image->working, w, h, 1);
	}
}

static inline void labelObjsResize (TLABELOBJLIST *objs, const int width, const int height)
{
	if (!objs) return;

	TLISTITEM *item = objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){
			switch (obj->objType){
	   		  case LABEL_OBJTYPE_IMGCACHEID:
	   		  case LABEL_OBJTYPE_ARTCACHEID:
				labelObjsArtcResize(obj->u.artc, width, height);
				break;

			  case LABEL_OBJTYPE_TEXT:
			  	labelObjsTextResize(obj->u.text, width, height);
			  	break;

	   		  case LABEL_OBJTYPE_IMAGE:
				labelObjsImageResize(obj->u.image, width, height);
				break;

			  case LABEL_OBJTYPE_CCOBJECT:
			  	//ccSetMetrics(obj->u.ccObj, -1, -1, width, height);
			  	break;
			};
		}
		item = listGetNext(item);
	};
}

static inline int labelSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	//printf("tvSetMetrics %p, %i %i %i %i\n",object, x, y, width, height);

	TLABEL *label = (TLABEL*)object;

	ccSetPosition(label, x, y);
	label->metrics.width = width;
	label->metrics.height = height;

	labelObjsResize(label->objs, width, height);

	return 1;
}

static inline void labelEnable (void *object)
{
	TLABEL *label = (TLABEL*)object;
	label->enabled = 1;

	// todo: enable child CC objects
	if (!label->objs) return;

#if 1
	TLISTITEM *item = label->objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){
			switch (obj->objType){
			  case LABEL_OBJTYPE_CCOBJECT:
			  	if (obj->u.ccObj && obj->u.ccObj->cc)
			  		ccEnable(obj->u.ccObj->cc);
			  	break;
			};
		}
		item = listGetNext(item);
	};
#endif
}

static inline void labelDisable (void *object)
{
	TLABEL *label = (TLABEL*)object;
	label->enabled = 0;

	if (!label->objs) return;
#if 1
	// todo: disable child CC objects
	TLISTITEM *item = label->objs->head;
	while(item){
		TLABELOBJ *obj = listGetStorage(item);
		if (obj){
			switch (obj->objType){
			  case LABEL_OBJTYPE_CCOBJECT:
			  	if (obj->u.ccObj && obj->u.ccObj->cc)
			  		ccDisable(obj->u.ccObj->cc);
			  	break;
			};
		}
		item = listGetNext(item);
	};
#endif
}

static inline void labelDelete (void *object)
{
	TLABEL *label = (TLABEL*)object;
	//printf("labelDelete in %p %i (%i)\n", label, label->id, label->pageOwner);

	labelObjsFree(label);
}

int labelNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t label_cb, int *id, const int width, const int height)
{
	TLABEL *label = (TLABEL*)object;

	label->pageOwner = pageOwner;
	if (id) *id = label->id;
	label->type = type;

	label->cb.msg = label_cb;
	label->cb.create = labelNew;
	label->cb.free = labelDelete;
	label->cb.render = labelRender;
	label->cb.enable = labelEnable;
	label->cb.disable = labelDisable;
	label->cb.input = labelInput;
	label->cb.setPosition = labelSetPosition;
	label->cb.setMetrics = labelSetMetrics;


	label->idSrc = 100;
	label->objs	= labelObjsAlloc();

	label->canDrag = 0;
	label->metrics.x = -1;
	label->metrics.y = -1;
	label->metrics.width = width;
	label->metrics.height = height;

	label->renderflags = LABEL_RENDER_BORDER_PRE | LABEL_RENDER_BORDER_POST;
	label->renderflags |= LABEL_RENDER_BLUR | LABEL_RENDER_BASE;
	label->renderflags |= LABEL_RENDER_CCOBJ | LABEL_RENDER_IMAGE | LABEL_RENDER_TEXT;


	//labelBaseSetColour(&label->base, 80<<24 | COL_BLUE_SEA_TINT);
	labelBaseSetColour(&label->base, 0x779777);

	uint32_t col_pre[] = {
		120<<24 | COL_BLUE_SEA_TINT,
		100<<24 | COL_BLUE_SEA_TINT,
		 70<<24 | COL_CYAN
		};
	labelBorderSetProfile(&label->border, LABEL_BORDER_SET_PRE, col_pre, 3);

	uint32_t col_post[] = {
		255<<24 | COL_BLUE_SEA_TINT,
		180<<24 | COL_BLUE_SEA_TINT,
		 70<<24 | COL_BLUE_SEA_TINT
		};
	labelBorderSetProfile(&label->border, LABEL_BORDER_SET_POST, col_post, 3);

	//printf("ccLabelNew %p %i (%i)\n", label, label->id, label->pageOwner);

	return 1;
}

