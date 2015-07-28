/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

#define INCL_LOADEXCEPTQ            // only once per application

#define DRIVES_WIN_VISIBLE_IN_MS    800     // WIN_DRIVES window visible after mouse leaves it
#define DRIVES_TIMER_TIME_IN_MS     250UL   // WM_TIMER called every ...
#define DRIVES_TIMER_RELOAD         DRIVES_WIN_VISIBLE_IN_MS / DRIVES_TIMER_TIME_IN_MS
// pmseek.c
#include <stdarg.h>
#include "pmseek.h"

#include "exceptq.h"

// prototypes ---------------------------------------------------------------

MRESULT EXPENTRY pmseekDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WinDrivesDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT controlPointer(HWND hwnd, ULONG idCtrl, HPOINTER hptr);
VOID onCtrlNotify(HWND hwnd, ULONG id, ULONG ulNotify, HWND hCtrl);
VOID onCommand(HWND hwnd, ULONG id, ULONG rc);
VOID resetBoxFonts(HWND hwnd, HWND hCtrl, BOOL bRemove);
VOID resetDlgFonts(HWND hwnd, HWND hCtrl);
void CheckSumDrives(HWND);

// globals ------------------------------------------------------------------

GLOBAL g;
structDriveInfo DriveInfo[27];
char DriveSelection[LB_DRIVES_FIRST_DRIVE_ENTRY][MAX_LENGHT_SELECTION_ENTRIES];     // includes 1 entry for separation bar

char caDriveTypeShort[] =
{                      /** Unknown drive type (?). */'-',       // 0
  /** Standard floppy drive (typically A: or B:). */  'F',        // 1
  /** Standard CD-ROM. */                             'C',        // 2
  /** Standard hard-disk. */                          'H',        // 3
  /** Network (remote) drive. */                      'N',        // 4
  /** Some removable (Zip-, EZ-, Jaz-Drive, etc.). */ 'Z',        // 5
  /** RAM-Drive (virtual disk in memory). */          'R',        // 6
  /** RSJ attached drive */                           '*'         // 7
};

// variables -----------------------------------------------------------------
static HAB hab;
static int iCount;
static ULONG ulTimerId;

INT main(int argc, char** argv)
  {
  EXCEPTIONREGISTRATIONRECORD exRegRec;
  HMQ hmq;
  QMSG qmsg;
  HWND hHlp;
  char cBuf[256];
  char cBuf2[512];
  int iExit = FALSE;
  APIRET rc = NO_ERROR;
  APIRET rc2;
  PSZ pszP = "";
  HMODULE hMMPMCRTS = NULLHANDLE;         /* Module handle                     */
  HMODULE hSW = NULLHANDLE;
  UCHAR    LoadError[256] = "";           /* Area for Load failure information */
  ULONG  aulCpList[8]  = {0},                /* Code page list        */
  ulBufSize     = 8 * sizeof(ULONG),  /* Size of output list   */
  ulListSize    = 0,                  /* Size of list returned */
  indx          = 0;                  /* Loop index            */


  TRACE("\n\n");
  TRACE("--------------------------------------------------------------");
  TRACE(" DataSeeker build by Andreas Buchinger  "__DATE__"  "__TIME__);
  TRACE("--------------------------------------------------------------");

  LoadExceptq(&exRegRec, "I", "DataSeeker v1.14.18");

  WrapperInit();

  // load NLS support if available
  NlsInit();
  FocInit();

  if ( !initApplication(argc, argv) ) return 2;

  hmq = WinStdInit(&hab);

  rc = DosQueryCp(ulBufSize,     /* Length of output code page list  */
                 aulCpList,      /* List of code pages               */
                 &ulListSize);   /* Length of list returned          */

   if (rc != NO_ERROR)
    {
    TRACE1("DosQueryCp error: return code = %u",rc);
    }
  else
    {
    for (indx=0; indx < ulListSize/sizeof(ULONG); indx++) TRACE2 ("aulCpList[%u] = %u", indx, aulCpList[indx]);

    WinSetCp(hmq, aulCpList[0]);
    }


  g.hwndMainWin = WinLoadDlg(HWND_DESKTOP,        // parent
                             NULLHANDLE,            // owner
                             pmseekDlgProc,         // dialog window proc
                             NULLHANDLE,            // module handle
                             WIN_MAIN,              // dialog template ID
                             NULL);                 // application data pointer

  TRACE1("g.hwndMainWin = 0x%0X", g.hwndMainWin);

  // 20100414 test for MultiMedia dlls
  // ToDo: to make this work there have to be made wrapper functions to all references to sw.dll.
  // currently this are -
  //  WinDefaultSize       (sw.11)
  //  WinDefSecondaryWindowProc (sw.8)
  //  WinLoadSecondaryWindow (sw.2)
  //  WinQuerySecondaryHWND (sw.52)

  rc = DosLoadModule(LoadError,                   /* Failure information buffer */
                     sizeof(LoadError),        /* Size of buffer             */
                     "hMMPMCRTS",              /* Module to load             */
                     &hMMPMCRTS);              /* Module handle returned     */
  rc = DosLoadModule(LoadError,                   /* Failure information buffer */
                     sizeof(LoadError),        /* Size of buffer             */
                     "hSW",                    /* Module to load             */
                     &hSW);                    /* Module handle returned     */

  if ( ( (rc = DosQueryModuleHandle("MMPMCRTS", &hMMPMCRTS)) != NO_ERROR ) || ( (rc2 = DosQueryModuleHandle("SW", &hSW) ) != NO_ERROR ) )
    {
    TRACE2("MMPMCRTS handle rc = 0x%X, SW handle rc2 = 0x%X", rc, rc2);
    TRACE("no MultiMedia dlls !!!!!!!");
    g.iHaveMMdlls = FALSE;
    NlsGet("missing mmpmcrts.dll/sw.dll", cBuf);
    WinSetDlgItemText(g.hwndMainWin, PB_DRIVES, cBuf);
    }
  else
    {
    TRACE2("MMPMCRTS handle rc = 0x%X, SW handle rc2 = 0x%X", rc, rc2);
    TRACE2("MMPMCRTS handle    = 0x%X, SW handle     = 0x%X", hMMPMCRTS, hSW);
    g.iHaveMMdlls = TRUE;
    g.hwndWinDrives = WinLoadSecondaryWindow( g.hwndMainWin,     // parent
                                              g.hwndMainWin,                         // owner
                                              (PFNWP) WinDrivesDlgProc,            // dialog window proc
                                              (HMODULE)NULL,                       // module handle
                                              WIN_DRIVES,                          // dialog template ID
                                              NULL);                               // application data pointer
    }


#define SZ_MAINTITLE         "Data Seeker"
  if ( g.hwndMainWin )
    {
    NlsGet("HelpFile", cBuf);
    hHlp = applInitHelp(g.hwndMainWin, cBuf, NULLHANDLE, HLP_MAIN, SZ_MAINTITLE);
    TRACE1("hHlp=0x%X", hHlp);
    WinExtMsgLoop(g.hwndMainWin, hab, &qmsg);
    applTerminateHelp(g.hwndMainWin, hHlp);
    } /* endif */
  WinStdEnd(g.hwndMainWin, hmq, hab);
  endApplication();

  // free NLS resources
  NlsClose();

  if ( iExit )
    { // wait before close when exception
    DosSleep(8000);
    exit ( 0xFF);
    }

  UninstallExceptq(&exRegRec);
  return 0;
  } /* end main */


