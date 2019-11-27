
CC=gcc
x64 := 0

ifeq ($(x64),1)
MDIR=x64
BITMODE = -m64
RESTARGET = 
ARCH = -march=native -mtune=native
else
MDIR=x32
BITMODE = -m32
RESTARGET = --target=pe-i386
#ARCH = -march=k8-sse3 -mtune=k8-sse3
ARCH = -march=native -mtune=native
endif


INCLUDE_DIR= -Iepg/sdk/include/ -I"../include/" -I"include/" 
LIB_DIR = -L"lib/$(MDIR)/" -Lepg/sdk/lib/ -L"lib" 
WINVER = -D_WIN32 -D_WIN32_WINNT=0x0701 -DWINVER=0x0701 -D__WIN32__=1 
SSEMMX = -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -DUSE_MMX1 -mmmx -msse -mfpmath=sse,387 
GLOP = -ftree-vectorize -floop-interchange -floop-strip-mine -floop-block -ftree-vectorizer-verbose=0
GCSE = -fgcse-las -fgcse-sm -fgcse-lm 
SCHED = -fmodulo-sched-allow-regmoves -fmodulo-sched
CFLAGSEXTRA = -funroll-loops -finline-functions -fomit-frame-pointer -Wno-strict-aliasing
CFLAGSOP = -Wall -O2 -std=gnu11 



CFLAGS = -flto $(ARCH) $(BITMODE) $(INCLUDE_DIR) $(GLOP) $(GCSE) $(WINVER) $(SSEMMX) $(SCHED) $(CFLAGSEXTRA) $(CFLAGSOP) -pipe -s 
LIBS = -flto $(BITMODE) $(LIB_DIR) -static-libgcc -lm -lgdi32 -lpsapi -lshlwapi -ldxguid -ldinput -lole32 -lcomctl32 -loleaut32 -luuid -lgdiplus -lwinmm -lddraw -lvlc.dll -lvlccore.dll

PRGOBJECTS = common/common.o \
	cc/cc.o \
	cc/label.o \
	cc/pane.o \
	common/artc.o \
	common/fileal.o \
	common/imagec.o \
	common/input.o \
	common/list.o \
	common/lock.o \
	common/stack.o \
	common/stringcache.o \
	common/transform.o \
	common/tree.o \
	epg/provider.o \
	epg/vlc.o \
	win/wingui.o \
	win/context.o \
	guide.o

PNG = ufont/libpng/png.o \
	ufont/libpng/pngerror.o \
	ufont/libpng/pngget.o \
	ufont/libpng/pngmem.o \
	ufont/libpng/pngpread.o \
	ufont/libpng/pngread.o \
	ufont/libpng/pngrio.o \
	ufont/libpng/pngrtran.o \
	ufont/libpng/pngrutil.o \
	ufont/libpng/pngset.o \
	ufont/libpng/pngtrans.o \
	ufont/libpng/pngwio.o \
	ufont/libpng/pngwrite.o \
	ufont/libpng/pngwtran.o \
	ufont/libpng/pngwutil.o

ZLIB = ufont/libpng/zutil.o \
	ufont/libpng/inftrees.o \
	ufont/libpng/inflate.o \
	ufont/libpng/inffast.o \
	ufont/libpng/crc32.o \
	ufont/libpng/adler32.o

UFONTOBJECTS = ufont/fio/fileio.o \
	ufont/libuf.o \
	ufont/ufont_primitives.o

# LIBMYLCD = lib/$(MDIR)/objs/*.o
LIBMYLCD = lib/$(MDIR)/libmylcd.a

EXAMPLES=guide.exe _guide.exe

guide.exe: $(PRGOBJECTS) $(UFONTOBJECTS) $(ZLIB) $(PNG) $(LIBMYLCD)
_guide.exe: $(PRGOBJECTS) $(UFONTOBJECTS) $(ZLIB) $(PNG) $(LIBMYLCD)


%.exe: 
	windres --output-format=coff $(RESTARGET) -i guide.rc -o res.o
	$(CC) -o $@ $^ res.o $(LIBS) -mwindows
	strip.exe $@


%.o: %.c $(DEPS) 
	$(CC) $(CFLAGS) -c -o $@ $< 


clean:
	rm -f *.exe *.o *.bak


all: $(EXAMPLES)

### PHONY define
.PHONY: all all-before all-after clean clean-custom



