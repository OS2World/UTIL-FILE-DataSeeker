/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// application.c : common application development routines
// -
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================

#define INCL_WINSWITCHLIST
#include "pmseek.h"

// the ordinal is 209
#pragma import(WINHSWITCHFROMHAPP,,"PMSHAPI", 209)
HSWITCH APIENTRY16 WINHSWITCHFROMHAPP (HAPP happ);


// definitions --------------------------------------------------------------
typedef struct _APLLWORKDIR APPLWORKDIR, * PAPPLWORKDIR;

struct _APLLWORKDIR
  {
  union
    {
    PAPPLWORKDIR pPrev;
    PAPPLWORKDIR pLast;
    } ;
  };

// prototypes ---------------------------------------------------------------


// globals ------------------------------------------------------------------
APPLWORKDIR workDir;    // linked list of working directories


//===========================================================================
// Returns the full file name of the executable of the current process.
// Parameters --------------------------------------------------------------
// PSZ pszPath : output buffer
// ULONG cch   : buffer size
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applExeFullName(PSZ pszPath, ULONG cch)
  {
  PPIB ppib;
  PTIB ptib;
  if ( DosGetInfoBlocks(&ptib, &ppib) ||
       DosQueryModuleName(ppib->pib_hmte, cch, pszPath) )
    return FALSE;
  return TRUE;
  }



