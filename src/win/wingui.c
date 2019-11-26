
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
#include <shlobj.h>
#include <gdiplus/gdiplus.h>






// demos.h
extern THWD *hw;
extern int DWIDTH;
extern int DHEIGHT;
extern int virtualDisplayId;


HANDLE hMsgWin;
static uint32_t winMsgThreadID;
static uintptr_t hWinMsgThread;
//static uint32_t winDispatchThreadID;
static uintptr_t hWinDispatchThread;
static TMLOCK *hDispatchLock;
static HANDLE hDispatchEvent;
static int showWindowState = 1;
static uint32_t taskbarRestartMsg[2];
static systray_t tray;
static int mouseCapState = MOUSE_CAP_OFF;




// send buffer to clipboard
static inline int copyClipBoardTextW (HWND hwnd, const wchar_t *cwdBuffer)
{
	DWORD len = wcslen(cwdBuffer);
	if (!len) return 0;

    // Allocate string for cwd
    HGLOBAL hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
    if (hdst){
    	LPWSTR dst = (LPWSTR)GlobalLock(hdst);
    	my_memcpy(dst, cwdBuffer, len * sizeof(WCHAR));
    	dst[len] = 0;

		// Set clipboard data
		if (!OpenClipboard(hwnd)){
			GlobalUnlock(hdst);
			return GetLastError();
		}
		
    	EmptyClipboard();
    	if (!SetClipboardData(CF_UNICODETEXT, hdst)){
    		CloseClipboard();
    		GlobalUnlock(hdst);
    		return GetLastError();
    	}
    	
    	CloseClipboard();
    	GlobalUnlock(hdst);
    	//GlobalFree(hdst);
    	return 1;
    }
    return 0;
}

static inline int clipboardSend (const char *str8)
{
	int ret = 0;
	wchar_t *strw = com_converttow(str8);
	if (strw){
		ret = copyClipBoardTextW(hMsgWin, strw);
		my_free(strw);
	}
	return ret;
}

int winuiGetWindowState ()
{
	return showWindowState;
}


static inline void centreCursor (widgets_t *ui, int x, int y)
{
	int resetCur = 0;
	  	
	if (x < 15){
	 	x = 15;
	 	resetCur = 1;
	 }else{
		int w = GetSystemMetrics(SM_CXSCREEN);
		if (x > w-15) x = w-15;
		resetCur = 1;
	}
	if (y < 15){
		y = 15;
		resetCur = 1;
	}else{
	    int h = GetSystemMetrics(SM_CYSCREEN);
	    if (y > h-15) y = h-15;
	    resetCur = 1;
	}
	if (resetCur)
		SetCursorPos(x, y);
		
  	ui->cursor.x = x;
  	ui->cursor.y = y;
	ui->cursor.dx = DWIDTH/2;
	ui->cursor.dy = DHEIGHT/2;
  	ui->cursor.isHooked = 1;
}

static inline void mouseMove (widgets_t *ui, const int buttonState, const int x, const int y, const int opMode)
{
	if (opMode == 0){
		// sanity check
		if (ui->cursor.isHooked != 1)
	  		centreCursor(ui, x, y);

  		ui->cursor.dx -= (ui->cursor.x - x);
		ui->cursor.dy -= (ui->cursor.y - y);
		
	}else if (opMode == 1){
		ui->cursor.dx += x;
		ui->cursor.dy += y;
		
	}else if (opMode == 2){
		ui->cursor.dx = x;
		ui->cursor.dy = y;
	}

	if (ui->cursor.dx > DWIDTH-1)
		ui->cursor.dx = DWIDTH-1;		
	else if (ui->cursor.dx < 0)
		ui->cursor.dx = 0;
		
	if (ui->cursor.dy > DHEIGHT-1)
		ui->cursor.dy = DHEIGHT-1;
	else if (ui->cursor.dy < 0)
		ui->cursor.dy = 0;

  	if (ui->cursor.LBState){	// enable mouse drag
  		TTOUCHCOORD pos;
  		pos.time = com_getTime(ui->cc);
  		pos.x = ui->cursor.dx;
  		pos.y = ui->cursor.dy;
  		pos.pen = 0;
  		pos.dt = 10;
  		pos.z1 = 100;
  		pos.z2 = 100;
  		pos.pressure = 100;
  		ui->cursor.LBState = 2;
	 	touchSimulate(ui, &pos, TOUCH_VINPUT|1, ui->cc);

  	}else if (opMode != 2){
  		//const double t0 = getTime(vp);
 		//if (t0 - vp->lastRenderTime > ((1.0/(double)(UPDATERATE_MAXUI+10.0))*1000.0))
  			// renderSignalUpdate(vp);
  	}
}

