
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



#ifndef _DEMOS_H_
#define _DEMOS_H_


#include <string.h>
#include <conio.h>
#include <windows.h>
#include <fcntl.h>
 
#define DISPLAYMAX	4

typedef struct {
	int	dd;
	int	pd;
	int width;
	int height;
	int value;
}TMYLCDDEMO;

//#undef mylog
//#define mylog printf



THWD *hw = NULL;
TFRAME *frame = NULL;
TRECT displays[DISPLAYMAX];
TRECT *display;
lDISPLAY did = 0;
int virtualDisplayId = 0;


int DWIDTH = 0;
int DHEIGHT	= 0;
int DDATA = 0;
int DBPP = 0;



static inline void setBPP (int *bpp)
{
	switch (*bpp){
		case 1: *bpp = LFRM_BPP_1; break;
		case 8: *bpp = LFRM_BPP_8; break;
		case 12: *bpp = LFRM_BPP_12; break;
		case 15: *bpp = LFRM_BPP_15; break;
		case 16: *bpp = LFRM_BPP_16; break;
		case 24: *bpp = LFRM_BPP_24; break;
		case 32: *bpp = LFRM_BPP_32; break;
		default: *bpp = LFRM_BPP_32A;
	}
}

int initLibrary ()
{

	mylog("initLibrary: starting libmylcd...\n");
    if (!(hw=lOpen(L"", L""))){
    	mylog("initLibrary: lOpen() failed\n");
    	return 0;
    }

	mylog("initLibrary: requesting global display surface\n");
    if (!(frame=lNewFrame(hw, DWIDTH, DHEIGHT, DBPP))){
    	lClose(hw);
    	mylog("initLibrary: lNewFrame() failed\n");
    	return 0;
    }else{
		display = displays;
		mylog("initLibrary: libmylcd started successfully\n");
    	return 1;
    }
}

void demoCleanup()
{

	mylog("cleanup: mylcd.dll shutting global frame handle\n");
	if (frame){
		lDeleteFrame(frame);
		frame = NULL;
	}
	
	if (did)
		lCloseDevice(hw, did);
	
	mylog("cleanup: libmylcd closing device handle\n");
	if (hw){
		lClose(hw);
		hw = NULL;
	}
	
	//CoUninitialize();
		
	mylog("cleanup: libmylcd shutdown\n");
}

int initDemoConfig (char *configfile)
{
	if (!initLibrary())
		return 0;

	TASCIILINE *al = readFileA(configfile);
	if (al == NULL){
		mylog("initConfig: readFileA(%s) failed\n", configfile);
		return 0;
	}

	CoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_SPEED_OVER_MEMORY);
	
	char dd[lMaxDriverNameLength*2];
	char pd[lMaxDriverNameLength*2];
	char *line = NULL;
	int data;
	int w, h;
	unsigned int i = 0;
	memset(displays, 0, sizeof(TRECT) * DISPLAYMAX);
	
	do{
		line = (char *)al->line[i];
		if (!strncmp(line, "bpp=32a", 4)){
			sscanf(line+4, "%d", &DBPP);
			if (DBPP == 32){
				if (line[6] == 'a' || line[6] == 'A')
					DBPP = LFRM_BPP_32A;
				else
					DBPP = LFRM_BPP_32;
			}else{
				setBPP(&DBPP);
			}
			lDeleteFrame(frame);
			frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
		}else if (!strncmp(line, "data=", 5)){
			sscanf(line+5, "%d", &data);
		}else if (!strncmp(line, "width=", 6)){
			sscanf(line+6, "%d", &w);
		}else if (!strncmp(line, "height=", 7)){
			sscanf(line+7, "%d", &h);
		}else if (!strncmp(line, "active=", 7)){
			if ((int)atoi(line+7)){
				DWIDTH = w; DHEIGHT = h; DDATA = data;
				mylog("initConfig: activating display:'%s' control:'%s' width:%i height:%i\n", dd, pd, 1+display->right-display->left, 1+display->btm-display->top);
				int ret = lSelectDevice(hw, dd, pd, 1+display->right-display->left, 1+display->btm-display->top, DBPP, DDATA, display);
				if (!ret){
					mylog("initConfig: display failed to activate:'%s' control:'%s'\n", dd, pd);
				}else{
					did += ret;
				}
				display++;
			}
		}else if (!strncmp(line, "[display ", 9)){
			//j = atoi(line+9)-1;
			//if (j < DISPLAYMAX)
			//	display = &displays[j];
		}else if (!strncmp(line, "portdriver=", 11)){
			sscanf(line+11, "%s", pd);
		}else if (!strncmp(line, "displaydriver=", 14)){
			sscanf(line+14, "%s", dd);
		}else if (!strncmp(line, "displaywindow=", 14)){
			sscanf(line+14, "%d,%d,%d,%d", &display->left, &display->top, &display->right, &display->btm);
		}
	}while(++i < al->tlines);
	freeASCIILINE(al);


	if (did){
		lResizeFrame(frame, DWIDTH, DHEIGHT, 0);
		lSetCapabilities(hw, CAP_BACKBUFFER, CAP_STATE_ON);

		lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
		lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
		lSetPixelWriteMode(frame, LSP_SET);
		lClearFrame(frame);
		//printf("DiD %i\n", did);
		
		virtualDisplayId = did;
		return did;
	}else{
		mylog("initConfig: exiting without activating a display\n");
		demoCleanup();
		return 0;
	}
}


#endif

