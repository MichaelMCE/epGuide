
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





#ifndef _INPUT_H_
#define _INPUT_H_



#define INPUT_FLAG_DOWN		0
#define INPUT_FLAG_SLIDE	1
#define INPUT_FLAG_UP		3



struct TGUIINPUT{
	int isHooked;				// is mouse hooked
	int slideHoverEnabled;		// hover detection without being hooked (mouse/pad)
	int draw;
	int x;				// Windows mouse co-ord X
	int y;				// Windows mouse co-ord Y
	int dx;				// local cursor co-ord X
	int dy;				// local cursor co-ord Y
	POINT pt;			// Windows cursor position when hooked
	int LBState;
	int MBState;
	int RBState;
	
	point_t dragRect0;		// from
	point_t dragRect1;		// to
	point_t dragRectDelta;	// size
	int dragRectIsEnabled;
	
	struct{
		int enableMMoveRenderSig;
		double renderMMoveTargetFPS;
	}virt;
};


struct TTOUCHSWIPE{
	int state;
	double t0;
	double dt;
	int sx;
	int sy;
	int ex;
	int ey;
	int dx;
	int dy;
	int dragMinH;
	int dragMinV;
	
	uint64_t u64value;
	int i32value;
	double dvalue;
	
	double velocityFactor;
	double decayRate;
	double decayFactor;
	double decayAdjust;
	double adjust;
	
	double velocity;
};


void touchSimulate (widgets_t *ui, const TTOUCHCOORD *pos, const int flags, TCC *cc);
//int touchDispatchFilter (TTOUCHCOORD *pos, const int flags, TCC *cc);

/*
void touchIn (TTOUCHCOORD *pos, int flags, void *ptr);
int touchDispatchFilter (TTOUCHCOORD *pos, const int flags, TVLCPLAYER *vp);
void touchDispatcherStop (TVLCPLAYER *vp);
void touchDispatcherStart (TVLCPLAYER *vp, const void *fn, const void *ptr);
void inputGetCursorPosition (TVLCPLAYER *vp, int *x, int *y);
unsigned int __stdcall inputDispatchThread (void *ptr);
*/


#endif
