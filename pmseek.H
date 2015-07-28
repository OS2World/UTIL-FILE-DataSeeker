/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

// pmseek.h

#ifndef _PM_SEEK_H_
   #define _PM_SEEK_H_

// version string displayed in product information dialog (prodinfo.c)
#define DATASEEKER_VERSION      "1.14"

// 20091202 AB ALTERNATE_MEM_MANAGMENT is Alessandros original own heap managment
//      comment out to use standard ibmcpp3.65 multithreading libraries and memory managment
//#define ALTERNATE_MEM_MANAGMENT   TRUE
//#define DEBUGMEM

//#ifdef __WATCOMC__              //20080x added by AB
//        #include <WatcomWrapper.h>
//#endif

// 20080822 AB strupr is not in all IBMCPP3.65 libraries
#define toupper(c) (((c >= 'a') && (c <= 'z')) ? c + 'A' - 'a' : c )
#define tolower(c) (((c >= 'A') && (c <= 'Z')) ? c - 'A' - 'a' : c )

#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR   /* File manager values  */
#define INCL_DOS
#define INCL_ERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS    /* DOS error values     */
#define INCL_TYPES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSNLS
#define INCL_DOSPROCESS
#define INCL_WIN
// 200808xx AB not needed anymore (can't remember why) #define INCL_WINWORKPLACE
#define INCL_GPI
#define INCL_MOU

// for multimedia
#define INCL_SW                 // secondary window, Window Control Functions
#define INCL_MMIOOS2    // MMIO File Services
#define INCL_MACHDR     // High-Level Services

#define INCL_LONGLONG

#include <os2.h>
#include <stdio.h>
#include <os2def.h>            // basic OS/2 data types
//#include <ctype.h>

#define __COMPATSTR__ 1     // error in VAC3.65 string.h with RC5
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <limits.h>

#include <os2me.h>      // secondary window

#include "wrappers.h"

#ifdef ALTERNATE_MEM_MANAGMENT      // 20091202
    #include "memmgr.h"
#endif

#include "stringUtil.h"
#include "fileutil.h"
#include "stringObj.h"
#include "pllist.h"
#include "wordarray.h"
#include "srchdata.h"
#include "loadsave.h"
#include "application.h"
#include "profile.h"

#include "ecolange.h"   // needed for NLS.h
#include "NLS.h"

#include "foc.h"        // needed for eCoSoft FileOpenContainer container
#include "afoc.h"

#include "include\apiExBase.h"
#include "include\apiExPM.h"

// 200808xx AB add trace definitions for Dennis Bareis pmprintf untility
#ifdef __DEBUG_TRACE__
        #include "pmprintf.h"
        #include <STDARG.H>
                #define  __PMPRINTF__
        //#define TRACE( ... )   PmpfF ( ( __VA_ARGS__ ) )        // doesn't work for VAC3.65 and ICCv4

        #define TRACE(text)                                             PmpfF((text))
        #define TRACE1(text, param1)                                    PmpfF((text, param1))
        #define TRACE2(text, param1, param2)                            PmpfF((text, param1, param2))
        #define TRACE3(text, param1, param2, param3)                    PmpfF((text, param1, param2, param3))
        #define TRACE4(text, param1, param2, param3, param4)            PmpfF((text, param1, param2, param3, param4))
        #define TRACE5(text, param1, param2, param3, param4, param5)    PmpfF((text, param1, param2, param3, param4, param5))
#else
        #define TRACE(text)
        #define TRACE1(text, param1)
        #define TRACE2(text, param1, param2)
        #define TRACE3(text, param1, param2, param3)
        #define TRACE4(text, param1, param2, param3, param4)
        #define TRACE5(text, param1, param2, param3, param4, param5)
#endif  // __DEBUG_TRACE__
// end trace definitions

#include "exceptq.h"

