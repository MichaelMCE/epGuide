
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



static inline int64_t keybtn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr);



static inline int keypadGeneratePadId (TKEYPAD *kp)
{
	return ccGenerateId(kp);
}

static inline TKP_PAD *keypadCreatePad (TKEYPAD *kp, const point_t *pos)
{
	TKP_PAD *pad = my_calloc(1, sizeof(TKP_PAD));
	if (pad){
		pad->id = keypadGeneratePadId(kp);
		pad->pos.x = pos->x;
		pad->pos.y = pos->y;
		pad->tKeys = 0;
		pad->keys = NULL;
		pad->flags = KP_PAD_RENDER | KP_PAD_INPUT;
	}
	return pad;
}

static inline TKP_PAD *keypadGetPad (TKEYPAD *kp, const int padId)
{
	if (!kp->tPads) return NULL;

	for (int i = 0; i < KP_MAXPADS; i++){
		if (kp->pads[i]){
			if (kp->pads[i]->id == padId)
				return kp->pads[i];
		}
	}
	return NULL;
}

static inline int keypadGetPadIdByIdx (TKEYPAD *kp, const int idx)
{
	if (idx >= 0 && idx < kp->tPads)
		return kp->pads[idx]->id;
	else
		return 0;
}

static inline int keypadAddPad (TKEYPAD *kp, TKP_PAD *pad)
{
	for (int i = 0; i < KP_MAXPADS; i++){
		if (kp->pads[i] == NULL){
			kp->pads[i] = pad;
			return ++kp->tPads;
		}
	}
	return -1;
}

static inline TKP_KEY *keypadCreateKey (TKEYPAD *kp, TKP_PAD *pad, const int id)
{
	TKP_KEY	*key = my_calloc(1, sizeof(TKP_KEY));
	if (key){
		key->id = id;
		
		TLISTITEM *kItem = listNewItem(key);
		//if (pad->tKeys)
			listAdd(pad->keys, kItem);

		//pad->keys = kItem;
		pad->tKeys++;
	}
	return key;
}

static inline int keypadAddButton (TKEYPAD *kp, const int padId, const int keyType, const UTF32 codePoint, const point_t *pos, TCCBUTTON2 *btn)
{
	TKP_PAD *pad = keypadGetPad(kp, padId);
	TKP_KEY	*key = keypadCreateKey(kp, pad, btn->id);

	key->pos.x = pos->x;
	key->pos.y = pos->y;
	key->code = codePoint;
	key->btn = btn;
	key->type = keyType;

	ccSetUserDataInt(btn, pad->id);
	ccSetUserData(btn, kp);
	ccEnable(btn);

	return key->id;
}

static inline int keypadAddKey (TKEYPAD *kp, const int padId, const int keyType, const point_t *posOffset, const int canAnimate, UTF32 codePoint, wchar_t *imageFacePath)
{

	//wprintf(L"%i,<%i,%i>\t '%s'\n", codePoint, posOffset->x, posOffset->y, imageFacePath);
	//printf("<%i,%i,%i>\n", codePoint, posOffset->x, posOffset->y);

	TKP_PAD *pad = keypadGetPad(kp, padId);
	if (!pad){
		//printf("no pads or pad %i not found\n", padId);
		return 0;
	}

	int id = 0;
	int x = kp->metrics.x + pad->pos.x + posOffset->x;
	int y = kp->metrics.y + pad->pos.y + posOffset->y;

	TCCBUTTON2 *btn = ccCreate(kp->cc, kp->pageOwner, CC_BUTTON2, keybtn_cb, &id, x, y);
	if (btn){
		if (keyType == KP_KEYS_BACKSPACE){
			btn->canDrag = 1;
			btn->flags.acceptDrag = 1;
		}
		
		wchar_t buffer[MAX_PATH+1];
		const int imgId = artManagerImageAdd(ccGetImageManager(kp->cc,CC_IMAGEMANAGER_IMAGE), com_buildSkinD(kp->cc, buffer, imageFacePath));
		button2FaceImgcSet(btn, imgId, 0, 0.0, 0, 0);
		button2FaceActiveSet(btn, BUTTON_PRI);
		button2AnimateSet(btn, canAnimate);
		button2FaceHoverSet(btn, 1, COL_HOVER, 0.8);
		keypadAddButton(kp, padId, keyType, codePoint, posOffset, btn);
	}

	return id;
}

static inline int vkbSettingsGet (TCFGENTRY **config, const char *key, void *value)
{
	return cfg_keyGet(config, key, value);
}

static inline int vkbSettingsSet (TCFGENTRY **config, const char *key, void *value)
{
	return cfg_keySet(config, key, value);
}

static inline TKPCFG_PADS *keypadCfgGetPad (TVKSETTINGS *cfg, const int padNo)
{
	TKPCFG_PADLIST *plist = &cfg->padList;
	
	if (padNo < plist->total)
		return plist->pads[padNo];
	return NULL;
}

static inline TKPCFG_KEYCODE *keypadCfgGetKey (TVKSETTINGS *cfg, const UTF32 code)
{
	TKPCFG_KEYLIST *klist = &cfg->keyList;
	
	for (int i = 0; i < klist->total; i++){
		if (klist->keys[i]->code == code)
			return klist->keys[i];
	}
	return NULL;
}

int keypadCfgBuildPad (TKEYPAD *kp, TVKSETTINGS *cfg, const int padNo)
{
	int tKeys = 0;

	TKPCFG_PADS *padKeys = keypadCfgGetPad(cfg, padNo);
	//printf("keypadCfgBuildPad %i\n", padKeys->total);
	
	wchar_t image[MAX_PATH+1];
	point_t pos = {0,0};
	
	TKP_PAD *pad = keypadCreatePad(kp, &pos);
	keypadAddPad(kp, pad);
	
	for (int i = 0; i < padKeys->total; i++){
		TKPCFG_KEYPOS *key = padKeys->list[i];
		if (!key){
			//printf("key %i missing in pad %i\n", i, padNo);
			continue;
		}
		
		TKPCFG_KEYCODE *uc = keypadCfgGetKey(cfg, key->code);
		if (!uc){
			//printf("uc missing for %i %i\n", i, key->code);
			continue;
		}
		
		_snwprintf(image, MAX_PATH, L"vkeyboard/%s", uc->image);

		int id = keypadAddKey(kp, pad->id, uc->type, &key->pos, uc->type != KP_KEYS_ENTER, uc->code, image);
		if (id) tKeys++;

		//if (uc->type != 1)
		//	printf("%i: %i, %i %i %i %i\n", i, id, uc->type, uc->code, key->pos.x, key->pos.y);
	}

	return tKeys;
}

static inline int strct (char *str, const char chr)
{
	int ct = 0;
	
	while (*str)
		ct += (*(str)++ == chr);
		
	return ct;
}

static inline int strct2 (char *str, const char chr1, const char chr2)
{
	int ct = 0;
	
	while (*str){
		if (str[0] == chr1 && str[1] == chr2){
			ct++;
			str++;
		}
		str++;
	}
		
	return ct;
}

