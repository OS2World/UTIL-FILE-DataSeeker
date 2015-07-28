@echo off
REM setting enviroment for compiler and toolkit
REM ibmcpp3.65 and os2tk45
call envicc365.cmd
REM call envtk45.cmd
REM CALL envwatcom

@setlocal
@prompt T:$t $p$g
echo IN MAKE.CMD

set DEBUG=1
set __HAVE_SENDMSG__=1
set __CODEPAGE__=850
@echo on
call nmake -nologo %1 %2 %3
prompt T:$t $p$g
@endlocal

