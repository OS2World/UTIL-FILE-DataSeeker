/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// .c :
// -
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------


// prototypes ---------------------------------------------------------------


// globals ------------------------------------------------------------------

//===========================================================================
// Display a message box with an error message.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// ULONG rc  : error message id.
// PSZ pParm : optional error message detail.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID printApplicationError(HWND hwnd, ULONG rc, PSZ pParm)
  {
  CHAR buf[512];
  ULONG cb;
  cb = WinLoadString(0, 0, rc - ERROR_PMSEEK + IDSERR_FIRST, 256, buf);
  if ( pParm )
    {
    sprintf(buf + cb, " (%.*s).", 507 - cb, pParm);
    }
  else
    {
    memcpy(buf + cb, ".", 2);
    } /* endif */
  TRACE1("Error:%s", buf);
  msgBox(hwnd, IDS_TITLE, buf, MB_MOVEABLE | MB_ERROR | MB_TITLEFROMRES);
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

VOID printDosApiError(HWND hwnd, ULONG rc)
  {
  CHAR buf[256];
  CHAR string[256];
  WinLoadString(0, 0, IDSERR_DOSAPI, 256, string);
  sprintf(buf, string, rc);
  TRACE1("Error:%s", buf);
  msgBox(hwnd, IDS_TITLE, buf, MB_MOVEABLE | MB_ERROR | MB_TITLEFROMRES);
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

ULONG _msgBox(HWND hwnd, PSZ pszTitle, PSZ pszMsg, ULONG fl)
  {
  CHAR title[256];
  CHAR msg[256];
  HAB hab;
  HMQ hmq;
  ULONG rc;
  if ( !hwnd ) hmq = WinCreateMsgQueue(hab = WinInitialize(0), 0);
  if ( fl & MB_TITLEFROMRES )
    {
    WinLoadString(0, 0, (ULONG)pszTitle, 256, title);
    pszTitle = title;
    fl &= ~MB_TITLEFROMRES;
    } /* endif */
  if ( fl & MB_MSGFROMRES )
    {
    WinLoadString(0, 0, (ULONG)pszMsg, 256, msg);
    pszMsg = msg;
    fl &= ~MB_MSGFROMRES;
    } /* endif */
  rc = WinMessageBox(HWND_DESKTOP, hwnd, pszMsg, pszTitle, 1, fl);
  if ( !hwnd )
    {
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
    } /* endif */
  return rc;
  }


//===========================================================================
// Standard PM thread initialization.
// Parameters --------------------------------------------------------------
// PHAB phab : anchor block
// Return value ------------------------------------------------------------
// HMQ : handle of the newly created message queue or NULLHANDLE in case
//       of error.
//===========================================================================

HMQ WinStdInit(PHAB phab)
  {
  return WinCreateMsgQueue(*phab = WinInitialize(0), 0);
  }


//===========================================================================
// Standard message loop.
// Parameters --------------------------------------------------------------
// HAB hab     : anchor block handle
// PQMSG pqmsg : message queue structure address
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID WinStdMsgLoop(HAB hab, PQMSG pqmsg)
  {
  while ( WinGetMsg(hab, pqmsg, NULLHANDLE, 0, 0) )
    WinDispatchMsg(hab, pqmsg);
  }


//===========================================================================
// Extended message loop. Intercept close window events from shutdown and
// from tasklist.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// HAB hab     : anchor block handle
// PQMSG pqmsg : message queue structure address
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID WinExtMsgLoop(HWND hwnd, HAB hab, PQMSG pqmsg)
  {
  for ( ;; )
    {
    // ordinary message loop
    if ( WinGetMsg(hab, pqmsg, NULLHANDLE, 0, 0) )
      {
      // 20100304 AB prevent crash when F1 is pressed while menu is down
      if ( pqmsg->msg == WM_HELP )
        {
        TRACE("WM_HELP");
        WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_HELPIDX), MPVOID);
        }
      else WinDispatchMsg(hab, pqmsg);
      }
    // shutdown
    else if ( !pqmsg->hwnd )
      {
      TRACE("WM_CLOSE shutdown");
      WinPostMsg(hwnd, WM_CLOSE, MPTRUE, MPVOID);
      }
    // tasklist
    else if ( pqmsg->hwnd == (HWND)pqmsg->mp2 )
      {
      TRACE("WM_CLOSE tasklist");
      WinPostMsg(hwnd, WM_CLOSE, MPFALSE, MPVOID);
      }
    else    // processing ordinary WM_CLOSE
      {
      TRACE("ordinary WM_CLOSE");
      break;
      } /* endif */
    } /* endfor */
  }


