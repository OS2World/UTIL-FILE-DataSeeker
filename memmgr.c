/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// memmgr.c : memory allocation procedures
// -
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


//#define DEBUGMEM

//#ifdef __WATCOMC__        // 20080xxxx AB added for trying Watcom but changed back to IBMCPP3.65
//  #include <WatcomWrapper.h>
//  #pragma aux _ucreate "_*"
//#endif

#define INCL_DOSPROCESS
#define INCL_DOSMISC

#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include <umalloc.h>
#include <builtin.h>        // IBM V.age fast RAM semaphores

#include "memmgr.h"
#include "ApiExBase.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*#define INCL_DOSPROCESS
// 200808xx AB we need DOSMISC, can't remember why anymore ...
#define INCL_DOSMISC
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <umalloc.h>
//#include <builtin.h>        // IBM V.age fast RAM semaphores
#include "memmgr.h"
#include "ApiExBase.h" */
// 200808xx AB add trace definitions for Dennis Bareis pmprintf untility
#ifdef __DEBUG_TRACE__
  #include "pmprintf.h"
  #include <STDARG.H>
  #define  __PMPRINTF__
//#define TRACE( ... )   PmpfF ( ( __VA_ARGS__ ) )        // doesn't work for VAC3.65 and ICCv4

  #define TRACE(text)                                     PmpfF((text))
  #define TRACE1(text, param1)                            PmpfF((text, param1))
  #define TRACE2(text, param1, param2)                    PmpfF((text, param1, param2))
  #define TRACE3(text, param1, param2, param3)    PmpfF((text, param1, param2, param3))
  #define TRACE4(text, param1, param2, param3, param4)    PmpfF((text, param1, param2, param3, param4))
#else
  #define TRACE(text)
  #define TRACE1(text, param1)
  #define TRACE2(text, param1, param2)
  #define TRACE3(text, param1, param2, param3)
  #define TRACE4(text, param1, param2, param3, param4)
#endif  // __DEBUG_TRACE__
// end trace definitions
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//#include "pmseek.h"

/* ToDo ---------------------------------------------------------------------
 * allocazione iniziale DosAllocMem() 16 o 32 MB eventualmente high memory
 * quando DosSubAllocMem() restituisce ERROR_DOSSUB_NOMEM allocare altro
   blocco di 16 or 32 MB (o maggiore se richiesto)
 * e' necessario mantenere lista blocchi allocati chiamando DosFreeMem()
   quando tutti i sottoblocchi sono stati liberati
 * usare altro tipo di semaforo (basato su CMPXCHNG)
 */

// definitions --------------------------------------------------------------

// RAM semaphore lock/unlock flags
#define HEAP_LOCK    1
#define HEAP_UNLOCK  0


// prototypes ---------------------------------------------------------------
static void* heapIncrease(Heap_t uhp, size_t* plen, int* pfl);
static void heapDecrease(Heap_t uhp, PVOID p, size_t size);


// globals ------------------------------------------------------------------
static PVOID pbasemem;      // address of memory used for the HEAP creation
static volatile INT isem;   // used by the RAM semaphore
static Heap_t hp = NULL;    // HEAP handle
#ifdef DEBUGMEM
  #include <stdio.h>
static HFILE hf;

VOID openLogFile(VOID)
  {
  ULONG ul;
  xDosOpen("dataseek.log", &hf, &ul, 0, 0,
           OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
           OPEN_SHARE_DENYNONE | OPEN_ACCESS_WRITEONLY, NULL);
  }

VOID closeLogFile(VOID)
  {
  DosClose(hf);
  }


VOID logwrite(PSZ msg, ULONG address, ULONG cb)
  {
  CHAR buf[80];
  cb = sprintf(buf, msg, address, cb);
  DosWrite(hf, buf, cb, &cb);
  }

VOID logfunction(PSZ pszFile, ULONG line, PSZ pszCall,
                 PSZ memfunc, PVOID address, ULONG cb)
  {
  CHAR buf[256];
  cb = sprintf(buf, "calling %s() from %s->[%d]%s() address: %08p size %u\n",
               memfunc, pszFile, line, pszCall, address, cb);
  DosWrite(hf, buf, cb, &cb);
  }

