
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


#ifndef _PANEL_H_
#define _PANEL_H_


#define PANEL_ITEM_SEPARATOR	0x01
#define PANEL_ITEM_DRIVE		0x02
#define PANEL_ITEM_LINK			0x04
#define PANEL_ITEM_MODULE		0x08
#define PANEL_ITEM_GENERAL		0x10


#define PANEL_ITEM_LABELTOTAL	(4)


typedef struct {
	char *text;				// utf8 formatted text label
	TMETRICS metrics;		// render location of label
	int font;
	TFRAME *strFrm;			// rendered text cache
	int colour;
}TPANELLABEL;

typedef struct {
	int isActive;
	unsigned int attributes;
	int id;
	TCCBUTTON2 *btn;
	TPANELLABEL labels[PANEL_ITEM_LABELTOTAL];	// single row text rendered below image. [0] = top most, [1] below that, etc..
	rect_t area;			// contact area (location) relative to panel. includes any text description
	void *udataPtr;			// private user storage

	void *storage;			// item level storage
}TPANELIMG;

struct TPANEL {
	COMMONCTRLOBJECT;

	int inputEnabled;		// to process input or not
	int itemTotal;			// total number of images

	int itemVertSpace;		// vertical padding between images, in pixels
	int itemHoriSpace;		// horizontal ^^
	point_t *itemOffset;	// offset each and every image location by point_t:x/y
	int itemMaxWidth;		// item width limit
	int vHeight;			// virtual depth(height) of panel;
	int font;

	TPANELIMG **list;		// button(s) container
	int listSize;			// capacity of list
	unsigned int objIdSrc;

	TTOUCHSWIPE swipe;
	int dragEnabled;

	TCCBUTTONS *btns;		// any button outside of designated panel area
};

int panelNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t panel_cb, int *id, const int butttonTotal, const int var2);

int panelImgAdd (TPANEL *panel, const int artId, char *label, void *udataPtr);
int panelImgAddEx (TPANEL *panel, const int artId, const int artIdAlt, const double scale, char *label, void *udataPtr);


int panelImgGetTotal (TPANEL *panel);
TPANELIMG *panelImgGetFirst (TPANEL *panel, int *idx);
TPANELIMG *panelImgGetNext (TPANEL *panel, int *idx);
TPANELIMG *panelImgGetItem (TPANEL *panel, const int id);

int panelImgPositionMetricsCalc (TPANELIMG **list, const int total, TMETRICS *metrics, const int horiSpace, const int vertSpace);

void *panelImgStorageGet (TPANEL *panel, const int id);
int panelImgStorageSet (TPANEL *panel, const int id, void *ptr);

void panelImgAttributeSet (TPANEL *panel, const int id, const int flag);
void panelImgAttributeClear (TPANEL *panel, const int id, const int flag);
int panelImgAttributeCheck (TPANEL *panel, const int id, const int flag);
int panelListResize (TPANEL *panel, const int newSize, const int keep);

TCCBUTTON *panelSetButton (TPANEL *panel, const int btn_id, wchar_t *img1, wchar_t *img2, const void *cb);
void panelSetBoundarySpace (TPANEL *panel, const int Vpad, const int Hpad);


int panelImgAddSubtext (TPANEL *panel, const int id, char *text, const int colour);
int panelInvalidate (TPANEL *panel);


// helper function, find/create a new artManager id
int genAMId (TPANEL *panel, wchar_t *path);

#endif

