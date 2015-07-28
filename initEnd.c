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
BOOL parseStartParms(INT argc, CHAR** argv);
BOOL loadProfile(VOID);
BOOL initHeap(VOID);
VOID setInterfaceFromIniData(HWND hwnd, PPMSEEKINI p);
LONG presParam(HWND hwnd, ULONG id, ULONG pp);


// globals ------------------------------------------------------------------


//===========================================================================
// Initialize the application.
// Parameters --------------------------------------------------------------
// INT argc     : count of startup parameters.
// CHAR ** argv : array of startup parameters.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL initApplication(int argc, char** argv)
  {
  ULONG rc;
  // allocate the heap used for searching files and text

#ifdef ALTERNATE_MEM_MANAGMENT      // 20091202
  if ( !initHeap() ) return FALSE;
#endif  // ALTERNATE_MEM_MANAGMENT

  // start the object window thread
  if ( !startObjectThread(&g.objWnd ,0x4000) )
    return endApplication(), FALSE;
  // get the text to be used for date, time and thousand separators
  PrfQueryProfileString(HINI_USER, "PM_National", "sDate", "/", g.nls.date, 4);
  PrfQueryProfileString(HINI_USER, "PM_National", "sTime", ":", g.nls.time, 4);
  PrfQueryProfileString(HINI_USER, "PM_National", "sThousand", ",",
                        g.nls.thousand, 4);
  // parse the start parameters (invalid parameters are not considered a
  // critical error: the application is started after displaying an error msg
  if ( !parseStartParms(argc, argv) )
    {
    TRACE("start parameters not correct");
    printApplicationError(NULLHANDLE, ERROR_INVPARMS, NULL);
    }
  // load the application profile (errors are not considered critical)
  loadProfile();
  // if required by the input parameters load a file
  if ( g.startData.loadFile )
    {
    TRACE("loading settings from file");
    if ( !loadFile(NULLHANDLE, String(g.startData.loadFile)) )
      {
      printApplicationError(NULLHANDLE, ERROR_LOADFILE,
                            String(g.startData.loadFile));
      // if loaded a history of searched files/text-strings delete the list
      // loaded via the initialization file
      }
    else if ( !g.loadData->isResult && g.startData.iniData )
      {
      PLListDel(g.startData.iniData->files);
      g.startData.iniData->files = NULL;
      PLListDel(g.startData.iniData->texts);
      g.startData.iniData->texts = NULL;
      } /* endif */
    StringDel(&g.startData.loadFile);
    } /* endif */
  g.sort = LIT_END;
  g.iHistSort = OPT_DONOTSORTHISTORY;  // 2008090x AB default do not sort histroy drop down boxes
  initUpperCaseTable();
  return TRUE;
  }


//===========================================================================
// Parse the startup parameters.
// Invalid parameters are not critical! After a warning message is displayed
// the program start normally
// Parameters --------------------------------------------------------------
// INT argc     : count of startup parameters.
// CHAR ** argv : array of startup parameters.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (valid/invalid parameters)
//===========================================================================
static
            BOOL parseStartParms(INT argc, CHAR** argv)
  {
  INT i;
  BOOL rc = TRUE;
  FILESTATUS3L fs;     // 20090615 LFS
  for ( i = 1; i < argc; ++i )
    {
    switch ( argv[i][0] )
      {
      // option
      case '-':
      case '/':
        // load file
        if ( (argv[i][1] == 'l') || (argv[i][1] == 'L') )
          {
          if ( g.startData.loadFile )
            {
            rc = FALSE;
            }
          else if ( argv[i][2] == ':' )
            {
            g.startData.loadFile = StringNew(&argv[i][3], -1);
            }
          else
            {
            g.startData.loadFile = StringNew(&argv[i][2], -1);
            } /* endif */
          // use an alternative INI file
          }
        else if ( (argv[i][1] == 'p') || (argv[i][1] == 'P') )
          {
          if ( g.startData.iniFile )
            {
            rc = FALSE;
            }
          else if ( argv[i][2] == ':' )
            {
            g.startData.iniFile = StringNew(&argv[i][3], -1);
            }
          else
            {
            g.startData.iniFile = StringNew(&argv[i][2], -1);
            } /* endif */
          }
        else if ( (argv[i][1] == 'i') || (argv[i][1] == 'I') )
          {
          g.startData.ignorePrfErrors = TRUE;
          }
        else
          {
          rc = FALSE;
          } /* endif */
        break;
      default:
        if ( !g.startData.fileToSearch )
          {
          g.startData.fileToSearch = StringNew(argv[i], -1);
          // if the file to search is a directory append "\*"
          if ( !xDosQueryPathInfo(argv[i], FIL_STANDARDL, &fs, sizeof(fs)) )
            {   // 20090615 LFS
            if ( fs.attrFile & FILE_DIRECTORY )
              g.startData.fileToSearch =
                          StringAppend(g.startData.fileToSearch, "\\*", 2);
            } /* endif */
          }
        else if ( !g.startData.textToSearch )
          {
          g.startData.textToSearch = StringNew(argv[i], -1);
          }
        else
          {
          rc = FALSE;
          } /* endif */
        break;
      } /* endswitch */
    } /* endfor */
  return rc;
  }


