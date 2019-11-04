
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



#ifndef _STRINGCACHE_H_
#define _STRINGCACHE_H_



#define STRINGIMAGECACHESIZE	(127)

typedef struct{
	unsigned int hash;	// hash of input string
	TFRAME *img;		// the preprocessed image/string
}TFRAMESTRINGITEM;

typedef struct{
	TFRAMESTRINGITEM cache[STRINGIMAGECACHESIZE+1];
	int totalAdded;
	
	void *hSurfaceLib;
}TFRAMESTRINGCACHE;




TFRAME *strcFindString (TFRAMESTRINGCACHE *strc, const unsigned int hash);
int strcAddString (TFRAMESTRINGCACHE *strc, TFRAME *img, const unsigned int hash);

TFRAMESTRINGCACHE *strcNew (void *hSurfaceLib);
void strcFree (TFRAMESTRINGCACHE *strc);
void strcFlush (TFRAMESTRINGCACHE *strc);

// debuging only
size_t strcGetStoredSize (TFRAMESTRINGCACHE *strc);

#endif

