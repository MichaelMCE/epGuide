
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


#include <math.h>
#include "../common.h"



#define _RGB(r,g,b)		(((r) << 16) | ((g) << 8) | (b))	// Convert to RGB
#define _GetRValue(c)	((int)(((c) & 0x00FF0000) >> 16))	// Red color component
#define _GetGValue(c)	((int)(((c) & 0x0000FF00) >> 8))	// Green color component
#define _GetBValue(c)	((int)( (c) & 0x000000FF))			// Blue color component
#define itofx(x)		((x) << 8)							// Integer to int point
#define ftofx(x)		(int)((x) * 256)					// Float to int point
#define fxtoi(x)		((x) >> 8)							// Fixed point to integer
#define Mulfx(x,y)		(((x) * (y)) >> 8)					// Multiply a int by a int
#define Divfx(x,y)		(((x) << 8) / (y))					// Divide a int by a int






// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
static const unsigned int stack_blur8_mul[] =
{
    512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
    454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
    482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
    437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
    497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
    320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
    446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
    329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
    505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
    399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
    324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
    268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
    451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
    385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
    332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
    289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static const unsigned int stack_blur8_shr[] =
{
     9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

static inline void blurStackFastNoAlpha (TFRAME *src, const int x1, const int y1, const int x2, const int y2, const int radius)
{
	enum enumColorOrder{R, G, B};

    const unsigned int *pImage = (unsigned int*)src->pixels;
    const unsigned int w = src->width;
    const unsigned int h = (y2-y1)+1; //src->height;
    
    unsigned int x = 0, xp = 0, yp = 0, i = 0, t = 0;
    unsigned int stack_ptr = 0;
    unsigned int stack_start = 0;

    unsigned char *src_pix_ptr = NULL;
    unsigned char *dst_pix_ptr = NULL;
    unsigned char *stack_pix_ptr = NULL;

    unsigned int sum_r = 0;
    unsigned int sum_g = 0;
    unsigned int sum_b = 0;
    //unsigned int sum_a = 0;
    unsigned int sum_in_r = 0;
    unsigned int sum_in_g = 0;
    unsigned int sum_in_b = 0;
    //unsigned int sum_in_a = 0;
    unsigned int sum_out_r = 0;
    unsigned int sum_out_g = 0;
    unsigned int sum_out_b = 0;
   // unsigned int sum_out_a = 0;

    const unsigned int wm  = (w - 1);
    const unsigned int hm  = (h - 1);
    unsigned int row_addr = 0;

	const unsigned int r = radius;
    const unsigned int mul_sum = stack_blur8_mul[r];
    const unsigned int shr_sum = stack_blur8_shr[r];
    const unsigned int div = r + r + 1;

    unsigned int *lpStack = my_malloc(sizeof(unsigned int) * div);
	const unsigned int *stack_data_ptr = lpStack;
    unsigned int y = 0;

    do{ 
        sum_r = 
        sum_g = 
        sum_b = 
        //sum_a = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
       // sum_in_a = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 0;
       // sum_out_a = 0;

        row_addr = (y * w);
        src_pix_ptr = (unsigned char*)(pImage + row_addr);
        i = 0;

        do {
            t = (i + 1);
            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
           // *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
            //sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
           // sum_out_a += *(stack_pix_ptr + A);

            if (i > 0){
                t = (r + 1 - i);
                
                if (i <= wm)
                    src_pix_ptr += 4; 

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
              //  *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
               // sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
               // sum_in_a += *(stack_pix_ptr + A);
            }
        }while(++i <= r);

        stack_ptr = r;
        xp = r;

        if (xp > wm) xp = wm;

        src_pix_ptr = (unsigned char*)(pImage + (xp + row_addr));
        dst_pix_ptr = (unsigned char*)(pImage + row_addr);
        x = 0;

        do{
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
            //*(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += 4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
            //sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);

            if (stack_start >= div) 
                stack_start -= div;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
          //  sum_out_a -= *(stack_pix_ptr + A);

            if (xp < wm){
                src_pix_ptr += 4;
                ++xp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
           // *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
           // sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
           // sum_a += sum_in_a;

            if (++stack_ptr >= div) 
                stack_ptr = 0;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            //sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
           // sum_in_a -= *(stack_pix_ptr + A);
        }while(++x < w);
    }while(++y < h);

    const unsigned int stride = (w << 2);
    stack_data_ptr = lpStack;
    x = 0;
    
    do{
        sum_r = 
        sum_g = 
        sum_b = 
        //sum_a = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
       // sum_in_a = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 0;
      //  sum_out_a = 0;

        src_pix_ptr = (unsigned char*)(pImage + x);
        i = 0;

        do{
            t = (i + 1);
        
            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
           // *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
           // sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
           // sum_out_a += *(stack_pix_ptr + A);

            if (i > 0){
                t = (r + 1 - i);
                
                if (i <= hm)
                    src_pix_ptr += stride; 

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
               // *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
               // sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
               // sum_in_a += *(stack_pix_ptr + A);
            }
        }while(++i <= r);

        stack_ptr = r;
        yp = r;

        if (yp > hm) yp = hm;

        src_pix_ptr = (unsigned char*)(pImage + (x + (yp * w)));
        dst_pix_ptr = (unsigned char*)(pImage + x);
        y = 0;

        do{
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
           // *(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += stride;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
           // sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);
            if (stack_start >= div)
                stack_start -= div;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
           // sum_out_a -= *(stack_pix_ptr + A);

            if (yp < hm){
                src_pix_ptr += stride;
                ++yp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            //*(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
           // sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
           // sum_a += sum_in_a;

            if (++stack_ptr >= div) stack_ptr = 0;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            //sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
            //sum_in_a -= *(stack_pix_ptr + A);
        }while(++y < h);
    }while(++x < w);
    
    my_free(lpStack);
}

void transBlur (TFRAME *src, const int radius)
{
	blurStackFastNoAlpha(src, 0, 0, src->width-1, src->height-1, radius);
}

void rotateBilinear (TFRAME *frame, TFRAME *des, const int degrees)
{
	
	const float _angle = ((float)-degrees/180.0f) * M_PI;
	const int bpp = 4;
	const int _pitch = frame->pitch;
	const int _width = (int)(abs((float)frame->width*cos(_angle)) + abs((float)frame->height*sin(_angle)) + 0.5f);
	const int _height = (int)(abs((float)frame->width*sin(_angle)) + abs((float)frame->height*cos(_angle)) + 0.5f);
	const int srcWidth = frame->width;
	const int srcHeight = frame->height;
	
	const int f_0_5 = ftofx(0.5f);
	const int f_H = itofx(srcHeight/2);
	const int f_W = itofx(srcWidth/2);
	const int f_cos = ftofx(cos(-_angle));
	const int f_sin = ftofx(sin(-_angle));
	const int f_1 = itofx(1);
	
	const int *restrict srcData = (int*restrict)frame->pixels;
	int *restrict desData = (int*restrict)des->pixels;
	int dwDstHorizontalOffset;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	
	int xx = (_width-srcWidth)/2;
	int xoffset = 0;
	if (xx < 0){
		xoffset = abs(xx)<<2;
		xx = 0;
	}
	const int yy = (_height-srcHeight)/2;
	int dwDstVerticalOffset = 0;
	
	for (int i = yy; i < _height-yy-1; i++){
		dwDstHorizontalOffset = xoffset;
	
		for (int j = xx; j < _width-xx-1; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i-_height/2);
			int f_j = itofx(j-_width/2);
			int f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
			int f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
			int m = fxtoi(f_m);
			int n = fxtoi(f_n);
			
			if ((m >= 0) && (m < srcHeight) && (n >= 0) && (n < srcWidth)){
				
				int f_f = f_m - itofx(m);
				int f_g = f_n - itofx(n);
				dwSrcTotalOffset = m*frame->pitch + n * bpp;
				const int dwSrcTopLeft = dwSrcTotalOffset;
				
				int dwSrcTopRight = dwSrcTotalOffset + bpp;
				if (n >= srcWidth-1)
					dwSrcTopRight = dwSrcTotalOffset;
					
				int dwSrcBottomLeft = dwSrcTotalOffset + _pitch;
				if (m >= srcHeight-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
					
				int dwSrcBottomRight = dwSrcTotalOffset + _pitch + bpp;
				if ((n >= srcWidth-1) || (m >= srcHeight-1))
					dwSrcBottomRight = dwSrcTotalOffset;
					
				int f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				int f_w2 = Mulfx(f_1-f_f, f_g);
				int f_w3 = Mulfx(f_f, f_1-f_g);
				int f_w4 = Mulfx(f_f, f_g);
				
				int pixel1 = srcData[dwSrcTopLeft>>2];
				int pixel2 = srcData[dwSrcTopRight>>2];
				int pixel3 = srcData[dwSrcBottomLeft>>2];
				int pixel4 = srcData[dwSrcBottomRight>>2];
				
				int f_r1 = itofx(_GetRValue(pixel1));
				int f_r2 = itofx(_GetRValue(pixel2));
				int f_r3 = itofx(_GetRValue(pixel3));
				int f_r4 = itofx(_GetRValue(pixel4));
				int f_g1 = itofx(_GetGValue(pixel1));
				int f_g2 = itofx(_GetGValue(pixel2));
				int f_g3 = itofx(_GetGValue(pixel3));
				int f_g4 = itofx(_GetGValue(pixel4));
				int f_b1 = itofx(_GetBValue(pixel1));
				int f_b2 = itofx(_GetBValue(pixel2));
				int f_b3 = itofx(_GetBValue(pixel3));
				int f_b4 = itofx(_GetBValue(pixel4));
				
				int f_red = Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4);
				int f_green = Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4);
				int f_blue = Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4);
				
				int red = (int)MAX(0, MIN(255, fxtoi(f_red)));
				int green = (int)MAX(0, MIN(255, fxtoi(f_green)));
				int blue = (int)MAX(0, MIN(255, fxtoi(f_blue)));
				
				desData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
			}
			dwDstHorizontalOffset += bpp;
		}
		dwDstVerticalOffset += _pitch;
	}
}

void rotateNearestNeighbour (TFRAME *frame, TFRAME *des, const int degrees, const int bgColor)
{

	const float _angle = ((float)-degrees/180.0f) * M_PI;
	int m_iBpp = 4;
	int m_iPitch = frame->pitch;
	int _width = (int)(abs((float)frame->width*cos(_angle)) + abs((float)frame->height*sin(_angle)) + 0.5f);
	int _height = (int)(abs((float)frame->width*sin(_angle)) + abs((float)frame->height*cos(_angle)) + 0.5f);
	
	int _pitch = m_iBpp * _width;
	while ((_pitch & 3) != 0)
		_pitch++;
		
	int f_0_5 = ftofx(0.5f);
	int f_H = itofx(frame->height/2);
	int f_W = itofx(frame->width/2);
	int f_cos = ftofx(cos(-_angle));
	int f_sin = ftofx(sin(-_angle));
	
	int dwSize = _pitch * _height;
	char *lpData = (char*)my_Malloc(dwSize+_pitch, funcname, linenumber);
	if (!lpData) return;
	
	int dwDstHorizontalOffset;
	int dwDstVerticalOffset = 0;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	int *lpSrcData = (int*)frame->pixels;
	int *lpDstData = (int*)lpData;
	
	for (int i = 0; i < _height; i++){
		dwDstHorizontalOffset = 0;
		
		for (int j = 0; j < _width; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i-_height/2);
			int f_j = itofx(j-_width/2);
			int f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
			int f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
			int m = fxtoi(f_m);
			int n = fxtoi(f_n);
			
			if ((m >= 0) && (m < frame->height) && (n >= 0) && (n < frame->width)){
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
			}else{
				lpDstData[dwDstTotalOffset>>2] = bgColor;
			}
			dwDstHorizontalOffset += m_iBpp;
		}
		dwDstVerticalOffset += _pitch;
	}
	
	my_Free(des->pixels, funcname, linenumber);
	des->pixels = (ubyte*)lpData;
	des->pitch = _pitch;
	des->width = _width;
	des->height = _height;
	des->frameSize = dwSize;
	
}