//===========================================================================
// Read the program initialization file.
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================
static
            BOOL loadProfile(VOID)
  {
  HFILE hf;
  PPLLIST plist;
  char *pszCopy;
  SIZEL screen;

  // defaults:
  TRACE1("I'm here ;-), try float %f", 12.3456);
  g.setting = g.setting = OPT_STRIPSPACES;
  g.maxHistory = 10;
  if ( !g.startData.iniFile && !(g.startData.iniFile = applIniFileName(FALSE)) )
    goto error0;
  // if the initialization file exists
  if ( fmExists(String(g.startData.iniFile)) )
    {
    // open the file
    TRACE1("reading ini file: %s", String(g.startData.iniFile));
    hf = fmOpen(String(g.startData.iniFile),
                FMO_IFEXIST | FMO_SHAREREAD | FMO_READONLY);
    if ( hf == FMO_ERROR ) goto error1;
    // allocate memory for reading the file start
    if ( !(g.startData.iniData = (PPMSEEKINI)malloc(sizeof(PMSEEKINI))) )
      goto error2;
    // read the file start
    if ( !fmRead(hf, g.startData.iniData, sizeof(PMSEEKINI)) ) goto error3;
    // if the searched file history was saved
    if ( g.startData.iniData->cbFiles )
      {
      // allocate memory for reading the history of searched files
      if ( !(plist = PLListDup(NULL, g.startData.iniData->cbFiles)) )
        goto error3;
      // read the history of searched files/strings
      if ( !fmRead(hf, plist, g.startData.iniData->cbFiles) ) goto error4;
      g.startData.iniData->files = plist;
      } /* endif */
    // if the searched text strings history was saved
    if ( g.startData.iniData->cbTexts )
      {
      // allocate memory for reading the history of searched files
      if ( !(plist = PLListDup(NULL, g.startData.iniData->cbTexts)) )
        goto error5;
      // read the history of searched files/strings
      if ( !fmRead(hf, plist, g.startData.iniData->cbTexts) ) goto error6;
      g.startData.iniData->texts = plist;
      } /* endif */
    fmClose(hf);
    g.setting = g.setting = g.startData.iniData->option;
    TRACE1("g.setting=0x%X", g.setting);
    g.maxHistory = g.startData.iniData->maxHistory;
    g.editor = StringNew(g.startData.iniData->achEditor, -1);
    // 200808xx AB if no editor in .ini, set default
    if ( !strlen((const unsigned char*) &g.startData.iniData->achEditor) )
      {
      TRACE("non valid string in ini file for 'editor', setting to default");
      StringSet(g.editor, "e.exe", -1);
      }
    g.cmdshell = StringNew(g.startData.iniData->achCmdShell, -1);
    // 200808xx AB if no cmd string in .ini, set default
    if ( !strlen((const unsigned char*) &g.startData.iniData->achCmdShell) )
      {
      TRACE("non valid string in ini file for 'CmdShell', setting to default");
      StringSet(g.cmdshell, "cmd.exe", -1);
      }

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

    // 20111226 AB check for reasonable values of FOC x, y
    screen.cx = WinSysVal(SV_CXSCREEN);
    screen.cy = WinSysVal(SV_CYSCREEN);
    g.iFocX = g.startData.iniData->iFocX;
    g.iFocY = g.startData.iniData->iFocY;
    if ( (g.iFocX > screen.cx - 200) || (g.iFocY > screen.cy - 250) || (g.iFocX < -20) || (g.iFocY < - 20) )
      { // set into visible area
      g.iFocX = 20;
      g.iFocY = 20;
      }
    strncpy (g.cFocLastDir, g.startData.iniData->cFocLastDir, sizeof(g.cFocLastDir) );
    g.cFocLastDir[sizeof(g.cFocLastDir) - 1] = '\0';
    }
  else    // set standard settings if no .ini file is found
    {
    // 200808xx AB set default search options if no .ini file
    TRACE("no ini file found! Setting default values");
    // g.state = IS_EDITOREPM;      // 20080906 AB default editor is EPM
    g.setting = g.setting = OPT_CASEINS | OPT_SHOWFDATE | OPT_SHOWFTIME | OPT_SHOWFSIZE | OPT_SHOWLINENO | OPT_STRIPSPACES | OPT_BOOLOP | OPT_RECUR | OPT_FIND_DIR;
    g.maxHistory = 15;
    StringSet(g.editor, "e.exe", -1);
    StringSet(g.cmdshell, "cmd.exe", -1);
    g.iFocX = 20;
    g.iFocY = 20;
    _getcwd( g.cFocLastDir, sizeof( g.cFocLastDir ));
    }
  return TRUE;
  error6:
  TRACE("error6");
  PLListDel(plist);
  plist = NULL;
  error5:
  TRACE("error5");
  PLListDel(g.startData.iniData->files);
  g.startData.iniData->files = NULL;
  error4:
  TRACE("error4");
  PLListDel(plist);
  error3:
  TRACE("error3");
  free(g.startData.iniData);
  g.startData.iniData = NULL;
  error2:
  TRACE("error2");
  fmClose(hf);
  error1:
  TRACE("error1");
  if ( !g.startData.ignorePrfErrors )
    printApplicationError(0, ERROR_LOADPROFILE, String(g.startData.iniFile));
  return FALSE;
  error0:
  TRACE("error0");
  if ( !g.startData.ignorePrfErrors )
    printApplicationError(NULLHANDLE, ERROR_LOADPROFILE, NULL);
  return FALSE;
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

VOID endApplication(VOID)
  {
  INT i;
  // free the start data (if any of them are still allocated)
  StringDel(&g.startData.iniFile);
  StringDel(&g.startData.loadFile);
  StringDel(&g.startData.fileToSearch);
  StringDel(&g.startData.textToSearch);
  // free the ini data (if any of them are still allocated)
  if ( g.startData.iniData )
    {
    PLListDel(g.startData.iniData->files);
    PLListDel(g.startData.iniData->texts);
    free(g.startData.iniData);
    } /* endif */
  // in case the program was closed while loading a file
  if ( g.loadData )
    {
    if ( g.loadData->isResult )
      {
      for ( i = 0; i < g.loadData->count; ++i )
        srchResDel(g.loadData->ares[i]);
      }
    else
      {
      PLListDel(g.loadData->files);
      PLListDel(g.loadData->texts);
      } /* endif */
    } /* endif */
  // free other STRING objects
  StringDel(&g.editor);
  StringDel(&g.cmdshell);
  // free the error list (if still allocated)
  PLListDel(g.errors);
  // if the search was interrupted because the user closed the window:
  if ( g.state & IS_WAITSEARCHEND ) srchResDel(g.pSrch);

#ifdef ALTERNATE_MEM_MANAGMENT      // 20091202
  MemHeapTerm();
#endif  // ALTERNATE_MEM_MANAGMENT

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
            BOOL initHeap(VOID)
  {

#ifdef ALTERNATE_MEM_MANAGMENT      // 20091202
  if ( !MemHeapInit() )
    {
    printDosApiError(NULLHANDLE, ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
    } /* endif */
#endif  // ALTERNATE_MEM_MANAGMENT

  return TRUE;
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID initMainDlg(HWND hwnd)
  {
  MRESULT rc;
  HAB hab = WinQueryAnchorBlock(hwnd);
  HWND hwndMenu;
  char cBuf[256];
  APIRET rc2 = 1;
  HMODULE hmodDLL;        /* resource handle                      */
  ULONG   idMenuid;       /* resource menu id                     */
  BOOL    fSuccess;       /* success indicator                    */
  ULONG   flOptions;      /* pop-up menu options                  */
  SWP swp;
  APIRET arc = 0;

  // set the dialog icon
  WinSendMsg(hwnd, WM_SETICON,
             (MPARAM)WinLoadPointer(HWND_DESKTOP, NULLHANDLE, WIN_MAIN),
             MPVOID);
  // load main window menu
  hwndMenu=WinLoadMenu(hwnd, NULLHANDLE, WIN_MAIN);

  // load pop-up menu for main window
  g.hwndMenuPuMain=WinLoadMenu( hwnd, NULLHANDLE, PU_MAIN);
  WinPopupMenu(HWND_OBJECT, HWND_OBJECT, g.hwndMenuPuMain, -32767, -32767, 0, 0);

  // load pop-up menu for list boxes
  g.hwndMenuPuLbox=WinLoadMenu( hwnd, NULLHANDLE, PU_LBOX);
  WinPopupMenu(HWND_OBJECT, HWND_OBJECT, g.hwndMenuPuLbox, -32767, -32767, 0, 0);

  TRACE3("hwndMenu=0x%X, hwndMenuPu=0x%X, hwndMenuPuLbox=0x%X", hwndMenu, g.hwndMenuPuMain, g.hwndMenuPuLbox);

  TranslateMenu(hwndMenu);

  // set offset for Pop-up menu
  if ( g.hwndMenuPuMain )
    {
    WinQueryWindowPos(g.hwndMenuPuMain, &swp);
    g.PopUpMainCy = swp.cy;
    TRACE1("g.PopUpMainCy=%d", g.PopUpMainCy);
    }

  // set offset for Pop-up menu
  if ( g.hwndMenuPuLbox )
    {
    WinQueryWindowPos(g.hwndMenuPuLbox, &swp);
    g.PopUpLboxCy = swp.cy;
    TRACE1("g.PopUpLboxCy=%d", g.PopUpLboxCy);
    }

  // load NLS data main window
  NlsGet("Files to search for", cBuf);            rc2 &= WinSetDlgItemText(hwnd, TXT_FILESRCH, cBuf);
  NlsGet("Text to search for (if any)", cBuf);    rc2 &= WinSetDlgItemText(hwnd, TXT_TEXTSRCH, cBuf);
  NlsGet("Search results history", cBuf);         rc2 &= WinSetDlgItemText(hwnd, TXT_SRCHRES, cBuf);
  NlsGet("Files found", cBuf);                    rc2 &= WinSetDlgItemText(hwnd, TXT_FILEFOUND, cBuf);
  NlsGet("Text found", cBuf);                     rc2 &= WinSetDlgItemText(hwnd, TXT_TEXTFOUND, cBuf);

  // load NLS for buttons
  NlsGet("~Search", cBuf);                        rc2 &= WinSetDlgItemText(hwnd, PB_SEARCH, cBuf);
  NlsGet("S~top", cBuf);                          rc2 &= WinSetDlgItemText(hwnd, PB_STOP, cBuf);
  TRACE1("rc2 from all 'WinSetDlgItemText' = %d (should be TRUE)", rc2);

  dMenuItemCheck(hwnd, IDM_VSORTNONE, TRUE);
  dMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VSORTNONE, TRUE);
  dMenuItemCheck(hwnd, IDM_VNOSPACES, TRUE);
  dMenuItemCheck(g.hwndMenuPuMain, PU_IDM_VNOSPACES, TRUE);


  DlgItemTextSet(hwnd, DD_SRCHRES, "");
  // if an history file was loaded via the startup parameters fills the
  // proper history listbox(es)
  if ( g.loadData )
    loadFile(hwnd, NULL);
  // if file-to-search was specified as startup parameter
  if ( g.startData.fileToSearch )
    {
    DlgItemTextSet(hwnd, DD_FILESRCH, String(g.startData.fileToSearch));
    StringDel(&g.startData.fileToSearch);
    } /* endif */
  // if text-to-search was specified as startup parameter
  if ( g.startData.textToSearch )
    {
    DlgItemTextSet(hwnd, DD_TEXTSRCH, String(g.startData.textToSearch));
    StringDel(&g.startData.textToSearch);
    } /* endif */

  // 20090925 AB make list boxes bigger for longer search strings
  TRACE2("WinWindowFromID(hwnd, DD_FILESRCH)=0x%X, WinWindowFromID(WinWindowFromID(hwnd, DD_FILESRCH, 0x29A)=0x%X", WinWindowFromID(hwnd, DD_FILESRCH), WinWindowFromID(WinWindowFromID(hwnd, DD_FILESRCH), 0x29A) );
  rc = WinSendMsg(WinWindowFromID(hwnd, DD_FILESRCH),
                  EM_SETTEXTLIMIT,
                  MPFROMSHORT(LIST_BOX_WIDTH),
                  (MPARAM)0);
  TRACE1("rc=%d (should be TRUE)", rc);
  TRACE1("DD_FILESRCH entries: %d", WinQueryLboxCount(WinWindowFromID( hwnd , DD_FILESRCH ) ) );
  TRACE2("CBID_LIST=0x%X, CBID_EDIT=0x%X", DlgItemHwnd(DlgItemHwnd(hwnd, DD_FILESRCH), CBID_LIST ), DlgItemHwnd(DlgItemHwnd(hwnd, DD_FILESRCH), CBID_EDIT ) );
  rc = WinSendMsg(WinWindowFromID(hwnd, DD_TEXTSRCH),
                  EM_SETTEXTLIMIT,
                  MPFROMSHORT(LIST_BOX_WIDTH),
                  (MPARAM)0);
  TRACE1("rc=%d (should be TRUE)", rc);
  TRACE1("DD_TEXTSRCH entries: %d", WinQueryLboxCount(WinWindowFromID( hwnd , DD_TEXTSRCH ) ) );

  TRACE2("WinWindowFromID(hwnd, EF_SEARCHING)=0x%X, WinWindowFromID(hwnd, DD_FILESRCH)=0x%X", WinWindowFromID(hwnd, EF_SEARCHING), WinWindowFromID(hwnd, DD_FILESRCH) );
  rc = WinSendDlgItemMsg(hwnd, EF_SEARCHING,
                         EM_SETTEXTLIMIT,
                         (MPARAM)(LIST_BOX_WIDTH + 20),       // some additional space for [R.I.]
                         (MPARAM)0);
  TRACE1("rc=%d (should be TRUE)", rc);
  rc = WinSendMsg(WinWindowFromID(hwnd, DD_SRCHRES),
                  EM_SETTEXTLIMIT,
                  MPFROMSHORT(LIST_BOX_WIDTH + 20),       // some additional space for [R.I.]
                  (MPARAM)0);
  TRACE1("rc=%d (should be TRUE)", rc);

  // subclass windows
  subclassWindows(hwnd);

  updateOptionsMenu(hwnd, g.setting);

  // 20080825 AB disable Selected menu on startup as long as no file is selected
  dMenuItemEnable(hwnd, IDM_SELECTED, FALSE);
  // 20081125 AB disable EDIT menu on startup, no logic behind, maybe remove completly?
  dMenuItemEnable(hwnd, IDM_EDIT, FALSE);

  // if data were loaded from the initialization file update the interface
  if ( g.startData.iniData )
    {
    TRACE1("load from iniData, hwnd: 0x%0X", hwnd);
    setInterfaceFromIniData(hwnd, g.startData.iniData);
    //free(g.startData.iniData);
    //g.startData.iniData = NULL;
    }
  else
    {
    TRACE("not from iniData.....");
    g.state |= IS_RESETDLGFONT;
    if ( !WinSetPresParam(hwnd, PP_FONTNAMESIZE, 11, "9.WarpSans") )
      WinSetPresParam(hwnd, PP_FONTNAMESIZE, 7, "8.Helv");
    WinSetPresParam(DlgItemHwnd(hwnd, LBOX_FILEFOUND), PP_FONTNAMESIZE, 13, "5.System VIO");
    WinSetPresParam(DlgItemHwnd(hwnd, LBOX_TEXTFOUND), PP_FONTNAMESIZE, 13, "5.System VIO");
    g.state &= ~IS_RESETDLGFONT;
    } /* endif */

  TRACE1("g.hwndMainWin = 0x%0X", g.hwndMainWin);
  TRACE1("hwnd = 0x%0X", hwnd);

  initSizes(hwnd);

  rc=WinSendMsg(hwnd, WM_UPDATEFRAME, (MPARAM) NULL, (MPARAM) NULL);
  //rc = WinSendMsg(hwnd, WM_UPDATEFRAME, (MPARAM)FCF_MENU, MPVOID);
  TRACE1("rc=%d (should be true)", rc);
  WinSetAccelTable(hab, WinLoadAccelTable(hab, 0, 1), hwnd);

  g.state |= IS_INITDONE;
  }


//===========================================================================
// Update the interface according to the values read from the initialiation
// file.
// Parameters --------------------------------------------------------------
// HWND hwnd    : window handle
// PPMSEEKINI p : initialization data
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID setInterfaceFromIniData(HWND hwnd, PPMSEEKINI p)
  {
  int i;

  TRACE("setInterface....");
  g.state |= IS_SIZEPOSFROMINI;
  if ( p->clrBgSrch >= 0 )
    DlgItemSetPresParam(hwnd, PB_SEARCH, PP_BACKGROUNDCOLOR, 4, &p->clrBgSrch);
  if ( p->clrFgSrch >= 0 )
    DlgItemSetPresParam(hwnd, PB_SEARCH, PP_FOREGROUNDCOLOR, 4, &p->clrFgSrch);
  if ( p->clrBgStop >= 0 )
    DlgItemSetPresParam(hwnd, PB_STOP, PP_BACKGROUNDCOLOR, 4, &p->clrBgStop);
  if ( p->clrFgStop >= 0 )
    DlgItemSetPresParam(hwnd, PB_STOP, PP_FOREGROUNDCOLOR, 4, &p->clrFgStop);
#ifdef __DEBUG_TRACE__
  //      // 20080728 AB no Help Button anymore 20090123 but TEST button when in DEBUG
  WinEnableWindow(DlgItemHwnd(hwnd, PB_TEST), TRUE);
  WinShowWindow(DlgItemHwnd(hwnd, PB_TEST), TRUE);
  if ( p->clrBgHelp >= 0 ) DlgItemSetPresParam(hwnd, PB_TEST, PP_BACKGROUNDCOLOR, 4, &p->clrBgHelp);
  if ( p->clrFgHelp >= 0 ) DlgItemSetPresParam(hwnd, PB_TEST, PP_FOREGROUNDCOLOR, 4, &p->clrFgHelp);
#endif
  //TRACE1("hwnd: 0x%0X",hwnd);
  if ( p->achFont )
    {
    i = strlen(p->achFont) + 1;
    TRACE2("have p->achFont, len: %d, font: %s", i, p->achFont);
    g.state |= IS_RESETDLGFONT;
    WinSetPresParam(hwnd, PP_FONTNAMESIZE, i, p->achFont);
    g.state &= ~IS_RESETDLGFONT;
    } /* endif */
  if ( p->achResFont )
    {
    i = strlen(p->achResFont) + 1;
    TRACE2("have p->achResFont, len: %d, font: %s", i, p->achResFont);
    WinSetPresParam(DlgItemHwnd(hwnd, LBOX_TEXTFOUND), PP_FONTNAMESIZE, i, p->achResFont);
    WinSetPresParam(DlgItemHwnd(hwnd, LBOX_FILEFOUND), PP_FONTNAMESIZE, i, p->achResFont);
    }

  if ( p->ver == INI_FILE_VERSION )
    { // only load lists if the ini file matches these version
    resetSrchFilTxtHist(DlgItemHwnd(hwnd, DD_FILESRCH), p->files);
    PLListDel(p->files);
    resetSrchFilTxtHist(DlgItemHwnd(hwnd, DD_TEXTSRCH), p->texts);
    PLListDel(p->texts);
    applWinPosSizeSet(hwnd, &p->swp);
    }
  else
    {
    TRACE2("INI file version mismatch, current=%d, file=%d", INI_FILE_VERSION, p->ver);
    }
  }


//===========================================================================
// When the window is closed save its size, position, font, etc.
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

VOID saveState(HWND hwnd)
  {
  PPMSEEKINI p;
  PPLLIST pFiles, pTexts;
  HFILE hf;
  BOOL success = FALSE;
  if ( !g.startData.iniFile || !(p = wMalloc(hwnd, sizeof(PMSEEKINI))) )
    goto exit0;
  memset(p, 0, sizeof(PMSEEKINI));
  // version
  p->ver = INI_FILE_VERSION;
  // window position and size
  applWinPosSize(hwnd, &p->swp);
  // presentation parameters
  p->clrBgSrch = presParam(hwnd, PB_SEARCH, PP_BACKGROUNDCOLOR);
  p->clrFgSrch = presParam(hwnd, PB_SEARCH, PP_FOREGROUNDCOLOR);
  p->clrBgStop = presParam(hwnd, PB_STOP, PP_BACKGROUNDCOLOR);
  p->clrFgStop = presParam(hwnd, PB_STOP, PP_FOREGROUNDCOLOR);
  // 20080728 AB no Help Button anymore, 20090123 take colors from PB_STOP instead for at this time inactive PB_TEST
  p->clrBgHelp = presParam(hwnd, PB_STOP, PP_BACKGROUNDCOLOR);
  p->clrFgHelp = presParam(hwnd, PB_STOP, PP_FOREGROUNDCOLOR);

  WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0, NULL, sizeof(p->achFont),
                    p->achFont, QPF_NOINHERIT);
  WinQueryPresParam(DlgItemHwnd(hwnd, LBOX_FILEFOUND),
                    PP_FONTNAMESIZE, 0, NULL, sizeof(p->achResFont),
                    p->achResFont, QPF_NOINHERIT);
  // search options
  p->option = g.setting & OPT_SRCHMASK;     // 20091205 AB options are not stored
  p->option |= g.setting & OPT_VIEWMASK;
  // maximum result history entries
  p->maxHistory = g.maxHistory;
  // editor
  if ( g.editor )
    memcpy(p->achEditor, String(g.editor), StringLen(g.editor) + 1);
  // command shell
  if ( g.cmdshell )
    memcpy(p->achCmdShell, String(g.cmdshell), StringLen(g.cmdshell) + 1);
  // history of searched files
  if ( !saveFilTxtHist(hwnd, DD_FILESRCH, &p->files) ) goto exit1;
  pFiles = p->files;
  if ( pFiles ) p->cbFiles = pFiles->cbTot;
  // history of searched text strings
  if ( !saveFilTxtHist(hwnd, DD_TEXTSRCH, &p->texts) ) goto exit2;
  pTexts = p->texts;
  if ( pTexts ) p->cbTexts = pTexts->cbTot;

  // FOC settings
  p->iFocX = g.iFocX;
  p->iFocY = g.iFocY;
  memcpy(p->cFocLastDir, g.cFocLastDir, strlen(g.cFocLastDir) + 1);

  // write the ini file
  hf = fmOpen(String(g.startData.iniFile),
              FMO_CREATE | FMO_REPLACE | FMO_SHARENONE | FMO_WRITEONLY);
  if ( hf == FMO_ERROR ) goto exit3;
  if ( !fmWrite(hf, p, sizeof(PMSEEKINI)) ||
       (pFiles && !fmWrite(hf, pFiles, pFiles->cbTot)) ||
       (pTexts && !fmWrite(hf, pTexts, pTexts->cbTot)) )
    goto exit4;
  success = TRUE;
  exit4:
  fmClose(hf);
  exit3:
  PLListDel(pTexts);
  exit2:
  PLListDel(pFiles);
  exit1:
  free(p);
  exit0:
  if ( !success && !g.startData.ignorePrfErrors )
    printApplicationError(hwnd, ERROR_SAVEINI,
                          (g.startData.iniFile ? String(g.startData.iniFile) :
                           NULL));
  }


//===========================================================================
// Return the value used for the given presentation parameter.
// If no presentation parameter is set return -1;
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// ULONG id  : control id
// ULONG pp  : presentation parameter
// Return value ------------------------------------------------------------
// LONG : current color or -1 if no presentation parameter is set.
//===========================================================================
//static
LONG presParam(HWND hwnd, ULONG id, ULONG pp)
  {
  LONG clr;
  if ( !DlgItemPresParam(hwnd, id, pp, 0, NULL, 4, &clr,
                         QPF_NOINHERIT | QPF_PURERGBCOLOR) )
    return -1;
  return clr;
  }


//===========================================================================
// When the window is closed release used resources:
// - history of results
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID freeResources(HWND hwnd)
  {
  freeResultHistory(hwnd);
  }

//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : drives window handle
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID initDrivesDlg(HWND hwnd)
  {
  HAB hab = WinQueryAnchorBlock(hwnd);
  char buf[6];
  char cBuf[256];
  int i, j;
  APIRET rc;

  // 20100415 only if MM dlls are found
  if ( !g.iHaveMMdlls ) return;

  buf[0] = '-';
  buf[1] = 0xB3;  // vertical line
  buf[2] = ' ';
  buf[4] = ':';
  buf[5] = '\0';

  // disable rectangle in list box (we make it with text instead) 20090129 AB
  WinSetWindowPos(DlgItemHwnd(hwnd, LB_DRIVES_RECT), HWND_TOP, 0 , 0, 20, 20, SWP_HIDE);

  NlsGet("Drives to search (if not specified)", cBuf);
  WinSetDlgItemText(g.hwndMainWin, PB_DRIVES, cBuf);

  TRACE("setting up selection button text");
  NlsGet("Hard disks", DriveSelection[0]);
  NlsGet("Network drives", DriveSelection[1]);
  NlsGet("CD/DVD drives", DriveSelection[2]);
  NlsGet("Zip/EZ/Jaz drives", DriveSelection[3]);
  NlsGet("ALL DRIVES", DriveSelection[4]);

  // fill in default drives list
  for ( i = 'A', j = LB_DRIVES_FIRST_DRIVE_ENTRY; i <= 'Z'; i++, j++ )
    {
    buf[3] = i;
    //     WinSendMsg(DlgItemHwnd(hwnd, LB_DRIVES), LM_INSERTITEM, (MPARAM) j, buf);
    }

  for ( i = CB_LBOX_FIRST; i < CB_LBOX_FIRST + LB_DRIVES_FIRST_DRIVE_ENTRY; i++ )
    {
    if ( ! (i - CB_LBOX_FIRST) )
      {
      TRACE2("DriveSelection[%d] text is '%s'", i, DriveSelection[i - CB_LBOX_FIRST]);
      }
    rc=WinSetDlgItemText(g.hwndMainDialogBox, i, DriveSelection[i - CB_LBOX_FIRST]);
    if ( !rc ) TRACE1("WinSetDlgItemText rc=%d (should be true)", rc);
    }

  g.iIsDrvSel[2] = TRUE;
  StartFillInfoThread();
  }

//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwndMenu : menu handle
// Return value: 0 if no error, 1 if error
//===========================================================================
int TranslateMenu(HWND hwndMenu)
  {
  // load NLS data for menus
  char cBuf[256];
  char sCtrl[32];
  char sAlt[32];
  char sShft[32];
  //HWND hwndMenu = DlgItemHwnd(hwnd, FID_MENU);
  APIRET rc = 1, rc2 = 0, arc = 1;

  NlsGet("Ctrl", sCtrl);
  NlsGet("Alt", sAlt);
  NlsGet("Shift", sShft);

  NlsGet("~File", cBuf);
  TRACE1("NLS for File=%s", cBuf);
  rc &= WinSetMenuItemText(hwndMenu, IDM_FILE, cBuf);
  NlsGet("~Open", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_OPEN, cBuf);
  NlsGet("~Save", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_SAVE, cBuf);
  NlsGet("Save ~as", cBuf);                               rc &= WinSetMenuItemText(hwndMenu, IDM_SAVEAS, cBuf);
  NlsGet("E~xit", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_EXIT, cBuf);

  NlsGet("~Edit", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_EDIT, cBuf);
  NlsGet("Cu~t", cBuf);   strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+X");
  rc &= WinSetMenuItemText(hwndMenu, IDM_CUT, cBuf);
  NlsGet("~Copy", cBuf);  strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+C");
  rc &= WinSetMenuItemText(hwndMenu, IDM_COPY, cBuf);
  NlsGet("~Paste", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+V");
  rc &= WinSetMenuItemText(hwndMenu, IDM_PASTE, cBuf);

  dMenuItemEnable(hwndMenu, IDM_SELECTED, TRUE);      // WinEnableMenuItem can be used with os2tk45
  NlsGet("~Selected", cBuf); rc &= WinSetMenuItemText(hwndMenu, IDM_SELECTED, cBuf);
  //rc2 = NlsGet("~Selected", cBuf);
  //TRACE2("rc2=%d, Trans.='%s'", rc2, cBuf);
  //rc &= WinSetMenuItemText(hwndMenu, IDM_SELECTED, "Ž_TEST_Ausgew„hlt");
  NlsGet("~Edit", cBuf);  strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+E / F4");
  rc &= WinSetMenuItemText(hwndMenu, IDM_EDITSEL, cBuf);
  NlsGet("~Open", cBuf);  strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+O");
  rc &= WinSetMenuItemText(hwndMenu, IDM_OPENSEL, cBuf);
  NlsGet("~Copy to clipboard", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+C");
  rc &= WinSetMenuItemText(hwndMenu, IDM_COPYFNAMES, cBuf);
  NlsGet("Copy whole l~ine to clipboard", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+"); strcat(cBuf, sShft); strcat(cBuf, "+C");
  rc &= WinSetMenuItemText(hwndMenu, IDM_COPYLINE, cBuf);
  NlsGet("Copy ~all to clipboard", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+"); strcat(cBuf, sAlt); strcat(cBuf, "+C");
  rc &= WinSetMenuItemText(hwndMenu, IDM_COPYALLFNAMES, cBuf);
  NlsGet("Open ~folder", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+F");
  rc &= WinSetMenuItemText(hwndMenu, IDM_OPENDIR, cBuf);
  NlsGet("Open command ~line", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+L / F2");
  rc &= WinSetMenuItemText(hwndMenu, IDM_OPENCLI, cBuf);
  NlsGet("Open ~settings", cBuf); strcat (cBuf, "\t"); strcat (cBuf, sCtrl), strcat(cBuf, "+P");
  rc &= WinSetMenuItemText(hwndMenu, IDM_PROPERTIES, cBuf);

  NlsGet("~View", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_VIEW, cBuf);
  NlsGet("D~o not sort", cBuf);                           rc &= WinSetMenuItemText(hwndMenu, IDM_VSORTNONE, cBuf);
  NlsGet("Sort ~ascending", cBuf);                        rc &= WinSetMenuItemText(hwndMenu, IDM_VSORTASCEND, cBuf);
  NlsGet("Sort d~escending", cBuf);                       rc &= WinSetMenuItemText(hwndMenu, IDM_VSORTDESCEND, cBuf);
  NlsGet("~Date", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_VFDATE, cBuf);
  NlsGet("~Time", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_VFTIME, cBuf);
  NlsGet("~Size", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_VFSIZE, cBuf);
  NlsGet("Line ~numbers", cBuf);                          rc &= WinSetMenuItemText(hwndMenu, IDM_VLINENO, cBuf);
  NlsGet("St~rip spaces", cBuf);                          rc &= WinSetMenuItemText(hwndMenu, IDM_VNOSPACES, cBuf);
  NlsGet("Settings~...", cBuf);                           rc &= WinSetMenuItemText(hwndMenu, IDM_SETTINGS, cBuf);


  NlsGet("~Help", cBuf);                                  rc &= WinSetMenuItemText(hwndMenu, IDM_HELP, cBuf);
  NlsGet("Help ~index", cBuf);                            rc &= WinSetMenuItemText(hwndMenu, IDM_HELPIDX, cBuf);
  NlsGet("~Product information", cBuf);                   rc &= WinSetMenuItemText(hwndMenu, IDM_PRODINFO, cBuf);

  TRACE1("rc from all 'WinSetxxxItemText' = %d (should be TRUE)", rc);

  // Translate Pop-up window
  NlsGet("~Recursive search", cBuf);                      arc =  WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_SETRECUR, cBuf);
  NlsGet("E~xclude binary files", cBuf);                  arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_SETTEXTONLY, cBuf);
  NlsGet("Case ~insensitive", cBuf);                      arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_SETCASEINS, cBuf);
  NlsGet("~Boolean operators", cBuf);                     arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_SETBOOLOP, cBuf);
  NlsGet("Search for ~folder names too", cBuf);           arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_SETDIRSEARCH, cBuf);

  NlsGet("D~o not sort", cBuf);                           arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VSORTNONE, cBuf);
  NlsGet("Sort ~ascending", cBuf);                        arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VSORTASCEND, cBuf);
  NlsGet("Sort d~escending", cBuf);                       arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VSORTDESCEND, cBuf);

  NlsGet("~Date", cBuf);                                  arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VFDATE, cBuf);
  NlsGet("~Time", cBuf);                                  arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VFTIME, cBuf);
  NlsGet("~Size", cBuf);                                  arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VFSIZE, cBuf);
  NlsGet("Line ~numbers", cBuf);                          arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VLINENO, cBuf);
  NlsGet("St~rip spaces", cBuf);                          arc &= WinSetMenuItemText(g.hwndMenuPuMain, PU_IDM_VNOSPACES, cBuf);

  TRACE1("arc=%d (should be 1)", arc);

  // Pop-up menu for list boxes (if not emty)
  NlsGet("~Edit", cBuf); strcat (cBuf, "\t"); strcat(cBuf, "F4");
  rc  = WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_EDITSEL, cBuf);
  NlsGet("~Open", cBuf);                                  rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_OPENSEL, cBuf);
  NlsGet("~Copy to clipboard", cBuf);                     rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_COPYFNAMES, cBuf);
  NlsGet("Copy whole l~ine to clipboard", cBuf);          rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_COPYLINE, cBuf);
  NlsGet("Copy ~all to clipboard", cBuf);                 rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_COPYALLFNAMES, cBuf);
  NlsGet("Open ~folder", cBuf);                           rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_OPENDIR, cBuf);
  NlsGet("Open command ~line", cBuf); strcat (cBuf, "\t"); strcat(cBuf, "F2");
  rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_OPENCLI, cBuf);
  NlsGet("Open ~settings", cBuf);                         rc &= WinSetMenuItemText(g.hwndMenuPuLbox, PU_IDM_PROPERTIES, cBuf);

  TRACE1("arc=%d (should be 1)", arc);

  return !rc;
  }


