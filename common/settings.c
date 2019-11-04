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

/*
#ifndef _strtoui64
_CRTIMP unsigned __int64 __cdecl _strtoui64(const char *_String,char **_EndPtr,int _Radix);
#endif
*/





static inline int calctotal (const TCFGENTRY *config_cfg)
{
	int ct = 0;
	while (config_cfg[ct].key) ct++;
	return ++ct; // +1 for NULL last item marker
}


/*
static inline int calcTotal (const TCFGENTRY **config_cfg)
{
	int ct = 0;
	while (config_cfg[ct]->key) ct++;
	return ++ct; // +1 for NULL last item marker
}
*/

// get...
char *cfg_configStrListItem (str_list *strList, const int index)
{
	if (index < strList->total)
		return strList->strings[index];
	else
		return NULL;
}

void cfg_configStrListFree (str_list *strList)
{
	if (strList){
		for (int j = 0; j < strList->total; j++){
			if (strList->strings[j])
				my_free(strList->strings[j]);
		}
	}
}

str_list *cfg_configStrListNew (const int total)
{
	str_list *strList = my_calloc(1, sizeof(str_list));
	if (strList)
		strList->total = total;
	return strList;
}

str_list *cfg_configStrListDup (const str_list *strList)
{
	str_list *newList = cfg_configStrListNew(strList->total);
	if (newList){
		for (int i = 0; i < strList->total && strList->strings[i]; i++)
			newList->strings[i] = my_strdup(strList->strings[i]);
	}
	return newList;
}

str_list *cfg_configStrListDupW (str_list *strList)
{
	str_list *newList = cfg_configStrListNew(strList->total);
	if (newList){
		for (int i = 0; i < strList->total; i++)
			newList->strings[i] = (char*)com_converttow(strList->strings[i]);
	}
	return newList;
}

TCFGENTRY *cfg_entryDup (const TCFGENTRY *item)
{
	TCFGENTRY *entry = my_calloc(1, sizeof(TCFGENTRY));
	//printf("entry %p\n", entry);


	if (item->key){
		entry->key = my_strdup(item->key);
		entry->type = item->type;
		entry->hash = item->hash;
		if (!entry->hash)
			entry->hash = com_generateHash(entry->key, strlen(entry->key));
		if (item->comment)
			entry->comment = my_strdup(item->comment);

		switch (entry->type){
		  case CFG_INT:
			entry->u.val32 = item->u.val32;
			entry->ptr = item->ptr;
			break;

		  case CFG_INT64:
		  	entry->u.val64 = item->u.val64;
		  	entry->ptr = item->ptr;
			break;

		  case CFG_HEX:
		  	entry->u.valu64 = item->u.valu64;
		  	entry->ptr = item->ptr;
			break;

		  case CFG_STRING:
		  	entry->u.valStr = my_strdup(item->u.valStr);
		  	entry->ptr = my_strdup(item->u.valStr);
			break;

		  case CFG_FLOAT:
		  	entry->u.valFloat = item->u.valFloat;
		  	entry->ptr = item->ptr;
			break;

		  case CFG_DOUBLE:
		  	entry->u.valDouble = item->u.valDouble;
		  	entry->ptr = item->ptr;
			break;

		  case CFG_CHAR:
		  	entry->u.valChar = item->u.valChar;
		  	entry->ptr = item->ptr;
			break;

		  case CFG_STRLIST:
	  		entry->u.strList.total = item->u.strList.total;
		  	for (int i = 0; i < item->u.strList.total && item->u.strList.strings[i]; i++)
		  		entry->u.strList.strings[i] = my_strdup(item->u.strList.strings[i]);

		  	entry->ptr = cfg_configStrListDup(&item->u.strList);
			break;

		  case CFG_BREAK:
		  	break;
		}
	}
	return entry;
}

TCFGENTRY **cfg_configDup (const TCFGENTRY *config_cfg)
{
	const int total = calctotal(config_cfg);

	TCFGENTRY **config = my_calloc(total+1, sizeof(TCFGENTRY*));
    if (config){
    	for (int i = 0; i < total; i++)
	    	config[i] = cfg_entryDup(&config_cfg[i]);
	}
    return config;
}

