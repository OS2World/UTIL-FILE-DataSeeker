/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// stringUtil.h :
//
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _STRINGUTIL_H_
   #define _STRINGUTIL_H_

   typedef BOOL (SZSEQFN)(PSZ, PVOID);
   typedef SZSEQFN * PSZSEQFN;

// macros -------------------------------------------------------------------

#define isNotLineBreak(_ch_)    (((_ch_) != '\r') && ((_ch_) != '\n'))

// prototypes ---------------------------------------------------------------
BOOL initUpperCaseTable(VOID);
INT szCompI(PSZ ps1, PSZ ps2);
INT szCmpNI(PSZ ps1, PSZ ps2, ULONG n);
PSZ szCpy(PSZ pDest, PSZ pSrc);
PSZ szCpyN(PSZ pDest, PSZ pSrc, ULONG cb);
PSZ szCat(PSZ pDest, PSZ pCat);
PSZ szStrip(PSZ psz, PSZ pss, PSZ pse);
ULONG szCharReplace(PSZ psz, INT ch, INT chnew, INT esc);
ULONG szSequenceIterate(PSZ psz, PSZSEQFN pFunc, PVOID pParm, BOOL errBreak);
PSZ szFromLong(PSZ psz, ULONG cb, LONG lIn, INT sep, INT pad);
PSZ szFromLongLong(PSZ psz, ULONG cb, LONGLONG lIn, INT sep, INT pad);
BOOL szWildCardMatchFileName(PSZ pat, PSZ str);
BOOL szWildCardMatchDirName(FILEFINDBUF3L ff, PSZ sPath, PSZ sPattern);
VOID szSecondsToFormattedTime(PSZ pszBuf, ULONG ulSeconds, INT timesep);
PSZ szUpperCase(PSZ psz);


// INLINED PROCEDURES -------------------------------------------------------

//===========================================================================
// Find the end of a line and optionally terminate it with a NUL.
// Correctly handles lines terminated by "\r", "\r\n" or "\n".
// Parameters --------------------------------------------------------------
// PSZ psz        : ASCII text
// BOOL terminate : if TRUE terminate the string with 0
// Return value ------------------------------------------------------------
// PSZ : start of the next line. *psz is 0 if there is no other line.
//===========================================================================
_Inline
PSZ szNextLine(PSZ psz, BOOL terminate) {
   while (*psz) {
      if (*psz == '\r') {
         if (terminate) *psz = 0;
         return (*++psz == '\n')? ++psz : psz;
      } /* endif */
      if (*psz == '\n') {
         if (terminate) *psz = 0;
         return ++psz;
      } /* endif */
      ++psz;
   } /* endwhile */
   return psz;
}


//===========================================================================
// Find the last occurrence in a text string of a character from "charSet".
// Parameters --------------------------------------------------------------
// PSZ pszText : text to be searched.
// PSZ charSet : set of characters to be searched.
// ULONG cb    : size of text string to be searched.
// Return value ------------------------------------------------------------
// PSZ : address of last occurrence.
//===========================================================================
_Inline
PSZ szCharSetNR(PSZ pszText, PSZ charSet, ULONG cb) {
   PSZ p;
   while (cb--) {
      for (p = charSet; *p; ++p) {
         if (pszText[cb] == *p) return pszText + cb;
      } /* endfor */
   } /* endwhile */
   return NULL;
}



#endif // #ifndef _STRINGUTIL_H_