static inline void mouseLBDown (widgets_t *ui, const int x, const int y)
{
	//printf("mouseLBDown %i,%i\n", x, y);

	TTOUCHCOORD pos;
	pos.x = ui->cursor.dx;
	pos.y = ui->cursor.dy;
	pos.pen = 0;
	pos.time = com_getTime(ui->cc);
	pos.dt = 1000;
	pos.z1 = 100;
	pos.z2 = 100;
	pos.pressure = 100;
	ui->cursor.LBState = 1;
	touchSimulate(ui, &pos, TOUCH_VINPUT|0, ui->cc);

	/*pos.pen = 1;	// generate a finger up response
	pos.dt = 10;
	touchSimulate(ui, &pos, TOUCH_VINPUT|3, vp);*/

	//// renderSignalUpdate(vp);
}

static inline void mouseLBUp (widgets_t *ui, const int x, const int y)
{
	//printf("mouseLBUp %i,%i %i\n", x, y, ui->cursor.LBState);
	
	if (ui->cursor.LBState/* == 2*/){
		TTOUCHCOORD pos;
	  	pos.x = ui->cursor.dx;
	  	pos.y = ui->cursor.dy;
	  	pos.pen = 1;
	  	pos.time = com_getTime(ui->cc);
	  	pos.dt = 5;
	  	pos.z1 = 100;
	  	pos.z2 = 100;
	  	pos.pressure = 100;
		touchSimulate(ui, &pos, TOUCH_VINPUT|3, ui->cc);
	}
	ui->cursor.LBState = 0;
}

static inline void mouseMBDown (widgets_t *ui, const int x, const int y)
{
	if (ui->cursor.isHooked){
		//captureMouse(0);
		//mHookUninstall();
	}
	ui->cursor.MBState = 1;
	// renderSignalUpdate(vp);
}
 
static inline void mouseMBUp (widgets_t *ui, const int x, const int y)
{
	ui->cursor.MBState = 0;
}

static inline void mouseRBDown (widgets_t *ui, const int x, const int y)
{
	//printf("mouseRBDown %i %i\n", x, y);
	
	ui->cursor.RBState = 1;
	// renderSignalUpdate(vp);
}

static inline void mouseRBUp (widgets_t *ui, const int x, const int y)
{
	ui->cursor.RBState = 0;
}	

static inline void mouseWheelForward (widgets_t *ui, const int x, const int y)
{
	//printf("mouseWheelForward %ix%i\n", x, y);
	
	guideScrollUp();
}

static inline void mouseWheelBack (widgets_t *ui, const int x, const int y)
{
	//printf("mouseWheelBack %ix%i\n", x,y);

	//unsigned int mpos = ((x&0xFFFF)<<16) | (y&0xFFFF);
	//page2Input(vp->pages, PAGE_IN_MOUSE, &mpos, PAGE_IN_WHEEL_BACK);
	guideScrollDown();
}

static inline void mouseWheelLeft (widgets_t *ui, const int x, const int y)
{
	//printf("mouseWheelLeft %ix%i\n", x,y);

	//unsigned int mpos = ((x&0xFFFF)<<16) | (y&0xFFFF);
	//page2Input(vp->pages, PAGE_IN_MOUSE, &mpos, PAGE_IN_WHEEL_LEFT);
}

static inline void mouseWheelRight (widgets_t *ui, const int x, const int y)
{
	//printf("mouseWheelRight %ix%i\n", x,y);

	//unsigned int mpos = ((x&0xFFFF)<<16) | (y&0xFFFF);
	//page2Input(vp->pages, PAGE_IN_MOUSE, &mpos, PAGE_IN_WHEEL_RIGHT);
}

static inline void mouseWheel (widgets_t *ui, const int msg, const int x, const int y)
{
	if (msg == WM_MWHEEL_FORWARD){
		//if (renderLock(vp)){
			mouseWheelForward(ui, x, y);
			//renderUnlock(vp);
		//}
	}else if (msg == WM_MWHEEL_BACK){
		//if (renderLock(vp)){
			mouseWheelBack(ui, x, y);
			//renderUnlock(vp);
		//}
	}else if (msg == WM_MWHEEL_LEFT){
		//if (renderLock(vp)){
			mouseWheelLeft(ui, x, y);
			//renderUnlock(vp);
		//}		
	}else if (msg == WM_MWHEEL_RIGHT){
		//if (renderLock(vp)){
			mouseWheelRight(ui, x, y);
			//renderUnlock(vp);
		//}
	}		
}

