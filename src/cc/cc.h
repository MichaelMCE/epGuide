
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





#ifndef _CC_H_
#define _CC_H_


#define CC_FONT_HANDLES			(16)			// maximum number of fonts that may be open per label control


enum _CCOBJTYPE
{
	CC_LABEL					= 10,
	CC_SLIDER_HORIZONTAL		= 20,
	CC_SLIDER_VERTICAL			= 25,
	CC_SCROLLBAR_HORIZONTAL		= 30,
	CC_SCROLLBAR_VERTICAL		= 35,
	CC_LISTBOX					= 40,
	CC_PANEL					= 50,
	CC_TV						= 60,	/* treeview control */
	CC_BUTTON					= 70,
	CC_KEYPAD					= 80,
	CC_BUTTON2					= 90,
	CC_PANE						= 100,
	CC_GRAPH					= 110
};

enum _ccmsg
{
	CC_MSG_ENABLED				= 100,
	CC_MSG_DISABLED,			//101
	
	CC_MSG_RENDER				= 110,
	CC_MSG_SETPOSITION,			//111
	
	CC_MSG_CREATE				= 120,
	CC_MSG_DELETE,				//121
	
	CC_MSG_INPUT				= 130,
	CC_MSG_HOVER,				//131

	SLIDER_MSG_VALCHANGED 		= 200,
	SLIDER_MSG_VALSET,

	LISTBOX_MSG_VALCHANGED		= 300,
	LISTBOX_MSG_ITEMSELECTED,
	LISTBOX_MSG_VALIDATED,

	SCROLLBAR_MSG_VALCHANGED	= 400,

	LABEL_MSG_DELETE			= 500,
	
	LABEL_MSG_TEXT_SELECTED_PRESS,
	LABEL_MSG_TEXT_SELECTED_SLIDE,
	LABEL_MSG_TEXT_SELECTED_RELEASE,
	
	LABEL_MSG_IMAGE_SELECTED_PRESS,
	LABEL_MSG_IMAGE_SELECTED_SLIDE,
	LABEL_MSG_IMAGE_SELECTED_RELEASE,
	
	LABEL_MSG_CCOBJ_SELECTED,

	LABEL_MSG_BASE_SELECTED_PRESS,	// 508
	LABEL_MSG_BASE_SELECTED_SLIDE,
	LABEL_MSG_BASE_SELECTED_RELEASE,

	BUTTON_MSG_SELECTED_PRESS	= 600,
	BUTTON_MSG_SELECTED_SLIDE,
	BUTTON_MSG_SELECTED_RELEASE,

	PANEL_MSG_ITEMSELECTED		= 700,
	PANEL_MSG_ITEMDELETE,

	TV_MSG_ITEMSELECT			= 800,
	TV_MSG_IMAGESELECT,
	TV_MSG_EXPANDERSTATE,
	TV_MSG_CHECKBOXSTATE,
	TV_MSG_ITEMDROP,
	TV_MSG_SB_INPUT,
	TV_MSG_KEYPAD_OPENED,
	TV_MSG_KEYPAD_CLOSED,
		
	KP_MSG_PAD_OPENED			= 900,
	KP_MSG_PAD_CLOSED,
	KP_MSG_PAD_PRESS,
	KP_MSG_PAD_ENTER,
	KP_MSG_PAD_RENDER,
	
