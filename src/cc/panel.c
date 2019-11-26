
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




// helper function, find/create a new artManager id
int genAMId (TPANEL *panel, wchar_t *path)
{
	wchar_t buffer[MAX_PATH+1];
	void *am = ccGetImageManager(panel->cc, CC_IMAGEMANAGER_ART);
	return artManagerImageAdd(am, com_buildSkinD(panel->cc,buffer,path));
}

int panelImgAttributeCheck (TPANEL *panel, const int id, const int flag)
{
	TPANELIMG *img = panelImgGetItem(panel, id);
	if (img)
		return ((img->attributes&flag) != 0);
	return -1;
}

void panelImgAttributeSet (TPANEL *panel, const int id, const int flag)
{
	TPANELIMG *img = panelImgGetItem(panel, id);
	if (img)
		img->attributes |= flag;
}

void panelImgAttributeClear (TPANEL *panel, const int id, const int flag)
{
	TPANELIMG *img = panelImgGetItem(panel, id);
	if (img)
		img->attributes &= ~flag;
}

TPANELIMG *panelImgGetNext (TPANEL *panel, int *idx)
{
	(*idx)++;
	for (int i = *idx; i < panel->listSize; i++){
		TPANELIMG *img = panel->list[i];
		if (img->isActive){
			if (idx) *idx = i;
			return img;
		}
	}
	return NULL;
}

TPANELIMG *panelImgGetFirst (TPANEL *panel, int *idx)
{
	for (int i = *idx; i < panel->listSize; i++){
		TPANELIMG *img = panel->list[i];
		if (img && img->isActive){
			if (idx) *idx = i;
			return img;
		}
	}
	return NULL;
}

TPANELIMG *panelImgGetItem (TPANEL *panel, const int id)
{
	for (int i = 0; i < panel->listSize; i++){
		TPANELIMG *img = panel->list[i];
		if (img && img->isActive){
			if (img->id == id)
				return img;
		}
	}
	return NULL;
}

int panelImgStorageSet (TPANEL *panel, const int id, void *ptr)
{
	TPANELIMG *img = panelImgGetItem(panel, id);
	if (img)
		img->storage = ptr;
	return img != NULL;
}

void *panelImgStorageGet (TPANEL *panel, const int id)
{
	TPANELIMG *img = panelImgGetItem(panel, id);
	if (img)
		return img->storage;
	else
		return NULL;
}

int panelImgGetTotal (TPANEL *panel)
{
	if (panel->itemTotal){
		int ct = 0;
		for (int i = 0; i < panel->listSize; i++)
			ct += panel->list[i]->isActive;
		return ct;
	}
	return 0;
}

static inline void panelImgLabelDelete (TPANELLABEL *label)
{
	if (label->text){
		my_free(label->text);
		label->text = NULL;
	}
	if (label->strFrm){
		lDeleteFrame(label->strFrm);
		label->strFrm = NULL;
	}
}

static inline void panelImgDelete (TPANEL *panel, TPANELIMG *img)
{
	if (img->btn){
		ccDelete(img->btn);
		img->btn = NULL;
	}

	for (int i = 0; i < PANEL_ITEM_LABELTOTAL; i++){
		panelImgLabelDelete(&img->labels[i]);
	}
}

void panelListDelete (TPANEL *panel)
{
	if (panel->itemTotal){
		for (int i = 0; i < panel->listSize; i++){
			if (panel->list[i]){
				// todo
				// ccSendMessage a per item message to signal cleanup and allow user side storage cleanup
				ccSendMessage(panel, PANEL_MSG_ITEMDELETE, panel->list[i]->id, i, panel->list[i]);
				panelImgDelete(panel, panel->list[i]);
				my_free(panel->list[i]);
			}
		}

		my_free(panel->list);
		panel->list = NULL;
		panel->itemTotal = 0;

	}else if (panel->list || panel->listSize){
		//printf("panel->list %p %i\n", panel->list, panel->listSize);
	}

}

