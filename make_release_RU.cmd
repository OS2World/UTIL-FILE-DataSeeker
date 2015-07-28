@echo off
REM setting enviroment for compiler and toolkit
REM ibmcpp3.65 and os2tk45
call envicc365.cmd
REM call envtk45.cmd

del *.obj
set DEBUG=
set __CODEPAGE__=866
@echo on
nmake -nologo