	PANE_MSG_TEXT_SELECTED		= 1000,		
	PANE_MSG_TEXT_SELECT_HELD,		
	PANE_MSG_IMAGE_SELECTED,				// image/text was pressed/clicked
	PANE_MSG_IMAGE_SELECT_HELD,				// image/text was pressed and held before release
	PANE_MSG_ITEM_ENABLED,
	PANE_MSG_ITEM_DISABLED,
	PANE_MSG_SLIDE_HELD,					// grabbed item (a)
	PANE_MSG_SLIDE_HOVER,					// grabbed item (a) which is hovering over item (b)
	PANE_MSG_SLIDE_RELEASE,					// grabbed item (a) which was released over item (b, if at all)
	PANE_MSG_SLIDE,
	PANE_MSG_BASE_SELECTED_PRESS,	// 1010
	PANE_MSG_BASE_SELECTED_RELEASE,
	PANE_MSG_BASE_SELECTED,					// if not a slide/move, sent after a PANE_MSG_BASE_SELECTED_RELEASE
	PANE_MSG_CELL_SELECTED_PRESS,
	PANE_MSG_CELL_SELECTED_SLIDE,
	PANE_MSG_CELL_SELECTED_RELEASE,
	PANE_MSG_CELL_DELETE,
	PANE_MSG_VALIDATED				// 1017
	
	//GRAPH_MSG_				= 1100
};

enum _IMAGEMAN
{
	CC_IMAGEMANAGER_IMAGE,		// anthing which isn't the above, eg; image viewer
	CC_IMAGEMANAGER_ART = CC_IMAGEMANAGER_IMAGE,		// album/track covers
//	CC_IMAGEMANAGER_SKIN,
	CC_IMAGEMANAGER_TOTAL
};

enum _IMGC
{
	IMGC_SHADOW_BLK,
	IMGC_SHADOW_BLU,
	IMGC_SHADOW_GRN,
	IMGC_SWATCH,
	IMGC_TOTAL,
};


typedef struct{
	int x;
	int y;
}point_t;

typedef struct{
	int x1;
	int y1;
	int x2;
	int y2;
}rect_t;

typedef struct{
	int isBuilt;
	TFRAME *topLeft;
	TFRAME *topRight;
	TFRAME *btmLeft;
	TFRAME *btmRight;
	TFRAME *vertBarTop;
	TFRAME *vertBarBtm;
	TFRAME *horiBarLeft;
	TFRAME *horiBarRight;
}TSHADOWUNDER;


typedef struct TGUIINPUT TGUIINPUT;
typedef struct TTOUCHSWIPE TTOUCHSWIPE;

typedef struct TCC TCC;
typedef struct TCCOBJ TCCOBJ;
typedef struct TCCOBJECT TCCOBJECT;
typedef struct TSLIDER TSLIDER;
typedef struct TLISTBOX TLISTBOX;
//typedef struct TLB TLB;
typedef struct TSCROLLBAR TSCROLLBAR;
typedef struct TTV TTV;
typedef struct TPANEL TPANEL;
typedef struct TLABEL TLABEL;
typedef struct TCCBUTTON TCCBUTTON;
typedef struct TCCBUTTON2 TCCBUTTON2;
typedef struct TCCBUTTONS TCCBUTTONS;
typedef struct TCCBUTTONS2 TCCBUTTONS2;
typedef struct TKEYPAD TKEYPAD;
typedef struct TPANE TPANE;
typedef struct TGRAPH TGRAPH;
typedef struct TLISTBOX TLISTBOX;


typedef int64_t (*TCommonCrtlCbMsg_t) (const void *object, const int msg, const int64_t dataInt1, const int64_t dataInt2, void *dataPtr);
typedef int (*TCommonCrtlCbRender_t) (void *object, TFRAME *frame);
typedef int (*TCommonCrtlCbCreate_t) (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t obj_cb, int *id, const int var1, const int var2);
typedef void (*TCommonCrtlCbDelete_t) (void *object);
typedef void (*TCommonCrtlCbEnable_t) (void *object);
typedef void (*TCommonCrtlCbDisable_t) (void *object);
typedef int (*TCommonCrtlCbInput_t) (void *object, TTOUCHCOORD *pos, const int flags);
typedef int (*TCommonCrtlCbSetPosition_t) (void *object, const int x, const int y);
typedef int (*TCommonCrtlCbSetMetrics_t) (void *object, const int x, const int y, const int width, const int height);