int panelListResize (TPANEL *panel, const int newSize, const int keep)
{
	//printf("panelListResize %i %i\n", newSize, keep);

	if (!keep/* && panel->itemTotal*/)
		panelListDelete(panel);


	if (!keep){
		panel->list = (TPANELIMG**)my_calloc(newSize+1, sizeof(TPANELIMG*));
		if (panel->list){
			for (int i = 0; i < newSize; i++){
				panel->list[i] = (TPANELIMG*)my_calloc(1, sizeof(TPANELIMG));
				if (!panel->list[i]){
					//printf("panelListResize: error allocating memory %i\n", sizeof(TPANELIMG));
					return 0;
				}
			}

			panel->itemTotal = newSize;
			panel->listSize = newSize;
			//printf("panelList resized to %i (%i)\n", newSize, keep);
			return 1;
		}
	}else{

		panel->list = (TPANELIMG**)my_realloc(panel->list, (newSize+1) * sizeof(TPANELIMG*));
		if (panel->list){
			for (int i = panel->listSize; i < newSize; i++){
				panel->list[i] = (TPANELIMG*)my_calloc(1, sizeof(TPANELIMG));
				if (!panel->list[i]){
					//printf("panelListResize: error allocating memory (%i)\n", sizeof(TPANELIMG));
					return 0;
				}
			}
			panel->itemTotal = newSize;
			panel->listSize = newSize;
			//printf("panelList resized to %i (%i)\n", newSize, keep);
			return 1;
		}
	}

	return 0;
}

TPANELIMG *panelGetItem (TPANEL *panel, const int id)
{
	for (int i = 0; i < panel->listSize; i++){
		TPANELIMG *item = panel->list[i];
		if (item->id == id)
			return item;
	}
	return NULL;
}

static inline int64_t ccbtn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg == CC_MSG_RENDER || msg == CC_MSG_INPUT) return 1;

	//TCCOBJECT *obj = (TCCOBJECT*)object;
	//printf("ccbtn_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", obj->id, obj->type, msg, (int)data1, (int)data2, dataPtr);

	TCCBUTTON *button = (TCCBUTTON*)object;
	//const int id = (int)data2;


	if (msg == BUTTON_MSG_SELECTED_PRESS){
		TPANEL *panel = (TPANEL*)ccGetUserData(button);
		TPANELIMG *item = panelGetItem(panel, button->id);
		
		//printf("panel ccbtn_cb, msg:%i, data1:%i, data2:%i, ptr:%p, %i %i\n", msg, (int)data1, (int)data2, dataPtr, button->id, item->id);
		ccSendMessage(panel, PANEL_MSG_ITEMSELECTED, item->id, item->attributes, item->udataPtr);
		return 1;
	}else {
		return 0;
	}
}

static inline int panelListInsertLast (TPANEL *panel, const int artId, const int artIdAlt, const double scale, char *label, void *udataPtr)
{
	if (!panel->itemTotal)
		panelListResize(panel, 1, 0);

	//printf("panel->itemTotal %i %i '%s'\n", panel->itemTotal, panel->listSize, label);

	if (panel->itemTotal){
		TPANELIMG *item = NULL;
		for (int i = 0; i < panel->listSize && !item; i++){
			if (!panel->list[i]->isActive){
				item = panel->list[i];
				break;
			}
		}

		if (!item){
			panelListResize(panel, panel->listSize+8, 1);

			for (int i = 0; i < panel->listSize && !item; i++){
				if (!panel->list[i]->isActive){
					item = panel->list[i];
					break;
				}
			}
		}

		if (item){
			if (!item->isActive){
				item->isActive = 1;
				//item->id = ++panel->objIdSrc;
				item->udataPtr = udataPtr;
				item->attributes = 0;
				item->storage = NULL;

				item->labels[0].text = my_strdup(label);
				item->labels[0].strFrm = NULL;
				item->labels[0].colour = 240<<24|COL_WHITE;

				/*for (int i = 1; i < PANEL_ITEM_LABELTOTAL; i++){
					item->labels[i].text = NULL;
					item->labels[i].strFrm = NULL;
					item->labels[i].colour = item->labels[0].colour;
				}*/


				memset(&item->area, 0, sizeof(item->area));
				item->btn = ccCreate(panel->cc, panel->pageOwner, CC_BUTTON2, ccbtn_cb, &item->id, panel->itemOffset->x, panel->itemOffset->y);
				//printf("create: %p, %i\n", item->button, item->button->id);
				
				item->btn->isChild = 1;
				ccSetUserData(item->btn, panel);
								
				/*int itemId = */button2FaceImgcSet(item->btn, artId, artIdAlt, scale, 0, 0);
				
				button2FaceConstraintsSet(item->btn, panel->metrics.x, panel->metrics.y, panel->metrics.x + panel->metrics.width-1, panel->metrics.y + panel->metrics.height-1);
				button2FaceConstraintsEnable(item->btn);
				button2FaceAutoSwapDisable(item->btn);
				button2AnimateSet(item->btn, 0);
				button2FaceHoverSet(item->btn, 1, COL_HOVER, 0.8);
				//if (artIdAlt)
				//	buttonFaceAutoSwapDisable(item->button);
				ccEnable(item->btn);

				//wprintf(L"panelListAdd %i: '%s'\n", item->id, path);

				return item->id;
			}
		}
	}

	//wprintf(L"panelListAdd failed %i '%s'\n", panel->listSize, path);
	return 0;
}

