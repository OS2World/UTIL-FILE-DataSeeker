/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// .h :
//
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _FILEUTIL_H_
   #define _FILEUTIL_H_

typedef struct _SCANTREE SCANTREE;
typedef SCANTREE * PSCANTREE;

typedef LONG (SCANTREEFN)(PSCANTREE);
typedef SCANTREEFN * PSCANTREEFN;
#pragma pack(2)
struct _SCANTREE {
   PVOID pUserData;         // user data pointer
   FILEFINDBUF3L ff;         // found file data     // 20090615 LFS
   CHAR achPath[CCHMAXPATH];       // file specific
   CHAR achMask[CCHMAXPATHCOMP];   // search mask
   USHORT flag;             // search flag (DosFindFirst() flags | SCANTREE_*)
   // private readonly members : do not modify these
   USHORT cbPath;           // path length including the last slash
   HEV hev;                 // event semaphore to break the loop
   PSCANTREEFN pfFile;      // procedure called for each matching file/directory
   // these are used only if the recursion flag (SCANTREE_RECUR) is set
   PSCANTREEFN pfDirPre;    // procedure called for each directory
   PSCANTREEFN pfDirPost;   // procedure called after a directory has been
                            // processed
} ;
#pragma pack()

// RECURSIVE FLAG:
#define SCANTREE_RECUR       0x10000000  // subdirectory recursion
#define SCANTREE_STOP        0x00008000  // stop execution
#define SCANTREE_ATTRMASK    0x00000027

// file open error
#define FMO_ERROR            ((HFILE)-1)
// creating flags
#define FMO_FAILIFNEW        0
#define FMO_CREATE           OPEN_ACTION_CREATE_IF_NEW
// mutually exclusive
#define FMO_NOTIFEXIST       0
#define FMO_IFEXIST          OPEN_ACTION_OPEN_IF_EXISTS
#define FMO_REPLACE          OPEN_ACTION_REPLACE_IF_EXISTS
// sharing flags (mutually exclusive)
#define FMO_SHARENONE        (OPEN_SHARE_DENYREADWRITE << 16)
#define FMO_SHAREREAD        (OPEN_SHARE_DENYWRITE << 16)
#define FMO_SHAREWRITE       (OPEN_SHARE_DENYREAD << 16)
#define FMO_SHAREALL         (OPEN_SHARE_DENYNONE << 16)
// access flags (mutually exclusive)
#define FMO_READONLY         (OPEN_ACCESS_READONLY << 16)
#define FMO_WRITEONLY        (OPEN_ACCESS_WRITEONLY << 16)
#define FMO_READWRITE        (OPEN_ACCESS_READWRITE << 16)

#define FMO_APPEND           0x80000000

// macros -------------------------------------------------------------------
#define fmIsNotCurDir(pszDir) \
((*((PULONG)(pszDir)) & 0x0000ffff) ^ 0x0000002e)

#define fmIsNotParDir(pszDir) \
((*((PULONG)(pszDir)) & 0x00ffffff) ^ 0x00002e2e)

// prototypes ---------------------------------------------------------------
BOOL fmExists(PSZ pszFile);
LONG fmTreeScan(PSZ pszFileSpec, ULONG flag, PVOID pUserData, HEV hev,
                PSCANTREEFN pfFile, PSCANTREEFN pfDirPre, PSCANTREEFN pfDirPost);
HFILE fmOpen(PSZ pszFile, ULONG flag);
BOOL fmClose(HFILE hf);
BOOL fmPtrSet(HFILE hf, LONG offset, ULONG flag);
BOOL fmRead(HFILE hf, PVOID pBuffer, ULONG cb);
BOOL fmWrite(HFILE hf, PVOID pBuffer, ULONG cb);
PSZ fmFileNameFromPath(PSZ pszFullFileName);
VOID fmFileNameEditExt(PSZ pszFileName, PSZ pszExt);
BOOL fmFileNameComp(PSZ pszPath, PSZ pszName);

#endif // #ifndef _FILEUTIL_H_