PVOID dbgMalloc(PSZ pszFile, ULONG line, PSZ pszCall, ULONG cb)
  {
  PVOID p = MemAlloc(cb);
  logfunction(pszFile, line, pszCall, "malloc", p, cb);
  return p;
  }

PVOID dbgRealloc(PSZ pszFile, ULONG line, PSZ pszCall, PVOID p, ULONG cb)
  {
  p = MemRealloc(p, cb);
  logfunction(pszFile, line, pszCall, "realloc", p, cb);
  return p;
  }
VOID dbgFree(PSZ pszFile, ULONG line, PSZ pszCall, PVOID p)
  {
  free(p);
  logfunction(pszFile, line, pszCall, "free", p, 0);
  }

VOID dbgHeapMin(PSZ pszFile, ULONG line, PSZ pszCall)
  {
  MemHeapMin();
  logfunction(pszFile, line, pszCall, "_heapmin", NULL, 0);

  }
PVOID dbgStrDup(PSZ pszFile, ULONG line, PSZ pszCall, PSZ psz)
  {
  PVOID p = MemStrDup(psz);
  logfunction(pszFile, line, pszCall, "strdup", p, 0);
  return p;
  }


#else
  #define openLogFile()
  #define closeLogFile()
  #define logwrite(msg, address, cb)
#endif

//===========================================================================
// MemHeapInit : HEAP initialization procedure:
//            reserves initial storage via DosAllocMem, creates and opens
//            the user HEAP
// ToDo : check if _upool() may improve the performance
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

BOOL MemHeapInit(VOID)
  {
  PVOID pheap;
  if ( DosAllocMem(&pbasemem, CB_HEAPTOTAL, PAG_READ | PAG_WRITE) ||
       DosSubSetMem(pbasemem, DOSSUB_INIT | DOSSUB_SPARSE_OBJ, CB_HEAPTOTAL) ||
       DosSubAllocMem(pbasemem, &pheap, CB_HEAPBLOCK - CB_SUBSETOVHD) ||
       (NULL == (hp = _ucreate(pheap, CB_HEAPBLOCK - CB_SUBSETOVHD,
                               !_BLOCK_CLEAN, _HEAP_REGULAR,
                               heapIncrease, heapDecrease))) ||
       _uopen(hp) )
    return FALSE;
#ifdef DEBUGMEM
  openLogFile();
#endif
  return TRUE;
  }


//===========================================================================
// MemHeapTerm : HEAP termination procedure:
//            closes and destroys the user HEAP, then frees the memory
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

BOOL MemHeapTerm(VOID)
  {
  if ( hp )
    {
    return !(_uclose(hp) ||
             _udestroy(hp, _FORCE) ||
             DosSubUnsetMem(pbasemem) ||
             DosFreeMem(pbasemem));
    } /* endif */
  closeLogFile();
  return FALSE;
  }


//===========================================================================
// heapIncrease : increases the HEAP size as needed
// Parameters --------------------------------------------------------------
// Heap_t uhp : user heap handle
// size_t* plen : size to grow
// int* pfl : heap initialization flag. When set to _BLOCK_CLEAN means that
//            the heap has been initialized to 0
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

static void* heapIncrease(Heap_t uhp, size_t* plen, int* pfl)
  {
  PVOID p = NULL;
  *plen = (size_t)RNDUP(*plen, CB_HEAPBLOCK);
  *pfl = !_BLOCK_CLEAN;
  logwrite("increaseHeap  -                    size : %8u\n", *plen, 0);
  if ( DosSubAllocMem(pbasemem, &p, *plen) )
    return NULL;
  return p;
  }


//===========================================================================
// heapDecrease : decreases the HEAP size returning memory to the system
// Parameters --------------------------------------------------------------
// Heap_t uhp : user heap handle
// PVOID p : address of the memory block to be freed
// size_t size : unused
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

static void heapDecrease(Heap_t uhp, PVOID p, size_t size)
  {
  logwrite("decreaseHeap  -                    size : %8u\n", size, 0);
  DosSubFreeMem(pbasemem, p, size);
  }