//===========================================================================
// Main dialog window procedure.
// Parameters --------------------------------------------------------------
// standard window procedure parameters
// Return value ------------------------------------------------------------
// MRESULT
//===========================================================================

MRESULT EXPENTRY pmseekDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  ULONG ulL;

  //if (mp2) TRACE3("msg: 0x%lX, mp1=0x%lX, mp2=0x%lX", msg, mp1, mp2);

  switch ( msg )
    {
    case WM_CONTROLPOINTER:     // mouse pointer handling - Hourglass pointer when searching
      return controlPointer(hwnd, (ULONG)mp1, (HPOINTER)mp2);
    case WM_INITDLG:
      // prevent WIN_DRIVES dialog arrange before it is initialized
      g.state |= IS_INIT_DRIVES_DONE;
      initMainDlg(hwnd);
      //TRACE1("g.hwndMainWin = 0x%0X", g.hwndMainWin);
      WinPostMsg(hwnd, WM_INITDLG2, (MPARAM) 0, (MPARAM) 0);
      return MRTRUE;
      break;
    case WM_INITDLG2:
      TRACE("WM_INITDLG2");
      initSizes(hwnd);
      // free ini data (used in initSizes)
      if ( g.startData.iniData )
        {
        free(g.startData.iniData);
        g.startData.iniData = NULL;
        }
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DD_FILESRCH));
      break;
    case WM_CONTROL:
      //TRACE2("WM_CONTROL: 0x%0lX 0x%0lX", mp1, mp2);
      if ( !(g.state & IS_SKIPCTRLNOTIF) )
        {
        onCtrlNotify(hwnd,  SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), (HWND)mp2); // 20090115 AB test with address
        //TRACE2("SHORT2FROMMP(mp1)=0x%X, length=%d", SHORT2FROMMP(mp1), WinQueryWindowTextLength( WinWindowFromID(hwnd, DD_FILESRCH) ) );
        if ( (SHORT2FROMMP(mp1) == EN_CHANGE) )
          {
          //TRACE("EN_CHANGE");
          //TRACE2("SHORT2FROMMP(mp1)=0x%X, length=%d", SHORT2FROMMP(mp1), WinQueryWindowTextLength( WinWindowFromID(hwnd, DD_FILESRCH) ) );
          }
        if ( (SHORT2FROMMP(mp1) == EN_OVERFLOW) )
          {
          TRACE("EN_OVERFLOW....");
          }
        if ( (SHORT2FROMMP(mp1) == LN_SELECT) )
          {
          //TRACE("LN_SELECT *******");
          }

        if ( (SHORT2FROMMP(mp1) == CBN_ENTER) /*|| (SHORT2FROMMP(mp1) == LN_SELECT)*/ )
          {
          int i, j;
          char sBuf[LIST_BOX_WIDTH];
          TRACE3("CBN_ENTER mp1=0x%X, mp2=0x%X, hwnd=0x%X", mp1, mp2, hwnd);
          i = WinQueryLboxSelectedItem( (HWND) mp2 );
          if ( i != LIT_NONE )
            {
            j = WinQueryLboxItemTextLength( (HWND) mp2, i );
            WinQueryLboxItemText( (HWND) mp2, i, sBuf, sizeof(sBuf) );
            //TRACE3("index=%d, length=%d, text=%s", i, j, sBuf );
            WinSetWindowText( (HWND) mp2, sBuf );
            }
          }
        else
          {
          //TRACE2("WM_CONTROL: 0x%0lX 0x%0lX", mp1, mp2);
          }
        }
      break;
    case WM_COMMAND:
      //TRACE2("WM_COMMAND: 0x%0lX 0x%0lX", mp1, mp2);
      onCommand(hwnd, (ULONG)mp1, (ULONG)mp2);
      break;
      // recalculate the size of the controls
    case WM_PRESPARAMCHANGED:
      TRACE2("WM_PRESPARAMCHANGED: 0x%0lX 0x%0lX", mp1, mp2);
      if ( ((LONG)mp1 == PP_FONTNAMESIZE) && (g.state & IS_INITDONE) )
        initSizes(hwnd);
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
    case WM_QUERYTRACKINFO:
      //TRACE2("WM_QUERYTRACKINFO: 0x%0lX 0x%0lX", mp1, mp2);
      return checkMinSize(hwnd, mp1, mp2);
    case WM_MINMAXFRAME:
      TRACE2("WM_MINMAXFRAME: 0x%0lX 0x%0lX", mp1, mp2);
      minMaxFrame(hwnd, !(((PSWP)mp1)->fl & SWP_MINIMIZE));
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
      break;
    case WM_SIZE:
      TRACE("WM_SIZE");
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
      break;
    case WM_CLOSE:
      TRACE("WM_CLOSE");
      WinSendMsg(g.hwndDriveInfo, WM_QUIT, 0, 0);
      if ( g.state & IS_SEARCHING )
        {
        g.state |= IS_WAITSEARCHEND;
        cmdStop(hwnd);
        }
      else if ( !(g.state & IS_EXITING) )
        {
        WinEnableWindow(hwnd, FALSE);
        g.state |= IS_EXITING;
        WinPostMsg(g.objWnd.hwnd, OWM_QUIT, (MPARAM)hwnd, MPVOID);
        } /* endif */
      break;
    case OWM_QUIT:
      TRACE("QWM_QUIT");
      if ( mp1 )
        {
        WinShowWindow(hwnd, FALSE);
        freeResources(hwnd);
        } /* endif */
      DosSleep(100);
      if ( WinThreadTerminated(&g.objWnd.tid) )
        {
        WinPostMsg(hwnd, WM_QUIT, 0, 0);
        }
      else
        {
        WinPostMsg(hwnd, OWM_QUIT, MPVOID, MPVOID);
        } /* endif */
      break;
    case WM_WINDOWPOSCHANGED:
      //TRACE2("WM_WINDOWPOSCHANGED: 0x%0lX 0x%0lX", mp1, mp2);
      if ( g.font.cx && (g.state & IS_INITDONE) && (((PSWP)mp1)->fl & SWP_SIZE) )
        {
        arrangeControls(hwnd, ((PSWP)mp1)->cx, ((PSWP)mp1)->cy);
        }
      return WinDefDlgProc(hwnd, msg, mp1, mp2); // does not help

      // 20100217 AB work around for menu destroy when resizing
      //WinPostMsg(hwnd, WM_UPDATEFRAME, (MPARAM)FCF_MENU, MPVOID); // Action bars are not drawn corretly reliable
      WinPostMsg(WinWindowFromID (hwnd, FID_MENU), WM_PRESPARAMCHANGED, (MPARAM)0, MPVOID);
      break;
    case WM_FOCUSCHANGE:
      TRACE1("WM_FOCUSCHANGE, mp2=%d", SHORT1FROMMP(mp2));
      if ( SHORT1FROMMP(mp2) )    // got focus
        {
        // set focus to Files to search DD box
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DD_FILESRCH));
        }
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
      break;
      // --------------------------------------------------------------------
      // 20100706 Test fÅr LCMD
    case WM_SETFOCUS:
      TRACE1("WM_SETFOCUS %d", SHORT1FROMMP(mp2));
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
      break;
    case WM_ACTIVATE:
      TRACE1("WM_ACTIVATE %d", SHORT1FROMMP(mp1));
      if ( SHORT1FROMMP(mp1) == FALSE )
        { // 20111213 AB deactivate WIN_DRIVES and drive update
        WinPostMsg(g.hwndWinDrives, WM_WIN_DRIVES_INVISIBLE, 0, 0);
        }
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
      break;
      // --------------------------------------------------------------------
    case SWM_PPCHANGED:
      TRACE("call resetBoxFonts");
      resetBoxFonts(hwnd, (HWND)mp1, (BOOL)mp2);
      break;
    case SWM_PPCHANGED2:
      resetDlgFonts(hwnd, (HWND)mp1);
      break;
      /*case WM_MOUSEENTER:
          //TRACE2("WM_MOUSEENTER: 0x%0lX 0x%0lX", mp1, mp2);
          break;
      case WM_MOUSELEAVE:
          //TRACE3("WM_MOUSELEAVE: 0x%0lX 0x%0lX hwnd: 0x%0lX", mp1, mp2 , g.hwndWinDrives);
          break;
      */
    case WM_BUTTON2DOWN:
      {
        BOOL    fSuccess;       /* success indicator                    */
        POINTL ptlMouse;

        // check if LBox Pop-Up was fired up and if not start standard Pop-Up
        if ( !WinIsWindowVisible( g.hwndMenuPuLbox ) )
        //if    (0)
          {
          /* get current mouse pointer position */
          WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
          /* map from desktop to client window */
          fSuccess = WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptlMouse, 1);

          fSuccess = WinPopupMenu(hwnd, hwnd, g.hwndMenuPuMain,
                                  //fSuccess = WinPopupMenu(hwnd, hwnd, g.hwndMenuPuMain,
                                  ptlMouse.x - 5, ptlMouse.y - g.PopUpMainCy + 10,
                                  PU_IDM_SETRECUR,
                                  //PU_POSITIONONITEM   |
                                  PU_HCONSTRAIN       |
                                  PU_VCONSTRAIN       |
                                  PU_MOUSEBUTTON1     |
                                  PU_KEYBOARD         |
                                  0);
          TRACE1("fSuccess=%d", fSuccess);
          {updateOptionsMenu(hwnd, g.setting);}   // {] to prevent linker crash !!!
          }
      }
      break;
    case WM_HELP:
      TRACE2("WM_HELP: mp1=0x%X, mp2=0x%X", mp1, mp2);
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_HELPIDX), MPVOID);
      break;
    default:
