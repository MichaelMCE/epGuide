
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


#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#define CFG_PATHSEPARATOR		"<|>"
#define CFG_COMMENTCOL			(50)
#define CFG_COMMENTCHAR			'\''
#define CFG_SEPARATOR			":"
#define CFG_COMMENT				"'"


enum _cfg_types {
	CFG_INT = 1,	// 32bit
	CFG_INT64,		// 64bit
	CFG_HEX,		// stored as a 64bit uint32_t, written to file in caps (ABC) 
	CFG_STRING,
	CFG_FLOAT,
	CFG_DOUBLE,
	CFG_CHAR,
	CFG_STRLIST,
	CFG_BREAK
};

typedef struct{
	char *key;
	char *value;
}str_keyvalue;

typedef struct {
	char *strings[256];
	int total;
}str_list;

typedef union {
	int32_t	 val32;
	int64_t	 val64;
	uint64_t valu64;		// hex storage
	char* 	valStr;
	float	valFloat;
	double	valDouble;
	char	valChar;
	str_list strList;
}TCFG_VALUE;

typedef struct {
	char *key;
	char type;			// data type

	TCFG_VALUE u;
	
	void *ptr;
	//uint64_t *ptr;
	int hash;
	char *comment;
}TCFGENTRY;



#define V_STR(x)	CFG_STRING, .u.valStr=(x)
#define V_INT32(x)	CFG_INT,	.u.val32=(x)
#define V_INT64(x)	CFG_INT64,	.u.val64=(x)
#define V_HEX(x)	CFG_HEX,	.u.valu64=(x)
#define V_FLT(x)	CFG_FLOAT,	.u.valFloat=(x)
#define V_DBL(x)	CFG_DOUBLE,	.u.valDouble=(x)
#define V_CHR(x)	CFG_CHAR,	.u.valChar=(x)
#define V_BRK(x)	CFG_BREAK,	.u.val32=(x)


#define V_SLIST1(a)							CFG_STRLIST,\
				.u.strList.total=1,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=NULL
				
#define V_SLIST2(a,b)						CFG_STRLIST,\
				.u.strList.total=2,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=NULL
				
#define V_SLIST3(a,b,c)						CFG_STRLIST,\
				.u.strList.total=3,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=NULL

#define V_SLIST4(a,b,c,d)					CFG_STRLIST,\
				.u.strList.total=4,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=NULL

#define V_SLIST5(a,b,c,d,e)					CFG_STRLIST,\
				.u.strList.total=5,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=NULL

#define V_SLIST6(a,b,c,d,e,f)				CFG_STRLIST,\
				.u.strList.total=6,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=NULL

#define V_SLIST7(a,b,c,d,e,f,g)				CFG_STRLIST,\
				.u.strList.total=7,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=NULL

#define V_SLIST8(a,b,c,d,e,f,g,h)			CFG_STRLIST,\
				.u.strList.total=8,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=h,\
				.u.strList.strings[8]=NULL

#define V_SLIST9(a,b,c,d,e,f,g,h,i)			CFG_STRLIST,\
				.u.strList.total=9,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=h,\
				.u.strList.strings[8]=i,\
				.u.strList.strings[9]=NULL
			
#define V_SLIST10(a,b,c,d,e,f,g,h,i,j)		CFG_STRLIST,\
				.u.strList.total=10,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=h,\
				.u.strList.strings[8]=i,\
				.u.strList.strings[9]=j,\
				.u.strList.strings[10]=NULL
				
#define V_SLIST11(a,b,c,d,e,f,g,h,i,j,k)	CFG_STRLIST,\
				.u.strList.total=11,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=h,\
				.u.strList.strings[8]=i,\
				.u.strList.strings[9]=j,\
				.u.strList.strings[10]=k,\
				.u.strList.strings[11]=NULL

#define V_SLIST12(a,b,c,d,e,f,g,h,i,j,k,l)	CFG_STRLIST,\
				.u.strList.total=12,\
				.u.strList.strings[0]=a,\
				.u.strList.strings[1]=b,\
				.u.strList.strings[2]=c,\
				.u.strList.strings[3]=d,\
				.u.strList.strings[4]=e,\
				.u.strList.strings[5]=f,\
				.u.strList.strings[6]=g,\
				.u.strList.strings[7]=h,\
				.u.strList.strings[8]=i,\
				.u.strList.strings[9]=j,\
				.u.strList.strings[10]=k,\
				.u.strList.strings[11]=l,\
				.u.strList.strings[12]=NULL





void cfg_configFree (TCFGENTRY **config);


int cfg_configRead (TCFGENTRY **config, const wchar_t *name);
int cfg_configWrite (TCFGENTRY **config, const wchar_t *name);
void cfg_configApplyDefaults (TCFGENTRY **config);
void cfg_configDump (TCFGENTRY **config);

int cfg_keySet (TCFGENTRY **config, const char *key, void *value);
int cfg_keyGet (TCFGENTRY **config, const char *key, void *value);
int cfg_keyGetW (TCFGENTRY **config, const char *key, void *value);


str_list *cfg_configStrListNew (const int total);
void cfg_configStrListFree (str_list *strList);
char *cfg_configStrListItem (str_list *strList, const int index);

int cfg_commentSet (TCFGENTRY **config, char *key, char *comment);
char *cfg_commentGet (TCFGENTRY **config, char *key);

TCFGENTRY **cfg_configDup (const TCFGENTRY *config_cfg);

TCFGENTRY *cfg_keyFind (TCFGENTRY **config, const char *key);


#endif

