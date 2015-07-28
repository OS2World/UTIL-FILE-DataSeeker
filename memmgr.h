/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// memmgr.h : memory allocation procedures
//
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _MEMMGR_H_
   #define _MEMMGR_H_


// allocate 64 MB of uncommitted memory
// commits-decommits the memory as needed in 64 KB blocks

#define CB_HEAPTOTAL      0x08000000      // total (maximum) heap size
#define CB_HEAPBLOCK      0x00040000      // heap increase/decrease step
#define CB_SUBSETOVHD     0x00000040      // heap suballocation overhead

// prototypes ---------------------------------------------------------------
BOOL MemHeapInit(VOID);
BOOL MemHeapTerm(VOID);
PVOID MemAlloc(ULONG cb);
PVOID MemRealloc(PVOID p, ULONG cb);
VOID MemFree(PVOID pv);
VOID MemHeapMin(VOID);
PSZ MemStrDup(PSZ pszIn);
PVOID MemDup(PVOID pData, ULONG cbData);
#ifdef DEBUGMEM
PVOID dbgMalloc(PSZ pszFile, ULONG line, PSZ pszCall, ULONG cb);
PVOID dbgRealloc(PSZ pszFile, ULONG line, PSZ pszCall, PVOID p, ULONG cb);
VOID dbgFree(PSZ pszFile, ULONG line, PSZ pszCall, PVOID p);
VOID dbgHeapMin(PSZ pszFile, ULONG line, PSZ pszCall);
PVOID dbgStrDup(PSZ pszFile, ULONG line, PSZ pszCall, PSZ psz);

#endif
#endif // #ifndef _MEMMGR_H_
