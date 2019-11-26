
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





#ifndef _PANE_H_
#define _PANE_H_


#define PANE_FONT					"uf/helvr24.uf"		// default font


#define PANE_LAYOUT_HORI			1		// left justified, multiple columns with horizontal scrolling
#define PANE_LAYOUT_VERT			2		// left justified, single column, muliple rows, vertical scrolling
#define PANE_LAYOUT_VERTCENTER		3		// middle justified, single column, muliple rows, vertical scrolling 
#define PANE_LAYOUT_HORICENTER		4		// middle justified, single row, multiple columns, horizontal scrolling
#define PANE_LAYOUT_TABLE			5		// left justified, multiple columns/rows


#define PANE_INVALIDATE				1

#define PANE_OBJ_STRING				1
#define PANE_OBJ_IMAGE				2
#define PANE_OBJ_CELL				3

#define PANE_OBJ_DATASPACE			2


#define PANE_DIRECTION_STOP			0				// no movement (left or right / up or down)
#define PANE_DIRECTION_LEFT			1				// right to left
#define PANE_DIRECTION_RIGHT		2				// left to right 
#define PANE_DIRECTION_UP			3
#define PANE_DIRECTION_DOWN			4

#define PANE_SLIDEMODE_PANE			1
#define PANE_SLIDEMODE_ITEM			2

#define PANE_COLOUR_TEXT			(255<<24|COL_WHITE)

#define PANE_INPUTSTATE_PRESSED			1
#define PANE_INPUTSTATE_SLIDE			2
#define PANE_INPUTSTATE_RELEASED		3

#define paneHasCells(a)  ((a)->flags.total.cell)


#define PANE_GRAB_AREA					150






// image allignment/placement (PANE_OBJ_IMAGE)
enum _imgPos {
	PANE_IMAGE_CENTRE = 1,
	PANE_IMAGE_NORTH,
	PANE_IMAGE_SOUTH,
	PANE_IMAGE_WEST,
	PANE_IMAGE_EAST,
	PANE_IMAGE_NW,
	PANE_IMAGE_NE,
	PANE_IMAGE_SW,
	PANE_IMAGE_SE,
	PANE_IMAGE_POS			// for this 'pos' is the local position, otherwise it's the offset around the above Geo. directions
};


enum _table {
	PANE_TABLE_ROW,
	PANE_TABLE_COL
};



#define PANE_SLIDE_DISABLED		(-1)
#define PANE_SLIDE_NONE			(0)
#define PANE_SLIDE_PRESS		(1)
#define PANE_SLIDE_SLIDE		(2)
#define PANE_SLIDE_RELEASE		(3)

#define maxImgCacheItems		(32)

typedef struct{
	int imgIds[maxImgCacheItems];
	int total;
	TARTMANAGER *am;
	void *ctx;
}img_readhead;


#define	PANE_CELL_MAXITEMS		(2)

typedef struct {
	int horiMin;			// left side
	int horiMax;			// right side
	int vertMin;			// top
	int vertMax;			// bottom
	float pixelsPerUnit;
	int cellHeight;			// assumes cells expand horizontally from left to right
	int cellSpaceHori;
	int cellSpaceVert;
}pane_tablelayout_t;

typedef struct {
	int id;
	int type;

	struct {
		//int32_t i32[PANE_OBJ_DATASPACE];
		int64_t i64[PANE_OBJ_DATASPACE];
		//float flt[PANE_OBJ_DATASPACE];
	}data;

	struct {
		TMETRICS metrics;
		int position;			// x: pane->layout.table.horiMin <> pane->layout.table.horiMax
		int row;				// y: pane->layout.table.vertMin <> pane->layout.table.vertMax
		int width;				// horizontal length, from position towards table.horiMax
		uint32_t colour;		// cell base/background colour
		
		struct {
			int id;				// id as returned by label control
			point_t pos;		// position within cell
			int64_t data64;
		}item[PANE_CELL_MAXITEMS];
		
		int isEnabled;
	}cell;

	struct {
		int itemId;				// label item id
		int imgcId;				// image manager id
		int dir;				// image position (PANE_DIRECTION_)
		TMETRICS metrics;		// position within label
		point_t offset;			// image placement offset from when using direction PANE_IMAGE_ (except PANE_IMAGE_POS)
	}image;

	struct {
		int itemId;				// label item id
		TMETRICS metrics;		// position within label, width/height of string
		uint32_t hasIcon:1;		// render ::image next to this
		uint32_t overlapIcon:1;	// text overlaps icon (same column space)
		uint32_t padding:30;
	}text;
	
	struct {
		void *object;
	}cc;
}TPANEOBJ;

typedef struct {
	int state;
	int slideEnabled;
	int slideMode;			// pane slide or item grab/slide
	
	struct {
		int x;
		int y;
		double time;		// ms, time event received (getTime(vp))
		int id;				// pos->id
	}start;
	
	struct {
		int x;
		int y;
	}last;					// previous location
		
	struct {
		int x;
		int y;
		double time;
	}delta;					// distance travelled from previous event to this (relative)

	struct {
		int x;
		int y;
	}travelled;				// total cumulative distance travelled
	
	struct {
		int x;
		int y;
		double dt;			// deltaTime, time between inital press and final release
		int id;				// pos->id
		
		struct {
			int x;
			int y;
			unsigned int area;	// area from start to release (up)
		}travelled;				// total cumulative distance travelled
	}released;

	struct {
		double x;
		double y;
	}acceleration;			// movement multiplier
	
	struct {
	  TLABEL *label;
	  int itemId;
	  int artId;
	  int dx;
	  int dy;
	  int id;
	  int state;
	  int heldId;
	  int heldType;
	}drag;
}TPANEINPUT;