// 20080xxx AB, why should we undef this stuff ? (needed for icc365 with 3.65 headers)
//   #undef _heapmin
//   #undef malloc
//   #undef free
//   #undef realloc
//   #undef strdup


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef ALTERNATE_MEM_MANAGMENT      // 20091202
   #ifdef DEBUGMEM
      #include <assert.h>

      #define malloc(_cb_)  \
         dbgMalloc(__FILE__, __LINE__, __FUNCTION__, (_cb_))

      #define free(_p_) \
         dbgFree(__FILE__, __LINE__, __FUNCTION__, (_p_))

      #define realloc(_p_, _cb_) \
         dbgRealloc(__FILE__, __LINE__, __FUNCTION__, (_p_), (_cb_))

      #define _heapmin() \
         dbgHeapMin(__FILE__, __LINE__, __FUNCTION__)

      #define strdup(_str_) \
         dbgStrDup(__FILE__, __LINE__, __FUNCTION__, (_str_))

   #else
      #define malloc(_cb_)          MemAlloc(_cb_)
      #define free(_p_)             MemFree(_p_)
      #define realloc(_p_, _cb_)    MemRealloc((_p_), (_cb_))
      #define _heapmin()            MemHeapMin()
      #define strdup(_str_)         MemStrDup(_str_)
   #endif
#endif  // ALTERNATE_MEM_MANAGMENT


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// help file name -----------------------------------------------------------

#define SZFILE_HLP            "DATASEEK.HLP"

// additional message box flags ---------------------------------------------

#define DDBOXES_OFFSET                                  40              // offset DD boxes from lower left corner (to not overlap buttons)

// 20080906 AB too bad that URE changes all values from hex to dec
#define MB_MSGFROMRES                   1024
#define MB_TITLEFROMRES                 2048

// state flags --------------------------------------------------------------

#define IS_SEARCHING                            1    // searching files
#define IS_STOPPED                         2    // user clicked on Stop
#define IS_WAITSEARCHEND                   4    // the user close the window
                                                // while it was working
#define IS_MUSTFREERESULT                  8    // the user interrupted the
                                                // search via the stop button
                                                // the result buffer must be
                                                // freed on the next search

#define IS_EDITOREPM                      16    // the selected editor is EPM
// 200808xx AB add MED and VisualSlickEdit
#define IS_EDITORMED                      32    // the selected editor is MED
#define IS_EDITORVS                       64    // the selected editor is VisualSlickEdit

#define IS_SKIPCTRLNOTIF                8192    // skip WM_CONTROL
#define IS_MSELENABLED                 16384    // 'Selected' menuitem enabled
#define IS_INITDONE                    32768    // initialization complete
#define IS_INIT_DRIVES_DONE                               65536 // initialization of WIN_DRIVES dialog complete

// listbox/dropdownbox font is beeing changed
#define IS_RESETBOXFONTS               65536
#define IS_RESETDLGFONT               131072

// result listboxes states
#define IS_RESLBOXSPLIT           0x00000000    // text found height ==
                                                // files found height
#define IS_RESLBOXTEXTHIDDEN        16777216    // hidden text found listbox
#define IS_RESLBOXFFMIN             33554432    // minimized files found listbox
#define IS_RESLBOXMASK              50331648    // valid flags mask

#define IS_SIZEPOSFROMINI         1073741824    // size-position set from INI file
#define IS_EXITING               -2147483648    // application termination

// generic busy flag
#define IS_BUSY     (IS_SEARCHING | IS_STOPPED | IS_WAITSEARCHEND | IS_EXITING)

// WM_COMMAND parameters sent by the object window procedure or produced ----
// by the accelerator table

#define CMD_SEARCHEND           900    // search completed
#define CMD_DELENTRY            901    // del entry from dropdownbox
#define CMD_DELALLENTRY         902    // del all dropdownbox entries
#define CMD_EDITSEL2            903    // edit selected file in same window

