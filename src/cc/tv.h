
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





#ifndef _TV_H_
#define _TV_H_




#if 1

#define TREEVIEW_RENDER_INDENT		(32 + 22)
#define TV_RENDER_INDENT			TREEVIEW_RENDER_INDENT
#define TEXT_INDENT_GUESS			(8)

#define EXPANDER_SIZE				(20)
#define CHECKBOX_SIZE				(20)
#define EXPANDER_OFFSET_Y			(3)
#define CHECKBOX_OFFSET_Y			(4)

#define MIN_IMAGE_WIDTH				(120+16)
#define MIN_IMAGE_HEIGHT			(120+16)

#else

#define TREEVIEW_RENDER_INDENT		(32)
#define TEXT_INDENT_GUESS			(8)

#define EXPANDER_SIZE				(8)
#define CHECKBOX_SIZE				(10)
#define EXPANDER_OFFSET_Y			(0)
#define CHECKBOX_OFFSET_Y			(0)

#define MIN_IMAGE_WIDTH				(64)
#define MIN_IMAGE_HEIGHT			(64)

#endif




#define TV_RENDER_BLUR				(0x0001)
#define TV_RENDER_UNDERLAY			(0x0002)
#define TV_RENDER_SCROLLWIDGETS		(0x0004)
#define TV_RENDER_LABELS			(0x0008)		/*  text */
#define TV_RENDER_IMAGES			(0x0010)
#define TV_RENDER_CHECKBOX			(0x0020)
#define TV_RENDER_EXPANDER			(0x0040)
#define TV_RENDER_BBOX				(0x0080)
#define TV_RENDER_IDTEXT			(0x0100)
#define TV_RENDER_LABELROWHL		(0x0200)		/* highlight label item text*/
#define TV_RENDER_AREAFILLED		(0x0400)
#define TV_RENDER_PASTEDEST			(0x0800)
#define TV_RENDER_DRAGITEM			(0x1000)
#define TV_RENDER_ALL				(TV_RENDER_BLUR |			\
									 TV_RENDER_UNDERLAY |		\
									 TV_RENDER_SCROLLWIDGETS |	\
									 TV_RENDER_LABELS |			\
									 TV_RENDER_CHECKBOX |		\
									 TV_RENDER_EXPANDER |		\
									 TV_RENDER_IDTEXT	|		\
									 TV_RENDER_LABELROWHL |		\
									 TV_RENDER_PASTEDEST |		\
									 TV_RENDER_DRAGITEM |		\
									 TV_RENDER_IMAGES)


enum tvObjType {
	TV_TYPE_CCOBJECT,
	TV_TYPE_LABEL,
	TV_TYPE_INT32,
	TV_TYPE_FLOAT,
	TV_TYPE_IMAGE
};

enum tvCheckboxState {
	TV_CHECKBOX_CLEAR,
	TV_CHECKBOX_CHECKED,
};

enum tvExpanderState {
	TV_EXPANDER_DISABLED,		/* nothing to see here, move along */
	TV_EXPANDER_ENABLED,		/* struct contains valid image data */
	TV_EXPANDER_DONTRENDER, 	/* don't render the checkbox */
	TV_EXPANDER_OPEN,			/* render open state  */
	TV_EXPANDER_CLOSED			/* render closed state  */
};

enum tvDrawImage {
	TV_DRAWIMAGE_DISABLED,
	TV_DRAWIMAGE_ENABLED,
	TV_DRAWIMAGE_DONTRENDER
};


enum _tvRenderAction {
	TV_ACTION_NOACTION,
	TV_ACTION_EXPANDER_CLOSE,		// contract node
	TV_ACTION_EXPANDER_OPEN,		// expand node
	TV_ACTION_EXPANDER_TOGGLE,
	TV_ACTION_CHECKBOX_TOGGLE,
	TV_ACTION_TRACK_SELECT,
	TV_ACTION_IMAGE_SELECT
};

typedef struct {
	int width;
	int height;
	TLPOINTEX pos;
	int action;
	void *opaque;
	int id;			// tree item id
}TTV_RENDER_ITEM;

typedef struct TTV_RENDER TTV_RENDER;
struct TTV_RENDER {
	TTV_RENDER_ITEM *rItem;
	TTV_RENDER *next;
};

typedef struct {
	int offsetX;
	int offsetY;
	int drawImage;
	int drawState;
	
	int artId;		// artwork image manager id (vp->am)
	double scale;
	
	void *opaque;
	
	int cbId;
}TTV_ITEM_DESC_IMAGE;