static inline int getPadStrListTotal (const char *str)
{
	int total = 0;
	char *dup = my_strdup(str);
	
	int tOpen = strct(dup, '<');
	int tClose = strct(dup, '>');
	total = strct2(dup, '>', '<')+1;
	
	if (tOpen != tClose || tOpen != total){
		my_free(dup);
		return 0;
	}
		
	//printf("getPadStrListTotal %i %i %i, %i\n", tOpen, tClose, total, strlen(dup));
	
	my_free(dup);
	return total;
}

static inline int getKeyStrType (const char *str)
{
	int type = str[0];
	if (type >= '1' && type <= '9')		// there are only 7 types and no type 0
		return type - '0';
	return 0;
}

static inline int getKeyStrListTotal (const char *str)
{
	int total = 0;
	char *dup = my_strdup(str);
	
	char *start = strchr(dup, '<');
	char *end = strchr(dup, '>');
	if (start && end){
		*end = 0;

		if (isdigit(*(++start)))
			total++;
			
		while (*(start++)){
			if (*start == ','){
				if (*(start+1) == '-') start++;
				if (isdigit(*(start+1)))
					total++;
			}
		}
	}
	
	//printf("%p %p\n", start, end);
	
	my_free(dup);
	return total;
}

static inline TKPCFG_KEYCODE *vkbParseStringKey (const char *str)
{
	
	TKPCFG_KEYCODE	*uc = my_calloc(1, sizeof(TKPCFG_KEYCODE));
	if (!uc) return NULL;	
	
	char image[MAX_PATH_UTF8+1];
	
	//int ret = 0;
	const int type = getKeyStrType(str);
	
	if (type == KP_KEYS_CHAR){
		if (strct((char*)str, ',') != 2) goto end;
		
		/*int ret = */sscanf(str, "%i,%i,%s", &uc->type, &uc->code, image);
		//printf("type 2: %i %i '%s', %i ##%s##\n", uc->type, uc->code, image, ret, str);
		uc->image = com_converttow(image);
		return uc;
		
	}else if (type == KP_KEYS_SWITCH){
		if (strct((char*)str, ',') != 2) goto end;
		
		uc->list.valTotal = 2;	// index 1 is used to store current padId when switching to dest
		uc->list.val = my_calloc(uc->list.valTotal, sizeof(int));
		if (!uc->list.val) goto end;
		
		/*int ret =*/ sscanf(str, "%i,%i,%s", &uc->type, &uc->list.val[0], image);
		//printf("type 2: %i %i '%s', %i ##%s##\n", uc->type, uc->code, image, ret, str);
		uc->image = com_converttow(image);
		uc->code = uc->type;
		return uc;
		
	}else if (type == KP_KEYS_TOGGLE){
		if (strct((char*)str, ',') != 3) goto end;
		
		uc->list.valTotal = 2;
		uc->list.val = my_calloc(uc->list.valTotal, sizeof(int));
		if (!uc->list.val) goto end;
		
		/*ret =*/ sscanf(str, "%i,<%i,%i>,%s", &uc->type, &uc->list.val[0], &uc->list.val[1], image);
		uc->image = com_converttow(image);
		uc->code = uc->type;
		//printf("@ %i %i %i '%s', %i ##%s##\n", uc->type, uc->list.val[0], uc->list.val[1], image, ret, str);
		return uc;
		
	}else if (type == KP_KEYS_CYCLE){
		const int total = getKeyStrListTotal(str);
		if (total){
			//printf("type 4 total %i\n", total);
		
			uc->list.valTotal = total;
			uc->list.val = my_calloc(uc->list.valTotal, sizeof(int));
			if (!uc->list.val) goto end;
		
			sscanf(str, "%i", &uc->type);
			char *start = strchr(str, '<');
			start++;

			for (int i = 0; i < total; i++){
				sscanf(start, "%i", &uc->list.val[i]);
				start = strchr(start, ',');
				start++;
			}
			sscanf(start, "%s", image);
			uc->image = com_converttow(image);
			uc->code = uc->type;
			//printf("@ %i %i %i '%s', %i ##%s##\n", uc->type, uc->list.val[0], uc->list.val[1], image, total, str);
			return uc;
		}
	}else if (type == KP_KEYS_ENTER || type == KP_KEYS_RETURN || type == KP_KEYS_BACKSPACE){
		if (strct((char*)str, ',') != 1) goto end;
		
		/*ret = */sscanf(str, "%i,%s", &uc->type, image);
		uc->image = com_converttow(image);
		uc->code = uc->type;
		//printf("@ %i '%s', %i ##%s##\n", uc->type, image, ret, str);
		return uc;
	}

end:	
	my_free(uc);
	return NULL;
}

static inline TKPCFG_PADS *vkbParseStringPad (const char *str)
{
	const int total = getPadStrListTotal(str);
	if (!total) return NULL;

	
	TKPCFG_PADS *pad = my_calloc(1, sizeof(TKPCFG_PADS));
	if (!pad) return NULL;

	pad->total = total;
	pad->list = my_calloc(pad->total, sizeof(TKPCFG_KEYPOS*));

	
	char *dup = my_strdup(str);
	char *start = dup;
	char *end = strchr(start, '>');
	
	for (int i = 0; i < total; i++){
		if (!start) break;

		int ltotal = getKeyStrListTotal(start);
		//printf("## ltotal: %i %i\n", i, ltotal);
		if (ltotal != 3) break;

		pad->list[i] = my_calloc(total, sizeof(TKPCFG_KEYPOS));
		if (pad->list[i]){
			start = strtok(++start, ",");
			pad->list[i]->code = atol(start);
			
			start = strtok(NULL, ",");
			pad->list[i]->pos.x = atol(start);
			
			start = strtok(NULL, ">");
			pad->list[i]->pos.y = atol(start);
		
			start = ++end;
			end = strchr(start, '>');

			//printf("%i %i %i %i\n", i, pad->list[i]->code, pad->list[i]->pos.x, pad->list[i]->pos.y);
		}else{
			break;
		}
	}
	
	my_free(dup);

	return pad;
}

static inline void vkey_commentsSetDefault (TCFGENTRY **config)
{
	cfg_commentSet(config, "keypad.firstPad", "Set lowercase as the primary pad");
	cfg_commentSet(config, "keypad.editbox.caret", "Default is 8629 (left downwards corner arrow)");
}

