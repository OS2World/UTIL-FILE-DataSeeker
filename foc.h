/*
 * Copyright (C) 2009-2010 eCo Software
 * Copyright (C) 2009-2010 Dmitry A.Steklenev
 */

#ifndef FOC_H
#define FOC_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

/* FOC Window classes */

#define WC_DIRTREE  "WC_DIRTREE"
#define WC_FILEVIEW "WC_FILEVIEW"
#define WC_PREVIEW  "WC_PREVIEW"

/* IDs of directory tree and file view containers */

#define FID_FILEVIEW        0x7FFF
#define FID_DIRTREE         0x7FFF

/* Directory Tree control styles */

#define DTS_TITLE           0x0020
#define DTS_READONLY        0x0040
#define DTS_BORDER          0x0080
#define DTS_SHOWALL         0x0100
#define DTS_CHECKED         0x0200
#define DTS_TREE            0x0400

/* Directory Tree notification messages */

#define DTN_CHDIR           0x0011
#define DTN_CONTEXTMENU     0x0012
#define DTN_DELETED         0x0013
#define DTN_NEW             0x0014
#define DTN_RENAMED         0x0015
#define DTN_HELP            0x0016
#define DTN_SETFOCUS        0x0017
#define DTN_KILLFOCUS       0x0018
#define DTN_PLACED          0x0019
#define DTN_CHECK           0x001A
#define DTN_UNCHECK         0x001B

/* Directory Tree messages */

#define DTM_REFRESH         0x0180
#define DTM_QUERYCURDIR     0x0181
#define DTM_CHDIR           0x0182
#define DTM_REQUESTNEWDIR   0x0183
#define DTM_REQUESTDELETE   0x0184
#define DTM_REQUESTEDIT     0x0185
#define DTM_SETFILEVIEW     0x0186
#define DTM_SETTREESTYLE    0x0187
#define DTM_QUERYCHECKED    0x0188
#define DTM_FREEFILELIST    0x0189
#define DTM_PLACECHECKS     0x0190

/* Data structure for DTN_RENAMED */

typedef struct _DIRRENAMEINFO
{
  ULONG    cb;                        /* Structure size. */
  CHAR     szOldDirName[CCHMAXPATH];  /* Old ASCIIZ directory path name. */
  CHAR     szNewDirName[CCHMAXPATH];  /* New ASCIIZ directory path name. */

} DIRRENAMEINFO;

typedef DIRRENAMEINFO* PDIRRENAMEINFO;

/* File View control styles */

#define FVS_MULTIPLESEL     0x0001    /* LS_MULTIPLESEL */
#define FVS_EXTENDSEL       0x0010    /* LS_EXTENDEDSEL */
#define FVS_SINGLESEL       0x0020
#define FVS_TITLE           0x0040
#define FVS_READONLY        0x0080
#define FVS_SHOWDOTS        0x0100
#define FVS_DETAIL          0x0200
#define FVS_ICON            0x0400
#define FVS_COLUMNS         0x0600
#define FVS_BORDER          0x0800
#define FVS_SHOWDIRS        0x1000
#define FVS_SHOWALL         0x2000
#define FVS_CHECKED         0x4000
#define FVS_VIEWSTYLE       0x4600
#define FVS_SELECTIONSTYLE  0x0031

/* File View notification messages */

#define FVN_CHDIR           0x0011
#define FVN_ENTER           0x0012
#define FVN_CONTEXTMENU     0x0013
#define FVN_DELETED         0x0014
#define FVN_NEW             0x0015
#define FVN_RENAMED         0x0016
#define FVN_SELECT          0x0017
#define FVN_DESELECT        0x0018
#define FVN_FILTER          0x0019
#define FVN_CHECK           0x001A
#define FVN_UNCHECK         0x001B
#define FVN_HELP            0x0020
#define FVN_CURSOR          0x0021
#define FVN_SETFOCUS        0x0022
#define FVN_KILLFOCUS       0x0023
#define FVN_PLACED          0x0024

/* File View messages */

#define FVM_REFRESH         0x0180
#define FVM_QUERYCURDIR     0x0181
#define FVM_QUERYCURSORED   0x0182
#define FVM_QUERYSELECTED   0x0183
#define FVM_FREEFILELIST    0x0184
#define FVM_CHDIR           0x0185
#define FVM_REQUESTNEWDIR   0x0186
#define FVM_REQUESTDELETE   0x0187
#define FVM_REQUESTEDIT     0x0188
#define FVM_SETVIEWSTYLE    0x0189
#define FVM_SORT            0x018A
#define FVM_QUERYSPLITBAR   0x018B
#define FVM_SETSPLITBAR     0x018C
#define FVM_SETDIRTREE      0x018D
#define FVM_QUERYSORT       0x018E
#define FVM_REQUESTSELECT   0x018F
#define FVM_SETPREVIEW      0x0190
#define FVM_QUERYCHECKED    0x0191
#define FVM_PLACECHECKS     0x0192

