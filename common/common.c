

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

typedef struct{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
}__attribute__ ((packed))TBGRA;		// 8888

typedef struct {
	union {
		TBGRA bgra;
		uint32_t colour;
	}u;
}TCOLOUR4;

#define VLCARGLEN	(32)
typedef struct{
	size_t len;
	char str[VLCARGLEN];
}vlcstr_t;

typedef struct{
    int argc;
    int enqueue;
    vlcstr_t arg[2];
}vlc_ipc_data_t;

#define BLUR_MAXOFFSET (16)
#define BLUR_MAXOFFSET_HALF (BLUR_MAXOFFSET/2)








void com_setTargetRate (void *ctx, const double fps)
{
	// do something
}

void com_renderSignalUpdate (void *ctx)
{
	// do something
}

double com_getTime (void *ctx)
{
	TCC *cc = (TCC*)ctx;

	uint64_t t1 = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((double)((uint64_t)(t1 - cc->timer.tStart) * cc->timer.resolution) * 1000.0);
}

void com_getTimeInit (void *ctx)
{
	TCC *cc = (TCC*)ctx;
	
	QueryPerformanceCounter((LARGE_INTEGER*)&cc->timer.tStart);
	QueryPerformanceFrequency((LARGE_INTEGER *)&cc->timer.freq);
	cc->timer.resolution = 1.0 / (double)cc->timer.freq;
}

/*uint64_t com_getTickCount ()
{
	return (uint64_t)GetTickCount64();
}*/

// returns 0 if target lies within, 1 if not
static inline int checkbounds (const TFRAME *const frm, const int x, const int y)
{
	if (x < 0 || x >= frm->width || y >= frm->height || y < 0)
		return 1;
	else
		return 0;
}

static inline int getPixel32_NB (const TFRAME *frm, const int x, const int y)
{
	return *(uint32_t*)(frm->pixels+((frm->pitch*y)+(x<<2)));
}

static inline int getPixel32 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return *(uint32_t*)(frm->pixels+((frm->pitch*y)+(x<<2)));
	else
		return 0;
}

static inline int getPixel32a_BL (TFRAME *frame, double x, double y)
{

	#define PSEUDO_FLOOR( V ) ((V) >= 0 ? (int)(V) : (int)((V) - 1))
	
	const int width = frame->width;
	const int height = frame->height;
	const int pitch = frame->pitch;
	const int spp = 4;
		
	//if (y < 0) y = 0;
	//if (x < 0) x = 0;

	double x1 = PSEUDO_FLOOR(x);
	double x2 = x1+1.0;
	
	if (x2 >= width) {
		x = x2 = (double)width-1.0;
		x1 = x2 - 1;
	}
	const int wx1 = (int)((x2 - x)*256.0);
	const int wx2 = 256-wx1;
	double y1 = PSEUDO_FLOOR(y);
	double y2 = y1+1.0;
	
	if (y2 >= height) {
		y = y2 = height-1.0;
		y1 = y2 - 1;
	}
	const int wy1 = (int)(256*(y2 - y));
	const int wy2 = 256 - wy1;
	const int wx1y1 = wx1*wy1;
	const int wx2y1 = wx2*wy1;
	const int wx1y2 = wx1*wy2;
	const int wx2y2 = wx2*wy2;

	const unsigned char *px1y1 = &frame->pixels[pitch * (int)y1 + spp * (int)x1];
	const unsigned char *px2y1 = px1y1 + spp;
	const unsigned char *px1y2 = px1y1 + pitch;
	const unsigned char *px2y2 = px1y1 + pitch+spp;

	const TCOLOUR4 *restrict cx1y1 = (TCOLOUR4*)px1y1;
	const TCOLOUR4 *restrict cx2y1 = (TCOLOUR4*)px2y1;
	const TCOLOUR4 *restrict cx1y2 = (TCOLOUR4*)px1y2;
	const TCOLOUR4 *restrict cx2y2 = (TCOLOUR4*)px2y2;
	
	TCOLOUR4 c;
	c.u.bgra.r = (wx1y1 * cx1y1->u.bgra.r + wx2y1 * cx2y1->u.bgra.r + wx1y2 * cx1y2->u.bgra.r + wx2y2 * cx2y2->u.bgra.r + 32768) / 65536;
	c.u.bgra.g = (wx1y1 * cx1y1->u.bgra.g + wx2y1 * cx2y1->u.bgra.g + wx1y2 * cx1y2->u.bgra.g + wx2y2 * cx2y2->u.bgra.g + 32768) / 65536;
	c.u.bgra.b = (wx1y1 * cx1y1->u.bgra.b + wx2y1 * cx2y1->u.bgra.b + wx1y2 * cx1y2->u.bgra.b + wx2y2 * cx2y2->u.bgra.b + 32768) / 65536;
	c.u.bgra.a = (wx1y1 * cx1y1->u.bgra.a + wx2y1 * cx2y1->u.bgra.a + wx1y2 * cx1y2->u.bgra.a + wx2y2 * cx2y2->u.bgra.a + 32768) / 65536;
	return c.u.colour;
}

static inline int ablend (const uint32_t des, const uint32_t src)
{
	const uint32_t alpha = (src&0xFF000000)>>24;
	//if (alpha == 0xFF) return src;

	const uint32_t odds2 = (src>>8) & 0xFF00FF;
	const uint32_t odds1 = (des>>8) & 0xFF00FF;
	const uint32_t evens1 = des & 0xFF00FF;
	const uint32_t evens2 = src & 0xFF00FF;
	const uint32_t evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const uint32_t oddRes = ((odds2-odds1)*alpha + (odds1<<8)) &0xFF00FF00;
	return (evenRes + oddRes);
}


