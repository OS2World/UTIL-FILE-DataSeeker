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
BOOL insertFoundFileText(PPLLITERDATA p);
BOOL insertFoundFile(PPLLITERDATA p);
BOOL resetSrchFTItem(PPLLITERDATA p);

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

VOID updateOptionsMenu(HWND hwnd, ULONG ulOption)
  {
  HWND hwndMenu = DlgItemHwnd(hwnd, FID_MENU);
  wMenuItemCheck(hwndMenu, CB_RECURSIVE, ulOption & OPT_RECUR);
  wMenuItemCheck(hwndMenu, CB_ONLYTXTFILES, ulOption & OPT_TEXTONLY);
  wMenuItemCheck(hwndMenu, CB_CASEINS, ulOption & OPT_CASEINS);
  wMenuItemCheck(hwndMenu, CB_BOOL, ulOption & OPT_BOOLOP);
  wMenuItemCheck(hwndMenu, CB_SETDIRSEARCH, ulOption & OPT_FIND_DIR);
  wMenuItemCheck(hwndMenu, IDM_VFDATE, ulOption & OPT_SHOWFDATE);
  wMenuItemCheck(hwndMenu, IDM_VFTIME, ulOption & OPT_SHOWFTIME);
  wMenuItemCheck(hwndMenu, IDM_VFSIZE, ulOption & OPT_SHOWFSIZE);
  wMenuItemCheck(hwndMenu, IDM_VLINENO, ulOption & OPT_SHOWLINENO);
  wMenuItemCheck(hwndMenu, IDM_VNOSPACES, ulOption & OPT_STRIPSPACES);

  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_SETRECUR, ulOption & OPT_RECUR);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_SETTEXTONLY, ulOption & OPT_TEXTONLY);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_SETCASEINS, ulOption & OPT_CASEINS);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_SETBOOLOP, ulOption & OPT_BOOLOP);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_SETDIRSEARCH, ulOption & OPT_FIND_DIR);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VFDATE, ulOption & OPT_SHOWFDATE);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VFTIME, ulOption & OPT_SHOWFTIME);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VFSIZE, ulOption & OPT_SHOWFSIZE);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VLINENO, ulOption & OPT_SHOWLINENO);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VNOSPACES, ulOption & OPT_STRIPSPACES);

  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTNONE, FALSE);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTASCEND, FALSE);
  wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTDESCEND, FALSE);
  switch ( g.sort )
    {
    case LIT_SORTASCENDING:
      wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTASCEND, TRUE);
      break;

    case LIT_SORTDESCENDING:
      wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTDESCEND, TRUE);
      break;

    default: // LIT_END:
      wMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTNONE, TRUE);
      break;
    }
  }


//===========================================================================
// Write the file data in a buffer optionally showing, file date, file time,
// file size and file name.
// Parameters --------------------------------------------------------------
// PSZ buf          : write buffer
// PFOUNDFILE pfile : file data
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID printFileDetails(PSZ buf, PFOUNDFILE pfile)
  {
  ULONG cb = 0;
  if ( g.setting & OPT_SHOWFDATE )
    {
    cb = sprintf(buf, "%d%s%02d%s%02d",
                 pfile->date.year + 1980, g.nls.date,
                 pfile->date.month, g.nls.date,
                 pfile->date.day);
    } /* endif */
  if ( g.setting & OPT_SHOWFTIME )
    {
    if ( cb ) memset(buf + cb, ' ', 2), cb += 2;
    cb += sprintf(buf + cb, "%02d%s%02d%s%02d",
                  pfile->time.hours, g.nls.time,
                  pfile->time.minutes, g.nls.time,
                  pfile->time.twosecs * 2);
    } /* endif */
  if ( g.setting & OPT_SHOWFSIZE )
    {
    szFromLongLong(buf + cb, 16, pfile->cb, g.nls.thousand[0], ' '); // 20090615 LFS
    cb += 15;
    } /* endif */
  if ( cb ) memset(buf + cb, ' ', 2), cb += 2;
  strcpy(buf + cb, pfile->name);
  }


