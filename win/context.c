
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



#include "../common.h"
#include <gdiplus/gdiplus.h>




typedef struct{
	int id;
	char *str;
}mitem_t;


static const mitem_t menuStrings[] ={
	{TRAY_MENU_TFREQ,		""					 },
	{TRAY_MENU_SEPARATOR, 	""					 },
	{TRAY_MENU_ADDEDMENU,	"Added channels"	 },
	{TRAY_MENU_OTHERMENU, 	"Other channels"	 },
	{TRAY_MENU_SEPARATOR, 	""					 },
	{TRAY_MENU_CLEAN,		"Clean"				 },
	{TRAY_MENU_GOTONOW,		"Go to Now-Showing"	 },
	{TRAY_MENU_COPYINFO,	"Copy programme info"},
	{TRAY_MENU_SEPARATOR, 	""					 },
	{TRAY_MENU_START,		"Start"				 },
	{TRAY_MENU_STOP,		"Stop"				 },
	{TRAY_MENU_NEXT,		"Next"				 },
	{TRAY_MENU_SEPARATOR, 	""					 },
	{TRAY_MENU_QUIT,		"Quit"				 },
	{0, ""}
};



static inline int contextMenuLoadImage (systray_t *tray, const int how, const int bm_idx, const wchar_t *resourceName)
{
	if (how == 1){
		const int flags = LR_LOADTRANSPARENT | LR_DEFAULTSIZE| LR_LOADMAP3DCOLORS | LR_COPYFROMRESOURCE/*|LR_VGACOLOR*/;
		tray->hbm[bm_idx] = (HBITMAP)LoadImageW(GetModuleHandle(0), resourceName, IMAGE_BITMAP, 0, 0, flags);

	}else if (how == 2){
		HBITMAP bm = NULL;
		GpBitmap *gbm = NULL;
	
		if (!GdipCreateBitmapFromResource(GetModuleHandle(0), resourceName, &gbm)){
			if (!GdipCreateHBITMAPFromBitmap(gbm, &bm, 0xFFFFFFFF))
				tray->hbm[bm_idx] = bm;
			GdipDisposeImage(gbm);
		}
	}else if (how == 3){
		HBITMAP bm = NULL;
		GpBitmap *gbm = NULL;
	
		if (!GdipCreateBitmapFromFile(resourceName, &gbm)){
			if (!GdipCreateHBITMAPFromBitmap(gbm, &bm, 0xFFFFFFFF))
				tray->hbm[bm_idx] = bm;
			GdipDisposeImage(gbm);
		}
	}

	return tray->hbm[bm_idx] != NULL;
}

static inline const char *contextMenuItemToString (const int itemIdx)
{
	for (int i = 0; menuStrings[i].id; i++){
		if (itemIdx == menuStrings[i].id)
			return menuStrings[i].str;
	}
	return "<>";
}

int contextMenuSetImage (systray_t *tray, const int itemIdx, const int bm_image)
{
	if (bm_image >= WINSYSTRAY_IMAGETOTAL) return 0;
	return (int)SetMenuItemBitmaps(tray->context.hmenu, itemIdx, MF_BYPOSITION|MF_BITMAP, tray->hbm[bm_image], NULL);
}

int contextMenuRemoveImage (systray_t *tray, const int itemIdx)
{
	return (int)SetMenuItemBitmaps(tray->context.hmenu, itemIdx, MF_BYPOSITION|MF_BITMAP, NULL, NULL);
}

void taskbarPostMessage (systray_t *tray, const int msg, const int var1, const intptr_t var2)
{
	PostMessage(tray->hwnd, msg, var1, var2);
}

void contextMenuDestroy (systray_t *tray)
{
	if (tray->context.hmenu)
		DestroyMenu(tray->context.hmenu);
	tray->context.hmenu = NULL;
}

void contextMenuHide (systray_t *tray)
{
	//ShowWindow(tray->volume.hwnd, SW_HIDE);
}

void contextMenuShow (systray_t *tray, const int calcPosition)
{
	if (calcPosition){
		if (!GetCursorPos(&tray->context.menuPos))
			return;
		RECT rc;
		GetWindowRect(GetDesktopWindow(), &rc);
		if (tray->context.menuPos.y > rc.bottom-52)
			tray->context.menuPos.y = rc.bottom-52;
	}

	SetForegroundWindow(GetLastActivePopup(tray->hwnd));
	const int flags = TPM_CENTERALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_NOANIMATION;
	TrackPopupMenu(tray->context.hmenu, flags, tray->context.menuPos.x, tray->context.menuPos.y, 0, tray->hwnd, NULL);
	PostMessage(tray->hwnd, WM_NULL, 0, 0);
}

static inline int contextMenuCheckItem (systray_t *tray, const int id)
{
	return ModifyMenuA(tray->context.hmenu, id, MF_CHECKED, id, contextMenuItemToString(id));
}

static inline void contextMenuUnCheckItem (systray_t *tray, const int id)
{
	ModifyMenuA(tray->context.hmenu, id, MF_UNCHECKED, id, contextMenuItemToString(id));
}