// file-found/text-found resize steps (must be a multiple of 2)
#define RESRESZ_STEPS          32
#define RESRESZ_BELOWMIN        0
#define RESRESZ_DEFAULT        (RESRESZ_STEPS >> 1)
#define RESRESZ_DISDOWN         1
#define RESRESZ_ENDOWN          2
#define RESRESZ_ENUP           (RESRESZ_STEPS - 2)
#define RESRESZ_DISUP          (RESRESZ_STEPS - 1)
#define RESRESZ_ABOVEMAX       RESRESZ_STEPS

// from old resid.h file ---------------------------------------------------------
#define IDS_SELRES             2
#define IDS_TITLE              3
#define IDS_SEARCHING          4
#define IDS_FILESFOUND         5

#define IDSERR_DOSAPI          16
#define IDSERR_MALLOC          17

#define IDSERR_FIRST           32

// USER PM messages ---------------------------------------------------------
#define OWM_QUIT             WM_USER
#define OWM_SEARCH           (WM_USER + 1)
#define OWM_UNCOMMIT         (WM_USER + 2)
// sent by the subclassed listboxes/dropdown boxes
#define SWM_PPCHANGED        (WM_USER + 3)
// sent by the subclassed static text controls
#define SWM_PPCHANGED2       (WM_USER + 4)
// messages for WIN_DRIVES dialog
#define WM_WIN_DRIVES_INIT_2    (WM_USER + 5)
#define WM_PB_DRIVES            (WM_USER + 6)
#define WM_TOGGLE_VISIBILITY    (WM_USER + 7)
#define WM_WIN_DRIVES_VISIBLE   (WM_USER + 8)
#define WM_WIN_DRIVES_INVISIBLE (WM_USER + 9)
#define WM_TEST                 (WM_USER + 10)
#define WM_NEW_DRIVE_INFO       (WM_USER + 11)
#define WM_DRIVE_UPDATE         (WM_USER + 12)
#define WM_INITDLG2             (WM_USER + 13)
#define WM_POPUP_LBOX_MENU      (WM_USER + 14)
#define WM_DRIVE_STOP_TIMER     (WM_USER + 15)
#define WM_DRIVE_START_TIMER     (WM_USER + 16)

// Options ------------------------------------------------------------------
#define OPT_RECUR                     1    //            1  recur subdirectories
#define OPT_TEXTONLY                  2    //            2  skip files containing NUL chars
#define OPT_FIND_DIR                  4    //            4  find directory entries too

#define OPT_CASEINS                4096    //       0x1000  text search is case insensitive
#define OPT_BOOLOP                 8192    //       0x2000  text contains boolean operators

#define OPT_SRCHMASK              12295    //       0x3007  valid search options mask

#define OPT_SHOWFDATE             65536    //      0x10000  show file date
#define OPT_SHOWFTIME            131072    //      0x20000  show file time
#define OPT_SHOWFSIZE            262144    //      0x40000  show file size
#define OPT_SHOWLINENO           524288    //      0x80000  show line numbers
#define OPT_STRIPSPACES         1048576    //     0x100000  strip spaces

#define OPT_VIEWMASK            2031616    //     0x1F0000  valid view options mask

#define OPT_PREVOPT           268435456    //   0x10000000  save the current options when
                                           //               the windows is closed
#define OPT_PREVVIEW          536870912    //   0x20000000  save the current view when the
                                           //               window is closed
#define OPT_BOOLOPOR         1073741824    //   0x40000000  text strings ORed
#define OPT_BOOLOPAND       -2147483648    //   0x80000000  text strings ANDed

// 20080902 AB added to do not sort file/text list
#define OPT_DONOTSORTHISTORY       256

// various defines-----------------------------------------------------------

#define LB_DRIVES_FIRST_DRIVE_ENTRY             6
#define MAX_LENGHT_SEPARATION_BAR               128
#define MAX_LENGHT_SELECTION_ENTRIES    80
// error handling flags (text search only) ----------------------------------