static inline void setPixel32 (const TFRAME *restrict frm, const int x, const int y, const int value)
{
	//if (!checkbounds(frm, x, y))
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = value;
}


static inline void setPixel32a (const TFRAME *restrict frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		 int *des = (int32_t*)(frm->pixels+((y*frm->pitch)+(x<<2)));
		*des = ablend(*des, value);
	}
}


void com_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	 
/*	if (from->hw != to->hw){
		int w = (x2 - x1);
		int h = (y2 - y1);
		printf("copyArea from %p %i %i %i %i %i %i (%i/%i)\n", from, dx, dy, x1, y1, x2, y2, w, h);
		//assert(from->hw == to->hw);
		return;
	}
*/	
	 
	if (dx < 0){
		x1 += abs(dx);
		dx = 0;
	}
	if (dy < 0){
		y1 += abs(dy);
		dy = 0;
	}
	
	if (x2 >= from->width) x2 = from->width-1;
	if (y2 >= from->height) y2 = from->height-1;
	
	const int w = (x2-x1)+1;
	if (dx+w >= to->width) x2 -= (dx+w - to->width);
	const int h = (y2-y1)+1;
	if (dy+h >= to->height) y2 -= (dy+h - to->height);


	#define getPixelAddress(f,x,y)	((f)->pixels+(((y)*(f)->pitch)+((x)<<2)))

	for (int y = y1; y <= y2; y++, dy++){
		int *psrc = (int*)getPixelAddress(from, 0, y);
		int *pdes = (int*)getPixelAddress(to, 0, dy);
		int xx = dx;

		__asm__("prefetch 64(%0)" :: "r" (psrc) : "memory");
		__asm__("prefetch 64(%0)" :: "wr" (pdes) : "memory");
		//__builtin_prefetch(psrc, 0, 1);
		//__builtin_prefetch(pdes, 1, 1);
		

		for (int x = x1; x <= x2; x++,xx++)
			pdes[xx] = ablend(pdes[xx], psrc[x]);
	}
}

void com_copyAreaNoBlend (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	if (dx < 0){
		x1 += abs(dx);
		dx = 0;
	}
	if (dy < 0){
		y1 += abs(dy);
		dy = 0;
	}
	if (x2 > from->width-1) x2 = from->width-1;
	if (y2 > from->height-1) y2 = from->height-1;
	
	const int w = (x2-x1)+1;
	if (dx+w >= to->width) x2 -= (dx+w - to->width);
	const int h = (y2-y1)+1;
	if (dy+h >= to->height) y2 -= (dy+h - to->height);
	
	int xx;
	int *restrict psrc; int *restrict pdes;
	 
	for (int y = y1; y <= y2; y++, dy++){
		psrc = lGetPixelAddress(from, 0, y);
		pdes = lGetPixelAddress(to, 0, dy);
		xx = dx;
		for (int x = x1; x <= x2; x++,xx++)
			pdes[xx] = psrc[x];
	}
}

void com_copyAreaScaled (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height)
{
	const double scalex = dest_width / (double)src_width;
	const double scaley = dest_height / (double)src_height;
	const double dx = 1.0 / scalex;
	const double dy = 1.0 / scaley;
	double y2 = src_y;
	const int src_x2 = dest_x + dest_width;
	
	
	for (int y = dest_y; y < dest_y + dest_height; y++){
		y2 += dy;
		double x2 = src_x;
		
		for (int x = dest_x; x < src_x2; x++){
			int col = getPixel32a_BL(from, x2, y2);
			setPixel32(to, x, y, col);
			x2 += dx;
		}
	}
}

static inline int UTF8ToUTF16 (const char *in, const size_t ilen, wchar_t *out, size_t olen)
{
	LPWSTR abuf = NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, in, ilen, NULL, 0);
	if (len > 0)
		abuf = out;
	else
		return 0;

	int ret = MultiByteToWideChar(CP_UTF8, 0, in, ilen, abuf, sizeof(wchar_t)*(len+1));
	if (ret > 0 && ret <= olen){
		out[ret] = 0;
		return ret;	
	}
	return 0;
}

static inline int UTF16ToUTF8 (const wchar_t *in, const size_t ilen, char *out, size_t olen)
{
	LPSTR abuf = NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, in, ilen, NULL, 0,  0, 0);
	if (len > 0)
		abuf = out;
	else
		return 0;

	int ret = WideCharToMultiByte(CP_UTF8, 0, in, ilen, abuf, len,  0, 0);
	if (ret > 0 && ret <= olen){
		out[ret] = 0;
	}	

	return (ret > 0);
}
static inline char *UTF16ToUTF8Alloc (const wchar_t *in, const size_t ilen)
{
	const size_t olen = 8 * (ilen+2);
	char *out = my_malloc(1+olen+sizeof(char));
	if (out){
		if (UTF16ToUTF8(in, ilen, out, olen)){
			//out = my_realloc(out, strlen(out)+1);
		}else{
			my_free(out);
			return NULL;
		}
	}
	return out;
}


static inline wchar_t *UTF8ToUTF16Alloc (const char *in, const size_t ilen)
{
	const size_t olen = sizeof(wchar_t) * (ilen+2);
	wchar_t *out = my_malloc(2+olen+sizeof(wchar_t));
	if (out){
		if (UTF8ToUTF16(in, ilen, out, olen)){
			//out = my_realloc(out, (wcslen(out)+1) * sizeof(wchar_t));
		}else{
			my_free(out);
			return NULL;
		}
	}
	return out;
}



