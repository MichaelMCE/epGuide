
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





#ifndef _COMMON_H_
#define _COMMON_H_



//#include <shlwapi.h>
#include <inttypes.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <mylcd.h>
#include "ufont/ufont.h"
#include "ufont/ufont_primitives.h"



int drawRectangle (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawLine (TFRAME *frame, int x1, int y1, int x2, int y2, const int colour);
int drawLineDotted (TFRAME *frm, const int x1, const int y1, const int x2, const int y2, const int colour);




#define VLC_VERSION				"2.0.7"
#define VLC_PATH				"M:\\RamDiskTemp\\vlc\\vlc.exe"
#define VLC_ARGS				"--high-priority --started-from-file --one-instance --quiet --ts-silent"
#define VLC_MRL					"dvb-t://frequency="
#define VLC_MRLLEN				(32)

#define SKINDROOT				L""
//#define KP_CONFIGFILE			SKINDROOT "/keypad.cfg"


#define COL_ALPHA(a)			((a)<<24)
#define COL_RED					(0xFF0000)
#define COL_GREEN				(0x00FF00)
#define COL_BLUE				(0x0000FF)
#define COL_WHITE				(0xFFFFFF)
#define COL_BLACK				(0x000000)
#define COL_GRAY				(0x111111 * 8)
#define COL_PURPLE				(COL_RED|COL_BLUE)
#define COL_YELLOW				(COL_RED|COL_GREEN)
#define COL_AQUA				(COL_GREEN|COL_BLUE)
#define COL_CYAN				(0x00B7EB)
#define COL_ORANGE				(0xFF7F11)
#define COL_BLUE_SEA_TINT		(0x508DC5)		/* blue, but not too dark nor too bright. eg; Glass Lite:Volume */
#define COL_GREEN_TINT			(0x00FF1E)		/* softer green. used for highlighting */
#define COL_PURPLE_GLOW			(0xFF10CF)		/* nice when used as a text glow effect */
#define COL_HOVER				(0xB328C672)
#define COL_TASKBARFR			(0x141414)
#define COL_TASKBARBK			(0xD4CAC8)
#define COL_SOFTBLUE			(0X7296D3)
#define ALPHA					COL_ALPHA(0xFF)

#define NSEX_LEFT				(0x01)
#define NSEX_RIGHT				(0x02)

#define SWIPE_UP				(0)
#define SWIPE_DOWN				(1)
#define SWIPE_SLIDE				(2)

#define TOUCH_VINPUT			(0x1000)


#define HK_CONTROL				VK_CONTROL
#define HK_ALT					VK_MENU
#define HK_SHIFT				VK_SHIFT


#define SKINFILEBPP				LFRM_BPP_32A
#define MAX_PATH_UTF8			((8*MAX_PATH)+1) 	/* should be large enough to cover every combination and eventuality */
#define MOFFSETX				(4)					/* cursor offset point x within bitmap */
#define MOFFSETY				(3)					/* as above but for y */


#define my_snprintf				__mingw_snprintf
#define my_sprintf				__mingw_sprintf
#define my_swprintf				__mingw_swprintf

#define my_memcpy(s1, s2, n)	memcpy(s1, s2, n)
#define my_malloc(n)			malloc(n)
#define my_calloc(n, e)			calloc(n, e)
#define my_realloc(p, n)		realloc(p, n)
#define my_free(p)				free(p)
#define my_strdup(p)			strdup(p)
#define my_wcsdup(p)			wcsdup(p)



#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


//#define ALLOW_FALLTHROUGH	
#define ALLOW_FALLTHROUGH		__attribute__ ((fallthrough))		// C and C++03
//#define ALLOW_FALLTHROUGH		[[gnu::fallthrough]]				// C++11 and C++14
//#define ALLOW_FALLTHROUGH		[[fallthrough]]						// C++17 and above




#include "common/fileal.h"
#include "common/lock.h"
#include "common/stack.h"
#include "common/list.h"
#include "common/fileal.h"
#include "common/settings.h"
//#include "common/editbox.h"
#include "common/tree.h"
#include "common/artc.h"
#include "common/transform.h"
#include "common/stringcache.h"
#include "common/imagec.h"
#include "cc/cc.h"
#include "win/context.h"
#include "win/wingui.h"
#include "epg/vlcheaders.h"
#include "epg/vlc.h"
#include "epg/provider.h"
#include "guide.h"



// frame copy with vertical bound clipping. src alpha is copied (no blending)
#define fastFrameCopy(_src, _des, _dx, _dy) \
{\
	const TFRAME *const __src = (TFRAME*)(_src);\
	const TFRAME *__des = (TFRAME*)(_des);\
	int __dx = (int)(_dx);\
	int __dy = (int)(_dy);\
\
	const int __spitch = __src->pitch;\
	const int __dpitch = __des->pitch;\
	void * restrict __psrc;\
	int __dys = 0;\
\
	if (__dy < 0){\
		__dys = abs(__dy);\
		if (__dys >= __src->height) goto copyEnd;\
		__psrc = lGetPixelAddress(__src, 0, __dys);\
		__dy = 0;\
	}else{\
		__psrc = lGetPixelAddress(__src, 0, 0);\
	}\
\
	if (__dy > __des->height-1)\
		__dy = __des->height-1;\
	void *restrict __pdes = lGetPixelAddress(__des, __dx, __dy);\
\
	int __r = __src->height - __dys;\
	if (__dy + __r > __des->height-1){\
		__r = (__des->height - __dy) - 1;\
		if (__r > 1) __r++;\
	}\
\
	while(__r-- > 0){\
		my_memcpy(__pdes, __psrc, __spitch);\
		__psrc += __spitch;\
		__pdes += __dpitch;\
	}\
copyEnd:;\
}



// ############################################################################################################################
// ############################################################################################################################
// ############################################################################################################################
// ############################################################################################################################


void com_swipeReset (TTOUCHSWIPE *swipe);

TFRAME *com_newImage (void *ctx, const wchar_t *filename, const int bpp);

void com_drawHex (TFRAME *frame, const int x, const int y, const char *font, const int var, const int colour);
void com_drawShadowUnderlay (void *ctx, TFRAME *frame, const int x, const int y, const int w, const int h, const int shadow);



wchar_t *com_removeLeadingSpacesW (wchar_t *var);
wchar_t *com_removeTrailingSpacesW (wchar_t *var);
char *com_removeLeadingSpaces (char *var);
char *com_removeTrailingSpaces (char *var);

void com_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
void com_copyAreaNoBlend (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
void com_copyAreaScaled (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height);

#define com_drawImage(s,d,dx,dy,x2,y2) com_copyArea((s),(d),(dx),(dy),0,0,(x2),(y2))
#define com_drawImageNoBlend(s,d,dx,dy,x2,y2) com_copyAreaNoBlend((s),(d),(dx),(dy),0,0,(x2),(y2))
#define com_drawImageOpaque(s,d,dx,dy,x2,y2) com_copyAreaNoBlend((s),(d),(dx),(dy),0,0,x2,(y2))


void com_drawImageScaledCenter (TFRAME *img, TFRAME *dest, const double scale, const double offsetX, const double offsetY);
void com_drawImageScaledOpacity (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height, const float opacityAlpha, const float opacityRGB);
void com_drawImageOpacity (TFRAME *from, TFRAME *to, const int dest_x, const int dest_y, double opacity);

void com_imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h);

uint32_t com_getHash (const char *str);			// ascii/utf8
uint32_t com_getHashW (const wchar_t *strw);	// utf16
//uint32_t com_generateHash (const void *data, const size_t dlen);

char *com_convertto8 (const wchar_t *wide);
wchar_t *com_converttow (const char *utf8);

void com_removeChr (char *str, const char a);

#define com_getTickCount() GetTickCount64()
double com_getTime(void *ctx);
void com_getTimeInit (void *ctx);


void com_renderSignalUpdate (void *ctx);
void com_setTargetRate (void *ctx, const double fps);


TFRAME *com_newStringEx  (void *ctx,       TMETRICS *metrics, const int bpp, const int flags, const char *font, const char *text, const int maxW, const int nsex_flags);
TFRAME *com_newStringEx2 (void *ctx, const TMETRICS *metrics, const int bpp, const int flags, const char *font, const char *text, int x, int y, const int maxW, const int maxH, const int nsex_flags);

wchar_t *com_buildSkinD (void *ctx, wchar_t *buffer, wchar_t *file);
wchar_t *com_buildSkinDEx (void *ctx, wchar_t *buffer, wchar_t *dir, wchar_t *file);


void com_drawShadowedImageAlpha (TFRAME *img1, TFRAME *dest, const int x, const int y, uint32_t colour, const int radius, const int offsetX, const int offsetY, const double alpha);
void com_drawShadowedImageComputed (TFRAME *img1, TFRAME *dest, TFRAME *img2, const int x, const int y, const int offsetX, const int offsetY);
void com_drawShadowedImageComputedScaled (TFRAME *img1, TFRAME *dest, TFRAME *img2, const int x, const int y, const int offsetX, const int offsetY, const float scale);
TFRAME *com_drawShadowedImageCreateBlurMask (TFRAME *img1, uint32_t colour, const int radius);


void com_outlineTextEnable (void *hSurfaceLib, const int colour);
void com_outlineTextDisable (void *hSurfaceLib);
void com_shadowTextEnable (void *hSurfaceLib, const int colour, const unsigned char trans);
void com_shadowTextDisable (void *hSurfaceLib);

void com_sendGlobalHotkey (const int modA, const int modB, const char key);

int com_processCreate (const char *cmdLine);
int com_processCreateW (const wchar_t *cmdLine);

HWND com_getVLCIPCHandle ();
void com_sendToVLC (HWND ipcwindow, const uint32_t transponderFreq, const int32_t channelId);
int com_startVLC (const uint32_t transponderFreq, const int32_t channelId);

void com_frame16ToBuffer24 (TFRAME *frm, uint8_t *buffer);
void com_frame16ToBuffer32 (TFRAME *frm, uint8_t *buffer);

#endif