struct TPANE {
	COMMONCTRLOBJECT;
	
	TLABEL *base;
	TLISTITEM *items;
	TLISTITEM *itemsLast;

	TLISTITEM *firstEnabledItem;
	TLISTITEM *lastEnabledItem;
	int firstEnabledImgId;
	int firstEnabledImgIdx;
	int firstEnabledStrId;
	int firstEnabledStrIdx;

	int idSrc;
		
	point_t pos;
	point_t offset;
	point_t previous;
	TPANEINPUT input;

	struct {
		int type;					// PANE_LAYOUT_
		int direction;				// PANE_DIRECTION_
		int isInvalidated;			// set when item list is modified, indicates item list objects require repositioning
		int horiColumnSpace;		// padding space between columns
		
		struct {
			int tItemWidth;			// 
			int tItemHeight;
			int vertLineHeight;		// configure line height (line pitch) of PANE_LAYOUT_VERT
			int aveLineHeight;
		}metrics;
		
		struct {
			int horiMin;			// left side
			int horiMax;			// right side
			int vertMin;			// top
			int vertMax;			// bottom
			float pixelsPerUnit;
			int cellSpaceHori;
			int cellSpaceVert;
		}table;
	}layout;

	struct {
		struct {
			int text;				// number of text objs added
			int img;				// number of img objs added
			int cell;				// true if a cell has been created
		}total;
		struct {
			int singleLine;			// 1: single line text only, no wrap around. 0: allow text to wrap around.
			int wordWrap;			// enable word wrap when :textSingleLine is disabled
			int renderFlags;
			point_t globalOffset;
		}text;
		struct {
			int enabled;
			int number;				// read ahead this number of images
		}readAhead;
	}flags;
};



int paneNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t panel_cb, int *id, const int width, const int height);

void paneRemoveAll (TPANE *pane);	// remove all items from pane
void paneRemoveItem (TPANE *pane, const int itemId);

//	 imgcId: string placed right side of image
//	-imgcId: string placed on top of image (overlaps)
int paneTextAdd   (TPANE *pane, const int imgcId, const double imgScale, const char *string, const char *font, const int64_t data64a);
int paneTextAddEx (TPANE *pane, const int imgcId, const double imgScale, const char *string, const char *font, const unsigned int colour, const int64_t data64a, const int64_t data64b);
int paneImageAdd  (TPANE *pane, const int imgcId, const double scale, const int dir, const int x, const int y, const int64_t data64a);


int paneCellCreate (TPANE *pane, const uint32_t colour, const int32_t row, const int32_t position, const int32_t width, const int64_t data64a);
void paneCellRemove (TPANE *pane, const int cellId);
void paneCellRemoveAll (TPANE *pane);
int paneCellAddString (TPANE *pane, const int cellId, const char *string, const char *font, const uint32_t colour, const int flags, const int x, const int y, const int64_t data64);
void paneTableSetLayoutMetrics (TPANE *pane, pane_tablelayout_t *layout);
TPANEOBJ *paneCellFind (TPANE *pane, const int64_t data64a, const uint64_t mask);
void paneCellColourSet (TPANE *pane, const int cellId, const uint32_t colour);
uint32_t paneCellColourGet (TPANE *pane, const int cellId);
int paneCellSetStringData (TPANE *pane, const int cellId, const int idx, int64_t data64);
int paneCellGetStringData (TPANE *pane, const int cellId, const int idx, int64_t *data64);
int paneCellIsHovered (TPANE *pane, const int x, const int y);

int paneTextReplace (TPANE *pane, const int textId, const char *string);
int paneImageReplace (TPANE *pane, const int imageId, const int newImageId);

int paneItemGetDetail (TPANE *pane, const int itemId, char **str, int *imgId);
int paneIndexToItemId (TPANE *pane, const int idx);		// base 0 idx
int paneItemGetData (TPANE *pane, const int itemId, int64_t *data64a);
int paneItemGetDataEx (TPANE *pane, const int itemId, int64_t *data64a, int64_t *data64b);

void paneSetLayout (TPANE *pane, const int layoutMode);
void paneSetAcceleration (TPANE *pane, const double x, const double y);
void paneSwipeDisable (TPANE *pane);
void paneSwipeEnable (TPANE *pane);

int paneScroll (TPANE *pane, const int nPixels);
int paneScrollSet (TPANE *pane, const int x, const int y, const int invalidate);
int paneScrollGet (TPANE *pane, int *xOffset, int *yOffset);
int paneScrollReset (TPANE *pane);

void paneDragEnable (TPANE *pane);
void paneDragDisable (TPANE *pane);

int paneFocusSet (TPANE *pane, const int itemId);
void paneInvalidate (TPANE *pane);

void paneTextMultilineEnable (TPANE *pane);
void paneTextMultilineDisable (TPANE *pane);
void paneTextWordwrapEnable (TPANE *pane);
void paneTextWordwrapDisable (TPANE *pane);


void panePreloadItems (TPANE *pane, const int count);

#endif