//===========================================================================
// Builds a file name with the same name of the file executing the current
// process end with the extension pszExt.
// Parameters --------------------------------------------------------------
// PSZ pszBuf : output buffer
// ULONG cch  : buffer size
// PSZ pszExt : extension for the new file name
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applExeEditExt(PSZ pszBuf, ULONG cch, PSZ pszExt)
  {
  PSZ psz;
  if ( applExeFullName(pszBuf, cch) )
    {
    fmFileNameEditExt(pszBuf, pszExt);
    return TRUE;
    } /* endif */
  return FALSE;
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

STRING applIniFileName(BOOL bUserIniPath)
  {
  CHAR buf[CCHMAXPATH];
  PSZ pszIniName;
  PSZ pszUserIni;
  PSZ pszOS2INI;
  TRACE("1");
  applExeEditExt(buf, CCHMAXPATH, "INI");
  if ( bUserIniPath && !DosScanEnv("USER_INI", &pszUserIni) )
    {
    pszIniName = fmFileNameFromPath(buf);
    pszOS2INI = fmFileNameFromPath(pszUserIni);
    memmove(buf + (pszOS2INI-pszUserIni), pszUserIni, strlen(pszUserIni) + 1);
    memcpy(buf, pszUserIni, pszOS2INI - pszUserIni);
    } /* endif */
  TRACE("2");
  return StringNew(buf, -1);
  }


//===========================================================================
// Set a new working drive and directory.
// Parameters --------------------------------------------------------------
// PSZ pszDir : new working drive and directory.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applWorkDirSet(PSZ pszDir)
  {
  BOOL rc = FALSE;
  // check if the drive letter was specified
  DosError(FERR_DISABLEHARDERR);
  if ( (pszDir[1] == ':') && DosSetDefaultDisk((*pszDir & ~0x20) - 'A' + 1) )
    goto end;
  rc = !DosSetCurrentDir(pszDir);
  end:
  DosError(FERR_ENABLEHARDERR);
  return rc;
  }


//===========================================================================
// Get the current working drive and directory.
// If 'pszDir' is NULL a STRING object is allocated and is initialized
// with the current directory. If 'pszDir' is a buffer address (the buffer
// size must be at least CCHMAXPATH) the current directory name is copied
// to 'pszDir' and (STRING)TRUE is returned.
// Parameters --------------------------------------------------------------
// PSZ pszDir : buffer where to copy the current drive and directory data.
//              NULL if a new STRING object is required.
// Return value ------------------------------------------------------------
// STRING : NULL in case of error, ((STRING)1) or real STRING object in case
//          of success.
//===========================================================================

STRING applWorkDir(PSZ pszDir)
  {
  STRING str = NULL;
  ULONG curDisk;
  ULONG cbBuf;
  if ( !pszDir )
    {
    if ( NULL == (str = StringNew(NULL, CCHMAXPATH)) ) return NULL;
    pszDir = String(str);
    } /* endif */
  // use the Dos* API to get the current working directory
  if ( xDosQueryPathInfo(".", FIL_QUERYFULLNAME, pszDir, CCHMAXPATH) )
    {
    if ( str ) StringDel(&str);
    return NULL;
    } /* endif */
  if ( str )
    {
    StringLengthReset(str);
    return str;
    }
  else
    {
    return(STRING)TRUE;
    } /* endif */
  }


//===========================================================================
// Set a new working drive and directory after having saved the current one.
// Parameters --------------------------------------------------------------
// PSZ pszDir : new working drive and directory.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

BOOL applWorkDirPush(PSZ pszDir)
  {
  ULONG cb;
  PAPPLWORKDIR pwd;
  // if the stack is not yet initialized store the current working directory
  if ( !workDir.pLast )
    {
    if ( !(workDir.pLast = (PAPPLWORKDIR)applWorkDir(NULL)) ) return FALSE;
    workDir.pLast->pPrev = NULL;
    } /* endif */
  cb = strlen(pszDir) + 1;
  if ( NULL == (pwd = malloc(sizeof(APPLWORKDIR) + cb)) ) return FALSE;
  if ( !applWorkDirSet(pszDir) )
    {
    free(pwd);
    return FALSE;
    } /* endif */
  pwd->pPrev = workDir.pLast;
  workDir.pLast = pwd;
  memcpy(pwd + 1, pszDir, cb);
  return TRUE;
  }


//===========================================================================
// Restore the previous working drive and directory.
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applWorkDirPop(VOID)
  {
  PAPPLWORKDIR pwd;
  // nothing to POP : return FALSE;
  if ( !workDir.pLast ||
       !(pwd = workDir.pLast->pPrev) ||
       !applWorkDirSet((PSZ)(pwd + 1)) )
    return FALSE;
  free(workDir.pLast);
  workDir.pLast = pwd;
  return TRUE;
  }


//===========================================================================
// Free all the resources allocated for the working directory stack.
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID applWorkDirStackDel(VOID)
  {
  PAPPLWORKDIR pwd;
  for ( pwd = workDir.pLast; pwd; workDir.pLast = pwd )
    {
    pwd = workDir.pLast->pPrev;
    free(workDir.pLast);
    } /* endfor */
  workDir.pLast = NULL;
  }

//===========================================================================
// Start an application.
// Parameters --------------------------------------------------------------
// HWND hwnd      : notification window handle
// PSZ pszExe     : executable name
// PSZ pszParms   : parameters
// PSZ pszWorkDir : working directory
// PSZ pszEnv     : environment values                          // values from os2tk45
// ULONG flMode   : start mode      APPSTART_DEFAULT            SAF_VALIDFLAGS  0x001F
//                                  mutually exclusive:         SAF_INSTALLEDCMDLINE  0x0001
//                                  APPSTART_MAX                SAF_STARTCHILDAPP     0x0002
//                                  APPSTART_MINI               SAF_MAXIMIZED         0x0004
//                                  APPSTART_BKGND              SAF_MINIMIZED         0x0008
//                                  APPSTART_HIDDEN             SAF_BACKGROUND        0x0010
//                                  other:
//                                  APPSTART_CHILD
//                                  APPSTART_FULLSCREEN
// Return value ------------------------------------------------------------
// HAPP : handle of the started application.
//===========================================================================

HAPP applStartApp(HWND hwnd, PSZ pszExe, PSZ pszParms,
                  PSZ pszWorkDir, PSZ pszEnv, ULONG flMode)
  {
  HAB   hab;              /* anchor-block handle                  */
  ERRORID  erridErrorCode;/* last error id code                   */
  HAPP            happ;
  HWND            hswitch;
  ULONG           rc;
  PROGDETAILS pd;
  memset(&pd, 0, sizeof(PROGDETAILS));
  pd.Length = sizeof(PROGDETAILS);
  pd.progt.progc = (flMode & APPSTART_FULLSCREEN) ?
                   PROG_FULLSCREEN : PROG_DEFAULT;
  pd.progt.fbVisible = SHE_VISIBLE;
  pd.pszExecutable = pszExe;
  pd.pszParameters = pszParms;
  pd.pszStartupDir = pszWorkDir;
  pd.pszEnvironment = pszEnv;
  //  pd.swpInitial.fl               = SWP_ACTIVATE | SWP_ZORDER ;         /* Window positioning   */
  //  pd.swpInitial.cy               = 0;                  /* Width of window      */
  //  pd.swpInitial.cx               = 0;                  /* Height of window     */
  //  pd.swpInitial.y                = 0;                  /* Lower edge of window */
  //  pd.swpInitial.x                = 0;                  /* Left edge of window  */
  //  pd.swpInitial.hwndInsertBehind = HWND_TOP;
  happ = WinStartApp(hwnd, &pd, NULL, NULL,
                     SAF_INSTALLEDCMDLINE | (flMode & 0xff));

  // 200808xx get the switch handle from the happ (from www Ilya/Alex)
  // to put cli window in front und set focus on it
  if ( flMode & APPSTART_WINSWITCHTOPROGRAMM )
    {
    TRACE("APPSTART_WINSWITCHTOPROGRAMM");
    hswitch = WINHSWITCHFROMHAPP(happ);
    if ( hswitch )
      {
      rc = WinSwitchToProgram(hswitch); if ( rc )
        {
        TRACE1("return 0x%lX", rc);
        }
      }
    }
  /* get last nonzero error for this anchor block */
  erridErrorCode = WinGetLastError(hab);

  TRACE2("Application happ:0x%X, LastError:0x%X", happ, erridErrorCode);


  /*
  PMERR_DOS_ERROR (0x1200)
  A DOS call returned an error.

  PMERR_INVALID_APPL (0x1530)
  Attempted to start an application whose type is not recognized by OS/2.

  PMERR_INVALID_PARAMETERS (0x1208)
  An application parameter value is invalid for its converted PM type. For example: a 4-byte value outside the range -32 768 to +32 767 cannot be converted to a SHORT, and a negative number cannot be converted to a ULONG or USHORT.

  PMERR_INVALID_WINDOW (0x1206)
  The window specified with a Window List call is not a valid frame window.

  PMERR_STARTED_IN_BACKGROUND (0x1532)
  The application started a new session in the background.
  */

  return happ;
  }


//===========================================================================
// Get window position and size. If the window is minimized set the SWP
// structure members to the restore position and size.
// This is to be used for saving window position and size in an INI file.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// PSWP pswp : (output) window position, size, visualization flags.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applWinPosSize(HWND hwnd, PSWP pswp)
  {
  if ( WinQueryWindowPos(hwnd, pswp) )
    {
    if ( pswp->fl & (SWP_MINIMIZE | SWP_MAXIMIZE) )
      {
      pswp->x = WinXRestore(hwnd);
      pswp->y = WinYRestore(hwnd);
      pswp->cx = WinCxRestore(hwnd);
      pswp->cy = WinCyRestore(hwnd);
      } /* endif */
    return TRUE;
    } /* endif */
  return FALSE;
  }


//===========================================================================
// Restore a window position size previously retrieved via applWinPosSize().
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle.
// PSWP pswp : window position and size data.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL applWinPosSizeSet(HWND hwnd, PSWP pswp)
  {
  APIRET rc;

  TRACE1("hwnd=0x%X", hwnd);
  TRACE4("x=%d, y=%d, cx=%d, cy=%d", pswp->x, pswp->y, pswp->cx, pswp->cy);

  if ( pswp->fl & (SWP_MINIMIZE | SWP_MAXIMIZE) )
    {
    TRACE("restore pos/size");
    WinXRestoreSet(hwnd, pswp->x);
    WinYRestoreSet(hwnd, pswp->y);
    WinCxRestoreSet(hwnd, pswp->cx);
    WinCyRestoreSet(hwnd, pswp->cy);
    } /* endif */


  TRACE4("g.startData.iniData->swp->x=%d, y=%d, cx=%d, cy=%d", g.startData.iniData->swp.x,
         g.startData.iniData->swp.y, g.startData.iniData->swp.cx, g.startData.iniData->swp.cy);

  rc = WinSetWindowPos(hwnd, pswp->hwndInsertBehind, pswp->x, pswp->y, pswp->cx, pswp->cy, pswp->fl);
  return rc;
  }

/* --------------------------------------------------------------------------
 Find an application file (i.e. a file with the executable name but with
 a different extension in the given PATH environment.
- Parameters -------------------------------------------------------------
 PSZ appFile : output (full path file name of the application file)
 PSZ envPath : environment path to search for the file.
 PSZ ext     : application file extension.
- Return value -----------------------------------------------------------
 BOOL : TRUE/FALSE (success/error).
-------------------------------------------------------------------------- */
BOOL applFileFind(PSZ appFile, PSZ envPath, PSZ ext)
  {
  CHAR buf[CCHMAXPATH];
  PSZ pname;
  if ( applExeEditExt(buf, sizeof(buf), "HLP") )
    {
    pname = fmFileNameFromPath(buf);
    if ( !DosSearchPath(SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY,
                        envPath, pname, appFile, CCHMAXPATH) )
      return TRUE;
    }
  return FALSE;
  }


//===========================================================================
// Initialize the help environment of an application.
// Parameters --------------------------------------------------------------
// HWND hwnd      : window handle.
// PSZ pszHelpLib : name of the help file. If this is NULL the help file
//                  name is built from the application name.
// HMODULE hmod   : handle of the resource module containing the help table.
// ULONG idHelp   : help table id.
// PSZ pszTitle   : title of the help window. If this is NULL the main
//                  window title is used.
// Return value ------------------------------------------------------------
// HWND : handle of the help instance. NULLHANDLE in case of error.
//===========================================================================

HWND applInitHelp(HWND hwnd, PSZ pszHelpLib,
                  HMODULE hmod, ULONG idHelp, PSZ pszTitle)
  {
  HWND hHelp;
  CHAR achHelpFile[CCHMAXPATH];
  CHAR achTitle[128];
  HELPINIT hi;
  memset(&hi, 0, sizeof(HELPINIT));
  hi.cb = sizeof(HELPINIT);
  // set the help table
  if ( idHelp < 0xffff )
    {
    hi.phtHelpTable = PHLPTBLFROMID(idHelp);
    }
  else
    {
    hi.phtHelpTable = (PHELPTABLE)idHelp;
    } /* endif */
  TRACE1("hi.phtHelpTable:0x%X", hi.phtHelpTable);
  // handle of the resource module containing the help table
  hi.hmodHelpTableModule = hmod;
  TRACE1("hi.hmodHelpTableModule:0x%X", hi.hmodHelpTableModule);
  // set the help window title
  if ( !pszTitle )
    {
    achTitle[sizeof(achTitle) - 1] = 0;
    WinQueryWindowText(hwnd, sizeof(achTitle) - 1, achTitle);
    pszTitle = achTitle;
    TRACE1("pszTitle:%s", pszTitle);
    } /* endif */
  hi.pszHelpWindowTitle = pszTitle;
  hi.fShowPanelId = CMIC_HIDE_PANEL_ID;
  TRACE1("hi.pszHelpWindowTitle:%s", hi.pszHelpWindowTitle);
  TRACE1("hi.fShowPanelId:%d", hi.fShowPanelId);

  if ( !fmExists(pszHelpLib) )
    {
    if ( applFileFind(achHelpFile, "HELP", "HLP") ) pszHelpLib = achHelpFile;
    else
      {
      TRACE("no help file name");
      TRACE("is help file in current directory? (same name as .exe)");
      return NULLHANDLE;
      }
    }
  hi.pszHelpLibraryName = pszHelpLib;
  TRACE1("hi.pszHelpLibraryName:%s", pszHelpLib);
  if ( !(hHelp = WinCreateHelpInstance(WinQueryAnchorBlock(hwnd), &hi)) )
    {
    TRACE("can not create WinCreateHelpInstance");
    return NULLHANDLE;
    }
  if ( hi.ulReturnCode )
    {
    TRACE("WinDestroyHelpInstance");
    WinDestroyHelpInstance(hHelp);
    hHelp = NULLHANDLE;
    }
  else
    {
    TRACE("WinAssociateHelpInstance");
    WinAssociateHelpInstance(hHelp, hwnd);
    } /* endif */
  return hHelp;
  }


//===========================================================================
// Deassociate and destroy an help instance.
// Parameters --------------------------------------------------------------
// HWND hwnd  : window handle
// HWND hHelp : help instance handle.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID applTerminateHelp(HWND hwnd, HWND hHelp)
  {
  if ( hHelp )
    {
    WinAssociateHelpInstance(NULLHANDLE, hwnd);
    WinDestroyHelpInstance(hHelp);
    } /* endif */
  }