static inline void mouseButton (widgets_t *ui, const int msg, const int x, const int y)
{
	//printf("mouseButton %i, %i %i\n", msg, x, y);

	if (msg == WM_MOUSEMOVE)
		mouseMove(ui, -1, x, y, 0);
		
	else if (msg == WM_LBUTTONDOWN)
		mouseLBDown(ui, x, y);
		
	else if (msg == WM_LBUTTONUP)
		mouseLBUp(ui, x, y);
		
	else if (msg == WM_MBUTTONDOWN)
		mouseMBDown(ui, x, y);
		
	else if (msg == WM_MBUTTONUP)
		mouseMBUp(ui, x, y);
		
	else if (msg == WM_RBUTTONDOWN)
		mouseRBDown(ui, x, y);
		
	else if (msg == WM_RBUTTONUP)
		mouseRBUp(ui, x, y);
}


// used for capturing mouse enter/exit virtual window, otherwise not required
#if 1
void mouseRawInputInit (HANDLE hwnd)
{

#if 0
	UINT nDevices;
	if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0){ 
		return;
	}
	
	PRAWINPUTDEVICELIST pRawInputDeviceList = (RAWINPUTDEVICELIST*)my_malloc(sizeof(RAWINPUTDEVICELIST) * nDevices);
	GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST));
	// do the job...
	//printf("Number of raw input devices: %i\n\n", nDevices);


	char *types[] = {"Mouse", "Keyboard", "HID", "unknown"};

	for (int i = 0; i < nDevices; i++){
		int type = pRawInputDeviceList[i].dwType;
		HANDLE hdevice = pRawInputDeviceList[i].hDevice;
		
		printf("device %i, type:%i, %s \n", i, type, types[type]);
		if (type == RIM_TYPEMOUSE){
			RID_DEVICE_INFO devinfo;
			unsigned int dsize = sizeof(devinfo);
			GetRawInputDeviceInfoA(hdevice, RIDI_DEVICEINFO, &devinfo, &dsize);
			//printf("# ret %i %i\n", ret, dsize);
			printf("\t id:%i buttons:%i samRate:%i\n", (int)devinfo.mouse.dwId, (int)devinfo.mouse.dwNumberOfButtons, (int)devinfo.mouse.dwSampleRate);
			
		}else if (type == RIM_TYPEKEYBOARD){
			RID_DEVICE_INFO devinfo;
			unsigned int dsize = sizeof(devinfo);
			GetRawInputDeviceInfoA(hdevice, RIDI_DEVICEINFO, &devinfo, &dsize);

			printf("\t type:%i subType:%i mode:%i tFuncKeys:%i tIndicators:%i tKeys:%i\n",
			    (int)devinfo.keyboard.dwType,
    			(int)devinfo.keyboard.dwSubType,
    			(int)devinfo.keyboard.dwKeyboardMode,
    			(int)devinfo.keyboard.dwNumberOfFunctionKeys,
    			(int)devinfo.keyboard.dwNumberOfIndicators,
    			(int)devinfo.keyboard.dwNumberOfKeysTotal);

		}else if (type == RIM_TYPEHID){
			RID_DEVICE_INFO devinfo;
			unsigned int dsize = sizeof(devinfo);
			GetRawInputDeviceInfoA(hdevice, RIDI_DEVICEINFO, &devinfo, &dsize);

    		printf("\t vid:%X pid:%X ver:%i usagePage:%i usage:%i\n",
    			(int)devinfo.hid.dwVendorId,
    			(int)devinfo.hid.dwProductId,
    			(int)devinfo.hid.dwVersionNumber,
    			(int)devinfo.hid.usUsagePage,
    			(int)devinfo.hid.usUsage);
		}
		printf("\n");
	}

	// after the job, free the RAWINPUTDEVICELIST
	my_free(pRawInputDeviceList);

#endif

	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 1;
	Rid.usUsage = 2; 
	Rid.dwFlags = RIDEV_INPUTSINK|RIDEV_EXINPUTSINK/*| RIDEV_NOLEGACY*//* | RIDEV_CAPTUREMOUSE*/; ///*RIDEV_REMOVE |*/ RIDEV_NOHOTKEYS /*| RIDEV_EXINPUTSINK */| RIDEV_CAPTUREMOUSE;
	Rid.hwndTarget = hwnd;
	
	if (RegisterRawInputDevices(&Rid, 1/*nDevices*/, sizeof(RAWINPUTDEVICE)) == FALSE){
		//printf("RawInput init failed:\n");
	}
	return;
}

