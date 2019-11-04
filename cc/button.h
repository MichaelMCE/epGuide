
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


#ifndef _CCBUTTON_H_
#define _CCBUTTON_H_



#define BUTTON_PRI					0		// primary face
#define BUTTON_SEC					1		// alt/secondary face

#define BUTTONS_RENDER_HOVER		0x01
#define BUTTONS_RENDER_ANIMATE		0x02

#define BUTTON_FACE_SWAP_HOVERED	1		// swap face when hovered
#define BUTTON_FACE_SWAP_SELECTED	2		// swap only when pressed/click

#define BUTTON_BORDER_PRE			LABEL_BORDER_SET_PRE
#define BUTTON_BORDER_POST			LABEL_BORDER_SET_POST


typedef struct{
	TLABEL *label;		// image container
	int itemId;			// label obj id
	int itemStrId;		// label child obj id, if any
	int imgId;			// imageManager reference
}TBUTTONLABEL;

struct TCCBUTTON{
	COMMONCTRLOBJECT;
	
	TCommonCrtlCbMsg_t clientMsgCb;
	TBUTTONLABEL *btnlbl[2];
	TBUTTONLABEL *active;
	int64_t dataInt64;

	struct {
		uint32_t disableInput:1;		// don't accept input but do render
		uint32_t disableRender:1;		// dont't render, accept input
		uint32_t acceptDrag:1;
		uint32_t canAnimate:1;			// 0:none, 1:zoom
		uint32_t canHover:1;			// mouse hover effect
		uint32_t highlightDisable:1;	// whether or not to use the highlight image (if set)
											// automatically swap image faces upon press/click, where applicable
		uint32_t autoFaceSwap;			// TODO: improve useability of this
	}flags;

	struct {
		int direction;		// 0: do nothing
		double scaleBy;		// scale label by this amount (labelImageScaleSet() )
		double scaleRate;	// change by per pass
		int updateRate;		//
	}zoom;
	
	struct {
		uint32_t colour;
		double alpha;
		//uint64_t time;		// time period
	}hover;

	int timerId;		// stuff location
};


// TODO: needs a lock
struct TCCBUTTONS{
	TCCBUTTON **list;
	int total;
	int pageId;
	uint64_t t0;
};




int ccbuttonNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t treeview_cb, int *id, const int x, const int y);

int ccButtonGetXWidth (TCCBUTTON *btn);			// returns location x + width
int ccButtonGetXHeight (TCCBUTTON *btn);		// returns location y + height


int buttonFacePathSet (TCCBUTTON *btn, wchar_t *pri, wchar_t *sec, const int x, const int y);
int buttonFaceImageSet (TCCBUTTON *button, TFRAME *pri, TFRAME *sec, const int x, const int y);
int buttonFaceSetImgId (TCCBUTTON *button, const int idx, const int imgId, const int x, const int y);	// imageManager

int buttonFaceActiveSet (TCCBUTTON *button, const int face);	// display primary or secondary image (BUTTON_PRI/BUTTON_SEC)
int buttonFaceActiveGet (TCCBUTTON *button);	// returns whichever face is displayed (BUTTON_PRI or BUTTON_SEC)

TFRAME *buttonFaceImageGet (TCCBUTTON *button);	// return active face. lock must be held during any operation upon the returned image

void buttonFaceAutoSwapEnable (TCCBUTTON *button, const int mode);		// enable automatic display of secondary face (mode:BUTTON_FACE_SWAP_HOVERED/SELECTED)
void buttonFaceAutoSwapDisable (TCCBUTTON *button);						// disable above

int buttonFaceConstraintsSet (TCCBUTTON *button, const int x1, const int y1, const int x2, const int y2);
int buttonFaceConstraintsEnable (TCCBUTTON *button);
int buttonFaceConstraintsDisable (TCCBUTTON *button);


void buttonBorderEnable (TCCBUTTON *button);
void buttonBorderDisable (TCCBUTTON *button);

// refer to cc/label.h for usage
void buttonBorderProfileSet (TCCBUTTON *button, const int set, const uint32_t *colour, const int total);

void buttonBaseColourSet (TCCBUTTON *button, const uint32_t colour);
void buttonBaseEnable (TCCBUTTON *button);
void buttonBaseDisable (TCCBUTTON *button);


int buttonAnimateSet (TCCBUTTON *button, const int state);	// sets the image face (if applied) to be animated when selected
int buttonAnimateGet (TCCBUTTON *button);

// an image face must have been set before calling this (face: BUTTON_PRI or BUTTON_SEC)
int buttonFaceTextSet (TCCBUTTON *button, const int face, const char *str, const int flags, const char *font, const int x, const int y);
// update string after its been set
int buttonFaceTextUpdate (TCCBUTTON *button, const int face, const char *str);
void buttonFaceTextColourSet (TCCBUTTON *button, const int face, const uint32_t fore, const uint32_t back, const uint32_t outline);
int buttonFaceTextWidthSet (TCCBUTTON *button, const int face, const int width);		// set maximum width
void buttonFaceTextFlagsSet (TCCBUTTON *button, const int face, const int pf_flags);	// set render flags
int buttonFaceTextMetricsGet (TCCBUTTON *button, const int face, int *x, int *y, int *width, int *height);


int buttonFaceHoverSet (TCCBUTTON *button, const int state, const uint32_t colour, const double alpha);
int buttonFaceHoverGet (TCCBUTTON *button);


void buttonDataSet (TCCBUTTON *button, int64_t data);
int64_t buttonDataGet (TCCBUTTON *button);



//	TCCBUTTONS 
// general button list wrapper API
TCCBUTTONS *buttonsCreate (TCC *cc, const int pageOwner, const int total, const TCommonCrtlCbMsg_t btn_cb);
TCCBUTTON *buttonsCreateButton (TCCBUTTONS *btns, wchar_t *pri, wchar_t *sec, const int id, const int initialState, const int canAnimate, const int x, const int y);
TCCBUTTON *buttonsButtonGet (TCCBUTTONS *btns, const int idx);

int buttonsTotalSet (TCCBUTTONS *btns, const int total, const TCommonCrtlCbMsg_t btn_cb);		// resize list
int buttonsTotalGet (TCCBUTTONS *btns);

int buttonsButtonToIdx (TCCBUTTONS *btns, TCCBUTTON *btn);

int buttonsStateGet (TCCBUTTONS *btns, const int id);
void buttonsStateSet (TCCBUTTONS *btns, const int idx, const int state);	// set state of an individual button
void buttonsSetState (TCCBUTTONS *btns, const int state);					// set state of all buttons to 'state'

void buttonsDeleteAll (TCCBUTTONS *btns);
uint32_t buttonsRenderAll (TCCBUTTONS *btns, TFRAME *frame, const int flags);
uint32_t buttonsRenderAllEx (TCCBUTTONS *btns, TFRAME *frame, const int flags, const int cx, const int cy);

int buttonsWidthGet (TCCBUTTONS *btns, const int id);
int buttonsHeightGet (TCCBUTTONS *btns, const int id);
int buttonsPosXGet (TCCBUTTONS *btns, const int id);
int buttonsPosYGet (TCCBUTTONS *btns, const int id);
void buttonsPosSet (TCCBUTTONS *btns, const int id, const int x, const int y);
void buttonsMetricsSet (TCCBUTTONS *btns, const int id, const int x, const int y, const int width, const int height);
int buttonsMetricsGet (TCCBUTTONS *btns, const int id, int *x, int *y, int *width, int *height);

#endif


