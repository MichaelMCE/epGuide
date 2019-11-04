// playlist media control overlay
// (next trk, play, art size, etc..)
//
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


#include "../common.h"



wchar_t *editboxGetString (TEDITBOX *input)
{
	if (*input->workingBuffer)
		return my_wcsdup(input->workingBuffer);
	return NULL;
}

static inline void toggleCaret (TEDITBOX *input)
{
	if (input->caretChar == EDITBOX_CARET1)	// custom chars added to 5x7.bdf
		input->caretChar = EDITBOX_CARET2;
	else
		input->caretChar = EDITBOX_CARET1;
}

int getCaretCharLeft (TEDITBOX *input, int n)
{
	return input->workingBuffer[input->caretPos-n]&0xFF;
}

int getCaretCharRight (TEDITBOX *input, int n)
{
	--n;
	return input->workingBuffer[input->caretPos+n]&0xFF;
}

int addCaret (TEDITBOX *input, wchar_t *src, wchar_t *des, size_t desSize)
{
	toggleCaret(input);
	size_t srcLen = wcslen(src);
	
	if (!srcLen){
		des[0] = input->caretChar;
		des[1] = 0;
	}else if (input->caretPos >= input->tKeys){
		wcsncpy(des, src, srcLen);
		des[srcLen] = input->caretChar;
		des[srcLen+1] = 0;
	}else{
		memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
		wcsncpy(des, src, input->caretPos);
		des[input->caretPos] = input->caretChar;
		wcsncpy(&des[input->caretPos+1], src+input->caretPos, wcslen(src+input->caretPos));
	}
	return 1;
}

int drawEditBox (TEDITBOX *input, TFRAME *frame, int x, int y, int width, wchar_t *ptext, size_t *offset)
{
	
	//lSetForegroundColour(frame->hw, 0xFFFFFFFF);
	
	width += x;
	TLPRINTR rect2 = {x+2,0,width,frame->height-1,x,0,x,frame->height-1};
	const int renderFlags = PF_DONTFORMATBUFFER|PF_WORDWRAP|PF_CLIPWRAP|PF_CLIPTEXTV;
	wchar_t *text = ptext + *offset;

	lPrintEx(frame, &rect2, EDITBOX_FONT, renderFlags|PF_DONTRENDER, LPRT_CPY, (char*)text);

	//if (!*input->workingBuffer) rect2.ey += 1;
	y -= rect2.ey;
	TLPRINTR rect = {x+2,y+2,width,frame->height-1,x,y,x,y+7};
	lDrawRectangleFilled(frame, x, y, width-1, y+rect2.ey+2, 180<<24);
	lPrintEx(frame, &rect, EDITBOX_FONT, renderFlags, LPRT_CPY, (char*)text);
	return y;
}

static inline int addKey (TEDITBOX *input, int key, int position)
{
	if (input->tKeys < EDITBOXIN_INPUTBUFFERLEN-1){
		if (position == input->tKeys){
			input->caretPos++;
			input->workingBuffer[input->tKeys++] = key;
			return 1;
		}else{
			wchar_t *src = input->workingBuffer;
			wchar_t *des = input->caretBuffer;
			
			memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
			wcsncpy(des, src, position);
			des[position] = key;
			wcsncpy(&des[position+1], src+position, wcslen(src+position));
			wcsncpy(src, des, wcslen(des));
			
			input->caretPos++;
			input->tKeys++;
			return 1;
		}
	}
	return 0;

}

int editboxAppendString (TEDITBOX *input, const wchar_t *string)
{
	const int len = wcslen(string);
	for (int i = 0; i < len; i++)
		addKey(input, string[i], input->caretPos);
	return 1;
}

int editboxAppendString8 (TEDITBOX *input, const char *string8)
{
	wchar_t *string = com_converttow(string8);
	if (string){
		editboxAppendString(input, string);
		my_free(string);
		return 1;
	}
	return 0;
}

int editboxSetString8 (TEDITBOX *input, const char *string8)
{
	addWorkingBuffer(input);
	nextHistoryBuffer(input);
	clearWorkingBuffer(input);
	return editboxAppendString8(input, string8);
}

int editboxSetString (TEDITBOX *input, const wchar_t *string)
{
	addWorkingBuffer(input);
	nextHistoryBuffer(input);
	clearWorkingBuffer(input);
	return editboxAppendString(input, string);
}

static inline void addKeyTab (TEDITBOX *input)
{
	addKey(input, L'\t', input->caretPos);
}

