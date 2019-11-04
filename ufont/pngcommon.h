
#ifndef _PNGCOMMON_H_
#define _PNGCOMMON_H_


#include "ufont_config.h"


//#define BPP_8		8
//#define BPP_12		12
//#define BPP_15		15
#define BPP_16		16
//#define BPP_24		24
//#define BPP_32		32



#include "libpng/png.h"

int32_t png_read (const char *filename, uint8_t *buffer, const int32_t bufferbpp, const int32_t ox, const int32_t oy);
int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp);


#if 0
int32_t png_read (const char *filename, uint8_t *buffer, const int32_t bufferbpp, const int32_t ox, const int32_t oy) {return 0;}
int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp) {return 0;}
#endif



#endif

