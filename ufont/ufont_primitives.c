
// libmylcd
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.

//#pragma GCC optimize ("-O2")


#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <math.h>
#include "ufont.h"
#include "ufont_primitives.h"


#if ENABLE_PRIMITIVES


#ifndef __WIN32__
static inline int abs (const int v)
{
	if (v < 0)
		return -v;
	else
		return v;
}
#endif


static inline _ufont_surface_t *fontGetRenderSurface (_ufont_t *font)
{
	return font->render.surface;
}

// clip rect to the target display space
static inline void bindRectangle (_ufont_surface_t *surface, int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 > surface->width-1)
		*x1 = surface->width-1;
	else if (*x1 < 0)
		*x1 = 0;
		
	if (*x2 > surface->width-1)
		*x2 = surface->width-1;
	else if (*x2 < 0)
		*x2 = 0;

	if (*y1 > surface->height-1)
		*y1 = surface->height-1;
	else if (*y1 < 0)
		*y1 = 0;
		
	if (*y2 > surface->height-1)
		*y2 = surface->height-1;
	else if (*y2 < 0)
		*y2 = 0;
}

/*
// set without bound checking
static inline void setPixel1_nb (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	*(surface->pixels+((y*pitch)+(x>>3))) |= (1<<(x&7));
}

static inline void unsetPixel1_nb (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	*(surface->pixels+((y*pitch)+(x>>3))) &= ~(1<<(x&7));
}

static inline void unsetPixel1_bc (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		*(surface->pixels+((y*pitch)+(x>>3))) &= ~(1<<(x&7));
}

// set with bound checking
static inline void setPixel1_bc (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		setPixel1_nb(surface, pitch, x, y);
}

static inline void setPixel1 (uint8_t *pixels, const int x)
{
	*(pixels+(x>>3)) |= (1<<(x&7));
}

static inline int getPixel1 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(pixels+((y * pitch)+(x>>3))) >>(x&7)&0x01;
}

static inline uint8_t getPixels8 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(pixels+((y * pitch)+(x>>3)));
}

static inline uint32_t getPixels32 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(uint32_t*)(pixels+((y * pitch)+(x>>3)));
}

static inline void setPixels8 (uint8_t *pixels, const int pitch, const int x, const int y, uint8_t data)
{
	*(pixels+((y * pitch)+(x>>3))) = data;
}

static inline void setPixel16_bc (_ufont_surface_t *surface, const int x, const int y, const uint16_t colour)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		*((uint16_t*)(surface->pixels+((surface->width<<1)*y)+(x<<1))) = colour;
}

static inline void setPixel16_nb (_ufont_surface_t *surface, const int x, const int y, const uint16_t colour)
{
	*((uint16_t*)(surface->pixels+((surface->width<<1)*y)+(x<<1))) = colour;
}

static inline uint16_t getPixel16 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *((uint16_t*)(pixels+(pitch*y)+(x<<1)));
}
*/

static inline int findRegion (_ufont_surface_t *surface, const int x, const int y)
{
	int code = 0;
	
	if (y >= surface->height)
		code |= 1; //top
	else if (y < 0)
		code |= 2; //bottom
		
	if (x >= surface->width)
		code |= 4; //right
	else if (x < 0)
		code |= 8; //left

	return code;
}

static inline int clipLine (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, int *x3, int *y3, int *x4, int *y4)
{
	
	int accept = 0, done = 0;
	int code1 = findRegion(surface, x1, y1); //the region outcodes for the endpoints
	int code2 = findRegion(surface, x2, y2);
	
	const int h = surface->height;
	const int w = surface->width;

	
	do{  //In theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
	  if (!(code1|code2)){
	  	accept = done = 1;  //accept because both endpoints are in screen or on the border, trivial accept
	  }else if (code1&code2){
	  	done = 1; //the line isn't visible on screen, trivial reject
	  }else{  //if no trivial reject or accept, continue the loop
	
	    int x, y;
	    int codeout = code1 ? code1 : code2;
	    if (codeout&1){			//top
	      x = x1 + (x2 - x1) * (h - y1) / (y2 - y1);
	      y = h - 1;
	    }else if (codeout & 2){	//bottom
	      x = x1 + (x2 - x1) * -y1 / (y2 - y1);
	      y = 0;
	    }else if (codeout & 4){	//right
	      y = y1 + (y2 - y1) * (w - x1) / (x2 - x1);
	      x = w - 1;
	    }else{					//left
	      y = y1 + (y2 - y1) * -x1 / (x2 - x1);
	      x = 0;
	    }
	    
	    if (codeout == code1){ //first endpoint was clipped
	      x1 = x; y1 = y;
	      code1 = findRegion(surface, x1, y1);
	    }else{ //second endpoint was clipped
	      x2 = x; y2 = y;
	      code2 = findRegion(surface, x2, y2);
	    }
	  }
	}while(!done);
	
	if (accept){
	  *x3 = x1;
	  *x4 = x2;
	  *y3 = y1;
	  *y4 = y2;
	  return 1;
	}else{
	 // *x3 = *x4 = *y3 = *y4 = 0;
	  return 0;
	}
}