typedef struct {
	TCommonCrtlCbMsg_t msg;
	TCommonCrtlCbRender_t render;
	TCommonCrtlCbCreate_t create;
	TCommonCrtlCbDelete_t free;
	TCommonCrtlCbEnable_t enable;
	TCommonCrtlCbDisable_t disable;
	TCommonCrtlCbInput_t input;
	TCommonCrtlCbSetPosition_t setPosition;
	TCommonCrtlCbSetMetrics_t setMetrics;
}TCCCB;


#define COMMONCTRLOBJECT			\
	TMLOCK *lock;					\
	/*struct{*/							\
		uint32_t enabled:1;			\
		uint32_t canDrag:1;			\
		uint32_t isChild:1;			\
		uint32_t isHovered:1;		\
		uint32_t processInput:1; /* inputEnabled */ \
		uint32_t input_dd_mm:1;		\
		uint32_t ccpadding:26;		\
	/*}f;*/								\
	uint32_t touchInputId;			\
	int id;							\
	int type;						\
	int pageOwner;					\
	void *userPtr;					\
	TCC *cc;						\
	TCCCB cb;						\
	int64_t userInt64;				\
	TCCOBJ *objRoot;				\
	TMETRICS metrics;

struct TCCOBJECT{
	COMMONCTRLOBJECT;
};

struct TCCOBJ {
	TCCOBJECT *obj;
	TCCOBJ *next;
};

typedef struct{
	_ufont_t *hLib;				// handle to ufont
	char *path;					// font file/location
	uint32_t hash;
}TFONTINST;

struct TCC {
	TCCOBJ *objs;
	int ccIdIdx;
	
	struct{		// input modal states
		int state;	// 1 enabled, someone has it, 0 disabled
		int id;		// this object has input focus (others won't receive input) but only when its pageOwner is currently rendering
	}modal;

	TARTMANAGER *im[CC_IMAGEMANAGER_TOTAL];
	TGUIINPUT *cursor;
	TFONTINST font[CC_FONT_HANDLES];
	
	// remove these, shouldnt be here
	
	TFRAMESTRINGCACHE *strc;
	double fTime;
	double rTime;
	void *hSurfaceLib;
	int dwidth;
	int dheight;
	HWND hMsgWin;				// main input event message thread
	TSHADOWUNDER shadow[4];		// black, blue and green underlays, idx 0 is empty
	int imgcIds[IMGC_TOTAL];	// imageCache handle Id's
		
	struct{
		uint64_t freq;
		uint64_t tStart;
		double resolution;
	}timer;

};

typedef struct widgets_t widgets_t;

#include "../common/input.h"
#include "label.h"
#include "button.h"
#include "button2.h"
#include "slider.h"
#include "scrollbar.h"
#include "panel.h"
#include "tv.h"
#include "keypad.h"
#include "pane.h"
#include "graph.h"
#include "listbox.h"





TCC *ccInit (void *hSurfaceLib, TGUIINPUT *input);
void ccDestroy (TCC *cc);
void ccCleanupMemory(TCC *cc);

// create a widget
void *ccCreate   (TCC *cc, const int pageOwner, const int cc_type, const TCommonCrtlCbMsg_t obj_cb, int *id, const int var1, const int var2);
void *ccCreateEx (TCC *cc, const int pageOwner, const int cc_type, const TCommonCrtlCbMsg_t obj_cb, int *id, const int var1, const int var2, void *udataPtr);
void ccDelete (void *object);

int ccLock (void *object);
void ccUnlock (void *object);

int64_t ccSendMessage (void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr);
int ccSetMetrics (void *object, int x, int y, int width, int height);
void ccGetMetrics (void *object, TMETRICS *metrics);
int ccSetPosition (void *object, const int x, const int y);
int ccGetPositionX (void *object);
int ccGetPositionY (void *object);
void ccGetPosition (void *object, int *x, int *y);
int ccPositionIsOverlapped (void *object, const int x, const int y);
int ccGetHeight (void *object);
int ccGetWidth (void *object);