/* File View messages parameter flags */

#define FVA_NONE            0x00000001
#define FVA_NAME            0x00000002
#define FVA_SIZE            0x00000004
#define FVA_TIME            0x00000008
#define FVA_FILE            0x00000010
#define FVA_DIRECTORY       0x00000020
#define FVA_DESCEND         0x80000000

/* Data structure for FVN_RENAMED */

typedef struct _FILERENAMEINFO
{
  ULONG    cb;                          /* Structure size. */
  CHAR     szOldFileName[CCHMAXPATH];   /* Old ASCIIZ file name. */
  CHAR     szNewFileName[CCHMAXPATH];   /* New ASCIIZ file name. */

} FILERENAMEINFO;

typedef FILERENAMEINFO* PFILERENAMEINFO;

/* Data structure for FVM_QUERYSELECTED, FVM_QUERYCHECKED,
   FVM_FREEFILELIST and FVM_REQUESTDELETE */

typedef struct _FILELIST
{
  ULONG    cb;                          /* Structure size. */
  ULONG    ulFQFCount;                  /* Number of file names. */
  PAPSZ    papszFQFilename;             /* Pointer to a table of pointers to fully-qualified
                                           file names. */
} FILELIST;

typedef FILELIST* PFILELIST;

/* Preview control styles */

#define PVS_BORDER          0x0001
#define PVS_HORIZONTAL      0x0002
#define PVS_VERTICAL        0x0004

/* Preview messages */

#define PVM_PREVIEWFILE     0x0180
#define PVM_CLEAR           0x0181
#define PVM_QUERYTHUMBSIZE  0x0182

/* FOC application data structures */

typedef struct _FOCFILEDLG
{
   ULONG   cbSize;                  /* Size of FOCFILEDLG structure.       */
   ULONG   fl;                      /* FDS_ flags. Alter behavior of dlg.  */
   ULONG   ulUser;                  /* User defined field.                 */
   LONG    lReturn;                 /* Result code from dialog dismissal.  */
   LONG    lSRC;                    /* System return code.                 */
   PSZ     pszTitle;                /* String to display in title bar.     */
   PSZ     pszOKButton;             /* String to display in OK button.     */
   PFNWP   pfnDlgProc;              /* Entry point to custom dialog proc.  */
   PSZ     pszIType;                /* Pointer to string containing        */
                                    /* initial EA type filter. Type        */
                                    /* does not have to exist in list.     */
   PAPSZ   papszITypeList;          /* Pointer to table of pointers that   */
                                    /* point to null terminated Type       */
                                    /* strings. End of table is marked     */
                                    /* by a NULL pointer.                  */
   PSZ     pszIDrive;               /* Pointer to string containing        */
                                    /* initial drive. Drive does not       */
                                    /* have to exist in drive list.        */
   PAPSZ   papszIDriveList;         /* Pointer to table of pointers that   */
                                    /* point to null terminated Drive      */
                                    /* strings. End of table is marked     */
                                    /* by a NULL pointer.                  */
   HMODULE hMod;                    /* Custom FOC template.                */
   CHAR    szFullFile[CCHMAXPATH];  /* Initial or selected fully           */
                                    /* qualified path and file.            */
   PAPSZ   papszFQFilename;         /* Pointer to table of pointers that   */
                                    /* point to null terminated FQFname    */
                                    /* strings. End of table is marked     */
                                    /* by a NULL pointer.                  */
   ULONG   ulFQFCount;              /* Number of files selected            */
   USHORT  usDlgId;                 /* Custom dialog id.                   */
   SHORT   x;                       /* X coordinate of the dialog          */
   SHORT   y;                       /* Y coordinate of the dialog          */
   SHORT   sEAType;                 /* Selected file's EA Type.            */
   struct  _DAXOPENFILE* pDaxData;  /* Reserved value, should be 0.        */
   SHORT   sReserved;               /* Reserved value, should be 0.        */

} FOCFILEDLG;

typedef FOCFILEDLG* PFOCFILEDLG;

typedef struct _FOCSELDIR
{
   ULONG   cbSize;                  /* Size of FOCSELDIR structure.        */
   ULONG   fl;                      /* FDS_ flags. Alter behavior of dlg.  */
   ULONG   ulUser;                  /* User defined field.                 */
   LONG    lReturn;                 /* Result code from dialog dismissal.  */
   LONG    lSRC;                    /* System return code.                 */
   PSZ     pszTitle;                /* String to display in title bar.     */
   PSZ     pszOKButton;             /* String to display in OK button.     */
   PFNWP   pfnDlgProc;              /* Entry point to custom dialog proc.  */
   HMODULE hMod;                    /* Custom FOC template.                */
   CHAR    szFullDir[CCHMAXPATH];   /* Initial directory name.             */
   USHORT  usDlgId;                 /* Custom dialog id.                   */
   SHORT   x;                       /* X coordinate of the dialog          */
   SHORT   y;                       /* Y coordinate of the dialog          */
   SHORT   sReserved;               /* Reserved value, should be 0.        */

} FOCSELDIR;