static inline int deleteKey (TEDITBOX *input, int position)
{
	wchar_t *src = input->workingBuffer;
	wchar_t *des = input->caretBuffer;
	
	memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	wcsncpy(des, src, input->caretPos-1);
	wcsncpy(&des[input->caretPos-1], src+input->caretPos, wcslen(src+input->caretPos));
	memset(src, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	wcsncpy(src, des, wcslen(des));
	
	if (input->iOffset) input->iOffset--;
	if (input->caretPos) input->caretPos--;
	if (input->tKeys) input->tKeys--;
	return 1;
}

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

int clipboardSend (void *ctx, const char *str8)
{
	
	int ret = 0;
	wchar_t *strw = com_converttow(str8);
	if (strw){
		TCC *cc = (TCC*)cc;
		ret = copyClipBoardTextW(cc->hMsgWin, strw);
		my_free(strw);
	}
	return ret;
}

// paste from clipboard
void pasteClipBoardTextW (HWND hwnd, TEDITBOX *input)
{
	if (OpenClipboard(hwnd)){
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle){
			wchar_t *buffer = (wchar_t*)GlobalLock(handle);
			if (buffer){
				while(*buffer){
					if (*buffer == VK_TAB)
						addKeyTab(input);
					else if (*buffer < 32)
						addKey(input, L' ', input->caretPos);
					else
						addKey(input, *buffer, input->caretPos);
					
					buffer++;
				}
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
}

void clearWorkingBuffer (TEDITBOX *input)
{
	memset(input->workingBuffer, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	input->iOffset = input->caretPos = input->tKeys = 0;
}

int nextHistoryBuffer (TEDITBOX *input)
{
	if (++input->historyBufferi >= EDITBOXIN_WORKINGBUFFERS)
		input->historyBufferi = 0;
	return input->historyBufferi;
}

int previousHistoryBuffer (TEDITBOX *input)
{
	if (--input->historyBufferi < 0)
		input->historyBufferi = EDITBOXIN_WORKINGBUFFERS-1;
	return input->historyBufferi;
}

void addWorkingBuffer (TEDITBOX *input)
{
	my_memcpy(input->buffers[input->historyBufferi], input->workingBuffer, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	//wcsncpy(input->buffers[input->historyBufferi], input->workingBuffer, EDITBOXIN_INPUTBUFFERLEN-1);
}

static inline void addHistoryBuffer (TEDITBOX *input)
{
	my_memcpy(input->workingBuffer, input->buffers[input->historyBufferi], EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	input->caretPos = input->tKeys = wcslen(input->workingBuffer);
	input->iOffset = 0;
}

int editBoxInputProc (TEDITBOX *input, HWND hwnd, int key)
{
	if (key == VK_LSHIFT || key == VK_SHIFT || key == VK_RSHIFT)
		return 0;
			
	//printf("%i\n",key);

	if (key&0x1000){
		key &= ~0x1000;
			
		if (key >= VK_PRIOR && key <= VK_DELETE){
			//printf("%i\n",key);
			if (key == VK_LEFT){
				if (--input->caretPos < 1){
					input->caretPos = 0;
					input->iOffset = 0;
				}
			}else if (key == VK_RIGHT){
				if (++input->caretPos > input->tKeys)
					input->caretPos = input->tKeys;

			}else if (key == VK_DELETE){
				if (++input->caretPos > input->tKeys)
					input->caretPos = input->tKeys;
				else
					deleteKey(input, input->caretPos);
			}else if (key == VK_UP){
				previousHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_DOWN){
				nextHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_HOME){
				input->caretPos = 0;
				input->iOffset = 0;
					
			}else if (key == VK_END){
				input->caretPos = input->tKeys;
				input->iOffset = 0;
					
			}else{
				return 0;
			}
			return 1;
		}
		return 0;
	
	}else if (key == 3){			// Control + C
 		copyClipBoardTextW(hwnd, input->workingBuffer);
 		
	}else if (key == 22){			// Control + V
		pasteClipBoardTextW(hwnd, input);
		
	}else if (key == 6){			// Control + F
		addWorkingBuffer(input);
		nextHistoryBuffer(input);
		clearWorkingBuffer(input);
		addKey(input, CMDPARSER_CMDIDENT, input->caretPos);
		
		const wchar_t *find = L"find ";
		int len = wcslen(find);
		for (int i = 0; i < len; i++)
			addKey(input, find[i], input->caretPos);
			
		return 0;
		
	}else if (key == VK_RETURN){
		//kHookOff();
		//kHookUninstall();
		return 2;
			
	}else if (key == VK_ESCAPE){
		//kHookOff();
		//kHookUninstall();
		return 3;
			
	}else if (key == VK_TAB){
		addKeyTab(input);
			
	}else if (key == VK_BACK){
	 	if (input->tKeys && input->caretPos)
	 		deleteKey(input, input->caretPos);

#if 0
	}else if (key == G15_SOFTKEY_1 || key == G15_SOFTKEY_2 || key == G15_SOFTKEY_3 || key == G15_SOFTKEY_4){
		kbHookOff();
		return -1;
#endif

	}else if (key < 31){
		// do nothing

	}else{
		addKey(input, key, input->caretPos);
	}

	return 1;
}
