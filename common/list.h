
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



#ifndef _LIST_H_
#define _LIST_H_




typedef struct TLISTITEM {
	struct TLISTITEM *next, *prev;
	void *storage;
}TLISTITEM;



TLISTITEM *listNewItem (void *storage);
int listDestroyAll	(TLISTITEM *u);
int listDestroyPrev (TLISTITEM *u);
int listDestroyNext (TLISTITEM *u);
int listDestroy		(TLISTITEM *u);

TLISTITEM *listRemove (TLISTITEM *u);		// returns next item in list
int listSwap (TLISTITEM *p, TLISTITEM *n);
void listInsert (TLISTITEM *u, TLISTITEM *p, TLISTITEM *n);

TLISTITEM *listGetLast (TLISTITEM *u);
#define listGetNext(u)			(((TLISTITEM*)(u))->next)
#define listGetPrev(u)			(((TLISTITEM*)(u))->prev)
#define listGetStorage(u)		(((TLISTITEM*)(u))->storage)
#define listSetStorage(u,ptr)	(((TLISTITEM*)(u))->storage=(ptr))

int listCount (TLISTITEM *u);

//void *listGetStorage (TLISTITEM *u);
//int listSetStorage (TLISTITEM *u, void *storage);



#define listAdd(list,item)					\
	if ((list))								\
		listInsert(((item)),NULL,(list));	\
	(list) = ((item));


#endif