#define TXTSRCH_NONE                  0x0000
#define TXTSRCH_TXTFOUNDLIST               1
#define TXTSRCH_OPENFILE                   2

#define LIST_BOX_WIDTH                  512     // width of listboxes
// data types ---------------------------------------------------------------

// fill files found listbox structure ---------------------------------------
typedef struct {
   HWND hCtrl;
   CHAR buf[300];
} FILLFILESFOUND, * PFILLFILESFOUND;

// object window data -------------------------------------------------------
typedef struct {
   TID tid;                 // thread id
   HEV hev;                 // envent semaphore handle
   HWND hwnd;               // object window handle
} OBJECTWND, * POBJECTWND;

// country specific characters ----------------------------------------------

typedef struct {
   CHAR date[4];            // date separator
   CHAR time[4];            // time separator
   CHAR thousand[4];        // thousand separator
} NLSSEPARATORS, * PNSLSEPARATORS;

// startup data -------------------------------------------------------------
typedef struct {
   STRING iniFile;          // name of the initialization file
   STRING loadFile;         // file to load if any
   STRING fileToSearch;     // file to search if any
   STRING textToSearch;     // text to search if any
   PPMSEEKINI iniData;      // initialization data if any
   BOOL ignorePrfErrors;    // ignore read/write profile errors
} STARTUPDATA, * PSTARTUPDATA;

#pragma pack(2)
// global data
typedef struct
    {
    OBJECTWND objWnd;        // object window data
    STARTUPDATA startData;   // program startup parameters
    HMODULE hMsgMod;         // handle of the message module
    HACCEL hacc;             // accelerator table handle
    ApiExPM_SIZES font;              // font size
    ULONG state;             // application state
    ULONG option;            // current options mask
    ULONG setting;           // previously saved options
    USHORT maxHistory;       // maximum history list length
    SHORT sort;              // sort mode
    int iHistSort;                       // sort mode in history fields (file/text)
    PSEARCHRES pSrch;        // current search results
    STRING editor;           // text file editor
    STRING cmdshell;         // command shell
//   PPLLIST pSrchRes;        // current search results
    PSEARCHBUFFER pSrchBuf;  // search buffer
    PPLLIST errors;          // errors list
    PDSHISTFILE loadData;    // data loaded from file via the worker thread
    NLSSEPARATORS nls;       // separators
    PFNWP pDefLboxProc;
    PFNWP pDefEfProc;
    PFNWP pDefEfProc2;
    PFNWP pDefTxtProc;       // default WC_STATIC procedure
    ULONG ulElapsed;         // used to calculate the elapsed time
    BOOL bFileScrolled;      // indicates that user has scrolled in file list while searching
    BOOL bTextScrolled;      // indicates that user has scrolled in text window while searching
    HWND hwndWinDrives;
    HWND hwndMainWin;
    HWND hwndDriveInfo;
    HWND hwndMainDialogBox;
    HWND hwndMenuPuMain;
    HWND hwndMenuPuLbox;
//   HWND hwndCaption;
    TID  DriveInfoTid;
    BOOL WinDrivesVisible;
    RECTL WinDrivesSwp;
    SWP PbDrivesR;
    int iDrivesTextMaxLenght;
    int iNumVisibleDrives;      // number of visible drives including 'general selection buttons'
    int iIsDrvSel[LB_DRIVES_FIRST_DRIVE_ENTRY + 26];        // is drive selected in LB_DRIVES
    INT driveflags[26];         // for wrappers.c
    int PopUpMainCy;
    int PopUpLboxCy;
    int iHaveMMdlls;
    int iFocX;
    int iFocY;
    char cFocLastDir[CCHMAXPATH];
    } GLOBAL, * PGLOBAL;
#pragma pack()
extern GLOBAL g;

