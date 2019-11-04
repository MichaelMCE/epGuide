@echo off

@call gccpath

ar -x libz.a
ar -x libpng.a

ar r libmylcd.a *.o
ranlib libmylcd.a

del *.o


