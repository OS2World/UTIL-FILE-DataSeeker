/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// application.h : common application development routines
//
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _APPLICATION_H_
   #define _APPLICATION_H_


// prototypes ---------------------------------------------------------------
BOOL applExeFullName(PSZ pszPath, ULONG cch);
BOOL applExeEditExt(PSZ pszBuf, ULONG cch, PSZ pszExt);
STRING applIniFileName(BOOL bUserIniPath);
BOOL applWorkDirSet(PSZ pszDir);
BOOL applFileFind(PSZ appFile, PSZ envPath, PSZ ext);
STRING applWorkDir(PSZ pszDir);
BOOL applWorkDirPush(PSZ pszDir);
BOOL applWorkDirPop(VOID);
VOID applWorkDirStackDel(VOID);
HAPP applStartApp(HWND hwnd, PSZ pszExe, PSZ pszParms,
                  PSZ pszWorkDir, PSZ pszEnv, ULONG flMode);
BOOL applWinPosSize(HWND hwnd, PSWP pswp);
BOOL applWinPosSizeSet(HWND hwnd, PSWP pswp);
HWND applInitHelp(HWND hwnd, PSZ pszHelpLib,
                  HMODULE hmod, ULONG idHelp, PSZ pszTitle);
VOID applTerminateHelp(HWND hwnd, HWND hHelp);

// definitions --------------------------------------------------------------
// applStartApp() flags

#define APPSTART_DEFAULT              0x0000
#define APPSTART_CHILD                SAF_STARTCHILDAPP
#define APPSTART_MAX                  SAF_MAXIMIZED
#define APPSTART_MINI                 SAF_MINIMIZED
#define APPSTART_BKGND                SAF_BACKGROUND
#define APPSTART_HIDDEN               SAF_MINIMIZED
#define APPSTART_FULLSCREEN           0x2000

// 20080831 AB define new value to distinguish between editor and cli
#define APPSTART_WINSWITCHTOPROGRAMM  0x8000

#endif // #ifndef _APPLICATION_H_