wchar_t *com_converttow (const char *utf8)
{
	wchar_t *out = NULL;
	if (utf8 && *utf8){
		out = UTF8ToUTF16Alloc(utf8, strlen(utf8));
		if (!out){
			//out = my_wcsdup(L"-");
			//if (!out){
				printf("no memory, bailing\n");
				abort();
			//}
		}else{
			//wprintf(L"%p '%s'\n", out, out);
		}
	}
	return out;
}

char *com_convertto8 (const wchar_t *wide)
{
	char *out = NULL;
	if (wide && *wide){
		out = UTF16ToUTF8Alloc(wide, wcslen(wide));
		if (!out){
			//out = my_strdup("-");
			//if (!out){
				printf("no memory, bailing\n");
				abort();
			//}
		}
	}
	return out;
}


// 32 bit magic FNV-1a prime
// Fowler/Noll/Vo hash
#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)0x811c9dc5)

static inline uint32_t fnv_32a_buf (const void *buf, const size_t len, uint32_t hval)
{
    unsigned char *restrict bp = (unsigned char*)buf;	/* start of buffer */
    unsigned char *restrict be = bp + len;				/* beyond end of buffer */

    /*
     * FNV-1a hash each octet in the buffer
     */
    while (bp < be){
		/* xor the bottom with the current octet */
		hval ^= (uint32_t)*bp++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hval *= FNV_32_PRIME;
#else
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
    }

    /* return our new hash value */
    return hval;
}

static inline uint32_t com_generateHash (const void *data, const size_t dlen)
{
	if (!dlen)
		return 0;
	else
		return fnv_32a_buf(data, dlen, FNV_32_PRIME);
}

uint32_t com_getHash (const char *path)
{
	return com_generateHash(path, strlen(path));
}

#if 1
uint32_t com_getHashW (const wchar_t *path)
{
	uint32_t hash = 0;
	char *out = UTF16ToUTF8Alloc(path, wcslen(path));
	if (out){
		hash = com_getHash(out);
		my_free(out);
	}
	return hash;
}
#endif

#if 0
void com_drawHex (TFRAME *frame, const int x, const int y, const char *font, const int var, const int colour)
{
	printf("com_drawHex(): update me to ufont..\n");
}

#elif 0

void com_drawHex (TFRAME *frame, const int x, const int y, const char *font, const int var, const int colour)
{
	int x2 = x;
	if (var < 10)		// because its faster..
		x2 += 8;
	else if (var < 100)
		x2 += 15;
	else if (var < 1000)
		x2 += 22;
	else if (var < 10000)
		x2 += 29;
	else
		x2 += 36;
		
	if (colour&0xFF000000)
		lDrawRectangleFilled(frame, x-3, y+2, x2, y+14, colour/*90<<24 | (colour&0xFFFFFF)*/);
	lPrintf(frame, x, y, font, LPRT_CPY, "%X", var);

}
#endif


void com_removeChr (char *str, const char a)
{
	char *strB = str;
	
	int slen = strlen(strB);
	for (int i = 0; i < slen; i++){
		if (strB[i] != a)
			*str++ = strB[i];
	}
	*str = 0;
}


#if 0
static inline void genShadow (TSHADOWUNDER *shadow, TFRAME *img)
{
	const int w = img->width/2;
	const int h = img->height/2;
	
	shadow->topLeft = lNewFrame(img->hw, w, h, img->bpp);
	shadow->topRight= lNewFrame(img->hw, w, h, img->bpp);
	shadow->btmLeft = lNewFrame(img->hw, w, h, img->bpp);
	shadow->btmRight= lNewFrame(img->hw, w, h, img->bpp);
	shadow->vertBarTop = lNewFrame(img->hw, 1, h/2, img->bpp);
	shadow->vertBarBtm = lNewFrame(img->hw, 1, h/2, img->bpp);
	shadow->horiBarLeft = lNewFrame(img->hw, w/2, 1, img->bpp);
	shadow->horiBarRight = lNewFrame(img->hw, w/2, 1, img->bpp);
	
	com_copyAreaNoBlend(img, shadow->topLeft, 0, 0, 0, 0, w-1, h-1);
	com_copyAreaNoBlend(img, shadow->topRight, 0, 0, w, 0, img->width-1, h-1);
	com_copyAreaNoBlend(img, shadow->btmLeft, 0, 0, 0, h, w-1, img->height-1);
	com_copyAreaNoBlend(img, shadow->btmRight, 0, 0, w, h, img->width-1, img->height-1);
	
	com_copyAreaNoBlend(img, shadow->vertBarTop, 0, 0, w, 0, w, (h/2)-1);
	com_copyAreaNoBlend(img, shadow->vertBarBtm, 0, 0, w, img->height-shadow->vertBarBtm->height, w, img->height-1);

	com_copyAreaNoBlend(img, shadow->horiBarLeft, 0, 0, 0, h, shadow->horiBarLeft->width-1, h);
	com_copyAreaNoBlend(img, shadow->horiBarRight, 0, 0, img->width-shadow->horiBarLeft->width, h, img->width-1, h);
	
	shadow->isBuilt = 1;
	
}