enum DRIVE_TYPE
        {
        /** Unknown drive type (?). */
        DT_UNKNOWN,             // 0
        /** Standard floppy drive (typically A: or B:). */
        DT_FLOPPY,              // 1
        /** Standard CD-ROM. */
        DT_CDROM,               // 2
        /** Standard hard-disk. */
        DT_HARDDISK,    // 3
        /** Network (remote) drive. */
        DT_NETWORK,             // 4
        /** Some removable type (Zip-Drive, EZ-Drive, Jaz-Drive, etc.). */
        DT_REMOVABLE,   // 5
        /** RAM-Drive (virtual disk in memory). */
        DT_RAMDRIVE,             // 6
        /** RSJ attached drive */
        DT_RSJ             // 7
        };

extern char caDriveTypeShort[];

// device attributes
typedef struct
        {
        int drive;
        BOOL isAvailable;                   /** Nonzero if this drive is avalible. */
        enum DRIVE_TYPE type;               /** Type of the drive. */
        char volumeName[256];               /** Volume name of the drive. */
        char fsName[256];                   /** Name of the File System on the drive. */
        char szDisplayString[256];          // string displayed in WIN_DRIVES dialog
        unsigned int serialNo;              /** Serial Number of the drive. */
        unsigned int bytesPerCluster;       /** File Allocation Unit Size (512 for HPFS). */
        unsigned int bytesPerSector;        /** Number of bytes per sector. */
        unsigned int numSectorsPerUnit;     /** Number of sectors per allocation unit. */
        unsigned int numUnits;              /** Number of allocation units. */
        unsigned int numAvailUnits;         /** Number of free allocation units. */
        long long diskSize;                 /** bytesPerSector * numSectorsPerUnit * numUnits. */
        long long diskFree;                 /** bytesPerSector * numSectorsPerUnit * numAvailUnits. */
        } structDriveInfo;
extern structDriveInfo DriveInfo[27];

extern char DriveSelection[][MAX_LENGHT_SELECTION_ENTRIES];

extern const char sSeparationBar[MAX_LENGHT_SEPARATION_BAR + 1];

// Error IDs ----------------------------------------------------------------
#define ERROR_PMSEEK          65536
#define ERROR_INVPARMS      ERROR_PMSEEK
#define ERROR_LINETOOLONG   (ERROR_PMSEEK + 1)
#define ERROR_LOADPROFILE   (ERROR_PMSEEK + 2)
#define ERROR_LOADFILE      (ERROR_PMSEEK + 3)
#define ERROR_SAVEINI       (ERROR_PMSEEK + 4)

// prototypes ---------------------------------------------------------------

// commands.c
VOID cmdSearch(HWND hwnd);
VOID cmdStop(HWND hwnd);
VOID cmdSearchEnd(HWND hwnd, ULONG rc);
VOID cmdNew(VOID);
VOID cmdPrev(VOID);
VOID cmdNext(VOID);
VOID cmdMoveResultCtls(HWND hwnd, INT delta);
VOID cmdSetOption(HWND hwnd, ULONG id, ULONG flag);
VOID cmdSortOption(HWND hwnd, ULONG id, ULONG flag);
VOID cmdResetSearchControls(HWND hwnd);
VOID foundFileSelected(HWND hwnd, HWND hCtrl/*, BOOL bSelectFirst*/);
VOID historyEntrySelected(HWND hwnd, HWND hCtrl);
VOID cmdDeleteEntry(HWND hwnd);
VOID cmdDeleteAllEntries(HWND hwnd);
VOID cmdSelectedEdit(HWND hwnd, BOOL sameInstance);
VOID cmdSelectedOpen(HWND hwnd, ULONG flag);
VOID cmdSelectedPath(HWND hwnd, BOOL openDir);
VOID cmdSelectedCopyFilename(HWND hwnd);
VOID cmdSelectedCopyLine(HWND hwnd);
VOID cmdSelectedCopyAllFilenames(HWND hwnd);

