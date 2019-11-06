
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



#ifndef _CONTEXT_H_
#define _CONTEXT_H_




#define APP_ICON					101

enum _bitmapicons {
	BM_FREQ,
	BM_TOTAL
};


#define TRAY_MENU_BASE				(10)

enum _context_menu{
	TRAY_MENU_TFREQ = TRAY_MENU_BASE,
	TRAY_MENU_START = 100,
	TRAY_MENU_STOP,
	TRAY_MENU_NEXT,
	TRAY_MENU_CLEAN,
	TRAY_MENU_GOTONOW,
	TRAY_MENU_COPYINFO,
	TRAY_MENU_SEPARATOR,
	TRAY_MENU_ADDEDMENU,
	TRAY_MENU_OTHERMENU,
	TRAY_MENU_QUIT
};


#define TRAY_MENU_CONTEXT_BASE		(TRAY_MENU_BASE)
#define TRAY_MENU_CONTEXT_MAX		(64)
#define TRAY_MENU_CONTEXT_UPPER		(TRAY_MENU_CONTEXT_BASE+TRAY_MENU_CONTEXT_MAX-1)


#define TRAY_MENU_FREQ_MASK			(0xFFFF)
#define TRAY_MENU_FREQ_SCAN			(0xFFFA)
#define TRAY_MENU_FREQ_CLEAN		(0xFFFB)




#define WINTOOLBAR_NANELENGTH		(63)
#define WINSYSTRAY_IMAGETOTAL		BM_TOTAL


typedef struct{
	int enabled;			// tray, menu, etc.. anything else
	int tipsEnabled;
	int infoEnabled;
	HANDLE hwnd;			// handle to main window
	HBITMAP hbm[WINSYSTRAY_IMAGETOTAL];	// BM_ resource icons

	struct{					// right click context menu
		HMENU hmenu;
		POINT menuPos;
	}context;
	
	struct{					// all channels menu
		HMENU hmenu;
		int allIdx;			// while this popup lies within the menu
	}channels;
	
	struct{					// all channels menu
		HMENU hmenu;
	}other;				
}systray_t;



//int contextMenuCreate (systray_t *tray);
void contextMenuDestroy (systray_t *tray);

void contextMenuShow (systray_t *tray, const int calcPosition);
void contextMenuHide (systray_t *tray);

void contextMenuEnableItem (systray_t *tray, const int id);
void contextMenuDisableItem (systray_t *tray, const int id);

int contextMenuSetImage (systray_t *tray, const int itemId, const int bm_image);
int contextMenuRemoveImage (systray_t *tray, const int itemId);


void taskbarTrayClose (systray_t *tray);
void initTray (systray_t *tray, HWND hwnd);


#endif

