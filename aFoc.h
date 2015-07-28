/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

/******************************************************************************
 aFoc.h - header file for Andis wrapper around eCo Software FOC container

 History:
 --------
 20111121 AB initial

******************************************************************************/

// --- Includes ---------------------------------------------------------------

// --- Defines ----------------------------------------------------------------

// --- Typedefs ---------------------------------------------------------------

// --- PUBLIC / EXTERNAL Variables --------------------------------------------

// --- Variables --------------------------------------------------------------

// --- Macros -----------------------------------------------------------------

// --- Function prototypes ----------------------------------------------------
MRESULT EXPENTRY FocFailDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

int     FocInit ( void );
int     FocIsAvail(void);
int     FocGetSearchList(HWND hwnd);


// eCoFoc function wrappers
/*BOOL    xFocInitialize        ( void );
HWND    xFocFileDlg           ( HWND hwndP, HWND hwndO, PFOCFILEDLG pfocd );
MRESULT xFocDefFileDlgProc    ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL    xFocFreeFileDlgList   ( PAPSZ papszFQFilename );
HWND    xFocSelectDir         ( HWND hwndP, HWND hwndO, PFOCSELDIR pfocd );
MRESULT xFocDefSelectDirProc  ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
PSZ     xFocPathRelToAbs      ( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
PSZ     xFocPathAbsToRel      ( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
APIRET  xFocDeleteDirTree     ( PCSZ pszDirName, PSZ pszFailName, ULONG cbFailName );
APIRET  xFocLoadFileInfo      ( PCSZ pszFileName, PFOCFILEINFO pInfo, ULONG );
BOOL    xFocFreeFileInfo      ( PFOCFILEINFO pInfo );
  */
// --- Code -------------------------------------------------------------------