void panelImgTextMetricsCalc (TPANEL *panel, THWD *hw, const int font, TPANELIMG **list, const int total)
{
	//printf("panelImgPositionCalc %i %i %i %i\n", metrics->x, metrics->y, metrics->width, metrics->height);

	for (int i = 0; i < total; i++){
		TPANELIMG *img = list[i];
		
		if (!img || !img->isActive) continue;
		if (!img->btn->enabled) continue;

		for (int l = 0; l < PANEL_ITEM_LABELTOTAL; l++){
			TPANELLABEL *label  = &img->labels[l];
			if (!label->text) break;

			label->font = font;
			lGetTextMetrics(hw, label->text, /*PF_FORCEAUTOWIDTH|PF_CLIPDRAW|*/PF_IGNOREFORMATTING, label->font, &label->metrics.width, &label->metrics.height);

			if (label->metrics.width > panel->itemMaxWidth)
				label->metrics.width = panel->itemMaxWidth;
			label->metrics.width -= 2;
		}
	}
}

int panelImgPositionMetricsCalc (TPANELIMG **list, const int total, TMETRICS *metrics, const int horiSpace, const int vertSpace)
{
	//printf("panelImgPositionCalc %i %i %i %i\n", metrics->x, metrics->y, metrics->width, metrics->height);

	//rect_t pos = {metrics->x, metrics->y, metrics->x+metrics->width-1, 0};
	rect_t pos = {0, 0, metrics->x+metrics->width-1, 0};

	for (int i = 0; i < total; i++){
		TPANELIMG *img = list[i];
		if (!img || !img->isActive) continue;
		if (!img->btn->enabled) continue;

		int bwidth = ccGetWidth(img->btn);
		int bheight = ccGetHeight(img->btn);
		int textWidth = 0;
		int textHeight = 0;

		for (int l = 0; l < PANEL_ITEM_LABELTOTAL; l++){
			TPANELLABEL *label  = &img->labels[l];
			if (!label->text) break;

			if (label->metrics.width > textWidth)
				textWidth = label->metrics.width;

			textHeight += label->metrics.height;
		}

		if (textWidth > bwidth) bwidth = textWidth;

		if (pos.x1+bwidth > pos.x2){
			pos.x1 = metrics->x;
			pos.y1 = pos.y2 + vertSpace;
			pos.y2 = pos.y1;
		}

		img->area.x1 = pos.x1;
		img->area.x2 = img->area.x1 + bwidth;
		pos.x1 += bwidth + horiSpace;
		img->area.y1 = pos.y1;

		for (int l = 0, h = 0; l < PANEL_ITEM_LABELTOTAL; l++){
			TPANELLABEL *label  = &img->labels[l];
			if (!label->text) break;

			label->metrics.x = img->area.x1 + abs((img->area.x2-img->area.x1) - label->metrics.width)/2;
			label->metrics.y = img->area.y1 + bheight + h;
			h += label->metrics.height;

			//printf("met calc: %i '%s' %i %i\n", l, label->text, label->metrics.x, label->metrics.y);
		}


		if (!(img->attributes&PANEL_ITEM_SEPARATOR))
			bheight += textHeight;

		img->area.y2 = img->area.y1 + bheight-1;
		if (img->area.y2 > pos.y2) pos.y2 = img->area.y2;

		//if (img->area.y1 < metrics->y) img->area.y1 = metrics->y;
		//if (img->area.y2 < metrics->y) img->area.y2 = metrics->y;
	}

	return pos.y2;	// returns virtual height of panel
}