void cfg_configFree (TCFGENTRY **config)
{
	int ct = 0;	// find upper NUL entry so we cna free it manually
	while (config[ct]->key) ct++;

	for (int i = 0; config[i]->key; i++){
		TCFGENTRY *entry = config[i];
		if (entry){
			if (entry->key){
				if (entry->type == CFG_STRING){
					if (entry->u.valStr)
						my_free(entry->u.valStr);
					if (entry->ptr)
						my_free(entry->ptr);

				}else if (entry->type == CFG_STRLIST){
					cfg_configStrListFree(&entry->u.strList);

					if (entry->ptr){
		  				cfg_configStrListFree(entry->ptr);
		  				my_free(entry->ptr);
				  	}
				}
				my_free(entry->key);
			}
			if (entry->comment)
				my_free(entry->comment);
			my_free(entry);
		}
	}
	my_free(config[ct]);
	my_free(config);
}

#if 0
void cfg_configDump (TCFGENTRY **config)
{
	for (int i = 0; config[i]->key; i++){
		if (config[i]->type == CFG_BREAK)
			printf("\n");
		else
			printf("%i: %s, %i, %p %X\n", i, config[i]->key, config[i]->type, config[i]->ptr, config[i]->hash);

		if (config[i]->type == CFG_INT){
			printf("\t%i %i\n", config[i]->u.val32, *(int32_t*)config[i]->ptr);

		}else if (config[i]->type == CFG_FLOAT){
			printf("\t%f %f\n", config[i]->u.valFloat, *(float*)config[i]->ptr);

		}else if (config[i]->type == CFG_DOUBLE){
			printf("\t%f %f\n", config[i]->u.valDouble, *(double*)config[i]->ptr);

		}else if (config[i]->type == CFG_STRING){
			if (config[i]->u.valStr)
				printf("\t'%s' '%s'\n", config[i]->u.valStr, (char*)config[i]->ptr);

		}else if (config[i]->type == CFG_STRLIST){
			str_list *strList = config[i]->ptr;
			for (int j = 0; j < strList->total; j++){
				if (strList->strings[j])
					printf("\t%i:%s\n",j, strList->strings[j]);
			}
		}
	}
}
#endif

void cfg_configApplyDefaults (TCFGENTRY **config)
{
	for (int i = 0; config[i]->key; i++){
		TCFGENTRY *entry = config[i];
	
		switch (entry->type){
		  case CFG_INT:
			*(int32_t*)entry->ptr = entry->u.val32;
			break;

		  case CFG_INT64:
		  	*(int64_t*)entry->ptr = entry->u.val64;
			break;

		  case CFG_HEX:
		  	*((uint64_t*)entry->ptr) = entry->u.valu64;
			break;

		  case CFG_STRING:
		  	if (entry->ptr) my_free(entry->ptr);
		  	entry->ptr = my_strdup(entry->u.valStr);
			break;

		  case CFG_FLOAT:
		  	*(float*)entry->ptr = entry->u.valFloat;
			break;

		  case CFG_DOUBLE:
		  	*(double*)entry->ptr = entry->u.valDouble;
			break;

		  case CFG_CHAR:
		  	*(char*)entry->ptr = entry->u.valChar;
			break;

		  case CFG_STRLIST:
		  	if (entry->ptr){
		  		cfg_configStrListFree(entry->ptr);
		  		my_free(entry->ptr);
		  	}
		  	str_list *strList = cfg_configStrListDup(&entry->u.strList);
		  	entry->ptr = strList;
			break;
		}
	}
}

str_keyvalue *findKey (const char *key, str_keyvalue *keys, const int ktotal)
{
#if 1
	const int keylen = strlen(key);

	for (int i = 0; i < ktotal; i++){
		if (!strncmp(key, keys[i].key, keylen))
			return &keys[i];
	}
#else
	for (int i = 0; i < ktotal; i++){
		if (keys[i].hash == hash)
			return &keys[i];
	}
#endif
	return NULL;
}

str_keyvalue *findKeyNext (const char *key, str_keyvalue *keys, const int ktotal, str_keyvalue *next)
{
	for (int i = 0; i < ktotal-1; i++){
		if (&keys[i] == next){
			i++;
			return findKey(key, &keys[i], ktotal-i);
		}
	}
	return NULL;
}

