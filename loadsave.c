/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// loadsave.c :
// data definitions used to save/restore history of searched files and text
// or history of results.
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------


// prototypes ---------------------------------------------------------------
ULONG saveFilTxtHistEntry(PSZ pBuf, PSAVEFILTXTHIST psfth);


// globals ------------------------------------------------------------------


//===========================================================================
// Store the current search criterion and search result in a packed linked
// list which then can be stored on the disk.
// Parameters --------------------------------------------------------------
// PSEARCHRES psr : search data to be saved.
// Return value ------------------------------------------------------------
// PPLList : allocated packed linked list or NULL in case of allocation error.
//===========================================================================

//PPLLIST saveSrchResSingle(PSEARCHRES psr) {
//   PPLLIST plist;
//   SAVESRCHRES ssr;
//   plist = PLListNew(0x10000);
//   ssr.psr = psr;
//   ssr.id = SRCHCR_FLSINGLE;
//   plist = PLListItemAddC(plist, 0, storeSrchCriteria, &ssr);
//   ssr.id = SRCHCR_OPTION;
//   plist = PLListItemAddC(plist, 0, storeSrchCriteria, &ssr);
//   ssr.id = SRCHCR_FILES;
//   plist = PLListItemAddC(plist, 0, storeSrchCriteria, &ssr);
//   ssr.id = SRCHCR_TEXT;
//   plist = PLListItemAddC(plist, 0, storeSrchCriteria, &ssr);
//   ssr.id =
//   plist = PLListListImport(plist, psr->result, 0);
//   return plist;
//}


//===========================================================================
// Callback procedure used to save search data on the disk.
// Parameters --------------------------------------------------------------
// PSRCHCRITERIA psrchcr : store-search-data-item structure
// PSEARCHRES psr        : search data to be stored
// Return value ------------------------------------------------------------
// ULONG : when 'psrchr' is NULL the size of the needed storage is returned.
//===========================================================================

ULONG storeSrchCriteria(PSRCHCRITERIA psrchcr, PSAVESRCHRES pssr)
  {
  switch ( pssr->id )
    {
    // store the flag (single search data or list of search data)
    case SRCHCR_FLSINGLE:
    case SRCHSR_FLLIST:
      if ( !psrchcr ) return sizeof(ULONG);
      psrchcr->option = pssr->id;
      break;
      // store the current search options
    case SRCHCR_OPTION:
      if ( !psrchcr ) return sizeof(ULONG);
      psrchcr->option = pssr->psr->option;
      break;
      // store the name of the file(s) to be searched
    case SRCHCR_FILES:
      if ( !psrchcr ) return StringLen(pssr->psr->fileSpec) + 5;
      memcpy(&psrchcr->text, pssr->psr->fileSpec,
             StringLen(pssr->psr->fileSpec) + 5);
      break;
      // store the text string(s) to be searched
    case SRCHCR_TEXT:
      if ( !psrchcr ) return StringLen(pssr->psr->textSpec) + 5;
      memcpy(&psrchcr->text, pssr->psr->fileSpec,
             StringLen(pssr->psr->textSpec) + 5);
      break;
    } /* endswitch */
  return 0;
  }