/*      if (mp2) TRACE4("not handled: hwnd: 0x%0lX msg:  0x%0lX, 0x%0lX 0x%0lX", hwnd, msg, mp1, mp2);
      TRACE1("unhandled msg:0x%0X", msg);
			if (msg == 0x56)
				{
				SWP     p;
				
				TRACE1 ("Options fl=0x%0lX", p.fl);
				TRACE4 ("cy=%ld, cx=%ld, y=%ld, x=%ld", p.cy, p.cx, p.y, p.x);
				}
*/				
//      return WinDefDlgProc(hwnd, msg, mp1, mp2);		 <--------- there was a trap with release builds und Min/Max buttons
//																																after making exceptq symbols work and removing /O from CFLAGs this is gone now :-(
      return WinDefDlgProc(hwnd, msg, mp1, mp2);		 
			break;
    } /* endswitch */
  return MRFALSE;
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
_Inline
            MRESULT controlPointer(HWND hwnd, ULONG idCtrl, HPOINTER hptr)
  {
  if ( (g.state & IS_EXITING) ||
       ((g.state & IS_SEARCHING) &&
        (idCtrl > WIN_MAIN) &&
        (idCtrl < TXT_TEXTFOUND)) )
    return(MRESULT)WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);
  return(MRESULT)hptr;
  }


