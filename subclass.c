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
MRESULT EXPENTRY newLboxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY newEfProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY newEfProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY newTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// globals ------------------------------------------------------------------


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

VOID subclassWindows(HWND hwnd)
  {
  g.pDefEfProc = WinSubclassWindow(DlgItemHwnd(DlgItemHwnd(hwnd, DD_FILESRCH), 0x29b), newEfProc);
  g.pDefEfProc2 = WinSubclassWindow( WinWindowFromID(hwnd, EF_SEARCHING), newEfProc2);
  WinSubclassWindow(DlgItemHwnd(DlgItemHwnd(hwnd, DD_TEXTSRCH), 0x29b), newEfProc);
  WinSubclassWindow(DlgItemHwnd(DlgItemHwnd(hwnd, DD_SRCHRES), 0x29b),  newEfProc);
  g.pDefLboxProc = WinSubclassWindow(DlgItemHwnd(hwnd, LBOX_FILEFOUND), newLboxProc);
  WinSubclassWindow(DlgItemHwnd(hwnd, LBOX_TEXTFOUND), newLboxProc);
  g.pDefTxtProc = WinSubclassWindow(DlgItemHwnd(hwnd, TXT_FILESRCH), newTextProc);
  WinSubclassWindow(DlgItemHwnd(hwnd, TXT_TEXTSRCH), newTextProc);
  WinSubclassWindow(DlgItemHwnd(hwnd, TXT_SRCHRES), newTextProc);
  WinSubclassWindow(DlgItemHwnd(hwnd, TXT_FILEFOUND), newTextProc);
  WinSubclassWindow(DlgItemHwnd(hwnd, TXT_TEXTFOUND), newTextProc);
  //WinSubclassWindow(WinWindowFromID(hwnd, EF_SEARCHING), newEfProc2);
  }


//===========================================================================
// New listbox procedure.
// Parameters --------------------------------------------------------------
// Ordinary window procedure parameters.
// Return value ------------------------------------------------------------
// MRESULT.
//===========================================================================
static MRESULT EXPENTRY newLboxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {

  if ( (msg == WM_PRESPARAMCHANGED) && ((ULONG)mp1 == PP_FONTNAMESIZE) &&
       !(g.state & IS_RESETBOXFONTS) )
    {
    WinSendMsg(WinParent(hwnd), SWM_PPCHANGED, (MPARAM)hwnd, MPVOID);
    return MRFALSE;
    } /* endif */

  switch ( msg )
    {
    case WM_BUTTON2DOWN:
      {
        BOOL    fSuccess;       /* success indicator                    */
        POINTL ptlMouse;
        int iIndex;

        TRACE1("WM_BUTTON2DOWN in LBox hwnd=0x%X", hwnd);

        // set focus for main window
        WinSetFocus(HWND_DESKTOP, g.hwndMainWin);

        // check if item is selected and if yes, pop up Lbox Pop-up
        iIndex = wLbxItemSelected(hwnd);
        if ( iIndex != LIT_NONE )
          {
          TRACE1("selected item %d", iIndex);
          /* get current mouse pointer position */
          WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
          /* map from desktop to client window */
          fSuccess = WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptlMouse, 1);
          fSuccess = WinPopupMenu(hwnd, g.hwndMainWin, g.hwndMenuPuLbox,
                                  //fSuccess = WinPopupMenu(hwnd, hwnd, g.hwndMenuPuMain,
                                  ptlMouse.x - 5, ptlMouse.y - g.PopUpLboxCy + 10,
                                  PU_IDM_SETRECUR,
                                  PU_HCONSTRAIN       |
                                  PU_VCONSTRAIN       |
                                  PU_MOUSEBUTTON1     |
                                  PU_KEYBOARD         |
                                  0);
          TRACE1("fSuccess=%d", fSuccess);
          return MRFALSE;
          }
      }
      break;

    default:
      break;
    }

  return g.pDefLboxProc(hwnd, msg, mp1, mp2);
  }


//===========================================================================
// New entry field procedure.
// Parameters --------------------------------------------------------------
// Ordinary window procedure parameters.
// Return value ------------------------------------------------------------
// MRESULT.
//===========================================================================
static
            MRESULT EXPENTRY newEfProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  if ( msg == EN_CHANGE )
    {
    //TRACE("EN_CHANGE occoured");
    }
  if ( msg == CBN_ENTER )
    {
    TRACE("CBN_ENTER occoured");
    }
  if ( SHORT2FROMMP(mp1) == CBN_ENTER )
    {

    TRACE("CBN_ENTER");
    }
  if ( (msg == EM_SETSEL) || (msg == EM_SETFIRSTCHAR) || (msg == EM_PASTE) )
    {
    }
  if ( (msg == WM_PRESPARAMCHANGED) && ((ULONG)mp1 == PP_FONTNAMESIZE) )
    {
    if ( !(g.state & IS_RESETBOXFONTS) )
      {
      WinSendMsg(WinParent(WinParent(hwnd)),
                 SWM_PPCHANGED, (MPARAM)hwnd, MPTRUE);
      return MRFALSE;
      } /* endif */
    }

  return g.pDefEfProc(hwnd, msg, mp1, mp2);
  }


//===========================================================================
// New entry field procedure.
// Parameters --------------------------------------------------------------
// Ordinary window procedure parameters.
// Return value ------------------------------------------------------------
// MRESULT.
//===========================================================================
static
            MRESULT EXPENTRY newEfProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  CHAR buf[4];
  // presentation parameters changed if the presentation parameter is not
  // inherited from the dialog notify the dialog window
  if ( (msg == WM_PRESPARAMCHANGED) && ((ULONG)mp1 == PP_FONTNAMESIZE) &&
       !(g.state & IS_RESETDLGFONT) &&
       WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0, NULL, 4, buf, QPF_NOINHERIT) )
    {
    WinSendMsg(WinParent(hwnd), SWM_PPCHANGED2, (MPARAM)hwnd, MPVOID);
    return MRFALSE;
    } /* endif */

  return g.pDefEfProc2(hwnd, msg, mp1, mp2);
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
static
            MRESULT EXPENTRY newTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  CHAR buf[4];
  // presentation parameters changed if the presentation parameter is not
  // inherited from the dialog notify the dialog window
  if ( (msg == WM_PRESPARAMCHANGED) && ((ULONG)mp1 == PP_FONTNAMESIZE) &&
       !(g.state & IS_RESETDLGFONT) &&
       WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0, NULL, 4, buf, QPF_NOINHERIT) )
    {
    WinSendMsg(WinParent(hwnd), SWM_PPCHANGED2, (MPARAM)hwnd, MPVOID);
    return MRFALSE;
    } /* endif */

  return g.pDefTxtProc(hwnd, msg, mp1, mp2);
  }