static inline void mouseRawInputProcess (HRAWINPUT ri)
{
	//printf("@@ mouseRawInputProcess %X\n", (int)ri);
	
	UINT dwSize;
	GetRawInputData(ri, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	RAWINPUT *raw = my_malloc(sizeof(RAWINPUT*) * dwSize);
	if (raw == NULL) return;
	
	//const int totalRecords = dwSize/sizeof(RAWINPUTHEADER);
	GetRawInputData(ri, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER));

	//printf("rawInput type:%i\n", (int)raw->header.dwType);

	if (raw->header.dwType == RIM_TYPEMOUSE){
		if (raw->data.mouse.usButtonFlags&RI_MOUSE_LEFT_BUTTON_UP){
			if (mouseCapState == MOUSE_CAP_ON){
				mouseCapState = MOUSE_CAP_OFF;
				//printf("rawInput UP %i,%i\n", ui->cursor.dx, cursor.dy);
				widgets_t *ui = guideGetUi();
				mouseButton(ui, WM_LBUTTONUP, ui->cursor.dx, ui->cursor.dy);
				setSigRender();
			}
		}else if (0 && raw->data.mouse.usButtonFlags&RI_MOUSE_WHEEL){
			
			int16_t dir = (int16_t)raw->data.mouse.usButtonData;
			
			if (dir == WHEEL_DELTA){
				printf("Wheel Up\n");
			}else if (dir == -WHEEL_DELTA){
				printf("Wheel Down\n");
			}			
#if 0
			wprintf(L"\nrawMouse:usFlags=%04x\nulExtraInformation:%i\nulButtons=%04x \nusButtonFlags=%04x \nusButtonData=%04x \nulRawButtons=%04x \nlLastX=%ld \nlLastY=%ld\n"
				"\n%i\n",
				
				raw->data.mouse.usFlags, (int)raw->data.mouse.ulExtraInformation, 
				raw->data.mouse.ulButtons, raw->data.mouse.usButtonFlags, 
				raw->data.mouse.usButtonData, raw->data.mouse.ulRawButtons,
				raw->data.mouse.lLastX, raw->data.mouse.lLastY,
				(int16_t)raw->data.mouse.usButtonData
				);
#endif			
		}

		
	}else if (raw->header.dwType == RIM_TYPEKEYBOARD){
		//printf("rawinput key %i %c %X %i\n", raw->data.keyboard.VKey, raw->data.keyboard.VKey, raw->data.keyboard.Message, raw->data.keyboard.Flags);
#if 0
		if (raw->data.keyboard.Flags == RI_KEY_MAKE){
			if (page2RenderGetState(vp->pages, PAGE_TETRIS)){	// is page visable
	  			tetrisInputProc(vp, pageGetPtr(vp, PAGE_TETRIS), raw->data.keyboard.VKey);
	  			renderSignalUpdate(vp);
	  			return;
	  		}
	  	}
#endif
	}

	//DefRawInputProc(&raw, totalRecords, sizeof(RAWINPUT));
	my_free(raw); 
}
#endif

static inline void contextMenuRemoveFreqImages (systray_t *tray)
{
	for (int i = 0; i < TRAY_MENU_CONTEXT_MAX; i++){
		if (!providerGetMrlFreq(i)) break;
		contextMenuRemoveImage(tray, i);
	}
}

static inline void contextUpdateProviderStatus (systray_t *tray)
{
	const int runState = providerGetState();
	if (!runState){
		contextMenuEnableItem(tray, TRAY_MENU_START);
		contextMenuDisableItem(tray, TRAY_MENU_STOP);
	}else{
		contextMenuEnableItem(tray, TRAY_MENU_STOP);
		contextMenuDisableItem(tray, TRAY_MENU_START);
	}
}

void winuiMenuUpdate ()
{
	contextMenuRemoveFreqImages(&tray);
	if (providerGetState())
		contextMenuSetImage(&tray, guideGetStationIdx(), BM_FREQ);
	contextUpdateProviderStatus(&tray);
}

static inline void channelMenuClean (HMENU channelsMenu)
{
	const int total = GetMenuItemCount(channelsMenu);
	for (int i = total; i >= 0; i--)
		RemoveMenu(channelsMenu, i, MF_BYPOSITION);
}

