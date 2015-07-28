/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

/******************************************************************************
 wrapper_sw.c - wrapper for secondary window functions

 Secondary window functions are defined in sw.dll coming with MultiMedia support
 in Warp4 (?). For systems without sw.dll and mmpmcrts.dll.

 History:
 --------
 20100415 AB creation

******************************************************************************/

// --- Includes ---------------------------------------------------------------
#include "pmseek.h"

// --- Defines ----------------------------------------------------------------

// --- Typedefs ---------------------------------------------------------------

// --- PUBLIC / EXTERNAL Variables --------------------------------------------

// --- Variables --------------------------------------------------------------
int iHaveMMdlls = FALSE;

// --- Macros -----------------------------------------------------------------

// --- Function prototypes ----------------------------------------------------
static APIRET (* APIENTRY xLngLoadDialect)      (PCHAR nlsfile, PLNGHANDLE phandle);
static APIRET (* APIENTRY xLngCloseDialect)     (LNGHANDLE Handle);
static APIRET (* APIENTRY xLngGetString)        (LNGHANDLE handle, PCHAR searchkey, PCHAR target, ULONG target_maxlength);
static PSZ    (* APIENTRY xLngGetStringPointer) (LNGHANDLE handle, PSZ searchkey);

// --- Code -------------------------------------------------------------------

/******************************************************************************
int SwInit(void)

Description:
------------
Initializes National Language Support. Checks if ecolange.dll is present on the
system and get lange function addresses if yes. If no, Sw calls do not actually
translate but work with the standard given text strings.

Parameters:
-----------
none

Returns:
--------
0 if ecolange.dll successfully loaded and procedure entry points are found
any other value means no translation will be made

******************************************************************************/
int SwInit()
  {
  HMODULE hEcoLange = NULLHANDLE;         /* Module handle                     */
  APIRET rc;
  char cBuf2[256];
  UCHAR    LoadError[256] = "";           /* Area for Load failure information */

  rc = DosLoadModule(LoadError,                 /* Failure information buffer */
                     sizeof(LoadError),        /* Size of buffer             */
                     "ECOLANGE",               /* Module to load             */
                     &hEcoLange);              /* Module handle returned     */

  if ( (rc = DosQueryModuleHandle("ECOLANGE", &hEcoLange) ) != NO_ERROR )
    {
    TRACE("no ECOLANGE.DLL !!!!!!!");
    iHaveMMdlls = FALSE;
    }
  else
    {
    rc |= DosQueryProcAddr(hEcoLange,                    // Handle to module
                           0,                            // Ordinal of ProcName specified or NULL
                           "LngLoadDialect",             // ProcName ASCII or NULL
                           (PFN *)&xLngLoadDialect);     // Address returned
    TRACE2("rc=%d, xLngLoadDialect at 0x%X", rc, xLngLoadDialect );
    rc |= DosQueryProcAddr(hEcoLange,                    // Handle to module
                           0,                            // Ordinal of ProcName specified or NULL
                           "LngCloseDialect",             // ProcName ASCII or NULL
                           (PFN *)&xLngCloseDialect);     // Address returned
    TRACE2("rc=%d, xLngCloseDialect at 0x%X", rc, xLngCloseDialect );
    rc |= DosQueryProcAddr(hEcoLange,                    // Handle to module
                           0,                            // Ordinal of ProcName specified or NULL
                           "LngGetString",             // ProcName ASCII or NULL
                           (PFN *)&xLngGetString);     // Address returned
    TRACE2("rc=%d, xLngGetString at 0x%X", rc, xLngGetString );

    if ( !rc )
      {
      iHaveMMdlls = TRUE;

      rc = xLngLoadDialect("DataSeek.nls", &SwLange);
      TRACE1("rc=%d, xLngLoadDialect", rc);
      rc = xLngGetString(SwLange, "Language", (char*)cBuf, sizeof(cBuf));
      TRACE1("rc=%d, xLngGetString", rc);
      TRACE1("Language   : %s", cBuf);
      //TRACE1("via pointer: %s", LngTrans("Language") );
      memset (cBuf, 'x', sizeof(cBuf) );
      SwGet("translator_name", cBuf);
      sprintf(cBuf2, "'%.40s'", cBuf);
      SwGet("LANGUAGE_VERSION", cBuf);
      strcat (cBuf2, ", Version: ");
      strncat (cBuf2, cBuf, 20);
      SwGet("LANGUAGE_DATE", cBuf);
      strcat (cBuf2, ", Date: ");
      strncat (cBuf2, cBuf, 25);

      TRACE1("Translated by %s", cBuf2);
      }
    }

  return rc;
  }

/******************************************************************************
int SwClose(void)

Description:
------------
Releases ressources used by NLS functions.

Parameters:
-----------
none

Returns:
--------
ERROR_INVALID_HANDLE - incorrect handle and/or
Returns errors from DosFreeMem, DosClose, UniFreeUconvObject (see ecolange.h)
******************************************************************************/
int SwClose()
  {
  if ( iHaveMMdlls ) return xLngCloseDialect(SwLange);

  return -1;
  }

/******************************************************************************
int SwGet(PSZ pszString, char *szTranslation)

Description:
------------
Get (copies) translated string into szTranslation matching the source text
pointed to by pszString

Parameters:
-----------
PSZ pszString      ... pointer to text which should be translated
char *szTranslation... pointer to memory where the translation will be copied to

Returns:
--------
value returned by LangeGetString (not described in current ecolange
documentation)
******************************************************************************/
APIRET SwGetTranslation(PSZ pszString, char *szTranslation, int iMaxLenght)
  {
  APIRET rc = NO_ERROR;

  if ( iHaveMMdlls )
    {
    rc = xLngGetString(SwLange, pszString, szTranslation, iMaxLenght);
    TRACE5("rc=%d, MaxLen=%4d, Len=%4d, '%s' to '%s'", rc, iMaxLenght, strlen(szTranslation), pszString, szTranslation);
    szTranslation[iMaxLenght - 1] = '\0';
    }
  else
    {
    strncpy(szTranslation, pszString, iMaxLenght);
    szTranslation[iMaxLenght - 1] = '\0';
    }

  return rc;
  }

