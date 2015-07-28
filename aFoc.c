/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

/******************************************************************************
 aFoc.c - File Open Container wrapper for ecort FOC.DLL

 Uses (eco)FOC if FOC.DLL is detected on the system. Otherwise the file/dir
   open button does nothing

 Requirements:
 eCo Software runtime (base, win) -- http://ecomstation.ru/ecosoft/runtime.php

 History:
 --------
 20111121 AB initial

******************************************************************************/

// --- Includes ---------------------------------------------------------------
#include "pmseek.h"

// --- Defines ----------------------------------------------------------------

// --- Typedefs ---------------------------------------------------------------

// --- PUBLIC / EXTERNAL Variables --------------------------------------------

// --- Variables --------------------------------------------------------------
static int iHaveFoc = FALSE;

static BOOL    (* EXPENTRY xFocInitialize)      ( void );
static HWND    (* EXPENTRY xFocFileDlg)         ( HWND hwndP, HWND hwndO, PFOCFILEDLG pfocd );
static MRESULT (* EXPENTRY xFocDefFileDlgProc)  ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
static BOOL    (* EXPENTRY xFocFreeFileDlgList) ( PAPSZ papszFQFilename );
static HWND    (* EXPENTRY xFocSelectDir)       ( HWND hwndP, HWND hwndO, PFOCSELDIR pfocd );
static MRESULT (* EXPENTRY xFocDefSelectDirProc)( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
static PSZ     (* EXPENTRY xFocPathRelToAbs)    ( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
static PSZ     (* EXPENTRY xFocPathAbsToRel)    ( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
static APIRET  (* EXPENTRY xFocDeleteDirTree)   ( PCSZ pszDirName, PSZ pszFailName, ULONG cbFailName );
static APIRET  (* EXPENTRY xFocLoadFileInfo)    ( PCSZ pszFileName, PFOCFILEINFO pInfo, ULONG );
static BOOL    (* EXPENTRY xFocFreeFileInfo)    ( PFOCFILEINFO pInfo );

static APIRET  (* EXPENTRY xeLoadFileInfo)      ( PCSZ pszFileName, PFOCFILEINFO pInfo, LONG lSize, ULONG );
static BOOL    (* EXPENTRY xeFreeFileInfo)      ( PFOCFILEINFO pInfo );

// --- Macros -----------------------------------------------------------------

// --- Function prototypes ----------------------------------------------------
VOID FocFailDlgInit(HWND hwnd);

// --- Code -------------------------------------------------------------------
/******************************************************************************
int FocInit(void)

Initializes File Open Container. If FOC.DLL is found the FOC function
  forwarders are initialized.

Returns: 0 if FOC.DLL successfully loaded and function entry points are found
******************************************************************************/
int FocInit()
    {
    HMODULE hEcoFoc = NULLHANDLE;         /* Module handle                     */
    APIRET rc;
    char cBuf2[256];
    UCHAR    LoadError[256] = "";           /* Area for Load failure information */

    rc = DosLoadModule(LoadError,                   /* Failure information buffer */
                          sizeof(LoadError),        /* Size of buffer             */
                          "FOC",                    /* Module to load             */
                          &hEcoFoc);                /* Module handle returned     */

    if ( (rc = DosQueryModuleHandle("FOC", &hEcoFoc) ) != NO_ERROR )
        {
        TRACE("no FOC.DLL !!!!!!!");
        iHaveFoc = FALSE;
        }
    else
        {
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCInitialize",              // ProcName ASCII or NULL
                              (PFN *)&xFocInitialize);      // Address returned
        TRACE2("rc=%d, xFocInitialize at      0x%X", rc, xFocInitialize );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCFileDlg",                 // ProcName ASCII or NULL
                              (PFN *)&xFocFileDlg);         // Address returned
        //TRACE2("rc=%d, xFocFileDlg at         0x%X", rc, xFocFileDlg );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCDefFileDlgProc",          // ProcName ASCII or NULL
                              (PFN *)&xFocDefFileDlgProc);  // Address returned
        //TRACE2("rc=%d, xFocDefFileDlgProc at  0x%X", rc, xFocDefFileDlgProc );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCFreeFileDlgList",         // ProcName ASCII or NULL
                              (PFN *)&xFocFreeFileDlgList); // Address returned
        //TRACE2("rc=%d, xFocFreeFileDlgList at 0x%X", rc, xFocFreeFileDlgList );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCSelectDir",               // ProcName ASCII or NULL
                              (PFN *)&xFocSelectDir);       // Address returned
        //TRACE2("rc=%d, xFocSelectDir at       0x%X", rc, xFocSelectDir );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCDefSelectDirProc",        // ProcName ASCII or NULL
                              (PFN *)&xFocDefSelectDirProc);// Address returned
        //TRACE2("rc=%d, xFocDefSelectDirProc   0x%X", rc, xFocDefSelectDirProc );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCPathRelToAbs",            // ProcName ASCII or NULL
                              (PFN *)&xFocPathRelToAbs);    // Address returned
        //TRACE2("rc=%d, xFocPathRelToAbs at    0x%X", rc, xFocPathRelToAbs );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCPathAbsToRel",            // ProcName ASCII or NULL
                              (PFN *)&xFocPathAbsToRel);    // Address returned
        //TRACE2("rc=%d, xFocPathAbsToRel at    0x%X", rc, xFocPathAbsToRel );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCDeleteDirTree",           // ProcName ASCII or NULL
                              (PFN *)&xFocDeleteDirTree);   // Address returned
        //TRACE2("rc=%d, xFocDeleteDirTree at   0x%X", rc, xFocDeleteDirTree );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCLoadFileInfo",            // ProcName ASCII or NULL
                              (PFN *)&xFocLoadFileInfo);    // Address returned
        //TRACE2("rc=%d, xFocLoadFileInfo at    0x%X", rc, xFocLoadFileInfo );
        rc |= DosQueryProcAddr(hEcoFoc,                     // Handle to module
                              0,                            // Ordinal of ProcName specified or NULL
                              "FOCFreeFileInfo",            // ProcName ASCII or NULL
                              (PFN *)&xFocFreeFileInfo);    // Address returned
        //TRACE2("rc=%d, xFocFreeFileInfo at    0x%X", rc, xFocFreeFileInfo );

        if ( !rc )
            {
            iHaveFoc = TRUE;

            rc |= !(xFocInitialize());
            //TRACE1("xFocInitialize returned %s", rc == NO_ERROR ? "OK" : "ERROR");
            }
        }

    if (rc) TRACE("--------------------> FocInit ERROR initializing foc.dll <--------------------");
    return rc;
    }


