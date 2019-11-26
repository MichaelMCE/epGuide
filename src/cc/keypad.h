
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





#ifndef _KEYPAD_H_
#define _KEYPAD_H_



//key types:
#define KP_KEYS_CHAR		1	// standard key with a unicode value.                               
#define KP_KEYS_SWITCH		2   // pad switch - go to a pad (eg; digits)                            
#define KP_KEYS_TOGGLE		3   // pad toggle - toggle between two pads (eg; shift)                 
#define KP_KEYS_CYCLE		4   // pad cycle - cycle through given list of pads                     
#define KP_KEYS_ENTER		5   // enter - input completed, send string to control and close keypad 
#define KP_KEYS_RETURN		6   // return - as above but leave control open                         
#define KP_KEYS_BACKSPACE	7   // backspace - delete whatever is left of the caret        
#define KP_KEYS_DELETE		9	// delete whatever is to the right of the caret

// listen flags
#define KP_INPUT_SINGLE				0x01		// callback per char entry
#define KP_INPUT_COMPLETE8			0x02		// callback when complete (enter pressed), passing utf8 (char*) string via data ptr
#define KP_INPUT_COMPLETEW			0x04		// callback when complete (enter pressed), passing utf16 (wchar_t*)string via data ptr
#define KP_INPUT_OPENED				0x08		// keypad was opened, no data available
#define KP_INPUT_CLOSED				0x10		// keypad was closed, no data available
#define KP_RENDER_PRE				0x20

//#define KP_CARET		0x21B5


#define KP_PAD_ABC		0
#define KP_PAD_abc		1
#define KP_PAD_DIGITS	2
#define KP_PAD_MISC1	3
#define KP_PAD_MISC2	4


// modifier keys
#define KP_VK_CONTROL	0x01
#define KP_VK_ALT		0x02
#define KP_VK_SHIFT		0x04
#define KP_VK_WIN		0x08
#define KP_VK_APPS		0x10		// context menu key
#define KP_VK_REPEAT	0x80

#define KP_SHIFT		1
#define KP_BACKSPACE	2
#define KP_ABC			3
#define KP_abc			4
#define KP_123			5
#define KP_MISC1		6			/* #@;		*/
#define KP_ENTER		13
#define KP_SPACE		32
#define KP_DOLLAR		36
#define KP_COMMA		44
#define KP_PERIOD		46
#define KP_APOS			39
#define KP_QUESTION		63
#define KP_UNDERSCORE	95
#define KP_VLINE		124
#define KP_TILDE		126
#define KP_CENT			162
#define KP_POUND		163
#define KP_YEN			165
#define KP_BROKENBAR	166
#define KP_COPYRIGHT	169
#define KP_NOTSIGN		172
#define KP_REGISTERED	174
#define KP_DEGREE		176
#define KP_PLUSMINUS	177
#define KP_MICRO		181
#define KP_THORN		222

#define KP_a_ACUTE		225		// á
#define KP_e_ACUTE		233		// é
#define KP_i_ACUTE		237		// í
#define KP_o_ACUTE		243		// ó
#define KP_u_ACUTE		250		// ú
#define KP_y_ACUTE		253		// ý
#define KP_c_ACUTE		263
#define KP_n_ACUTE		324
#define KP_r_ACUTE		341
#define KP_z_ACUTE		378
#define KP_g_ACUTE		501

#define KP_m_ACUTE		7743
#define KP_w_ACUTE		7811

#define KP_WHITESTAR	0x2606
#define KP_WHITEHEART	0x2661

#define KP_LANGLEBRKT	0x3008
#define KP_RANGLEBRKT	0x3009
#define KP_LDBANGLEBRKT	0x300A
#define KP_RDBANGLEBRKT	0x300B
#define KP_LCORNERBRKT	0x300C
#define KP_RCORNERBRKT	0x300D
#define KP_LWCORNERBRKT	0x300E
#define KP_RWCORNERBRKT	0x300F
#define KP_LBLKLENTBRKT	0x3010
#define KP_RBLKLENTBRKT	0x3011

#define KP_CHONE		0x4E00
#define KP_CHTWO		0x4E8C
#define KP_CHTHREE		0x4E09



#define KP_DIVIDE		247
#define KP_MODACCENT	710		// modifier letter flex accent
#define KP_DELTA		916
#define KP_OMEGA		937
#define KP_PI			960

#define KP_LDBLQUOTE	8220
#define KP_RDBLQUOTE	8221
#define KP_LANGLEQUOTE	8249
#define KP_RANGLEQUOTE	8250
#define KP_EURO			8364
#define KP_TRADEMARK	8482
#define KP_OHM			8486
#define KP_INFINITY		8734
#define KP_INTEGRAL		8747

#define KP_ALMOSTEQUAL	8776
#define KP_LESSTHAN		8804
#define KP_GREATERTHAN	8805
#define KP_SMILEY		9786

#define KP_NOTESINGLE	9834
#define KP_NOTEDOUBLE	9835


#define KP_PAD_RENDER				0x01
#define KP_PAD_INPUT				0x02

#define KP_MAXLISTENERS				16			// max listeners at any given moment per input
#define KP_MAXPADS					16
#define KP_MAXFKEYS					16
#define KP_EDITBOXSIZE				2047
#define KP_SLIDE_BACKSPACE			900.0			// time to hold (& slide if using mouse) backspace before it clears the buffer, in ms
#define KP_DBCLICK_BACKSPACE		155.0



/*################################################################################################################*/
/*################################################################################################################*/
/*################################################################################################################*/


