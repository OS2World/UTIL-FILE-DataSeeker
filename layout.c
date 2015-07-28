/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// layout.c :
// procedures which control the size of the dialog and the layout of its
// inner controls
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------
#define IDX(id)  ((id) - TXT_FILESRCH)
#define CCTRLS (IDX(LAST_CONTROL))  // count of controls to be moved and sized
// 20081201 AB changed

// prototypes ---------------------------------------------------------------


// globals ------------------------------------------------------------------

const char sSeparationBar[MAX_LENGHT_SEPARATION_BAR + 1] = {
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 16
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 32
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 48
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 64
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 80
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 96
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 112
  0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,   // 128
  0x00
};

//===========================================================================
// When dialog size changes rearrange the size and postition of the inner
// controls.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// LONG cx   : new dialog width
// LONG cy   : new dialog height
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID arrangeControls(HWND hwnd, LONG cx, LONG cy)
  {
  SWP aswp[CCTRLS];
  RECTL r;
  INT i, cyResult;
  int iTemp;
  ApiExPM_SIZES du;
  BOOL bRc;

  du.cx = g.font.cx >> 1;
  du.cy = g.font.cy >> 2; // ñ= 2 dialog units
  // calculate the inner rectangle
  if ( cx < 0 )
    {
    WinQueryWindowRect(hwnd, &r);
    }
  else
    {
    RectSet(&r, 0, 0, cx, cy);
    } /* endif */
  WinCalcFrameRect(hwnd, &r, TRUE);
  // initialize SWP hwnd, x, width and flag
  //TRACE1("number of controls %d", CCTRLS);
  for ( i = 0; i < CCTRLS; i++ )
    {
    //TRACE2("[%d] arrange %d", i, TXT_FILESRCH + i);
    aswp[i].hwnd = DlgItemHwnd(hwnd, TXT_FILESRCH + i);
    aswp[i].x = r.xLeft + du.cx;
    aswp[i].cx = r.xRight - r.xLeft - g.font.cx;
    aswp[i].fl = SWP_SIZE | SWP_MOVE;
    } /* endfor */

  //TRACE2("r.x: 0x%0X, r.x % 8: 0x%0X", r.xLeft, r.xLeft % 8);
  // TXT_FILESRCH   // 20081201 AB changed to half of the width
  //                      not handled till now is g.font.cx, necessary ?
  aswp[IDX(TXT_FILESRCH)].cx /= 2;

  // BUTTON : Search
  aswp[IDX(PB_SEARCH)].y = r.yBottom + du.cy;
  aswp[IDX(PB_SEARCH)].cx = 6 + (g.font.cx * 16);
  aswp[IDX(PB_SEARCH)].cy = 12 + g.font.cy;
  // BUTTON : Stop
  aswp[IDX(PB_STOP)].x = mwposXright(aswp, IDX(PB_SEARCH)) + du.cx;
  aswp[IDX(PB_STOP)].y = aswp[IDX(PB_SEARCH)].y;
  aswp[IDX(PB_STOP)].cx = aswp[IDX(PB_SEARCH)].cx;
  aswp[IDX(PB_STOP)].cy = aswp[IDX(PB_SEARCH)].cy;
  // BUTTON : Help
  // 20080728 AB no Help Button anymore
  //aswp[IDX(PB_TEST)].x = mwposXright(aswp, IDX(PB_STOP)) + g.font.cx + du.cx;
  aswp[IDX(PB_TEST)].y = aswp[IDX(PB_SEARCH)].y;
  aswp[IDX(PB_TEST)].cx = aswp[IDX(PB_SEARCH)].cx;
  aswp[IDX(PB_TEST)].x = mwposXright(aswp, IDX(TXT_FILEFOUND)) - aswp[IDX(PB_TEST)].cx;
  aswp[IDX(PB_TEST)].cy = aswp[IDX(PB_SEARCH)].cy;

  // 20091017 AB info area
  aswp[IDX(TXT_INFO)].x = mwposXright(aswp, IDX(PB_STOP)) + g.font.cx + du.cx;
  aswp[IDX(TXT_INFO)].y = aswp[IDX(PB_SEARCH)].y + 4;
  aswp[IDX(TXT_INFO)].cx = mwposXright(aswp, IDX(TXT_FILEFOUND)) - aswp[IDX(TXT_INFO)].x;
  aswp[IDX(TXT_INFO)].cy = aswp[IDX(PB_SEARCH)].cy - 8;   //

  // LISTBOX : Text found
  aswp[IDX(LBOX_TEXTFOUND)].y = mwposYtop(aswp, IDX(PB_SEARCH)) + du.cy + du.cy;
  cyResult = r.yTop - r.yBottom - 28 - aswp[IDX(LBOX_TEXTFOUND)].y -
             (du.cy * 17) - (g.font.cy << 3);
  if ( g.state & IS_RESLBOXTEXTHIDDEN )
    {   // hidden text result listbox
    aswp[IDX(LBOX_TEXTFOUND)].cy = 0;
    aswp[IDX(TXT_TEXTFOUND)].cy = 0;
    aswp[IDX(LBOX_FILEFOUND)].y = aswp[IDX(LBOX_TEXTFOUND)].y;
    aswp[IDX(LBOX_FILEFOUND)].cy = cyResult + g.font.cy + 2 + 3 * du.cy;
    aswp[IDX(TXT_TEXTFOUND)].y = 0;
    }
  else
    {
    if ( g.state & IS_RESLBOXFFMIN )
      {     // minimized file found listbox
      aswp[IDX(LBOX_TEXTFOUND)].cy = cyResult - g.font.cy - 4;
      aswp[IDX(LBOX_FILEFOUND)].cy = g.font.cy + 4;
      }
    else
      {
      aswp[IDX(LBOX_TEXTFOUND)].cy = cyResult >> 1;
      aswp[IDX(LBOX_FILEFOUND)].cy = cyResult - aswp[IDX(LBOX_TEXTFOUND)].cy;
      } /* endif */
    aswp[IDX(TXT_TEXTFOUND)].y = mwposYtop(aswp, IDX(LBOX_TEXTFOUND)) + du.cy;
    aswp[IDX(TXT_TEXTFOUND)].cy = g.font.cy + 2;
    aswp[IDX(LBOX_FILEFOUND)].y = mwposYtop(aswp, IDX(TXT_TEXTFOUND)) +
                                  du.cy + du.cy;
    } /* endif */

  // TEXT : Files found
  aswp[IDX(TXT_FILEFOUND)].cx -= du.cx * 20;
  aswp[IDX(TXT_FILEFOUND)].y = mwposYtop(aswp, IDX(LBOX_FILEFOUND)) + du.cy;
  aswp[IDX(TXT_FILEFOUND)].cy = g.font.cy + 2;
  // RESIZER BUTTONS : down
  aswp[IDX(PB_MOVEDOWN)].x = mwposXright(aswp, IDX(TXT_FILEFOUND));
  aswp[IDX(PB_MOVEDOWN)].cx = du.cx * 10;
  aswp[IDX(PB_MOVEDOWN)].y = aswp[IDX(TXT_FILEFOUND)].y;
  aswp[IDX(PB_MOVEDOWN)].cy = g.font.cy + 2 + du.cy;
  // RESIZER BUTTONS : up
  aswp[IDX(PB_MOVEUP)].x = mwposXright(aswp, IDX(PB_MOVEDOWN));
  aswp[IDX(PB_MOVEUP)].cx = aswp[IDX(PB_MOVEDOWN)].cx;
  aswp[IDX(PB_MOVEUP)].y = aswp[IDX(TXT_FILEFOUND)].y;
  aswp[IDX(PB_MOVEUP)].cy = aswp[IDX(PB_MOVEDOWN)].cy;
  // TEXT : progress text (normally hidden)
  aswp[IDX(EF_SEARCHING)].y = mwposYtop(aswp, IDX(TXT_FILEFOUND)) +
                              du.cy + du.cy;
  aswp[IDX(EF_SEARCHING)].cy = g.font.cy + 2;
  // DROPDOWN : Select results
  // 2008090x AB changed to DDBOXES_OFFSET to prevent overlap of Search/Stop button
  aswp[IDX(DD_SRCHRES)].y =  DDBOXES_OFFSET;
  aswp[IDX(DD_SRCHRES)].cy = mwposYtop(aswp, IDX(TXT_FILEFOUND)) +
                             du.cy + du.cy + 6 + g.font.cy - DDBOXES_OFFSET;
  // TEXT : Select results
  aswp[IDX(TXT_SRCHRES)].y = mwposYtop(aswp, IDX(DD_SRCHRES)) + du.cy + 3;
  aswp[IDX(TXT_SRCHRES)].cy = g.font.cy + 2;
  // DROPDOWN : Text to search for (if any):
  aswp[IDX(DD_TEXTSRCH)].y = DDBOXES_OFFSET;
  aswp[IDX(DD_TEXTSRCH)].cy = mwposYtop(aswp, IDX(TXT_SRCHRES)) +
                              du.cy + du.cy + 6 + g.font.cy - DDBOXES_OFFSET;
  // TEXT : Text to search for (if any):
  aswp[IDX(TXT_TEXTSRCH)].y = mwposYtop(aswp, IDX(DD_TEXTSRCH)) + du.cy + 3;
  aswp[IDX(TXT_TEXTSRCH)].cy = g.font.cy + 2;
  // DROPDOWN : Files to search for:
  aswp[IDX(DD_FILESRCH)].y = DDBOXES_OFFSET;
  aswp[IDX(DD_FILESRCH)].cy = mwposYtop(aswp, IDX(TXT_TEXTSRCH)) +
                              du.cy + du.cy + 6 + g.font.cy - DDBOXES_OFFSET;
  // 20111124 AB place FOC button and make space for it from DD_FILESRCH
  aswp[IDX(PB_FOC)].cx  = du.cx * 10;
  aswp[IDX(PB_FOC)].x = aswp[IDX(DD_FILESRCH)].x + aswp[IDX(DD_FILESRCH)].cx - aswp[IDX(PB_FOC)].cx;
  aswp[IDX(PB_FOC)].cy = aswp[IDX(PB_MOVEDOWN)].cy;
  aswp[IDX(PB_FOC)].y = aswp[IDX(DD_FILESRCH)].y + aswp[IDX(DD_FILESRCH)].cy - aswp[IDX(PB_FOC)].cy;
  aswp[IDX(DD_FILESRCH)].cx -= (aswp[IDX(PB_FOC)].cx + 2);

  // TEXT : Files to search for:
  aswp[IDX(TXT_FILESRCH)].y = mwposYtop(aswp, IDX(DD_FILESRCH)) + du.cy + 3;
  aswp[IDX(TXT_FILESRCH)].cy = g.font.cy + 2;

  // DD_DRIVES : List box to specify drives to search for   20081201 AB added
  aswp[IDX(PB_DRIVES)].y = aswp[IDX(TXT_FILESRCH)].y - 4;
  aswp[IDX(PB_DRIVES)].cy = g.font.cy + 2 + 8;
  iTemp = (r.xRight - r.xLeft + g.font.cx + 1) / 2;
  aswp[IDX(PB_DRIVES)].x =  iTemp - iTemp % 8 + 4;    // set on byte boundary cause secondary window starts on byte boundary too
  aswp[IDX(PB_DRIVES)].cx = aswp[IDX(TXT_FILESRCH)].cx /*- aswp[IDX(TXT_FILESRCH)].cx % 8*/;  // same as 'Files to search' text (half the width)

  //TRACE4("PB_FOC: x=%d, y=%d, cx=%d, cy=%d", aswp[IDX(PB_FOC)].x, aswp[IDX(PB_FOC)].y, aswp[IDX(PB_FOC)].cx, aswp[IDX(PB_FOC)].cy);

  if ( ! (WinSetMultWindowPos(WinQueryAnchorBlock(hwnd), aswp, CCTRLS )) )        // 20090126 AB - 2 added, does not work with WIN_DRIVES windows
  // 20091017 AB changed CCTRLS
    {
    TRACE1("error: 0x%lX", WinGetLastError(WinQueryAnchorBlock(hwnd)));
    }

  arrangeWinDrives(g.hwndWinDrives);
  }

