
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





#ifndef _LISTBOX_H_
#define _LISTBOX_H_


#define LISTBOX_FONT_DEFAULT			LFTW_B34
#define LISTBOX_LINEHEIGHT_DEFAULT		37
#define SCROLLBAR_VERTWIDTH_DEFAULT		SCROLLBAR_VERTWIDTH

struct TLISTBOX {
	COMMONCTRLOBJECT;

	TPANE *pane;

	struct{
		TSCROLLBAR *vert;
		int location;		// 1:left, 2:right
		int offset;			// inner positional offset from edge
		int width;
		int draw;
	}scrollbar;

	struct{
		int lineHeight;
		int scrollDelta;
		char *font;			// default font
		int artMaxHeight;
		double imgScale;
	}flags;
	
	struct{
		int imgId;
		int drawShadow;
		int colour;			// type: SHADOW_GREEN/BLACK/BLUE

		struct{
			int x;			// image adjustment offset from center
			int y;
		}offset;
	}underlay;
	
	struct{
		int itemId;

		struct{
			unsigned int fore;
			unsigned int back;
			unsigned int outline;
		}colour;
		
		struct{
			unsigned int fore;
			unsigned int back;
			unsigned int outline;
		}colourPre;
	}highlighted;
};


int listboxAddItem (TLISTBOX *listbox, const char *string, const int imgId, const unsigned int colour, const int64_t udataInt64);
int listboxUpdateItem (TLISTBOX *listbox, const int itemId, const char *string);
int listboxUpdateItemEx (TLISTBOX *listbox, const int itemId, const char *string, const char *font, const int newImgId);
void listboxRemoveItem (TLISTBOX *listbox, const int itemId);
void listboxRemoveAll (TLISTBOX *listbox);
int listboxGetTotal (TLISTBOX *listbox);		// returns total items

void labelSetItemFont (TLISTBOX *listbox, const int itemId, const char *font);
void listboxSetItemColour (TLISTBOX *listbox, const int itemId, const unsigned int fore, const unsigned int back, const unsigned int outline);
void listboxGetItemColour (TLISTBOX *listbox, const int itemId, unsigned int *fore, unsigned int *back, unsigned int *outline);
void listboxSetHighlightedItem (TLISTBOX *listbox, const int itemId);
void listboxSetHighlightedColour (TLISTBOX *listbox, const unsigned int fore, const unsigned int back, const unsigned int outline);

int listboxItemGet (TLISTBOX *listbox, const int itemId, char **str, int *imgId);		// free str with my_free()
int listboxIndexToItemId (TLISTBOX *listbox, const int idx);							// base 0 index

int listboxGetFocus (TLISTBOX *listbox);
int listboxSetFocus (TLISTBOX *listbox, const int pixelOffset);

int listboxScrollDown (TLISTBOX *listbox);
int listboxScrollUp (TLISTBOX *listbox);
int listboxScroll (TLISTBOX *listbox, const int byNPixels);

int listboxSetScrollbarWidth (TLISTBOX *listbox, const int swidth);

void listboxSetFont (TLISTBOX *listbox, const char *font);
void listboxSetLineHeight (TLISTBOX *listbox, const int lineHeight);

int listboxSetUnderlsy (TLISTBOX *listbox, const int artId, const double scale, const int opacity, const int offsetX, const int offsetY, const int shadowIdx);		//shadowIdx: 0:none, SHADOW_BLACK/BLUE/GREEN

int listboxNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t lb_cb, int *id, const int maxWidth, const int maxHeight);






#endif