// initEnd.c
BOOL initApplication(int argc, char** argv);
VOID endApplication(VOID);
VOID initMainDlg(HWND hwnd);
VOID saveState(HWND hwnd);
VOID freeResources(HWND hwnd);
HWND initHelp(HWND hwnd);
VOID endHelp(HWND hwnd, HWND hHlp);
VOID initDrivesDlg(HWND hwnd);
int  TranslateMenu(HWND hwndMenu);
LONG presParam(HWND hwnd, ULONG id, ULONG pp);

// layout.c
VOID arrangeControls(HWND hwnd, LONG cx, LONG cy);
VOID arrangeWinDrives(HWND hwnd);
VOID initSizes(HWND hwnd);
VOID getMinSize(HWND hwnd, ApiExPM_PSIZES psize);
MRESULT checkMinSize(HWND hwnd, MPARAM mp1, MPARAM mp2);
VOID minMaxFrame(HWND hwnd, BOOL flRestore);

// loadsave.c
BOOL loadFile(HWND hwnd, PSZ pszFile);
BOOL saveFilTxtHist (HWND hwnd, ULONG id, PPLLIST* ppl);

// objwin.c
BOOL startObjectThread(POBJECTWND pObjWnd, ULONG cbStack);
VOID /*_System*/ objWinThreadProc(/*POBJECTWND pObjWnd*/ void *arg);    // 20091118 AB changed to _beginthread

// pmaux.c
VOID updateOptionsMenu(HWND hwnd, ULONG ulOption);
VOID printFileDetails(PSZ buf, PFOUNDFILE pfile);
VOID refreshResult(HWND hwnd, BOOL bRefreshFileList/*, BOOL bSelect*/);
VOID srchResHistAddEntry(HWND hwnd, PSEARCHRES psr, BOOL bLoop);
VOID resetFilesFoundText(HWND hwnd);
VOID resetSrchFilTxtHist(HWND hCtrl, PPLLIST plist);
VOID freeResultHistory(HWND hwnd);

// pmutil.c
VOID printApplicationError(HWND hwnd, ULONG rc, PSZ pParm);
VOID printDosApiError(HWND hwnd, ULONG rc);
ULONG _msgBox(HWND hwnd, PSZ pszTitle, PSZ pszMsg, ULONG fl);
HMQ WinStdInit(PHAB phab);
VOID WinStdMsgLoop(HAB hab, PQMSG pqmsg);
VOID WinExtMsgLoop(HWND hwnd, HAB hab, PQMSG pqmsg);
VOID WinStdEnd(HWND hwnd, HMQ hmq, HAB hab);
ULONG DlgItemTextLoad(HWND hwnd, ULONG id, ULONG idStr);
STRING DlgItemTextToString(HWND hwnd, STRING str, ULONG id);
PVOID wMalloc(HWND hwnd, ULONG cb);
BOOL dLboxItemAddUnique(HWND hwnd, ULONG id, PSZ pszItem, ULONG flag);

// prodinfo.c
MRESULT EXPENTRY aboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// searchdata.c
PSEARCHRES srchResNew(HWND hwnd);
VOID srchResDel(PSEARCHRES p);
PSEARCHRES loadSavedResult(PPLLIST plr);
PPLLIST saveSrchResSingle(PSEARCHRES psr);
ULONG storeSrchCriteria(PSRCHCRITERIA psrchcr, PSAVESRCHRES pssr);
ULONG addFileItem(PFOUNDFILE pff, PSCANTREE pst);
ULONG textLineNew(PTEXTLINE ptl, PDOSEARCH pds);
ULONG errorInfoAdd(PFFTERROR pfftError, ULONG rc);

// settings.c
MRESULT EXPENTRY setsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// subclass.c
VOID subclassWindows(HWND hwnd);

// DriveInfo.c
int StartFillInfoThread(void);

// function wrappers --------------------------------------------------------
#define msgBox(hwnd, title, message, flag) \
   (_msgBox((hwnd), (PSZ)(title), (PSZ)(message), (flag)))