static inline int channelMenuAddChannels (HMENU channelsMenu, const int tFreqIdx)
{
	const int bufferLen = 32;
	char buffer[bufferLen];
	int chIdx = 0;	
	int ret = 0;
	int tAdded = 0;
	uint32_t tFreq = providerGetMrlFreq(tFreqIdx);
	
	do{
		if ((ret=guideGetChannelName(tFreq, chIdx, buffer, bufferLen)))
			tAdded += AppendMenuA(channelsMenu, MF_ENABLED|MF_STRING, ((tFreqIdx+1)<<16)|chIdx, buffer);
		chIdx++;
	}while(ret);

	return tAdded;
}

static inline void channelMenuAddCmds (HMENU channelsMenu, const int tFreqIdx)
{
	AppendMenuA(channelsMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA(channelsMenu, MF_ENABLED|MF_STRING, ((tFreqIdx+1)<<16)|TRAY_MENU_FREQ_SCAN,  "Scan");
	AppendMenuA(channelsMenu, MF_ENABLED|MF_STRING, ((tFreqIdx+1)<<16)|TRAY_MENU_FREQ_CLEAN, "Clean");
}

static inline int channelMenuAddAllChannels (HMENU channelsMenu)
{
	uint32_t freq;
	int32_t i = 0;
	int tAdded = 0;

	while((freq=providerGetMrlFreq(i)))
		tAdded += channelMenuAddChannels(channelsMenu, i++);
	return tAdded;
}

static inline void menuFillChannellist (const int tFreqIdx)
{
	//printf("menuFillChannellist: %i\n", tFreqIdx);

	HMENU tfreqMenu = providerGetData(tFreqIdx);
	
	channelMenuClean(tfreqMenu);
	channelMenuAddChannels(tfreqMenu, tFreqIdx);
	channelMenuAddCmds(tfreqMenu, tFreqIdx);
}

static inline void menuFillAllChannellist (const int tFreqIdx)
{
	//printf("menuFillAllChannellist: %i\n", tFreqIdx);

	channelMenuClean(tray.channels.hmenu);
	if (!channelMenuAddAllChannels(tray.channels.hmenu))
		AppendMenuA(tray.channels.hmenu, MF_SEPARATOR, 0, NULL);
}

static inline void sendToVLC (const uint32_t freqIdx, const uint32_t chIdx, uint32_t pid)
{
	uint32_t transponderFreq = providerGetMrlFreq(freqIdx);
	if (!transponderFreq) return;
	if (!pid)
		pid = guideGetChannelPid(transponderFreq, chIdx);

	//printf("--> %i %i\n", (int)transponderFreq, (int)channelId);

	if (transponderFreq && pid){
		HWND hwnd = com_getVLCIPCHandle();
		if (hwnd)
			com_sendToVLC(hwnd, transponderFreq, pid);
		else
			com_startVLC(transponderFreq, pid);
	}
}

static inline void contextMenuHandleSelection (systray_t *tray, HWND hwnd, const int itemId)
{
	if (itemId == TRAY_MENU_START){
		guideTimerStartCycleLong();
		guideProviderContinue();
		contextMenuSetImage(tray, guideGetStationIdx(), BM_FREQ);
		contextUpdateProviderStatus(tray);
		
	}else if (itemId == TRAY_MENU_STOP){
		//timerAutoStop();
		guideProviderStop();
		//contextMenuRemoveFreqImages(tray);
		//contextUpdateProviderStatus(tray);
		 
	}else if (itemId == TRAY_MENU_NEXT){
		guideTimerRestartCycle();
		guideProviderNext();
		contextMenuRemoveFreqImages(tray);
		contextMenuSetImage(tray, guideGetStationIdx(), BM_FREQ);
		contextUpdateProviderStatus(tray);
		
	}else if (itemId == TRAY_MENU_CLEAN){
		guideCleanGuide();

	}else if (itemId == TRAY_MENU_GOTONOW){
		guideScrollTimeNow();
		
	}else if (itemId == TRAY_MENU_COPYINFO){
		char *str8 = guideGetProgrammeInfoStr();
		if (str8){
			clipboardSend(str8);
			my_free(str8);
		}
	}else if (itemId == TRAY_MENU_QUIT){
		guideTimerStopCycle();
		removeApplState();
		PostMessage(tray->hwnd, WM_QUIT, 0, 0);
		PostMessage(hwnd, WM_QUIT, 0, 0);
	}
}

static int hoverActive = 0;
	  	
static inline LRESULT CALLBACK winMsgCb (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//void *ptr = (TVLCPLAYER*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
	//if (message != 255 && message != 5536)
	//	printf("## winMsgCb: %i %i %i\n", message, (int)wParam, (int)lParam);
	
	switch (message){
	  case WM_INPUT:
	  	mouseRawInputProcess((HRAWINPUT)lParam);
	  	//printf("## mouse WM_INPUT: %p, %i %i %i\n", hwnd, message, (int)wParam, (int)lParam);
	  	break;
	  
	  
	  //  WM_DD_ events generated by the libmylcd DirectDraw driver
	  case WM_DD_LBUTTONDOWN:
	  	mouseCapState = MOUSE_CAP_ON;
	  	ALLOW_FALLTHROUGH;
	  case WM_DD_RBUTTONDOWN:
	  case WM_DD_MBUTTONDOWN:{
	  	//printf("\n\n");
	  	widgets_t *ui = guideGetUi();
	  	ui->cursor.dx = lParam&0xFFFF;
	  	ui->cursor.dy = (lParam>>16)&0xFFFF;
	  	
	  	//printf("WM_DD_xBUTTON DOWN %i %i\n",  cursor.dx, cursor.dy);
	  	mouseButton(ui, message-WM_MM, ui->cursor.dx, ui->cursor.dy);
	  	setSigRender();
	  	break;
	  }	
	  // up event handled via rawInput, used only to capture in focus but non client area MOUSEUP
	  case WM_DD_LBUTTONUP:{
	  	break;
	  	
	  	widgets_t *ui = guideGetUi();
	  	ui->cursor.dx = lParam&0xFFFF;
	  	ui->cursor.dy = (lParam>>16)&0xFFFF;
	  	
	  	//printf("WM_DD_xBUTTON UP %i %i\n",  cursor.dx, cursor.dy);
	  	mouseButton(ui, message-WM_MM, ui->cursor.dx, ui->cursor.dy);
	  	//setSigRender();
	  	break;
	  }	
	  case WM_DD_RBUTTONUP:
	  case WM_DD_MBUTTONUP:{
	  	widgets_t *ui = guideGetUi();
	  	ui->cursor.dx = lParam&0xFFFF;
	  	ui->cursor.dy = (lParam>>16)&0xFFFF;
	  	
	  	//if (mouseCapState == MOUSE_CAP_OFF){
	  		//printf("WM_DD_xBUTTON UP %i %i\n", cursor.dx, cursor.dy);
	  		mouseButton(ui, message-WM_MM, ui->cursor.dx, ui->cursor.dy);
	  		setSigRender();
	  	//}
	  	break;
	  }	
	  case WM_DD_MOUSEMOVE:{
	  	widgets_t *ui = guideGetUi();
	  	int x = lParam&0xFFFF;
	  	int y = (lParam>>16)&0xFFFF;
		mouseMove(ui, (int)wParam&0x7, x, y, 2);
	  	//printf("WM_DD_MOUSEMOVE %i %i: %i\n", cursor.dx, cursor.dy, cursor.LBState);
	  	
		if (ccHoverRenderSigGetState(ui->cc)){	// do we want a render update per mouse move event
	  		int objId = ccIsHoveredMM(ui->cc, 1, x, y, 1);
	  		//printf("WM_DD_MOUSEMOVE %i, %i %i\n", objId, x, y);
			if (objId){
				//printf("WM_DD_MOUSEMOVE IsHovered %i %i\n", objId, hoverActive);
				if (ui->cursor.LBState){
					setSigRender();
					hoverActive = 1;
				}else if (objId == ui->paneEPG->id){
					if (paneCellIsHovered(ui->paneEPG, x, y)){
						setSigRender();
						hoverActive = 1;
					}else if (hoverActive){
						hoverActive = 0;
						setSigRender();
					}
				}else if (objId == ui->paneChan->id){
					hoverActive = 1;
					setSigRender();
				}else if (objId == ui->paneCtrl->id){
					hoverActive = 1;
					setSigRender();
				}else if (hoverActive){
					hoverActive = 0;
					setSigRender();
				}
	  		}
		}
	  	break;
	  }
	  case WM_DD_MOUSEHWHEEL:{
	  	int x = (int)lParam&0xFFFF;
	  	int y = (int)(lParam>>16)&0xFFFF;
		const int delta = (wParam>>16)&0xFF;
		
		if (delta == WHEEL_DELTA)
			wParam = WM_MWHEEL_RIGHT;
		else if (delta == WHEEL_DELTA+16)
			wParam = WM_MWHEEL_LEFT;
		else
			break;
			
		HWND wnd = GetForegroundWindow();
		RECT rc, clientrc;
		GetWindowRect(wnd, &rc);
		GetClientRect(wnd, &clientrc);
			
		x -= rc.left;
		y -= rc.top;
		if (x < 0 || x >= clientrc.right) break;
		if (y < 0 || y >= clientrc.bottom) break;

		mouseWheel(guideGetUi(), wParam, x, y);
		break;
	  }
	  case WM_DD_MOUSEWHEEL:{
	  	int x = (int)lParam&0xFFFF;
	  	int y = (int)(lParam>>16)&0xFFFF;
	  	//printf("WM_DD_MOUSEWHEEL: %i %i, %i\n", x, y, (int)(wParam>>16)&0xFFFF);
	  
		if (((int)wParam>>16) > 0)
			wParam = WM_MWHEEL_FORWARD;
		else if (((int)wParam>>16) < 0)
			wParam = WM_MWHEEL_BACK;
		else
			break;

		HWND wnd = GetForegroundWindow();
		RECT rc, clientrc;
		GetWindowRect(wnd, &rc);
		GetClientRect(wnd, &clientrc);
			
		x -= rc.left;
		y -= rc.top;
		if (x < 0 || x >= clientrc.right) break;
		if (y < 0 || y >= clientrc.bottom) break;

		mouseWheel(guideGetUi(), wParam, x, y);
		break;
	  }

	  case WM_DD_MINIMZE:
		//printf("WM_DD_MINIMZE\n");
		lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_HIDE, NULL);
		showWindowState = 0;
	  	break;
	  case WM_DD_MAXIMIZE:
		//printf("WM_DD_MAXIMIZE\n");
		lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_SHOW, NULL);
		showWindowState = 1;
		setSigRender();
	  	break;
	  case WM_DD_RESTORE:
		//printf("WM_DD_RESTORE\n");
		lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_SHOW, NULL);
		showWindowState = 1;
		setSigRender();
	  	break;

	  //case WM_MENURBUTTONUP:
	  	//printf("WM_MENURBUTTONUP: %X %X\n", (int)wParam, (int)lParam);
	  	//break;
	  //case WM_CONTEXTMENU:
	  	//printf("WM_CONTEXTMENU: %X %X\n",  (int)wParam, (int)lParam);
	  	//break;
	  	
	  case WM_MENUSELECT:{
	  	const int itype = (wParam>>16)&0xFFFF;
	  	const int idx = (int)(wParam&0xFFFF);
	  	//POINT pt;
	  	//GetCursorPos(&pt);
	  	//HWND hwin = WindowFromPhysicalPoint(pt);

	  	//printf("WM_MENUSELECT: %i %i, %X %X, %p %p\n", itype, idx, (int)wParam, (int)lParam, (void*)tray.context.hmenu, (void*)tray.channels.hmenu);
		if (itype&MF_POPUP){
	  		if ((HMENU)lParam == tray.context.hmenu && tray.channels.allIdx == idx)
		  		menuFillAllChannellist(idx);
	  		else if ((HMENU)lParam == tray.context.hmenu && idx < TRAY_MENU_CONTEXT_MAX)
	  			menuFillChannellist(idx);
	  	}
	  	break;
	  }
	  case WM_SYSTRAY:
	  	//printf("WM_SYSTRAY: %X %X\n", (int)wParam, (int)lParam);
		if (wParam == APP_ICON){
			if (!tray.enabled) return 0;
			
			if (lParam == WM_LBUTTONUP){
				if (showWindowState){
					lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_HIDE, NULL);
					showWindowState = 0;
				}else{
					lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_SHOW, NULL);
					showWindowState = 1;
					setSigRender();
				}
			}else if (lParam == WM_RBUTTONUP){
				contextMenuShow(&tray, 1);
			}
		}
		break;
	  case WM_ADDCH:{
	  	chpid_t *ch = (chpid_t*)lParam;
		AppendMenuA(tray.other.hmenu, MF_ENABLED|MF_STRING, 0x80000000|(((ch->freqIdx+1)<<16)|(ch->pid&0xFFFF)), ch->name);
		break;
	  }
	  case WM_COMMAND:
	  	//printf("WM_COMMAND: %X %X\n", (int)wParam, (int)lParam);
		if ((wParam&TRAY_MENU_FREQ_MASK) == TRAY_MENU_FREQ_SCAN){			// update channel list and guide items (rescans transponder)
			int32_t tfreqIdx = ((wParam&0xFF0000)>>16)-1;
			guideProviderSet(tfreqIdx);
			guideTimerStartSingle();
			winuiMenuUpdate();
			
  		}else if ((wParam&TRAY_MENU_FREQ_MASK) == TRAY_MENU_FREQ_CLEAN){	// remove all guide items (programme listings) from all channels attached to this tfreq's
  			int32_t tfreqIdx = ((wParam&0xFF0000)>>16)-1;
	  		guideRemoveTransponderListings(tfreqIdx);
	  		setSigRender();
	  		
  		}else if ((wParam&0x80000000)){
  			sendToVLC(((wParam&0xFF0000)>>16)-1,  0, wParam&0xFFFF);

  		}else if ((wParam&0xFF0000)){
  			//printf("channel: %u %i\n", providerGetMrlFreq(((wParam&0xFF0000)>>16)-1), wParam&0xFF);
  			sendToVLC(((wParam&0xFF0000)>>16)-1,  wParam&0xFF, 0);

		}else if (wParam > TRAY_MENU_CONTEXT_UPPER){			// other menu items
	  		contextMenuHandleSelection(&tray, hwnd, wParam);
		}

		break;
	  case WM_CREATE:
		break;
	  case WM_QUIT: 
		DestroyWindow(hwnd);
		ALLOW_FALLTHROUGH;
	  case WM_CLOSE: 
		return 0;
 	  case WM_DESTROY:
		PostQuitMessage(0);
	    return 0;
	  case WM_DD_CLOSE:
	  	removeApplState();
		PostMessage(hwnd, WM_QUIT, 0, 0);
	  	return 0;
	}

	if (message == taskbarRestartMsg[0] || message == taskbarRestartMsg[1]){
		//printf("taskbarRestartMsg %i\n", message);
		taskbarTrayClose(&tray);
		initTray(&tray, hwnd);
		contextUpdateProviderStatus(&tray);
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);	
}