int ccGetState (void *object);
void ccEnable (void *object);
void ccDisable (void *object);

int ccSetImageManager (TCC *cc, const uint32_t managerId, TARTMANAGER *manager);
TARTMANAGER *ccGetImageManager (TCC *cc, const uint32_t managerId);

int ccRender (void *object, TFRAME *frame);
int ccRenderEx (void *object, TFRAME *frame, const int cx, const int cy);

// for debug use 
int ccRenderAll (TCC *cc, TFRAME *frame, const int owner, const int cx, const int cy);

#define ccIsHovered(a) (((TCCOBJECT*)(a))->isHovered)

void *ccGetUserData (void *object);
void ccSetUserData (void *object, void *data);
void ccSetUserDataInt (void *object, const int64_t data);
int64_t ccGetUserDataInt (void *object);

int ccHandleInput (void *object, TTOUCHCOORD *pos, const int flags);
int ccHandleInputAll (TCC *cc, TTOUCHCOORD *pos, const int flags, int page);

void ccInputEnable (void *object);
void ccInputDisable (void *object);

void ccInputGetPosition (void *object, point_t *pos);

// enable/disable global hover detection when sliding
void ccInputSlideHoverDisable (void *object);
void ccInputSlideHoverEnable (void *object);

int ccCanDrag (const TCC *cc, const TTOUCHCOORD *pos, int page);

int ccGetOwner (void *object);							// returns PAGE_ id
int ccSetOwner (void *object, const int newOwner);
void *ccGetPageOwner (void *object);					// returns page struct ptr

int ccGenerateId (void *object);

int ccGetModal (TCC *cc, int *id);
int ccSetModal (TCC *cc, const int id);

int ccFontAdd (TCC *cc, const char *path);
void ccFontRemove (TCC *cc, const int idx);
int ccFontOpen (TCC *cc, const int idx);
void ccFontClose (TCC *cc, const int idx);
int ccFontIsOpen (TCC *cc, const int idx);
void *ccFontGetHandle (TCC *cc, const int idx);


int ccIsHoveredMM (TCC *cc, const int owner, const int x, const int y, const int setIfTrue);
int ccHoverRenderSigEnable (TCC *cc, const double fps);
void ccHoverRenderSigDisable (TCC *cc);
int ccHoverRenderSigGetState (TCC *cc);
double ccHoverRenderSigGetFPS (TCC *cc);

TCCOBJECT *ccGetObject (TCCOBJECT *obj, const int id);
/*
static inline int ccGetId (TVLCPLAYER *vp, const int lookup_idx)
{
	return vp->gui.ccIds[lookup_idx];
}*/
#define ccGetId(v,i) ((v)->gui.ccIds[(i)])

int ccDumpRect (TCCOBJ *list, const int page, int x1, int y1, int x2, int y2);
//void ccdumpobjs (TCCOBJ *list);


static inline void ccDrawRectangle (TFRAME *frame, TMETRICS *metrics, const uint32_t colour)
{
	lDrawRectangle(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, COLOUR_24TO16(colour));
}

static inline void ccDrawRectangleFilled (TFRAME *frame, TMETRICS *metrics, const uint32_t colour)
{
	lDrawRectangleFilled(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, COLOUR_24TO16(colour));
}


/*

int64_t Msg (const void *object, const int msg, const int64_t dataInt1, const int64_t dataInt2, void *dataPtr)
{
	return 0;
}

int Render (void *object, TFRAME *frame)
{
	return 0;
}

void Delete (void *object)
{
	return;
}

void Enable (void *object)
{
	return;
}

void Disable (void *object)
{
	return;
}

int Input (void *object, TTOUCHCOORD *pos, const int flags)
{
	return 0;
}

int SetPosition (void *object, const int x, const int y)
{
	return 0;
}

int SetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	return 0;
}

*/

#endif