static inline TCFGENTRY **vkey_configCreate (TVKSETTINGS *cfg)
{	
	const TCFGENTRY config_cfg[] = {
    	{" ", V_BRK(0), NULL},

    	{"keypad.position.x",			V_INT32(4),		&cfg->keypad.position.x},
    	{"keypad.position.y",			V_INT32(80),	&cfg->keypad.position.y},
    	{"keypad.firstPad",				V_INT32(1),		&cfg->keypad.padFirst},
    	{"keypad.editbox.position.x",	V_INT32(4),		&cfg->keypad.editbox.position.x},
    	{"keypad.editbox.position.y",	V_INT32(16),	&cfg->keypad.editbox.position.y},
    	{"keypad.editbox.width",		V_INT32(704),	&cfg->keypad.editbox.width},
    	{"keypad.editbox.height",		V_INT32(43),	&cfg->keypad.editbox.height},
    	{"keypad.editbox.font",			V_INT32(4),		&cfg->keypad.editbox.font},
    	{"keypad.editbox.caret",		V_INT32(8629),	&cfg->keypad.editbox.caret},

    	{" ", V_BRK(0), NULL},

    	{"keypad.key.", V_SLIST1("keytype,unicodevalue1,image.png"), (void*)&cfg->keypad.keyList},

		{" ", V_BRK(0), NULL},

    	{"keypad.pad.", V_SLIST1("<unicodevalue1,posx,posy><unicodevalue2,posx,posy><unicodevalue3,posx,posy>"), (void*)&cfg->keypad.padList},

    	{NULL, V_INT32(0), NULL, 0, NULL}
	};
	
	TCFGENTRY **config = cfg_configDup(config_cfg);
	vkey_commentsSetDefault(config);
	return config;
}

static inline void keypadListenerDelete (TKP_LISTENER *listener)
{
	my_free(listener);
}

static inline TKP_LISTENER *keypadListenerNew (TKP_LISTENERS *listeners)
{
	TKP_LISTENER *listener = my_calloc(1, sizeof(TKP_LISTENER));
	if (!listener) return 0;
	
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		if (listeners->obj[i] == NULL){
			listeners->obj[i] = listener;
			return listener;
		}
	}
	
	//printf("keypadListenerNew: listen space exhusted\n");
	
	my_free(listener);
	return NULL;
}

void keypadListenerRemoveAll (TKEYPAD *kp)
{
	TKP_LISTENERS *listeners = &kp->listener;
	
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		if (listeners->obj[i]){
			keypadListenerDelete(listeners->obj[i]);
			listeners->obj[i] = NULL;
		}
	}
}

// remove this CC object from list of listeners
void keypadListenerRemove (TKEYPAD *kp, const int ccId)
{
	TKP_LISTENERS *listeners = &kp->listener;
	
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		if (listeners->obj[i]){
			if (listeners->obj[i]->id == ccId){
				keypadListenerDelete(listeners->obj[i]);
				listeners->obj[i] = NULL;
			}
		}
	}
}

// return total listeners
int keypadListenerGetTotal (TKEYPAD *kp)
{
	TKP_LISTENERS *listeners = &kp->listener;
	
	int ct = 0;
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		TKP_LISTENER *obj = listeners->obj[i];
		if (obj && obj->id)
			ct++;
	}
	return ct;
}

// check if this cc object is listening for input
int keypadListenerIsListening (TKEYPAD *kp, const int ccId)
{
	TKP_LISTENERS *listeners = &kp->listener;
	
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		TKP_LISTENER *obj = listeners->obj[i];
		if (obj && obj->id == ccId)
			return (obj->flags > 0);
	}
	return 0;
}

// add this CC object to list of listeners
int keypadListenerAdd (TKEYPAD *kp, const int ccId, const int mode, const int64_t udata)
{
	TKP_LISTENERS *listeners = &kp->listener;
	
	if (!keypadListenerIsListening(kp, ccId)){
		TKP_LISTENER *obj = keypadListenerNew(listeners);
		if (!obj) return 0;
	
		obj->id = ccId;
		obj->flags = mode;
		obj->data64 = udata;
	
		return 1;
	}
	return 0;
}

char *keypadEditboxGetBuffer8 (TKP_EDITBOX *eb)
{
	return com_convertto8(eb->buffer);
}

wchar_t *keypadEditboxGetBufferW (TKP_EDITBOX *eb)
{
	return my_wcsdup(eb->buffer);
}

// send a general event to listeners
static inline int keypadInputDispatchEvent (TKEYPAD *kp, TKP_LISTENERS *listeners, const int flags, const int msg, const uint64_t data1, const uint64_t unused, void *ptr)
{
	int ct = 0;
	
	for (int i = 0; i < KP_MAXLISTENERS; i++){
		TKP_LISTENER *obj = listeners->obj[i];
		if (obj){
			if (obj->flags & flags){
				TCCOBJECT *ccObj = ccGetObject((TCCOBJECT*)kp, obj->id);
				if (ccObj){
					ccSendMessage(ccObj, msg, data1, obj->data64, ptr);
					ct++;
				}
			}
		}
	}
	return ct;
}

int keypadDispatchEvent (TKEYPAD *kp, const int flags, const int msg, const uint64_t data1, void *ptr)
{
	return keypadInputDispatchEvent(kp, &kp->listener, flags, msg, data1, 0, ptr);
}

// send buffer contents to listeners
static inline int keypadInputDispatchString (TKEYPAD *kp, TKP_LISTENERS *listeners)
{

	char *str8 = keypadEditboxGetBuffer8(&kp->editbox);
	wchar_t *strW = keypadEditboxGetBufferW(&kp->editbox);
	if (!str8 || !strW) return 0;


	int ct = keypadInputDispatchEvent(kp, listeners, KP_INPUT_COMPLETE8, KP_MSG_PAD_ENTER, KP_INPUT_COMPLETE8, 0, str8);
	ct += keypadInputDispatchEvent(kp, listeners, KP_INPUT_COMPLETEW, KP_MSG_PAD_ENTER, KP_INPUT_COMPLETEW, 0, strW);
	
	my_free(str8);
	my_free(strW);
	
	return ct;
}

// send char to anyone listening
static inline int keypadInputDispatchChar (TKEYPAD *kp, TKP_LISTENERS *listeners, const UTF32 code)
{
	return keypadInputDispatchEvent(kp, listeners, KP_INPUT_SINGLE, KP_MSG_PAD_PRESS, code, 0, NULL);
}

// delete char left of caret
static inline int keypadEditboxDeleteCharLeft (TKP_EDITBOX *eb)
{
	if (eb->caretPos < 1)
		return 0;
	
	for (int i = eb->caretPos-1; eb->buffer[i]; i++)
		eb->buffer[i] = eb->buffer[i+1];

	eb->buffer[eb->tChars] = 0;

	if (--eb->tChars < 0) eb->tChars = 0;
	if (--eb->caretPos < 0) eb->caretPos = 0;
	
	return eb->caretPos;
}

// delete char right of caret
static inline int keypadEditboxDeleteCharRight (TKP_EDITBOX *eb)
{

	if (eb->caretPos < 0 || eb->caretPos >= eb->tChars)
		return 0;
	
	eb->caretPos++;

	for (int i = eb->caretPos-1; eb->buffer[i]; i++)
		eb->buffer[i] = eb->buffer[i+1];

	eb->buffer[eb->tChars] = 0;

	if (--eb->tChars < 0) eb->tChars = 0;
	if (--eb->caretPos < 0) eb->caretPos = 0;
	
	return eb->caretPos;
}