typedef FOCSELDIR* PFOCSELDIR;

/* FOCFILEINFO flags. */

#define FIS_HBITMAP     0x00000001UL
#define FIS_HBITMAP2    0x00000002UL
#define FIS_HPOINTER    0x00000004UL
#define FIS_XML         0x00080000UL

typedef struct _FOCFILEINFO
{
   ULONG   cbSize;                  /* Size of FOCFILEINFO structure.      */
   ULONG   fl;                      /* FIS_ flags. Specifies type of the   */
                                    /* returned information.               */
   LHANDLE hImage;                  /* Handle of the thumbnail image.      */
   PSZ     pszInfo;                 /* Pointer to the information string.  */

} FOCFILEINFO;

typedef FOCFILEINFO* PFOCFILEINFO;

/* FOC dialogs and control ids */

#define   FOCID_FILE_DIALOG     DID_FILE_DIALOG
#define   FOCID_SELECT_DIR      300
#define   FOCID_TOOLBAR         301
#define   FOCID_FILENAME_TXT    DID_FILENAME_TXT
#define   FOCID_FILENAME_ED     DID_FILENAME_ED
#define   FOCID_FILTER_TXT      DID_FILTER_TXT
#define   FOCID_FILTER_CB       DID_FILTER_CB
#define   FOCID_SPLITTER        302
#define   FOCID_DIRECTORY       DID_DIRECTORY_LB
#define   FOCID_FILES           DID_FILES_LB
#define   FOCID_HELP_PB         DID_HELP_PB
#define   FOCID_APPLY_PB        DID_APPLY_PB
#define   FOCID_OK_PB           DID_OK_PB
#define   FOCID_CANCEL_PB       DID_CANCEL_PB
#define   FOCID_PREVIEW         304

/* FOC toolbar ids. */

#define   FOCID_DELETE          0x7FFF
#define   FOCID_DETAILVIEW      0x7FFE
#define   FOCID_ICONVIEW        0x7FFD
#define   FOCID_NEWFOLDER       0x7FFC
#define   FOCID_REFRESH         0x7FFB
#define   FOCID_RENAME          0x7FFA
#define   FOCID_SORT_NAME       0x7FF9
#define   FOCID_SORT_SIZE       0x7FF8
#define   FOCID_SORT_TIME       0x7FF7
#define   FOCID_SHOW_FOLDERS    0x7FF6
#define   FOCID_SHOW_HIDDEN     0x7FF5
#define   FOCID_SHOW_PREVIEW    0x7FF4
#define   FOCID_VIEW            0x7FF3
#define   FOCID_SORT            0x7FF2
#define   FOCID_SHOW            0x7FF1
#define   FOCID_COPY            0x7FF0
#define   FOCID_COPY_FILENAME   0x7FEF
#define   FOCID_COPY_PATHNAME   0x7FEE
#define   FOCID_CLEAR_LASTDIRS  0x7F9F
#define   FOCID_LASTDIRS        0x7FA0
#define   FOCID_CLEAR_LASTFILES 0x7FBF
#define   FOCID_LASTFILES       0x7FC0
#define   FOCID_FIRST           0x7F9F
#define   FOCID_LAST            0x7FFF

/* FOC messages. */

#define   FDM_MENUEND          (WM_USER+43)

#pragma pack()

HWND    EXPENTRY FOCFileDlg( HWND hwndP, HWND hwndO, PFOCFILEDLG pfocd );
MRESULT EXPENTRY FOCDefFileDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL    EXPENTRY FOCFreeFileDlgList( PAPSZ papszFQFilename );
HWND    EXPENTRY FOCSelectDir( HWND hwndP, HWND hwndO, PFOCSELDIR pfocd );
MRESULT EXPENTRY FOCDefSelectDirProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
PSZ     EXPENTRY FOCPathRelToAbs( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
PSZ     EXPENTRY FOCPathAbsToRel( PCSZ pszBaseName, PCSZ pszPathName, PSZ pszResult, ULONG ulSize );
APIRET  EXPENTRY FOCDeleteDirTree( PCSZ pszDirName, PSZ pszFailName, ULONG cbFailName );
APIRET  EXPENTRY FOCLoadFileInfo( PCSZ pszFileName, PFOCFILEINFO pInfo, ULONG );
BOOL    EXPENTRY FOCFreeFileInfo( PFOCFILEINFO pInfo );
BOOL    EXPENTRY FOCInitialize( void );

/* Preview plugins interface. */

APIRET  EXPENTRY eLoadFileInfo( PCSZ pszFileName, PFOCFILEINFO pInfo, LONG lSize, ULONG );
BOOL    EXPENTRY eFreeFileInfo( PFOCFILEINFO pInfo );

#ifdef __cplusplus
}
#endif
#endif /* FOC_H */
