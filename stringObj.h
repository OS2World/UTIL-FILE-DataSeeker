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


#ifndef _STRINGOBJ_H_
   #define _STRINGOBJ_H_

typedef struct __STROBJ _STROBJ;
typedef _STROBJ* STRING;
typedef STRING* PSTRING;

struct __STROBJ {
   INT len;          // string length
   CHAR ach[4];      // string data
} ;


// macros: ------------------------------------------------------------------

// return the length of the string _str_
#define StringLen(_str_)    ((_str_)->len)
// return the content of the string _str_
#define String(_str_)       ((_str_)->ach)
// duplicate a string object
#define StringDup(_str_)    (StringNew(String(_str_), StringLen(_str_)))
// insert a character at the beginning of a string object
#define StringPrependChar(_str_, _chr_) \
   (StringInsertChar((_str_), (_chr_), 0))
#define StringAppendChar(_str_, _chr_) \
   (StringInsertChar((_str_), (_chr_), -1))
#define StringPrepend(_str_, _psz_, _cb_) \
   (StringInsert((_str_), (_psz_), (_cb_), 0))
#define StringAppend(_str_, _psz_, _cb_) \
   (StringInsert((_str_), (_psz_), (_cb_), -1))


// Prototypes ---------------------------------------------------------------

STRING StringNew(PSZ sz, INT cb);
VOID StringDel(PSTRING pstr);
STRING StringSet(STRING str, PSZ sz, INT cb);
STRING StringStrip(STRING str, PSZ pss, PSZ pse);
ULONG StringCharReplace(STRING str, INT ch, INT chnew, INT esc);
STRING StringInsert(STRING str, PSZ psz, LONG cb, LONG offset);
STRING StringInsertChar(STRING str, INT chr, LONG offset);
ULONG StringLengthReset(STRING str);
//STRING StringFromFile(PSZ pszFileName, LONG offset, LONG cb);

#endif // #ifndef _STRINGOBJ_H_
