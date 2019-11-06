
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





#ifndef _SCROLLBAR_H_
#define _SCROLLBAR_H_




#define SCROLLBAR_VERTWIDTH			(44)






enum _SCROLLBARBTN
{
	SCROLLBAR_BTN_BASE,
	SCROLLBAR_BTN_TOTAL,
};


struct TSCROLLBAR {
	COMMONCTRLOBJECT;
	TCCBUTTONS *ccbtns;
	
	int inputId;
	point_t cursor;
	int64_t rangeMin;
	int64_t rangeMax;
	int64_t firstItem;
	int64_t displayedItems;
	
	struct{
		int drawBlur;
		int drawBase;
		int drawFrame;
	}flags;
	
	double scalePos;
};

int scrollbarNew (TCCOBJECT *object, void *unused, const int pageOwner, const int scrollbarType, const TCommonCrtlCbMsg_t scrollbar_cb, int *id, const int var1, const int var2);

void scrollbarSetRange (TSCROLLBAR *scrollbar, const int64_t min, const int64_t max, const int64_t first, const int64_t viewable);
int64_t scrollbarGetFirstItem (TSCROLLBAR *scrollbar);
int64_t scrollbarSetFirstItem (TSCROLLBAR *scrollbar, int64_t item);

void scrollbarBaseEnable (TSCROLLBAR *scrollbar);
void scrollbarBaseDisable (TSCROLLBAR *scrollbar);

double scrollbarSetValueFloat (TSCROLLBAR *scrollbar, const double value);

#endif

