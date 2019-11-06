
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




#ifndef _CCBUTTON2_H_
#define _CCBUTTON2_H_

#define HIGHLIGHTPERIOD				(200)


/*
#define BUTTON_PRI					0		// primary face
#define BUTTON_SEC					1		// alt/secondary face

#define BUTTONS_RENDER_HOVER		0x01
#define BUTTONS_RENDER_ANIMATE		0x02

#define BUTTON_FACE_SWAP_HOVERED	1		// swap face when hovered
#define BUTTON_FACE_SWAP_SELECTED	2		// swap only when pressed/click

#define BUTTON_BORDER_PRE			LABEL_BORDER_SET_PRE
#define BUTTON_BORDER_POST			LABEL_BORDER_SET_POST

*/


typedef struct{
	TLABEL *label;		// image container
	int itemId;			// label obj id
	int itemStrId;		// label child obj id, if any
	int imgId;			// artManager image id
}TBUTTON2LABEL;

struct TCCBUTTON2 {
	COMMONCTRLOBJECT;
	
	TCommonCrtlCbMsg_t clientMsgCb;
	TBUTTON2LABEL *btnlbl[2];
	TBUTTON2LABEL *active;
	int64_t dataInt64;

	struct {
		unsigned int disableInput:1;		// don't accept input but do render
		unsigned int disableRender:1;		// dont't render, accept input
		unsigned int acceptDrag:1;
		unsigned int canAnimate:1;			// 0:none, 1:zoom
		unsigned int canHover:1;			// mouse hover effect
		unsigned int highlightDisable:1;	// whether or not to use the highlight image (if set)
											// automatically swap image faces upon press/click, where applicable
		unsigned int autoFaceSwap;			// TODO: improve useability of this
	}flags;

	struct {
		int direction;		// 0: do nothing
		double scaleBy;		// scale label by this amount (labelImageScaleSet() )
		double scaleRate;	// change by per pass
		int updateRate;		//
	}zoom;
	
	struct {
		unsigned int colour;
		double alpha;
		//uint64_t time;		// time period
	}hover;

	int timerId;
	//void *imageManager;
};

/*
// TODO: needs a lock
struct TCCBUTTONS2{
	TCCBUTTON2 **list;
	int total;
	int pageId;
	uint64_t t0;
};
*/

int ccbutton2New (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t treeview_cb, int *id, const int x, const int y);


int button2FaceImgcSet (TCCBUTTON2 *btn, const int priId, const int secId, const double scale, const int x, const int y);

int button2FaceActiveSet (TCCBUTTON2 *button, const int face);	// display primary or secondary image
int button2FaceActiveGet (TCCBUTTON2 *button);	// returns whichever face is displayed (BUTTON_PRI or BUTTON_SEC)

void button2FaceAutoSwapEnable (TCCBUTTON2 *button, const int mode);		// enable automatic display of secondary face
void button2FaceAutoSwapDisable (TCCBUTTON2 *button);		// disable above

int button2FaceConstraintsSet (TCCBUTTON2 *button, const int x1, const int y1, const int x2, const int y2);
int button2FaceConstraintsEnable (TCCBUTTON2 *button);
int button2FaceConstraintsDisable (TCCBUTTON2 *button);


void button2BorderEnable (TCCBUTTON2 *button);
void button2BorderDisable (TCCBUTTON2 *button);

// refer to cc/label.h for usage
void button2BorderProfileSet (TCCBUTTON2 *button, const int set, const unsigned int *colour, const int total);

void button2BaseColourSet (TCCBUTTON2 *button, const unsigned int colour);
void button2BaseEnable (TCCBUTTON2 *button);
void button2BaseDisable (TCCBUTTON2 *button);


int button2AnimateSet (TCCBUTTON2 *button, const int state);	// sets the image face (if applied) to be animated when selected
int button2AnimateGet (TCCBUTTON2 *button);

// an image face must have been set before calling this
int button2FaceTextSet (TCCBUTTON2 *button, const int face, const char *str, const int flags, const char *font, const int x, const int y);
// update string after its been set
int button2FaceTextUpdate (TCCBUTTON2 *button, const int face, const char *str);
void button2FaceTextColourSet (TCCBUTTON2 *button, const int face, const unsigned int fore, const unsigned int back, const unsigned int outline);
int button2FaceTextWidthSet (TCCBUTTON2 *button, const int face, const int width);		// set maximum width
void button2FaceTextFlagsSet (TCCBUTTON2 *button, const int face, const int pf_flags);	// set render flags
int button2FaceTextMetricsGet (TCCBUTTON2 *button, const int face, int *x, int *y, int *width, int *height);


int button2FaceHoverSet (TCCBUTTON2 *button, const int state, const unsigned int colour, const double alpha);
int button2FaceHoverGet (TCCBUTTON2 *button);

int ccButton2GetXWidth (TCCBUTTON2 *btn);		// returns location x + width
int ccButton2GetXHeight (TCCBUTTON2 *btn);		// returns location y + height

void button2DataSet (TCCBUTTON2 *button, int64_t data);
int64_t button2DataGet (TCCBUTTON2 *button);


#endif


