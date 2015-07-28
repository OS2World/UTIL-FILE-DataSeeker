# To add new extensions to the inference rules uncomment and redefine this:
#.SUFFIXES:
#
#.SUFFIXES: \
#    .C .obj .rc .res

# compiler, linker, resource compiler, resource binder MACRO

CC = icc.exe
CL = ilink.exe
#CL = ilink5.exe

# 20080726 use RC Version 5.x cause 4.0xx do not work
# 20081223 sometimes it's necessary to use rc4 for other/better error messages !!!
#RB = rc.exe
#RC = rc.exe
RB = rc5.exe
RC = rc5.exe
MS = mapsym.exe
MX = mapxqs.exe

# Options
MSFLAGS = /A /N /L
MXFLAGS = -V -l
# add a few #defines cause otherwise rc complains about #undef in pmseek.h
RCFLAGS = -r -n -I .\include -d _heapmin -d malloc -d free -d realloc -d strdup
RBFLAGS = -x2 -n

# compiler and linker flags

# Debug version
!ifdef DEBUG
#CFLAGS = /Ss /se /Ti /tm /gm+ /G5 /C /Q /qnoro /I.\include /D__DEBUG_TRACE__ /D__NO_EXCEPTION_HANDLERS__
CFLAGS = /Ss /se /Ti /tm /gm+ /G5 /C /Q /qnoro /I.\include /D__DEBUG_TRACE__
#LFLAGS = /DE /E:2 /PACKD /A:4 /OPTF /NOL /NOE /M /STack:0x4000 /DEFaultlibrarysearch /LINENUMBERS pmprintf.lib mmpm2.lib helpers36.lib
LFLAGS = /DEbug /PACKD /A:4 /OPTF /NOL /NOEXEpack /Map /LINENUMBERS pmprintf.lib mmpm2.lib
!else
# RELEASE version

CFLAGS = /Ss /se /Tn+ /Gm+ /G5 /C /Q /I.\include
#CFLAGS = /Ss /se /O /Oc /G5 /C /Q /gm+ /I.\include    <- 20091017 AB no need to disable debug info, only about 30 bytes
#LFLAGS = /E:2 /PACKD /A:4 /OPTF /NOL /Map /LINENUMBERS mmpm2.lib helpers36.lib ecolange.lib
LFLAGS = /DEbug /PACKD /A:4 /OPTF /NOLogo /Map /LINENUMBERS mmpm2.lib

!endif

# Some VisualAge C++ compiler options explained [default in brackets]:
# /c:   compile only, no link (we'll call the linker explicitly)
# /fi+: precompile header files                         [/fe-]
# /g3|4|5: optimize for 386/486/Pentium                 [/g3]
# /gd-: link runtime statically                         [/gd-]
# /ge-: create DLL code                                 [/ge+]
#           This switches between EXE and DLL initialization code
#           for the .OBJ file. /ge+ is only needed when the object
#           file contains a main() function. For libraries to be
#           used either with EXE's or DLL's, use /ge+.
# /gh+: generate profiling hooks for VAC profiler
# /gi+: fast integer execution
# /Gl+: remove unreferenced functions (when comp.+link in 1 step)
# /gm+: multithread libraries
# /gm+: disable stack probes (single-thread only!)
# /kc+: produce preprocessor warnings
# /o+:  optimization (inlining etc.)
# /oi-: no inlining (?)
# /ol+: use intermediate linker; do _not_ use with debug code
# /q+:  suppress icc logo
# /Re : don't use subsystem libraries (!= Rn)
# /se:  all language extensions
# /si+: allow use of precompiled header files
# /ss:  allow double slashes comments in C too
# /ti+: debug code
# /Tn+: Generate line-number-only debug information
# /tdp: compile everything as C++, even if it has a .C suffix
# /tm:  use debug memory management (malloc etc.)
# /tn:  add source line numbers to object files (for mapfile); a subset of /ti+
# /Wcls: class problems
# /Wcnd: conditional exprs problems (= / == etc.)
# /Wcmp: possible unsigned comparison redundancies
# /Wcns: operations involving constants
# /Wcnv: conversions
# /Wcpy: copy constructor problems
# /Weff: statements with no effect (annoying)
# /Wgen: generic debugging msgs
# /Wobs: obsolete features
# /Word: unspecified evaluation order
# /Wpar: list non-referenced parameters (annoying)
# /Wppc: list possible preprocessor problems (.h dependencies)
# /Wpro: warn if funcs have not been prototyped
# /Wrea: mark code that cannot be reached
# /Wret: check consistency of return levels
# /Wuni: uninitialized variables
# /Wuse: unused variables
# /w2:   produce error and warning messages, but no infos