//===========================================================================
// Emtpy and re-fills the file-found text-found listboxes according to the
// current visualization options.
// Parameters --------------------------------------------------------------
// HWND hwnd                : window handle
// BOOL bRefreshFileList    : TRUE/FALSE (refresh both lists / only text list)
// BOOL bSelect             : select first entry    // 20081119 AB
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID refreshResult(HWND hwnd, BOOL bRefreshFileList/*, BOOL bSelect*/)
  {
  FILLFILESFOUND fff;
  if ( g.pSrch )
    {
    if ( bRefreshFileList )
      {
      // empty the "found files" and "found text" listboxes
      fff.hCtrl = DlgItemHwnd(hwnd, LBOX_FILEFOUND);
      wLbxEmpty(fff.hCtrl);
      dLbxEmpty(hwnd, LBOX_TEXTFOUND);
      // fill the "found files" list box
      PLListIterateC(g.pSrch->result, NULLHANDLE, PPLLITER_NORECUR,
                     (g.pSrch->textSpec ? insertFoundFileText: insertFoundFile),
                     &fff, 0 );
      // select the first file found
      wLbxItemSelect(fff.hCtrl, 0);
      }
    else
      {
      dLbxEmpty(hwnd, LBOX_TEXTFOUND);
      foundFileSelected(hwnd, DlgItemHwnd(hwnd, LBOX_FILEFOUND)/*, bSelect*/);
      } /* endif */
    } /* endif */
  }


//===========================================================================
// Callback procedure used to fill the found files list box.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data
// Return value ------------------------------------------------------------
// BOOL : always return TRUE.
//===========================================================================
static BOOL insertFoundFileText(PPLLITERDATA p)
  {
  ULONG cb = 0;

  PFILLFILESFOUND pfff = (PFILLFILESFOUND)p->pParm;
  printFileDetails(pfff->buf, (PFOUNDFILE)p->pData);
  wLbxItemAndHndIns(pfff->hCtrl, g.sort, pfff->buf, p->hItem);
  return TRUE;
  }


//===========================================================================
// Callback procedure used to fill the found files list box.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data
// Return value ------------------------------------------------------------
// BOOL : always return TRUE.
//===========================================================================
static BOOL insertFoundFile(PPLLITERDATA p)
  {
  ULONG cb = 0;

  PFILLFILESFOUND pfff = (PFILLFILESFOUND)p->pParm;
  printFileDetails(pfff->buf, (PFOUNDFILE)p->pData);

  //20100301 AB changed to ..AndHnd.. to fix 'copy filename after changing sort order'
  wLbxItemAndHndIns(pfff->hCtrl, g.sort, pfff->buf, p->hItem);
  return TRUE;
  }


//===========================================================================
// Insert a new entry in the search result history listbox.
// Parameters --------------------------------------------------------------
// HWND hwnd      : window handle.
// PSEARCHRES psr : search result data.
// BOOL bLoop     : TRUE when it is called in a loop on loading a file
//                  containing a previously saved search results history.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID srchResHistAddEntry(HWND hwnd, PSEARCHRES psr, BOOL bLoop)
  {
  CHAR buf[LIST_BOX_WIDTH + 20];
  HWND hDDbox;
  ULONG citems;
  // build the history string:
  // [XXXXX] searched files specifications < searched text strings spec. >
  // where [XXXXX] are the search options
  sprintf(buf, "[%c%c%c%c%c] %.256s%s <%.256s%s>",    // 20091202 ToDo: 256 should be LIST_BOX_WIDTH / 2
          ((psr->option & OPT_RECUR)? 'R': '-'),      // recursive
          ((psr->option & OPT_TEXTONLY)? 'T': '-'),   // only text files
          ((psr->option & OPT_CASEINS)? 'I': '-'),    // case insensitive
          ((psr->option & OPT_BOOLOP)? 'B': '-'),     // boolean operators
          ((psr->option & OPT_FIND_DIR)? 'D': '-'),     // boolean operators
          String(psr->fileSpec),
          ((StringLen(psr->fileSpec) > LIST_BOX_WIDTH/2) ? "..." : ""),
          (psr->textSpec ? String(psr->textSpec) : ""),
          ((psr->textSpec && (StringLen(psr->textSpec) > LIST_BOX_WIDTH/2 - 20))? "..." : "")); // ToDo: 20091217 AB not really elegant
                                                                                                // but was never much better
  hDDbox = DlgItemHwnd(hwnd, DD_SRCHRES);
  wLbxItemAndHndIns(hDDbox, LIT_END, buf, psr);
  if ( !bLoop )
    {
    g.state |= IS_SKIPCTRLNOTIF;
    WinSetWindowText(hDDbox, buf);
    g.state &= ~IS_SKIPCTRLNOTIF;
    // check the history limit
    for ( citems = wLbxItemCount(hDDbox); citems > g.maxHistory; --citems )
      {
      psr = (PSEARCHRES)wLbxItemHnd(hDDbox, 0);
      srchResDel(psr);
      wLbxItemDel(hDDbox, 0);
      } /* endfor */
    } /* endif */
  }