int panelInvalidate (TPANEL *panel)
{

	TMETRICS metrics = {panel->metrics.x, panel->metrics.y, panel->metrics.width, panel->metrics.height};
	panelImgTextMetricsCalc(panel, panel->cc->hSurfaceLib, panel->font, panel->list, panel->listSize);
	panel->vHeight = panelImgPositionMetricsCalc(panel->list, panel->listSize, &metrics, panel->itemHoriSpace, panel->itemVertSpace);
	return panel->vHeight;
}

int panelImgAddSubtext (TPANEL *panel, const int id, char *text, const int colour)
{
	TPANELIMG *item = panelImgGetItem(panel, id);
	if (!item) return -1;

	for (int i = 0; i < PANEL_ITEM_LABELTOTAL; i++){
		TPANELLABEL *label  = &item->labels[i];
		if (!label->text){
			label->text = my_strdup(text);
			label->colour = colour;
			return i;
		}
	}

	return -2;
}

int panelImgAdd (TPANEL *panel, const int artId, char *label, void *udataPtr)
{
	int id = panelListInsertLast(panel, artId, 0, 1.0, label, udataPtr);
	//wprintf(L"panelImgAdd id:%i '%s'\n", id, path);

	return id;
}

int panelImgAddEx (TPANEL *panel, const int artId, const int artIdAlt, const double scale, char *label, void *udataPtr)
{
	int id = panelListInsertLast(panel, artId, artIdAlt, scale, label, udataPtr);
	//wprintf(L"panelImgAddEx id:%i '%s'\n", id, path);

	return id;
}

static inline int isInPanel (TPANEL *panel, const int x, const int y)
{
	if (x >= panel->metrics.x && x < panel->metrics.x+panel->metrics.width){
		if (y >= panel->metrics.y && y < panel->metrics.y+panel->metrics.height)
			return 1;
	}
	return 0;
}

void panelEnable (void *object)
{
	TPANEL *panel = (TPANEL*)object;
	panel->enabled = 1;
}

void panelDisable (void *object)
{
	TPANEL *panel = (TPANEL*)object;
	panel->enabled = 0;
}