int findKeyListTotal (const char *key, str_keyvalue *keys, const int ktotal)
{
	int total = 0;
	int actual = 0;
	for (int i = 0; i < ktotal-1; i++){
		str_keyvalue *found = findKey(key, &keys[i], ktotal-i);
		if (found){
			total++;

			char *pt = strrchr(found->key, '.');
			if (pt && *pt){
				pt++;
				if (*pt && isdigit(*pt))
					actual++;
			}
			while((found=findKeyNext(key, keys, ktotal, found))){
				char *pt = strrchr(found->key, '.');
				if (pt && *pt){
					pt++;
					if (*pt && isdigit(*pt))
						actual++;
				}
				total++;
			}
			break;
		}
	}
	return actual;
}

int cfg_configRead (TCFGENTRY **config, const wchar_t *filename)
{

	TASCIILINE *al = readFileW(filename);
	if (!al) return 0;
	if (al->tlines < 10){
		freeASCIILINE(al);
		return 0;
	}

	const int tlines = al->tlines;
	str_keyvalue keys[tlines];
	memset(keys, 0, sizeof(keys));

	int i = 0;
	for (int j = 0; j < tlines; j++){
		if (!*al->line[j] || *al->line[j] == CFG_COMMENTCHAR || *al->line[j] == ' ')
			continue;

		keys[i].key = strtok((char*)al->line[j], CFG_SEPARATOR);
		if (keys[i].key){
			keys[i].key = com_removeLeadingSpaces(com_removeTrailingSpaces(keys[i].key));
			//keys[i].hash = com_generateHash(keys[i].key, strlen(keys[i].key));
			keys[i].value = strtok(NULL, CFG_COMMENT);
			if (keys[i].value){
				keys[i].value = com_removeLeadingSpaces(com_removeTrailingSpaces(keys[i].value));
				if (*keys[i].value) i++;
			}
		}
	}
	const int ktotal = i;


	for (int i = 0; config[i]->key; i++){
		TCFGENTRY *entry = config[i];

		str_keyvalue *key = findKey(entry->key, keys, ktotal);
		if (!key) continue;

		//printf("%i '%s'\n", entry->hash, entry->key);

		switch (entry->type){
		  case CFG_INT:
			*(int32_t*)entry->ptr = atoi(key->value);
			break;

		  case CFG_INT64:
		  	*(int64_t*)entry->ptr = _atoi64(key->value);
			break;

		  case CFG_HEX:
		  	sscanf(key->value, "%I64X", (uint64_t*)entry->ptr);
			break;

		  case CFG_STRING:
		  	if (entry->ptr) my_free(entry->ptr);
		  	entry->ptr = my_strdup(key->value);
			break;

		  case CFG_FLOAT:
		  	*(float*)entry->ptr = (float)atof(key->value);
			break;

		  case CFG_DOUBLE:
		  	*(double*)entry->ptr = (double)atof(key->value);
			break;

		  case CFG_CHAR:
		  	*(char*)entry->ptr = key->value[0];
			break;

		  case CFG_STRLIST:{
		  	int total = findKeyListTotal(entry->key, keys, ktotal);
		  	//printf("strList total %i %X '%s'\n", total, entry->hash, entry->key);

		  	if (total){
		  		str_list *strList = my_calloc(1, sizeof(str_list));
		  		if (strList){
			  		strList->total = total;

			  		for (int i = 0; i < total && key; i++){
			  			strList->strings[i] = my_strdup(key->value);
		  				key = findKeyNext(entry->key, keys, ktotal, key);
		  			}

		  			if (entry->ptr){
		  				cfg_configStrListFree(entry->ptr);
		  				my_free(entry->ptr);
		  			}
		  			entry->ptr = strList;
		  		}
		  	}
			break;
		  }
		}
	}

	freeASCIILINE(al);
	return tlines;
}

static inline int settingsWriteUtf8Marker (FILE *fp)
{
	return fprintf(fp,"﻿");
}

static inline int settingsWriteBreak (FILE *fp)
{
	return fprintf(fp, "\r\n");
}

static inline int settingsWriteString (FILE *fp, const char *buffer)
{
	fprintf(fp, "%s", buffer);
	return settingsWriteBreak(fp);
}

