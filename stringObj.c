/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// stringObj.c :
// a structure describing text or binary data strings.
// the first member is an integer representing the string length (including
// the NUL char), the second member is a variable size array of chars holding
// the string data:
// struct _STRING {
//    ULONG cb;
//    CHAR ach[4];
// } ;
//
//
// STRING string(INT cb);
// -
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------

// string allocation step in bytes
#define STR_CBGROW 0x20
// calculate allocation for a string of _cb_ characters
#define STR_CBALLOC(_cb_)  (RNDUP((_cb_) + 5, STR_CBGROW))


// prototypes ---------------------------------------------------------------


// globals ------------------------------------------------------------------

//===========================================================================
// Convert a C text string to a resizeable string object.
// Parameters --------------------------------------------------------------
// PSZ sz   : initialization text string.
//            If NULL an unitialized object of size 'cb' is allocated.
// ULONG cb : string size. If 'cb' is -1 the length of the string is assumed.
//            If 'cb' is 0 or 'sz' is NULL an empty string is allocated.
// Return value ------------------------------------------------------------
// STRING : allocated object, NULL in case of error
//===========================================================================

STRING StringNew(PSZ sz, INT cb)
  {
  STRING str;
  TRACE2("'%s', len=%d", sz, cb);
  if ( !cb && (!sz || !*sz) ) cb = 0;
  if ( cb < 0 ) cb = strlen(sz);
  if ( NULL != (str = (STRING)malloc(STR_CBALLOC(cb))) )
    {
    str->len = cb;
    //TRACE3("sz=%d, sz='%.20s', len=%d", sz, sz, str->len);
    if ( sz )
      {
      //TRACE("copy string");
      memcpy(str->ach, sz, cb + 1);
      str->ach[cb] = 0;
      } /* endif */
    } /* endif */
  TRACE2("len=%d, '%s'", str->len, String(str));
  return str;
  }


//===========================================================================
// Destroy a string object.
// Parameters --------------------------------------------------------------
// PSTRING pstr : address of the string to be freed.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID StringDel(PSTRING pstr)
  {
  free(*pstr);
  *pstr = NULL;
  }


//===========================================================================
// Change the content of a string object.
// Parameters --------------------------------------------------------------
// STRING str : string object whose content has to be changed
// PSZ sz     : new string content
// INT cb     : length of sz. If 'cb' is < 0 then the length is calculated.
// Return value ------------------------------------------------------------
// STRING : updated string object (the address of the object may have been
//          changed).
//          NULL in case of error.
//===========================================================================

STRING StringSet(STRING str, PSZ sz, INT cb)
  {
  STRING strnew = str;
  // if 'str' is NULL just create a new object
  if ( !str ) return StringNew(sz, cb);
  // check the other parameters
  if ( cb < 0 ) cb = strlen(sz);
  // if the new string does not fit in the current object realloc() it
  if ( (STR_CBALLOC(cb) == STR_CBALLOC(str->len)) ||
       (NULL != (strnew = (STRING)realloc(str, STR_CBALLOC(cb)))) )
    {
    strnew->len = cb;
    // use memmove just in case sz is a substring of str->ach
    if ( cb && sz ) memmove(strnew->ach, sz, cb + 1);
    strnew->ach[cb] = 0;
    } /* endif */
  else TRACE("hm....?");
  return strnew;
  }


//===========================================================================
// Insert cb characters from the char* psz into a STRING object.
// Parameters --------------------------------------------------------------
// STRING str  : string object to be modified. If this is NULL a new string
//               is allocated.
// PSZ psz     : data to be inserted into the STRING object.
//               If 0 psz is inserted at the beginning, if -1 psz is
//               appended to the end of the STRING data.
// LONG cb     : amount of data to be inserted from 'psz' to 'str'.
//               If cb = -1, the whole string psz is inserted.
//               If cb is 0 the STRING is not modified.
// LONG offset : insertion offset.
// Return value ------------------------------------------------------------
// STRING : updated STRING object. NULL in case of error.
//===========================================================================

STRING StringInsert(STRING str, PSZ psz, LONG cb, LONG offset)
  {
  ULONG cbNewLen;
  STRING strNew = str;
  if ( !str )
    {
    if ( (offset == 0) || (offset == -1) ) return StringNew(psz, cb);
    return NULL;                         // invalid offset
    } /* endif */
  if ( cb )
    {
    if ( cb == -1 ) cb = strlen(psz);
    if ( offset == -1 )
      {
      offset = str->len;
      }
    else if ( offset > str->len )
      {      // invalid offset
      return NULL;
      } /* endif */
    cbNewLen = str->len + cb;
    // if the new string does not fit in the current object realloc() it
    if ( (STR_CBALLOC(cbNewLen) == STR_CBALLOC(str->len)) ||
         (NULL != (strNew = (STRING)realloc(str, STR_CBALLOC(cbNewLen)))) )
      {
      if ( offset < strNew->len )
        memmove(String(strNew) + offset + cb,
                String(strNew) + offset,
                StringLen(strNew) - offset);
      memcpy(String(strNew) + offset, psz, cb);
      String(strNew)[cbNewLen] = 0;
      StringLen(strNew) = cbNewLen;
      } /* endif */
    } /* endif */
  return strNew;
  }


