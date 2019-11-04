
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


#ifndef _EDITBOX_H_
#define _EDITBOX_H_

#define EDITBOXIN_WORKINGBUFFERS	8
#define EDITBOXIN_INPUTBUFFERLEN	2047
#define EDITBOXCMD_MAXCMDLEN		31
#define EDITBOXCMD_MAXCMDS			127
#define EDITBOXCMD_MAXDESCLEN		383

#define CMDPARSER_CMDIDENT			L'#'




typedef struct{
	int state;
	
	void (*pfunc) (wchar_t *var, int vlen, void *uptr, int data1, int data2);
	wchar_t name[EDITBOXCMD_MAXCMDLEN+1];	/* command name */
	wchar_t alias[EDITBOXCMD_MAXCMDLEN+1];	/* an alias */
	wchar_t desc[EDITBOXCMD_MAXDESCLEN+1];

	void *uptr;	// user variables
	int data1;
}TEDITBOXCMD;

typedef struct{
	int historyBufferi;
	int caretChar;
	int caretPos;
	int tKeys;
	size_t iOffset;

	int registeredCmdTotal;
	TEDITBOXCMD	registeredCmds[EDITBOXCMD_MAXCMDS];
	wchar_t buffers[EDITBOXIN_WORKINGBUFFERS][EDITBOXIN_INPUTBUFFERLEN+1];
	wchar_t caretBuffer[EDITBOXIN_INPUTBUFFERLEN+1];
	wchar_t workingBuffer[EDITBOXIN_INPUTBUFFERLEN+1];
}TEDITBOX;

wchar_t *editboxGetString (TEDITBOX *input);
int editboxSetString (TEDITBOX *input, const wchar_t *string);
int editboxAppendString (TEDITBOX *input, const wchar_t *string);
int editboxSetString8 (TEDITBOX *input, const char *string8);
int editboxAppendString8 (TEDITBOX *input, const char *string8);

int addCaret (TEDITBOX *input, wchar_t *src, wchar_t *des, size_t desSize);
int drawEditBox (TEDITBOX *input, TFRAME *frame, int x, int y, int width, wchar_t *ptext, size_t *offset);
void drawCharTotal (TEDITBOX *input, TFRAME *frame, const int x, const int y);

int editBoxInputProc (TEDITBOX *input, HWND hwnd, int key);



void clearWorkingBuffer (TEDITBOX *input);
void addWorkingBuffer (TEDITBOX *input);

int previousHistoryBuffer (TEDITBOX *input);
int nextHistoryBuffer (TEDITBOX *input);

int exitboxProcessString (TEDITBOX *input, wchar_t *txt, int ilen, void *ptr);
void editboxDoCmdRegistration (TEDITBOX *input, void *vp);

int clipboardSend (void *ctx, const char *str8);

#endif