//===========================================================================
// Standard PM thread termination:
// - destroy the main window (hwnd)
// - destroy the message queue (hmq)
// - calls WinTerminate()
// Parameters --------------------------------------------------------------
// HWND hwnd : main window handle
// HMQ hmq   : message queue handle
// HAB hab   : anchor block handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID WinStdEnd(HWND hwnd, HMQ hmq, HAB hab)
  {
  WinDestroyAccelTable(WinQueryAccelTable(hab, hwnd));
  WinDestroyWindow(hwnd);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd    : dialog window handle
// ULONG id     : control ID
// HMODULE hmod : handle of the resource module
// ULONG idStr  : string ID
// Return value ------------------------------------------------------------
// ULONG : length of the loaded string
//===========================================================================

ULONG DlgItemTextLoad(HWND hwnd, ULONG id, ULONG idStr)
  {
  CHAR buf[512];
  char cBuf[256];
  ULONG ul;
  ul = WinLoadString(0, 0, idStr, 512, buf);
  NlsGet(buf, cBuf);
  DlgItemTextSet(hwnd, id, cBuf);
  return ul;
  }


//===========================================================================
// Read the content of a dialog item to a STRING object.
// Parameters --------------------------------------------------------------
// HWND hwnd  : window handle
// STRING str : string object
// ULONG id   : dialog item handle
// Return value ------------------------------------------------------------
// STRING : dialog item text STRING or NULL in case of error.
//===========================================================================

STRING DlgItemTextToString(HWND hwnd, STRING str, ULONG id)
  {
  ULONG cb;
  cb = DlgItemTextLength(hwnd, id);
  TRACE2("id:%d len=%d", id, cb );
  if ( !cb ) return NULL;
  if ( NULL != (str = StringSet(str, "\0" /*NULL*/, cb )) )   // 20091126 changed to "\0"
    {
    DlgItemText(hwnd, id, StringLen(str) + 1, String(str));
    //        DlgItemText(hwnd, id, StringLen(str), String(str));   //20091109 ToDo: why +1 ???
    TRACE1("StringLen=%d", StringLen(str) );
    }
  else
    {
    msgBox(hwnd, IDS_TITLE, IDSERR_MALLOC,
           MB_MOVEABLE | MB_ERROR | MB_MSGFROMRES | MB_TITLEFROMRES);
    } /* endif */
  TRACE1("DlgItemTextToString=%s", String(str));
  return str;
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

PVOID wMalloc(HWND hwnd, ULONG cb)
  {
  PVOID p;
  if ( NULL == (p = malloc(cb)) )
    msgBox(hwnd, IDS_TITLE, IDSERR_MALLOC,
           MB_MOVEABLE | MB_ERROR | MB_MSGFROMRES | MB_TITLEFROMRES);
  return p;
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

BOOL dLboxItemAddUnique(HWND hwnd, ULONG id, PSZ pszItem, ULONG flag)
  {
  if ( pszItem )
    {
    hwnd = DlgItemHwnd(hwnd, id);
    if ( 0 > wLbxItemTextSearch(hwnd, flag & 0xffff, LIT_FIRST, pszItem) )
      // 2008080x AB sort & non sort added
      if ( g.iHistSort == OPT_DONOTSORTHISTORY ) return(0 <= wLbxItemIns(hwnd, 0, pszItem));
      else return(0 <= wLbxItemIns(hwnd, LIT_SORTASCENDING, pszItem));

    } /* endif */
  return TRUE;
  }