static inline int settingsWriteLine (FILE *fp, char *buffer, const int blen, const char *comment)
{
	if (!comment){
		return settingsWriteString(fp, buffer);
	}else{
		int flen = CFG_COMMENTCOL - blen;
		if (flen <= 0) flen = 4;

		char filler[flen+1];
		memset(filler, 32, flen);

		if (!blen){
			buffer = "";
			flen = 0;
		}
		filler[flen] = 0;
		fprintf(fp, "%s%s%c  %s", buffer, filler, CFG_COMMENTCHAR, comment);
		return settingsWriteBreak(fp);
	}
}

static inline int settingsWriteStrList (FILE *fp, const char *key, const str_list *strList)
{
	char buffer[MAX_PATH_UTF8*2];
	int written = 0;

	for (int j = 0; j < strList->total && strList->strings[j]; j++){
		int slen = __mingw_snprintf(buffer, sizeof(buffer), "%s%i"CFG_SEPARATOR" %s", key, j+1, strList->strings[j]);
		written += settingsWriteLine(fp, buffer, slen, NULL);
	}
	return written;
}

int settingsWriteKeys (FILE *fp, TCFGENTRY **config)
{
	char buffer[MAX_PATH_UTF8+1];
	int slen = 0;

	for (int i = 0; config[i]->key; i++){
		TCFGENTRY *entry = config[i];

		switch (entry->type){
		  case CFG_INT:
		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %i", entry->key, *(int32_t*)entry->ptr);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;

		  case CFG_INT64:{
		  	char buffer[64];
		  	_i64toa(*(int64_t*)entry->ptr, buffer, 10);
		  	//slen = _snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %I64i", entry->key, *(int64_t*)entry->ptr);
		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %s", entry->key, buffer);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;
		  }
		  case CFG_HEX:{
		  	uint64_t val = *((uint64_t*)entry->ptr);
		  	ULARGE_INTEGER ui = (ULARGE_INTEGER)val;
		  	//slen = _snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %I64X", entry->key, val);
		  	if (ui.HighPart)
		  		slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %X%X", entry->key, (uint32_t)ui.HighPart, (uint32_t)ui.LowPart);
		  	else
		  		slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %X", entry->key, (uint32_t)ui.LowPart);
		  	//printf("i64 #%s# %I64X\n", buffer, val);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
		  }
			break;

		  case CFG_STRING:{
		  	char *str = entry->ptr;
		  	if (!str) str = entry->u.valStr;

		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %s", entry->key, str);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;
		  }
		  case CFG_FLOAT:
		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %f", entry->key, *(float*)entry->ptr);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;

		  case CFG_DOUBLE: 
		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %f", entry->key, *(double*)entry->ptr);
		  	//printf("writeDouble: %s: %f '%s' #%s#\n", entry->key, *(double*)entry->ptr, buffer, CFG_SEPARATOR);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;

		  case CFG_CHAR:
		  	slen = __mingw_snprintf(buffer, sizeof(buffer), "%s"CFG_SEPARATOR" %c", entry->key, *(char*)entry->ptr);
		  	settingsWriteLine(fp, buffer, slen, entry->comment);
			break;

		  case CFG_STRLIST:{
		  	str_list *strList = entry->ptr;
			if (!strList) strList = &entry->u.strList;

			settingsWriteStrList(fp, entry->key, strList);
			break;
		  }
		  case CFG_BREAK:
		  	settingsWriteBreak(fp);
		  	break;
		}
	}
	return ftell(fp);
}

int cfg_configWrite (TCFGENTRY **config, const wchar_t *filename)
{

	FILE *fp = _wfopen(filename, L"w+b");
	if (!fp) return 0;

	settingsWriteUtf8Marker(fp);
	settingsWriteBreak(fp);
	settingsWriteBreak(fp);
	settingsWriteLine(fp, NULL, 0, "All paths' are UTF8 encoded");
	settingsWriteBreak(fp);
	settingsWriteBreak(fp);
	settingsWriteKeys(fp, config);
	settingsWriteBreak(fp);

	int len = ftell(fp);
	fclose(fp);
	return len;
}

TCFGENTRY *cfg_keyFind (TCFGENTRY **config, const char *key)
{
	const int hash = com_generateHash(key, strlen(key));
	for (int i = 0; config[i]->key; i++){
		TCFGENTRY *entry = config[i];
		if (hash == entry->hash)
			return entry;
	}
	return NULL;
}