//===========================================================================
// Process notification messages.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// ULONG id       : control ID
// ULONG ulNotify : notify code
// HWND hCtrl     : control handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID onCtrlNotify(HWND hwnd, ULONG id, ULONG ulNotify, HWND hCtrl)
  {
  switch ( id )
    {
    case DD_FILESRCH:         //
      //TRACE("onCtrlNotify - in DD_FILESRCH");
      if ( ulNotify == CBN_EFCHANGE )
        WinEnableControl(hwnd, PB_SEARCH,
                         DlgItemTextLength(hwnd, DD_FILESRCH));
      break;
    case DD_SRCHRES:
      //TRACE("onCtrlNotify - in DD_SRCHRES");
      if ( ulNotify == CBN_LBSELECT )
        {
        historyEntrySelected(hwnd, hCtrl);
        }
      else
        {
        } /* endif */
      break;
    case LBOX_FILEFOUND:
      //TRACE1("onCtrlNotify - in LBOX_FILEFOUND ulNotify=%u", ulNotify);
      if ( ulNotify == LN_SELECT )
        {
        foundFileSelected(hwnd, hCtrl/*, FALSE*/);  // do not select first found
        g.bFileScrolled = TRUE;                     // 20081119 AB indicate there was a 'select' in file list during search
        }
      else if ( ulNotify == LN_ENTER )
        {
        // open the folder containing the file
        if ( pmKbdIsKeyPressed(VK_CTRL) )
          {
          cmdSelectedPath(hwnd, TRUE);
          // open the file properties notebook
          }
        else if ( pmKbdIsKeyPressed(VK_ALT) )
          {
          cmdSelectedOpen(hwnd, OPEN_SETTINGS);
          // edit the file via the topmost EPM window via DDE
          }
        else if ( pmKbdIsKeyPressed(VK_SHIFT) )
          {
          cmdSelectedEdit(hwnd, TRUE);
          // edit the file via the selected editor
          }
        else
          {
          // 20100301 AB changed from Edit to Open
          cmdSelectedOpen(hwnd, FALSE);
          } /* endif */
        } /* endif */
      if ( ulNotify == LN_SCROLL )  // 20081119 AB indicate there was scrolled in file list during search
        {
        //TRACE("set bFileScrolled");
        g.bFileScrolled = TRUE;
        }
      break;
    case LBOX_TEXTFOUND:
      //TRACE("onCtrlNotify - in LBOX_TEXTFOUND");
      if ( ulNotify == LN_ENTER )
        {
        // open the folder containing the file
        if ( pmKbdIsKeyPressed(VK_CTRL) )
          {
          cmdSelectedPath(hwnd, TRUE);
          // open the file properties notebook
          }
        else if ( pmKbdIsKeyPressed(VK_ALT) )
          {
          cmdSelectedOpen(hwnd, OPEN_SETTINGS);
          // edit the file via the topmost EPM window via DDE
          }
        else if ( pmKbdIsKeyPressed(VK_SHIFT) )
          {
          cmdSelectedEdit(hwnd, TRUE);
          // edit the file via the selected editor
          }
        else
          {
          cmdSelectedEdit(hwnd, FALSE);
          } /* endif */
        } /* endif */
      if ( ulNotify == LN_SCROLL )  // 20081119 AB indicate there was scrolled in file list during search
        {
        TRACE("set bTextScrolled");
        g.bTextScrolled = TRUE;
        }
      break;
    default:
      //TRACE1("onCtrlNotify - don't know what to do, id = %d", id);
      break;
    } /* endswitch */
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// ULONG id  : button id
// ULONG rc  : meaingless only for commands posted by the object window
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID onCommand(HWND hwnd, ULONG id, ULONG rc)
  {
  ULONG ulTemp;
  RECTL rclWork;
  SWP swp;

  switch ( id )
    {
    // buttons ------------------------------------------------------------
    case PB_SEARCH:          // Search/Stop button clicked
    case IDM_SEARCH:
      cmdSearch(hwnd);
      break;
    case PB_STOP:
      cmdStop(hwnd);
      //CRASH;        // trigger exception (to test exception handler and log file writting)
      break;
    case PB_DRIVES:
      if ( g.iHaveMMdlls ) WinSendMsg(g.hwndWinDrives, WM_TOGGLE_VISIBILITY, 0, 0);
      break;
    case PB_TEST:
      TRACE("PB_TEST");
      EXCEPTQ_DEBUG_EXCEPTION;     // ToDo: 20111229 AB does nothing but next line crashes ;-)
      {PSZ p = NULL; *p = 'a';}
      break;
    case PB_FOC:        // 20121213 AB FOC handling
      TRACE("PB_FOC");
      if ( FocIsAvail() )
        {
        FocGetSearchList(hwnd);
        }
      else
        {
        WinDlgBox(HWND_DESKTOP, hwnd, FocFailDlgProc, 0, DLG_FOC_FAIL, NULL);
        }

      break;
    case PB_MOVEDOWN:        // down arrow
      cmdMoveResultCtls(hwnd, -1);
      break;
    case PB_MOVEUP:          // up arrow
      cmdMoveResultCtls(hwnd, +1);
      break;
      // menu items ---------------------------------------------------------
    case IDM_OPEN:             // open
      break;
    case IDM_SAVE:             // save
      break;
    case IDM_EXIT:             // save
      WinPostMsg(g.objWnd.hwnd, OWM_QUIT, (MPARAM)hwnd, MPVOID);
      break;
    case IDM_EDITSEL:          // selected - edit
    case PU_IDM_EDITSEL:
      cmdSelectedEdit(hwnd, pmKbdIsKeyPressed(VK_SHIFT));
      break;
    case IDM_OPENSEL:          // selected - open
    case PU_IDM_OPENSEL:
      cmdSelectedOpen(hwnd, OPEN_DEFAULT);
      break;
    case IDM_PROPERTIES:
    case PU_IDM_PROPERTIES:
      cmdSelectedOpen(hwnd, OPEN_SETTINGS);
      break;
    case IDM_COPY:              // 20080929 test accelerator table
      TRACE("IDM_COPY");
      break;
    case IDM_COPYFNAMES:       // selected - copy to clipboard
    case PU_IDM_COPYFNAMES:
      TRACE("IDM_COPYFNAMES");
      cmdSelectedCopyFilename(hwnd);
      break;
    case IDM_COPYALLFNAMES:
    case PU_IDM_COPYALLFNAMES:
      TRACE("IDM_COPYALLFNAMES");
      cmdSelectedCopyAllFilenames(hwnd);
      break;
    case IDM_COPYLINE:
    case PU_IDM_COPYLINE:
      TRACE("IDM_COPYLINE");
      cmdSelectedCopyLine(hwnd);
      break;
    case IDM_OPENDIR:
    case PU_IDM_OPENDIR:
      cmdSelectedPath(hwnd, TRUE);
      break;
    case IDM_OPENCLI:
    case PU_IDM_OPENCLI:
      cmdSelectedPath(hwnd, FALSE);
      break;
      /*      case IDM_COMMAND:          // selected - execute command
               break;
      */
      // View submenu -------------------------------------------------------
    case IDM_VFDATE:
    case PU_IDM_VFDATE:
      cmdSetOption(hwnd, IDM_VFDATE, OPT_SHOWFDATE);
      break;
    case IDM_VFTIME:
    case PU_IDM_VFTIME:
      cmdSetOption(hwnd, IDM_VFTIME, OPT_SHOWFTIME);
      break;
    case IDM_VFSIZE:
    case PU_IDM_VFSIZE:
      cmdSetOption(hwnd, IDM_VFSIZE, OPT_SHOWFSIZE);
      break;
    case IDM_VLINENO:
    case PU_IDM_VLINENO:
      cmdSetOption(hwnd, IDM_VLINENO, OPT_SHOWLINENO);
      break;
    case IDM_VNOSPACES:
    case PU_IDM_VNOSPACES:
      cmdSetOption(hwnd, IDM_VNOSPACES, OPT_STRIPSPACES);
      break;

#define SetPopUpMenu(item,state)    WinSendMsg(g.hwndMenuPuMain, MM_SETITEMATTR, MPFROM2SHORT(item, TRUE), MPFROM2SHORT(MIA_CHECKED, state) )
#define IsPopUpMenuSet(item)        WinSendMsg(g.hwndMenuPuMain, MM_QUERYITEMATTR, MPFROM2SHORT(item, TRUE),MPFROMSHORT(MIA_CHECKED))
    case IDM_VSORTNONE:
    case PU_IDM_VSORTNONE:
      dMenuItemCheck(hwnd, IDM_VSORTNONE, TRUE);
      dMenuItemCheck(hwnd, IDM_VSORTASCEND, FALSE);
      dMenuItemCheck(hwnd, IDM_VSORTDESCEND, FALSE);

      SetPopUpMenu(PU_IDM_VSORTNONE, TRUE);
      SetPopUpMenu(PU_IDM_VSORTASCEND, FALSE);
      SetPopUpMenu(PU_IDM_VSORTDESCEND, FALSE);
      g.sort = LIT_END;
      refreshResult(hwnd, TRUE);
      //cmdSortOption(hwnd, IDM_VSORTNONE, LIT_END);
      break;
    case IDM_VSORTASCEND:
    case PU_IDM_VSORTASCEND:
      TRACE("IDM_VSORTASCEND or PU_IDM_VSORTASCEND");
      dMenuItemCheck(hwnd, IDM_VSORTNONE, FALSE);
      dMenuItemCheck(hwnd, IDM_VSORTASCEND, TRUE);
      dMenuItemCheck(hwnd, IDM_VSORTDESCEND, FALSE);

      SetPopUpMenu(PU_IDM_VSORTNONE, FALSE);
      SetPopUpMenu(PU_IDM_VSORTASCEND, TRUE);
      SetPopUpMenu(PU_IDM_VSORTDESCEND, FALSE);
      g.sort = LIT_SORTASCENDING;
      refreshResult(hwnd, TRUE);
      //cmdSortOption(hwnd, IDM_VSORTASCEND, LIT_SORTASCENDING);
      break;
    case IDM_VSORTDESCEND:
    case PU_IDM_VSORTDESCEND:
      dMenuItemCheck(hwnd, IDM_VSORTNONE, FALSE);
      dMenuItemCheck(hwnd, IDM_VSORTASCEND, FALSE);
      dMenuItemCheck(hwnd, IDM_VSORTDESCEND, TRUE);

      SetPopUpMenu(PU_IDM_VSORTNONE, FALSE);
      SetPopUpMenu(PU_IDM_VSORTASCEND, FALSE);
      SetPopUpMenu(PU_IDM_VSORTDESCEND, TRUE);
      g.sort = LIT_SORTDESCENDING;
      refreshResult(hwnd, TRUE);
      //cmdSortOption(hwnd, IDM_VSORTDESCEND, LIT_SORTDESCENDING);
      break;

      // Pop-up menu
    case PU_IDM_SETRECUR:
      if ( IsPopUpMenuSet(PU_IDM_SETRECUR) )
        {
        TRACE("clear RECUR");
        g.setting &= ~OPT_RECUR;
        TRACE1("g.setting=0x%X", g.setting);
        //SetPopUpMenu(PU_IDM_SETRECUR, FALSE);
        }
      else
        {
        TRACE("set RECUR");
        g.setting |= OPT_RECUR;
        TRACE1("g.setting=0x%X", g.setting);
        //SetPopUpMenu(PU_IDM_SETRECUR, TRUE);
        }
      updateOptionsMenu(hwnd, g.setting);
      break;

    case PU_IDM_SETTEXTONLY:
      if ( IsPopUpMenuSet(PU_IDM_SETTEXTONLY) ) g.setting &= ~OPT_TEXTONLY;
      else g.setting |= OPT_TEXTONLY;
      updateOptionsMenu(hwnd, g.setting);
      break;

    case PU_IDM_SETCASEINS:
      if ( IsPopUpMenuSet(PU_IDM_SETCASEINS) ) g.setting &= ~OPT_CASEINS;
      else g.setting |= OPT_CASEINS;
      updateOptionsMenu(hwnd, g.setting);
      break;

    case PU_IDM_SETBOOLOP:
      if ( IsPopUpMenuSet(PU_IDM_SETBOOLOP) ) g.setting &= ~OPT_BOOLOP;
      else g.setting |= OPT_BOOLOP;
      updateOptionsMenu(hwnd, g.setting);
      break;

    case PU_IDM_SETDIRSEARCH:
      if ( IsPopUpMenuSet(PU_IDM_SETDIRSEARCH) ) g.setting &= ~OPT_FIND_DIR;
      else g.setting |= OPT_FIND_DIR;
      updateOptionsMenu(hwnd, g.setting);
      break;

    case IDM_SETTINGS:
      WinDlgBox(HWND_DESKTOP, hwnd, setsDlgProc, 0, DLG_SETTINGS, NULL);
      break;

      // Help submenu -------------------------------------------------------
    case IDM_HELPIDX:
      TRACE("try to start help");
      if ( !WinSendMsg(WinQueryHelpInstance(hwnd), HM_HELP_INDEX, MPVOID, MPVOID) )
        {
        TRACE("no help manager associated");
        }
      break;
    case IDM_PRODINFO:
      // 200808xx AB no WARP3 differece any more till someone request it...
      ulTemp = DLG_ABOUT + (SysInfo(QSV_VERSION_MINOR) < 40);
      WinDlgBox(HWND_DESKTOP, hwnd, aboutDlgProc, 0, DLG_ABOUT, NULL);
      break;
      // commands sent by the object window ---------------------------------
    case CMD_SEARCHEND:       // search terminated
      cmdSearchEnd(hwnd, rc);
      break;
      // commands sent by the accelerators
    case CMD_DELENTRY:
      cmdDeleteEntry(hwnd);
      break;
    case CMD_DELALLENTRY:
      cmdDeleteAllEntries(hwnd);
      break;
    case CMD_EDITSEL2:
      cmdSelectedEdit(hwnd, TRUE);
      break;

    case DID_CANCEL:    // close list box
      TRACE("close lbox");
      WinPostMsg (WinWindowFromID (hwnd, DD_FILESRCH), CBM_SHOWLIST, MPFROMSHORT (FALSE), MPFROMSHORT (0) );
      WinPostMsg (WinWindowFromID (hwnd, DD_SRCHRES),  CBM_SHOWLIST, MPFROMSHORT (FALSE), MPFROMSHORT (0) );
      WinPostMsg (WinWindowFromID (hwnd, DD_TEXTSRCH), CBM_SHOWLIST, MPFROMSHORT (FALSE), MPFROMSHORT (0) );
      if ( g.iHaveMMdlls ) WinPostMsg(g.hwndWinDrives, WM_WIN_DRIVES_INVISIBLE, 0, 0);
      break;

    default:
      TRACE1("unhandled command 0x%X", id);
      break;
      // menu commands ------------------------------------------------------
    } /* endswitch */
  }


//===========================================================================
// When the user drag a font on a listbox or drop down box the font of all
// the other listboxes/dropdown boxes is updated accordingly.
// Parameters --------------------------------------------------------------
// HWND hwnd    : dialog window handle
// HWND hCtrl   : handle of the window whose font has been changed.
// BOOL bRemove : TRUE when the parameter must be removed from hCtrl.
//                This is necessary when hCtrl is the handle of the entry
//                filed of a combobox.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID resetBoxFonts(HWND hwnd, HWND hCtrl, BOOL bRemove)
  {
  CHAR buf[64];
  ULONG cb;
  cb = WinQueryPresParam(hCtrl, PP_FONTNAMESIZE, 0, NULL,
                         sizeof(buf), buf, QPF_NOINHERIT);
  g.state |= IS_RESETBOXFONTS;
  if ( bRemove ) WinRemovePresParam(hCtrl, PP_FONTNAMESIZE);
  WinSetPresParam(DlgItemHwnd(hwnd, DD_FILESRCH), PP_FONTNAMESIZE, cb, buf);
  WinSetPresParam(DlgItemHwnd(hwnd, DD_TEXTSRCH), PP_FONTNAMESIZE, cb, buf);
  WinSetPresParam(DlgItemHwnd(hwnd, DD_SRCHRES), PP_FONTNAMESIZE, cb, buf);
  WinSetPresParam(DlgItemHwnd(hwnd, LBOX_FILEFOUND), PP_FONTNAMESIZE, cb, buf);
  WinSetPresParam(DlgItemHwnd(hwnd, LBOX_TEXTFOUND), PP_FONTNAMESIZE, cb, buf);
  g.state &= ~IS_RESETBOXFONTS;
  }


//===========================================================================
// When the font of a subclassed caption (WC_STATIC-SS_TEXT) control changes,
// the message SWM_PPCHANGED2 is sent to the dialog window.
// The dialog window gets the font used by the control, reset the control
// presentation parameter (so it can still inherit the dialog font) and
// use the font of the control.
// Parameters --------------------------------------------------------------
// HWND hwnd  : dialog window handle
// HWND hCtrl : handle of the window whose font has been changed.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID resetDlgFonts(HWND hwnd, HWND hCtrl)
  {
  CHAR buf[64];
  ULONG cb;
  cb = WinQueryPresParam(hCtrl, PP_FONTNAMESIZE, 0, NULL,
                         sizeof(buf), buf, QPF_NOINHERIT);
  WinRemovePresParam(hCtrl, PP_FONTNAMESIZE);
  g.state |= IS_RESETDLGFONT;
  WinSetPresParam(hwnd, PP_FONTNAMESIZE, cb, buf);
  g.state &= ~IS_RESETDLGFONT;
  }

//===========================================================================
// Main dialog window procedure.
// Parameters --------------------------------------------------------------
// standard window procedure parameters
// Return value ------------------------------------------------------------
// MRESULT
//===========================================================================

MRESULT EXPENTRY WinDrivesDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  MRESULT ret = 0;
  APIRET rc;
  BOOL  fSuccess;         /* success indicator                    */
  POINTL ptlMouse;        /* mouse pointer position               */
  RECTL rclWork;          /* client area                          */
  char buf[MAX_LENGHT_SEPARATION_BAR + 1];
  int i, j, k;
  HWND hwndLb;
  SHORT sIndex;
  PSZ pszText;
  SWP   swp;
  SHORT iX, iY;
  CHAR   szMsg[100];
  CHAR     szDefaultSize[CCHMAXPATH];   /* buffer for default size menu text */
  RECTL rcl;
  char cBuf[20];

  OWNERITEM *LbItem;      // structure for WM_DRAWITEM    //    typedef struct _OWNERITEM {
  //    HWND hwnd;
  //    HPS hps;
  //    ULONG fsState;
  //    ULONG fsAttribute;
  //    ULONG fsStateOld;
  //    ULONG fsAttributeOld;
  //    RECTL rclItem;
  //    LONG idItem;
  //    ULONG hItem;
  //     } OWNERITEM, *POWNERITEM
  switch ( msg )
    {
    case WM_INITDLG:
      WinShowWindow( hwnd, FALSE );     /* show window. */
      g.hwndMainDialogBox = WinQuerySecondaryHWND(hwnd, QS_DIALOG);
      TRACE1("g.hwndMainDialogBox: 0x%0X", g.hwndMainDialogBox);
      //hwndCaption = ccInitialize ( (HWND) g.hwndMainDialogBox );
      WinSendMsg(hwnd, WM_WIN_DRIVES_INIT_2, 0, 0);
      return MRTRUE;
      break;

    case WM_WIN_DRIVES_INIT_2:
      TRACE("WM_WIN_DRIVES_INIT_2");
      initDrivesDlg(hwnd);
      TRACE("WM_WIN_DRIVES_INIT_2 - after initDrivesDlg");
      g.state &= ~IS_INIT_DRIVES_DONE;
      arrangeWinDrives(hwnd);
      TRACE("WM_WIN_DRIVES_INIT_2 - after arrangeWinDrives");
      g.state |= IS_INIT_DRIVES_DONE;
      g.WinDrivesVisible = FALSE;
      g.iDrivesTextMaxLenght = 20;    // set to usefull initial value
      break;
    case WM_TOGGLE_VISIBILITY:
      if ( !g.WinDrivesVisible )
        {
        WinPostMsg(g.hwndWinDrives, WM_WIN_DRIVES_VISIBLE, 0, 0);
        }
      else
        {
        WinPostMsg(g.hwndWinDrives, WM_WIN_DRIVES_INVISIBLE, 0, 0);
        }
      break;
    case WM_WIN_DRIVES_VISIBLE:
      TRACE("Show WinDrives");
      WinPostMsg(g.hwndDriveInfo, WM_DRIVE_UPDATE, 0, 0);
      WinPostMsg(g.hwndDriveInfo, WM_DRIVE_START_TIMER, 0, 0);
      g.WinDrivesVisible = TRUE;
      arrangeWinDrives(hwnd);
      rc = WinShowWindow(g.hwndWinDrives, TRUE);
      //rc = WinShowWindow(g.hwndMainDialogBox, TRUE);
      //WinQueryWindow(hwndClient, QW_PARENT), FID_HORZSCROLL);
      //rc |= WinShowWindow(WinWindowFromID(g.hwndWinDrives, FID_VERTSCROLL), TRUE);
      //TRACE1("rc=%d", rc);
      WinSendDlgItemMsg(g.hwndMainDialogBox, PB_DRIVES, BM_SETCHECK, MPFROMSHORT(TRUE), 0);
      iCount = DRIVES_TIMER_RELOAD;
      ulTimerId = WinStartTimer(hab, hwnd, 0, DRIVES_TIMER_TIME_IN_MS);
      TRACE1("Timer started %ld", ulTimerId);
      WinQueryWindowRect(g.hwndWinDrives, &rclWork);
      TRACE2("hwndWinDrives.x = %d to %d", rclWork.xLeft, rclWork.xRight);
      TRACE2("hwndWinDrives.y = %d to %d", rclWork.yTop, rclWork.yBottom);
      TRACE1("g.hwndWinDrives: 0x%0X", g.hwndWinDrives);
      break;
    case WM_WIN_DRIVES_INVISIBLE:
      TRACE("Hide WinDrives");
      g.WinDrivesVisible = FALSE;
      WinPostMsg(g.hwndDriveInfo, WM_DRIVE_STOP_TIMER, 0, 0);
      WinShowWindow(g.hwndWinDrives, FALSE);
      WinSendDlgItemMsg(g.hwndMainWin, PB_DRIVES, BM_SETCHECK, MPFROMSHORT(FALSE), 0);
      break;
    case WM_TIMER:
      //iCount = DRIVES_TIMER_RELOAD; // disable timer overflow for test purposes
      //TRACE("WM_Timer");
      iCount--;
      /* get current mouse pointer position */
      WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
      /* map from desktop to client window */
      fSuccess |= WinMapWindowPoints(HWND_DESKTOP, g.hwndWinDrives, &ptlMouse, 1);
      //sprintf(szMsg, "x = %ld   y = %ld", ptlMouse.x, ptlMouse.y);
      //TRACE1("g.hwndMainWin = 0x%0X", g.hwndMainWin);
      //WinSetDlgItemText(g.hwndMainWin, PB_DRIVES, szMsg);        // 20090225

      /* check if new mouse position is inside the WIN_DRIVES */
      WinQueryWindowRect(g.hwndWinDrives, &rclWork);
      if ( WinPtInRect(hab, &rclWork, &ptlMouse) )
        {   /* pointer is in WIN_DRIVES */
        iCount = DRIVES_TIMER_RELOAD;
        }
      WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
      fSuccess |= WinMapWindowPoints(HWND_DESKTOP, DlgItemHwnd(g.hwndMainWin, PB_DRIVES), &ptlMouse, 1);
      WinQueryWindowRect(DlgItemHwnd(g.hwndMainWin, PB_DRIVES), &rclWork);
      if ( WinPtInRect(hab, &rclWork, &ptlMouse) )
        {   /* pointer is in PB_DRIVES */
        iCount = DRIVES_TIMER_RELOAD;
        }
      if ( !(iCount > 0) )    // timeout for drives list box
        {
        WinStopTimer(hab, hwnd, ulTimerId);
        WinPostMsg(g.hwndWinDrives, WM_WIN_DRIVES_INVISIBLE, 0, 0);
        }
      break;

    case WM_NEW_DRIVE_INFO:
      TRACE("WM_NEW_DRIVE_INFO");
      g.iDrivesTextMaxLenght = 0;
      // fill in new drive info
      //TRACE_L2("fill into LB new drive info");
      k = g.iNumVisibleDrives;
      for ( i = 'A' - 'A' + 1, j = LB_DRIVES_FIRST_DRIVE_ENTRY; i <= 'Z' - 'A' + 1; i++ )
        {
        //TRACE2_L2("'%c', type=%d", i + 'A' - 1, DriveInfo[i].type);
        if ( DriveInfo[i].type != DT_UNKNOWN )
          {
          if ( g.iDrivesTextMaxLenght < strlen(DriveInfo[i].szDisplayString) )
            {
            g.iDrivesTextMaxLenght = strlen(DriveInfo[i].szDisplayString);
            }
          WinSetDlgItemText(hwnd, CB_LBOX_FIRST + j, DriveInfo[i].szDisplayString);
          WinShowWindow(DlgItemHwnd(hwnd, CB_LBOX_FIRST + j), TRUE);
          j++;
          }
        }
      g.iNumVisibleDrives = j;
      if ( g.iNumVisibleDrives != k )
        {
        TRACE1("g.iNumVisibleDrives=%d", g.iNumVisibleDrives);
        // hide buttons for which is no drive there
        while ( j <= (CB_LBOX_LAST - CB_LBOX_FIRST) )
          {
          WinSetDlgItemText(hwnd, CB_LBOX_FIRST + j, "-");
          WinShowWindow(DlgItemHwnd(hwnd, CB_LBOX_FIRST + j), FALSE);
          j++;
          }

        arrangeWinDrives(hwnd);
        }
      break;

    case WM_VSCROLL:
      TRACE2("WM_VSCROLL, pos: %d, code: %d", SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
      switch ( SHORT2FROMMP(mp2) )
        {
        case SB_LINEUP:     // Sent if the operator clicks on the up arrow of the scroll bar, or presses the VK_UP key.
          TRACE("SB_LINEUP");
          break;

        case SB_LINEDOWN:   // Sent if the operator clicks on the down arrow of the scroll bar, or presses the VK_DOWN key.
          TRACE("SB_LINEDOWN");
          break;

        case SB_PAGEUP:     // Sent if the operator clicks on the area above the slider, or presses the VK_PAGEUP key.
          TRACE("SB_PAGEUP");
          break;

        case SB_PAGEDOWN:   // Sent if the operator clicks on the area below the slider, or presses the VK_PAGEDOWN key.
          TRACE("SB_PAGEDOWN");
          break;

        case SB_SLIDERPOSITION: // Sent to indicate the final position of the slider.
          TRACE("SB_SLIDERPOSITION");
          break;

        case SB_SLIDERTRACK:    // If the operator moves the scroll bar slider with the pointer device, this is sent every time the slider position changes.
          TRACE("SB_SLIDERTRACK");
          break;

        case SB_ENDSCROLL:      // Sent when the operator has finished scrolling, but only if the operator has not been doing any absolute slider positioning.
          TRACE("SB_ENDSCROLL");
          break;
        }
      break;

    case WM_PAINT:
      //TRACE("WM_PAINT");
      break;

    case WM_MEASUREITEM:        //send LM_SETITEMHEIGHT to set item height
      //TRACE2("WM_MEASUREITEM: 0x%0lX 0x%0lX", mp1, mp2);
      //TRACE2("g.font.cy:%d  g.font.cx: %d", g.font.cy, g.font.cx);
      ret = MRFROM2SHORT(g.font.cy, (g.font.cx * g.iDrivesTextMaxLenght));
      break;

    case WM_COMMAND:
      TRACE("WM_COMMAND");

      break;

      /*      case WM_CLOSE:
                  break;
              */
    case WM_CONTROL:                                                  // @8
      TRACE("WM_CONTROL");
      switch ( SHORT1FROMMP(mp1) )
        {
        case CB_LBOX_FIRST:     // Hard Disks
          i = SHORT1FROMMP(mp1);
          // check if press or release
          if ( WinQueryButtonCheckstate(hwnd, i) )
            {   // release
            WinCheckButton(hwnd, i, 0);
            // search drives for Hard Drives and if yes, release it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'H' )
                {
                WinCheckButton(hwnd, i, 0);
                }
              }
            }
          else
            {   // press Hard Drives
            WinCheckButton(hwnd, i, 1);
            // search drives for Hard Drives and if yes, check it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'H' )
                {
                WinCheckButton(hwnd, i, 1);
                }
              }
            }
          CheckSumDrives(hwnd);
          break;

        case CB_LBOX1:          // Network Drives
          i = SHORT1FROMMP(mp1);
          // check if press or release
          if ( WinQueryButtonCheckstate(hwnd, i) )
            {   // release
            WinCheckButton(hwnd, i, 0);
            // search drives for Network Drives and if yes, release it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'N' )
                {
                WinCheckButton(hwnd, i, 0);
                }
              }
            }
          else
            {   // press Hard Drives
            WinCheckButton(hwnd, i, 1);
            // search drives for Network Drives and if yes, check it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'N' )
                {
                WinCheckButton(hwnd, i, 1);
                }
              }
            }
          CheckSumDrives(hwnd);
          break;

        case CB_LBOX2:          // CD/DVD Drives
          i = SHORT1FROMMP(mp1);
          // check if press or release
          if ( WinQueryButtonCheckstate(hwnd, i) )
            {   // release
            WinCheckButton(hwnd, i, 0);
            // search drives for CD Drives and if yes, release it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'C' )
                {
                WinCheckButton(hwnd, i, 0);
                }
              }
            }
          else
            {   // press Hard Drives
            WinCheckButton(hwnd, i, 1);
            // search drives for CD Drives and if yes, check it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'C' )
                {
                WinCheckButton(hwnd, i, 1);
                }
              }
            }
          CheckSumDrives(hwnd);
          break;

        case CB_LBOX3:          // Zip/EZ/Jaz Drives
          i = SHORT1FROMMP(mp1);
          // check if press or release
          if ( WinQueryButtonCheckstate(hwnd, i) )
            {   // release
            WinCheckButton(hwnd, i, 0);
            // search drives for Zip Drives and if yes, release it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'Z' )
                {
                WinCheckButton(hwnd, i, 0);
                }
              }
            }
          else
            {   // press Hard Drives
            WinCheckButton(hwnd, i, 1);
            // search drives for Zip Drives and if yes, check it too
            for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY - 1; i <= CB_LBOX_LAST; i++ )
              {
              WinQueryDlgItemText(hwnd, i, 2, cBuf);
              if ( cBuf[0] == 'Z' )
                {
                WinCheckButton(hwnd, i, 1);
                }
              }
            }
          CheckSumDrives(hwnd);
          break;

        case CB_LBOX4:          // All Drives
          i = SHORT1FROMMP(mp1);
          // check if press or release
          if ( WinQueryButtonCheckstate(hwnd, i) )
            {   // release
            WinCheckButton(hwnd, i, 0);
            // search drives for Hard Drives and if yes, release it too
            for ( i = CB_LBOX_FIRST; i <= CB_LBOX_LAST; i++ )
              {
              WinCheckButton(hwnd, i, 0);
              }
            }
          else
            {   // press Hard Drives
            WinCheckButton(hwnd, i, 1);
            // search drives for Hard Drives and if yes, check it too
            for ( i = CB_LBOX_FIRST; i <= CB_LBOX_LAST; i++ )
              {
              WinCheckButton(hwnd, i, 1);
              }
            }
          CheckSumDrives(hwnd);
          break;

        default:
          i = SHORT1FROMMP(mp1);
          if ( i >= CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY && i <= CB_LBOX_LAST )
            {
            WinQueryDlgItemText(hwnd, i, 6, cBuf);
            TRACE2("i=%d, cBuf='%s'", i, cBuf);
            TRACE1("CB_LBOX_ drive '%c:'", cBuf[3]);
            if ( WinQueryButtonCheckstate(hwnd, i) )
              {
              WinCheckButton(hwnd, i, 0);
              }
            else
              {
              WinCheckButton(hwnd, i, 1);
              }
            CheckSumDrives(hwnd);
            }
          break;
        }
      break;
    case WM_SIZE:
      TRACE("WM_SIZE");
      break;

    default:
      //if (mp2) TRACE3("not handled: msg:  0x%0lX, 0x%0lX 0x%0lX", msg, mp1, mp2);
      //TRACE1("unhandled msg:0x%0X", msg);
      return WinDefSecondaryWindowProc(hwnd, msg, mp1, mp2);
      break;

    } /* endswitch */
  return WinDefSecondaryWindowProc(hwnd, msg, mp1, mp2);
  //  return ret;
  }