/******************************************************************************
int FocIsAvail(void);

Return TRUE if FOC.DLL successfully loaded
******************************************************************************/
int FocIsAvail(void)
  {
  return iHaveFoc;
  }

/******************************************************************************
int FocFailDlgProc(void);

Dialog Procedure for Foc Fail dialog (no FOC.DLL)

Returns: usually what WinDefDlgProc returns
******************************************************************************/
MRESULT EXPENTRY FocFailDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch ( msg )
        {
        case WM_INITDLG:
            FocFailDlgInit(hwnd);
            return MRTRUE;
            break;
        default:
            return WinDefDlgProc(hwnd, msg, mp1, mp2);
            break;
        } /* endswitch */
    return MRFALSE;
    }

/******************************************************************************
VOID FocFailDlgInit(HWND hwnd)

Init Foc Fail dialog (no FOC.DLL)

Returns
******************************************************************************/
VOID FocFailDlgInit(HWND hwnd)
    {
    SWP swp;
    HWND hCtrl;
    char cBuf[512];
    MRESULT mres = 0;
    int i;
    ERRORID  erridErrorCode;

    hCtrl = DlgItemHwnd(hwnd, DLG_FOC_FAIL);
    WinQueryWindowPos(hwnd, &swp);
    TRACE2("position x: %d  y: %d", swp.x, swp.y);
    WinSetWindowPos(hwnd, 0, swp.x - 325, swp.y - 10, 0, 0, SWP_MOVE);

    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_SETSEL, (MPARAM) 0, (MPARAM) -1);
    TRACE1("mres=%d (should be 1)", mres);

    NlsGet("FOC.DLL not found", cBuf);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, cBuf, (MPARAM) 0);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, "\n", (MPARAM) 0);
    NlsGet("FOC_FAIL_2", cBuf);
    if (strncpy(cBuf, ".", 2) == 0) cBuf[0] = ' ';  // clear buffer if no translation available
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, cBuf, (MPARAM) 0);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, "\n", (MPARAM) 0);
    NlsGet("FOC_FAIL_3", cBuf);
    if (strncpy(cBuf, ".", 2) == 0) cBuf[0] = ' ';  // clear buffer if no translation available
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, cBuf, (MPARAM) 0);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, "\n", (MPARAM) 0);
    NlsGet("FOC_FAIL_4", cBuf);
    if (strncpy(cBuf, ".", 2) == 0) cBuf[0] = ' ';  // clear buffer if no translation available
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, cBuf, (MPARAM) 0);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, "\n", (MPARAM) 0);
    NlsGet("FOC_FAIL_5", cBuf);
    if (strncpy(cBuf, ".", 2) == 0) cBuf[0] = ' ';  // clear buffer if no translation available
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, cBuf, (MPARAM) 0);
    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_INSERT, "\n", (MPARAM) 0);

    mres=WinSendMsg(WinWindowFromID(hwnd, MLE_FOC_FAIL), MLM_SETFIRSTCHAR, (MPARAM) 0, (MPARAM) 0);
    TRACE1("mres=%d", mres);

    // get last nonzero error for this anchor block
    erridErrorCode = WinGetLastError(WinQueryAnchorBlock(hwnd));
    TRACE1("WinGetLastError LastError:0x%X",  erridErrorCode);

    NlsGet("Close", cBuf);
    WinSetDlgItemText(hwnd, DID_OK, cBuf);
    }