//===========================================================================
// Insert a character into a STRING object.
// Parameters --------------------------------------------------------------
// STRING str  : string object to be modified. If this is NULL a new string
//               is allocated.
// INT chr     : character to be inserted into the string.
// LONG offset : insertion offset.
// Return value ------------------------------------------------------------
// STRING : updated STRING object. NULL in case of error.
//===========================================================================

STRING StringInsertChar(STRING str, INT chr, LONG offset)
  {
  return StringInsert(str, (PSZ)&chr, offset, ((chr > 0xff) ? 2 : 1));
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// STRING str : string object whose content must be stripped
// PSZ pss: string containing the characters to be stripped from the string
//          start. If the address is less than 0xffff it is interpreted as
//          char and all characters == (CHAR)pss will be stripped.
//          NULL means no leading characters are to be stripped.
// PSZ pse: string containing the characters to be stripped from the string
//          end. The string is truncated by setting the character value to 0.
//          NULL means no trailing characters are to be stripped.
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================

STRING StringStrip(STRING str, PSZ pss, PSZ pse)
  {
  PSZ pnew;
  ULONG cb;
  if ( !str ) return NULL;
  pnew = szStrip(str->ach, pss, pse);
  cb = strlen(pnew);
  if ( cb != str->len )
    {
    memmove(str->ach, pnew, cb);
    str->len = cb;
    str->ach[cb] = 0;
    if ( STR_CBALLOC(cb) != STR_CBALLOC(str->len) )
      str = realloc(str, STR_CBALLOC(cb));
    } /* endif */
  return str;
  }


//===========================================================================
// Replace all occurrences of the character 'ch' in 'str' with 'chnew'.
// Return the found occurrences.
// Note: 'ch' or 'chnew' can be 0.
// Parameters --------------------------------------------------------------
// STRING str : string object whose content must be modified
// INT ch    : character to be replaced
// INT chnew : character to be written in substitution of 'ch'
// INT esc   : optional escape character. If this is not 0 the occurrences
//             of 'ch' following 'esc' are not substituted.
// Return value ------------------------------------------------------------
// ULONG : count of 'ch' occurrences in the string.
//===========================================================================

ULONG StringCharReplace(STRING str, INT ch, INT chnew, INT esc)
  {
  ULONG cch;
  INT i;
  if ( !str ) return 0;
  for ( i = 0, cch = 0; i < StringLen(str); ++i )
    {
    if ( String(str)[i] == esc )
      {
      ++i;
      }
    else if ( String(str)[i] == ch )
      {
      String(str)[i] = chnew;
      ++cch;
      } /* endif */
    } /* endfor */
  return cch;
  }


//===========================================================================
// Update the 'len' member with the real length of the string and reallocate
// the string. This is useful when an empty string was previously allocated.
// Parameters --------------------------------------------------------------
// STRING str : string object whose content must be modified
// Return value ------------------------------------------------------------
// ULONG : current string length.
//===========================================================================

ULONG StringLengthReset(STRING str)
  {
  ULONG prevLen = str->len;
  str->len = strlen(str->ach);
  if ( prevLen > STR_CBALLOC(str->len) )
    str = realloc(str, STR_CBALLOC(str->len));
  return str->len;
  }


//===========================================================================
// Read the content of a file into a STRING obj.
// Parameters --------------------------------------------------------------
// PSZ pszFileName : name of the file.
// LONG offset     : offset from file start or file end if negative.
// LONG cb         : number of bytes to read.
//                   If this is less than 0 or 0 reads the whole file.
// Return value ------------------------------------------------------------
// STRING : allocated object, NULL in case of error
//===========================================================================

// 20090615 LFS not used with DataSeeker
/*STRING StringFromFile(PSZ pszFileName, LONG offset, LONG cb) {
   FILESTATUS3L fs;
   HFILE hf;
   ULONG ul;
   PSZ psz = NULL;
   ULONG offsetFlag;
   STRING str = NULL;
   // open the file
   if (!xDosOpen(pszFileName,  &hf, &ul, 0, FILE_NORMAL,             // no LFS // 20090615 LFS
                OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_SEQUENTIAL | OPEN_ACCESS_READONLY, NULL)) {
      // check the file size
      if (xDosQueryFileInfo(hf, FIL_STANDARDL, &fs, sizeof(fs)))    // 20090615 LFS
         goto closeFile;
      // check the cb parameter
      if ((cb <= 0) || (cb > fs.cbFile)) cb = fs.cbFile;
      // check the offset parameter
      if (offset) {
         if (offset < 0) {
            offsetFlag = FILE_END;
            if ((-offset) < cb) goto closeFile;  // invalid offset
         } else {
            offsetFlag = FILE_BEGIN;
            if (offset > cb) goto closeFile;     // invalid offset
         } // endif
         if (DosSetFilePtr(hf, offset, offsetFlag, &ul)) goto closeFile;    // no LFS // 20090615 LFS
      } // endif
      // allocate the string object and read the file
      if (NULL != (str = StringNew(NULL, cb))) {
         if (DosRead(hf, String(str), cb, &ul)) {                           // no LFS // 20090615 LFS
            StringDel(&str);
         } else {
            String(str)[ul] = 0;
         } // endif
      } // endif
      closeFile:                    //********** closeFile label **********
      DosClose(hf);
   } // endif
   return str;
}
*/