static inline void drawLine16 (_ufont_surface_t *surface, int x0, int y0, int x1, int y1, const uint16_t colour)
{
	if (!clipLine(surface, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return;
		
	int dy = y1 - y0;
	int dx = x1 - x0;
	
	int stepx, stepy;
	uint16_t *pixels = (uint16_t*)surface->pixels;
	
	if (dy < 0) { dy = -dy;  stepy = -surface->width; } else { stepy = surface->width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= surface->width;
	y1 *= surface->width;
	pixels[x0+y0] = colour;
	
	if (dx > dy) {
	    int fraction = dy - (dx >> 1);
	    while (x0 != x1) {
	        if (fraction >= 0) {
	            y0 += stepy;
	            fraction -= dx;
	        }
	        x0 += stepx;
	        fraction += dy;
	        pixels[x0+y0] = colour;
	    }
	} else {
	    int fraction = dx - (dy >> 1);
	    while (y0 != y1) {
	        if (fraction >= 0) {
	            x0 += stepx;
	            fraction -= dy;
	        }
	        y0 += stepy;
	        fraction += dx;
	        pixels[x0+y0] = colour;
	    }
	}
}

static inline void drawLine1_set (_ufont_surface_t *surface, int x1, int y1, int x2, int y2)
{
	const int srcPitch = CALC_PITCH_1(surface->width);
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (abs(dx) >= abs(dy)){
            float y = y1+0.5f;
            float dly = dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx++){
                    setPixel1_bc(surface, srcPitch, xx, (int)y);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx--){
                    setPixel1_bc(surface, srcPitch, xx, (int)y);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5f;
           	float dlx = dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy++){
       	            setPixel1_bc(surface, srcPitch, (int)x, yy);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy--){
   	                setPixel1_bc(surface, srcPitch, (int)x, yy);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	setPixel1_bc(surface, srcPitch, x1, y1);
    }
}

static inline void drawLine1_clear (_ufont_surface_t *surface, int x1, int y1, int x2, int y2)
{
	const int srcPitch = CALC_PITCH_1(surface->width);
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (abs(dx) >= abs(dy)){
            float y = y1+0.5f;
            float dly = dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx++){
                    unsetPixel1_bc(surface, srcPitch, xx, (int)y);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx--){
                    unsetPixel1_bc(surface, srcPitch, xx, (int)y);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5f;
           	float dlx = dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy++){
       	            unsetPixel1_bc(surface, srcPitch, (int)x, yy);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy--){
   	                unsetPixel1_bc(surface, srcPitch, (int)x, yy);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	unsetPixel1_bc(surface, srcPitch, x1, y1);
    }
}

static inline void drawLine1 (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const int set)
{
	if (!clipLine(surface, x1, y1, x2, y2, &x1, &y1, &x2, &y2))
		return;
		
	if (set)
		drawLine1_set(surface, x1, y1, x2, y2);
	else
		drawLine1_clear(surface, x1, y1, x2, y2);
}

void fontDrawLine (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawLine1(surface, x1, y1, x2, y2, set&0x01);
}

void surfaceDrawLine (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour)
{
	if (surface->bpp == SURFACE_BPP_16)
		drawLine16(surface, x1, y1, x2, y2, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawLine1(surface, x1, y1, x2, y2, colour!=0);
}

static inline void drawRectangleFilled1 (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const int set)
{
	const int pitch = CALC_PITCH_1(surface->width);
	
	if (set){
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x <= x2; x++)
				setPixel1_nb(surface, pitch, x, y);
		}
	}else{
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x <= x2; x++)
				unsetPixel1_nb(surface, pitch, x, y);
		}
	}
}

static inline void drawRectangleFilled16 (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour)
{
	for (int y = y1; y <= y2; y++){
		for (int x = x1; x <= x2; x++)
			setPixel16_nb(surface, x, y, colour);
	}
}

