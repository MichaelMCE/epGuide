@echo off


del *.bak
del *.o
del *.s
del *.def

rem del *.bmp
rem del *.tga
rem del *.pgm
rem del *.png
rem del *.raw

del *.exe

cd cc
@call clean.bat
cd..

cd common
@call clean.bat
cd..

cd ufont
@call clean.bat
cd ..

cd epg
@call clean.bat
cd..

cd win
@call clean.bat
cd..