.rc.res:
   $(RC) $(RCFLAGS) $<

.C.obj:
   $(CC) $(CFLAGS) $<

all: dataseek.exe ".\translation\dataseek.inf" DataSeek.nls

DataSeek.nls: .\NLS\de_DE .\NLS\EN_US .\NLS\ES_ES .\NLS\FR_FR .\NLS\IT_IT .\NLS\JA_JP .\NLS\NL_NL .\NLS\RU_RU .\NLS\SV_SE .\NLS\ZH_CN .\NLS\ZH_TW
   @if exist DataSeek.nls del DataSeek.nls
#   zip -D -q DataSeek.nls .\NLS\*  some zip versions do not honor -D
   @cd NLS
   zip -q ..\DataSeek.nls *
   @cd ..

dataseekerobjs =    application.obj commands.obj fileutil.obj initend.obj layout.obj loadsave.obj \
                    memmgr.obj objwin.obj pllist.obj pmaux.obj pmseek.obj pmutil.obj prodinfo.obj \
                    searchdata.obj settings.obj stringobj.obj stringutil.obj subclass.obj wordarray.obj \
                    wrappers.obj DriveInfo.obj NLS.obj aFoc.obj

DataSeek.exe: $(dataseekerobjs) DataSeeker.res
!ifdef __HAVE_SENDMSG__
# closes DataSeeker.exe if it's running, sendmsg.exe is from ... package (at hobbes?)
# wrapped around in a .cmd script to always return 0 and insert a little delay
   @echo closing running instance....
   @sendmsg.cmd "Data Seeker" WM_QUIT >NUL
!endif
# make prodinfo and pmseek every time to reflect build date in About dialog
   $(CC) $(CFLAGS) prodinfo.c
   $(CC) $(CFLAGS) pmseek.c
   call make_buildlevel.cmd
   $(CL) @<<
      $(LFLAGS) /PM:PM
      /OUT:$@ dataseeker.def $(dataseekerobjs)
<<
    $(RB) $(RBFLAGS) DataSeeker.res dataseek.exe
#    $(MS) $(MSFLAGS) $*.map >NUL
    $(MX) $(MXFLAGS) $*.map -o $*.xqs

".\translation\dataseek.inf" : ".\translation\dataseek.ipf"
    ipfc -i -D:1 -C:$(__CODEPAGE__) -L:ENU $*
    copy ".\translation\dataseek.inf" DataSeek.hlp

*.c *.res: pmseek.h include\*.h

# unfortunately nmake do not support autodepend
# the following is only a half hearted solution
# after messing around in header files you should touch pmseek.h or nmake -a
*.c: $*.h


# Clean target
clean :
    @if exist *.obj del *.obj
# delete all *.old files and copy current .exe/.dll to .old
    @if exist *.old del *.old
    @if exist *.exe rename *.exe *.exe.old
    @if exist *.dll rename *.dll *.dll.old
    @if exist *.res del *.res
    @if exist *.msg del *.msg
    @if exist *.inf del *.inf
    @if exist *.map del *.map
    @if exist *.NLS del *.NLS
    @echo *** All output files deleted ***