// add a char at the caret position
static inline int keypadEditboxAddChar (TKP_EDITBOX *eb, const wchar_t code)
{
	if (eb->tChars >= KP_EDITBOXSIZE)
		return 0;
	
	// if caret isn't at the end of string then insert char without overwriting 
	// move whatever is to the right by one position
	if (eb->caretPos < eb->tChars){
		for (int i = eb->tChars; i >= eb->caretPos; i--)
			eb->buffer[i+1] = eb->buffer[i];
	}
	
	
	eb->buffer[eb->caretPos] = code;
	
	if (++eb->tChars >= KP_EDITBOXSIZE)
		eb->tChars = KP_EDITBOXSIZE;

	if (++eb->caretPos >= KP_EDITBOXSIZE)
		eb->caretPos = KP_EDITBOXSIZE;

	eb->buffer[eb->tChars] = 0;

	return eb->caretPos;
}

int keypadEditboxSetUndoBufferW (TKP_EDITBOX *eb, wchar_t *strw)
{
	int len = wcslen(strw);
	if (len > 0){
		wcsncpy(eb->history, strw, len);
		eb->history[len] = 0;
	}
	return len;	
}


int keypadEditboxSetUndoBuffer8 (TKP_EDITBOX *eb, char *str8)
{
	wchar_t *strw = com_converttow(str8);
	if (strw){
		int len = keypadEditboxSetUndoBufferW(eb, strw);
		my_free(strw);
		return len;
	}
	return 0;
}

void keypadEditboxSetUserData (TKP_EDITBOX *eb, const int64_t udata)
{
	eb->udata64 = udata;
}

int64_t keypadEditboxGetUserData (TKP_EDITBOX *eb)
{
	return eb->udata64;
}

static inline void keypadEditboxUpdateLabel (TKP_EDITBOX *eb, wchar_t *str)
{
	char *str8 = com_convertto8(str);
	if (str8){
		labelStringSet(eb->label, eb->labelId, str8);
		my_free(str8);		
	}
}

static inline int keypadEditboxAddCaret (TKP_EDITBOX *eb, wchar_t *src, wchar_t *des, const UTF32 charetChar)
{
	//toggleCaret(eb);
	
	size_t srcLen = wcslen(src);
	
	if (!srcLen){
		des[0] = charetChar;
		des[1] = 0;
	}else if (eb->caretPos >= eb->tChars){
		wcsncpy(des, src, srcLen);
		des[srcLen] = charetChar;
		des[srcLen+1] = 0;
	}else{
		memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
		wcsncpy(des, src, eb->caretPos);
		des[eb->caretPos] = charetChar;
		wcsncpy(&des[eb->caretPos+1], src+eb->caretPos, wcslen(src+eb->caretPos));
	}
	return 1;
}

int keypadEditboxCaretMoveStart (TKP_EDITBOX *eb)
{
	eb->caretPos = 0;
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);
	labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);

	return eb->caretPos;
}

int keypadEditboxCaretMoveEnd (TKP_EDITBOX *eb)
{
	eb->caretPos = eb->tChars;
	
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);

	TLABEL *label = eb->label;
	wchar_t *buffer = eb->caret;
	int len = wcslen(buffer);
	if (len < 2){
		labelStringSetLeftOffsetIndex(label, eb->labelId, 0);
		return 1;
	}

	int width = 0;
	labelStringGetMetrics(label, eb->labelId, NULL, NULL, &width, NULL);
	const int cspace = lGetFontCharacterSpacing(label->cc->hSurfaceLib, KEYPAD_INPUT_FONT);
	TWCHAR *glyphA = lGetGlyph(label->cc->hSurfaceLib, NULL, L'a', KEYPAD_INPUT_FONT);
	if (!glyphA) return 0;
	
	const int spaceWidth = glyphA->w - 6;
	const int boxWidth = ccGetWidth(label);
	int end = 0;

	for (int i = len-1; i >= 0; i--){
		UTF32 wc = buffer[i];
		TWCHAR *glyph = lGetGlyph(label->cc->hSurfaceLib, NULL, wc, KEYPAD_INPUT_FONT);
		if (!glyph) glyph = glyphA;
		
		if (wc == 32)
			end += (spaceWidth + cspace);
		else if (wc == 9)
			end += ((spaceWidth*4) + cspace);
		else
			end += (glyph->w + glyph->xoffset + cspace);
		
		//printf("%i '%c' %i %i\n", i, wc, width, end);
		if (end >= boxWidth){
			labelStringSetLeftOffsetIndex(label, eb->labelId, i+1);
			break;
		}
	}
		
	return eb->caretPos;
}

int keypadEditboxCaretMoveLeft (TKP_EDITBOX *eb)
{
	if (--eb->caretPos < 0) eb->caretPos = 0;
	
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);

	wchar_t *buffer = eb->caret;
	int len = wcslen(buffer);
	if (len < 2){
		labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);
		return 1;
	}

	int start = labelStringGetLeftOffsetIndex(eb->label, eb->labelId);
	if (eb->caretPos <= start+1 && start > 0)
		labelStringSetLeftOffsetIndex(eb->label, eb->labelId, start-1);

	return eb->caretPos;
}

int keypadEditboxCaretMoveRight (TKP_EDITBOX *eb)
{

	if (++eb->caretPos > eb->tChars) eb->caretPos = eb->tChars;
	
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);


	TLABEL *label = eb->label;
	wchar_t *buffer = eb->caret;
	int len = wcslen(buffer);
	if (len < 2){
		labelStringSetLeftOffsetIndex(label, eb->labelId, 0);
		return 1;
	}

	int spaceWidth;
	int width = 0;
	labelStringGetMetrics(label, eb->labelId, NULL, NULL, &width, NULL);
	int cspace = lGetFontCharacterSpacing(label->cc->hSurfaceLib, KEYPAD_INPUT_FONT);
	
	TWCHAR *glyphA = lGetGlyph(label->cc->hSurfaceLib, NULL, L'a', KEYPAD_INPUT_FONT);
	if (!glyphA) return 0;

	TWCHAR *glyphSpace = lGetGlyph(label->cc->hSurfaceLib, NULL, L' ', KEYPAD_INPUT_FONT);
	if (glyphSpace)
		spaceWidth = glyphSpace->dwidth;
	else
		spaceWidth = glyphA->w - 6;

	const int rightScrollBreak = ccGetWidth(label) * 0.910;	
	int end = 0;
	int start = labelStringGetLeftOffsetIndex(label, eb->labelId);

	for (int i = start; i < len; i++){
		UTF32 wc = buffer[i];
		TWCHAR *glyph = lGetGlyph(label->cc->hSurfaceLib, NULL, wc, KEYPAD_INPUT_FONT);
		if (!glyph) glyph = glyphA;
		
		if (wc == 32)
			//end += (spaceWidth + cspace);
			end += spaceWidth;
		else if (wc == 9)
			end += ((spaceWidth*4) + cspace);
		else if (!glyph->w)
			end += glyph->dwidth + cspace;
		else
			end += (glyph->w + glyph->xoffset + cspace);
		
		if (wc == eb->caretChr){
			if (end >= rightScrollBreak)
				labelStringSetLeftOffsetIndex(label, eb->labelId, -1);
			break;
		}
	}

	return eb->caretPos;
}