static inline void buildShadow (void *ctx, TSHADOWUNDER *s, const int shadow)
{
	TCC *cc = (TCC*)ctx;
	
	int shadowImg;
	if (shadow == SHADOW_BLUE)
		shadowImg = cc->imgcIds[IMGC_SHADOW_BLU];
	else if (shadow == SHADOW_GREEN)
		shadowImg = cc->imgcIds[IMGC_SHADOW_GRN];
	else/*if (shadow == SHADOW_BLACK)*/
		shadowImg = cc->imgcIds[IMGC_SHADOW_BLK];

	void *im = ccGetImageManager(cc, CC_IMAGEMANAGER_IMAGE);
	TFRAME *img = imageManagerImageAcquire(im, shadowImg);
	if (img){
		//printf("buildShadow %i %X, %i\n", shadow, shadowImg, s->isBuilt);
		genShadow(s, img);
		imageManagerImageRelease(im, shadowImg);
		imageManagerImageFlush(im, shadowImg);
	}
}

void com_drawShadowUnderlay (void *ctx, TFRAME *img, const int x, const int y, const int w, const int h, const int shadow)
{
	TCC *cc = (TCC*)ctx;
	
	TSHADOWUNDER *s = &cc->shadow[shadow];
	if (!s->isBuilt){
		buildShadow(ctx, s, shadow);
		if (!s->isBuilt) return;
	}
		
	const int sw = s->topLeft->width;
	const int sh = s->topLeft->height;
	const int offset = sw/2;
	
	// top left
	com_copyArea(s->topLeft, img, x-offset+1, y-offset+1, 0, 0, sw-1, sh-1);
	
	//top+btm bar
	const int barWidth = (offset + (w - offset - offset))-1;
	for (int i = offset+1; i < barWidth; i++)
		com_copyArea(s->vertBarTop, img, x+i, y-offset+1, 0, 0, 1, s->vertBarTop->height-1);

	// top right
	com_copyArea(s->topRight, img, (x+w)-offset-1, y-offset+1, 0, 0, sw-1, sh-1);

	const int barHeight = (offset + (h - offset - offset))-1;
	for (int i = offset+1; i < barHeight; i++)
		com_copyArea(s->horiBarLeft, img, x-offset+1, y+i, 0, 0, s->horiBarLeft->width-1, 1);
	for (int i = offset+1; i < barHeight; i++)
		com_copyArea(s->horiBarRight, img, (x+w)-1, y+i, 0, 0, s->horiBarRight->width-1, 1);
		
	// btm left
	com_copyArea(s->btmLeft, img, x-offset+1, (y+h)-offset-1, 0, 0, sw-1, sh-1);

	for (int i = offset+1; i < barWidth; i++)
		com_copyArea(s->vertBarBtm, img, x+i, (y+h)-1, 0, 0, 1, s->vertBarBtm->height-1);
		
	// btm right
	com_copyArea(s->btmRight, img, (x+w)-offset-1, (y+h)-offset-1, 0, 0, sw-1, sh-1);
}
#endif

#if 0
TFRAME *com_newImage (void *ctx, const wchar_t *filename, const int bpp)
{
	TCC *cc = (TCC*)ctx;
	
#if 0
	static volatile int ct = 0;
	static volatile size_t memused = 0;
	
	char *path = convertto8(filename);
	//printf("newImage '%s'\n", path);
	//fflush(stdout);

	const double t0 = getTime(vp);	
	TFRAME *img = lNewImage(vp->ml->hw, filename, bpp);
	if (img){
		const double t1 = getTime(vp);
		memused += img->frameSize + sizeof(TFRAME) + sizeof(void*) + sizeof(TPIXELPRIMITVES);
		
		printf("newImage %.1f (%.2f) - %i: %u %ix%i '%s'\n", t1, t1-t0, ++ct, memused/1024, img->width, img->height, path);
	}else{
		printf("newImage FAILED '%s'\n", path);
	}
	if (path)
		my_free(path);
	return img;
#else
	return lNewImage(cc->hSurfaceLib, filename, bpp);
#endif
}
#endif

void com_swipeReset (TTOUCHSWIPE *swipe)
{
  	swipe->state = SWIPE_UP;
  	swipe->adjust = 0.0;
  	swipe->decayAdjust = 0.0;
}

/*
static inline void drawImageScaled (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height)
{
	const double scaley = dest_height / (double)src_height;
	const double scalex = dest_width / (double)src_width;
	const double dx = 1.0 / scalex;
	const double dy = 1.0 / scaley;
	double y2 = src_y;
	
	for (int y = dest_y; y < dest_y + dest_height; y++){
		y2 += dy;
		double x2 = src_x;
		
		for (int x = dest_x; x < dest_x + dest_width; x++){
			int col = getPixel32_NB(from, x2, y2);
			setPixel32a(to, x, y, col);
			x2 += dx;
		}
	}
}
*/
/*
static inline void drawImageScaledB (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int dest_x1, const int dest_y1, const int dest_x2, const int dest_y2, const double scale)
{

	const double dx = (1.0 - dest_x1) / scale;
	const double dy = (1.0 - dest_y1) / scale;
	double y2 = src_y;
		
	for (int y = dest_y1; y <= dest_y2; y++){
		y2 += dy;
		double x2 = src_x;
	
		for (int x = dest_x1; x <= dest_x2; x++){
			setPixel32(to, x, y, getPixel32(from, x2, y2));
			x2 += dx;
		}
	}
}

void com_drawImageScaledCenter (TFRAME *img, TFRAME *dest, const double scale, const double offsetX, const double offsetY)
{
	double swidth = (img->width / scale);
	double sheight = (img->height / scale);
	double cx = (dest->width / 2.0) - (offsetX/scale);
	double cy = (dest->height / 2.0) - (offsetY/scale);
	double x = cx - (swidth/2.0);
	double y = cy - (sheight/2.0);
	//printf("drawISC %.2f %.2f %.5f, %.2f %.2f, %.2f %.2f\n", x, y, scale, swidth, sheight, offsetX, offsetY);
		
	drawImageScaledB(img, dest, x, y, 0, 0, dest->width-1, dest->height-1, scale);
}
*/