// --------------------------------------------------------------------------
      /*  Views for the wpOpen(), wpFilterMenu and wpModifyMenu methods */

#define OPEN_DEFAULT      0
#define OPEN_CONTENTS     1
#define OPEN_SETTINGS     2
#define OPEN_HELP         3
#define OPEN_RUNNING      4
#define OPEN_PROMPTDLG    5
#define OPEN_PALETTE      121
#define CLOSED_ICON       122
#define OPEN_USER          25856


#endif // #ifndef _PM_SEEK_H_

#define ICON_MAIN               1
#define ICON_PMSEEK             1

#define WIN_MAIN                        1
#define DLG_ABOUT                       90

#define DLG_SETTINGS          100
#define ICON_DOWN                       2
#define ICON_UP                         4


#define ST_ACDATASEEKER         100
#define ST_VERSION              101

#define ST_ALESSANDROFELICECANTATORE 104

#define TXT_FILESRCH            100
#define DD_FILESRCH             101
#define TXT_TEXTSRCH            102
#define DD_TEXTSRCH             103
#define TXT_SRCHRES             104
#define DD_SRCHRES              105
#define TXT_FILEFOUND           106
#define LBOX_FILEFOUND          107
#define TXT_TEXTFOUND           108
#define LBOX_TEXTFOUND          109
#define PB_SEARCH               110
#define PB_STOP                 111
#define PB_TEST                 112
#define EF_SEARCHING            113
#define PB_MOVEDOWN             114
#define PB_MOVEUP               115
#define PB_DRIVES               116
#define PB_FOC                  117

#define TXT_INFO                118
#define LAST_CONTROL            119

#define TXT_SEARCHING_OLD          1113

#define WIN_DRIVES              119
#define LB_DRIVES               120

#define IDM_FILE                        200
#define IDM_OPEN                        201
#define IDM_SAVE                        202
#define IDM_SAVEAS                      856
#define IDM_EXIT                        203

#define CB_FDATE                        201
#define CB_FTIME                        202
#define CB_FSIZE                        203
#define CB_LINENO                       204
#define CB_STRIPSPACES          205
#define CB_RECURSIVE            207
#define CB_ONLYTXTFILES         208
#define CB_CASEINS              209
#define CB_BOOL                 210
#define SPB_HISTORY             213

#define CB_DEFVIEW             200
#define CB_SHOWFDATE           201
#define CB_SHOWFTIME           202
#define CB_SHOWFSIZE           203
#define CB_SHOWLINENO          204
#define CB_SETUSEPREV          206
#define CB_SETRECUR            207
#define CB_SETTEXTONLY         208
#define CB_SETCASEINS          209
#define CB_SETBOOLOP           210
#define CB_SETDIRSEARCH        211

#define PB_SETEDIT                                              212
#define EF_SETEDIT                      217
#define EF_SETCLISHELL                          214
#define PB_SETCLISHELL                                  215
#define SPB_SETHISTORY                                  213
#define PB_SETHELP                                              216

#define IDM_SELECTED            400
#define IDM_EDITSEL             401
#define IDM_OPENSEL             402
#define IDM_PROPERTIES          403
#define IDM_COPYALLFNAMES       404
#define IDM_COPYFNAMES          405
#define IDM_OPENDIR             406
#define IDM_OPENCLI             407
#define IDM_COPYLINE            420
#define IDM_SEARCH              421

#define IDM_VFDATE              501
#define IDM_VFTIME              502
#define IDM_VFSIZE              503
#define IDM_VLINENO             504
#define IDM_VNOSPACES           505
#define IDM_VSORTNONE           506
#define IDM_VSORTASCEND         507
#define IDM_VSORTDESCEND        508
#define IDM_VIEWFILE            520
#define IDM_FILESORTMODE        521
#define IDM_TEXTDETAILS         522