int keypadEditboxGetCharTotal (TKP_EDITBOX *eb)
{
	return eb->tChars;
}

static inline void keypadEditboxClear (TKP_EDITBOX *eb)
{
	eb->buffer[0] = 0;
	eb->buffer[1] = 0;
	eb->tChars = 0;
	eb->caretPos = 0;
	
	labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);
}

int keypadEditboxSetBufferW (TKP_EDITBOX *eb, wchar_t *strw)
{
	int len = wcslen(strw);
	wcsncpy(eb->buffer, strw, len);
	eb->buffer[len] = 0;
	eb->tChars = len;
	eb->caretPos = len;

	labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);
	keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
	keypadEditboxUpdateLabel(eb, eb->caret);
	return len;
}

int keypadEditboxSetBuffer8 (TKP_EDITBOX *eb, char *str8)
{
	wchar_t *strw = com_converttow(str8);
	if (strw){
		int len = keypadEditboxSetBufferW(eb, strw);
		my_free(strw);
		return len;
	}
	return 0;
}

// revert apply undo buffer to the front buffer
int keypadEditboxUndoBuffer (TKP_EDITBOX *eb)
{
	wchar_t *strw = eb->history;
	
	int len = wcslen(strw);
	if (len > 0){
		wcsncpy(eb->buffer, strw, len);
		eb->buffer[len] = 0;
		eb->tChars = len;
		eb->caretPos = len;
		
		labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);
		keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
		keypadEditboxUpdateLabel(eb, eb->caret);
	}

	return len;
}

// send buffer to clipboard
int keypadClipBoardSet (TKEYPAD *kp, HWND hwnd, const wchar_t *buffer)
{
	DWORD len = wcslen(buffer);
	if (!len) return 0;

	//wprintf(L"@@ keypadClipBoardSet '%s'\n", buffer);

    // Allocate string
    HGLOBAL hdst = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, (len + 10) * sizeof(wchar_t));
    if (hdst){
    	LPWSTR dst = (LPWSTR)GlobalLock(hdst);
    	my_memcpy(dst, buffer, len * sizeof(wchar_t));
    	//dst[len] = 0;
		GlobalUnlock(hdst);
		
		// Set clipboard data
		SetClipboardViewer(hwnd);
		
		if (!OpenClipboard(hwnd))
			return GetLastError();
		
    	EmptyClipboard();

    	HANDLE hData = SetClipboardData(CF_UNICODETEXT, hdst);
    	if (!hData && GetLastError() == ERROR_INVALID_HANDLE){
    		hData = SetClipboardData(CF_UNICODETEXT, hdst);
    	}
    	
    	if (!hData){
    		CloseClipboard();
    		return GetLastError();
    	}
    	
    	CloseClipboard();
    	//GlobalFree(hdst);
    	return 1;
    }
    return 0;
}

int keypadClipBoardSet8 (TKEYPAD *kp, HWND hwnd, const char *buffer)
{
	int ret = 0;
	wchar_t *str = com_converttow(buffer);
	if (str){
		keypadClipBoardSet(kp, hwnd, str);
		my_free(str);
	}
	return ret;
}


// paste from clipboard
int keypadClipBoardGet (TKEYPAD *kp, HWND hwnd, TKP_EDITBOX *eb)
{
	int ct = 0;
	
	if (OpenClipboard(hwnd)){
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle){
			wchar_t *buffer = (wchar_t*)GlobalLock(handle);
			if (buffer){
				while(*buffer){
					if (*buffer == VK_TAB)
						keypadEditboxAddChar(eb, L'\t');
					else if (*buffer < 32)
						keypadEditboxAddChar(eb, L' ');
					else
						keypadEditboxAddChar(eb, *buffer);
					
					labelStringSetLeftOffsetIndex(eb->label, eb->labelId, 0);
					keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
					keypadEditboxUpdateLabel(eb, eb->caret);
					
					buffer++;
					ct++;
				}
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
	return ct;
}

static inline void keypadEditboxRender (TKP_EDITBOX *eb, TFRAME *frame, const wchar_t *buffer, const int x, const int y)
{
	ccRender(eb->label, frame);
}

static inline void keypadDeleteKey (TKP_KEY *key)
{
	ccDelete(key->btn);
	my_free(key);
}

static inline void keypadDeletePad (TKP_PAD *pad)
{
	TLISTITEM *kItem = pad->keys;
	
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key){
			keypadDeleteKey(key);
		}
		kItem = kItem->next;
	}
	
	listDestroyAll(pad->keys);
	my_free(pad);
}

static inline TKP_KEY *keypadGetKey (TKP_PAD *pad, const int keyId)
{
	TLISTITEM *kItem = pad->keys;
	
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key->id == keyId)
			return key;
		kItem = kItem->next;
	}
	return NULL;
}

static inline TKP_KEY *keypadGetKeyByCode (TKP_PAD *pad, const int code)
{
	TLISTITEM *kItem = pad->keys;
	
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key->code == code)
			return key;
		kItem = kItem->next;
	}
	return NULL;
}

// disable pad from both render and input
static inline void keypadPadDisable (TKEYPAD *kp, const int id)
{
	TKP_PAD *pad = keypadGetPad(kp, id);
	if (!pad) return;
	
	pad->flags &= ~(KP_PAD_RENDER & KP_PAD_INPUT);
	
	TLISTITEM *kItem = pad->keys;
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key)
			ccDisable(key->btn);
		kItem = kItem->next;
	}
}

// enable both render and accept input
static inline void keypadPadEnable (TKEYPAD *kp, const int id)
{
	TKP_PAD *pad = keypadGetPad(kp, id);
	if (!pad) return;
	
	pad->flags |= KP_PAD_RENDER | KP_PAD_INPUT;
	
	TLISTITEM *kItem = pad->keys;
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key)
			ccEnable(key->btn);
		kItem = kItem->next;
	}
}


static inline TKP_KEY *keypadPadGetKey (TKP_PAD *pad, const int keyId)
{
	TLISTITEM *kItem = pad->keys;
	
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key->id == keyId)
			return key;

		kItem = kItem->next;
	}
	return NULL;
}

void keypadPlayKeyAlert (TKEYPAD *kp)
{
#if 0
	Beep(1150, 20);
#endif
}

static inline int keypadSetActive (TKEYPAD *kp, const int padId)
{
	keypadPadDisable(kp, kp->activePadId);
	keypadPadEnable(kp, padId);
	int previous = kp->activePadId;
	kp->activePadId = padId;
	return previous;
}

static inline int keypadGetActive (TKEYPAD *kp)
{
	return kp->activePadId;
}

