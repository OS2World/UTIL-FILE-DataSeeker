/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// .h :
//
// --2003  - Alessandro Felice Cantatore
// 20111226 AB added settings for FOC
//===========================================================================


#ifndef _SETTINGS_H_
  #define _SETTINGS_H_

#define INI_FILE_VERSION 101    // 20111216 AB was 100 before v1.10
                                //  changing the version disables importing history lists

typedef struct
  {
  ULONG ver;                     // file version
  SWP swp;                       // main window size and position
  LONG clrBgSrch;                // Search button background color
  LONG clrFgSrch;                // Search button foreground color
  LONG clrBgStop;                // Stop button background color
  LONG clrFgStop;                // Stop button foreground color
  LONG clrBgHelp;                // Help button background color
  LONG clrFgHelp;                // Help button foreground color
  CHAR achFont[64];              // Dialog font
  CHAR achResFont[64];           // font listbox/dropdown boxes
  ULONG option;                  // opzioni
  ULONG maxHistory;              // max size of the history of results
  CHAR achEditor[CCHMAXPATH];           // editor
  CHAR achCmdShell[CCHMAXPATH];         // command shell
  union
    {
    PPLLIST files;              // searched files history
    ULONG cbFiles;              // sizeof the searched files/strings history
    } ;
  union
    {
    PPLLIST texts;              // searched stings history
    ULONG cbTexts;              // sizeof the searched files/strings history
    } ;
  int iFocX;                      // x position of FileOpenContainer
  int iFocY;                      // y position of FOC
  char cFocLastDir[CCHMAXPATH]; // last selected directory with FOC
  } PMSEEKINI, * PPMSEEKINI;


#endif // #ifndef _SETTINGS_H_
