/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

/******************************************************************************
 NLS.h - National language support

 Uses (eco)lange if ECOLANGE.DLL is detected on the system. If not, uses the
 'search' string

 Requirements:
 eCo Software runtime (base) -- http://ecomstation.ru/ecosoft/runtime.php

 History:
 --------
 20100209 AB creation

******************************************************************************/

// --- Includes ---------------------------------------------------------------

// --- Defines ----------------------------------------------------------------

// --- Typedefs ---------------------------------------------------------------

// --- PUBLIC / EXTERNAL Variables --------------------------------------------
extern LANGE NlsLange;
extern int iHaveNls;

// --- Variables --------------------------------------------------------------

// --- Macros -----------------------------------------------------------------
//#define LngTrans(var)         LngGetStringPointer(NlsLange, var) 20100209 AB does not work no xGetStringPointer
#define NlsGet(var1, var2)      NlsGetTranslation(var1, var2, sizeof(var2))

// --- Function prototypes ----------------------------------------------------
int     NlsInit (void);
int     NlsClose(void);
APIRET  NlsGetTranslation  (PSZ pszString, char *szTranslation, int iMaxLength);

// --- Code -------------------------------------------------------------------