int panelRender (void *object, TFRAME *frame)
{
	TPANEL *panel = (TPANEL*)object;
	TTOUCHSWIPE *swipe = &panel->swipe;

	if (swipe->decayAdjust > swipe->decayRate){
		swipe->decayAdjust -= swipe->decayRate;
		if (swipe->dy > 0)
			swipe->adjust += (swipe->decayAdjust * swipe->decayFactor);
		else
			swipe->adjust -= (swipe->decayAdjust * swipe->decayFactor);
	}else{
		swipe->decayAdjust = 0.0;
	}

	const int adjust = (int)swipe->adjust;
	if (abs(adjust) >= swipe->dragMinV>>1){
		int value = swipe->i32value - adjust;
		if (value < 0){
			panel->itemOffset->y = 0;
			swipe->decayAdjust = 0.0;

		}else if (value > panel->vHeight-21){
			panel->itemOffset->y = panel->vHeight-21;
			swipe->decayAdjust = 0.0;

		}else{
			panel->itemOffset->y = value;
		}
	}

	const int posX = panel->metrics.x + panel->itemOffset->x;
	const int posY = panel->metrics.y - panel->itemOffset->y;
	const rect_t constraint = {panel->metrics.x, panel->metrics.y, panel->metrics.x+panel->metrics.width-1, panel->metrics.y+panel->metrics.height-1};
	TMETRICS metrics = {1, 0, 0, 0};


	const int blurOp = LTR_BLUR4;
	lSetRenderEffect(frame->hw, blurOp);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_COLOUR, COL_BLUE_SEA_TINT);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_RADIUS, 3);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_SETTOP, 1);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_X, 1);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_Y, 1);
	lSetFilterAttribute(frame->hw, blurOp, LTRA_BLUR_ALPHA, 1000);


	int idx = 0;
	TPANELIMG *img;
	while((img=panelImgGetFirst(panel, &idx))){
		idx++;

		if (img->btn->enabled){
			int xOffset = abs(ccGetWidth(img->btn) - ((img->area.x2-img->area.x1)+1))/2;		// center image horizontally
			const int y = posY + img->area.y1;
			const int y1 = (y + img->btn->metrics.height) - panel->metrics.y;
			ccSetPosition(img->btn, posX + img->area.x1 + xOffset, y);
			
			if (y1 >= 0 && y < panel->metrics.height+panel->metrics.y){
				//printf("render: %i %i, %i %i, %i %i, %i\n", idx, img->btn->id, img->area.y1, y, y+panel->itemOffset->y, panel->itemOffset->y, (y + img->btn->metrics.height)-panel->metrics.y);
				ccRender(img->btn, frame);
			}

			if (y >= panel->metrics.height+panel->metrics.y)
				continue;

			if (!(img->attributes&PANEL_ITEM_SEPARATOR)){
				for (int i = 0; i < PANEL_ITEM_LABELTOTAL; i++){
					TPANELLABEL *label = &img->labels[i];
					if (!label->text) break;

					if (posY + label->metrics.y + 24 >= constraint.y1 && posY + label->metrics.y <= constraint.y2){
						if (!label->strFrm){
							lSetForegroundColour(frame->hw, label->colour);
							label->strFrm = com_newStringEx(frame->hw, &metrics, LFRM_BPP_32A, 0, /*label->font*/NULL, label->text, panel->itemMaxWidth, NSEX_RIGHT);
						}

						if (label->strFrm){
							TFRAME *text = label->strFrm;

							/*if (img->attributes&PANEL_ITEM_SEPARATOR){
								label->metrics.x = constraint.x2-1 - text->width;
								//label->metrics.y = text->height;
							}*/

							rect_t pos = {posX+label->metrics.x, posY+label->metrics.y, posX+label->metrics.x+text->width-1, posY+label->metrics.y+text->height-1};

							int x = 0, y = 0;
							if (pos.x1 < constraint.x1){
								x = abs(constraint.x1 - pos.x1);
								pos.x1 += abs(constraint.x1 - pos.x1);
							}
							if (pos.y1 < constraint.y1){
								y = abs(constraint.y1 - pos.y1);
								pos.y1 += abs(constraint.y1 - pos.y1);
							}
							if (pos.x2 > constraint.x2)
								pos.x2 -= abs(constraint.x2 - pos.x2);
							if (pos.y2 > constraint.y2)
								pos.y2 -= abs(constraint.y2 - pos.y2);

							com_copyArea(text, frame, pos.x1, pos.y1, x, y, (pos.x2 - pos.x1), y+(pos.y2 - pos.y1));
						}
#if DRAWMISCDETAIL
						lDrawRectangle(frame, posX + label->metrics.x, posY + label->metrics.y, posX + label->metrics.x+label->metrics.width, posY + label->metrics.y+label->metrics.height, 0xFF00FF00);
#endif
					}
				}
			}
#if DRAWTOUCHRECTS
			lDrawRectangle(frame, posX + img->area.x1, posY + img->area.y1, posX + img->area.x2, posY + img->area.y2, DRAWTOUCHRECTCOL);
#endif
		}
	}

	lDrawLine(frame, panel->metrics.x+2, panel->metrics.y-1, panel->metrics.x+panel->metrics.width-3, panel->metrics.y-1, 255<<24|COL_BLUE_SEA_TINT);
	lSetRenderEffect(frame->hw, LTR_DEFAULT);

	const uint32_t dt = buttonsRenderAll(panel->btns, frame, BUTTONS_RENDER_HOVER|BUTTONS_RENDER_ANIMATE);
	if (swipe->decayAdjust > 0.0 || dt < 1000)
		com_setTargetRate(panel->cc, 25);
	else
		com_setTargetRate(panel->cc, UPDATERATE_BASE);


	return 1;
}

int panelInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TPANEL *panel = (TPANEL*)object;
	if (!panel->inputEnabled) return 0;

	//const int x = pos->x - panel->metrics.x;
	//const int y = pos->y - panel->metrics.y;
	//printf("panelInput %i %i, %i %i\n", x, y, panel->canDrag, panel->dragEnabled);

	TTOUCHSWIPE *swipe = &panel->swipe;
	//static double preDt;

	/*if (!isInPanel(panel, pos->x, pos->y) && !flags){
		int ret = buttonsCheckButtonPress(panel->buttons, panel->cc->vp, pos, flags);

		if (ret) swipeReset(swipe);
		return 0;
	}*/

	if (panel->dragEnabled){
		if (!pos->pen && flags == 0 && pos->dt > 80){			// pen down
			swipe->state = 1;
			swipe->t0 = pos->time;//getTime(panel->cc->vp);
			swipe->sx = pos->x;
			swipe->sy = pos->y;
			swipe->adjust = 0.0;
			swipe->decayAdjust = 0.0;
			swipe->i32value = panel->itemOffset->y;

			//printf("\npen down\n");

		}else if (!pos->pen && swipe->state == 1 && flags == 1){	// dragging
			if (swipe->t0 < 1.0)
				swipe->t0 = pos->time;//getTime(panel->cc->vp);
			swipe->ex = pos->x;
			swipe->ey = pos->y;
			swipe->dx = swipe->ex - swipe->sx;
			swipe->dy = swipe->ey - swipe->sy;
			swipe->dt = pos->time /*getTime(panel->cc->vp)*/ - swipe->t0;

			//dbprintf(panel->cc->vp, "%f, %.4f %.4f\n", swipe->t0, swipe->dt, swipe->dt-preDt);
			//preDt = swipe->dt;
			
			if (swipe->dt > 0.0){
				double velocity = swipe->dy / swipe->dt;
				swipe->velocity = velocity;
			}
			
			int dt = (int)(swipe->dt*10);
			if (dt && fabs(swipe->dy) >= swipe->dragMinV){
				swipe->adjust = swipe->dy;
				swipe->decayFactor = fabs(swipe->velocity * 10.0);

				//printf("drag: %.1f %i %.4f\n", swipe->adjust, dt, swipe->velocity);
				com_renderSignalUpdate(panel->cc);
				return 0;
			}

		}else if (pos->pen && swipe->state == 1 && flags == 3){	// pen up
			swipe->ex = pos->x;
			swipe->ey = pos->y;
			swipe->dx = swipe->ex - swipe->sx;
			swipe->dy = swipe->ey - swipe->sy;
			swipe->state = 0;

			swipe->dt = com_getTime(panel->cc) - swipe->t0;
			if (swipe->dt > 0.0){
				double velocity = swipe->dy / swipe->dt;
				swipe->velocity = velocity;
			}
			
			if (swipe->dt > 0 && abs(swipe->dy) > swipe->dragMinV){
				swipe->decayAdjust = abs(swipe->dy)/(double)swipe->dt;
				swipe->decayAdjust *= swipe->velocityFactor;
				swipe->decayFactor = fabs(swipe->velocity * 10.0);
			}

			//printf("pen up\n");
			
			if (abs(swipe->dy) > swipe->dragMinV){
				com_renderSignalUpdate(panel->cc);
				//swipeReset(swipe);
				return 0;
			}
		}
	}else{
		com_swipeReset(swipe);
	}


	if (!ccPositionIsOverlapped(panel, pos->x, pos->y)){
		//printf("panelInput ccPositionIsOverlapped == 0\n");
		return 0;
	}