//===========================================================================
// When dialog size changes rearrange the size and postition of the WIN_DRIVES
// window
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle of parent (main) dialog window
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID arrangeWinDrives(HWND hwnd)
  {
  SWP aswp[2];    //
  RECTL rButton, rMain;
  INT i, cyResult, iY;
  ApiExPM_SIZES du;
  BOOL rc;
  char buf[50];

  // 20100415 only if MM dlls are found
  if ( !g.iHaveMMdlls ) return;

  du.cx = g.font.cx >> 1;
  du.cy = g.font.cy >> 2; // ñ= 2 dialog units
  // calculate the inner rectangle
  WinQueryWindowRect(g.hwndMainWin, &rMain);
  WinQueryWindowPos(DlgItemHwnd(g.hwndMainWin, PB_DRIVES), (PSWP) &g.PbDrivesR);

  //TRACE2("win .x=%d win .y=%d", rMain.xLeft, rMain.yTop);
  //TRACE2("win.cx=%d win.cy=%d", rMain.xRight - rMain.xLeft, rMain.yTop - rMain.yBottom);
  if ( ! (g.state & IS_INIT_DRIVES_DONE) )
    {   // first call at init, make window as big to fit all entries
        // secondary window will add the scroll bar if necessary
    TRACE("first call after program start");
    aswp[0].x =     g.PbDrivesR.x;
    aswp[0].cx =    g.PbDrivesR.cx;
    aswp[0].y =     DDBOXES_OFFSET ;//+254; // slider test;
    aswp[0].cy =    (LB_DRIVES_FIRST_DRIVE_ENTRY + 26 ) * (g.font.cy + 4) + 8;
    TRACE2("windrives cy: %d, g.font.cy: %d", aswp[0].cy, g.font.cy);
    }
  else
    {
    aswp[0].x =     g.PbDrivesR.x;
    aswp[0].cx =    g.PbDrivesR.cx;
    aswp[0].y =     DDBOXES_OFFSET ;//+254; // 254 = slider test;
    aswp[0].cy =    (g.iNumVisibleDrives + 1) * (g.font.cy + 4) + 8 ;
    TRACE2("Wi.cy=%d Wi.y=%d",   aswp[0].cy, aswp[0].y);
    }
  if ( aswp[0].cx < 100 )
    {
    aswp[0].cx = 100;
    }
  //TRACE2("win .x=%d win .y=%d", aswp[0].x, aswp[0].y);
  //TRACE2("win.cx=%d win.cy=%d", aswp[0].cx, aswp[0].cy);

  aswp[1].y =     -2;     // offset for drive entries
  aswp[1].cy =    aswp[0].cy - 4;
  aswp[1].x =     32;
  aswp[1].cx =    aswp[0].cx - 54;
  if ( aswp[1].cx < 100 )
    {
    aswp[1].cx = 100;
    }

  if ( ! (g.state & IS_INIT_DRIVES_DONE) )
    {
    TRACE("first call to move WIN_DRIVES buttons");
    TRACE2("windrv.y=%d cy=%d",     aswp[1].y, aswp[1].cy);

    WinShowWindow(DlgItemHwnd(hwnd, LB_DRIVES), FALSE);
    for ( i = CB_LBOX_FIRST; i <= CB_LBOX_LAST; i++ )
      {
      iY = aswp[1].y + (CB_LBOX_LAST - i ) * (g.font.cy + 4);
      //TRACE2("iY = %d, CB_x: %d", iY, i);
      rc = WinSetWindowPos(DlgItemHwnd(hwnd, i), HWND_TOP, aswp[1].x, iY, aswp[1].cx, g.font.cy, SWP_SIZE | SWP_MOVE );
      if ( !rc )
        {
        TRACE1("WinSetWindowPos for CB_LBOX_FIRST + %d not successfull", i - CB_LBOX_FIRST);
        }
      }
    }
  else
    {

    rc = WinSetWindowPos(g.hwndMainDialogBox, 0, aswp[0].x, aswp[0].y, aswp[0].cx, aswp[0].cy, SWP_SIZE | SWP_MOVE);
    // resize window to match visible drives
    if ( g.iNumVisibleDrives > 6 && g.iNumVisibleDrives < 33 )
      {   // if number of visible drives seems to be correct, set window accordingly
      aswp[0].cy = (g.iNumVisibleDrives + 2) * (g.font.cy + 4) + 4;
      aswp[0].y = g.PbDrivesR.y - aswp[0].cy;
      if ( aswp[0].y < DDBOXES_OFFSET )
        {
        aswp[0].cy -= (DDBOXES_OFFSET - aswp[0].y);
        aswp[0].y = DDBOXES_OFFSET;
        }
      TRACE2("iNumVisibleDrives: %d, WinHeight: %d", g.iNumVisibleDrives, aswp[0].cy);
      }
    else g.iNumVisibleDrives = 32;

    for ( i = CB_LBOX_FIRST;  i <= CB_LBOX_LAST; i++ )
      {
      if ( i < CB_LBOX_FIRST + g.iNumVisibleDrives )
        {
        iY = aswp[1].y + (CB_LBOX_FIRST + g.iNumVisibleDrives - i ) * (g.font.cy + 4);
        }
      else iY = 0;
      //TRACE2("iY = %d, CB_x: %d", iY, i);
      rc = WinSetWindowPos(DlgItemHwnd(hwnd, i), HWND_TOP, aswp[1].x, iY, aswp[1].cx, g.font.cy, SWP_SIZE | SWP_MOVE );
      }
    }

  for ( i = CB_LBOX_FIRST;  i <= CB_LBOX_LAST; i++ )
    {
    WinQueryWindowPos(DlgItemHwnd(hwnd, i), (PSWP) &g.PbDrivesR);   // missuse of PbDrivesR
    //TRACE2("i: %d, y: %d", i, g.PbDrivesR.y);
    }

  i = aswp[1].cx / g.font.cx - 7;
  //TRACE2("NumCharsSeparation: %d font.cx: %d", i, g.font.cx);
  if ( i > 49 ) i = 49;
  if ( i < 0 ) i = 0;
  strncpy (DriveSelection[LB_DRIVES_FIRST_DRIVE_ENTRY - 1], sSeparationBar, i);
  DriveSelection[LB_DRIVES_FIRST_DRIVE_ENTRY - 1][i] = '\0';
  WinSetDlgItemText(g.hwndMainDialogBox, CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1, DriveSelection[LB_DRIVES_FIRST_DRIVE_ENTRY - 1]);

  TRACE4("x1: %d, y1: %d, cx1: %d, cy1: %d", aswp[1].x, aswp[1].y, aswp[1].cx, aswp[1].cy);
  rc = WinSetWindowPos(g.hwndWinDrives, 0, aswp[0].x, aswp[0].y, aswp[0].cx, aswp[0].cy, SWP_SIZE | SWP_MOVE);
  WinDefaultSize (g.hwndWinDrives);     // 20111129 AB necessary to do once to draw interior
  rc = WinSetWindowPos(g.hwndWinDrives, 0, aswp[0].x, aswp[0].y, aswp[0].cx, aswp[0].cy, SWP_SIZE | SWP_MOVE);
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

VOID initSizes(HWND hwnd)
  {
  HPS hps;
  POINTL apt[3];
  SWP swp;
  ApiExPM_SIZES size;          // 200808xx AB SIZES is defined elsewhere too
  SIZEL screen;
  int iwidth, idepth;

  if ( NULLHANDLE != (hps = WinGetPS(hwnd)) )
    {
    GpiQueryTextBox(hps, 3, "w p", 3, apt);
    g.font.cx = apt[2].x / 3;
    g.font.cy = apt[0].y - apt[1].y;
    WinReleasePS(hps);
    } /* endif */
  // minimum size according to the current font:
  getMinSize(hwnd, &size);
  // get the screen size
  screen.cx = WinSysVal(SV_CXSCREEN);
  screen.cy = WinSysVal(SV_CYSCREEN);
  TRACE2("screen.cx %d, screen.cy %d", screen.cx, screen.cy);  // = 1920 x 1200 independent of xpager

  // 20100301 HACK - setting size in applWinPosSizeSet do not work so try it here again
  if ( g.startData.iniData )
    {
    TRACE4("g.startData.iniData->swp->x=%d, y=%d, cx=%d, cy=%d", g.startData.iniData->swp.x,
           g.startData.iniData->swp.y, g.startData.iniData->swp.cx, g.startData.iniData->swp.cy);
    WinSetWindowPos(hwnd, NULLHANDLE, g.startData.iniData->swp.x,
                    g.startData.iniData->swp.y, g.startData.iniData->swp.cx, g.startData.iniData->swp.cy, SWP_MOVE | SWP_SIZE);

    // 20090225 AB check if out of visible page (when window is in another xpager window place it in current )
    if ( (g.startData.iniData->swp.x > screen.cx - 200) || (g.startData.iniData->swp.y > screen.cy - 250)
         || (g.startData.iniData->swp.x < -200) || (g.startData.iniData->swp.y < -200) )
      {
      TRACE("moving required");
      WinSetWindowPos(hwnd, NULLHANDLE, 25, 35, 0, 0, SWP_MOVE);
      }

    }
  else TRACE("NO g.startData.iniData !!!!!!!!!!");

  WinQueryWindowPos(hwnd, &swp);
  TRACE4("x=%d, y=%d, cx=%d, cy=%d", swp.x, swp.y, swp.cx, swp.cy);
  if ( (swp.cx < size.cx) || (swp.cy < size.cy) )
    {
    swp.cx = min(screen.cx, max(size.cx, swp.cx));
    swp.cy = min(screen.cy, max(size.cy, swp.cy));
    TRACE("size to small, make bigger");
    WinSetWindowPos(hwnd, NULLHANDLE, swp.x, swp.y, swp.cx, swp.cy,
                    SWP_SIZE);
    if ( !(g.state & IS_INITDONE) ) arrangeControls(hwnd, swp.cx, swp.cy);
    }
  else
    {
    arrangeControls(hwnd, swp.cx, swp.cy);
    } /* endif */
  TRACE4("x=%d, y=%d, cx=%d, cy=%d", swp.x, swp.y, swp.cx, swp.cy);
  if ( !(g.state & (IS_INITDONE | IS_SIZEPOSFROMINI)) )
    {
    TRACE("Size - reset to default");
    swp.x = (screen.cx - swp.cx) / 4;
    swp.y = (screen.cy - swp.cy) / 2;
    WinSetWindowPos(hwnd, NULLHANDLE, swp.x, swp.y, 0, 0, SWP_MOVE);
    } /* endif */

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

VOID getMinSize(HWND hwnd, ApiExPM_PSIZES psize)
  {
  RECTL r;
  // minimum size according to the current font:
  r.xLeft = 0;
  r.xRight = g.font.cx * 74 + 30;
  // 200808xx AB set min size fix in code here (not font dependable anymore)
  r.xRight = 340;
  r.yBottom = 0;
  r.yTop = g.font.cy * 20 + 54;
  // 200808xx AB set min size fix in code here (not font dependable anymore)
  r.yTop = 426;
  //   TRACE2("g.font.cx: %d  g.font.cy: %d", g.font.cx, g.font.cy);
  WinCalcFrameRect(hwnd, &r, FALSE);
  psize->cx = r.xRight - r.xLeft;
  psize->cy = r.yTop - r.yBottom;
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

MRESULT checkMinSize(HWND hwnd, MPARAM mp1, MPARAM mp2)
  {
  ApiExPM_SIZES size;
  MRESULT mr;
  mr = WinDefDlgProc(hwnd, WM_QUERYTRACKINFO, mp1, mp2);
  if ( WS_MINIMIZED & WinQueryWindowULong(hwnd, QWL_STYLE) ) return mr;
  getMinSize(hwnd, &size);
  ((PTRACKINFO)mp2)->ptlMinTrackSize.x = size.cx;
  ((PTRACKINFO)mp2)->ptlMinTrackSize.y = size.cy;
  return mr;
  }


//===========================================================================
// minMaxFrame : when the window is minimized/restored hides/shows the
//               controls positioned on the low left corner, so, in case
//               the 'Minimize window to desktop' option is set, the controls
//               do not cover the window icon.
// Parameters --------------------------------------------------------------
// HWND hwnd   : main dialog window handle
// BOOL flShow : TRUE/FALSE -> show/hide the controls
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID minMaxFrame(HWND hwnd, BOOL flRestore)
  {
  DlgItemShow(hwnd, PB_SEARCH, flRestore);
  DlgItemShow(hwnd, LBOX_TEXTFOUND, flRestore);
  }

