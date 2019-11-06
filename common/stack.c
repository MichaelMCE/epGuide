
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




static inline int stackLock (TSTACK *stack)
{
	return lockWait(stack->lock, INFINITE);
}

static inline void stackUnlock (TSTACK *stack)
{
	lockRelease(stack->lock);
}

TSTACK *stackCreate (const int length)
{
	TSTACK *stack = my_calloc(1, sizeof(TSTACK));
	if (stack){
		stack->sp = -1;
		stack->size = length;
		stack->itemSize = sizeof(intptr_t);
		stack->data = my_calloc(stack->size, stack->itemSize);
		stack->lock = lockCreate("stackCreate");
	}
	return stack;
}

void stackDestroy (TSTACK *stack)
{
	stackLock(stack);
	lockClose(stack->lock);
	my_free(stack->data);
	my_free(stack);
}

static inline int stackResize (TSTACK *stack, const int newSize)
{
	stack->size = newSize;
	stack->data = my_realloc(stack->data, stack->size * stack->itemSize);
	return (stack->data != NULL);
}

int stackPush (TSTACK *stack, intptr_t data)
{
	int ret = 0;
	if (stackLock(stack)){
		if (stack->sp >= stack->size-1)
			stackResize(stack, stack->size * 2);

		stack->data[++stack->sp] = data;
		ret = stack->sp+1;
		stackUnlock(stack);
	}
	return ret;
}

int stackPop (TSTACK *stack, intptr_t *data)
{
	int ret = 0;
	if (stackLock(stack)){
		if (stack->sp < 0){
			ret = 0;
			
		}else if (stack->sp >= 0){
			if (data) *data = stack->data[stack->sp];
			stack->sp--;
			//ret = (stack->sp > 0);
			ret = 1;
		}
		stackUnlock(stack);
	}
	return ret;
}

int stackCount (TSTACK *stack)
{
	int ret = 0;
	
	if (stackLock(stack)){
		ret = stack->sp+1;
		stackUnlock(stack);
	}
	return ret;
}

int stackPeekEx (TSTACK *stack, intptr_t *data, const int depth)
{
	int ret = 0;
	
	if (stackLock(stack)){
		if (stack->sp - depth >= 0){
			*data = stack->data[stack->sp - depth];
			ret = 1;
		}
		stackUnlock(stack);
	}
	return ret;
}

int stackPeek (TSTACK *stack, intptr_t *data)
{
	int ret = 0;
	
	if (stackLock(stack)){
		if (stack->sp >= 0){
			*data = stack->data[stack->sp];
			ret = 1;
		}
		stackUnlock(stack);
	}
	return ret;
}

int stackClear (TSTACK *stack)
{
	int ret = 0;
	
	if (stackLock(stack)){
		stack->sp = 0;
		ret = 1;
		stackUnlock(stack);
	}
	return ret;
}

intptr_t *stackCopyIntPtr (TSTACK *stack, int *length)
{
	intptr_t *copy = NULL;
	
	if (stackLock(stack)){
		if (stack->sp >= 0){
			copy = my_calloc(stack->sp+1, sizeof(intptr_t));
		
			for (int i = 0; i <= stack->sp; i++)
				copy[i] = (intptr_t)stack->data[i];
				
			*length = stack->sp+1;
		}else{
			*length = 0;
		}
		stackUnlock(stack);
	}
	return copy;
}

int64_t *stackCopyInt64 (TSTACK *stack, int *length)
{
	int64_t *copy = NULL;
	
	if (stackLock(stack)){
		if (stack->sp >= 0){
			copy = my_calloc(stack->sp+1, sizeof(int64_t));
		
			for (int i = 0; i <= stack->sp; i++)
				copy[i] = (int64_t)stack->data[i];
				
			*length = stack->sp+1;
		}else{
			*length = 0;
		}
		stackUnlock(stack);
	}
	return copy;
}

int32_t *stackCopyInt32 (TSTACK *stack, int *length)
{
	int32_t *copy = NULL;
	
	if (stackLock(stack)){
		if (stack->sp >= 0){
			copy = my_calloc(stack->sp+1, sizeof(int32_t));
		
			for (int i = 0; i <= stack->sp; i++)
				copy[i] = (int32_t)stack->data[i];
				
			*length = stack->sp+1;
		}else{
			*length = 0;
		}
		stackUnlock(stack);
	}
	return copy;
}

void *stackCopyPtr (TSTACK *stack, int *length)
{
	return (void*)stackCopyIntPtr(stack, length);
}

#if 0
void stackDump (TSTACK *stack)
{
	if (stackLock(stack)){
		for (int i = 0; i <= stack->sp; i++){
			printf("stackDump %i %i\n", i, stack->data[i]);
		}
		stackUnlock(stack);
	}
}
#endif
