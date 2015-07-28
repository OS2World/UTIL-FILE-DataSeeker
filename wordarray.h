/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING. 
*******************************************************************************/

//===========================================================================
// wordarray.h : this is used to separate a string in words.
//
// Organization of data in a word array:
// - header:
//   ULONG        array elements
//   PSZ          point to the whole string
// - array:
//   ULONG        1st word length
//   PSZ          1st word address
//   ...
//   ULONG        Nth word length
//   PSZ          Nth word address
// - data storage:
//   CHAR[]       storage for string content
//
// --2003  - Alessandro Felice Cantatore
//===========================================================================

#define WORDARRAY_MAX_WORDS     50       // 20090925 AB, was 2, needed for multiple drive search

#ifndef _WORDARRAY_H_
   #define _WORDARRAY_H_

// text word data -----------------------------------------------------------

typedef struct {
   ULONG cb;            // word length
   PSZ pw;              // word address
} TXTWORD, * PTXTWORD;

typedef struct {
   ULONG ci;            // word count
   PSZ psz;             // points to the whole string
   TXTWORD aw[WORDARRAY_MAX_WORDS];       // variable length array of words // 20090923 AB
} WORDARRAY, * PWORDARRAY;

typedef BOOL (*PWAITERFN)(PTXTWORD, PVOID);

// prototypes ---------------------------------------------------------------

PWORDARRAY WordArrayNew(PSZ sz, LONG cb, INT sep, INT esc, BOOL skip);
PTXTWORD WordArrayWord(PWORDARRAY pwa, INT idx);
ULONG WordArrayIterate(PWORDARRAY pwa, PWAITERFN pFunc, PVOID pParm, BOOL bbrk);
VOID WordArrayDel(PWORDARRAY pwa);
PSZ WordArrayToString(PWORDARRAY pWa, int iSep);

#endif // #ifndef _WORDARRAY_H_