//===========================================================================
// heapLock : suspends the current thread, giving CPU time to other threads,
//            until the HEAP access semaphore is unlocked by the thread
//            currently owning it.
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

_Inline VOID heapLock(VOID)
  {
  while ( __lxchg(&isem, HEAP_LOCK) ) DosSleep(1);
  }


//===========================================================================
// heapUnlock : unlocks the integer used to serialize the HEAP access
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

_Inline VOID heapUnlock(VOID)
  {
  __lxchg(&isem, HEAP_UNLOCK);
  }


//===========================================================================
// MemAlloc : multithread memory allocation function (works as malloc)
//            it ensures that the access tho HEAP is properly serialized
// Parameters --------------------------------------------------------------
// ULONG cb : count of bytes of memory to be allocated
// Return value ------------------------------------------------------------
// PVOID : address of allocated memory or NULL in case of error
//===========================================================================

PVOID MemAlloc(ULONG cb)
  {
  PVOID pv = NULL;
  if ( !cb ) return NULL;
  // if the RAM semaphore is locked gives CPU time to the other threads
  // so they can finish their job and release the semaphore
  heapLock();
  pv = _umalloc(hp, cb);
  heapUnlock();
  logwrite("malloc        - address : %08x size : %8u\n", (ULONG)pv, cb);
  TRACE2("malloc        - address : %08x size : %8u", (ULONG)pv, cb);
  return pv;
  }


//===========================================================================
// xpRealloc : exported multithread memory allocation function (works as realloc)
//            it ensures that the access tho HEAP is properly serialized
// Parameters --------------------------------------------------------------
// ULONG cb : count of bytes of memory to be allocated
// Return value ------------------------------------------------------------
// PVOID : address of allocated memory or NULL in case of error
//===========================================================================

PVOID MemRealloc(PVOID p, ULONG cb)
  {
  // if the RAM semaphore is locked gives CPU time to the other threads
  // so they can finish their job and release the semaphore
  logwrite("realloc start - address : %08x size : %8u\n", (ULONG)p, cb);
  heapLock();
  p = p? realloc(p, cb) : _umalloc(hp, cb);
  heapUnlock();
  logwrite("realloc end   - address : %08x size : %8u\n", (ULONG)p, cb);
  return p;
  }


//===========================================================================
// _mtfree : multithread memory free procedure (works as free)
// Parameters --------------------------------------------------------------
// PVOID pv: memory address to be freed
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID MemFree(PVOID pv)
  {
  if ( pv )
    {
    heapLock();
    free(pv);
    heapUnlock();
    logwrite("free          - address : %08x\n", (ULONG) pv, 0);
    } /* endif */
  }


//===========================================================================
// _mtheapmin : returns unused HEAP chunks to the system (serialized access)
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID MemHeapMin(VOID)
  {
  heapLock();
  _uheapmin(hp);
  heapUnlock();
  }


//===========================================================================
// Thread safe version of strdup.
// Parameters --------------------------------------------------------------
// PSZ pszIn : text string to be duplicated.
// Return value ------------------------------------------------------------
// PSZ : address of the duplicated text string or NULL in case of error.
//===========================================================================

PSZ MemStrDup(PSZ pszIn)
  {
  ULONG cb = strlen(pszIn) + 1;
  PSZ pszOut = MemAlloc(cb);
  if ( pszOut ) memcpy(pszOut, pszIn, cb);
  return pszOut;
  }


//===========================================================================
// Duplicate a buffer of binary data.
// Parameters --------------------------------------------------------------
// PVOID pData  : data to be duplicated.
// ULONG cbData : sizeof data to be duplicated.
// Return value ------------------------------------------------------------
// PVOID : address of duplicated data or NULL in case of error.
//===========================================================================

PVOID MemDup(PVOID pData, ULONG cbData)
  {
  PVOID pnew;
  if ( !pData || !cbData ) return NULL;
  if ( NULL != (pnew = MemAlloc(cbData)) )
    memcpy(pnew, pData, cbData);
  return pnew;
  }
