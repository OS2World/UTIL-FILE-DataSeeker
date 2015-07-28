/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// .c :
// - button and menu item commands procedures.
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------


// prototypes ---------------------------------------------------------------
VOID enableControlsOnSearch(HWND hwnd, BOOL bEnable);
BOOL insertTextLine(PPLLITERDATA p);
BOOL insertTextLineNo(PPLLITERDATA p);

// globals ------------------------------------------------------------------

//===========================================================================
// The Search/Stop button was clicked.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdSearch(HWND hwnd)
  {
  ULONG ul;
  char szBuf[LIST_BOX_WIDTH], *ptr;      // 20091020 changed to 512 cause of longer input strings
  int iLength;

  if ( !(g.state & IS_SEARCHING) )
    {
    g.bFileScrolled = FALSE;        // 20081119 AB new search started, reset scrolling
    g.bTextScrolled = FALSE;
    TRACE("reset bFileScrolled");
    if ( g.state & IS_MUSTFREERESULT )
      {
      g.state &= ~IS_MUSTFREERESULT;
      srchResDel(g.pSrch);
      g.pSrch = NULL;
      } /* endif */

    // 20091109 AB clear info area
    WinShowWindow(WinWindowFromID((g.hwndMainWin), (TXT_INFO)), FALSE);

    // 200808xx AB check if file specification ends with backslash or :, and ad * if yes
    iLength = WinQueryDlgItemText(hwnd, DD_FILESRCH, sizeof(szBuf) - 2, (PSZ)szBuf);
    TRACE2("search files:%.400s, length:%d", szBuf, iLength);
    //TRACE1("&szBuf=0x%X", szBuf);
    if ( (szBuf[iLength - 1] == '\\') || (szBuf[iLength - 1] == ':') )
      {
      TRACE("append *");
      ptr = strcat(szBuf, "*");
      WinSetDlgItemText(hwnd, DD_FILESRCH, (PSZ) ptr);
      }

    // remove previous entries from the found files/text listboxes
    dLbxEmpty(hwnd, LBOX_FILEFOUND);
    dMenuItemEnable(hwnd, IDM_SELECTED, FALSE);
    g.state &= ~IS_MSELENABLED;
    dLbxEmpty(hwnd, LBOX_TEXTFOUND);
    if ( NULL != (g.pSrch = srchResNew(hwnd)) )
      {
      // add the current search criterion to the drop down box content
      dLboxItemAddUnique(hwnd, DD_FILESRCH, String(g.pSrch->fileSpec), 0);
      if ( g.pSrch->textSpec ) dLboxItemAddUnique(hwnd, DD_TEXTSRCH, String(g.pSrch->textSpec), LSS_CASESENSITIVE);
      g.state |= IS_SEARCHING;
      // show the progress controls
      DlgItemShow(hwnd, DD_SRCHRES, FALSE);
      DlgItemTextLoad(hwnd, TXT_SRCHRES, IDS_SEARCHING);
      DlgItemTextSet(hwnd, EF_SEARCHING, "");
      DlgItemTextLoad(hwnd, TXT_FILEFOUND, IDS_FILESFOUND);
      DlgItemShow(hwnd, EF_SEARCHING, TRUE);
      // disable some controls and menu items
      enableControlsOnSearch(hwnd, FALSE);
      // make the worker thread start the search operation
      DosPostEventSem(g.objWnd.hev);
      WinPostMsg(g.objWnd.hwnd, OWM_SEARCH, (MPARAM)hwnd, MPVOID);
      }
    else
      {
      msgBox(hwnd, IDS_TITLE, IDSERR_MALLOC,
             MB_MOVEABLE | MB_ERROR | MB_MSGFROMRES | MB_TITLEFROMRES);
      } /* endif */
    } /* endif */
  }