void contextMenuEnableItem (systray_t *tray, const int id)
{
	ModifyMenuA(tray->context.hmenu, id, MF_ENABLED, id, contextMenuItemToString(id));
}

void contextMenuDisableItem (systray_t *tray, const int id)
{
	ModifyMenuA(tray->context.hmenu, id, MF_DISABLED|MF_GRAYED, id, contextMenuItemToString(id));
}

static inline int contextMenuAddString (systray_t *tray, const int id)
{
	return AppendMenuA(tray->context.hmenu, MF_STRING, id, contextMenuItemToString(id));
}

static inline int contextMenuAddSeparator (systray_t *tray)
{
	return AppendMenuA(tray->context.hmenu, MF_SEPARATOR, 0, NULL);
}

static inline int contextMenuAddSubmenu (systray_t *tray, const char *str)
{
	tray->channels.hmenu = CreatePopupMenu();
	int ret = AppendMenuA(tray->context.hmenu, MF_POPUP, (UINT_PTR)tray->channels.hmenu, str);
	AppendMenuA(tray->channels.hmenu, MF_SEPARATOR, 0, NULL);
	return ret;
}

static inline int contextMenuAddAllSubmenu (systray_t *tray, const char *str)
{
	tray->other.hmenu = CreatePopupMenu();
	int ret = AppendMenuA(tray->context.hmenu, MF_POPUP, (UINT_PTR)tray->other.hmenu, str);
	//AppendMenuA(tray->all.hmenu, MF_SEPARATOR, 0, NULL);
	return ret;
}

static inline int contextMenuAddFreq (systray_t *tray, const int pos, const char *str)
{
	int ret = 0;
	wchar_t *strw = com_converttow(str);
   	if (strw){
		void *menu = CreatePopupMenu();
		providerSetData(pos, menu);
		ret = AppendMenuW(tray->context.hmenu, MF_POPUP, (UINT_PTR)menu, strw);
		my_free(strw);
	}
	return ret;
}

static inline int contextMenuCreate (systray_t *tray)
{
	tray->context.hmenu = CreatePopupMenu();
	return (tray->context.hmenu != NULL);
}

static inline void contextMenuPopulate (systray_t *tray)
{
	char buffer[32];
	
	int pos = 0;
	for (pos = 0; pos < 32; pos++){
		uint32_t freq = providerGetMrlFreq(pos);
		if (!freq) break;
		my_snprintf(buffer, sizeof(buffer), "%u", freq);
		contextMenuAddFreq(tray, pos, buffer);
	}

	for (int i = 1; menuStrings[i].id; i++){
		if (menuStrings[i].id == TRAY_MENU_SEPARATOR){
			contextMenuAddSeparator(tray);
		}else if (menuStrings[i].id == TRAY_MENU_ADDEDMENU){
			contextMenuAddSubmenu(tray, menuStrings[i].str);
			tray->channels.allIdx = pos+i-1;
		}else if (menuStrings[i].id == TRAY_MENU_OTHERMENU){
			contextMenuAddAllSubmenu(tray, menuStrings[i].str);
			//tray->other.allIdx = pos+i-1;
		}else{
			contextMenuAddString(tray, menuStrings[i].id);
		}
	}
}

void taskbarTrayClose (systray_t *tray)
{
	if (!tray->enabled) return;

	contextMenuDestroy(tray);
	
	for (int i = 0; i < WINSYSTRAY_IMAGETOTAL; i++){
		if (tray->hbm[i])
			DeleteObject(tray->hbm[i]);
	}
	
	NOTIFYICONDATA nData;
	memset(&nData, 0, sizeof(NOTIFYICONDATA));
	nData.cbSize = sizeof(nData);
	nData.hWnd = tray->hwnd;
	nData.uID = APP_ICON;

	Shell_NotifyIconA(NIM_DELETE, &nData);
}

static inline int taskbarTrayInit (systray_t *tray, HWND hwnd)
{

	NOTIFYICONDATA nData;
	memset(&nData, 0, sizeof(NOTIFYICONDATA));

	nData.hWnd = tray->hwnd = hwnd;
	nData.cbSize = sizeof(NOTIFYICONDATA);
	nData.uID = APP_ICON;

	nData.hIcon = LoadIconA(GetModuleHandle(0), "APP_PRIMARY");
	nData.uCallbackMessage = WM_SYSTRAY;
	strcpy(nData.szTip, "epGuide");
	nData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	if (!Shell_NotifyIconA(NIM_ADD, &nData))
		return 0;

	// load resource images (currently 13x13 bitmaps)
	contextMenuLoadImage(tray, 1, BM_FREQ, L"BM_FREQ");
	contextMenuCreate(tray);
	contextMenuPopulate(tray);

	return 1;
}


void initTray (systray_t *tray, HWND hwnd)
{
	taskbarTrayInit(tray, hwnd);
	contextMenuSetImage(tray, guideGetStationIdx(), BM_FREQ);
	tray->enabled = 1;
	
}

