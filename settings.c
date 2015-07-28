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
VOID saveDlgState(HWND hwnd);
VOID onCtrlNotify(HWND hwnd, ULONG id, ULONG ulNotify, HWND hCtrl);

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

MRESULT EXPENTRY setsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  switch ( msg )
    {
    case WM_INITDLG:
      {
        char cBuf[256];
        APIRET rc = 1;

        TRACE1("settings hwnd=0x%X", hwnd);
        // load NLS data for settings dialog
        NlsGet("Data Seeker settings", cBuf);           rc &= WinSetWindowText(hwnd, cBuf);

        NlsGet("Search", cBuf);                         rc &= WinSetDlgItemText(hwnd, GB_DEFAULTSEARCHOPTIONS, cBuf);
        NlsGet("~Recursive search", cBuf);              rc &= WinSetDlgItemText(hwnd, CB_SETRECUR, cBuf);
        NlsGet("E~xclude binary files", cBuf);          rc &= WinSetDlgItemText(hwnd, CB_SETTEXTONLY, cBuf);
        NlsGet("Case ~insensitive", cBuf);              rc &= WinSetDlgItemText(hwnd, CB_SETCASEINS, cBuf);
        NlsGet("~Boolean operators", cBuf);             rc &= WinSetDlgItemText(hwnd, CB_SETBOOLOP, cBuf);
        NlsGet("Search for ~folder names too", cBuf);   rc &= WinSetDlgItemText(hwnd, CB_SETDIRSEARCH, cBuf);

        NlsGet("General", cBuf);                        rc &= WinSetDlgItemText(hwnd, GB_GENERALOPTIONS, cBuf);
        NlsGet("~Edit files with", cBuf);               rc &= WinSetDlgItemText(hwnd, ST_EDITFILESWITH, cBuf);
        NlsGet("C~ommand shell", cBuf);                 rc &= WinSetDlgItemText(hwnd, ST_COMMANDSHELL, cBuf);
        NlsGet("~OK", cBuf);                            rc &= WinSetDlgItemText(hwnd, DID_OK, cBuf);
        NlsGet("~Cancel", cBuf);                        rc &= WinSetDlgItemText(hwnd, DID_CANCEL, cBuf);
        NlsGet("Help", cBuf);                           rc &= WinSetDlgItemText(hwnd, PB_SETHELP, cBuf);
        NlsGet("History", cBuf);                        rc &= WinSetDlgItemText(hwnd, GB_HISTORY, cBuf);
        NlsGet("~Sort entries", cBuf);                  rc &= WinSetDlgItemText(hwnd, CB_SORTHISTORYENTRIES, cBuf);
        NlsGet("~Maximum entries", cBuf);               rc &= WinSetDlgItemText(hwnd, ST_MAXIMUMENTRIES, cBuf);

        dBtnCheckSet(hwnd, CB_DEFVIEW, (g.setting & OPT_PREVVIEW) > 0);
        dBtnCheckSet(hwnd, CB_SHOWFDATE, (g.setting & OPT_SHOWFDATE) > 0);
        dBtnCheckSet(hwnd, CB_SHOWFTIME, (g.setting & OPT_SHOWFTIME) > 0);
        dBtnCheckSet(hwnd, CB_SHOWFSIZE, (g.setting & OPT_SHOWFSIZE) > 0);
        dBtnCheckSet(hwnd, CB_SHOWLINENO, (g.setting & OPT_SHOWLINENO) > 0);
        dBtnCheckSet(hwnd, CB_STRIPSPACES, (g.setting & OPT_STRIPSPACES) > 0);
        dBtnCheckSet(hwnd, CB_SETUSEPREV, (g.setting & OPT_PREVOPT) > 0);

        dBtnCheckSet(hwnd, CB_SETRECUR, (g.setting & OPT_RECUR) > 0);
        dBtnCheckSet(g.hwndMenuPuMain, PU_IDM_SETRECUR, (g.setting & OPT_RECUR) > 0);
        dBtnCheckSet(hwnd, CB_SETTEXTONLY, (g.setting & OPT_TEXTONLY) > 0);
        dBtnCheckSet(g.hwndMenuPuMain, PU_IDM_SETTEXTONLY, (g.setting & OPT_TEXTONLY) > 0);
        dBtnCheckSet(hwnd, CB_SETCASEINS, (g.setting & OPT_CASEINS) > 0);
        dBtnCheckSet(g.hwndMenuPuMain, PU_IDM_SETCASEINS, (g.setting & OPT_CASEINS) > 0);
        dBtnCheckSet(hwnd, CB_SETBOOLOP, (g.setting & OPT_BOOLOP) > 0);
        dBtnCheckSet(g.hwndMenuPuMain, PU_IDM_SETBOOLOP, (g.setting & OPT_BOOLOP) > 0);
        dBtnCheckSet(hwnd, CB_SETDIRSEARCH, (g.setting & OPT_FIND_DIR) > 0);
        dBtnCheckSet(g.hwndMenuPuMain, PU_IDM_SETDIRSEARCH, (g.setting & OPT_FIND_DIR) > 0);

        dBtnCheckSet(hwnd, CB_SORTHISTORYENTRIES, (g.setting & ~OPT_DONOTSORTHISTORY) > 0);
        mp1 = (MPARAM)(!(g.setting & OPT_PREVVIEW));
        // 20090321 AB disabled - no need
        mp1 = (MPARAM)(!(g.setting & OPT_PREVOPT));
        DlgItemEnableMulti(hwnd, msg, CB_SETRECUR, CB_SETDIRSEARCH, mp1);
        DlgItemEnableMulti(g.hwndMenuPuMain, msg, PU_IDM_SETRECUR, PU_IDM_SETDIRSEARCH, mp1);
        dSpinBtnSetValue(hwnd, SPB_SETHISTORY, g.maxHistory);
        if ( g.editor )
          {
          TRACE("editor found");
          DlgItemTextSet(hwnd, EF_SETEDIT, String(g.editor));
          }
        else
          {     // 20080xxx AB add default editor if not set (fixed program crash)
          TRACE("setting default editor");
          StringSet(g.editor, "e.exe", -1);
          }
        if ( g.cmdshell )
          DlgItemTextSet(hwnd, EF_SETCLISHELL, String(g.cmdshell));
        else
          {     // 20080xxx AB add default editor if not set (fixed program crash)
          TRACE("setting default command shell");
          StringSet(g.cmdshell, "cmd.exe", -1);
          }
        //return MRTRUE; // don't let PM set focus on dialog - 20100306 not what we like
        break;
      }
    case WM_COMMAND:
      switch ( (ULONG)mp1 )
        {
        case DID_OK:
          TRACE("save settings");
          saveDlgState(hwnd);
        case DID_CANCEL:
          WinDismissDlg(hwnd, (ULONG)mp1 == DID_OK);
          break;
        case PB_SETEDIT:
          break;
        case PB_SETCLISHELL:
          break;
        } /* endswitch */
      break;
    case WM_CONTROL:
      onCtrlNotify(hwnd,  SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), (HWND)mp2);
      break;
    case WM_CLOSE:
      WinDismissDlg(hwnd, FALSE);
      break;
    default:
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
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
static VOID saveDlgState(HWND hwnd)
  {
  ULONG ul;
  char *pszCopy;

#define SetPopUpMenu(item,state)    WinSendMsg(g.hwndMenuPuMain, MM_SETITEMATTR, MPFROM2SHORT(item, TRUE), MPFROM2SHORT(MIA_CHECKED, state) )
#define IsPopUpMenuSet(item)        WinSendMsg(g.hwndMenuPuMain, MM_QUERYITEMATTR, MPFROM2SHORT(item, TRUE),MPFROMSHORT(MIA_CHECKED))

  bitSetMaskValue(g.setting, OPT_RECUR, dBtnCheckState(hwnd, CB_SETRECUR) );
  SetPopUpMenu(PU_IDM_SETRECUR, dBtnCheckState(hwnd, CB_SETRECUR) );
  bitSetMaskValue(g.setting, OPT_TEXTONLY, dBtnCheckState(hwnd, CB_SETTEXTONLY) );
  SetPopUpMenu(PU_IDM_SETTEXTONLY, dBtnCheckState(hwnd, CB_SETTEXTONLY) );
  bitSetMaskValue(g.setting, OPT_CASEINS, dBtnCheckState(hwnd, CB_SETCASEINS) );
  SetPopUpMenu(PU_IDM_SETCASEINS, dBtnCheckState(hwnd, CB_SETCASEINS) );
  bitSetMaskValue(g.setting, OPT_BOOLOP, dBtnCheckState(hwnd, CB_SETBOOLOP) );
  SetPopUpMenu(PU_IDM_SETBOOLOP, dBtnCheckState(hwnd, CB_SETBOOLOP) );
  bitSetMaskValue(g.setting, OPT_FIND_DIR, dBtnCheckState(hwnd, CB_SETDIRSEARCH) );
  SetPopUpMenu(PU_IDM_SETDIRSEARCH, dBtnCheckState(hwnd, CB_SETDIRSEARCH) );
  // 20090319 AB make saved setting the current one
  g.setting = g.setting;
  TRACE1("g.setting: 0x%X", g.setting);
  if ( dSpinBtnLong(hwnd, SPB_SETHISTORY, &ul) ) g.maxHistory = ul;
  g.editor = DlgItemTextToString(hwnd, g.editor, EF_SETEDIT);
  g.cmdshell = DlgItemTextToString(hwnd, g.cmdshell, EF_SETCLISHELL);
  if ( !g.editor )
    {           // 20080xxx AB add default editor if not set (fixed program crash)
    TRACE("non valid string for 'editor', setting to default");
    g.editor = StringNew("e.exe", -1);
    }
  if ( !g.cmdshell )
    {           // 20080xxx AB add default cmd if not set (fixed program crash)
    TRACE("non valid string for 'CmdShell', setting to default");
    g.cmdshell = StringNew("cmd.exe", -1);
    }
  TRACE2("g.editor:  %d  '%s'", g.editor->len, String(g.editor));
  TRACE2("g.cmdshell:%d  '%s'", g.cmdshell->len, String(g.cmdshell));
  //  bitSetMaskValue(g.state, IS_EDITOREPM, fmFileNameComp(String(g.editor), "EPM.EXE"));
  // 20080822 AB
  pszCopy = strupr(strdup(String(g.editor))); // to upper case
  if ( strstr(pszCopy, "EPM.EXE") )
    {
    g.state |= IS_EDITOREPM; g.state &= ~IS_EDITORMED; g.state &= ~IS_EDITORVS;
    };
  if ( strstr(pszCopy, "MED.EXE") )
    {
    g.state |= IS_EDITORMED; g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORVS;
    };
  if ( strstr(pszCopy, "MED.CMD") )
    {
    g.state |= IS_EDITORMED; g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORVS;
    };
  if ( strstr(pszCopy, "VS.EXE" ) )
    {
    g.state |= IS_EDITORVS;  g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORMED;
    };
  if ( strstr(pszCopy, "VS.CMD" ) )
    {
    g.state |= IS_EDITORVS;  g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORMED;
    };
  if ( strstr(pszCopy, "VSA.EXE") )
    {
    g.state |= IS_EDITORVS;  g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORMED;
    };
  if ( strstr(pszCopy, "VSA.CMD") )
    {
    g.state |= IS_EDITORVS;  g.state &= ~IS_EDITOREPM; g.state &= ~IS_EDITORMED;
    };
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// ULONG id       : control ID
// ULONG ulNotify : notify code
// HWND hCtrl     : control handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static VOID onCtrlNotify(HWND hwnd, ULONG id, ULONG ulNotify, HWND hCtrl)
  {
  switch ( id )
    {
    case CB_DEFVIEW:         // Previous view
      if ( (ulNotify == BN_CLICKED) || (ulNotify == BN_DBLCLICKED) )
        {
        ulNotify = !wBtnCheckState(hCtrl);
        DlgItemEnableMulti(hwnd, id, CB_SHOWFDATE, CB_STRIPSPACES, ulNotify);
        } /* endif */
      break;
    case CB_SETUSEPREV:
      TRACE("CB_SETUSEPREV");
      if ( (ulNotify == BN_CLICKED) || (ulNotify == BN_DBLCLICKED) )
        {
        ulNotify = !wBtnCheckState(hCtrl);
        TRACE1("ulNotify=0x%X", ulNotify);
        DlgItemEnableMulti(hwnd, id, CB_SETRECUR, CB_SETDIRSEARCH, ulNotify);
        DlgItemEnableMulti(g.hwndMenuPuMain, id, PU_IDM_SETRECUR, PU_IDM_SETDIRSEARCH, ulNotify);
        } /* endif */
      break;

    default:
      break;
    } /* endswitch */
  }