static inline HANDLE initGUI (void *opaque)
{
	InitCommonControls();

	const char *szClassName = "epGuide";
    WNDCLASSEX wincl;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = winMsgCb;
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;
    wincl.hCursor = NULL;
    wincl.hbrBackground = NULL;
    wincl.style = CS_OWNDC;
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    if (!RegisterClassEx (&wincl))
        return NULL;

    HWND hMsgWin = CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,\
	  0, 0, HWND_DESKTOP, NULL, wincl.hInstance, NULL);

	if (hMsgWin){
    	SetWindowLongPtr(hMsgWin, GWLP_USERDATA, (LONG_PTR)opaque);
    	ShowWindow(hMsgWin, SW_HIDE);
    	initTray(&tray, hMsgWin);
    	contextUpdateProviderStatus(&tray);
    }

    return hMsgWin;
}


static inline int processWindowMessages ()
{
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	int ret = 0;
	
	if ((ret=GetMessage(&msg, NULL, 0, 0)) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return ret;
}

static inline unsigned int __stdcall winMessageThread (void *ptr)
{
	//printf("winMessageThread in\n");
	
	hMsgWin = initGUI(NULL);

	taskbarRestartMsg[0] = RegisterWindowMessage("TaskbarCreated");
	taskbarRestartMsg[1] = RegisterWindowMessage("TaskbarButtonCreated");
	mouseRawInputInit(hMsgWin);
	
	// enable WM_DD_ callbacks
	lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_HWNDTARGET, (intptr_t*)hMsgWin);

	while (testApplState())
		processWindowMessages();

	// disable WM_DD_ callbacks
	lSetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_HWNDTARGET, NULL);
	//if (channelsMenu) DestroyMenu(channelsMenu);
	taskbarTrayClose(&tray);

	
	//printf("winMessageThread out\n");
	_endthreadex(1);
	return 1;

}

int startMouseCapture ()
{
	hWinMsgThread = _beginthreadex(NULL, THREADSTACKSIZE, winMessageThread, NULL, 0, &winMsgThreadID);
	//hWinDispatchThread = _beginthreadex(NULL, THREADSTACKSIZE, inputDispatchThread, 0, &winDispatchThreadID);
	hDispatchLock = lockCreate("mouseDispatch");
	hDispatchEvent = CreateEvent(NULL, 0, 0, NULL);
	return (int)winMsgThreadID;
}

void endMouseCapture ()
{
	PostMessage(hMsgWin, WM_QUIT,0,0); // wakeup the message thread
	WaitForSingleObject((HANDLE)hWinMsgThread, INFINITE);
	CloseHandle((HANDLE)hWinMsgThread);
	
	SetEvent(hDispatchEvent);
	WaitForSingleObject((HANDLE)hWinDispatchThread, INFINITE);
	CloseHandle((HANDLE)hWinDispatchThread);
	hWinDispatchThread = 0;
	
	CloseHandle(hDispatchEvent);
	hDispatchEvent = NULL;
	
	lockClose(hDispatchLock);
	hDispatchLock = NULL;
}
