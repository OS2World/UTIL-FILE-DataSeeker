/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// prodinfo.c :
// -
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"
#include "build.h"

// definitions --------------------------------------------------------------


// prototypes ---------------------------------------------------------------
VOID initDlg(HWND hwnd);
VOID startBrowser(HWND hwnd);


// globals ------------------------------------------------------------------

char DateTime[] = { "BUILD_DATE_TIME "__DATE__" "__TIME__};    // entry in object file which is used for bldlevel information
char MONTHS[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

MRESULT EXPENTRY aboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  switch ( msg )
    {
    case WM_INITDLG:
      initDlg(hwnd);
      break;
      //        case WM_COMMAND:
      // 20080xxx AB we have no URL anymore
      /*         if ((ULONG)mp1 == PB_URL)
                      {
                      startBrowser(hwnd);
                      break;
                      }
      */
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

VOID initDlg(HWND hwnd)
  {
  SWP swp;
  HWND hCtrl;
  char cBuf[512];
  char cBuf1[20];
  char cBuf2[20];
  char cBuf3[20];
  char sMonth[10];
  int i, iDay, iMonth, iYear;
  MRESULT mres = 0;

  hCtrl = DlgItemHwnd(hwnd, DLG_ABOUT);
  WinQueryWindowPos(hwnd, &swp);
  TRACE2("position x: %d  y: %d", swp.x, swp.y);
  WinSetWindowPos(hwnd, 0, swp.x - 225, swp.y - 10, 0, 0, SWP_MOVE);
  hCtrl = DlgItemHwnd(hwnd, BAR_DARK);
  WinQueryWindowPos(hCtrl, &swp);
  WinSetWindowPos(hCtrl, 0, swp.x, swp.y - swp.cy + 1, swp.cx, 1,
                  SWP_SIZE | SWP_MOVE);
  hCtrl = DlgItemHwnd(hwnd, BAR_LITE);
  WinQueryWindowPos(hCtrl, &swp);
  WinSetWindowPos(hCtrl, 0, swp.x + 1, swp.y + swp.cy - 2, swp.cx, 1,
                  SWP_SIZE | SWP_MOVE);

  // 20080xxx AB add build information in About Dialog (BUILD DATE TIME)
  NlsGet("Version", cBuf1);
  NlsGet("Build", cBuf2);
  //strcpy(cBuf3, __DATE__);
  sscanf(__DATE__, "%s %d %d", &sMonth, &iDay, &iYear);
  TRACE3("sMonth=%s, iDay=%d, iYear=%d", sMonth, iDay, iYear);
  for ( i = 0; i < 12; i++ )
    {
    if ( strstr(sMonth, MONTHS[i]) )
      {
      iMonth = i + 1;
      break;
      }
    }
  strcpy(cBuf3, __TIME__);
  cBuf3[5]='\0';  // delete seconds
#ifdef __DEBUG_TRACE__
  sprintf(cBuf, "%s:  " DATASEEKER_VERSION "    %s:   %04d-%02d-%02d   %s     Build:  %04d    DEBUG", cBuf1, cBuf2, iYear, iMonth, iDay, cBuf3, VER_BUILD);
#else
  sprintf(cBuf, "%s:  " DATASEEKER_VERSION "    %s:   %04d-%02d-%02d   %s     Build:  %04d", cBuf1, cBuf2, iYear, iMonth, iDay, cBuf3, VER_BUILD);
#endif  // __DEBUG_TRACE__
  WinSetDlgItemText(hwnd, ST_VERSION, cBuf);

  // Translator info box (MLE) line 1 is author, version and date
  NlsGet("ABOUT_INFO_LINE1", cBuf);
  // check if valid string is found
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) strcpy(cBuf, "DataSeeker is hosted at netlabs - http://dataseeker.netlabs.org/");
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  // lines 2 till 6
  NlsGet("ABOUT_INFO_LINE2", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  NlsGet("ABOUT_INFO_LINE3", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) strcpy(cBuf, "Language translation library not working. Check your system for ecolange.dll. For National Language Support eCo Software runtime (base) is required - http://ecomstation.ru/ecosoft/runtime.php\n\n\nDataSeek.nls have to be in the same directory as DataSeek.exe (or the working directory setting of the DataSeek.exe object).");
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  NlsGet("ABOUT_INFO_LINE4", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  NlsGet("ABOUT_INFO_LINE5", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  NlsGet("ABOUT_INFO_LINE6", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_SETFIRSTCHAR, (MPARAM) 0, (MPARAM) 0);
  NlsGet("ABOUT_INFO_LINE7", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_SETFIRSTCHAR, (MPARAM) 0, (MPARAM) 0);
  NlsGet("ABOUT_INFO_LINE8", cBuf);
  if ( strstr(cBuf, "ABOUT_INFO_LINE") ) cBuf[0] = '\0';
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, "\n", (MPARAM) 0);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_INSERT, cBuf, (MPARAM) 0);
  TRACE1("mres=%d", mres);
  mres=WinSendMsg(WinWindowFromID(hwnd, MLE_TRANSLATOR_INFO), MLM_SETFIRSTCHAR, (MPARAM) 0, (MPARAM) 0);

  NlsGet("Close", cBuf);
  WinSetDlgItemText(hwnd, DID_OK, cBuf);
  }



//===========================================================================
// --- not needed anymore
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID startBrowser(HWND hwnd)
  {
  CHAR pszExe[CCHMAXPATH];
  CHAR pszPath[CCHMAXPATHCOMP];
  if ( PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                             "DefaultBrowserExe", "", pszExe, sizeof(pszExe))
       &&
       PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                             "DefaultWorkingDir", "", pszPath, sizeof(pszPath)) )
    applStartApp(hwnd, pszExe, "http://space.tin.it/scienza/acantato", pszPath, NULL, APPSTART_DEFAULT);
  }


