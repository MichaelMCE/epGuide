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



#ifndef _STACK_H_
#define _STACK_H_



// FILO/LIFO stack

typedef struct {
	intptr_t *data;
	int itemSize;
	int size;		// max number of items stack (stack length)
	int sp;			// current position
	TMLOCK *lock;
}TSTACK;




TSTACK *stackCreate (const int length);
void stackDestroy (TSTACK *stack);

int stackPush (TSTACK *stack, intptr_t data);
int stackPop (TSTACK *stack, intptr_t *data);
int stackPeek (TSTACK *stack, intptr_t *data);
int stackPeekEx (TSTACK *stack, intptr_t *data, const int depth);		// depth = look back by n
int stackCount (TSTACK *stack);
int stackClear (TSTACK *stack);

// generate a copy (an array) of stack data, destroy with my_free()
void *stackCopyPtr (TSTACK *stack, int *length);
intptr_t *stackCopyIntPtr (TSTACK *stack, int *length);
int64_t *stackCopyInt64 (TSTACK *stack, int *length);
int32_t *stackCopyInt32 (TSTACK *stack, int *length);

void stackDump (TSTACK *stack);




#endif