/******************************************************************************
int FocGetSearchList(HWND hwnd)

Opens FOC and places selected file(s) into DD_FILESRCH

Returns number of selected items
******************************************************************************/
int FocGetSearchList(HWND hwnd)
  {
  HWND hDlg;
  int i;
  char cBuf[LIST_BOX_WIDTH];
  char cTitle[80];
  char cOkay[32];
  char cCancel[32];
  SIZEL screen;
  FOCFILEDLG focdlg = { sizeof( FOCFILEDLG )};

  focdlg.fl = FDS_OPEN_DIALOG | FDS_MULTIPLESEL;
  focdlg.pszIType = "";
  NlsGet("Select files or directories", cTitle);    // FOC window title
  focdlg.pszTitle = cTitle;
  NlsGet("OK", cOkay);                              // Okay button
  focdlg.pszOKButton = cOkay;
  NlsGet("Cancel", cCancel);                        // Cancel button
  //focdlg.pszXXX = cCancel;                        // 20111226 AB ToDo: currently not supported by FOC
  strncpy(focdlg.szFullFile, g.cFocLastDir, sizeof( focdlg.szFullFile ));
  focdlg.szFullFile[sizeof(focdlg.szFullFile) - 3] = '\0';
  if ( focdlg.szFullFile[strlen(focdlg.szFullFile) - 1] != '.' ) strcat( focdlg.szFullFile, "\\." );

  // check if window is outside visible space
  screen.cx = WinSysVal(SV_CXSCREEN);
  screen.cy = WinSysVal(SV_CYSCREEN);
  if ( (g.iFocX > screen.cx - 200) || (g.iFocY > screen.cy - 250) || (g.iFocX < -20) || (g.iFocY < - 20) )
    { // set into visible area
    g.iFocX = 20;
    g.iFocY = 20;
    }
  focdlg.x = g.iFocX;     // restore position
  focdlg.y = g.iFocY;
  hDlg = xFocFileDlg( HWND_DESKTOP, hwnd, &focdlg );
  TRACE2( "xFocFileDlg return %08X, lReturn is %d", hDlg, focdlg.lReturn );
  g.iFocX = focdlg.x;
  g.iFocY = focdlg.y;
  //TRACE2("focdlg.x=%d, focdlg.y=%d", focdlg.x, focdlg.y);
  strncpy(g.cFocLastDir, focdlg.szFullFile, sizeof( g.cFocLastDir ));
  g.cFocLastDir[sizeof(g.cFocLastDir) - 1] = '\0';

  if( focdlg.lReturn == DID_OK )
    {
    TRACE1( "FOCFileDlg return szFullFile: %s", focdlg.szFullFile );
    strncpy(cBuf, focdlg.szFullFile , sizeof(cBuf));
    cBuf[sizeof(cBuf) - 3] = '\0';    // save some space for \* if needed below

    if( focdlg.papszFQFilename )
      {
      TRACE2( "FOCFileDlg return papszFQFilename[%02d]: %s", 0, (*focdlg.papszFQFilename)[0] );
      for( i = 1; i < focdlg.ulFQFCount; i++ )
        {
        TRACE2( "FOCFileDlg return papszFQFilename[%02d]: %s", i, (*focdlg.papszFQFilename)[i] );
        if (strlen((*focdlg.papszFQFilename)[i]) + strlen(cBuf) < (sizeof(cBuf) - 5) )
          {
          strcat(cBuf, " | ");
          strcat(cBuf, (*focdlg.papszFQFilename)[i]);
          }
        }
      }
    else
      {   // returned a directory, add '\*' or '*' for root
      if (cBuf[strlen(cBuf) - 1] == '\\') strcat(cBuf, "*");    // root append '*' only
      else strcat(cBuf, "\\*");
      }

    if (strlen(cBuf) ) WinSetDlgItemText(hwnd, DD_FILESRCH, cBuf);

    if( focdlg.papszFQFilename )
      {
      xFocFreeFileDlgList( focdlg.papszFQFilename );
      }
    }

  return focdlg.ulFQFCount;
  }