#if 1
void com_drawImageScaledOpacity (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height, const float opacityAlpha, const float opacityRGB)
{

	const float scalex = dest_width / (float)src_width;
	const float scaley = dest_height / (float)src_height;
	
	if (opacityAlpha < 0.960f/* || from->udata_int == SHELF_IMGTYPE_NOART*/){

		float opacity = opacityAlpha;
		if (opacity > 1.0f) opacity = 1.0f;
		const float dx = (1.0f) / scalex;
		
		for (int y = dest_y; y < dest_y + dest_height; y++){
			float y2 = src_y + (y-dest_y) / scaley;
			float x2 = src_x;
			for (int x = dest_x; x < dest_x + dest_width; x++){
				//int col = getPixel32a_BL(from, x2, y2);
				int col = getPixel32_NB(from, x2, y2);
				int a = ((col>>24)&0xFF) * opacity;
				int r = ((col>>16)&0xFF) * opacityRGB;
				int g = ((col>>8)&0xFF) * opacityRGB;
				int b =  (col&0xFF) * opacityRGB;
				setPixel32a(to, x, y, (a<<24) | (r<<16) | (g<<8) | b);
				x2 += dx;
			}
		}
	}else{
	
#if 1
			// fastest. plain copy - no resampling or scaling
		fastFrameCopy(from, to, dest_x, dest_y);

#elif 0		// with bilinear sampling, high quality but slower
		for (int y = dest_y; y < dest_y + dest_height; y++){
			int y2 = src_y + (y-dest_y) / scaley;
			for (int x = dest_x; x < dest_x + dest_width; x++){
				int x2 = src_x + (x-dest_x) / scalex;
				setPixel32a(to, x, y, getPixel32a_BL(from, x2, y2));
			}
		}
#else		// faster, with scaling, but doesn't look so great
		for (int y = dest_y; y < dest_y + dest_height; y++){
			y2 = src_y + (y-dest_y) / scaley;
			int *psrc = lGetPixelAddress(from, src_x, y2);
			
			for (int x = dest_x; x < dest_x + dest_width; x++){
				x2 = (x-dest_x) / scalex;
				setPixel32a(to, x, y, psrc[(int)x2]);
			}
		}
#endif
	}
}
#endif

/*
void com_drawImageOpacity (TFRAME *from, TFRAME *to, const int dest_x, const int dest_y, double opacity)
{
	
	const int dest_width = from->width;
	const int dest_height = from->height;

	if (opacity > 1.0) opacity = 1.0;
	const double opacitycol = opacity * 0.65;

	for (int y = dest_y; y < dest_y + dest_height; y++){
		int y2 = y - dest_y;

		for (int x = dest_x; x < dest_x + dest_width; x++){
			int x2 = x - dest_x;
			int col = getPixel32_NB(from, x2, y2);
				
			int a = ((col>>24)&0xFF) * opacity;
			int r = ((col>>16)&0xFF) * opacitycol;
			int g = ((col>>8)&0xFF) * opacitycol;
			int b =  (col&0xFF) * opacitycol;
				
			setPixel32a(to, x, y, (a<<24) | (r<<16) | (g<<8) | b);
		}
	}
}
*/


void com_imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h)
{
	const int fg_sar_num = 1; const int fg_sar_den = 1;
	const int bg_sar_den = 1; const int bg_sar_num = 1;

	if (fg_w < 1 || fg_w > 8191) fg_w = bg_w;
	if (fg_h < 1 || fg_h > 8191) fg_h = bg_h;
	*w = bg_w;
	*h = (bg_w * fg_h * fg_sar_den * bg_sar_num) / (float)(fg_w * fg_sar_num * bg_sar_den);
	if (*h > bg_h){
		*w = (bg_h * fg_w * fg_sar_num * bg_sar_den) / (float)(fg_h * fg_sar_den * bg_sar_num);
		*h = bg_h;
	}
}

#if 0
TFRAME *com_newStringEx (void *ctx, TMETRICS *metrics, const int bpp, const int flags, const char *font, const char *text, const int maxW, const int nsex_flags)
{
	printf("com_newStringEx(): update me to ufont..\n");
	return NULL;
}

TFRAME *com_newStringEx2 (void *ctx, const TMETRICS *metrics, const int bpp, const int flags, const char *font, const char *text, int x, int y, const int maxW, const int maxH, const int nsex_flags)
{
	printf("com_newStringEx2(): update me to ufont..\n");
	return NULL;
}


#elif 0
TFRAME *com_newStringEx (void *ctx, TMETRICS *metrics, const int bpp, const int flags, const int font, const char *text, const int maxW, const int nsex_flags)
{
	TCC *cc = (TCC*)ctx;
	
	if (!*text){
		//printf("newStringEx: invalid string\n");
		return NULL;
	}

	metrics->width = maxW;
	TFRAME *str = lNewStringEx(cc->hSurfaceLib, metrics, bpp, flags|PF_EXTRA|PF_IGNOREFORMATTING|PF_CLIPDRAW, font, text);
	//printf("newStringEx %i,%i %i '%s'\n", str->width, str->height, maxW, text);
	
	if (str){
		if (str->width > maxW){
			TFRAME *tmp = lNewFrame(str->hw, maxW, str->height, str->bpp);
			if (tmp){
				if (nsex_flags&NSEX_LEFT)
					com_copyAreaNoBlend(str, tmp, 0, 0, 0, 0, tmp->width-1, tmp->height-1);
				else if (nsex_flags&NSEX_RIGHT)
					com_copyAreaNoBlend(str, tmp, 0, 0, str->width-maxW, 0, str->width-1, str->height-1);
					
				lDeleteFrame(str);
				str = tmp;
			}
		}
		//lDrawRectangle(str, 0, 0, str->width-1, str->height-1, 0xFF0000FF);
		lDrawRectangle(str, 0, 0, str->width-1, str->height-1, 0xFFFF);
	}
	return str;
}

