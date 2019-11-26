
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





#ifndef _LOCK_H_
#define _LOCK_H_


#define DEBUG_LOCK			0
#define LOCK_NAMELENGTH		(63)



typedef struct {
	HANDLE hMutex;
	volatile int lockCount;
#if (!RELEASEBUILD)
	char name[LOCK_NAMELENGTH+1];
#endif
}TMLOCK;




TMLOCK *lockCreate (const char *name);
void lockClose (TMLOCK *lock);


#if (!DEBUG_LOCK)
int lockWait (TMLOCK *lock, const int mstime);
void lockRelease (TMLOCK *lock);

#else
void _lockRelease (TMLOCK *lock, const char *func, const int line);
int _lockWait (TMLOCK *lock, const int mstime, const char *func, const int line);

#define lockWait(a,b) _lockWait((a),(b),__FILE__,__LINE__)
#define lockRelease(a) _lockRelease((a),__FILE__,__LINE__)

#endif

#endif