//===========================================================================
// Reset the files found text showing the count of found files.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle.
// Return value ------------------------------------------------------------
// VOID
//
//  20100525 AB reworked for NLS
//
//===========================================================================

VOID resetFilesFoundText(HWND hwnd)
  {
  ULONG cb, cb2;
  CHAR buf[256];
  char cBuf[256];
  char cBuf2[256];
  int iLen;

  memset(cBuf, ' ', sizeof(cBuf));
  cb = WinLoadString(0, 0, IDS_FILESFOUND, sizeof(buf), buf);
  NlsGet(buf, cBuf);
  cb = strlen (cBuf);
  if ( cb > 40 )
    {
    cb = 40;
    TRACE1("limit string to %d", cb);
    cBuf[cb] = '\0';
    }
  //   szFromLong(cBuf + cb, 16,
  szFromLong(cBuf + cb, 13,         // 20081120 AB try to make looking better
             PLListItemsCount(g.pSrch->result, 0, 0),
             g.nls.thousand[0], ' ');

  //    TRACE1("%s", cBuf);
  memset(cBuf + cb + 12, ' ', 50); // 20081120 AB from 15 to 13 (12 !) and from 8 to 50
  cb += 25;                        // 20081120 AB from 23 to 25
  WinLoadString(0, 0, IDS_ELAPSEDTIME, sizeof(buf), buf);
  NlsGet(buf, cBuf2);
  cb2 = strlen (cBuf2);
  if ( cb2 > 40 )
    {
    cb2 = 40;
    TRACE1("limit string to %d", cb2);
    cBuf2[cb2] = '\0';
    }

  strncpy(cBuf + cb, cBuf2, sizeof(cBuf) - cb);
  cBuf[sizeof(cBuf) - 1] = '\0';
  cb += cb2;
  //    TRACE1("%s", cBuf);
  if ( strlen(cBuf) < sizeof(cBuf) - 30 )
    {
    strcat(cBuf, "       ");         // 20081120 AB added a few blanks (min. 2)
    //        TRACE1("%s", cBuf);
    szSecondsToFormattedTime(cBuf + cb + 3, g.ulElapsed / 1000, g.nls.time[0]);
    }
  TRACE1("%s", cBuf);
  DlgItemTextSet(hwnd, TXT_FILEFOUND, cBuf);
  }


//===========================================================================
// Reset the content of the Files-to-search or of the Text-to-search
// history lists.
// Parameters --------------------------------------------------------------
// HWND hCtrl    : listbox control window handle
// PPLLIST plist : list containing the new history list.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID resetSrchFilTxtHist(HWND hCtrl, PPLLIST plist)
  {
  wLbxEmpty(hCtrl);
  PLListIterateC(plist, 0, PPLLITER_NORECUR, resetSrchFTItem, hCtrl, 50);  // 20091026 AB max. 50 entries
  }


//===========================================================================
// Callback procedure used to fill the searched-files/text history listbox.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data
// Return value ------------------------------------------------------------
// BOOL : always return TRUE.
//===========================================================================
static BOOL resetSrchFTItem(PPLLITERDATA p)
  {
  wLbxItemIns((HWND)p->pParm, LIT_END, (PSZ)p->pData);
  //TRACE1("insert '%s'", (PSZ)p->pData);
  return TRUE;
  }


//===========================================================================
// Free all the resources used to keep the history of search results.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

VOID freeResultHistory(HWND hwnd)
  {
  HWND hDDbox;
  INT i;
  hDDbox = DlgItemHwnd(hwnd, DD_SRCHRES);
  for ( i = wLbxItemCount(hDDbox) - 1; i >= 0; --i )
    {
    srchResDel((PSEARCHRES)wLbxItemHnd(hDDbox, i));
    wLbxItemDel(hDDbox, i);
    } /* endfor */
  }


