@echo off
REM setting enviroment for compiler and toolkit
REM ibmcpp3.65 and os2tk45
call envicc365.cmd
REM call envtk45.cmd

@del *.obj
set DEBUG=
set __CODEPAGE__=850
@echo on
call nmake -nologo

@echo .
@echo ********************************************
@echo Now start copying files to Release directory
@Pause
@if NOT DIREXIST Release ( md Release )
@del /yq .\Release\*

@copy Data*.exe         .\Release\.
@copy *.hlp             .\Release\.
@copy *.nls             .\Release\.
@copy *.xqs             .\Release\.
@copy Readm*            .\Release\.
@copy datas*txt         .\Release\.
@copy COPYING           .\Release\.
@copy SYBASE_OW_LICENSE .\Release\.

@zip -jD .\Release\DataSeek.nls     .\NLS\*
@zip -jD .\Release\dataseek114.zip  .\Release\*

