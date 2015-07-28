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


// globals ------------------------------------------------------------------

//===========================================================================
// Create a word array from a NUL terminated string.
// Parameters --------------------------------------------------------------
// PSZ sz    : string to be word separated
// LONG cb   : string length. If <= 0 the length is calculated via strlen.
// INT sep   : word separator
// INT esc   : escape character. 'sep' characters preceeded by 'esc' are not
//             considered as separators. Set this to 0 if no escape
//             characters are needed.
// BOOL skip : skip flag. Indicate what should be the behaviour when 'sz'
//             contains multiple consecutive separator characters.
//             When 'skip' is TRUE they are treated as an unique separator
//             character.
//             When 'skip' is FALSE an empty word is added for each
//             separator character following the first one.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

PWORDARRAY WordArrayNew(PSZ sz, LONG cb, INT sep, INT esc, BOOL skip)
  {
  PWORDARRAY pwa = 0;
  PSZ p, pprev, psz;
  ULONG cwords, cesc;
  ULONG ulArraySize;

  // check parameters
  if ( !sz ) return NULL;
  if ( cb <= 0 ) cb = strlen(sz);
  if ( !cb ) return NULL;
  // first count the words
  for ( p = sz, cwords = 1, cesc = 0; *p; )   // 20091201 AB argh - took me a lot of hours to find this bug (was cwords = 0)
    {
    if ( *p == esc )
      {
      ++cesc;
      ++p;
      }
    else if ( *p == sep )
      {
      ++p;
      if ( skip ) while ( *p && (*p == sep) ) ++p;
      if ( *p ) ++cwords;
      continue;
      } /* endif */
    ++p;
    } /* endfor */
  ulArraySize = sizeof(WORDARRAY) /*- cesc*/ + sizeof(TXTWORD) * cwords + cb + 1;   // 20091203 man, why have you subtracted cesc?
  pwa = (PWORDARRAY)malloc(ulArraySize);
  //    TRACE2("addr=0x%X, size %u", pwa, ulArraySize);
  TRACE3("words=%d, sizeof(TXTWORD)=%d, sizeof(WORDARRAY)=%d", cwords, sizeof(TXTWORD), sizeof(WORDARRAY));
  if ( pwa )
    {
    if ( cwords > WORDARRAY_MAX_WORDS )
      {
      cwords = WORDARRAY_MAX_WORDS;
      TRACE1("limit word arrays to %d", cwords);
      }
    pwa->ci = cwords;
    pwa->psz = (PSZ)pwa + sizeof(WORDARRAY) + sizeof(TXTWORD) * cwords;
    for ( p = sz, pwa->aw[0].pw = pprev = psz = pwa->psz, cwords = 0;; )
      {
      if ( !*p )
        {
        pwa->aw[cwords].cb = psz - pprev;
        *psz++ = 0;
        break;
        }
      else if ( *p == esc )
        {
        ++p;
        }
      else if ( *p == sep )
        {
        pwa->aw[cwords].cb = psz - pprev;
        *psz++ = 0;
        pprev = psz;
        ++p;
        if ( skip ) while ( *p && (*p == sep) ) ++p;
        if ( !*p ) break;
        ++cwords;
        if ( cwords < WORDARRAY_MAX_WORDS )
          {
          pwa->aw[cwords].pw = psz;
          continue;
          }
        else
          {
          char sBuf[128];
          // 20091109 AB display warning in info area
          sprintf(sBuf, " No more than %d search criteria allowed", WORDARRAY_MAX_WORDS );
          TRACE1("%s", sBuf);
          WinSetDlgItemText(g.hwndMainWin, TXT_INFO, sBuf);
          WinShowWindow(WinWindowFromID((g.hwndMainWin), (TXT_INFO)), TRUE);
          //*psz = '\0';
          break;
          }
        } /* endif */
      //TRACE1("%c", *p);
      *psz++ = *p++;
      } /* endfor */
    } /* endif */
  return pwa;
  }


//===========================================================================
// Return the word 'idx'-th of the WORDARRAY 'pwa'.
// Parameters --------------------------------------------------------------
// PWORDARRAY pwa : word array object.
// INT idx        : word index.
// Return value ------------------------------------------------------------
// PTXTWORD : required text word or NULL if 'idx' is invalid.
//===========================================================================

