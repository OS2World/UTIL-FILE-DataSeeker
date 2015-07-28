@echo off
mode co80,102
set debug=

rem - check if the flag file exists
if exist @debug@ del @debug@ > nul
rem - touch all sources so new object files are generated
for %%1 in (*.c) do touch %%1

goto build

:tryagain
echo.
echo Press a key to start NMAKE
echo Press Ctrl-C to terminate
pause > nul

:build
cls
nmake -nologo
if errorlevel 1 goto tryagain

lxlite dataseek.exe