void rotateBicubic (TFRAME *frame, TFRAME *des, const int degrees, int bgColor)
{

	float _angle = ((float)-degrees/180.0f) * M_PI;
	int m_iBpp = 4;
	int _width = (int)(abs((float)frame->width*cos(_angle)) + abs((float)frame->height*sin(_angle)) + 0.5f);
	int _height = (int)(abs((float)frame->width*sin(_angle)) + abs((float)frame->height*cos(_angle)) + 0.5f);
	
	int _pitch = m_iBpp * _width;
	while ((_pitch & 3) != 0)
		_pitch++;
		
	int f_0_5 = ftofx(0.5f);
	int f_H = itofx(frame->height/2);
	int f_W = itofx(frame->width/2);
	int f_cos = ftofx(cos(-_angle));
	int f_sin = ftofx(sin(-_angle));
	int f_1 = itofx(1);
	int f_2 = itofx(2);
	int f_4 = itofx(4);
	int f_6 = itofx(6);
	int f_gama = ftofx(1.04f);
	
	int dwSize = _pitch * _height;
	char *lpData = (char*)my_Malloc(dwSize+_pitch, funcname, linenumber);
	if (!lpData) return;
	
	int dwDstHorizontalOffset;
	int dwDstVerticalOffset = 0;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	int *lpSrcData = (int*)frame->pixels;
	int *lpDstData = (int*)lpData;
	
	for (int i = 0; i < _height; i++){
		dwDstHorizontalOffset = 0;
		
		for (int j = 0; j < _width; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i-_height/2);
			int f_j = itofx(j-_width/2);
			int f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
			int f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
			int m = fxtoi(f_m);
			int n = fxtoi(f_n);
			
			if ((m >= 0) && (m < frame->height) && (n >= 0) && (n < frame->width)){
				int f_f = f_m - itofx(m);
				int f_g = f_n - itofx(n);
				dwSrcTotalOffset = m*frame->pitch + n*m_iBpp;
				int dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset - frame->pitch - m_iBpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset - frame->pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset - frame->pitch + m_iBpp;
				if ((m < 1) || (n >= frame->width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset - frame->pitch + m_iBpp + m_iBpp;
				if ((m < 1) || (n >= frame->width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
				if (n >= frame->width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
				if (n >= frame->width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset + frame->pitch - m_iBpp;
				if ((m >= frame->height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset + frame->pitch;
				if (m >= frame->height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset + frame->pitch + m_iBpp;
				if ((m >= frame->height-1) || (n >= frame->width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset + frame->pitch + m_iBpp + m_iBpp;
				if ((m >= frame->height-1) || (n >= frame->width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset + frame->pitch + frame->pitch - m_iBpp;
				if ((m >= frame->height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset + frame->pitch + frame->pitch;
				if (m >= frame->height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset + frame->pitch + frame->pitch + m_iBpp;
				if ((m >= frame->height-2) || (n >= frame->width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset + frame->pitch + frame->pitch + m_iBpp + m_iBpp;
				if ((m >= frame->height-2) || (n >= frame->width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				int f_red=0, f_green=0, f_blue=0;
				
				for (int k = -1; k < 3; k++){
					int f = itofx(k)-f_f;
					int f_fm1 = f - f_1;
					int f_fp1 = f + f_1;
					int f_fp2 = f + f_2;
					int f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					int f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					int f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					int f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					int f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					
					for (int l = -1; l < 3; l++){
						int f = itofx(l)-f_g;
						int f_fm1 = f - f_1;
						int f_fp1 = f + f_1;
						int f_fp2 = f + f_2;
						int f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						int f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						int f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						int f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						int f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						int f_R = Mulfx(f_RY,f_RX);
						int _k = ((k+1)*4) + (l+1);
						int f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						int f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						int f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				
				int red = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_red,f_gama))));
				int green = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_green,f_gama))));
				int blue = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_blue,f_gama))));
				
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
			}else{
				lpDstData[dwDstTotalOffset>>2] = bgColor;
			}
			dwDstHorizontalOffset += m_iBpp;
		}
		dwDstVerticalOffset += _pitch;
	}
	
	my_Free(des->pixels, funcname, linenumber);
	des->pixels = (ubyte*)lpData;
	des->pitch = _pitch;
	des->width = _width;
	des->height = _height;
	des->frameSize = dwSize;
}

static inline void bitblit (TFRAME *src, TFRAME *des, const int x1, const int y1, const int x2, const int y2, const int desX, int desY)
{
	const int pitch = ((x2-x1)+1)<<2;
	
	for (int y = y1; y <= y2; y++){
		char *psrc = lGetPixelAddress(src, x1, y);
		char *pdes = lGetPixelAddress(des, desX, desY++);
		
		//for (int x = x1; x <= x2; x++)
		//	*pdes++ = *psrc++;
		my_memcpy(pdes, psrc, pitch);
		pdes += pitch;
	}
}

void scaleNearestNeighbour (TFRAME *frame, TFRAME *des, const int width, const int height, const int desX, const int desY)
{

	void *m_lpData = frame->pixels;
	int m_iBpp = 4;
	int m_iPitch = frame->pitch;
	int _width = MAX(1, width);
	int _height = MAX(1, height);
	float dx = (float)frame->width / (float)_width;
	float dy = (float)frame->height / (float)_height;
	
	int _pitch = m_iBpp * _width;
	while ((_pitch & 3) != 0)
		_pitch++;
		
	int f_dx = ftofx(dx);
	int f_dy = ftofx(dy);
	
	int dwDstHorizontalOffset;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	int *lpSrcData = (int*)m_lpData;
	int *lpDstData = (int*)des->pixels;
	
	const int minw = MIN(des->width, _width);
	int x = (_width - des->width)/2;
	if (x < 0) x = 0;
	
	const int minh = MIN(des->height, _height);
	int y = (_height - des->height)/2;
	if (y < 0) y = 0;
	int dwDstVerticalOffset = desY*des->pitch;
	
	for (int i = y; i < y+minh; i++){
		dwDstHorizontalOffset = desX<<2;
		
		for (int j = x; j < x+minw; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i);
			int f_j = itofx(j);
			int f_a = Mulfx(f_i, f_dy);
			int f_b = Mulfx(f_j, f_dx);
			int m = fxtoi(f_a);
			int n = fxtoi(f_b);
			dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
			lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
	
			dwDstHorizontalOffset += m_iBpp;
		}
		dwDstVerticalOffset += des->pitch;
	}
}

void scaleBilinear (TFRAME *frame, TFRAME *des, const int width, const int height, const int desX, const int desY)
{

	void *m_lpData = frame->pixels;
	int m_iBpp = 4;
	int m_iPitch = frame->pitch;
	int _width = MAX(1, width);
	int _height = MAX(1, height);
	float dx = (float)frame->width / (float)_width;
	float dy = (float)frame->height / (float)_height;
	
	int _pitch = m_iBpp * _width;
	while ((_pitch & 3) != 0)
		_pitch++;
		
	int f_dx = ftofx(dx);
	int f_dy = ftofx(dy);
	int f_1 = itofx(1);
	
	int dwDstHorizontalOffset;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	int *lpSrcData = (int*)m_lpData;
	int *lpDstData = (int*)des->pixels;
	
	const int minw = MIN(des->width, _width);
	int x = (_width - des->width)/2;
	if (x < 0) x = 0;
	
	const int minh = MIN(des->height, _height);
	int y = (_height - des->height)/2;
	if (y < 0) y = 0;
	int dwDstVerticalOffset = desY*des->pitch;
	
	for (int i = y; i < y+minh; i++){
		dwDstHorizontalOffset = desX<<2;
		
		for (int j = x; j < x+minw; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i);
			int f_j = itofx(j);
			int f_a = Mulfx(f_i, f_dy);
			int f_b = Mulfx(f_j, f_dx);
			
			int m = fxtoi(f_a);
			int n = fxtoi(f_b);
			int f_f = f_a - itofx(m);
			int f_g = f_b - itofx(n);
			
			dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
			int dwSrcTopLeft = dwSrcTotalOffset;
			int dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
			
			if (n >= frame->width-1)
				dwSrcTopRight = dwSrcTotalOffset;
			int dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
			if (m >= frame->height-1)
				dwSrcBottomLeft = dwSrcTotalOffset;
			int dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
			if ((n >= frame->width-1) || (m >= frame->height-1))
				dwSrcBottomRight = dwSrcTotalOffset;
				
			int f_w1 = Mulfx(f_1-f_f, f_1-f_g);
			int f_w2 = Mulfx(f_1-f_f, f_g);
			int f_w3 = Mulfx(f_f, f_1-f_g);
			int f_w4 = Mulfx(f_f, f_g);
			
			int pixel1 = lpSrcData[dwSrcTopLeft>>2];
			int pixel2 = lpSrcData[dwSrcTopRight>>2];
			int pixel3 = lpSrcData[dwSrcBottomLeft>>2];
			int pixel4 = lpSrcData[dwSrcBottomRight>>2];
			
			int f_r1 = itofx(_GetRValue(pixel1));
			int f_r2 = itofx(_GetRValue(pixel2));
			int f_r3 = itofx(_GetRValue(pixel3));
			int f_r4 = itofx(_GetRValue(pixel4));
			int f_g1 = itofx(_GetGValue(pixel1));
			int f_g2 = itofx(_GetGValue(pixel2));
			int f_g3 = itofx(_GetGValue(pixel3));
			int f_g4 = itofx(_GetGValue(pixel4));
			int f_b1 = itofx(_GetBValue(pixel1));
			int f_b2 = itofx(_GetBValue(pixel2));
			int f_b3 = itofx(_GetBValue(pixel3));
			int f_b4 = itofx(_GetBValue(pixel4));
			
			int red = (int)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
			int green = (int)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
			int blue = (int)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
			
			lpDstData[dwDstTotalOffset>>2] = (255<<24) | _RGB(red, green, blue);
	
			dwDstHorizontalOffset += m_iBpp;
		}
		dwDstVerticalOffset += des->pitch;
	}
}

void scaleBicubic (TFRAME *frame, TFRAME *des, const int width, const int height, const int desX, const int desY)
{

	void *m_lpData = frame->pixels;
	int m_iBpp = 4;
	int m_iPitch = frame->pitch;
	int _width = MAX(1, width);
	int _height = MAX(1, height);
	float dx = (float)frame->width / (float)_width;
	float dy = (float)frame->height / (float)_height;
	
	int _pitch = m_iBpp * _width;
	while ((_pitch & 3) != 0)
		_pitch++;
		
	int f_dx = ftofx(dx);
	int f_dy = ftofx(dy);
	int f_1 = itofx(1);
	int f_2 = itofx(2);
	int f_4 = itofx(4);
	int f_6 = itofx(6);
	int f_gama = ftofx(1.04f);
	
	int dwDstHorizontalOffset;
	int dwDstTotalOffset;
	int dwSrcTotalOffset;
	int *lpSrcData = (int*)m_lpData;
	int *lpDstData = (int*)des->pixels;
	
	const int minw = MIN(des->width, _width);
	int x = (_width - des->width)/2;
	if (x < 0) x = 0;
	
	const int minh = MIN(des->height, _height);
	int y = (_height - des->height)/2;
	if (y < 0) y = 0;
	int dwDstVerticalOffset = desY*des->pitch;
	
	for (int i = y; i < y+minh; i++){
		dwDstHorizontalOffset = desX<<2;
		
		for (int j = x; j < x+minw; j++){
			dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
	
			int f_i = itofx(i);
			int f_j = itofx(j);
			int f_a = Mulfx(f_i, f_dy);
			int f_b = Mulfx(f_j, f_dx);
			
			int m = fxtoi(f_a);
			int n = fxtoi(f_b);
			int f_f = f_a - itofx(m);
			int f_g = f_b - itofx(n);
			
			dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
			int dwSrcOffsets[16];
			
			dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
			if ((m < 1) || (n < 1))
				dwSrcOffsets[0] = dwSrcTotalOffset;
			dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
			if (m < 1)
				dwSrcOffsets[1] = dwSrcTotalOffset;
			dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
			if ((m < 1) || (n >= frame->width-1))
				dwSrcOffsets[2] = dwSrcTotalOffset;
			dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
			if ((m < 1) || (n >= frame->width-2))
				dwSrcOffsets[3] = dwSrcTotalOffset;
			dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
			if (n < 1)
				dwSrcOffsets[4] = dwSrcTotalOffset;
			dwSrcOffsets[5] = dwSrcTotalOffset;
			dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
			if (n >= frame->width-1)
				dwSrcOffsets[6] = dwSrcTotalOffset;
			dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
			if (n >= frame->width-2)
				dwSrcOffsets[7] = dwSrcTotalOffset;
			dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
			if ((m >= frame->height-1) || (n < 1))
				dwSrcOffsets[8] = dwSrcTotalOffset;
			dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
			if (m >= frame->height-1)
				dwSrcOffsets[9] = dwSrcTotalOffset;
			dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
			if ((m >= frame->height-1) || (n >= frame->width-1))
				dwSrcOffsets[10] = dwSrcTotalOffset;
			dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
			if ((m >= frame->height-1) || (n >= frame->width-2))
				dwSrcOffsets[11] = dwSrcTotalOffset;
			dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
			if ((m >= frame->height-2) || (n < 1))
				dwSrcOffsets[12] = dwSrcTotalOffset;
			dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
			if (m >= frame->height-2)
				dwSrcOffsets[13] = dwSrcTotalOffset;
			dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
			if ((m >= frame->height-2) || (n >= frame->width-1))
				dwSrcOffsets[14] = dwSrcTotalOffset;
			dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
			if ((m >= frame->height-2) || (n >= frame->width-2))
				dwSrcOffsets[15] = dwSrcTotalOffset;
	
			int f_red=0, f_green=0, f_blue=0;
			
			for (int k = -1; k < 3; k++){
				
				int f = itofx(k)-f_f;
				int f_fm1 = f - f_1;
				int f_fp1 = f + f_1;
				int f_fp2 = f + f_2;
				int f_a = 0;
				if (f_fp2 > 0)
					f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
				int f_b = 0;
				if (f_fp1 > 0)
					f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
				int f_c = 0;
				if (f > 0)
					f_c = Mulfx(f,Mulfx(f,f));
				int f_d = 0;
				if (f_fm1 > 0)
					f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					
				int f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
				
				for (int l = -1; l < 3; l++){
					
					int f = itofx(l)-f_g;
					int f_fm1 = f - f_1;
					int f_fp1 = f + f_1;
					int f_fp2 = f + f_2;
					int f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					int f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					int f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					int f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						
					int f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					int f_R = Mulfx(f_RY,f_RX);
	
					int _k = ((k+1)*4) + (l+1);
					int f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
					int f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
					int f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
					
					f_red += Mulfx(f_rs,f_R);
					f_green += Mulfx(f_gs,f_R);
					f_blue += Mulfx(f_bs,f_R);
				}
			}
			int red = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_red,f_gama))));
			int green = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_green,f_gama))));
			int blue = (int)MAX(0, MIN(255, fxtoi(Mulfx(f_blue,f_gama))));
	
			lpDstData[dwDstTotalOffset>>2] = (255<<24)|_RGB(red, green, blue);
			dwDstHorizontalOffset += m_iBpp;
		}
		dwDstVerticalOffset += des->pitch;
	}
}