typedef struct {
	int objType;		// a cc object, utf8 encoded text label, an image label or number
	int enabled;

	//struct {
	//	TCCOBJECT *obj;		// a common control object
	//}cc;
	struct {
		//char *text;
		int colour;
		char *font;
		int flags;
		int offsetX;
		int offsetY;
	}label;
	int32_t varInt32;
	float varFloat;
	//TTV_ITEM_DESC_IMAGE varImage;

	struct {
		int drawExpander;
		int expanderState;
		//TFRAME *imgOpen;
		//TFRAME *imgClose;
	}expander;
	
	struct {
		int drawCheckbox;
		int checkState;
		//TFRAME *imgBase;
		//TFRAME *imgChecked;
		//TFRAME *imgNotChecked;
	}checkbox;

	TTV_ITEM_DESC_IMAGE *image;
	int imageTotal;

	TMETRICS metrics;
	
	uint64_t time;	// time when last down (touch/pressed/clicked)
	int tState;		// last touch state
		
	void *opaque;
}TTV_ITEM_DESC;

typedef struct {
	int type;			// item or node/branch
	int id;				// ident of this item

	char *name;
	void *storage;

	int parentId;
	int *children;		// if a node/branch then this will contain a list of children
	
	void *entry;
}TTV_ITEM;


typedef struct {
	TTV_RENDER_ITEM post;		// item being dragged
	TTV_RENDER_ITEM dest;		// released on to or above this item

	int state;
	int ox;						// X delta offset (location) of image within dragged item
	int oy;						// Y, as above
	int sx;						// original location of dragged item
	int sy;						// as above
	int action;					// 0:do nothing, 1:dragged on to item, 2:above item
}TTV_RENDER_DRAG;

typedef struct {
	int from;
	int to;
	int action;
}TTV_INPUT_DROP;

typedef struct{
	char *font;
	int colour;
	point_t pos;
}TTV_RENDER_IDT;

struct TTV {
	COMMONCTRLOBJECT;
		
	TSCROLLBAR *sbVert;
	int sbVertPosition;					// thumb position
	int sbVertPositionRate;
	
	int cbIdSource;			// source for image callback id's
	
	int renderStartY;
	
	int inputEnabled;
	int renderFlags;
	int dragEnabled;
	int renameEnabled;
	
	TTREE *tree;
	int rootId;
	
	TTV_RENDER *preRender;
	TTV_RENDER_ITEM postRender[64];	// actual rendered object locations
	int tPostItems;

	TTV_RENDER_DRAG drag;			// item being dragged/moved
	TTV_RENDER_IDT idt;
};


int tvNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t treeview_cb, int *id, const int var1, const int var2);

int tvTreeCreate (TTV *tv, const char *name, const int rootId);
int tvTreeAddNode (TTV *tv, const int nodeId, char *name, const int id, TTV_ITEM_DESC *desc);
int tvTreeAddItem (TTV *tv, const int nodeId, char *name, const int id, TTV_ITEM_DESC *desc);
void tvTreeMove (TTV *tv, const int fromId, const int toId);
void tvTreeDelete (TTV *tv, const int id);	// delete root (everything), item or node
void treeRenameItem (TTV *tv, const int id, const char *newName);

void tvTreeSetStorage (TTV *tv, const int id, void *data);
void *tvTreeGetStorage (TTV *tv, const int id);

TTV_ITEM *tvTreeGetItem (TTV *tv, const int id, void *fromEntry);
void tvTreeFreeItem (TTV_ITEM *item);


int tvScrollbarSetWidth (TTV *tv, const int width);
int tvScrollbarGetWidth (TTV *tv);

void tvBuildPrerender (TTV *tv, const int id);

void *tvItemOpaqueGet (TTV_ITEM *item);
TTV_ITEM_DESC *tvTreeItemGetDesc (TTV_ITEM *item);

void tvTreeDeleteItems (TTV *tv, const int id);

int tvGenImageCbId (TTV *tv);

void tvScrollUp (TTV *tv);
void tvScrollDown (TTV *tv);

void tvSetCheckNode (TTV *tv, TTV_ITEM *item, const int state);
void tvCheckClearAll (TTV *tv);

void tvTreeEntryMove (TTV *tv, const int fromId, const int toId);
void tvTreeEntryMoveToTail (TTV *tv, const int fromId, const int toId);

void tvCollapseAll (TTV *tv);
void tvExpandAll (TTV *tv);

int tvExpandTo (TTV *tv, const int id);
void playlistExpandTo (TTV *tv, const int uid);

int tvTreeCountItems (TTV *tv, const int id);

void tvInputDisable (TTV *tv);
void tvInputEnable (TTV *tv);

void tvItemEnable (TTV *tv, const int id);


void tvRenderSetNodeIdLocation (TTV *tv, const int x, const int y);
void tvRenderSetNodeIdColour (TTV *tv, const int colour);
void tvRenderSetNodeIdFont (TTV *tv, const char *font);


// enter/leave keypad rename mode
void tvRenameEnable (TTV *tv);
void tvRenameDisable (TTV *tv);
int tvRenameGetState (TTV *tv);


#endif