TFRAME *com_newStringEx2 (void *ctx, const TMETRICS *metrics, const int bpp, const int flags, const int font, const char *text, int x, int y, const int maxW, const int maxH, const int nsex_flags)
{
	TCC *cc = (TCC*)ctx;

//	printf("newStringEx2: #%s#  %i\n", text, maxW);
	
	if (!*text){
		//printf("newStringEx: invalid string\n");
		return NULL;
	}
	
	TFRAME *str = lNewStringEx(cc->hSurfaceLib, metrics, bpp, flags|PF_EXTRA|PF_IGNOREFORMATTING|PF_CLIPDRAW/*|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP*/, font, text);
	//printf("newStringEx2 %i,%i %i '%s'\n", str->width, str->height, maxW, text);
	if (!str) return NULL;
	
	//lSaveImage(str, L"tmp.png", IMG_PNG|IMG_KEEPALPHA, 0, 0);
	
	int w = MIN(str->width, maxW);
	int h = MIN(str->height, maxH);
	
	if (y < 0){
		h += y;
		y = abs(y);
	}else{
		y = 0;
	}
	if (x < 0){
		w += x;
		x = abs(x);
	}else{
		x = 0;
	}	
		
	if (y > 0 || w != str->width || h != str->height){
		TFRAME *tmp = lNewFrame(str->hw, w, h, str->bpp);
		if (tmp){
			if (nsex_flags&NSEX_LEFT)
				com_copyAreaNoBlend(str, tmp, 0, 0, x, y, str->width-1, str->height-1);
			else if (nsex_flags&NSEX_RIGHT)
				com_copyAreaNoBlend(str, tmp, 0, 0, str->width-w, 0, str->width-1, tmp->height-1);
				
			lDeleteFrame(str);
			str = tmp;
		}
	}
	
	//lDrawRectangle(str, 0, 0, str->width-1, str->height-1, 0xFF0000FF);
	lDrawRectangle(str, 0, 0, str->width-1, str->height-1, 0xF800);
	
	return str;
}
#endif 

#if 0
wchar_t *com_buildSkinDEx (void *ctx, wchar_t *buffer, wchar_t *dir, wchar_t *file)
{
	//TCC *cc = (TCC*)ctx;

	*buffer = 0;

	/*wchar_t *skin = NULL;
	if (!*vp->gui.skin.folder){
		settingsGetW(vp, "skin.folder", &skin);
		
		if (skin){
			wcscpy(vp->gui.skin.folder, skin);
			my_free(skin);
		}
	}
	skin = vp->gui.skin.folder;
	my_swprintf(buffer, L"%ls/%ls/%ls/%ls", SKINDROOT, skin, dir, file);
	*/
	
	my_swprintf(buffer, L"%ls/%ls/%ls", SKINDROOT, dir, file);
	
	//wprintf(L"skinEx: #%s#\n", buffer);
	
	return buffer;
}

wchar_t *com_buildSkinD (void *ctx, wchar_t *buffer, wchar_t *file)
{
	//TCC *cc = (TCC*)ctx;
	
	*buffer = 0;
	//wchar_t *skin = vp->gui.skin.dir;
	
	/*wchar_t *skin = NULL;
	if (!*vp->gui.skin.folder){
		settingsGetW(vp, "skin.folder", &skin);
		
		if (skin){
			wcscpy(vp->gui.skin.folder, skin);
			my_free(skin);
		}
	}
	skin = vp->gui.skin.folder;
	my_swprintf(buffer, L"%ls/%ls/%ls", SKINDROOT, skin, file);
	*/
	
	my_swprintf(buffer, L"%ls/%ls", SKINDROOT, file);
	
	//printf("skinEx: #%s#\n", buffer);
	
	return buffer;
}
#endif