int transRotate (TFRAME *src, TFRAME *des, const int angle, const int type)
{
	TFRAME *tmp;
	
	if (type == ROTATE_BILINEAR){
		rotateBilinear(src, des, angle);
		return 1;
		
	}else if (type == ROTATE_BICUBIC){
		tmp = lNewFrame(src->hw, 8, 8, src->bpp);
		rotateBicubic(src, tmp, angle, 0);
		
	}else if (type == ROTATE_NEIGHBOUR){
		tmp = lNewFrame(src->hw, 8, 8, src->bpp);
		rotateNearestNeighbour(src, tmp, angle, 0);
	}else{
		return 0;
	}
		
	const int h = MIN(tmp->height,des->height);
	const int y = (abs(tmp->height - h)/2);
	const int desY = (des->height-h)/2;
	const int w = MIN(tmp->width,des->width);
	const int x = (abs(tmp->width - w)/2);
	const int desX = ((des->width-w)/2);

	if (desX){
		for (int y = 0; y < des->height; y++){
			void *pixels = lGetPixelAddress(des, 0, y);
			memset(pixels, 0, desX<<2);
			
			pixels += (desX<<2) + (w<<2);
			memset(pixels, 0, (desX+1)<<2);
		}
	}
	
	bitblit(tmp, des, x, y, x+w-1, y+h-1, desX, desY);

	lDeleteFrame(tmp);
	return 1;
}