static inline int64_t ebbtn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	if (msg != LABEL_MSG_TEXT_SELECTED_PRESS)
		return 1;
		
	TLABEL *label = (TLABEL*)object;
	TKEYPAD *kp = ccGetUserData(label);
	
	int x = (data1>>16)&0xFFFF;
	x -= (MOFFSETX+18);
	//int y = data1&0xFFFF;
		

	int lw = 0;
	labelStringGetMetrics(label, data2, NULL, NULL, &lw, NULL);
	//printf("label: %i - %i\n", x, lw);
	
	int cspace = lGetFontCharacterSpacing(label->cc->hSurfaceLib, KEYPAD_INPUT_FONT);
	//int width= 0;
	
	TKP_EDITBOX *eb = &kp->editbox;
	wchar_t *buffer = eb->caret;

	int len = wcslen(buffer);
	if (len < 2) return 1;
	int end = 0;
	int spaceWidth;
	
	TWCHAR *glyphA = lGetGlyph(label->cc->hSurfaceLib, NULL, L'a', KEYPAD_INPUT_FONT);
	if (!glyphA) return 0;
	
	TWCHAR *glyphSpace = lGetGlyph(label->cc->hSurfaceLib, NULL, L' ', KEYPAD_INPUT_FONT);
	if (glyphSpace)
		spaceWidth = glyphSpace->dwidth;
	else
		spaceWidth = glyphA->w - 6;
		
	const int start = labelStringGetLeftOffsetIndex(eb->label, eb->labelId);
	for (int i = start; i < len; i++){
		UTF32 wc = buffer[i];
		TWCHAR *glyph = lGetGlyph(label->cc->hSurfaceLib, NULL, wc, KEYPAD_INPUT_FONT);
		if (!glyph) glyph = glyphA;
		//printf("%i %i %i %i %i, %i %i\n", i, wc, end, glyph->w, glyph->w + glyph->xoffset, glyph->xoffset, cspace);
		
		if (wc == 32)
			//end += (spaceWidth + cspace);
			end += spaceWidth;
		else if (wc == 9)
			end += ((spaceWidth<<2) + cspace);
		else if (!glyph->w)
			end += glyph->dwidth + cspace;
		else
			end += (glyph->w + glyph->xoffset + cspace);
		
		if (end >= x + glyph->xoffset + cspace){
			if (i < eb->caretPos)
				eb->caretPos = i+1;
			else
				eb->caretPos = i;
			keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
			keypadEditboxUpdateLabel(eb, eb->caret);
			break;
		}
	}
	return 1;
}

static inline int keypadInputKeyPress (TKEYPAD *kp, TKP_PAD *pad, const int keyType, TKP_KEY *key)
{
	if (keyType == KP_KEYS_CHAR){
		// add key chars to buffer
		//printf("keypad pressed: %p %p %p\n", kp, pad, key);
		//printf("\t\t: kp:%i pad:%i key:%i, code:%u\n", kp->id, pad->id, key->id, key->code);
		
		TKP_EDITBOX *eb = &kp->editbox;
		keypadEditboxAddChar(eb, key->code);
		keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
		keypadEditboxUpdateLabel(eb, eb->caret);
		keypadInputDispatchChar(kp, &kp->listener, key->code);
		
	}else if (keyType == KP_KEYS_SWITCH){		// eg; digits
		//printf("key switch\n");
		TKPCFG_KEYCODE *uc = keypadCfgGetKey(&kp->cfg, key->code);
		
		int padId = keypadGetPadIdByIdx(kp, uc->list.val[0]-1);
		
		int nextId;
		if (keypadGetActive(kp) != padId){	// is not it so switch to it
			nextId = padId;
			uc->list.val[1] = keypadGetActive(kp);
		}else{							// is already on it so switch to previous pad
			nextId = uc->list.val[1];
			if (nextId == padId){
				if (uc->list.val[0]-2 > 0)
					nextId = keypadGetPadIdByIdx(kp, uc->list.val[0]-2);
				else
					nextId = keypadGetPadIdByIdx(kp, 0);
			}
			uc->list.val[1] = keypadGetActive(kp);
		}
		
		if (!nextId)
			nextId = keypadGetPadIdByIdx(kp, 0);
		
		keypadSetActive(kp, nextId);
					
	}else if (keyType == KP_KEYS_TOGGLE){		// eg; shift
		//printf("key toggle\n");
		TKPCFG_KEYCODE *uc = keypadCfgGetKey(&kp->cfg, key->code);
		
		//printf("%i %i\n", uc->list.val[0], uc->list.val[0]);
		
		int padIdA = keypadGetPadIdByIdx(kp, uc->list.val[0]-1);
		int padIdB = keypadGetPadIdByIdx(kp, uc->list.val[1]-1);
		
		int nextId;
		if (keypadGetActive(kp) == padIdA)
			nextId = padIdB;
		else if (keypadGetActive(kp) == padIdB)
			nextId = padIdA;
		else
			nextId = padIdA;

		keypadSetActive(kp, nextId);
			
	}else if (keyType == KP_KEYS_CYCLE){		// eg; cycling through pads
		//printf("key cycle\n");
		
		TKPCFG_KEYCODE *uc = keypadCfgGetKey(&kp->cfg, key->code);

		for (int i = 0; i < uc->list.valTotal; i++){
			int padId = keypadGetPadIdByIdx(kp, uc->list.val[i]-1);
			if (padId == keypadGetActive(kp)){
				int nextId;
				if (i+1 < uc->list.valTotal)
					nextId = keypadGetPadIdByIdx(kp, uc->list.val[i+1]-1);
				else
					nextId = keypadGetPadIdByIdx(kp, uc->list.val[0]-1);

				//printf("nextId %i, %i %i\n", nextId, uc->list.val[i+1]-1, uc->list.val[0]-1);

				keypadSetActive(kp, nextId);
				break;
			}
		}
	}else if (keyType == KP_KEYS_ENTER){
		//printf("key enter\n");
		keypadInputDispatchString(kp, &kp->listener);
		
		if (keypadListenerGetTotal(kp)){
			ccDisable(kp);
/*
			if (page2RenderGetState(kp->cc->vp->pages, PAGE_VKEYBOARD)){
				void *ptr = page2PageStructGet(kp->cc->vp->pages, PAGE_VKEYBOARD);
				if (ptr) page2SetPrevious(ptr);
			}
*/
		}
	}else if (keyType == KP_KEYS_RETURN){
		//printf("key return\n");
		keypadInputDispatchString(kp, &kp->listener);
		
	}else if (keyType == KP_KEYS_BACKSPACE){
		//printf("key backspace/left delete\n");
		
		TKP_EDITBOX *eb = &kp->editbox;
		keypadEditboxDeleteCharLeft(eb);
		keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
		keypadEditboxUpdateLabel(eb, eb->caret);

	}else if (keyType == KP_KEYS_DELETE){
		//printf("key right delete \n");
		
		TKP_EDITBOX *eb = &kp->editbox;
		keypadEditboxDeleteCharRight(eb);
		keypadEditboxAddCaret(eb, eb->buffer, eb->caret, eb->caretChr);
		keypadEditboxUpdateLabel(eb, eb->caret);
	}
	return 1;
}