/**********************************************************************
void CheckSumDrives(HWND hwnd)

Update text in PB_DRIVES (drives to search summary button) dependend
on current selecton in WinDrives dialog (drives list boxes)
**********************************************************************/
void CheckSumDrives(HWND hwnd)
  {
  int i, iOn;
  char cBuf[7];
  char sDrv[100] = "";
  int iSetAll = TRUE, iSetHard = TRUE, iSetCD = TRUE, iSetZip = TRUE, iSetNet = TRUE;
  int iFoundHard = FALSE, iFoundCD = FALSE, iFoundZip = FALSE, iFoundNet = FALSE;

  for ( i = CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY ; i <= CB_LBOX_LAST; i++ )
    {
    memset(cBuf, '\0', sizeof(cBuf) );
    WinQueryDlgItemText(hwnd, i, 7, cBuf);
    if ( !WinQueryButtonCheckstate(hwnd, i) )
      {
      //TRACE1("%s", cBuf);
      iSetAll = FALSE;
      switch ( cBuf[0] )
        {
        case 'H':
          iSetHard = FALSE;
          iFoundHard = TRUE;
          break;

        case 'N':
          iSetNet = FALSE;
          iFoundNet = TRUE;
          break;

        case 'C':
          iSetCD = FALSE;
          iFoundCD = TRUE;
          break;

        case 'Z':
          iSetZip = FALSE;
          iFoundZip = TRUE;
          break;

        case '-':   // no active drive, do nothing
          break;

        default:
          //TRACE2("i= %d, drive %c", i, cBuf[3]);
          iSetAll = FALSE;
          break;
        }
      }
    else
      {   // button is checked,
      switch ( cBuf[0] )
        {
        case 'H':
          iFoundHard = TRUE;
          strncat(sDrv, &cBuf[3], 3);
          break;

        case 'N':
          iFoundNet = TRUE;
          strncat(sDrv, &cBuf[3], 3);
          break;

        case 'C':
          iFoundCD = TRUE;
          strncat(sDrv, &cBuf[3], 3);
          break;

        case 'Z':
          iFoundZip = TRUE;
          strncat(sDrv, &cBuf[3], 3);
          break;

        default:
          if ( cBuf[0] != '-' )
            {
            strncat(sDrv, &cBuf[3], 3);
            }
        }

      }
    }

  if ( iFoundHard ) WinCheckButton(hwnd, CB_LBOX_FIRST, iSetHard);
  if ( iFoundNet ) WinCheckButton(hwnd, CB_LBOX1, iSetNet);
  if ( iFoundCD ) WinCheckButton(hwnd, CB_LBOX2, iSetCD);
  if ( iFoundZip ) WinCheckButton(hwnd, CB_LBOX3, iSetZip);
  WinCheckButton(hwnd, CB_LBOX4, iSetAll);

  if ( !strlen(sDrv) )
    {
    NlsGet("Drives to search (if not specified)", sDrv);
    }
  WinSetDlgItemText(g.hwndMainWin, PB_DRIVES, sDrv);
  }