int transScale (TFRAME *src, TFRAME *des, const int width, const int height, const int desX, const int desY, const int type)
{
	TFRAME *tmp = NULL;
	if (src == des){
		tmp = lNewFrame(src->hw, des->width, des->height, des->bpp);
	}else{
		tmp = des;
		if (type&SCALE_CLEANDES)
			memset(des->pixels, 0, des->frameSize);
	}
	
	if ((type&0xFF) == SCALE_BILINEAR)
		scaleBilinear(src, tmp, width, height, desX, desY);
	else if ((type&0xFF) == SCALE_BICUBIC)
		scaleBicubic(src, tmp, width, height, desX, desY);
	else if ((type&0xFF) == SCALE_NEIGHBOUR)
		scaleNearestNeighbour(src, tmp, width, height, desX, desY);

	if (src == des){
		my_memcpy(src->pixels, tmp->pixels, tmp->frameSize);
		lDeleteFrame(tmp);
	}
	return 1;
}

#if 0
void transBrightness (TFRAME *src, const int brightness)
{
	// Check for valid bitmap
	int m_iBpp = 4;
	// Calculate brightness params
	int _brightness = MAX(-255, MIN(255, brightness));
	
	// Change bitmap brightness
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpDstData = (int*)src->pixels;
	
	for (int i = 0; i < src->height; i++){
		dwHorizontalOffset = 0;
		
		for (int j = 0; j < src->width; j++){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;
	
			// Update bitmap
			int red = _GetRValue(lpDstData[dwTotalOffset>>2]);
			int green = _GetGValue(lpDstData[dwTotalOffset>>2]);
			int blue = _GetBValue(lpDstData[dwTotalOffset>>2]);
			
			red = (int)MAX(0, MIN(red+_brightness, 255));
			green = (int)MAX(0, MIN(green+_brightness, 255));
			blue = (int)MAX(0, MIN(blue+_brightness, 255));
			lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);
	
			// Update horizontal offset
			dwHorizontalOffset += m_iBpp;
		}
	
		// Update vertical offset
		dwVerticalOffset += src->pitch;
	}
}
#endif


