
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


#ifndef _SLIDER_H_
#define _SLIDER_H_


enum _sliderface{
	SLIDER_FACE_MIDSRC,
	SLIDER_FACE_LEFT,
	SLIDER_FACE_TOP = SLIDER_FACE_LEFT,
	SLIDER_FACE_MID,
	SLIDER_FACE_RIGHT,
	SLIDER_FACE_BTM = SLIDER_FACE_RIGHT,
	SLIDER_FACE_TIP,
	SLIDER_FACE_TOTAL
};


struct TSLIDER {
	COMMONCTRLOBJECT;

	TLABEL *base;
	int faceIds[SLIDER_FACE_TOTAL];	// label obj id's
	int imgIds[SLIDER_FACE_TOTAL];	// imageManager image id's

	int inputId;
	int64_t rangeMin;
	int64_t rangeMax;
	int64_t value;
	double scalePos;

	TRECT pad;			// activate area (in pixels) surrounding slider
};

int sliderNew (TCCOBJECT *object, void *unused, const int pageOwner, const int sliderType, const TCommonCrtlCbMsg_t slider_cb, int *id, const int width, const int height);



int sliderFaceSet (TSLIDER *slider, int face, wchar_t *path);
void sliderFacesApply (TSLIDER *slider);

int sliderGetSliderRect (TSLIDER *slider, TLPOINTEX *rt);
int sliderSetTipPosition (TSLIDER *slider, const double pos);

void sliderHoverEnable (TSLIDER *slider, const int colour, const double alpha);
void sliderHoverDisable (TSLIDER *slider);

void sliderSetRange (TSLIDER *slider, const int64_t min, const int64_t max);
void sliderGetRange (TSLIDER *slider, int64_t *min, int64_t *max);
double sliderSetValueFloat (TSLIDER *slider, const double value);
double sliderGetValueFloat (TSLIDER *slider);
int64_t sliderSetValue (TSLIDER *slider, const int64_t value);
int64_t sliderGetValue (TSLIDER *slider);
int64_t sliderGetValueLookup (TSLIDER *slider, const int mx);	// calculate value based upon cursor position


#endif



