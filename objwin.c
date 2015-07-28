/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// objwin.c :
// - worker thread procedure (object window)
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------
#define SZ_OBJWNDCLASS     "stdObjWnd"

// prototypes ---------------------------------------------------------------

// in pmseek.h VOID /*_System*/ objWinThreadProc(POBJECTWND pObjWnd);       // 20091118 AB changed to _beginthread
MRESULT EXPENTRY objWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID searchFilesAndText(HWND hwndNotify);
BOOL checkFindText(PDOSEARCH pds);
BOOL scanFiles(PTXTWORD pwFileSpec, PDOSEARCH pds);
LONG fnTextSearch(PSCANTREE pst);
PPLLIST searchTextOr(PPLLIST pLines, PDOSEARCH pds);
PPLLIST searchTextIOr(PPLLIST pLines, PDOSEARCH pds);
PPLLIST searchTextAnd(PPLLIST pLines, PDOSEARCH pds);
PPLLIST searchTextIAnd(PPLLIST pLines, PDOSEARCH pds);
PPLLIST searchText(PPLLIST pLines, PDOSEARCH pds);
PPLLIST searchTextI(PPLLIST pLines, PDOSEARCH pds);
BOOL linkLine(PPLLIST* ppList, PDOSEARCH pds);
LONG fnFileMatch(PSCANTREE pst);
PSZ nextLine(PSZ psz, PSZ* ppendl);
PSZ stripLeadSpaces(PSZ p);
VOID replaceNuls(PSZ p, ULONG cbData);
BOOL ExpandWithDriveLetters(PTXTWORD txtWord, PVOID pszDrives);

// globals ------------------------------------------------------------------