int keypadInputKeyPressCb (TKEYPAD *kp, const int keyChar, const int64_t data)
{
	//printf("keypadInputKeyPressCb %i\n", keyChar);
	
	if (keyChar == VK_RETURN){		// enter
		keypadInputKeyPress(kp, NULL, KP_KEYS_ENTER, NULL);
		return 1;
	}else if (keyChar == VK_BACK){
		keypadInputKeyPress(kp, NULL, KP_KEYS_BACKSPACE, NULL);
		return 1;
	}else if (keyChar == KP_KEYS_DELETE){
		keypadInputKeyPress(kp, NULL, KP_KEYS_DELETE, NULL);
		return 1;
	}
	
	// find the key from the config
	for (int i = 0; i < kp->tPads; i++){
		TKP_PAD *pad = keypadGetPad(kp, keypadGetPadIdByIdx(kp,i));
		TKP_KEY *key = keypadGetKeyByCode(pad, keyChar);
		if (key){
			//printf("key found\n");
			keypadPlayKeyAlert(kp);
			kp->lastKeyPressTime0 = key->time = com_getTickCount();		
			keypadInputKeyPress(kp, pad, key->type, key);
			return 1;
		}
	}
	//printf("key not found %i\n", keyChar);
	return 0;
}

static inline int64_t keybtn_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg == CC_MSG_RENDER /*|| msg == CC_MSG_INPUT*/ || msg == CC_MSG_SETPOSITION) return 1;
	//if (msg == CC_MSG_ENABLED || msg == CC_MSG_DISABLED) return 1;
		
	TCCBUTTON2 *btn = (TCCBUTTON2*)object;
	//printf("ccbtn_cb, id:%i, objType:%i, msg:%i, data1:%i, data2:%i, ptr:%p\n", btn->id, btn->type, msg, (int)data1, (int)data2, dataPtr);
	//const int id = (int)data2;

	if (msg == BUTTON_MSG_SELECTED_PRESS){
		//printf("BUTTON_MSG_SELECTED_PRESS\n");
		
		TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
		TKEYPAD *kp = ccGetUserData(btn);
		keypadPlayKeyAlert(kp);
		
		kp->lastKeyPressTime0 = com_getTickCount();
		int padId = ccGetUserDataInt(btn);
		TKP_PAD *pad = keypadGetPad(kp, padId);
		TKP_KEY *key = keypadGetKey(pad, btn->id);
		key->time = pos->time;
		
		keypadInputKeyPress(kp, pad, key->type, key);
		
	}else if (msg == BUTTON_MSG_SELECTED_RELEASE){
		//printf("BUTTON_MSG_SELECTED_RELEASE\n");
		
		/*if (!isVirtual(btn->cc->vp))*/ return 1;
		
		TKEYPAD *kp = ccGetUserData(btn);
		int padId = ccGetUserDataInt(btn);
		TKP_PAD *pad = keypadGetPad(kp, padId);
		TKP_KEY *key = keypadGetKey(pad, btn->id);
		
		if (key->type == KP_KEYS_BACKSPACE){
			double dt = com_getTime(btn->cc) - kp->lastKeyReleaseTime;
			//printf("key up %.0f\n", dt);
			if (dt < KP_DBCLICK_BACKSPACE){
				TKP_EDITBOX *eb = &kp->editbox;
				keypadEditboxClear(eb);
			}
			kp->lastKeyReleaseTime = com_getTime(btn->cc);
		}
		
	}else if (msg == BUTTON_MSG_SELECTED_SLIDE){
		TKEYPAD *kp = ccGetUserData(btn);
		kp->lastKeyPressTime0 = com_getTickCount();
		int padId = ccGetUserDataInt(btn);
		TKP_PAD *pad = keypadGetPad(kp, padId);
		TKP_KEY *key = keypadGetKey(pad, btn->id);
		
		if (key->type == KP_KEYS_BACKSPACE){
			//printf("key slide KP_BACKSPACE %i %i\n", (int)data1, (int)data2);

			TTOUCHCOORD *pos = (TTOUCHCOORD*)dataPtr;
			int dt = pos->time - key->time;
			if (dt > KP_SLIDE_BACKSPACE){
				//printf("fkey slide %i %i %i %i\n", btn->id, idx, pos->dt, dt);
				TKP_EDITBOX *eb = &kp->editbox;
				keypadEditboxClear(eb);
				key->time = pos->time + KP_SLIDE_BACKSPACE;
			}
		}
	}
	return 1;
}

static inline int keypadRemovePad (TKEYPAD *kp, const int padId)
{
	for (int i = 0; i < KP_MAXPADS; i++){
		if (kp->pads[i]){
			if (kp->pads[i]->id == padId){
				kp->pads[i] = NULL;
				kp->tPads--;
				return 1;
			}
		}
	}
	return 0;
}

static inline int keypadBuild (TKEYPAD *kp, TVKSETTINGS *cfg, const wchar_t *configfile)
{	
	TCFGENTRY **config = cfg->config;
	
	int entries = cfg_configRead(config, configfile);
	if (!entries){
		cfg_configWrite(config, configfile);
		entries = cfg_configRead(config, configfile);
		//if (!entries)
			//wprintf(L"problem reading keypad config '%s'\n", configfile);
	}

	vkbSettingsGet(config, "keypad.position.x", &kp->metrics.x);
	vkbSettingsGet(config, "keypad.position.y", &kp->metrics.y);


	// import and process key list definitions
	str_list *strList = NULL;
	vkbSettingsGet(config, "keypad.key.", &strList);
	if (strList){
		//printf("vkey total keys defined %i\n", strList->total);

		cfg->keyList.total = strList->total;
		cfg->keyList.keys = my_calloc(strList->total, sizeof(TKPCFG_KEYCODE*));
		
		if (cfg->keyList.keys){
			for (int i = 0; i < cfg->keyList.total; i++){
				char *key = cfg_configStrListItem(strList, i);
				if (key)
					cfg->keyList.keys[i] = vkbParseStringKey(key);
			}
		}
		cfg_configStrListFree(strList);
		my_free(strList);
	}
	
	if (cfg->keyList.total < 10){
		//wprintf(L"keypad config '%s' is invalid\n", configfile);
		return 0;
	}


	// configure keys in to keypads
	vkbSettingsGet(config, "keypad.pad.", &strList);
	if (strList){
		//printf("vkey total pads defined %i\n", strList->total);

		cfg->padList.total = strList->total;
		cfg->padList.pads = my_calloc(strList->total, sizeof(TKPCFG_PADS*));

		if (cfg->padList.pads){
			for (int i = 0; i < cfg->padList.total; i++){
				char *keys = cfg_configStrListItem(strList, i);
				if (keys)
					cfg->padList.pads[i] = vkbParseStringPad(keys);
			}
		}
		cfg_configStrListFree(strList);
		my_free(strList);
	}
	if (cfg->padList.total < 1){
		//wprintf(L"keypad config '%s' is invalid\n", configfile);
		return 0;
	}


	// now build each pad
	for (int i = 0; i < cfg->padList.total; i++)
		keypadCfgBuildPad(kp, cfg, i);

	// disable everything until enable an request is sent
	for (int i = 0; i < kp->tPads; i++)
		keypadPadDisable(kp, keypadGetPadIdByIdx(kp, i));


	int ebx, eby, ebw, ebh;
	vkbSettingsGet(config, "keypad.editbox.caret", &kp->editbox.caretChr);
	vkbSettingsGet(config, "keypad.editbox.position.x", &ebx);
	vkbSettingsGet(config, "keypad.editbox.position.y", &eby);
	vkbSettingsGet(config, "keypad.editbox.width", &ebw);
	vkbSettingsGet(config, "keypad.editbox.height", &ebh);
	
	
	int lbId;
	kp->editbox.label = ccCreate(kp->cc, kp->pageOwner, CC_LABEL, ebbtn_cb, &lbId, ebw, ebh);
	kp->editbox.labelId = labelTextCreate(kp->editbox.label, " ", /*PF_TEXTBOUNDINGBOX|PF_GLYPHBOUNDINGBOX|*/PF_LEFTJUSTIFY, "uf/helvr24.uf"/*KEYPAD_INPUT_FONT*/, 0, 2);
	labelRenderFlagsSet(kp->editbox.label, LABEL_RENDER_TEXT);
	labelRenderFilterSet(kp->editbox.label, kp->editbox.labelId, 0);
	ccSetUserData(kp->editbox.label, kp);
	ccSetPosition(kp->editbox.label, kp->metrics.x+ebx, kp->metrics.y+eby);
	keypadEditboxClear(&kp->editbox);


	// set initial pad
	int val;
	vkbSettingsGet(config, "keypad.firstPad", &val);
	keypadSetActive(kp, keypadGetPadIdByIdx(kp, val-1));
	keypadPadEnable(kp, keypadGetActive(kp));


	//cfg_configWrite(config, configfile);
	return 1;
}