typedef struct{
	point_t position;
	int width;
	int height;
	char *font;
	UTF32 caret;
}TCFGEDITBOX;

typedef struct{
	point_t position;
	int padFirst;
	TCFGEDITBOX editbox;
	
	str_list keyList;
	str_list padList;
}TCFGKEYPAD;

typedef struct{
	int *val;
	int valTotal;
}TKPCFG_ALIST;

typedef struct{
	int type;
	UTF32 code;
	TKPCFG_ALIST list;
	wchar_t *image;
}TKPCFG_KEYCODE;

typedef struct{
	TKPCFG_KEYCODE **keys;
	int total;
}TKPCFG_KEYLIST;

typedef struct{
	UTF32 code;
	point_t pos;
}TKPCFG_KEYPOS;

typedef struct{
	TKPCFG_KEYPOS **list;
	int total;
}TKPCFG_PADS;

typedef struct{
	TKPCFG_PADS **pads;
	int total;
}TKPCFG_PADLIST;

typedef struct{
	TKPCFG_KEYLIST keyList;
	TKPCFG_PADLIST padList;
	TCFGKEYPAD keypad;
	
	TCFGENTRY  **config;
}TVKSETTINGS;





/*################################################################################################################*/
/*################################################################################################################*/
/*################################################################################################################*/


typedef struct TKP_PAD TKP_PAD;

typedef struct {
	int type;			// key type - see above
	point_t pos;			// location relative to pad
	TCCBUTTON2 *btn;	// pixmap container. location is relative to frame (.pos + control)
	UTF32 code;			// unicode code point
	int id;
	uint64_t time;		// time when last pressed
}TKP_KEY;

struct TKP_PAD {
	point_t pos;			// location relative to control
	int flags;
	TLISTITEM *keys;
	int tKeys;
	int id;				// ident of this pad
};

typedef struct {
	wchar_t buffer[KP_EDITBOXSIZE+1];		// the editing string
	wchar_t caret[KP_EDITBOXSIZE+1];		// above buffer but with caret char added at render stage
	wchar_t history[KP_EDITBOXSIZE+1];		// undo buffer
	int tChars;		// total chars in buffer
	int caretPos;	// edit position
	UTF32 caretChr;
	int64_t udata64;
	
	TLABEL *label;		// string is rendered via this
	int labelId;		// itemId of inserted string
}TKP_EDITBOX;

typedef struct {
	int id;			// common control object id
	int flags;		// listen mode/type
	
	int64_t data64;	// opaque user data sent with each message callback
}TKP_LISTENER;


typedef struct {
	TKP_LISTENER *obj[KP_MAXLISTENERS];		// should make this a list
	int total;
}TKP_LISTENERS;

struct TKEYPAD {
	COMMONCTRLOBJECT;
	int isBuilt;

	TKP_PAD *pads[KP_MAXPADS];			// list of pads
	int tPads;
	int activePadId;
	
	uint64_t lastKeyPressTime0;	
	double lastKeyReleaseTime;

	TKP_LISTENERS listener;		// may implement this as a filter instead..
	TKP_EDITBOX editbox;
	TVKSETTINGS cfg;			// keypad configuration as imported [from file]
};


int keypadNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t tv_cb, int *id, const int unused1, const int unused2);




int keypadListenerAdd (TKEYPAD *kp, const int ccId, const int mode, const int64_t udata);
void keypadListenerRemove (TKEYPAD *kp, const int ccId);
void keypadListenerRemoveAll (TKEYPAD *kp);
int keypadListenerIsListening (TKEYPAD *kp, const int ccId);
int keypadListenerGetTotal (TKEYPAD *kp);


char *keypadEditboxGetBuffer8 (TKP_EDITBOX *eb);		// free with my_free()
wchar_t *keypadEditboxGetBufferW (TKP_EDITBOX *eb);		// free with my_free()
int keypadEditboxSetBufferW (TKP_EDITBOX *eb, wchar_t *strw);
int keypadEditboxSetUndoBufferW (TKP_EDITBOX *eb, wchar_t *strw);
int keypadEditboxSetBuffer8 (TKP_EDITBOX *eb, char *str8);
int keypadEditboxSetUndoBuffer8 (TKP_EDITBOX *eb, char *str8);

int keypadEditboxUndoBuffer (TKP_EDITBOX *eb);			// undo the buffer
void keypadEditboxSetUserData (TKP_EDITBOX *eb, const int64_t udata);
int64_t keypadEditboxGetUserData (TKP_EDITBOX *eb);
int keypadEditboxGetCharTotal (TKP_EDITBOX *eb);
//int keypadEditboxAddChar (TKP_EDITBOX *eb, const wchar_t code);

int keypadClipBoardSet (TKEYPAD *kp, HWND hwnd, const wchar_t *buffer);
int keypadClipBoardGet (TKEYPAD *kp, HWND hwnd, TKP_EDITBOX *eb);
int keypadClipBoardSet8 (TKEYPAD *kp, HWND hwnd, const char *buffer);

void keypadPlayKeyAlert (TKEYPAD *kp);

int keypadEditboxCaretMoveLeft (TKP_EDITBOX *eb);
int keypadEditboxCaretMoveRight (TKP_EDITBOX *eb);
int keypadEditboxCaretMoveEnd (TKP_EDITBOX *eb);
int keypadEditboxCaretMoveStart (TKP_EDITBOX *eb);

int buildKeypad (TKEYPAD *kp);

int keypadInputKeyPressCb (TKEYPAD *kp, const int key, const int64_t data);

int keypadDispatchEvent (TKEYPAD *kp, const int flags, const int msg, const uint64_t data1, void *ptr);

#endif