PTXTWORD WordArrayWord(PWORDARRAY pwa, INT idx)
  {
  return(pwa && (idx >= 0) && (idx < pwa->ci)) ? &pwa->aw[idx] : NULL;
  }


//===========================================================================
// Execute a procedure for each text word in the word array 'pwa'.
// If the procedure returns FALSE and the 'break' parameter is TRUE, the
// iteration is interrupted.
// Return the count of processed words.
// Parameters --------------------------------------------------------------
// PWORDARRAY pwa  : word array object.
// PWAITERFN pFunc : callback procedure executed for each word in the array.
// PVOID pParm     : optional parameter of the callback procedure.
// BOOL bbrk       : TRUE to stop the iteration if 'pFunc' returns FALSE.
// Return value ------------------------------------------------------------
// ULONG : count of the text words succesfully processed.
//===========================================================================

ULONG WordArrayIterate(PWORDARRAY pwa, PWAITERFN pFunc, PVOID pParm, BOOL bbrk)
  {
  ULONG i;
  char *pTemp;

  pTemp = pwa->psz;
  TRACE2("number: %d  pwa.psz=%s", pwa->ci, pwa->psz);
  if ( !pwa || !pFunc ) return 0;
  for ( i = 0; i < pwa->ci; ++i )
    {
    TRACE3("pwa->aw[%d].cb: %d, %s", i, pwa->aw[i].cb, pwa->aw[i].pw);
    if ( bbrk & !pFunc(&pwa->aw[i], pParm) ) break;
    }
  return i;
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

VOID WordArrayDel(PWORDARRAY pwa)
  {
  free(pwa);
  }

/******************************************************************************
PSZ WordArrayToString(PWORDARRAY pWa, int iSep)

Historie:
---------
created:   AB 20091221
changed:

Description:
------------
Converts a WordArray to one single string. String size is dynamically increased
to fit all words in the array.

Parameters:
-----------
pWa ... pointer to WordArray
iSep... word separator

Returns:
--------
pointer to the converted string or 0 in case of error
******************************************************************************/
#define MALLOC_SIZE         4096
#define STRING_MAX_LENGTH   10000

PSZ WordArrayToString(PWORDARRAY pWa, int iSep)
  {
  int i, iNumChar = 0;
  PSZ p, pT, pEnd, szString;

  pT = szString = malloc(MALLOC_SIZE);
  pEnd = pT + MALLOC_SIZE;
  if ( !szString )
    {
    TRACE("can not malloc memeory!!!!!!!!");
    return FALSE;
    }

  // check parameters
  if ( !pWa )
    {
    TRACE("invalid WordArray");
    return 0;
    }

  for ( i = 0; i < pWa->ci; i++ )
    {   // iterate through each WordArray member
    p = pWa->aw[i].pw;
    while ( *p && (iNumChar < STRING_MAX_LENGTH) )
      {
      // copy to destination, reallocate if insufficient memory
      *pT = *p;                   // copy to destination
      p++;                        // increment source pointer
      pT++; iNumChar++;           // increment destination pointer
      if ( !(pT < pEnd - 3) )     // leave two bytes for separator and \0
        {
        // no more space in current string left
        TRACE1("try to reallocate %d", iNumChar + MALLOC_SIZE);
        szString = realloc(szString, iNumChar + MALLOC_SIZE);
        if ( !szString )
          {
          TRACE("reallocation not possible");
          return FALSE;
          }
        else
          {   // reallocation successfull, correct pointers
          pT = szString + iNumChar;
          pEnd = szString + iNumChar + MALLOC_SIZE;
          //TRACE3("szString=0x%X, pEnd=0x%X, pT=0x%X", szString, pEnd, pT);
          }
        }
      }
    if ( !(iNumChar < STRING_MAX_LENGTH) )
      {
      TRACE1("no more than %d chars are allowed !!!!!!!!", STRING_MAX_LENGTH);
      }

    // append separator to string before process next word
    *pT = iSep;
    pT++; iNumChar++;
    }

  // append trailing 0 and delete last separator
  *(--pT) = '\0';
  TRACE3("len=%d, iNumChar=%d, '%s'", strlen(szString), iNumChar, szString);

  /*    for (i = 0; i < iNumChar && i < 512; i++)
          {
          TRACE2("(0x%X) %c", (int) szString[i], szString[i] );
          }
  */
  return szString;
  }