void fontDrawRectangleFilled (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	bindRectangle(surface, &x1, &y1, &x2, &y2);
	
	drawRectangleFilled1(surface, x1, y1, x2, y2, set&0x01);
}

void surfaceDrawRectangleFilled (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour)
{
	bindRectangle(surface, &x1, &y1, &x2, &y2);
	
	if (surface->bpp == SURFACE_BPP_16)
		drawRectangleFilled16(surface, x1, y1, x2, y2, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawRectangleFilled1(surface, x1, y1, x2, y2, colour!=0);
}

static inline void drawRectangle1 (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const int set)
{
	const int srcPitch = CALC_PITCH_1(surface->width);

	int _x1 = x1;
	if (_x1 < 0) _x1 = 0;
	
	int _y1 = y1;
	if (_y1 < 0) _y1 = 0;
	
	int _x2 = x2;
	if (_x2 < 0)
		_x2 = 0;
	else if (_x2 >= surface->width)
		_x2 = surface->width-1;
	
	int _y2 = y2;
	if (_y2 < 0)
		_y2 = 0;
	else if (_y2 >= surface->height)
		_y2 = surface->height-1;
			
	if (set){
		if (y1 >= 0 && y1 <= surface->height)
		for (int x = _x1; x <= _x2; x++)
			setPixel1_nb(surface, srcPitch, x, _y1);
		
		if (x1 >= 0 && x1 <= surface->width)
		for (int y = _y1+1; y <= _y2-1; y++)
			setPixel1_nb(surface, srcPitch, _x1, y);

		if (x2 >= 0 && x2 <= surface->width)
		for (int y = _y1+1; y <= _y2-1; y++)
			setPixel1_nb(surface, srcPitch, _x2, y);

		if (y2 >= 0 && y2 <= surface->height)
		for (int x = _x1; x <= _x2; x++)
			setPixel1_nb(surface, srcPitch, x, _y2);
	}else{
		if (y1 >= 0 && y1 <= surface->height)
		for (int x = _x1; x <= _x2; x++)
			unsetPixel1_nb(surface, srcPitch, x, _y1);
		
		if (x1 >= 0 && x1 <= surface->width)
		for (int y = _y1+1; y <= _y2-1; y++)
			unsetPixel1_nb(surface, srcPitch, _x1, y);

		if (x2 >= 0 && x2 <= surface->width)
		for (int y = _y1+1; y <= _y2-1; y++)
			unsetPixel1_nb(surface, srcPitch, _x2, y);

		if (y2 >= 0 && y2 <= surface->height)
		for (int x = _x1; x <= _x2; x++)
			unsetPixel1_nb(surface, srcPitch, x, _y2);
	}
}

static inline void drawRectangle16 (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour)
{
	// top
	drawLine16(surface, x1, y1, x2, y1, colour);
	// left
	drawLine16(surface, x1, y1+1, x1, y2-1, colour);
	// right
	drawLine16(surface, x2, y1+1, x2, y2-1, colour);
	// bottom
	drawLine16(surface, x1, y2, x2, y2, colour);
}

void fontDrawRectangle (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawRectangle1(surface, x1, y1, x2, y2, set&0x01);
}

void surfaceDrawRectangle (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour)
{
	if (surface->bpp == SURFACE_BPP_16)
		drawRectangle16(surface, x1, y1, x2, y2, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawRectangle1(surface, x1, y1, x2, y2, colour!=0);
}

static inline void circlePts16 (_ufont_surface_t *surface, int xc, int yc, int x, int y, const uint16_t colour)
{
	setPixel16_bc(surface, xc+y, yc-x, colour);
	setPixel16_bc(surface, xc-y, yc-x, colour);
	setPixel16_bc(surface, xc+y, yc+x, colour);
	setPixel16_bc(surface, xc-y, yc+x, colour);
	setPixel16_bc(surface, xc+x, yc+y, colour);
	setPixel16_bc(surface, xc-x, yc+y, colour);
	setPixel16_bc(surface, xc+x, yc-y, colour);
	setPixel16_bc(surface, xc-x, yc-y, colour);
}

static inline void drawCircle16 (_ufont_surface_t *surface, const int xc, const int yc, const int radius, const uint16_t colour)
{
	float x = 0.0f; 
	float y = radius;
	float p = 1.0f-radius;

	circlePts16(surface, xc, yc, x, y, colour);
	while (x < y){
		x += 1.0f;
		if (p < 0.0f){
			p += 2.0f*x+1.0f;
		}else{
			y -= 1.0f;
			p += 2.0f*x+1.0f-2.0f*y;
		}
		circlePts16(surface, xc, yc, x, y, colour);
	}
}

static inline void circlePts1 (_ufont_surface_t *surface, const int srcPitch, int xc, int yc, int x, int y, const uint32_t set)
{
	if (set){
		setPixel1_bc(surface, srcPitch, xc+y, yc-x);
		setPixel1_bc(surface, srcPitch, xc-y, yc-x);
		setPixel1_bc(surface, srcPitch, xc+y, yc+x);
		setPixel1_bc(surface, srcPitch, xc-y, yc+x);
		setPixel1_bc(surface, srcPitch, xc+x, yc+y);
		setPixel1_bc(surface, srcPitch, xc-x, yc+y);
		setPixel1_bc(surface, srcPitch, xc+x, yc-y);
		setPixel1_bc(surface, srcPitch, xc-x, yc-y);
	}else{
		unsetPixel1_bc(surface, srcPitch, xc+y, yc-x);
		unsetPixel1_bc(surface, srcPitch, xc-y, yc-x);
		unsetPixel1_bc(surface, srcPitch, xc+y, yc+x);
		unsetPixel1_bc(surface, srcPitch, xc-y, yc+x);
		unsetPixel1_bc(surface, srcPitch, xc+x, yc+y);
		unsetPixel1_bc(surface, srcPitch, xc-x, yc+y);
		unsetPixel1_bc(surface, srcPitch, xc+x, yc-y);
		unsetPixel1_bc(surface, srcPitch, xc-x, yc-y);
	}
}

static inline void drawCircle1 (_ufont_surface_t *surface, const int xc, const int yc, const int radius, const uint32_t set)
{
	float x = 0.0f; 
	float y = radius;
	float p = 1.0f-radius;

	const int srcPitch = CALC_PITCH_1(surface->width);
	
	circlePts1(surface, srcPitch, xc, yc, x, y, set);
	while (x < y){
		x += 1.0f;
		if (p < 0.0f){
			p += 2.0f*x+1.0f;
		}else{
			y -= 1.0f;
			p += 2.0f*x+1.0f-2.0f*y;
		}
		circlePts1(surface, srcPitch, xc, yc, x, y, set);
	}
}

void fontDrawCircle (_ufont_t *font, const int xc, const int yc, const int radius, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawCircle1(surface, xc, yc, radius, set);
}

void surfaceDrawCircle (_ufont_surface_t *surface, const int xc, const int yc, const int radius, const uint16_t colour)
{
	if (surface->bpp == SURFACE_BPP_16)
		drawCircle16(surface, xc, yc, radius, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawCircle1(surface, xc, yc, radius, colour!=0);
}

static inline void horLine1 (_ufont_surface_t *surface, int y, int x1, int x2, const uint32_t set)
{
	drawLine1(surface, x1, y, x2, y, set);
}

static inline void drawCircleFilled1 (_ufont_surface_t *surface, int xc, int yc, const int radius, const uint32_t set)
{	
	if (xc + radius < 0 || xc - radius >= surface->width || yc + radius < 0 || yc - radius >= surface->height)
		return;
  	
	int y = radius;
	int p = 3 - (radius << 1);
	int a, b, c, d, e, f, g, h, x = 0;
	int pb = yc + radius + 1, pd = yc + radius + 1;
  
	while (x <= y){
		a = xc + x; b = yc + y;
		c = xc - x; d = yc - y;
		e = xc + y; f = yc + x;
		g = xc - y; h = yc - x;

		if (b != pb)
			horLine1(surface, b, a, c, set);
		if (d != pd)
			horLine1(surface, d, a, c, set);
		if (f != b)
			horLine1(surface, f, e, g, set);
		if (h != d && h != f)
			horLine1(surface, h, e, g, set);
		pb = b; pd = d;

		if (p < 0)
			p += (x++ << 2) + 6;
		else
			p += ((x++ - y--) << 2) + 10;
	}
}

static inline void horLine16 (_ufont_surface_t *surface, int y, int x1, int x2, const uint16_t colour)
{
	drawLine16(surface, x1, y, x2, y, colour);
}

static inline void drawCircleFilled16 (_ufont_surface_t *surface, int xc, int yc, const int radius, const uint16_t colour)
{	
	if (xc + radius < 0 || xc - radius >= surface->width || yc + radius < 0 || yc - radius >= surface->height)
		return;
  	
	int y = radius;
	int p = 3 - (radius << 1);
	int a, b, c, d, e, f, g, h, x = 0;
	int pb = yc + radius + 1, pd = yc + radius + 1;
  
	while (x <= y){
		a = xc + x; b = yc + y;
		c = xc - x; d = yc - y;
		e = xc + y; f = yc + x;
		g = xc - y; h = yc - x;

		if (b != pb)
			horLine16(surface, b, a, c, colour);
		if (d != pd)
			horLine16(surface, d, a, c, colour);
		if (f != b)
			horLine16(surface, f, e, g, colour);
		if (h != d && h != f)
			horLine16(surface, h, e, g, colour);
		pb = b; pd = d;

		if (p < 0)
			p += (x++ << 2) + 6;
		else
			p += ((x++ - y--) << 2) + 10;
	}
}
	
void fontDrawCircleFilled (_ufont_t *font, const int xc, const int yc, const int radius, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawCircleFilled1(surface, xc, yc, radius, set);
}

void surfaceDrawCircleFilled (_ufont_surface_t *surface, const int xc, const int yc, const int radius, const uint16_t colour)
{
	if (surface->bpp == SURFACE_BPP_16)
		drawCircleFilled16(surface, xc, yc, radius, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawCircleFilled1(surface, xc, yc, radius, colour!=0);
}

static inline void drawArc1 (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint32_t set)
{
	float pipi = M_PI;
	
	a1 = 360.0f - (-a1 + 90.0f);
	a2 = 360.0f - ( a2 + 90.0f);

	const int N = fabsf(a2-a1)+8.0f;
	const float da = (a2-a1)*(2.0f*pipi/360.0f)/(float)(N-1.0f);
	const float a = a1*2.0f*pipi/360.0f;
	
	float lastx = x + (r1*cosf(a));
	float lasty = y + (r2*sinf(a));

	for (float loop = 1.0f; loop < N; loop += 1.0f){
		float myx = x + (r1*cosf(a+loop*da));
		float myy = y + (r2*sinf(a+loop*da));
		drawLine1(surface, lastx, lasty, myx, myy, set);
			
		lastx = myx;
		lasty = myy;
	}
}

static inline void drawArc16 (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint16_t colour)
{
	float pipi = M_PI;
	
	a1 = 360.0f - (-a1 + 90.0f);
	a2 = 360.0f - ( a2 + 90.0f);

	const int N = fabsf(a2-a1)+8.0f;
	const float da = (a2-a1)*(2.0f*pipi/360.0f)/(float)(N-1.0f);
	const float a = a1*2.0f*pipi/360.0f;
	float lastx = x + (r1*cosf(a));
	float lasty = y + (r2*sinf(a));

	for (float loop = 1.0f; loop < N; loop += 1.0f){
		float myx = x + (r1*cosf(a+loop*da));
		float myy = y + (r2*sinf(a+loop*da));
		drawLine16(surface, lastx, lasty, myx, myy, colour);

		lastx = myx;
		lasty = myy;
	}
}

void fontDrawArc (_ufont_t *font, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawArc1(surface, x, y, r1, r2, a1, a2, set&0x01);
}

void surfaceDrawArc (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint16_t colour)
{
	if (surface->bpp == SURFACE_BPP_16)
		drawArc16(surface, x, y, r1, r2, a1, a2, colour);
	else if (surface->bpp == SURFACE_BPP_1)
		drawArc1(surface, x, y, r1, r2, a1, a2, colour!=0);
}

void fontDrawEllipse (_ufont_t *font, const int x, const int y, const int r1, const int r2, const uint32_t set)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	drawArc1(surface, x, y, r1, r2,  0.0f,  90.0f, set);
	drawArc1(surface, x, y, r1, r2, 90.0f, 360.0f, set);
}

void surfaceDrawEllipse (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, const uint16_t colour)
{
	surfaceDrawArc(surface, x, y, r1, r2,  0.0f,  90.0f, colour);
	surfaceDrawArc(surface, x, y, r1, r2, 90.0f, 360.0f, colour);
}

void fontInvertRectangle (_ufont_t *font, int x1, int y1, int x2, int y2)
{
	_ufont_surface_t *surface = fontGetRenderSurface(font);
	const int pitch = CALC_PITCH_1(surface->width);
	bindRectangle(surface, &x1, &y1, &x2, &y2);

	for (int y = y1; y <= y2; y++){
		for (int x = x1; x <= x2; x++){
			if (!getPixel1(surface->pixels, pitch, x, y))
				setPixel1_nb(surface, pitch, x, y);
			else
				unsetPixel1_nb(surface, pitch, x, y);
		}
	}
}


#else


void drawRectangle (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour){}

#endif