/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// loadsave.h :
// data definitions used to save/restore history of searched files and text
// or history of results.
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _LOADSAVE_H_
   #define _LOADSAVE_H_

// file flags ---------------------------------------------------------------
#define HIST_FILETEXT              "HISTSRCH"
#define HIST_RESULT                "HISTRESL"


// store search criteria IDs ------------------------------------------------

#define SRCHCR_FLSINGLE          0   // flag (single search data)
#define SRCHSR_FLLIST            1   // flag (list of search data)
#define SRCHCR_OPTION            2   // current search options
#define SRCHCR_FILES             3   // files to be searched
#define SRCHCR_TEXT              4   // text to be searched

// data used to save the history of searched text and files -----------------
#pragma pack(2)
typedef struct {
   HWND hwnd;              // drop down list handle
   SHORT i;                // item index
   SHORT cb;               // item size in bytes (including NUL terminator)
} SAVEFILTXTHIST, * PSAVEFILTXTHIST;

// data used to save the search results -------------------------------------

typedef struct {
//   HWND hwnd;
   PSEARCHRES psr;         // search data (criteria and result) to be saved
   ULONG id;               // id of the current piece of data beeing saved
} SAVESRCHRES, * PSAVESRCHRES;

// search criteria union ----------------------------------------------------

typedef union {
   ULONG option;
   _STROBJ text;
} SRCHCRITERIA, * PSRCHCRITERIA;


// structure of the file data -----------------------------------------------
typedef struct {
   union {
      BOOL isResult;         // type flag as integer
      CHAR flag[8];          // type flag (either HIST_FILETEXT or HIST_RESULT)
   } ;
   union {
      PPLLIST files;         // history of file specifications
      ULONG count;           // count of result lists
      ULONG cbFiles;         // sizeof the files list in bytes
   } ;
   union {
      PPLLIST texts;         // history of text specifications
      ULONG aCbRes[1];       // array of sizes of PLLIST structures
      PSEARCHRES ares[1];    // array of SEARCHRES structures
      ULONG cbTexts;         // sizeof the text string list in bytes
   } ;
} DSHISTFILE, * PDSHISTFILE;

#define PLLFNAMEFROMFILE(_p_) \
   ((PPLLIST)((PBYTE)(_p_) + (_p_)->offFiles))
#define PLLTEXTFROMFILE(p) \
   ((PPLLIST)((PBYTE)(_p_) + (_p_)->offTexts))

#endif // #ifndef _LOADSAVE_H_
