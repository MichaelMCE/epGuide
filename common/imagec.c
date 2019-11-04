
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



static inline void imgLoaderNewImgSetSignal (TASYNCIMGLOAD *imgLoader)
{
	SetEvent(imgLoader->loadEvent);
}

static inline int imgLoaderWaitForNewImageLoadSignal (TASYNCIMGLOAD *imgLoader)
{
	return (WaitForSingleObject(imgLoader->loadEvent, INFINITE) == WAIT_OBJECT_0);
}

static inline unsigned int __stdcall imgLoaderThread (void *ptr)
{
	//printf("@@ imgLoaderThread start %i\n", (int)GetCurrentThreadId());

	TASYNCIMGLOAD *imgLoader = (TASYNCIMGLOAD*)ptr;

	do{
		if (imgLoaderWaitForNewImageLoadSignal(imgLoader)){
			if (testApplState()){
				intptr_t data;
				while(stackPop(imgLoader->stack, &data)){
					int imgId = data;
					if (imgId){
						//printf("artManagerImagePreload preLoad: %X\n", imgId);
						/*int ret =*/ artManagerImagePreload(imgLoader->am, imgId);
						//printf("artManagerImagePreload done: %X = %i\n", imgId, ret);
					}
				}
			}
		}else{
			lSleep(10);
		}
	}while(testApplState());


	//printf("@@ imgLoaderThread end %i\n", (int)GetCurrentThreadId());

	_endthreadex(1);
	return 1;
}

int imgLoaderAddImage (TASYNCIMGLOAD *imgLoader, const int imgId)
{
	//printf("imgLoaderAddImage imgId %X\n", imgId);
	
	if (stackPush(imgLoader->stack, imgId)){
		imgLoaderNewImgSetSignal(imgLoader);
		return 1;
	}
	return 0;
}


static inline void imgLoaderFree (TASYNCIMGLOAD *imgLoader)
{
	stackDestroy(imgLoader->stack);
	my_free(imgLoader);
}

void imgLoaderShutdown (TASYNCIMGLOAD *imgLoader)
{

	imgLoaderNewImgSetSignal(imgLoader);
	WaitForSingleObject((HANDLE)imgLoader->hThread, INFINITE);
	CloseHandle((HANDLE)imgLoader->hThread);
	imgLoader->hThread = 0;
	
	CloseHandle(imgLoader->loadEvent);
	imgLoader->loadEvent = NULL;

	imgLoaderFree(imgLoader);
}

TASYNCIMGLOAD *imgLoaderNew (TARTMANAGER *am, const int initialSize)
{
	TASYNCIMGLOAD *imgLoader = my_calloc(1, sizeof(TASYNCIMGLOAD));
	if (imgLoader){
		imgLoader->am = am;
		imgLoader->stack = stackCreate(initialSize);
		imgLoader->loadEvent = CreateEvent(NULL, 0, 0, NULL);
		imgLoader->hThread = _beginthreadex(NULL, THREADSTACKSIZE, imgLoaderThread, imgLoader, 0, &imgLoader->loadLockThreadId);
	}
	return imgLoader;
}	

#if 0
// timer TIMER_IMAGECACHEFLUSH
void timer_cacheFlush (void *ctx)
{
	//printf("timer_cacheFlush\n");
	
	int page = pageGet(vp);
	if (page == PAGE_SEARCH){
		//artManagerFlush(vp->am);
		artManagerFlush(vp->im);
		pageDispatchMessage(vp->pages, PAGE_MSG_IMAGE_FLUSH, page, 0, NULL);
		
		timerSet(vp, TIMER_IMAGECACHEFLUSH, 3*60*1000);
		return;
	}
	
	//int flushed = ccLabelFlushAll(vp->cc);
	//printf("ccLabelFlushAll: flushed %i images\n", flushed);
		
	/*flushed =*/ imageManagerFlush(vp->im);
	//printf("im Flush: flushed %i images\n", flushed);
	artManagerFlush(vp->am);
	
	//invalidateShadows(vp->gui.shadow);
	pageDispatchMessage(vp->pages, PAGE_MSG_IMAGE_FLUSH, page, 0, NULL);
	timerSet(vp, TIMER_IMAGECACHEFLUSH, 3*60*1000);
}
#endif