//===========================================================================
// Load a search criteria or search result file.
// Parameters --------------------------------------------------------------
// HWND hwnd   : window handle. If this is NULLHANDLE the file content is
//               only read to memory and the memory is not freed.
// PSZ pszFile : name of the file to be read. If this is NULL and hwnd is
//               not NULLHANDLE the content of g.loadData is transferred
//               to memory and g.loadData is freed.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL loadFile(HWND hwnd, PSZ pszFile)
  {
  HFILE hf;
  PPLLIST p;
  INT i;
  ULONG cb;
  HWND hDDbox;
  ITERITEM iterItem;
  // check the input parameters
  if ( !hwnd && !pszFile ) return FALSE;
  // if the file name was specified
  if ( pszFile )
    {
    // allocate a buffer for reading the header
    if ( NULL == (g.loadData = (PDSHISTFILE)malloc(0x400)) ) goto error0;
    if ( FMO_ERROR == (hf = fmOpen(pszFile, FMO_SHAREALL)) ) goto error1;
    if ( !fmRead(hf, g.loadData, 0x400) ) goto error2;
    // check the file type:
    // if this is a list of searched files and text strings
    if ( !memcmp(g.loadData->flag, HIST_FILETEXT, 8) )
      {
      // reduce the buffer size to the size of the DSHISTFILE structure
      g.loadData = realloc(g.loadData, sizeof(DSHISTFILE));
      // allocate memory for reading the list of searched files
      if ( !(p = PLListDup(NULL, g.loadData->cbFiles)) ) goto error2;
      // read the list of searched files
      if ( !fmPtrSet(hf, sizeof(DSHISTFILE), FILE_BEGIN) ||
           !fmRead(hf, p, g.loadData->cbFiles) )
        goto error2;
      g.loadData->files = p;
      // allocate memory for reading the list of searched text strings
      if ( !(p = PLListDup(NULL, g.loadData->cbTexts)) ) goto error3;
      // read the list of searched texts
      if ( !fmRead(hf, p, g.loadData->cbTexts) ) goto error3;
      g.loadData->texts = p;
      g.loadData->isResult = FALSE;
      // if this is a list of search results
      }
    else if ( !memcmp(g.loadData->flag, HIST_RESULT, 8) )
      {
      // reduce the buffer size to the size needed to contain the file header
      cb = sizeof(DSHISTFILE) + (g.loadData->count - 1) * sizeof(PPLLIST);
      g.loadData = realloc(g.loadData, cb);
      // reset the file pointer to point past the file header
      if ( !fmPtrSet(hf, cb, FILE_BEGIN) ) goto error2;
      // loops through all the data
      for ( i = 0; i < g.loadData->count; ++i )
        {
        // allocate storage to read a result item
        if ( !(p = PLListDup(NULL, g.loadData->aCbRes[i])) ||
             // read the result data into the allocated storage
             !fmRead(hf, p, g.loadData->aCbRes[i]) ||
             // convert the read data to a SEARCHRES structure
             !(g.loadData->ares[i] = loadSavedResult(p)) )
          {
          do
            {
            srchResDel(g.loadData->ares[--i]);
            } while ( i );
          PLListDel(p);
          goto error2;
          } /* endif */
        PLListDel(p);
        } /* endfor */
      g.loadData->isResult = TRUE;
      }
    else
      {
      goto error2;
      } /* endif */
    fmClose(hf);
    } /* endif */
  if ( hwnd && g.loadData )
    {
    // if the loaded data is results data fills the result history list
    if ( g.loadData->isResult )
      {
      // first destroy the current search result history
      hDDbox = DlgItemHwnd(hwnd, DD_SRCHRES);
      for ( i = wLbxItemCount(hDDbox) - 1; i >= 0; --i )
        {
        srchResDel((PSEARCHRES)wLbxItemHnd(hDDbox, i));
        wLbxItemDel(hDDbox, i);
        } /* endfor */
      // build the new search result history
      for ( i = 0; i < g.loadData->count; ++i )
        {
        srchResHistAddEntry(hwnd, g.loadData->ares[i], TRUE);
        } /* endfor */
      // if the loaded data is searched file/text history fills the files
      // and text lits
      }
    else
      {
      resetSrchFilTxtHist(DlgItemHwnd(hwnd, DD_FILESRCH), g.loadData->files);
      PLListDel(g.loadData->files);
      resetSrchFilTxtHist(DlgItemHwnd(hwnd, DD_TEXTSRCH), g.loadData->texts);
      PLListDel(g.loadData->texts);
      } /* endif */
    free(g.loadData);
    } /* endif */
  return TRUE;
  error3:
  PLListDel(g.loadData->files);
  error2:
  fmClose(hf);
  error1:
  free(g.loadData);
  g.loadData = NULL;
  error0:
  return FALSE;
  }


//===========================================================================
// Save the content of the drop down box 'id' in a packed linked list.
// Parameters --------------------------------------------------------------
// HWND hwnd    : window handle
// ULONG id     : drop down box id
// PPLLIST* ppl : (output) allocated packed linked list.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL saveFilTxtHist (HWND hwnd, ULONG id, PPLLIST* ppl)
  {
  SAVEFILTXTHIST sfth;
  PPLLIST p, pnew;
  ULONG ci;
  INT i;
  // get the handle of the drop down list
  sfth.hwnd = DlgItemHwnd(hwnd, id);
  // get the count of the items in the drop down list (if 0 just return TRUE).
  if ( !(ci = wLbxItemCount(sfth.hwnd)) ) return TRUE;
  // create a packed linked list
  if ( !(p = PLListNew(0)) ) return FALSE;
  // populate the list
  for ( sfth.i = 0; sfth.i < ci; ++sfth.i )
    {
    pnew = PLListItemAddC(p, 0, NULL, saveFilTxtHistEntry, &sfth);
    if ( !pnew )
      {
      PLListDel(p);
      return FALSE;
      } /* endif */
    p = pnew;
    } /* endfor */
  PLListShrink(p);
  *ppl = p;
  return TRUE;
  }


//===========================================================================
// Callback procedure used to store the content of a drop down list into a
// packed linked list.
// Parameters --------------------------------------------------------------
// PSZ pBuf              : buffer where to store drop down list item text.
// PSAVEFILTXTHIST psfth : listbox handle and item id data
// Return value ------------------------------------------------------------
// ULONG : when 'pBuf' is NULL return the size of the storage needed to
//         save the data.
//===========================================================================
static
            ULONG saveFilTxtHistEntry(PSZ pBuf, PSAVEFILTXTHIST psfth)
  {
  if ( !pBuf )
    return(psfth->cb = wLbxItemTextLength(psfth->hwnd, psfth->i) + 1);
  wLbxItemText(psfth->hwnd, psfth->i, psfth->cb, pBuf);
  return 0;
  }


