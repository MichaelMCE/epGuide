
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


/*

int listSetStorage (TLISTITEM *u, void *storage)
{
	u->storage = storage;
	return 1;
}*/

/*
void *listGetStorage (TLISTITEM *u)
{
	return u->storage;
}
*/

/*	return last item in list beginning at u
*/
TLISTITEM *listGetLast (TLISTITEM *u)
{
	TLISTITEM *n = u;
	do{
		if (!n->next) return n;
		n = n->next;
	}
	while(n);

	return NULL;
}

int listCount (TLISTITEM *u)
{
	if (!u) return 0;
	
	int i = 1;
	TLISTITEM *current = u;
	while ((current = current->next))
		i++;
	return i;
}

/*	insert u between p and n (or, to the right of p - to the left of n)
*/
void listInsert (TLISTITEM *u, TLISTITEM *p, TLISTITEM *n)
{
	if (p){
		u->prev = p;
		p->next = u;
	}else{
		u->prev = NULL;
	}
	
	if (n){
		u->next = n;
		n->prev = u;
	}else{
		u->next = NULL;
	}
}

/*	swap p and n
*/
int listSwap (TLISTITEM *p, TLISTITEM *n)
{
	if ((p->prev == n) || (p->next == n) || (n->prev == p) || (n->next == p)){
		TLISTITEM *tmp;
		if (p->next != n){
			tmp = p;
			p = n;
			n = tmp;
		}
	
		tmp = p->prev;
		if (p->prev) p->prev->next = n;
		p->prev = n;
		p->next = n->next;	
	
		if (n->next) n->next->prev = p;
		n->prev = tmp;
		n->next = p;
		return 1;
	}
	else {
		TLISTITEM *pn = p->next;
		TLISTITEM *pp = p->prev;
		listInsert(p, n->prev, n->next);
		listInsert(n, pp, pn);
		if (n->prev) n->prev->next = n;
		if (p->next) p->next->prev = p;
		return 1;
	}
}

/*	remove u from list and link adjacent links to each other
*/
TLISTITEM *listRemove (TLISTITEM *u)
{
	TLISTITEM *next = u->next;
	
	if (u->prev)
		u->prev->next = u->next;
	if (u->next)
		u->next->prev = u->prev;
		
	u->prev = NULL;
	u->next = NULL;

	return next;
}

/*	delete this user and free its resources
*/
int listDestroy (TLISTITEM *u)
{
	if (u){
		my_free(u);
		return 1;
	}else{
		return 0;
	}
}

/*	delete this and all users to the right
*/
int listDestroyNext (TLISTITEM *u)
{
	if (!u) return 0;
	
	TLISTITEM *next = u;
	TLISTITEM *n;

	int i = 0;
	do{
		n = next->next;
		listDestroy(next);
		next = n;
		i++;
	}while(next);
	
	return i;
}

/*	delete this and all users to the left
*/
int listDestroyPrev (TLISTITEM *u)
{
	if (!u) return 0;
	
	TLISTITEM *prev = u;
	TLISTITEM *p;

	int i = 0;
	do{
		p = prev->prev;
		listDestroy(prev);
		prev = p;
		i++;
	}while(prev);
	
	return i;
}

/*	delete every user
	returns total users (objects) deleted
*/
int listDestroyAll (TLISTITEM *u)
{
	if (!u) return 0;
	
	int c = listDestroyPrev(u->prev) + listDestroyNext(u->next);
	c += listDestroy(u);
	return c;
}

/*	create a user object
*/
TLISTITEM *listNewItem (void *storage)
{
	TLISTITEM *item = (TLISTITEM*)my_calloc(1, sizeof(TLISTITEM));
	if (item)
		item->storage = storage;
	return item;
}

