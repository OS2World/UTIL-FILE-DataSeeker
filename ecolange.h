/****************************** Module Header ******************************
*
*   (c) Copyright eCoSoftware 2006-2009. All Rights Reserved
*
*   Module Name: LANGE.H
*
*   eComStation NLS include file.
*
\***************************************************************************/
#ifndef  LANGE_H_INCLUDED

#define  LANGE_H_INCLUDED

#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LNGHANDLE LHANDLE
typedef LNGHANDLE * PLNGHANDLE;

// Error codes
#define ERR_LNG_NO_ERROR             0
#define ERR_LNG_NO_KEY               1
#define ERR_LNG_NO_TARGET_BUFFER     2
#define ERR_LNG_NO_SEARCH_KEY        3

/* Loads language strings from pointed file and returns handle for future work with this strings.
 * Returns errors from
 * DosAllocMem, DosFreeMem, DosScanEnv, DosSearchPath,
 * DosOpen, DosRead, DosSetFilePtr, DosQueryCp, UniMapCpToUcsCp, UniCreateUconvObject,
 * and also:
 * ERROR_BAD_FORMAT   - incorrect language file format
 * ERROR_INVALID_NAME - incorrect dialect name
 * ERROR_NO_ITEMS     - dialect not found
 */
APIRET APIENTRY LngLoadDialect(PCHAR nlsfile, PLNGHANDLE phandle);

/* LngCloseDialect - Destroys all memory structures related to pointed handle.
 * Returns errors from DosFreeMem, DosClose, UniFreeUconvObject
 * and also:
 * ERROR_INVALID_HANDLE - incorrect handle
 */

APIRET APIENTRY LngCloseDialect(LNGHANDLE Handle);

/* LngGetString - returns string according to searchkey value.
 */
APIRET APIENTRY LngGetString(LNGHANDLE handle, PCHAR searchkey, PCHAR target, ULONG target_maxlength);

/*
 * LngGetStringPointer - returns pointer to string like in GNU gettext
 */
PSZ APIENTRY LngGetStringPointer(LNGHANDLE handle, PSZ searchkey);

/****************************************************************************/
/*                                                                          */
/* For easy approaches bellow macros allows to not use handles              */
/*                                                                          */
/****************************************************************************/

#ifdef LNG_DEFAULT_HANDLE

LHANDLE DefaultHandle;

#define LngDefaultInit(s)   LngLoadDialect(s, &LngDefaultHandle)
#define LngDefaultUnInit    LngCloseDialect(LngDefaultHandle)
#define LngTranslate(s)     LngGetStringPointer(LngDefaultHandle, s)

/****************************************************************************/
/*                                                                          */
/* For gettext compatability bellow macros can be used                      */
/*                                                                          */
/****************************************************************************/

#ifdef LNG_GETTEXT
#define gettext(s) LngTranslate(s)
#define dgettext(d,s) LngTranslate(s)
#define dcgettext(d,s,c) LngTranslate(s)

#define ngettext(s,s2,n) ((n)==1 ? LngTranslate(s) : LngTranslate(s2))
#define dngettext(d,s,s2,n) ((n)==1 ? LngTranslate(s) : LngTranslate(s2))
#define dcngettext(d,s,s2,n,c) ((n)==1 ? LngTranslate(s) : LngTranslate(s2))

#define bindtextdomain(Domainname, Dirname)
#define bind_textdomain_codeset(Domainname, Codeset)
#endif


#endif

/****************************************************************************/
/*                                                                          */
/* For OS/2 MSG API compatability bellow macros can be used                 */
/*                                                                          */
/****************************************************************************/

#ifdef LNG_MSGAPI
#endif

/****************************************************************************/
/*                                                                          */
/* All following functions are obsoled and not documented. Use at your risk */
/*                                                                          */
/****************************************************************************/

#define LANGE LHANDLE
typedef LANGE * PLANGE;
APIRET APIENTRY LngLoadLanguage(PCHAR nlsfile, PCHAR dialect, PLNGHANDLE phandle);
APIRET APIENTRY LngUnLoadLanguage(LNGHANDLE handle);
APIRET APIENTRY LngOpenDialect(PCHAR MsgFile, PLNGHANDLE Handle);
APIRET APIENTRY LngDumpKeysData(LNGHANDLE id);
#define LangeLoadLanguage LngLoadLanguage
#define LangeGetString LngGetString
#define LangeUnLoadLanguage LngUnLoadLanguage
#define LangeDumpKeysData LngDumpKeysData

#ifdef __cplusplus
        }
#endif


#endif /* LANGE_H_INCLUDED */