//===========================================================================
// start the object window thread
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL startObjectThread(POBJECTWND pObjWnd, ULONG cbStack)
  {
  APIRET rc;
  if ( (NO_ERROR == (rc = DosCreateEventSem((PSZ)NULL, &pObjWnd->hev, 0L, FALSE)))
       &&
       -1 != (g.objWnd.tid = _beginthread(objWinThreadProc, NULL, cbStack, pObjWnd) )  // 20091118 AB changed to _beginthread
       /*(NO_ERROR == (rc =
                    DosCreateThread(&g.objWnd.tid, (PFNTHREAD)g.objWnd.tid,
                                    (ULONG)pObjWnd,
                                    CREATE_READY | STACK_SPARSE, cbStack)))*/
     )
    rc = DosWaitEventSem(pObjWnd->hev, SEM_INDEFINITE_WAIT);
  if ( rc ) printDosApiError(NULLHANDLE, rc);
  return(BOOL)!rc;
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

VOID /*_System*/ objWinThreadProc(/*POBJECTWND*/ void *arg)
  {       // 20091118 AB changed to _beginthread
  EXCEPTIONREGISTRATIONRECORD exRegRec;
  POBJECTWND pObjWnd;
  QMSG qmsg;
  HMQ hmq;
  HAB hab;

  LoadExceptq(&exRegRec, NULL, NULL);

  pObjWnd = arg;
  hmq = WinCreateMsgQueue(hab = WinInitialize(0), 0);
  // make only the first thread receive WM_QUIT on shutdown
  WinCancelShutdown(hmq, TRUE);
  // register the object window class
  if ( WinRegisterClass(hab, SZ_OBJWNDCLASS, objWinProc, 0L, 0L) &&
       (NULLHANDLE != ( pObjWnd->hwnd =
                        WinCreateWindow(HWND_OBJECT, SZ_OBJWNDCLASS, NULL, 0, 0, 0, 0, 0,
                                        NULLHANDLE, HWND_BOTTOM, 0, NULL, NULL))) )
    {
    // signal the main thread that the worker thread is ready
    DosPostEventSem( pObjWnd->hev);
    // message loop
    WinStdMsgLoop(hab, &qmsg);
    } /* endif */
  WinStdEnd( pObjWnd->hwnd, hmq, hab);

  UninstallExceptq(&exRegRec);
  return;

  }


//===========================================================================
// Object window procedure
// Parameters --------------------------------------------------------------
// standard window procedure parameters
// Return value ------------------------------------------------------------
// MRESULT
//===========================================================================
static
            MRESULT EXPENTRY objWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  switch ( msg )
    {
    case OWM_SEARCH:
      searchFilesAndText((HWND)mp1);
      break;
    case OWM_QUIT:
      saveState((HWND)mp1);
      WinPostMsg(hwnd, WM_QUIT, MPVOID, MPVOID);
      WinPostMsg((HWND)mp1, OWM_QUIT, MPTRUE, MPVOID);
      break;
    default:
      return WinDefWindowProc(hwnd, msg, mp1, mp2);
    } /* endswitch */
  return(MRESULT)FALSE;
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================
static
            VOID searchFilesAndText(HWND hwndNotify)
  {
  DOSEARCH ds;
  ULONG cb;
  ULONG startTime, endTime;
  PSZ pszBuf, pszP, pszCombined;
  PSZ pszString;
  int iLen, i, rc;
  char szDrives[26 * 4 + 1];
  int iExit = FALSE;

  startTime = WinGetCurrentTime(NULLHANDLE);
  memset(&ds, 0, sizeof(ds));
  ds.hwnd = hwndNotify;
  ds.hwndLboxFile = DlgItemHwnd(hwndNotify, LBOX_FILEFOUND);
  ds.hwndProgress = DlgItemHwnd(hwndNotify, EF_SEARCHING);
  // allocate the search buffer
  cb = g.pSrch->textSpec ? sizeof(SEARCHBUFFER) : CB_BUFFERGENERIC;
  TRACE1("malloc g.pSrchBuf size %d", cb);
  if ( NULL == (g.pSrchBuf = malloc(cb)) ) goto error0;

  //TRACE1("g.pSrchBuf 0x%X", g.pSrchBuf);
  //TRACE1("g.pSrchBuf->fileData %s", g.pSrchBuf->fileData);
  //strcpy(g.pSrchBuf->fileData, "empty");
  //TRACE1("g.pSrchBuf->fileData %s", g.pSrchBuf->fileData);
  // 20090317 AB
  // set buffer to input buffer
  //TRACE2("pszBuf=0x%X, pszCombined=0x%X", pszBuf, pszCombined);
  pszBuf = String(g.pSrch->fileSpec);
  pszCombined = String(g.pSrch->fileSpec);
  TRACE1("String(g.pSrch->fileSpec)='%s'", String(g.pSrch->fileSpec));

  // check if muliple files were specified
  ds.fileSpec = WordArrayNew(pszCombined , strlen(pszCombined), '|', '^', TRUE);
  //TRACE2("pszBuf=0x%X, pszCombined=0x%X", pszBuf, pszCombined);
  TRACE2("pszCombined=%s, len=%d", pszCombined, strlen(pszCombined) );

  // check if drive is specified in file definition and expand if not with the drives selected in Drives Dialog
  WinQueryDlgItemText(g.hwndMainWin, PB_DRIVES, sizeof(szDrives), szDrives);
  rc = WordArrayIterate(ds.fileSpec, (PWAITERFN) ExpandWithDriveLetters, &szDrives, FALSE);
  TRACE1("rc=%d", rc);
  // now the wordarray holds the single specified files expanded by the drive letters
  // f.i. index 0: D:\*txt | E:\*txt | T:\*txt
  //      index 1: D:\rea* | E:\rea* | T:\rea*
  //      ....
  // what's left is - expand each index to hold only 1 entry
  // first converte the word array to one long string, and then back to word array
  pszString = WordArrayToString(ds.fileSpec, '|');
  if ( !pszString )
    {
    TRACE("can not convert to string");
    goto error1;
    }
  ds.fileSpec = WordArrayNew(pszString , strlen(pszString), '|', '^', TRUE);

  if ( !ds.fileSpec || !pszString ) goto error1;
  // free memory
  free (pszString);
  TRACE1("%d search criteria", ds.fileSpec->ci);    //<---- crash !!!!!!!!
  // check the searched text separating it in words if needed
  if ( g.pSrch->textSpec && !checkFindText(&ds) ) goto error2;
  // iterate the file scansion for each file entry
  WordArrayIterate(ds.fileSpec, (PWAITERFN)scanFiles, (PVOID)&ds, FALSE); // 20090923 AB continue search on next |   //TRUE);  // in &ds
  TRACE("I'm here");
  TRACE3("g.pSrch->result addr=0x%X, g.pSrch->result->cbTot=%d, g.pSrch->result->count=%d", g.pSrch->result, g.pSrch->result->cbTot, g.pSrch->result->count);
  if ( g.pSrch->result->count ) TRACE1("g.pSrchBuf->fileData %80s", g.pSrchBuf->fileData);    // 20140529 AB fix trap with DEBUG and no results
  free(g.pSrchBuf);
  //TRACE("after free");
  // realloc in PLListShrink does not work !!!!!!
  if ( g.pSrch->result ) if ( !PLListShrink(g.pSrch->result) ) TRACE("error reallocating (PLListShrink)");
    //TRACE("after PLListShrink");
  TRACE3("g.pSrch->result addr=0x%X, g.pSrch->result->cbTot=%d, g.pSrch->result->count=%d", g.pSrch->result, g.pSrch->result->cbTot, g.pSrch->result->count);
  _heapmin();
  //TRACE("after _heapmin");
  //TRACE1("ds.fileSpec=0x%X", ds.fileSpec);
  WordArrayDel(ds.fileSpec);
  //TRACE("after WordArrayDel ds.fileSpec");
  WordArrayDel(ds.textSpec);
  //TRACE("after WordArrayDel ds.textSpec");
  goto end;
  error2:
  TRACE("error2");
  WordArrayDel(ds.fileSpec);
  error1:
  TRACE("error1");
  free(g.pSrchBuf);
  error0:
  TRACE("error0");
  ds.rc = ERROR_NOT_ENOUGH_MEMORY;
  end:

  if ( iExit )
    { // close application if exception occured
    TRACE("iExit is TRUE");
    DosSleep(8000);
    exit ( 0xFF);
    }
  endTime = WinGetCurrentTime(NULLHANDLE);
  g.ulElapsed = (endTime < startTime) ?
                0xffffffff - startTime + endTime :
                endTime - startTime;
  WinPostMsg(hwndNotify, WM_COMMAND, (MPARAM)CMD_SEARCHEND, (MPARAM)ds.rc);
  }


//===========================================================================
// Parse the string to be searched separating it in words if the boolean
// operator option is active.
// Parameters --------------------------------------------------------------
// PDOSEARCH pds : search operating data
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================
static
            BOOL checkFindText(PDOSEARCH pds)
  {
  ULONG cwords = 0;
  PSZ p;
  if ( g.pSrch->option & OPT_BOOLOP )
    {
    // check if there are | or & operators
    for ( p = String(g.pSrch->textSpec); *p; p++ )
      {
      if ( *p == '^' )
        {
        p++;
        // OR operator
        }
      else if ( *p == '|' )
        {
        pds->textSpec = WordArrayNew(String(g.pSrch->textSpec),
                                     StringLen(g.pSrch->textSpec),
                                     '|', '^', TRUE);
        if ( !pds->textSpec ) return FALSE;
        g.pSrch->option |= OPT_BOOLOPOR;
        return TRUE;
        // AND operator
        }
      else if ( *p == '&' )
        {
        pds->textSpec = WordArrayNew(String(g.pSrch->textSpec),
                                     StringLen(g.pSrch->textSpec),
                                     '&', '^', TRUE);
        if ( !pds->textSpec ) return FALSE;
        // set the AND mask
        if ( pds->textSpec->ci > 32 ) pds->textSpec->ci = 32;
        cwords = pds->textSpec->ci;
        if ( cwords > 1 )
          {
          while ( cwords-- ) pds->ANDmask |= (1 << cwords);
          g.pSrch->option |= OPT_BOOLOPAND;
          } /* endif */
        return TRUE;
        } /* endif */
      } /* endfor */
    } /* endif */
  // single text (boolean operator option disabled or no boolean operator found)
  pds->textSpec = WordArrayNew(String(g.pSrch->textSpec),
                               StringLen(g.pSrch->textSpec),
                               0, 0, TRUE);
  if ( !pds->textSpec ) return FALSE;
  return TRUE;
  }


//===========================================================================
// Search the required files, open them and search their content.
// If any of the searched strings are found (OR or AND logic - according
// to the current settings) the data of the files and of the found
// text are stored.
// Parameters --------------------------------------------------------------
// PTXTWORD pwFileSpec : file specifications
// HWND hwnd           : notification window handle
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================
static
            BOOL scanFiles(PTXTWORD pwFileSpec, PDOSEARCH pds)
  {
  return 0 <=
              fmTreeScan(pwFileSpec->pw,
                         FILEATTR_ANY | ((g.setting & OPT_RECUR) ? SCANTREE_RECUR: 0),
                         pds, g.objWnd.hev,
                         (g.pSrch->textSpec? fnTextSearch: fnFileMatch),
                         NULL, NULL);
  }


//===========================================================================
// Callback procedure called for each file.
// - check if the file matches the optional search criteria (besides file
//   name pattern).
// - opens the file and read its content to buffer (this may require multiple
// - reads if the file size is bigger than CB_FILEBUFFER bytes.
// - search all the occurrences of the searched text string(s)
// - if any occurrences were found:
//   - store line number and text of all occurrences
//   - store the file name
// - in case of error store the name of the file which caused the error and
//   the error code.
// Parameters --------------------------------------------------------------
// PSCANTREE pst : tree scansion data.
// Return value ------------------------------------------------------------
// LONG : always return >= 0 as errors should not interrupt scanning.
//===========================================================================
static
            LONG fnTextSearch(PSCANTREE pst)
  {
  HFILE hf;
  ULONG ul;
  LONGLONG /*cb,*/ llcb, ulResources, ulTemp;            // 20090615 LFS
  LONGLONG cbPrev;
  PDOSEARCH pds = (PDOSEARCH)pst->pUserData;
  PPLLIST pFoundText, pnew;
  PSZ p, pLastLine;
  HNDPLLITEM hFileItem, hTextList;
  char buffer[128];       // only for TRACE

  ulResources = 0;
  // if the file length is 0 just return 0
  if ( pst->ff.cbFile == 0 ) return 0;     // 20090615 LFS
  llcb = pst->ff.cbFile;
  pds->rc = ERROR_NOT_ENOUGH_MEMORY;
  if ( NULL == (pFoundText = PLListNew(0)) )
    {
    TRACE("can't PLListNew");
    goto error;
    }
  ulResources |= TXTSRCH_TXTFOUNDLIST;
  // build the full file name
  memcpy(g.pSrchBuf->fileData, pst->achPath, pst->cbPath);
  memcpy(g.pSrchBuf->fileData + pst->cbPath,
         pst->ff.achName, pst->ff.cchName + 1);
  // show the current file name
  WinSetWindowText(pds->hwndProgress, g.pSrchBuf->fileData);
  // open the file for reading
  //TRACE1("searching %s", g.pSrchBuf->fileData);

  pds->rc = xDosOpen(g.pSrchBuf->fileData, &hf, &ul, llcb,
                     FILE_NORMAL,        // ulAttribute                                      // 20090615 LFS
                     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,                   // fsOpenFlags
                     OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,     // fsOpenMode
                     NULL);
  if ( pds->rc )
    {
    TRACE3("ERROR: 0x%X (%d), %s", pds->rc, pds->rc, g.pSrchBuf->fileData);
    goto error;
    }
  ulResources |= TXTSRCH_OPENFILE;
  pds->linecount = 0;
  pds->curMask = 0;
  pLastLine = NULL;
  cbPrev = 0;
  do
    {
    // if the user pressed Stop interrupt execution
    if ( ERROR_TIMEOUT == DosWaitEventSem(g.objWnd.hev, SEM_IMMEDIATE_RETURN) )
      break;
    // if a part of a line from the previous cicle is in the buffer
    // move it to the buffer start
    if ( pLastLine )
      memmove(g.pSrchBuf->readFile, pLastLine, cbPrev);

    // read as much of the file as it fit in the buffer
    ulTemp = CB_BUFFERREAD - cbPrev - 1;
    if ( llcb < ulTemp )
      {
      ulTemp = llcb;
      }
    pds->rc = DosRead(hf, g.pSrchBuf->readFile + cbPrev,          // 20090615 LFS
                      ulTemp, &ul);
    if ( pds->rc )
      {
      TRACE1("ERROR: 0x%X", pds->rc);
      goto error;
      }

    // if "text only" is set and any NUL char is found the search is interrupted
    if ( g.pSrch->option & OPT_TEXTONLY )
      {
      if ( memchr(g.pSrchBuf->readFile + cbPrev, 0, ul) )
        {
        TRACE("error");
        goto error;
        }
      // if this is a binary file replace NUL characters with spaces
      }
    else
      {
      replaceNuls(g.pSrchBuf->readFile + cbPrev, ul);
      } /* endif */
    // if there are no unread characters left, terminate the buffer
    llcb -= ul;
    sprintf (buffer, "left: %llu bytes", llcb);
    //TRACE2("%s (%llu)", buffer, llcb);            // 20090705 lld with pmprintf test !!
    if ( 0 == llcb )                              // 20090615 LFS
      {
      // is this correct ????
      g.pSrchBuf->readFile[ul + cbPrev] = 0;
      //         pLastLine = NULL;
      // otherwise find the last occurrence of '\r' or '\n', put a 0 to
      // signal the end of the line, make 'pLastLine' point to the beginning
      // of the next line
      }
    else
      {
      for ( pLastLine = g.pSrchBuf->readFile + ul + cbPrev;
          pLastLine > g.pSrchBuf->readFile;
          pLastLine-- )
        {
        if ( *pLastLine == '\n' )
          {
          if ( *(pLastLine - 1) == '\r' )
            {
            *(pLastLine - 1) = 0;
            }
          else
            {
            *pLastLine = 0;
            } /* endif */
          pLastLine++;
          break;
          } /* endif */
        if ( *pLastLine == '\r' )
          {
          *pLastLine = 0;
          pLastLine++;
          break;
          } /* endif */
        } /* endfor */
      // if the buffer doesn't contain neither '\r' nor '\n' consider
      // the file invalid (error code line length exceeding 5MB)
      if ( pLastLine == g.pSrchBuf->readFile )
        {
        pds->rc = ERROR_LINETOOLONG;
        TRACE1("ERROR: 0x%X", pds->rc);
        goto error;
        }
      else
        {
        cbPrev = (ul + cbPrev) - (LONGLONG)(pLastLine - g.pSrchBuf->readFile);
        } /* endif */
      } /* endif */

    // check if the buffer contains any of the searched strings
    pds->pLine = g.pSrchBuf->readFile;
    // multiple text strings search
    if ( pds->textSpec->ci > 1 )
      {
      // the text strings are separated by the OR boolean operator
      if ( g.pSrch->option & OPT_BOOLOPOR )
        {
        // the search is case insensitive
        if ( g.pSrch->option & OPT_CASEINS )
          {
          pFoundText = searchTextIOr(pFoundText, pds);
          // the search is case sensitive
          }
        else
          {
          pFoundText = searchTextOr(pFoundText, pds);
          } /* endif */
        // the text strings are separated by the AND boolean operator
        }
      else
        {
        // the search is case insensitive
        if ( g.pSrch->option & OPT_CASEINS )
          {
          pFoundText = searchTextIAnd(pFoundText, pds);
          // the search is case sensitive
          }
        else
          {
          pFoundText = searchTextAnd(pFoundText, pds);
          } /* endif */
        } /* endif */
      // single text string search
      }
    else
      {
      // the search is case insensitive
      if ( g.pSrch->option & OPT_CASEINS )
        {
        pFoundText = searchTextI(pFoundText, pds);
        // the search is case sensitive
        }
      else
        {
        pFoundText = searchText(pFoundText, pds);
        } /* endif */
      } /* endif */
    // the only possible next error condition can be due to failed allocation

    //pds->rc = ERROR_NOT_ENOUGH_MEMORY;  // 20090703 AB argh, Alessandro it seems you really like == instead of = :-)

    // 20090726 AB not true - !pFoundText can mean also, there's really no match !!! ToDo:
    // 20091217 maybe this was a memory problem too? Not observed anymore after repairing heap bugs
    if ( !pFoundText )
      {
      TRACE1("ERROR: 0x%X", pds->rc);
      goto error;
      }
    //TRACE1("found text %d times", pFoundText->count);
    } while ( llcb ); /* enddo */     // 20090615 LFS
  DosClose(hf);
  ulResources &= ~TXTSRCH_OPENFILE;
  // if any match was found add the current file to the list of found files
  if ( PLListRootCount(pFoundText) && (pds->ANDmask == pds->curMask) )
    {
    // the only possible next error condition can be due to failed allocation
    pds->rc = ERROR_NOT_ENOUGH_MEMORY; // 20090703 AB and again ==
    // add the current file data to the list of found files
    pnew = PLListNodeAddC(g.pSrch->result, 0, &hFileItem, addFileItem, pst);
    if ( !pnew )
      {
      TRACE("can't PLListNodeAddC");
      goto error;
      }
    g.pSrch->result = pnew;
    // add the list of found text lines as a child of the file data
    pnew = PLListListImport(g.pSrch->result, pFoundText, hFileItem);
    if ( !pnew )
      {
      TRACE("can't PLListListImport");
      goto error;
      }
    g.pSrch->result = pnew;
    pds->rc = NO_ERROR;

    // add the file name to the listbox
    printFileDetails(g.pSrchBuf->fileData,
                     PLLISTNODEDATA(PFOUNDFILE, g.pSrch->result, hFileItem));
    wLbxItemAndHndIns(pds->hwndLboxFile, g.sort,
                      g.pSrchBuf->fileData, hFileItem);
    TRACE2("found string in: %s, %s", g.pSrchBuf->fileData, pFoundText);
    } /* endif */
  PLListDel(pFoundText);
  return 1;

  error :

  if ( ulResources & TXTSRCH_OPENFILE ) DosClose(hf);
  if ( ulResources & TXTSRCH_TXTFOUNDLIST ) PLListDel(pFoundText);
  // critical error
  if ( pds->rc == ERROR_NOT_ENOUGH_MEMORY )
    {
    TRACE("ERROR_NOT_ENOUGH_MEMORY");
    g.errors = PLListItemAddC(g.errors, 0, NULL, errorInfoAdd, pds->rc);    // 20090702 AB added
    return -1;
    }
  else if ( pds->rc )
    {
    g.errors = PLListItemAddC(g.errors, 0, NULL, errorInfoAdd, pds->rc);
    }
  else
    {                                                                       // 20090702 AB added
    TRACE2("ERROR: 0x%X (%d)", pds->rc, pds->rc);
    g.errors = PLListItemAddC(g.errors, 0, NULL, errorInfoAdd, pds->rc);
    }/* endif */

  return 0;
  }


//===========================================================================
// Search-text loop (multiple ORed text strings, case sensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds   : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================
_Inline
            PPLLIST searchTextOr(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  INT i;
  while ( *p )
    {
    // return character
    if ( *p == '\r' )
      {
      if ( *++p == '\n' ) ++p;
      pds->pLine = p;
      pds->linecount++;
      // newline character
      }
    else if ( *p == '\n' )
      {
      pds->pLine = ++p;
      pds->linecount++;
      // other character:
      // check if any string matches the current part of the text
      }
    else
      {
      // loop through all the searched strings
      for ( i = 0; i < pds->textSpec->ci; ++i )
        {
        // if a matching string is found
        if ( (*p == pds->textSpec->aw[i].pw[0]) &&
             !memcmp(p + 1, &pds->textSpec->aw[i].pw[1],
                     pds->textSpec->aw[i].cb - 1) )
          {
          // get to the next line
          p = nextLine(p + pds->textSpec->aw[i].cb, &pds->pEndl);
          if ( !linkLine(&pFoundText, pds) ) return NULL;
          pds->pLine = p;
          pds->linecount++;
          // a string matched: no need to go on with the loop
          break;
          } /* endif */
        } /* endfor */
      // if no match was found must increase the char pointer
      if ( i == pds->textSpec->ci ) ++p;
      } /* endif */
    } /* endwhile */
  return pFoundText;
  }


//===========================================================================
// Search-text loop (multiple ORed text strings, case insensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds  : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================
_Inline
            PPLLIST searchTextIOr(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  INT i;
  while ( *p )
    {
    // return character
    if ( *p == '\r' )
      {
      if ( *++p == '\n' ) ++p;
      pds->pLine = p;
      pds->linecount++;
      // newline character
      }
    else if ( *p == '\n' )
      {
      pds->pLine = ++p;
      pds->linecount++;
      // other character:
      // check if any string matches the current part of the text
      }
    else
      {
      // loop through all the searched strings
      for ( i = 0; i < pds->textSpec->ci; ++i )
        {
        // if a matching string is found
        if ( !szCmpNI(p, pds->textSpec->aw[i].pw, pds->textSpec->aw[i].cb) )
          {
          // get to the next line
          p = nextLine(p + pds->textSpec->aw[i].cb, &pds->pEndl);
          // add the current line to the list of matching lines
          if ( !linkLine(&pFoundText, pds) ) return NULL;
          pds->pLine = p;
          pds->linecount++;
          // a string matched: no need to go on with the loop
          break;
          } /* endif */
        } /* endfor */
      // if no match was found must increase the char pointer
      if ( i == pds->textSpec->ci ) ++p;
      } /* endif */
    } /* endwhile */
  return pFoundText;
  }


//===========================================================================
// Search-text loop (multiple ANDed text strings, case sensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds   : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================

PPLLIST searchTextAnd(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  INT i;
  BOOL matched;

  for ( matched = FALSE;; )
    {

    loopStart:         // -------------------------------------------------------

    switch ( *p )
      {
      // end of text --------------------------------
      case 0 :
        pds->pEndl = p + 1;
        if ( matched ) goto addLine;
        return pFoundText;
        // end of line (return) -----------------------
      case '\r':
        *p = 0;
        pds->pEndl = ++p;
        if ( *p == '\n' ) ++p;    // skip next 'newline' if present
        if ( matched ) goto addLine;
        goto resetLine;
        // end of line (new line) ---------------------
      case '\n':
        *p = 0;
        pds->pEndl = ++p;
        if ( matched ) goto addLine;
        goto resetLine;
        // other characters ---------------------------
      default:

        // loop through the words to be searched
        for ( i = 0; i < pds->textSpec->ci; ++i )
          {
          // if the current word matches the searched text
          if ( (*p == pds->textSpec->aw[i].pw[0]) &&
               !memcmp(p + 1, &pds->textSpec->aw[i].pw[1],
                       pds->textSpec->aw[i].cb - 1) )
            {
            // update the mask of the matched words
            pds->curMask |= 1 << i;
            // if all the required strings have already been found in
            // the current file link the current line and get to the next
            if ( pds->ANDmask == pds->curMask )
              {
              p = nextLine(p + pds->textSpec->aw[i].cb, &pds->pEndl);
              goto addLine;
              } /* endif */
            // set the match flag
            matched = TRUE;
            // move the pointer past the matched word
            p += pds->textSpec->aw[i].cb;
            // go on checking the new character in p
            goto loopStart;
            } /* endif */
          } /* endfor */
        // no match found move to the next character
        ++p;
      } /* endswitch */
    } /* endfor */

  // --------------------------------------------------------------------------
  // add the current line to the list of lines containing matched text
  addLine:
  matched = FALSE;
  if ( !linkLine(&pFoundText, pds) ) return NULL;
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // reset the line pointer and the line counter
  resetLine:
  pds->pLine = p;
  pds->linecount++;
  if ( *p ) goto loopStart;
  // --------------------------------------------------------------------------

  return pFoundText;
  }


//===========================================================================
// Search-text loop (multiple ANDed text strings, case insensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds   : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================
_Inline
            PPLLIST searchTextIAnd(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  INT i;
  BOOL matched;

  for ( matched = FALSE;; )
    {

    loopStart:         // -------------------------------------------------------

    switch ( *p )
      {
      // end of text --------------------------------
      case 0 :
        pds->pEndl = p + 1;
        if ( matched ) goto addLine;
        return pFoundText;
        // end of line (return) -----------------------
      case '\r':
        *p = 0;
        pds->pEndl = ++p;
        if ( *p == '\n' ) ++p;    // skip next 'newline' if present
        if ( matched ) goto addLine;
        goto resetLine;
        // end of line (new line) ---------------------
      case '\n':
        *p = 0;
        pds->pEndl = ++p;
        if ( matched ) goto addLine;
        goto resetLine;
        // other characters ---------------------------
      default:

        // loop through the words to be searched
        for ( i = 0; i < pds->textSpec->ci; ++i )
          {
          // if the current word matches the searched text
          if ( !szCmpNI(p, pds->textSpec->aw[i].pw,
                        pds->textSpec->aw[i].cb) )
            {
            // update the mask of the matched words
            pds->curMask |= 1 << i;
            // if all the required strings have already been found in
            // the current file link the current line and get to the next
            if ( pds->ANDmask == pds->curMask )
              {
              p = nextLine(p + pds->textSpec->aw[i].cb, &pds->pEndl);
              goto addLine;
              } /* endif */
            // set the match flag
            matched = TRUE;
            // move the pointer past the matched word
            p += pds->textSpec->aw[i].cb;
            // go on checking the new character in p
            goto loopStart;
            } /* endif */
          } /* endfor */
        // no match found move to the next character
        ++p;
      } /* endswitch */
    } /* endfor */

  // --------------------------------------------------------------------------
  // add the current line to the list of lines containing matched text
  addLine:
  matched = FALSE;
  if ( !linkLine(&pFoundText, pds) ) return NULL;
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // reset the line pointer and the line counter
  resetLine:
  pds->pLine = p;
  pds->linecount++;
  if ( *p ) goto loopStart;
  // --------------------------------------------------------------------------

  return pFoundText;
  }


//===========================================================================
// Search-text loop (single text strings, case sensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds   : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================
_Inline
            PPLLIST searchText(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  while ( *p )
    {
    // return character
    if ( *p == '\r' )
      {
      if ( *++p == '\n' ) ++p;
      pds->pLine = p;
      pds->linecount++;
      // newline character
      }
    else if ( *p == '\n' )
      {
      pds->pLine = ++p;
      pds->linecount++;
      // other character:
      // check if any string matches the current part of the text
      }
    else
      {
      if ( (*p++ == pds->textSpec->aw[0].pw[0]) &&
           !memcmp(p, &pds->textSpec->aw[0].pw[1],
                   pds->textSpec->aw[0].cb - 1) )
        {
        p = nextLine(p + pds->textSpec->aw[0].cb - 1, &pds->pEndl);
        if ( !linkLine(&pFoundText, pds) ) return NULL;
        pds->pLine = p;
        pds->linecount++;
        } /* endif */
      } /* endif */
    } /* endwhile */
  return pFoundText;
  }


//===========================================================================
// Search-text loop (single text strings, case insensitive).
// Parameters --------------------------------------------------------------
// PPLLIST pFoundText : list of found lines.
// PDOSEARCH pds   : line data (line count, line length, line content).
// Return value ------------------------------------------------------------
// PPLLIST : packed linked list storing the found text strings.
//===========================================================================
_Inline
            PPLLIST searchTextI(PPLLIST pFoundText, PDOSEARCH pds)
  {
  PSZ p = pds->pLine;
  while ( *p )
    {
    // return character
    if ( *p == '\r' )
      {
      if ( *++p == '\n' ) ++p;
      pds->pLine = p;
      pds->linecount++;
      // newline character
      }
    else if ( *p == '\n' )
      {
      pds->pLine = ++p;
      pds->linecount++;
      // other character:
      // check if any string matches the current part of the text
      }
    else
      {
      if ( !szCmpNI(p, pds->textSpec->aw[0].pw, pds->textSpec->aw[0].cb) )
        {
        p = nextLine(p + pds->textSpec->aw[0].cb, &pds->pEndl);
        if ( !linkLine(&pFoundText, pds) ) return NULL;
        pds->pLine = p;
        pds->linecount++;
        }
      else
        {
        ++p;
        } /* endif */
      } /* endif */
    } /* endwhile */
  return pFoundText;
  }


//===========================================================================
// Add a line of text to the list of found lines.
// Parameters --------------------------------------------------------------
// PPLLIST* ppList : address of the list of found lines.
// PDOSEARCH pds   : search data.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================
_Inline
            BOOL linkLine(PPLLIST* ppList, PDOSEARCH pds)
  {
  PPLLIST pnew;
  // skip the leading spaces if required by the current options
  pds->pLine = stripLeadSpaces(pds->pLine);
  pds->cbLine = pds->pEndl - pds->pLine;
  // store the line in the list
  if ( NULL == (pnew = PLListItemAddC(*ppList, 0, NULL, textLineNew, pds)) )
    {
    PLListDel(*ppList);
    return FALSE;
    } /* endif */
  *ppList = pnew;
  return TRUE;
  }


//===========================================================================
// Callback procedure called for each file.
// - if the file matches the optional search criteria (besides file name
//    pattern) store the file data.
// Parameters --------------------------------------------------------------
// PSCANTREE pst : tree scansion data.
// Return value ------------------------------------------------------------
// LONG : always reurn 1 as errors should not interrupt scanning.
//===========================================================================
static
            LONG fnFileMatch(PSCANTREE pst)
  {
  ULONG cb;
  PDOSEARCH pds = (PDOSEARCH)pst->pUserData;
  PFOUNDFILE pff;
  PPLLIST pnew;
  HNDPLLITEM hItem;
  // add the file to the list of found files
  pnew = PLListItemAddC(g.pSrch->result, 0, &hItem, addFileItem, pst);
  if ( !pnew )
    {
    // 200808xx AB double == was probably not your intention Alessandro
    pds->rc = ERROR_NOT_ENOUGH_MEMORY;
    return -1;
    } /* endif */
  g.pSrch->result = pnew;
  pff = PLLISTITEMDATA(PFOUNDFILE, g.pSrch->result, hItem);
  // show the name of the file beeing processed
  WinSetWindowText(pds->hwndProgress, pff->name);
  // add the file name to the listbox
  printFileDetails(g.pSrchBuf->fileData, pff);
  //   wLbxItemIns(pds->hwndLboxFile, g.sort, g.pSrchBuf->fileData);
  wLbxItemAndHndIns(pds->hwndLboxFile, g.sort, g.pSrchBuf->fileData, hItem);
  //TRACE1("found %s", g.pSrchBuf->fileData);
  return 1;
  }


//===========================================================================
// Loops through a text string finding the first return or new line character.
// Strip trailing spaces substituting the first space with a NUL character.
// Return the beginning of the next line e the address of the line end.
// Note : this works only if when called from the searchText*() functions.
// Parameters --------------------------------------------------------------
// PSZ psz    : text string to be terminated at line end.
// PSZ ppendl : line end pointer
// Return value ------------------------------------------------------------
// PSZ : address of the first character of the next line.
//===========================================================================
_Inline
            PSZ nextLine(PSZ psz, PSZ* ppendl)
  {
  PSZ pend;
  for ( ;; )
    {
    if ( *psz == '\r' )
      {
      // strip trailing spaces
      pend = psz - 1;
      if ( *++psz == '\n' ) ++psz;
      break;
      } /* endif */
    if ( *psz == '\n' )
      {
      pend = psz - 1;
      ++psz;
      break;
      } /* endif */
    if ( !*psz )
      {
      pend = psz - 1;
      ++psz;
      break;
      } /* endif */
    ++psz;
    } /* endfor */
  while ( (*pend == ' ') || (*pend == '\t') ) --pend;
  *++pend = 0;
  *ppendl = pend + 1;
  return psz;
  }


//===========================================================================
// Strip the leading spaces and tabs from the string p.
// Parameters --------------------------------------------------------------
// PSZ p : text strings to be stripped.
// Return value ------------------------------------------------------------
// PSZ : address of the first non-space character.
//===========================================================================
_Inline
            PSZ stripLeadSpaces(PSZ p)
  {
  if ( g.setting & OPT_STRIPSPACES )
    while ( (*p == ' ') || (*p == '\t') ) ++p;
  return p;
  }


//===========================================================================
// Replace NUL characters with spaces to allow text search in binary files.
// Parameters --------------------------------------------------------------
// PSZ p        : buffer address.
// ULONG cbData : buffer size.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID replaceNuls(PSZ p, ULONG cbData)
  {
  PSZ pNul;
  while ( NULL != (pNul = memchr(p, 0, cbData)) )
    {
    cbData -= pNul - p;
    *pNul = ' ';
    p = pNul;
    } /* endwhile */
  }

/******************************************************************************
BOOL ExpandWithDriveLetters (PTXTWORD pTxtWord, PVOID pszDrives)

Historie:
---------
created:   AB 20091220
changed:

Description:
------------
Callback function for WordIterate. Expands TextWords with specified drives.
f.i. readme* ---> D:\readme* | E:\readme* | T:\readme*

Parameters:
-----------
pTxtWord... pointer to TXTWORD which should be expanded
pszDrives.. pointer to string which holds drive definition (D: E: T:...)

Returns:
--------
TRUE if succesfull, FALSE in case of error
******************************************************************************/
static BOOL ExpandWithDriveLetters(PTXTWORD pTxtWord, PVOID pszDrives)
  {
  PSZ pszBuf, pszP, pszCombined;
  int iLen, i, rc;

  TRACE3("%4d %s, %s", pTxtWord->cb, pTxtWord->pw, pszDrives);

  if ( pTxtWord->cb > LIST_BOX_WIDTH )
    {   // impossible long string, shorten it
    pTxtWord->cb = LIST_BOX_WIDTH;
    }

  // 20091228 AB fixed crash when drive is specified
  // if drives are specified it is needed to set pszCo... to the input string, otherwise it would not be valid near return statement
  pszCombined = pTxtWord->pw;

  // check if search string includes ':', if not, allocate string buffer 26x lenght
  // search string and compose entries for all selected drives
  if ( !memchr(pTxtWord->pw, (int) ':', pTxtWord->cb) )
    {    // no drive specified, allocate 26 times the search entry lenght + 3
    TRACE1("no drive specified (%s)", pTxtWord->pw);
    iLen = 26 * (6 + pTxtWord->cb);   // 'x:\... | ' are 6 chars to add
    pszBuf = malloc(iLen);
    if ( pszBuf )
      {
      TRACE1("allocated %d bytes", iLen);
      pszCombined = pszBuf;
      //WinQueryDlgItemText(g.hwndMainWin, PB_DRIVES, sizeof(szDrives), szDrives);
      if ( strchr(pszDrives, ':') )
        {   // search drives button include ':' so it's not default text anymore
        *pszBuf = '\0';
        pszP = pszDrives;
        while ( *pszP )
          {   // copy Drive letter and :
          if ( *pszP != ' ' )
            {
            *pszBuf = *pszP;
            pszBuf++;
            }
          else
            {   // append back slash
            *pszBuf = '\\';
            pszBuf++;
            // remove leading blanks
            i = 0;
            while ( (pTxtWord->pw[i] == ' ') && (i < pTxtWord->cb) ) i++;

            // remove leading backslash
            i = 0;
            while ( (pTxtWord->pw[i] == '\\') && (i < pTxtWord->cb) ) i++;

            if ( i == pTxtWord->cb )
              {   // empty file spec
              //pszBuff -= 3;             // remove drive letter\: and (is below) ToDo: test this
              break;                      // break
              }
            // append fileSpec
            while ( i < pTxtWord->cb )
              {
              *pszBuf = pTxtWord->pw[i];
              pszBuf++;
              i++;
              }
            // append ' | '
            *pszBuf = ' ';
            pszBuf++;
            *pszBuf = '|';
            pszBuf++;
            *pszBuf = ' ';
            pszBuf++;
            }
          // next char from PB_DRIVES
          pszP++;
          }
        pszBuf -= 3;
        *pszBuf = '\0';
        TRACE2("combined file search len: %d string: %s", strlen(pszCombined), pszCombined);
        }
      else
        { // no : in "files to search" string, and no drives in Drives dialog selected
        pszCombined = pTxtWord->pw;
        }
      }
    else
      {
      TRACE("ERROR - can't allocate space for 'multiple drive' search string");
      return FALSE;
      }

    }

  pTxtWord->pw = pszCombined;
  pTxtWord->cb = strlen(pszCombined);
  return TRUE;    // FALSE ends interation
  }