#if 1
	int idx = 0;
	TPANELIMG *img;
	while((img=panelImgGetFirst(panel, &idx))){
		idx++;

		//printf("input in: %p, %i\n", img->btn, img->btn->id);

		if (!flags && img->btn && img->btn->enabled){
			int ret = ccHandleInput(img->btn, pos, flags);
			if (ret > 0){
				if (panel->btns)
					panel->btns->t0 = com_getTickCount();

				com_swipeReset(swipe);
				return ret;
			}
		}
		
		//printf("input out b: %p, %i\n", img->button, img->button->id);
	}
#endif
	return 0;
}

int panelSetPosition (void *object, const int x, const int y)
{
	TPANEL *panel = (TPANEL*)object;
	panel->metrics.x = x;
	panel->metrics.y = y;
	return 1;
}

int panelSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TPANEL *panel = (TPANEL*)object;
	panel->metrics.width = width;
	panel->metrics.height = height;
	return ccSetPosition(panel, x, y);
}

void panelDelete (void *object)
{
	TPANEL *panel = (TPANEL*)object;
	if (panel){
		panelListDelete(panel);

		if (panel->btns)
			buttonsDeleteAll(panel->btns);
	}
}

TCCBUTTON *panelSetButton (TPANEL *panel, const int btn_id, wchar_t *img1, wchar_t *img2, const void *cb)
{
	TCCBUTTON *btn = buttonsCreateButton(panel->btns, img1, img2, btn_id, 1, 0, 0, 0);
	btn->cb.msg = cb;
	ccSetUserData(btn, panel);
	ccSetPosition(btn, panel->metrics.x, panel->metrics.y);
	return btn;
}

void panelSetBoundarySpace (TPANEL *panel, const int Vpad, const int Hpad)
{
	panel->itemVertSpace = Vpad;		// vertical padding between images, in pixels
	panel->itemHoriSpace = Hpad;
}

int panelNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t panel_cb, int *id, const int buttonTotal, const int var2)
{
	TPANEL *panel = (TPANEL*)object;

	panel->pageOwner = pageOwner;
	if (id) *id = panel->id;
	panel->type = type;

	panel->cb.msg = panel_cb;
	panel->cb.render = panelRender;
	panel->cb.create = panelNew;
	panel->cb.free = panelDelete;
	panel->cb.enable = panelEnable;
	panel->cb.disable = panelDisable;
	panel->cb.input = panelInput;
	panel->cb.setPosition = panelSetPosition;
	panel->cb.setMetrics = panelSetMetrics;

	panel->canDrag = 1;
	panel->inputEnabled = 1;
	panel->objIdSrc = 0;

	panel->dragEnabled = 1;

	panel->itemTotal = 0;		// number of images added
	panel->itemVertSpace = 8;	// vertical padding between images, in pixels
	panel->itemHoriSpace = 16;	// horizontal padding
	//panel->itemOffset->x = 0;	// offset each and every image location by point_t:x/y
	//panel->itemOffset->y = 0;
	panel->itemMaxWidth = panel->cc->dwidth * 0.27;
	panel->font = PANEL_FONT;
	panel->list = NULL;		// item container
	panel->listSize = 0;

	if (buttonTotal)
		panel->btns = buttonsCreate(panel->cc, pageOwner, buttonTotal, ccbtn_cb);

	TTOUCHSWIPE *swipe = &panel->swipe;
	swipe->state = 0;
	swipe->t0 = 0.0;	// drag state time
	swipe->dt = 0.0;	// drag end time
	swipe->sx = 0;	// start
	swipe->sy = 0;
	swipe->ex = 0;	// end
	swipe->ey = 0;
	swipe->dx = 0;	// delta
	swipe->dy = 0;
	swipe->dragMinV = 8;	// (14 pixels @ 272 height)
	swipe->dragMinH = swipe->dragMinV;
	swipe->velocityFactor = 1.60;
	swipe->adjust = 0.0;
	swipe->decayRate = 0.010;
	swipe->decayFactor = 10.000;


	return 1;
}
