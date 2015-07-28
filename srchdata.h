/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// srchdata.h :
// Data structures used to perform the search and to store the search results.
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _SRCHDATA_H_
   #define _SRCHDATA_H_

/*
 ******** SEARCH RESULTS ********
 The search results are organized in a packed linked list structured as follows:

 ÚÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³list headÃÄÂÄ´1st file foundÃÄÂÄ´1st matching line³
 ÀÄÄÄÄÄÄÄÄÄÙ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                                ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
             ³                  ÃÄ´2nd matching line³
                                  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
             ³                  ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ÀÄ´nth matching line³
             ³                    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

             ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
             ÀÄ´nth file foundÃÄÂÄ´1st matching line³
               ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                                ³
                                  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ÀÄ´nth matching line³
                                  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ


 When the sarch result are saved on the disk the following structure is used:

 ÚÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³list headÃÄÂÄ´find options       ³
 ÀÄÄÄÄÄÄÄÄÄÙ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
             ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
             ÃÄ´file(s) to be found³
             ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
             ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
             ÃÄ´text(s) to be found³
             ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
             ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
             ÀÄ´found files node   ÃÄÂÄ´1st file foundÃÄÂÄ´1st matching line³
               ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                                                        ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                     ³                  ÃÄ´2nd matching line³
                                                          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                                     ³                  ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                                        ÀÄ´nth matching line³
                                     ³                    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

                                     ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                     ÀÄ´nth file foundÃÄÂÄ´1st matching line³
                                       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                                                        ³
                                                          ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                                        ÀÄ´nth matching line³
                                                          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

/* --------------------------------------------------------------------------
  LINE OF TEXT
  used to store text lines containing an occurrence of the searched text
*/
typedef struct {
   ULONG lineno;            // line number (0 is the first line)
   CHAR line[4];            // line content
} TEXTLINE, * PTEXTLINE;


/* --------------------------------------------------------------------------
  found file item data
  used to store data of files which satisfy the searcH criteria
*/
typedef struct {
   FDATE date;              // file date
   FTIME time;              // file time
   LONGLONG cb;             // file size
   CHAR name[CCHMAXPATH];            // file name   // 20091125 AB Test from 4 to ToDo:
} FOUNDFILE, * PFOUNDFILE;


/* --------------------------------------------------------------------------
  search results
  used to store both the search criteria (i.e. specifications of searched
  files/text and search options) and the search result (list of file and
  of text lines)
*/
typedef struct {
   ULONG option;           // current search options
   STRING fileSpec;        // specific of the file(s) to be searched
   STRING textSpec;        // specific of the text string(s) to be searched
   PPLLIST result;         // list of found files (sublist of found strings)
} SEARCHRES, * PSEARCHRES;

/* --------------------------------------------------------------------------
  FindFileTextERROR data
  used to keep track of which files could not be searched and for what reason
  (i.e. what DOS* API error was reported by the system)
*/
typedef struct {
   ULONG rc;                // error code
   CHAR file[4];            // name of the file beeing processed when the
                            // error occurred
} FFTERROR, * PFFTERROR;

/* --------------------------------------------------------------------------
  Search buffer
  buffer used to build the full name of the searched file and to read the
  file content.
*/

// sizes of the buffer used during search operations ------------------------
#define CB_BUFFERGENERIC          0x00000400   // general purpose buffer
#define CB_BUFFERREAD             0x00A00000   // read file buffer (20090708 AB changed from 1MB to 10MB)

typedef struct {
   CHAR fileData[CB_BUFFERGENERIC];    // file date/time/size/name
   CHAR readFile[CB_BUFFERREAD];       // read file buffer
} SEARCHBUFFER, * PSEARCHBUFFER;

/* --------------------------------------------------------------------------
  data structure used to perform the search operations
*/
typedef struct {
   HWND hwnd;               // handle of the notification window
   HWND hwndLboxFile;       // handle of the found files listbox
   HWND hwndProgress;       // handle of the window showing the name of the
                            // file beeing processed
   PWORDARRAY fileSpec;     // specific of the file(s) to be searched
   PWORDARRAY textSpec;     // specific of the text string(s) to be searched
   ULONG ANDmask;           // in case the boolean AND operator is used
                            // this allows to check if all the required
                            // text strings were found
   ULONG curMask;           // current found strings (see previous member)
   ULONG linecount;         // line count
   ULONG cbLine;            // length of the current line
   PSZ pLine;               // current line pointer
   PSZ pEndl;               // end of line pointer
   ULONG rc;                // worker thread return code
                            // (0 = success - !0 = error)
} DOSEARCH, * PDOSEARCH;



#endif // #ifndef _SRCHDATA_H_