void transPixelize (TFRAME *src, const int size)
{
	// Check for valid bitmap
	int m_iBpp = 4;
	// Calculate pixelize params
	int _size = MAX(1, MIN((src->width>>4), size));
	int f_size = Divfx(itofx(1),itofx(_size*_size));
	
	// Create temporary bitmap
	int dwSize = src->pitch * src->height;
	char *lpData = (char*)my_Malloc(dwSize*sizeof(char)+src->pitch, funcname, linenumber);
	if (!lpData) return;
	
	// Pixelize bitmap
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpSrcData = (int*)src->pixels;
	int *lpDstData = (int*)lpData;
	
	for (int i = 0; i < src->height; i+=_size){
		dwHorizontalOffset = 0;
		
		for (int j = 0; j < src->width; j+=_size){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;
	
			// Update bitmap
			int dwSrcOffset = dwTotalOffset;
			int f_red = 0, f_green = 0, f_blue = 0;
			
			for (int k = 0; k < _size; k++){
				int m = i + k;
				if (m >= src->height-1) m = src->height - 1;
				
				for (int l = 0; l < _size; l++){
					int n = j + l;
					if (n >= src->width-1)
						n = src->width - 1;
					dwSrcOffset = m*src->pitch + n*m_iBpp;
					f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
					f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
					f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
				}
			}
			f_red = Mulfx(f_size,f_red);
			f_green = Mulfx(f_size,f_green);
			f_blue = Mulfx(f_size,f_blue);
			int newPixel = _RGB(fxtoi(f_red),fxtoi(f_green),fxtoi(f_blue));
			
			for (int k = 0; k<_size; k++){
				int m = i + k;
				if (m >= src->height-1)
					m = src->height - 1;
					
				for (int l = 0; l<_size; l++){
					int n = j + l;
					if (n >= src->width-1)
						n = src->width - 1;
					dwSrcOffset = m*src->pitch + n*m_iBpp;
					lpDstData[dwSrcOffset>>2] = newPixel;
				}
			}
	
			// Update horizontal offset
			dwHorizontalOffset += (_size*m_iBpp);
		}
	
		// Update vertical offset
		dwVerticalOffset += (_size*src->pitch);
	}
	
	// Update bitmap info
	my_Free(src->pixels, funcname, linenumber);
	src->pixels = (unsigned char*)lpData;
	
}