//===========================================================================
// The Stop button was clicked.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdStop(HWND hwnd)
  {
  ULONG ul;
  // Stop button function
  if ( g.state & IS_SEARCHING )
    {
    DlgItemEnable(hwnd, PB_SEARCH, FALSE);
    // signal the object window to stop
    DosResetEventSem(g.objWnd.hev, &ul);
    g.state |= IS_STOPPED;
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

VOID cmdSearchEnd(HWND hwnd, ULONG rc)
  {
  if ( g.state & IS_WAITSEARCHEND )
    {
    WinPostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    }
  else if ( rc >= ERROR_PMSEEK )
    {
    printApplicationError(hwnd, rc, NULL);
    }
  else if ( rc )
    {
    printDosApiError(hwnd, rc);
    // success! Store the current search criterion in the history drop down box
    }
  else
    {
    if ( g.state & IS_STOPPED )
      {
      g.state &= ~IS_STOPPED;
      g.state |= IS_MUSTFREERESULT;
      }
    else
      {
      // insert the current search result into the history listbox
      srchResHistAddEntry(hwnd, g.pSrch, FALSE);
      } /* endif */
    // show the count of found files
    resetFilesFoundText(hwnd);
    // select the first file
    if ( !g.bFileScrolled )                 // 20081120 AB only when not scrolled during serach
      {
      //  select select first file entry
      TRACE("select first file entry");
      dLbxItemSelect(hwnd, LBOX_FILEFOUND, 0);
      }
    else
      {
      TRACE("do NOT select first file entry");
      }
    } /* endif */
  cmdResetSearchControls(hwnd);
  // AGGIUNGERE CODICE PER MOSTRARE LISTA ERRORI !!!
  if ( g.errors )
    {
    PLListDel(g.errors);
    g.errors = NULL;
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

VOID cmdMoveResultCtls(HWND hwnd, INT delta)
  {
  RECTL r;
  if ( delta > 0 )
    {            // move up
    if ( g.state & IS_RESLBOXTEXTHIDDEN )
      {          // -> same height
      g.state &= ~IS_RESLBOXTEXTHIDDEN;
      DlgItemEnable(hwnd, PB_MOVEDOWN, TRUE);
      }
    else if ( g.state & IS_RESLBOXFFMIN )
      {        // illegal move
      WinAlarm(HWND_DESKTOP, WA_WARNING);
      }
    else
      {                                       // minimize file list
      g.state |= IS_RESLBOXFFMIN;
      DlgItemEnable(hwnd, PB_MOVEUP, FALSE);
      } /* endif */
    }
  else
    {                    // move down
    if ( g.state & IS_RESLBOXTEXTHIDDEN )
      {          // illegal move
      WinAlarm(HWND_DESKTOP, WA_WARNING);
      }
    else if ( g.state & IS_RESLBOXFFMIN )
      {        // -> same height
      DlgItemEnable(hwnd, PB_MOVEUP, TRUE);
      g.state &= ~IS_RESLBOXFFMIN;
      }
    else
      {                                       // minimize text list
      g.state |= IS_RESLBOXTEXTHIDDEN;
      DlgItemEnable(hwnd, PB_MOVEDOWN, FALSE);
      } /* endif */
    } /* endif */
  WinQueryWindowRect(hwnd, &r);
  arrangeControls(hwnd, r.xRight, r.yTop);
  }


//===========================================================================
// Change a search or view option.
// Parameters --------------------------------------------------------------
// HWND hwnd  : window handle
// ULONG id   : menu item ID
// ULONG flag : new option bit flag
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdSetOption(HWND hwnd, ULONG id, ULONG flag)
  {
  //TRACE3("g.setting=0x%X, flag=0x%X, g.setting & OPT_SHOWFDATE = 0x%X", g.setting, flag, g.setting & OPT_SHOWFDATE);
  bitToggleMask(g.setting, flag);
  //TRACE1("g.setting=0x%X", g.setting);
  flag = g.setting & flag;
  dMenuItemCheck(hwnd, id, flag);
  if ( (id >= IDM_VFDATE) && (id <= IDM_VFSIZE) )
    {
    refreshResult(hwnd, TRUE/*, FALSE*/);     // 20081119 AB for bSelect
    }
  else if ( id == IDM_VLINENO )
    {
    refreshResult(hwnd, FALSE/*, FALSE*/);
    } /* endif */
  }


//===========================================================================
// Change the sort options.
// Parameters --------------------------------------------------------------
// HWND hwnd  : window handle
// ULONG id   : menu item ID
// ULONG flag : new sort mode
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
// ToDo: 20100225 not needed anymore
/*VOID cmdSortOption(HWND hwnd, ULONG id, ULONG flag)
    {
    dMenuItemCheck(hwnd, id, TRUE);
    dMenuItemCheck(hwnd, IDM_VSORTNONE - g.sort - 1, FALSE);
    dMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTNONE - g.sort - 1, FALSE);
    g.sort = flag;
    refreshResult(hwnd, TRUE/*, FALSE*//*);
    }
  */

//===========================================================================
// Restore the text and the visibility of some controls as soon as the
// the search terminates.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdResetSearchControls(HWND hwnd)
  {
  g.state &= ~IS_SEARCHING;
  DlgItemShow(hwnd, DD_SRCHRES, TRUE);
  DlgItemShow(hwnd, EF_SEARCHING, FALSE);
  DlgItemTextLoad(hwnd, TXT_SRCHRES, IDS_SELRES);
  enableControlsOnSearch(hwnd, TRUE);
  }


//===========================================================================
// During searchs disables the submenus (excuding Help), the drop down
// boxes and the Result browsing buttons.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================
static VOID enableControlsOnSearch(HWND hwnd, BOOL bEnable)
  {
  DlgItemEnable(hwnd, PB_SEARCH, bEnable);
  DlgItemEnable(hwnd, PB_STOP, !bEnable);
  DlgItemEnable(hwnd, DD_FILESRCH, bEnable);
  DlgItemEnable(hwnd, DD_TEXTSRCH, bEnable);
  hwnd = DlgItemHwnd(hwnd, FID_MENU);
  wMenuItemEnable(hwnd, IDM_FILE, bEnable);
  //   wMenuItemEnable(hwnd, IDM_OPTIONS, bEnable);
  }


//===========================================================================
// If the search mode is file and text show the text found in the selected
// file.
// Parameters --------------------------------------------------------------
// HWND hwnd    : window handle.
// HWND hCtrl   : file found listbox window handle.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID foundFileSelected(HWND hwnd, HWND hCtrl)
  {
  HNDPLLITEM hLinesList;
  HWND hwndTxtList;
  if ( !(g.state & IS_MSELENABLED) )
    {
    g.state |= IS_MSELENABLED;
    dMenuItemEnable(hwnd, IDM_SELECTED, TRUE);
    } /* endif */
  if ( g.pSrch->textSpec )
    {
    hwndTxtList = DlgItemHwnd(hwnd, LBOX_TEXTFOUND);
    wLbxEmpty(hwndTxtList);
    hLinesList = wLbxItemSelectedHnd(hCtrl);
    PLListIterateC(g.pSrch->result, hLinesList, PPLLITER_NORECUR,
                   ((g.setting & OPT_SHOWLINENO) ? insertTextLineNo : insertTextLine),
                   hwndTxtList, 0);
    // 20080822 AB select first entry in Textfound box (to start edit with correct line)
    if ( !g.bTextScrolled )
      {
      TRACE("SetListBoxCursor TEXT to first entry");
      WinSendMsg (WinWindowFromID (hwnd, LBOX_TEXTFOUND), LM_SELECTITEM, MPFROMSHORT ((SHORT) 0), MPFROMSHORT ((BOOL) TRUE));
      }
    else
      {
      TRACE("SetListBoxCursor TEXT NOT to first entry");
      }

    } /* endif */
  }


//===========================================================================
// Callback procedure used to fill the found text list box.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data
// Return value ------------------------------------------------------------
// BOOL : always return TRUE.
//===========================================================================
static BOOL insertTextLine(PPLLITERDATA p)
  {
  wLbxItemAndHndIns((HWND)p->pParm, LIT_END,
                    ((PTEXTLINE)p->pData)->line + 7,
                    ((PTEXTLINE)p->pData)->lineno);
  return TRUE;
  }


//===========================================================================
// Callback procedure used to fill the found text list box.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data
// Return value ------------------------------------------------------------
// BOOL : always return TRUE.
//===========================================================================
static BOOL insertTextLineNo(PPLLITERDATA p)
  {
  wLbxItemAndHndIns((HWND)p->pParm, LIT_END,
                    ((PTEXTLINE)p->pData)->line,
                    ((PTEXTLINE)p->pData)->lineno);
  return TRUE;
  }


//===========================================================================
// An entry in the search result history has been selected: update the
// options, the files - text to search fields, the file found list and
// select the first file.
// Parameters --------------------------------------------------------------
// HWND hwnd  : main window handle
// HWND hCtrl : handle of the history drop down box
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID historyEntrySelected(HWND hwnd, HWND hCtrl)
  {
  TRACE("historyEntrySelected");
  if ( g.state & IS_MUSTFREERESULT )
    {
    g.state &= ~IS_MUSTFREERESULT;
    srchResDel(g.pSrch);
    } /* endif */
  g.pSrch = (PSEARCHRES)wLbxItemSelectedHnd(hCtrl);
  // 20100217 work around when hitting DEL in history EF, check if g.pSrch is valid and return if not
  if ( g.pSrch == 0 )
    {
    TRACE("invalid selection (did you press DEL Guillaume? ;-)");
    return;
    }
  // update the options
  updateOptionsMenu(hwnd, g.pSrch->option);
  // update the "file to search" field
  TRACE1("String(g.pSrch->fileSpec)=%s", String(g.pSrch->fileSpec));
  TRACE1("text=%s", (g.pSrch->textSpec? String(g.pSrch->textSpec) : "__") );
  DlgItemTextSet(hwnd, DD_FILESRCH, String(g.pSrch->fileSpec));
  // update the "text to search" field
  DlgItemTextSet(hwnd, DD_TEXTSRCH, (g.pSrch->textSpec? String(g.pSrch->textSpec) : ""));
  //TRACE1("DD_TEXTSRCH length=%d", WinQueryWindowTextLength( WinWindowFromID(hwnd, DD_TEXTSRCH) ) );
  TRACE1("text=%s", (g.pSrch->textSpec? String(g.pSrch->textSpec) : "__") );
  refreshResult(hwnd, TRUE/*, FALSE*/);
  //TRACE1("length=%d", WinQueryWindowTextLength( WinWindowFromID(hwnd, DD_FILESRCH) ) );
  }


//===========================================================================
// Delete the selected entry from the listbox holding the focus.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdDeleteEntry(HWND hwnd)
  {
  HWND hFocus;
  INT iitem;
  PSEARCHRES psr;
  // get the handle of the focus window
  hFocus = WinQueryFocus(HWND_DESKTOP);
  // if it is the entry field of a drop down box
  if ( !(g.state & IS_BUSY) &&
       (CBID_EDIT == WinQueryWindowUShort(hFocus, QWS_ID)) )
    {
    // get the handle of the drop down box
    hFocus = WinParent(hFocus);
    WinEnableWindow(hFocus, FALSE);
    // if there is a selected item
    if ( 0 <= (iitem = wLbxItemSelected(hFocus)) )
      {
      // check if this is the history of results drop down box
      if ( DD_SRCHRES == WinQueryWindowUShort(hFocus, QWS_ID) )
        {
        // get the associated result data
        psr = (PSEARCHRES)wLbxItemHnd(hFocus, iitem);
        // empty the files found and text found lists
        dLbxEmpty(hwnd, LBOX_FILEFOUND);
        dLbxEmpty(hwnd, LBOX_TEXTFOUND);
        // free the result data
        srchResDel(psr);
        g.pSrch = NULL;
        } /* endif */
      // remove the item
      wLbxItemDel(hFocus, iitem);
      _heapmin();
      }
    else
      {
      goto warning;
      } /* endif */
    WinSetWindowText(hFocus, "");
    WinEnableWindow(hFocus, TRUE);
    }
  else
    {
    goto warning;
    } /* endif */
  return;
  warning:
  WinAlarm(HWND_DESKTOP, WA_WARNING);
  }


//===========================================================================
// Delete all entries from the listbox holding the focus.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdDeleteAllEntries(HWND hwnd)
  {
  HWND hFocus;
  INT iitem;
  PSEARCHRES psr;
  // get the handle of the focus window
  hFocus = WinQueryFocus(HWND_DESKTOP);
  // if it is the entry field of a drop down box
  if ( !(g.state & IS_BUSY) &&
       (CBID_EDIT == WinQueryWindowUShort(hFocus, QWS_ID)) )
    {
    // get the handle of the drop down box
    hFocus = WinParent(hFocus);
    WinEnableWindow(hFocus, FALSE);
    if ( !wLbxItemCount(hFocus) ) goto warning;
    // check if this is the history of results drop down box
    if ( DD_SRCHRES == WinQueryWindowUShort(hFocus, QWS_ID) )
      {
      // empty the files found and text found lists
      dLbxEmpty(hwnd, LBOX_FILEFOUND);
      dLbxEmpty(hwnd, LBOX_TEXTFOUND);
      // free all the history of results
      freeResultHistory(hwnd);
      _heapmin();
      }
    else
      {
      wLbxEmpty(hFocus);
      } /* endif */
    WinSetWindowText(hFocus, "");
    WinEnableWindow(hFocus, TRUE);
    }
  else
    {
    goto warning;
    } /* endif */
  return;
  warning:
  WinAlarm(HWND_DESKTOP, WA_WARNING);
  }


//===========================================================================
// Edit the selected file.
// Parameters --------------------------------------------------------------
// HWND hwnd         : window handle
// BOOL sameInstance : edit the file in a previous editor instance.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

VOID cmdSelectedEdit(HWND hwnd, BOOL sameInstance)
  {
  CHAR buf[300];
  INT lineNo;
  PFOUNDFILE pff;
  // get the selected file data
  pff = (PFOUNDFILE)PLListItemData(g.pSrch->result,
                                   dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND));
  lineNo = (INT)dLbxItemSelectedHnd(hwnd, LBOX_TEXTFOUND);
  if ( (g.state & IS_EDITOREPM) && (lineNo >= 0) )
    {       // 2008090x AB call EPM with 'postme' (requested by Andreas Schnellbacher)
    sprintf(buf, "%s \"%s\" \'postme %d\'", (sameInstance ? "/R" : ""), pff->name, lineNo + 1);
    TRACE("Editor is EPM");
    }
  else if ( (g.state & IS_EDITORMED) && (lineNo >= 0) )
    {
    sprintf(buf, "\"%s\" %d", pff->name, lineNo + 1);
    TRACE("Editor is MED");
    }
  else if ( (g.state & IS_EDITORVS) && (lineNo >= 0) )
    {
    sprintf(buf, "+new \"%s\" -#%d", pff->name, lineNo + 1);
    TRACE("Editor is VisualSlickEdit");
    }
  else
    { // unknown editor, starting without params
    TRACE("unknown editor, using no command line options");
    sprintf(buf, "\"%s\"", pff->name);
    } /* endif */
  // 20090823 ABcheck if editor is set and use e.exe if not
  if ( !g.editor )
    {
    TRACE("g.editor is not set - using e.exe");
    g.editor = StringNew("e.exe", -1);
    }
  TRACE2("StartApp with %s %s", String(g.editor), buf);
  if ( !applStartApp(hwnd, String(g.editor), buf, NULL, NULL, 0) )
    WinAlarm(HWND_DESKTOP, WA_ERROR);
  }


//===========================================================================
// Copy Filename to clipboard surrounded by "      20081008 AB added
// Parameters --------------------------------------------------------------
// HWND hwnd         : window handle
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

VOID cmdSelectedCopyFilename(HWND hwnd)
  {
  HAB hab;                 /* anchor-block handle. */
  CHAR buf[300];
  INT lineNo;
  PFOUNDFILE pff;
  SHORT shIndex;
  ULONG ulHndl;

  // get the selected file data
  ulHndl = dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND);
  TRACE1("ulHndl=%d", ulHndl);
  pff = (PFOUNDFILE)PLListItemData(g.pSrch->result, ulHndl );

  TRACE1("FileName=%s", pff->name);

  if ( WinOpenClipbrd(hab) )
    {
    PVOID pvShrObject = NULL;
    if ( DosAllocSharedMem(&pvShrObject, NULL, strlen(pff->name)+3, PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE) == NO_ERROR )
      {
      strcpy(pvShrObject, "\"");
      strcat(pvShrObject, pff->name);
      strcat(pvShrObject, "\"");
      WinSetClipbrdData(hab, (ULONG)pvShrObject, CF_TEXT, CFI_POINTER);
      }
    }
  WinCloseClipbrd(hab);
  }


//===========================================================================
// Copy whole line to clipboard surrounded by "      20081008 AB added
// Parameters --------------------------------------------------------------
// HWND hwnd         : window handle
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

#define BUF_LEN 300

VOID cmdSelectedCopyLine(HWND hwnd)
  {
  HAB hab;                 /* anchor-block handle. */
  CHAR buf[BUF_LEN];
  INT lineNo;
  PFOUNDFILE pff;
  // get the selected file data
  //pff = (PFOUNDFILE)PLListItemData(g.pSrch->result,
  //                             dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND));
  dLbxItemSelectedText(hwnd, LBOX_FILEFOUND, BUF_LEN, &buf);
  TRACE1("buf=%s", buf);
  if ( WinOpenClipbrd(hab) )
    {
    PVOID pvShrObject = NULL;
    if ( DosAllocSharedMem(&pvShrObject, NULL, strlen(buf)+1, PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE) == NO_ERROR )
      {
      strcpy(pvShrObject, buf);
      WinSetClipbrdData(hab, (ULONG)pvShrObject, CF_TEXT, CFI_POINTER);
      }
    }
  WinCloseClipbrd(hab);

  }


//===========================================================================
// Edit the selected file.
// Parameters --------------------------------------------------------------
// HWND hwnd         : window handle
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

VOID cmdSelectedCopyAllFilenames(HWND hwnd)
  {
  CHAR buf[300];
  INT lineNo;
  PFOUNDFILE pff;
  // get the selected file data
  pff = (PFOUNDFILE)PLListItemData(g.pSrch->result,
                                   dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND));
  TRACE1("FileName=%s", pff->name);
  }


//===========================================================================
// Use the WPS interface to open a file via the associated program or
// as a settings notebook.
// Parameters --------------------------------------------------------------
// HWND hwnd  : window handle
// ULONG flag : open mode (OPEN_DEFAULT / OPEN_SETTINGS)
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

VOID cmdSelectedOpen(HWND hwnd, ULONG flag)
  {
  PFOUNDFILE pff;
  // get the selected file data
  pff = (PFOUNDFILE)PLListItemData(g.pSrch->result,
                                   dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND));
  WinOpenObject(WinQueryObject(pff->name), flag, TRUE);
  }