int cfg_keyGet (TCFGENTRY **config, const char *key, void *value)
{
	TCFGENTRY *entry = cfg_keyFind(config, key);

	//printf("cfg_keyGet '%s' %p\n", key, entry);

	if (entry){
		switch (entry->type){
		  case CFG_INT:
			*(int32_t*)value = *(int32_t*)entry->ptr;
			break;

		  case CFG_INT64:
		  	*(int64_t*)value = *(int64_t*)entry->ptr;
			break;

		  case CFG_HEX:
		  	*(uint64_t*)value = *(uint64_t*)entry->ptr;
			break;

		  case CFG_STRING:
		  	*(char**)value = my_strdup(entry->ptr);
		  	//printf("CFG_STRING #%s# %p\n", key, *(char**)value);
			break;

		  case CFG_FLOAT:
		  	*(float*)value = *(float*)entry->ptr;
			break;

		  case CFG_DOUBLE:
		  	*(double*)value = *(double*)entry->ptr;
			break;

		  case CFG_CHAR:
		  	*(char*)value = *(char*)entry->ptr;
			break;

		  case CFG_STRLIST:
		  	*(str_list**)value = cfg_configStrListDup(entry->ptr);
			break;
		}
	}
	return entry != NULL;
}

int cfg_keyGetW (TCFGENTRY **config, const char *key, void *value)
{
	TCFGENTRY *entry = cfg_keyFind(config, key);
	if (entry){
		switch (entry->type){
		  case CFG_INT:
			*(int32_t*)value = *(int32_t*)entry->ptr;
			break;

		  case CFG_INT64:
		  	*(int64_t*)value = *(int64_t*)entry->ptr;
			break;

		  case CFG_HEX:
		  	*(uint64_t*)value = *(uint64_t*)entry->ptr;
			break;

		  case CFG_STRING:
		  	*(wchar_t**)value = com_converttow(entry->ptr);
			break;

		  case CFG_FLOAT:
		  	*(float*)value = *(float*)entry->ptr;
			break;

		  case CFG_DOUBLE:
		  	*(double*)value = *(double*)entry->ptr;
			break;

		  case CFG_CHAR:
		  	*(wchar_t*)value = *(char*)entry->ptr;
			break;

		  case CFG_STRLIST:
		  	*(str_list**)value = cfg_configStrListDupW(entry->ptr);
			break;
		}
	}
	return entry != NULL;
}

int cfg_keySet (TCFGENTRY **config, const char *key, void *value)
{
	TCFGENTRY *entry = cfg_keyFind(config, key);
	if (entry){
		switch (entry->type){
		  case CFG_INT:
			*(int32_t*)entry->ptr = *(int32_t*)value;
			break;

		  case CFG_INT64:
		  	*(int64_t*)entry->ptr = *(int64_t*)value;
			break;

		  case CFG_HEX:
		  	*(uint64_t*)entry->ptr = *(uint64_t*)value;
			break;

		  case CFG_STRING:
		  	if (entry->ptr) my_free(entry->ptr);
		  	entry->ptr = my_strdup(value);
		  	//printf("string set for '%s'\n", (char*)entry->ptr);
			break;

		  case CFG_FLOAT:
		  	*(float*)entry->ptr = *(float*)value;
			break;

		  case CFG_DOUBLE:
		  	*(double*)entry->ptr = *(double*)value;
		  	//printf("setting double %p %f\n", entry->ptr, (double)*(double*)entry->ptr);
			break;

		  case CFG_CHAR:
		  	*(char*)entry->ptr = *(char*)value;
			break;

		  case CFG_STRLIST:
		  	if (entry->ptr){
		  		cfg_configStrListFree(entry->ptr);
		  		my_free(entry->ptr);
		  	}
		  	//entry->ptr = cfg_configStrListDup(value);
		  	entry->ptr = value;
			break;
		}
	}

	return (entry != NULL);
}

char *cfg_commentGet (TCFGENTRY **config, char *key)
{
	TCFGENTRY *entry = cfg_keyFind(config, key);
	if (entry && entry->comment)
		return  my_strdup(entry->comment);
	return NULL;
}

int cfg_commentSet (TCFGENTRY **config, char *key, char *comment)
{
	TCFGENTRY *entry = cfg_keyFind(config, key);
	if (entry){
		if (entry->comment)
			my_free(entry->comment);
		entry->comment = my_strdup(comment);
	}else{
		printf("cfg_commentSet: invalid key: '%s'\n", key);
	}
	return (entry != NULL);
}