#if 0
void transContrast (TFRAME *src, const int contrast)
{
	// Check for valid bitmap
	int m_iBpp = 4;
	
	// Calculate contrast params
	int _contrast = MAX(1, MIN(100, contrast));
	int f_contrast = ftofx(1.0f/_contrast);
	int f_128 = itofx(96);
	
	// Change bitmap contrast
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpDstData = (int*)src->pixels;
	
	for (int i = 0; i < src->height; i++){
		dwHorizontalOffset = 0;
		
		for (int j = 0; j < src->width; j++){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;
	
			// Update bitmap
			int f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
			int f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
			int f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
			f_red = Mulfx(f_red-f_128, f_contrast) + f_128;
			f_green = Mulfx(f_green-f_128, f_contrast) + f_128;
			f_blue = Mulfx(f_blue-f_128, f_contrast) + f_128;
			
			int red = (int)MAX(0, MIN(fxtoi(f_red), 255));
			int green = (int)MAX(0, MIN(fxtoi(f_green), 255));
			int blue = (int)MAX(0, MIN(fxtoi(f_blue), 255));
			lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);
	
			// Update horizontal offset
			dwHorizontalOffset += m_iBpp;
		}
	
		// Update vertical offset
		dwVerticalOffset += src->pitch;
	}
	
}
#endif