//===========================================================================
// If 'openDir' is TRUE open the folder containing the selected file,
// otherwise open a command line window.
// Parameters --------------------------------------------------------------
// HWND hwnd    : window handle
// BOOL openDir : TRUE/FALSE (open as folder / as command line)
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID cmdSelectedPath(HWND hwnd, BOOL openDir)
  {
  CHAR buf[CCHMAXPATH];
  CHAR buf2[CCHMAXPATH];
  CHAR buf3[CCHMAXPATH];
  char *p;
  PFOUNDFILE pff;
  ULONG cb;
  int i, iLen;

  // get the selected file data
  pff = (PFOUNDFILE)PLListItemData(g.pSrch->result,
                                   dLbxItemSelectedHnd(hwnd, LBOX_FILEFOUND));
  cb = (ULONG)(fmFileNameFromPath(pff->name) - pff->name) - 1;
  memcpy(buf3, pff->name, cb);   // copy path of selected item into buf3
  buf3[cb] = 0;
  if ( openDir )
    {
    WinOpenObject(WinQueryObject(buf3), OPEN_DEFAULT, TRUE);
    }
  else
    {
    // 20090823 AB check if cmdshell is set and use cmd.exe as default if not
    if ( !g.cmdshell )
      {
      g.cmdshell = StringNew("cmd.exe", -1);
      }
    // 20130626 AB extract parameters from string <-- ToDo handle programms which include blank in name surrounded with "
    // copy parameters to buf2
    p = strstr(String(g.cmdshell), " ");
    if ( p )
       {
       strcpy(buf2, p);
       }
    i = 0;
    p = String(g.cmdshell);
    while (*p && i < CCHMAXPATH - 2)
       {
       if (*p == ' ') break;
       buf[i] = *p;
       p++;
       i++;
       }
    buf[i] = '\0'; // skip all after blank from program name
    TRACE2("app='%s', params='%s'", buf, buf2);
    // 20080902 AB changed to open cli with switch to winlist
    applStartApp(hwnd, buf, buf2, buf3, NULL, APPSTART_WINSWITCHTOPROGRAMM);
    } /* endif */
  }

