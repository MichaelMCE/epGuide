
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




TMLOCK *lockCreate (const char *name)
{
	TMLOCK *lock = my_calloc(1, sizeof(TMLOCK));
	if (lock){
#if (!RELEASEBUILD)
		strncpy(lock->name, name, LOCK_NAMELENGTH);
#endif
		lock->hMutex = CreateMutex(NULL, FALSE, NULL);
		lock->lockCount = 0;
		//printf("lockCreate: '%s', %i\n", name, (int)GetCurrentThreadId());
	}
	return lock;
}

void lockClose (TMLOCK *lock)
{
	if (lock){
		if (lock->hMutex){
			//printf("lockClose: '%s' %i, %i\n", lock->name, lock->lockCount, (int)GetCurrentThreadId());
			HANDLE mutex = lock->hMutex;
			lock->hMutex = NULL;
			CloseHandle(mutex);
			lock->lockCount = 0;
		}
		my_free(lock);
	}
}


#if DEBUG_LOCK
int _lockWait (TMLOCK *lock, const int mstime, const char *func, const int line)
#else
int lockWait (TMLOCK *lock, const int mstime)
#endif
{
	if (lock && lock->hMutex){
		//printf("lockWait: '%s', %i, %i (%s:%i)\n", lock->name, lock->lockCount, (int)GetCurrentThreadId(), func, line);
		//int ret = (WaitForSingleObject(lock->hMutex, mstime) == WAIT_OBJECT_0);
		
		int ret = 1;

		if (mstime == INFINITE || mstime > 60*1000){
			while (ret){
				ret = WaitForSingleObject(lock->hMutex, 10000);
				if (ret == WAIT_OBJECT_0){
					ret = 1;
					break;
				}else if (ret == WAIT_TIMEOUT){
					//printf("possible deadlock: thread:%X ct:%i, %s\n", (int)GetCurrentThreadId(), lock->lockCount, lock->name);
				}else{
					//int err = (int)GetLastError();
					//printf("lock failed: ret:%X GetLstEr:%X thread:%X ct:%i, %s\n", ret, err, (int)GetCurrentThreadId(), lock->lockCount, lock->name);
					return 0;
				}
			}
		}else{
			ret = (WaitForSingleObject(lock->hMutex, mstime) == WAIT_OBJECT_0);
		}

		if (ret){
			//printf("lockWait: got '%s', %i, %i (%s:%i)\n", lock->name, lock->lockCount+1, (int)GetCurrentThreadId(), func, line);
			lock->lockCount++;
		}
		return ret;
	}
	return 0;
}

#if DEBUG_LOCK
void _lockRelease (TMLOCK *lock, const char *func, const int line)
#else
void lockRelease (TMLOCK *lock)
#endif
{
	if (lock && lock->hMutex){
		//printf("lockRelease: '%s', %i, %i (%s:%i)\n", lock->name, lock->lockCount, (int)GetCurrentThreadId(), func, line);
		lock->lockCount--;
		ReleaseMutex(lock->hMutex);
	}
}