#if 0
void transSharpen (TFRAME *src, const float value)
{
	// Check for valid bitmap
	int m_iBpp = 4;
	// Calculate sharp params
	int f_6 = ftofx(value);
	int f_1_2 = ftofx(1.0f/2.0f);
	
	// Create temporary bitmap
	int dwSize = src->pitch * src->height;
	char *lpData = (char*)my_Malloc(dwSize * sizeof(char)+src->pitch, funcname, linenumber);
	if (!lpData) return;
	
	// Sharp bitmap
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpSrcData = (int*)src->pixels;
	int *lpDstData = (int*)lpData;
	
	for (int i = 0; i < src->height; i++){
		dwHorizontalOffset = 0;
		
		for (int j = 0; j < src->width; j++){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;
	
			// Update bitmap
			int dwSrcOffset = dwTotalOffset;
			int f_red = 0, f_green = 0, f_blue = 0;
			
			for (int k=-1; k<=1; k++){
				int m = i + k;
				if (m < 0)
					m = 0;
				if (m >= src->height-1)
					m = src->height - 1;
					
				for (int l=-1; l<=1; l++){
					int n = j + l;
					
					if (n < 0)
						n = 0;
					if (n >= src->width-1)
						n = src->width - 1;
					dwSrcOffset = m*src->pitch + n*m_iBpp;
					
					if ((k == 0) && (l == 0)){
						f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_6);
						f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_6);
						f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_6);
					}else if (((k == -1) && (l == 0)) || ((k == 0) && (l == -1)) || ((k == 0) && (l == 1)) || ((k == 1) && (l == 0))){
						f_red -= itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						f_green -= itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						f_blue -= itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
					}
				}
			}
			
			int red = (int)MAX(0, MIN(fxtoi(Mulfx(f_red,f_1_2)), 255));
			int green = (int)MAX(0, MIN(fxtoi(Mulfx(f_green,f_1_2)), 255));
			int blue = (int)MAX(0, MIN(fxtoi(Mulfx(f_blue,f_1_2)), 255));
			lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);
	
			// Update horizontal offset
			dwHorizontalOffset += m_iBpp;
		}
	
		// Update vertical offset
		dwVerticalOffset += src->pitch;
	}
	
	// Update bitmap info
	my_Free(src->pixels, funcname, linenumber);
	src->pixels = (unsigned char*)lpData;
}
#endif

#if 0
void transGrayscale (TFRAME *src)
{
	// Check for valid bitmap
	int m_iBpp = 4;
	// Calculate grayscale params
	int f_w1 = ftofx(0.299f);
	int f_w2 = ftofx(0.587f);
	int f_w3 = ftofx(0.114f);
	
	// Grayscale bitmap
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpDstData = (int*)src->pixels;
	
	for (int i = 0; i < src->height; i++){
		dwHorizontalOffset = 0;
		
		for (int j = 0; j < src->width; j++){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;
	
			// Update bitmap
			int f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
			int f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
			int f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
			int f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
			int newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
			lpDstData[dwTotalOffset>>2] = newPixel;
	
			// Update horizontal offset
			dwHorizontalOffset += m_iBpp;
		}
	
		// Update vertical offset
		dwVerticalOffset += src->pitch;
	}
}
#endif

