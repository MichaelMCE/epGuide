
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



#ifndef _IMAGEC_H_
#define _IMAGEC_H_


#define	THREADSTACKSIZE				(8*1024*1024)


#define TIMAGEMANAGER				TARTMANAGER

#define imageManagerNew				artManagerNew
#define imageManagerDelete			artManagerDelete
#define imageManagerFlush			artManagerFlush 
#define imageManagerSetPathPrefix 	artManagerSetPathPrefix  
#define imageManagerImageSetPath    artManagerImageSetPath   
#define imageManagerImageAdd        artManagerImageAdd       
#define imageManagerImageDelete     artManagerImageDelete    
#define imageManagerImageAcquire    artManagerImageAcquire   
#define imageManagerImageRelease    artManagerImageRelease   
#define imageManagerImageClone      artManagerImageClone     
#define imageManagerImageGetMetrics artManagerImageGetMetrics
#define imageManagerImageFlush      artManagerImageFlush     
#define imageManagerImagePreload	artManagerImagePreload



typedef struct {
	TSTACK *stack;		// list of imagemanager id's
	TARTMANAGER *am;
		
	uintptr_t hThread;
	HANDLE loadEvent;
	unsigned int loadLockThreadId;
}TASYNCIMGLOAD;



TASYNCIMGLOAD *imgLoaderNew (TARTMANAGER *am, const int initialSize);
void imgLoaderShutdown (TASYNCIMGLOAD *imgLoader);
int imgLoaderAddImage (TASYNCIMGLOAD *imgLoader, const int imgId);

void timer_cacheFlush (void *ctx);

#endif