#define IDM_SETTINGS            608

#define IDM_HELPIDX             701
#define IDM_PRODINFO            705

#define GB_DEFAULTVIEW          811
#define GB_DEFAULTSEARCHOPTIONS 812
#define GB_GENERALOPTIONS       813
#define SEPERATOR_LINE          814

#define BAR_LITE 803
#define BAR_DARK 802

#define IDM_HELP                        841

#define IDM_CUT                         851
#define IDM_COPY                        852
#define IDM_PASTE                       853
#define ST_ANDREASBUCHINGER     854
#define IDM_SEPARATOR           961




#define IC_DATASEEKER 965
#define IDM_VFSORTDESCEND 508
#define IDM_SEL_CPYNAMES 405
#define IDM_EDIT 830
#define IDM_VIEW 829
#define OPEN_UNKNOWN 849
#define HLP_SUBTABLE 861
#define HLP_MAIN 950
#define HLPS_MAIN 906
#define HLPS_MAIN_EXT 959
#define CB_SORTHISTORYENTRIES 962
#define IS_SEARCHING_2 1
#define IS_SRCHING 1
#define IC_ABOUT_ICON 964

#define LB_DRIVES_RECT 967
#define CB_LBOX_FIRST 1000
#define CB_LBOX1 1001
#define CB_LBOX2 1002
#define CB_LBOX3 1003
#define CB_LBOX4 1004
#define CB_LBOX5 1005
#define CB_LBOX6 1006
#define CB_LBOX7 1007
#define CB_LBOX8 1008
#define CB_LBOX9 1009
#define CB_LBOX10 1010
#define CB_LBOX11 1011
#define CB_LBOX12 1012
#define CB_LBOX13 1013
#define CB_LBOX14 1014
#define CB_LBOX15 1015
#define CB_LBOX16 1016
#define CB_LBOX17 1017
#define CB_LBOX18 1018
#define CB_LBOX19 1019
#define CB_LBOX20 1020
#define CB_LBOX21 1021
#define CB_LBOX22 1022
#define CB_LBOX23 1023
#define CB_LBOX24 1024
#define CB_LBOX25 1025
#define CB_LBOX26 1026
#define CB_LBOX27 1027
#define CB_LBOX28 1028
#define CB_LBOX29 1029
#define CB_LBOX30 1030
#define CB_LBOX31 1031
#define CB_LBOX_LAST 1032

#define GB_HISTORY  814
#define ST_COMMANDSHELL 702
#define ST_EDITFILESWITH 701
#define ST_MAXIMUMENTRIES 700
#define MLE_TRANSLATOR_INFO 1131

#define PU_MAIN             550
#define PU_IDM_DONOTSORT    551

#define PU_IDM_VFDATE       571
#define PU_IDM_VFTIME       572
#define PU_IDM_VFSIZE       573
#define PU_IDM_VLINENO      574
#define PU_IDM_VNOSPACES    575
#define PU_IDM_VSORTNONE    576
#define PU_IDM_VSORTASCEND  577
#define PU_IDM_VSORTDESCEND 578

#define PU_IDM_SETRECUR     580
#define PU_IDM_SETTEXTONLY  582
#define PU_IDM_SETCASEINS   583
#define PU_IDM_SETBOOLOP    584
#define PU_IDM_SETDIRSEARCH  585

#define PU_LBOX             590
#define PU_IDM_EDITSEL      591
#define PU_IDM_OPENSEL      592
#define PU_IDM_COPYFNAMES   595
#define PU_IDM_COPYLINE     598
#define PU_IDM_COPYALLFNAMES 594
#define PU_IDM_OPENDIR      596
#define PU_IDM_OPENCLI      597
#define PU_IDM_PROPERTIES   593
#define IDS_ELAPSEDTIME 6

#define DLG_FOC_FAIL 1141
#define MLE_FOC_FAIL 1146
#define ST_TESTTEXT 1145
