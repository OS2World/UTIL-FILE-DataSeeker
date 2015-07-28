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
CHAR mapCaseTable[256];

//===========================================================================
// Initialize the table for case insensitive comparation.
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL initUpperCaseTable(VOID)
  {
  INT i;
  COUNTRYCODE cc = {0, 0};
  for ( i = 0; i < 256; ++i ) mapCaseTable[i] = i;
  return !DosMapCase(256, &cc, mapCaseTable);
  }

//===========================================================================
// Compare C strings case insensitively.
// Parameters --------------------------------------------------------------
// PSZ ps1 : string 1
// PSZ ps2 : string 2
// Return value ------------------------------------------------------------
// INT : 0 if the strings are equal.
//===========================================================================

INT szCompI(PSZ ps1, PSZ ps2)
  {
  for ( ; (mapCaseTable[*ps1] == mapCaseTable[*ps2]) && mapCaseTable[*ps1];
      ++ps1, ++ps2 ) ;
  return mapCaseTable[*ps1] - mapCaseTable[*ps2];
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// PSZ ps1 : string 1
// PSZ ps2 : string 2
// ULONG n : maximume count of characters which have to be compared.
// Return value ------------------------------------------------------------
// INT : 0 if the strings are equal.
//===========================================================================

INT szCmpNI(PSZ ps1, PSZ ps2, ULONG n)
  {
  for ( ;
      (mapCaseTable[*ps1] == mapCaseTable[*ps2]) && mapCaseTable[*ps1] && --n;
      ++ps1, ++ps2 ) ;
  return mapCaseTable[*ps1] - mapCaseTable[*ps2];
  }


//===========================================================================
// Copy source to destination returning the address of the string end.
// Parameters --------------------------------------------------------------
// PSZ pDest : destination string
// PSZ pSrc  : source string
// Return value ------------------------------------------------------------
// PSZ end of destination string
//===========================================================================

PSZ szCpy(PSZ pDest, PSZ pSrc)
  {
  ULONG cb = strlen(pSrc);
  return(PSZ)memcpy(pDest, pSrc, cb + 1) + cb;
  }


//===========================================================================
// strcpy() replacement.
// Unlike strcpy() the destination string is not padded with zeros, it is
// always terminated by a NUL character, and the end of the destination
// string is returned.
// Parameters --------------------------------------------------------------
// PSZ pDest : destination string
// PSZ pSrc  : source string
// Return value ------------------------------------------------------------
// PSZ end of destination string
//===========================================================================

PSZ szCpyN(PSZ pDest, PSZ pSrc, ULONG cb)
  {
  cb = min(strlen(pSrc), cb - 1);
  return pDest[cb] = 0, (PSZ)memcpy(pDest, pSrc, cb) + cb;
  }


//===========================================================================
// Concatenate pCat to pDest returning the address of the string end.
// Parameters --------------------------------------------------------------
// PSZ pDest : destination string
// PSZ pCat  : string to be appended to destination
// Return value ------------------------------------------------------------
// PSZ end of destination string
//===========================================================================

PSZ szCat(PSZ pDest, PSZ pCat)
  {
  ULONG cbDest = strlen(pDest);
  ULONG cbCat = strlen(pCat);
  return(PSZ)memcpy(pDest + cbDest, pCat, cbCat + 1) + cbCat;
  }


//===========================================================================
// Uppercases the string psz;
// Parameters --------------------------------------------------------------
// PSZ psz : string to be uppercased.
// Return value ------------------------------------------------------------
// PSZ : uppercased string.
//===========================================================================

PSZ szUpperCase(PSZ psz)
  {
  while ( *psz ) *psz = mapCaseTable[*psz], ++psz;
  return psz;
  }


//===========================================================================
// szStrip: modifies a C string deleting all leading characters included
//          in pss and all trailing characters included in pse
//          The psz must not be a read only string.
// Parameters --------------------------------------------------------------
// PSZ psz: string to be stripped
// PSZ pss: string containing the characters to be stripped from the string
//          start. If the address is less than 0xffff it is interpreted as
//          char and all characters == (CHAR)pss will be stripped.
//          NULL means no leading characters are to be stripped.
// PSZ pse: string containing the characters to be stripped from the string
//          end. The string is truncated by setting the character value to 0.
//          NULL means no trailing characters are to be stripped.
// Return value ------------------------------------------------------------
// PSZ : first valid character (excluding stripped characters) of psz
//===========================================================================

PSZ szStrip(PSZ psz, PSZ pss, PSZ pse)
  {
  INT i;
  ULONG cb;
  if ( !psz ) return NULL;
  // strips leading characters
  if ( (ULONG)pss > 0xffff )
    {
    cb = strlen(pss);
    while ( *psz && memchr(pss, *psz, cb) ) ++psz;
    }
  else if ( pss )
    {
    while ( *psz && (*psz == (UCHAR)pss) ) ++psz;
    } /* endif */
  // strips trailing characters
  if ( pse )
    {
    if ( *psz ) i = strlen(psz) - 1;
    if ( (ULONG)pse > 0xffff )
      {
      cb = strlen(pse);
      while ( i && memchr(pse, psz[i], cb) ) --i;
      }
    else
      {
      while ( i && (psz[i] == (UCHAR)pse) ) --i;
      } /* endif */
    psz[++i] = 0;
    } /* endif */
  return psz;
  }


//===========================================================================
// Replace all occurrences of the character 'ch' in 'psz' with 'chnew'.
// Return the found occurrences.
// 'chnew' can be 0.
// Parameters --------------------------------------------------------------
// PSZ psz   : string to be modified
// INT ch    : character to be replaced
// INT chnew : character to be written in substitution of 'ch'
// INT esc   : optional escape character. If this is not 0 the occurrences
//             of 'ch' following 'esc' are not substituted.
// Return value ------------------------------------------------------------
// ULONG : count of 'ch' occurrences in the string.
//===========================================================================

ULONG szCharReplace(PSZ psz, INT ch, INT chnew, INT esc)
  {
  ULONG cch;
  if ( !psz ) return 0;
  for ( cch = 0; *psz; ++psz )
    {
    if ( *psz == esc )
      {            // skip escaped characters
      ++psz;
      }
    else if ( *psz == ch )
      {    // replace ch
      *psz = chnew, ++cch;
      } /* endif */
    } /* endfor */
  return cch;
  }


//===========================================================================
// Iterate a sequence of NUL terminated strings executing the callback
// procedure for each string.
// Return the count of succesfully processed strings.
// Parameters --------------------------------------------------------------
// PSZ psz        : address of the start of the C strings sequence
// PSZSEQFN pFunc : callback procedure if this is NULL the function returns
//                  the count of strings
// PVOID pParm    : optional callback procedure parameter (set to NULL
//                  if this not to be used).
// BOOL errBreak  : if TRUE stop processing the sequence if pFunc returns
//                  FALSE;
// Return value ------------------------------------------------------------
// ULONG : count of processed strings
//===========================================================================

ULONG szSequenceIterate(PSZ psz, PSZSEQFN pFunc, PVOID pParm, BOOL errBreak)
  {
  ULONG count;
  for ( count = 0; *psz; ++count )
    {
    if ( pFunc && errBreak && !pFunc(psz, pParm) ) break;
    psz += strlen(psz) + 1;
    } /* endfor */
  return count;
  }


//===========================================================================
// Converts a signed long to a char string optionally separating thousands
// the 'sep' character and aligning it to the right padding it with the 'pad'
// character.
// Parameters --------------------------------------------------------------
// PSZ psz    : output buffer address
// ULONG cb   : buffer size (including space for the terminator)
// LONG lIn   : signed long to be converted
// INT sep    : separator if !0 the string is thousand separated
// INT pad    : if !0 the string is right aligned and padded on the left
//              with the 'pad' character
// Return value ------------------------------------------------------------
// PSZ addres of the converted string
//===========================================================================

PSZ szFromLong(PSZ psz, ULONG cb, LONG lIn, INT sep, INT pad)
  {
  INT i = 0;
  PSZ p, p2;
  BOOL sign;
  if ( lIn < 0 )
    {
    sign = TRUE;
    lIn *= -1;
    ++psz;
    --cb;
    }
  else
    {
    sign = FALSE;
    } /* endif */
  for ( i = 1, p = psz; ; ++i )
    {
    *p++ = lIn % 10 + '0';
    if ( 0 == (lIn /= 10) ) break;
    if ( sep && !(i % 3) ) *p++ = sep;
    } /* endfor */
  if ( pad ) for ( i = p - psz + 1; i < cb; ++i ) *p++ = pad;
  *p = 0;
  for ( --p, p2 = psz; p > p2; i = *p, *p-- = *p2, *p2++ = i ) ;
  if ( sign ) *(--psz) = '-';
  return psz;
  }


//===========================================================================
// 20090615 LFS
// Converts a signed long to a char string optionally separating thousands
// the 'sep' character and aligning it to the right padding it with the 'pad'
// character.
// Parameters --------------------------------------------------------------
// PSZ psz    : output buffer address
// ULONG cb   : buffer size (including space for the terminator)
// LONGLONG llIn   : signed LONGLONG structure to be converted
// INT sep    : separator if !0 the string is thousand separated
// INT pad    : if !0 the string is right aligned and padded on the left
//              with the 'pad' character
// Return value ------------------------------------------------------------
// PSZ addres of the converted string
//===========================================================================
// 20090615 LFS
PSZ szFromLongLong(PSZ psz, ULONG cb, LONGLONG llIn, INT sep, INT pad)
  {
  INT i = 0;
  PSZ p, p2;
  BOOL sign;
  long long llTemp;

  llTemp = llIn;/*.ulHi << 32;*/
  //llTemp += llIn.ulLo;

  if ( llTemp < 0 )
    {
    sign = TRUE;
    llTemp *= -1;
    ++psz;
    --cb;
    }
  else
    {
    sign = FALSE;
    } /* endif */
  for ( i = 1, p = psz; ; ++i )
    {
    *p++ = llTemp % 10 + '0';
    if ( 0 == (llTemp /= 10) ) break;
    if ( sep && !(i % 3) ) *p++ = sep;
    } /* endfor */
  if ( pad ) for ( i = p - psz + 1; i < cb; ++i ) *p++ = pad;
  *p = 0;
  for ( --p, p2 = psz; p > p2; i = *p, *p-- = *p2, *p2++ = i ) ;
  if ( sign ) *(--psz) = '-';
  return psz;
  }


//===========================================================================
// Compare 'pattern' (containing wildcards '*' and '?') with the file name
// 'name'.
// Parameters --------------------------------------------------------------
// PSZ pat : pattern string containing wildcards.
// PSZ str : file name to be compared with pattern.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (match/mistmatch)
//===========================================================================

BOOL szWildCardMatchFileName(PSZ pat, PSZ str)
  {
  PSZ s, p;
  BOOL star = FALSE;

  loopStart:
  for ( s = str, p = pat; *s; ++s, ++p )
    {
    switch ( *p )
      {
      case '?':
        if ( *s == '.' ) goto starCheck;
        break;
      case '*':
        star = TRUE;
        str = s, pat = p;
        do
          {
          ++pat;
          } while ( *pat == '*' );
        if ( !*pat ) return TRUE;
        goto loopStart;
      default:
        if ( mapCaseTable[*s] != mapCaseTable[*p] )
          goto starCheck;
        break;
      } /* endswitch */
    } /* endfor */
  while ( *p == '*' ) ++p;
  return(!*p);

  starCheck:
  if ( !star ) return FALSE;
  str++;
  goto loopStart;
  }

//===========================================================================
// Compare 'pattern' (containing wildcards '*' and '?') with the dir name
// build from sPath\ff.achName
// Parameters --------------------------------------------------------------
// FILEFINDBUF3L ff ... file info
// PSZ sPath        ... path
// PSZ sPattern     ... pattern to compare with
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (match/mistmatch)
//===========================================================================

BOOL szWildCardMatchDirName(FILEFINDBUF3L ff, PSZ sPath, PSZ sPattern)
  {
  PSZ p;
  int i;

  //TRACE3("ff.cchName=%d, ff.achName=%s, sPath=%s", ff.cchName, ff.achName, sPath);
  if ( ff.cchName != 1 ) return FALSE;                    // match only on single character dir name entry
  if ( ff.achName[ff.cchName - 1] != '.' ) return FALSE;  // match only on '.' entry

  // dir with name xxx\. found  extract last dir entry
  {
    CHAR cBuf[CCHMAXPATH];

    i = strlen(sPath);
    p = sPath + i - 3;    // point to end of string excluding trailing \*
    while ( *p != '\\' && i > 0 )
      {
      p--;
      i--;
      }
    p++;
    strcpy(cBuf, p);
    cBuf[strlen(cBuf) - 2] = '\0';
    //TRACE2("sPattern=%s, extracted dir name=%s", sPattern, cBuf);
    return szWildCardMatchFileName(sPattern, cBuf);
  }
  }


//===========================================================================
// VOID secondsToFormattedTime(PSZ pszBuf, ULONG ulSeconds, INT timesep);
// Converts an amount of seconds expressed as unsigned long to
// a formatted string of text with the time divided in hours, minutes
// and seconds. Maximum allowed seconds is 359999 (99:59:59 hh:mm:ss)
// Parameters --------------------------------------------------------------
// PSZ pszBuf      : (output) converted time buffer
// ULONG ulSeconds : seconds to be converted to hh:mm:ss
// INT timesep     : time separator character
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID szSecondsToFormattedTime(PSZ pszBuf, ULONG ulSeconds, INT timesep)
  {
  // overflow check
  if ( ulSeconds >= 360000 )
    {
    memcpy(pszBuf, "--:--:--", 9);
    pszBuf[2] = pszBuf[5] = timesep;
    }
  else
    {
    pszBuf[0] = (ulSeconds / 36000) + '0';
    pszBuf[1] = ((ulSeconds / 3600) % 10) + '0';
    pszBuf[2] = timesep;
    pszBuf[3] = (((ulSeconds / 60) % 60) / 10) + '0';
    pszBuf[4] = ((ulSeconds / 60) % 10) + '0';
    pszBuf[5] = timesep;
    pszBuf[6] = ((ulSeconds % 60) / 10) + '0';
    pszBuf[7] = (ulSeconds % 10) + '0';
    pszBuf[8] = 0;
    } /* endif */
  }


/*
**  Portable, public domain strupr() & strlwr()
*/

//#include <ctype.h>

char *strupr(char *str)
  {
  char *string = str;

  if ( str )
    {
    for ( ; *str; ++str )
      *str = toupper(*str);
    }
  return string;
  }

char *strlwr(char *str)
  {
  char *string = str;

  if ( str )
    {
    for ( ; *str; ++str )
      *str = tolower(*str);
    }
  return string;
  }



