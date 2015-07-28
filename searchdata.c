/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// searchdata.c : procedures to handle allocations used to store search
//                results and error informations
// -
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------


// prototypes ---------------------------------------------------------------


// globals ------------------------------------------------------------------


//===========================================================================
// Allocate and initialize the SEARCHRES structure to store the data of a
// new search operation.
// Parameters --------------------------------------------------------------
// HWND hwnd : dialog window handle
// Return value ------------------------------------------------------------
// PSEARCHRES : address of the allocated data or NULL in case of error
//===========================================================================

PSEARCHRES srchResNew(HWND hwnd)
  {
  PSEARCHRES p;
  if ( NULL != (p = malloc(sizeof(SEARCHRES))) )
    {
    memset(p, 0, sizeof(SEARCHRES));
    // get the name of the file(s) to be searched
    p->fileSpec = DlgItemTextToString(hwnd, p->fileSpec, DD_FILESRCH);
    TRACE1("search string %s", String(p->fileSpec));
    if ( !p->fileSpec ) goto error;
    // store the text string(s) to be searched
    p->textSpec = DlgItemTextToString(hwnd, p->textSpec, DD_TEXTSRCH);
    p->option = g.setting;
    // create a packed linked list to store the found files
    if ( NULL == (p->result = PLListNew(0)) ) goto error;
    // create a packed linked list to store error information
    if ( NULL == (g.errors = PLListNew(0x1000)) ) goto error;
    } /* endif */
  return p;
  error :
  TRACE("error");
  srchResDel(p);
  return NULL;
  }


//===========================================================================
// Destroy a SEARCHRES object.
// Parameters --------------------------------------------------------------
// PSEARCHRES p : object to be destroyed
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID srchResDel(PSEARCHRES p)
  {
  if ( p )
    {
    StringDel(&p->fileSpec);
    StringDel(&p->textSpec);
    PLListDel(p->result);
    free(p);
    } /* endif */
  }


//===========================================================================
// Convert search result data from the PPLLIST used to save it to disk to
// a SEARCHRES structure.
// Parameters --------------------------------------------------------------
// PPLLIST plr : packed linked list containing the search result data.
// Return value ------------------------------------------------------------
// PSEARCHRES : data converted in SEARCHRES structure format.
//              NULL in case of error.
//===========================================================================

PSEARCHRES loadSavedResult(PPLLIST plr)
  {
  PSEARCHRES psr;
  ITERITEM iterItem;
  if ( NULL != (psr = malloc(sizeof(SEARCHRES))) )
    {
    memset(psr, 0, sizeof(SEARCHRES));
    iterItem.hNode = 0;
    // search option
    if ( !PLListItemFirst(plr, &iterItem) ) goto error;
    psr->option = *((PULONG)iterItem.pData);
    // searched file(s)
    if ( !PLListItemNext(plr, &iterItem) ||
         !(psr->fileSpec = StringNew((PSZ)iterItem.pData, -1)) )
      goto error;
    // searched text string(s)
    if ( !PLListItemNext(plr, &iterItem) ||
         !(psr->textSpec = StringNew((PSZ)iterItem.pData, -1)) )
      goto error;
    // search result (files found and text lines)
    if ( !PLListItemNext(plr, &iterItem) ||
         !(psr->result = PLListExport(plr, PLLHITEM(plr, iterItem.pNext),
                                      FALSE, 0)) )
      goto error;
    } /* endif */
  return psr;
  error:
  srchResDel(psr);
  return NULL;
  }


//===========================================================================
// Callback procedure to add a file data to the list of found files.
// Parameters --------------------------------------------------------------
// PFOUNDFILE pff : address of storage where to store file data
// PSCANTREE pst  : current file data.
// Return value ------------------------------------------------------------
// ULONG : size of needed allocation when 'pff' is NULL.
//===========================================================================

ULONG addFileItem(PFOUNDFILE pff, PSCANTREE pst)
  {
  if ( !pff )
    return sizeof(FOUNDFILE) + pst->cbPath + pst->ff.cchName - 3;
  pff->date = pst->ff.fdateLastWrite;
  pff->time = pst->ff.ftimeLastWrite;
  pff->cb = pst->ff.cbFile;
  TRACE2("'%.20s', len: %d", pst->achPath, pst->cbPath);
  memmove(pff->name, &(pst->achPath), pst->cbPath);
  memcpy(pff->name + pst->cbPath, pst->ff.achName, pst->ff.cchName + 1);
  return 0;
  }


//===========================================================================
// Callback procedure to store a text line containing the searched text
// as a packed linked list item.
// Parameters --------------------------------------------------------------
// PTEXTLINE ptl : address of storage reserved for line data.
// PDOSEARCH pds : current line data.
// Return value ------------------------------------------------------------
// ULONG : when 'ptl' is NULL this is the amount of the needed storage.
//===========================================================================

ULONG textLineNew(PTEXTLINE ptl, PDOSEARCH pds)
  {
  if ( !ptl ) return pds->cbLine + 11;
  ptl->lineno = pds->linecount;
  sprintf(ptl->line, "%6d %s", pds->linecount + 1, pds->pLine);
  //   memcpy(ptl->line, pds->pLine, pds->cbLine);
  return 0;
  }


//===========================================================================
// Callback procedure to store error informations (Dos* API error code and
// file name) as a packed linked list item.
// Parameters --------------------------------------------------------------
// PFFTERROR pfftError : error data structure
// ULONG rc            : Dos* API error code
// Return value ------------------------------------------------------------
// LONG : return value (0 to continue the search, -1 to stop it)
//===========================================================================

ULONG errorInfoAdd(PFFTERROR pfftError, ULONG rc)
  {
  if ( !pfftError ) return strlen(g.pSrchBuf->fileData) + 5;
  pfftError->rc = rc;
  strcpy(pfftError->file, g.pSrchBuf->fileData);
  return 0;
  }