/*
void com_drawShadowedImage (TFRAME *img1, TFRAME *dest, const int x, const int y, uint32_t colour, const int radius, const int offsetX, const int offsetY)
{
	
	TFRAME *img2 = lNewFrame(img1->hw, img1->width+BLUR_MAXOFFSET, img1->height+BLUR_MAXOFFSET, LFRM_BPP_32A);
	if (!img2) return;

	// set shadow colour
	colour &= 0xFFFFFF;
	uint32_t *src = lGetPixelAddress(img1, 0, 0);
	uint32_t *des = lGetPixelAddress(img2, 0, 0);
	int tpixels = img2->height * img2->width;
	while(tpixels--) *des++ = colour;
	
	// create shadow mask
	des = (uint32_t*)img2->pixels;
	for (int y = BLUR_MAXOFFSET_HALF; y < img1->height+BLUR_MAXOFFSET_HALF; y++){
		des = lGetPixelAddress(img2, BLUR_MAXOFFSET_HALF, y);
		for (int x = BLUR_MAXOFFSET_HALF; x < img1->width+BLUR_MAXOFFSET_HALF; x++)
			*des++ = ((*src++)&0xFF000000) | colour;
	}

	lBlurImage(img2, lBLUR_STACKFAST, radius);
	//lBlurImage(img2, lBLUR_GAUSSIAN, radius);
	
	com_drawImage(img2, dest, x-BLUR_MAXOFFSET_HALF+offsetX, y-BLUR_MAXOFFSET_HALF+offsetY, img2->width-1, img2->height-1);
	com_drawImage(img1, dest, x, y, img1->width-1, img1->height-1);
	lDeleteFrame(img2);
}
*/
/*
TFRAME *com_drawShadowedImageCreateBlurMask (TFRAME *img1, uint32_t colour, const int radius)
{
	TFRAME *img2 = lNewFrame(img1->hw, img1->width+BLUR_MAXOFFSET, img1->height+BLUR_MAXOFFSET, LFRM_BPP_32A);
	if (!img2) return NULL;
	
	colour &= 0xFFFFFF;
	uint32_t *src = lGetPixelAddress(img1, 0, 0);
	uint32_t *des = lGetPixelAddress(img2, 0, 0);
	int tpixels = img2->height * img2->width;
	while(tpixels--) *des++ = colour;
	
	
	des = (uint32_t*)img2->pixels;
	for (int y = BLUR_MAXOFFSET_HALF; y < img1->height+BLUR_MAXOFFSET_HALF; y++){
		des = lGetPixelAddress(img2, BLUR_MAXOFFSET_HALF, y);
		for (int x = BLUR_MAXOFFSET_HALF; x < img1->width+BLUR_MAXOFFSET_HALF; x++)
			*des++ = ((*src++)&0xFF000000) | colour;
	}

	lBlurImage(img2, lBLUR_STACKFAST, radius);
	//lBlurImage(img2, lBLUR_HUHTANEN, radius);
	//lBlurImage(img2, lBLUR_GAUSSIAN, radius);
	return img2;
}
*/
/*
void com_drawShadowedImageComputed (TFRAME *img1, TFRAME *dest, TFRAME *img2, const int x, const int y, const int offsetX, const int offsetY)
{
	com_drawImage(img2, dest, x-BLUR_MAXOFFSET_HALF+offsetX, y-BLUR_MAXOFFSET_HALF+offsetY, img2->width-1, img2->height-1);
	com_drawImage(img1, dest, x, y, img1->width-1, img1->height-1);
}

void com_drawShadowedImageComputedScaled (TFRAME *img1, TFRAME *dest, TFRAME *img2, const int x, const int y, const int offsetX, const int offsetY, const float scale)
{
	drawImageScaled(img2, dest, 0, 0, img2->width-1, img2->height-1, x-(BLUR_MAXOFFSET_HALF*scale)+(offsetX*scale), y-(BLUR_MAXOFFSET_HALF*scale)+(offsetY*scale), img2->width*scale, img2->height*scale);
	drawImageScaled(img1, dest, 0, 0, img1->width-1, img1->height-1, x, y, img1->width*scale, img1->height*scale);
}*/

void com_drawShadowedImageAlpha (TFRAME *img1, TFRAME *dest, const int x, const int y, uint32_t colour, const int radius, const int offsetX, const int offsetY, const double alpha)
{
	TFRAME *img2 = lNewFrame(img1->hw, img1->width+BLUR_MAXOFFSET, img1->height+BLUR_MAXOFFSET, LFRM_BPP_32A);
	if (!img2) return;
	

	// set shadow colour
	colour &= 0xFFFFFF;
	uint32_t *src = lGetPixelAddress(img1, 0, 0);
	uint32_t *des = lGetPixelAddress(img2, 0, 0);
	int tpixels = img2->height * img2->width;
	while(tpixels--) *des++ = colour;
	
	// create shadow mask
	des = (uint32_t*)img2->pixels;
	for (int y = BLUR_MAXOFFSET_HALF; y < img1->height+BLUR_MAXOFFSET_HALF; y++){
		des = lGetPixelAddress(img2, BLUR_MAXOFFSET_HALF, y);
		for (int x = BLUR_MAXOFFSET_HALF; x < img1->width+BLUR_MAXOFFSET_HALF; x++){
			float a = (*src>>24) * alpha;
			*des++ = ((*src++)&(int)a<<24) | colour;
		}
	}

	lBlurImage(img2, lBLUR_STACKFAST, radius);
	//lBlurImage(img2, lBLUR_GAUSSIAN, radius);
	
	com_drawImage(img2, dest, x-BLUR_MAXOFFSET_HALF+offsetX, y-BLUR_MAXOFFSET_HALF+offsetY, img2->width-1, img2->height-1);
	com_drawImage(img1, dest, x, y, img1->width-1, img1->height-1);
	lDeleteFrame(img2);
}

#if 0
void com_outlineTextEnable (void *ctx, const int colour)
{
	printf("com_outlineTextEnable(): update me to ufont..\n");
	/*TCC *cc = (TCC*)ctx;
	lSetFilterAttribute(cc->hSurfaceLib, LTR_OUTLINE2, 0, colour);
	lSetRenderEffect(cc->hSurfaceLib, LTR_OUTLINE2);*/
}

void com_outlineTextDisable (void *ctx)
{
	printf("com_outlineTextDisable(): update me to ufont..\n");
	/*TCC *cc = (TCC*)ctx;
	lSetRenderEffect(cc->hSurfaceLib, LTR_DEFAULT);*/
}