int buildKeypad (TKEYPAD *kp)
{
	if (!kp->isBuilt)
		kp->isBuilt = keypadBuild(kp, &kp->cfg, KP_CONFIGFILE);
	return kp->isBuilt;
}

int keypadRender (void *object, TFRAME *frame)
{
	TKEYPAD *kp = (TKEYPAD*)object;

	if (!buildKeypad(kp)) return 0;

	TKP_PAD *pad = keypadGetPad(kp, keypadGetActive(kp));
	if (!pad) return 0;	// shouldn't happen

	TLISTITEM *kItem = pad->keys;
	while(kItem){
		TKP_KEY *key = listGetStorage(kItem);
		if (key)
			ccRender(key->btn, frame);
		kItem = kItem->next;
	}

	//dbwprintf(kp->cc->vp, L"str %i '%s'\n", wcslen(kp->editbox.buffer), kp->editbox.buffer);
	
	TKP_EDITBOX *eb = &kp->editbox;
	keypadEditboxRender(eb, frame, eb->caret, 8, 96);
	return 1;
}

void keypadEnable (void *object)
{
	//printf("keypadEnable\n");

	TKEYPAD *kp = (TKEYPAD*)object;
	if (!buildKeypad(kp)) return;

	/*int ret =*/ keypadInputDispatchEvent(kp, &kp->listener, KP_INPUT_OPENED, KP_MSG_PAD_OPENED, kp->id, 0, NULL);
	//printf("keypadInputDispatchEvent enable %i\n", ret);

	ccEnable(kp->editbox.label);
}

void keypadDisable (void *object)
{
	//printf("keypadDisabled\n");
	
	TKEYPAD *kp = (TKEYPAD*)object;
	if (!kp->isBuilt) return;
	
	/*int ret =*/ keypadInputDispatchEvent(kp, &kp->listener, KP_INPUT_CLOSED, KP_MSG_PAD_CLOSED, kp->id, 0, NULL);
	//printf("keypadDisable: keypadInputDispatchEvent close %i\n", ret);
	
	ccDisable(kp->editbox.label);
}

int keypadInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	return 0;
}

int keypadSetPosition (void *object, const int x, const int y)
{
	return 0;
}

int keypadSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	return 0;
}

static inline void vkcfgFreeKeycode (TKPCFG_KEYCODE *uc)
{
	if (uc->list.val)
		my_free(uc->list.val);
	my_free(uc->image);
	my_free(uc);
}

static inline void vkcfgFreeKeyList (TKPCFG_KEYLIST *keyList)
{
	for (int i = 0; i < keyList->total; i++){
		if (keyList->keys[i])
			vkcfgFreeKeycode(keyList->keys[i]);
	}
	my_free(keyList->keys);
	//my_free(keyList);
}

static inline void vkcfgFreePadKeypos (TKPCFG_KEYPOS *keypos)
{
	my_free(keypos);
}

static inline void vkcfgFreePad (TKPCFG_PADS *pad)
{
	for (int i = 0; i < pad->total; i++)
		vkcfgFreePadKeypos(pad->list[i]);

	my_free(pad->list);
	my_free(pad);
}

static inline void vkcfgFreePadList (TKPCFG_PADLIST *padList)
{
	for (int i = 0; i < padList->total; i++)
		vkcfgFreePad(padList->pads[i]);

	my_free(padList->pads);
	//my_free(padList);
}

void keypadDelete (void *object)
{
	TKEYPAD *kp = (TKEYPAD*)object;

	cfg_configFree(kp->cfg.config);
	if (!kp->isBuilt) return;
	
	keypadListenerRemoveAll(kp);
	
	for (int i = 0; i < KP_MAXPADS; i++){
		TKP_PAD *pad = kp->pads[i];
		if (pad){
			keypadRemovePad(kp, pad->id);
			keypadDeletePad(pad);
		}
	}
	
	ccDelete(kp->editbox.label);
	vkcfgFreePadList(&kp->cfg.padList);
	vkcfgFreeKeyList(&kp->cfg.keyList);

}

int keypadNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t tv_cb, int *id, const int x, const int y)
{
	TKEYPAD *kp = (TKEYPAD*)object;

	kp->pageOwner = pageOwner;
	if (id) *id = kp->id;
	kp->type = type;
		
	
	kp->cb.msg = tv_cb;
	kp->cb.render = keypadRender;
	kp->cb.create = keypadNew;
	kp->cb.free = keypadDelete;
	kp->cb.enable = keypadEnable;
	kp->cb.disable = keypadDisable;
	kp->cb.input = keypadInput;
	kp->cb.setPosition = keypadSetPosition;
	kp->cb.setMetrics = keypadSetMetrics;

	kp->metrics.x = x;
	kp->metrics.y = y;
	kp->metrics.width = kp->cc->dwidth - kp->metrics.x;		// fix this
	kp->metrics.height = kp->cc->dheight - kp->metrics.y;
	kp->canDrag = 0;
	kp->tPads = 0;
	kp->isBuilt = 0;


	TVKSETTINGS *cfg = &kp->cfg;
	TCFGENTRY **config = vkey_configCreate(cfg);
	cfg->config = config;
	cfg_configApplyDefaults(config);
	
	//buildKeypad(kp);
	return 1;
}