void com_shadowTextEnable (void *ctx, const int colour, const unsigned char trans)
{
	printf("com_shadowTextEnable(): update me to ufont..\n");
	/*TCC *cc = (TCC*)ctx;
	lSetRenderEffect(cc->hSurfaceLib, LTR_SHADOW);
	// set direction to South-East, shadow thickness to 5, offset by 1 pixel(s) and transparency to trans
	lSetFilterAttribute(cc->hSurfaceLib, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E|LTRA_SHADOW_N|LTRA_SHADOW_W | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(0) | LTRA_SHADOW_TR(trans));
	lSetFilterAttribute(cc->hSurfaceLib, LTR_SHADOW, 1, colour);
	*/
}

void com_shadowTextDisable (void *ctx)
{
	printf("com_shadowTextDisable(): update me to ufont..\n");
	
	//TCC *cc = (TCC*)ctx;
	//lSetRenderEffect(cc->hSurfaceLib, LTR_DEFAULT);
}

#endif


// don't allow Windows drag'n'drop to screw with the path and abililty to find [data] files
static inline void resetCurrentDirectory ()
{
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	my_swprintf(szPath, L"%ls%ls", drive, dir);
	
	SetCurrentDirectoryW(szPath);
}

int com_processCreateW (const wchar_t *cmdLine)
{
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);

	resetCurrentDirectory();
	CreateProcessW(NULL, (wchar_t*)cmdLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

	//printf("processCreateW: %p %p %i\n", pi.hProcess, pi.hThread, (int)pi.dwProcessId);

	/*printf("waiting..\n");
	int ret = (int)WaitForSingleObject(pi.hThread, INFINITE);
	printf("waiting.. done %i\n", ret == WAIT_OBJECT_0);
	*/
	return pi.dwProcessId;
}

int com_processCreate (const char *cmdLine)
{
	int pid = 0;
	wchar_t *path = com_converttow(cmdLine);
	if (path){
		pid = com_processCreateW(path);
		my_free(path);
	}

	return pid;
}


void com_frame16ToBuffer24 (TFRAME *frm, uint8_t *buffer)
{
	uint16_t *pp = (uint16_t*)frm->pixels;

	for (int y = 0; y < frm->height; y++){
		uint16_t *p = &pp[y*frm->width];
		
		for (int x = 0; x < frm->width; x++, p++){
			buffer[(x*3)+2] = (*p&0xF800) >> 8;
			buffer[(x*3)+1] = (*p&0x07E0) >> 3;
			buffer[(x*3)+0] = (*p&0x001F) << 3;
		}
		buffer += (frm->width*3);
	}
}

void com_frame16ToBuffer32 (TFRAME *frm, uint8_t *buffer)
{
	uint16_t *pp = (uint16_t*)frm->pixels;
	const int pitch = frm->width*4;
	
	
	for (int y = 0; y < frm->height; y++){
		uint16_t *p = &pp[y*frm->width];
		
		for (int x = 0; x < frm->width; x++, p++){
			buffer[(x<<2)+3] = 0xFF;
			buffer[(x<<2)+2] = (*p&0xF800) >> 8;
			buffer[(x<<2)+1] = (*p&0x07E0) >> 3;
			buffer[(x<<2)+0] = (*p&0x001F) << 3;
		}
		buffer += pitch;
	}
}

static inline int CALLBACK enumWindowsProc (HWND hwnd, LPARAM lParam)
{
    HWND *wnd = (HWND*)lParam;
    char name[1024];
    
    GetWindowText(hwnd, name, sizeof(name)-1);
    if (!strncmp(name, "VLC ipc ", 8)){
   	    *wnd = hwnd;
   	    return false;
    }
    return true;
}

HWND com_getVLCIPCHandle ()
{
	HWND hwnd = 0;
	EnumWindows(enumWindowsProc, (LPARAM)&hwnd);
	return hwnd;
}

void com_sendToVLC (HWND ipcwindow, const uint32_t transponderFreq, const int32_t channelId)
{
	//printf("sendToVLC\n");
	
	COPYDATASTRUCT wm_data = {0};
	vlc_ipc_data_t *data = my_calloc(1, sizeof(vlc_ipc_data_t));
		
	data->argc = 2;
	data->enqueue = 0;
	data->arg[0].len = data->arg[1].len = VLCARGLEN;
	
	my_snprintf(data->arg[0].str, VLCARGLEN-1, VLC_MRL"%i", transponderFreq);
	my_snprintf(data->arg[1].str, VLCARGLEN-1, ":program=%i", channelId);
		
	wm_data.dwData = 0;
	wm_data.cbData = sizeof(vlc_ipc_data_t);
	wm_data.lpData = data;
		
	//printf("ipcwindow %p %i\n", ipcwindow, (int)wm_data.cbData);

#if 1
	if (ipcwindow)
		SendMessage(ipcwindow, WM_COPYDATA, 0, (LPARAM)&wm_data);
#endif
	my_free(data);
}

int com_startVLC (const uint32_t transponderFreq, const int32_t channelId)
{
	//printf("startVLC\n");
	
	char path[4096];

	my_snprintf(path, sizeof(path)-1, VLC_PATH" "VLC_MRL"%i :program=%i "VLC_ARGS, (int)transponderFreq, (int)channelId);
	return com_processCreate(path);
}

void com_sendGlobalHotkey (const int modA, const int modB, const char key)
{
	if (modA) keybd_event(modA, 0, 0, 0);
	if (modB) keybd_event(modB, 0, 0, 0);
	keybd_event(key, 0, 0, 0);
	
	Sleep(15);
	
	keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
	if (modB) keybd_event(modB, 0, KEYEVENTF_KEYUP, 0);
	if (modA) keybd_event(modA, 0, KEYEVENTF_KEYUP, 0);
}

