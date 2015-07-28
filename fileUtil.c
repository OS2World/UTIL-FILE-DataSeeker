/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// fileUtil.c : basic file management.
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#include "pmseek.h"

// definitions --------------------------------------------------------------
#define MKOPENFLAG(_flag_)  \
   ((_flag_) & (FMO_CREATE | FMO_IFEXIST | FMO_REPLACE))
#define MKOPENMODE(_flag_) \
   (((_flag_) & 0x7fff0000) >> 16 | OPEN_FLAGS_SEQUENTIAL)  // 20090213 AB OPEN_FLAGS_SEQUENTIAL added (ecomstation.misc)
// no real advatage on my JFS partitions, but should not hurt


// prototypes ---------------------------------------------------------------
LONG scanTree(PSCANTREE ppt);
LONG scanDir(PSCANTREE ppt);

// globals ------------------------------------------------------------------

//===========================================================================
// Returns TRUE if the file pszFile exists.
// Parameters --------------------------------------------------------------
// PSZ pszFile : name of the file whose existence must be checked.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (existent/unexistent file)
//===========================================================================

BOOL fmExists(PSZ pszFile)
  {
  HDIR hdir = HDIR_CREATE;
  FILEFINDBUF3L ffb;
  ULONG ul = 1;
  TRACE("fmExists");
  if ( !xDosFindFirst(pszFile, &hdir, FILEATTR_ANY | FILE_DIRECTORY, &ffb,
                      sizeof(ffb), &ul, FIL_STANDARDL) )
    {    // 20090615 LFS
    DosFindClose(hdir);
    return TRUE;
    } /* endif */
  return FALSE;
  }


//===========================================================================
// Scan a tree of directories and files matching the specific defined via
// 'pszFileSpec' and the 'flag' attributes. For each found directory and
// file optionally execute a callback procedure.
// Parameters --------------------------------------------------------------
// PSZ pszFileSpec : file specifics
// ULONG flag      : file attributes plus recursion flag
// PVOID pUserData : user defined data
// PSCANTREEFN pfFile    :
// PSCANTREEFN pfDirPre  :
// PSCANTREEFN pfDirPost :
// Return value ------------------------------------------------------------
// LONG : if >= 0 number of processed files
//        if <  0 Dos* API return code (* -1)
//===========================================================================

LONG fmTreeScan(PSZ pszFileSpec, ULONG flag, PVOID pUserData, HEV hev,
                PSCANTREEFN pfFile, PSCANTREEFN pfDirPre, PSCANTREEFN pfDirPost)
  {
  SCANTREE st;
  LONG rc;
  DosError(FERR_DISABLEHARDERR);
  while ( *pszFileSpec == ' ' )       // 20081119 removing leading blanks
    {
    pszFileSpec++;
    }
  TRACE1("pszFileSpec=%s", pszFileSpec);
  rc = (LONG)xDosQueryPathInfo(pszFileSpec, FIL_QUERYFULLNAME,        // 20090615 LFS
                               st.achPath, sizeof(st.achPath));
  DosError(FERR_ENABLEHARDERR);
  if ( rc )
    {
    TRACE1("rc = %d", rc);
    return -rc;
    }
  st.pUserData = pUserData;
  st.hev = hev;
  st.pfFile = pfFile;
  st.pfDirPre = pfDirPre;
  st.pfDirPost = pfDirPost;
  // find the last slash before the file name
  pszFileSpec = strrchr(st.achPath, '\\') + 1;
  st.cbPath = pszFileSpec - st.achPath;
  strcpy(st.achMask, pszFileSpec);
  // if must recur through the subdirectories
  if ( flag & SCANTREE_RECUR )
    {
    strcpy(pszFileSpec, "*");
    st.flag = flag & 0xffff;
    return scanTree(&st);
    // must scan just the current directory
    }
  else
    {
    st.flag = flag;
    return scanDir(&st);
    } /* endif */
  }


//===========================================================================
// Recursively scan a tree of directories, optionally executing the callback
// procedures for each found file and directory.
// Parameters --------------------------------------------------------------
// PSCANTREE pst : tree scansion data
// Return value ------------------------------------------------------------
// LONG : >= 0 count of processed file
//        <  0 Dos* API return code as a negative number or application
//             defined return code.
//===========================================================================
static LONG scanTree(PSCANTREE pst)
  {
  HDIR hDir = HDIR_CREATE;
  ULONG ul = 1;
  LONG cfile = 0;             // processed files count
  LONG res = 0;               // callback procedure result
  ULONG cbPath = pst->cbPath; // current path length including trailing '\\'
  // inizia scansione
  res = -xDosFindFirst(pst->achPath, &hDir, FILEATTR_ANY | FILE_DIRECTORY,
                       &pst->ff, sizeof(pst->ff), &ul, FIL_STANDARDL);      // 20090615 LFS
  if ( res ) return res;
  do
    {
    if ( pst->hev &&
         (ERROR_TIMEOUT == DosWaitEventSem(pst->hev, SEM_IMMEDIATE_RETURN)) )
      break;
    //TRACE1("%s", pst->achPath);
    if ( pst->ff.attrFile & FILE_DIRECTORY )
      {
      // skip current (".") and parent ("..") directories
      if ( fmIsNotCurDir(pst->ff.achName) && fmIsNotParDir(pst->ff.achName) )
        {
        // append the current directory name to the path
        ul = pst->ff.cchName;
        memcpy(pst->achPath + cbPath, pst->ff.achName, ul + 1);
        // calls the first process-directory procedure
        if ( pst->pfDirPre )
          {
          if ( (res = pst->pfDirPre(pst)) < 0 ) goto error;
          cfile += res;
          } /* endif */
        // append "\*" to the current directory name and recur
        memcpy(pst->achPath + cbPath + ul, "\\*", 3);
        pst->cbPath = cbPath + ul + 1;
        if ( (res = scanTree(pst)) < 0 ) goto error;
        pst->cbPath = cbPath;
        cfile += res;
        // calls the second process-directory procedure
        if ( pst->pfDirPost )
          {
          if ( (res = pst->pfDirPost(pst)) < 0 ) goto error;
          cfile += res;
          } /* endif */
        memcpy(pst->achPath + cbPath, "*", 2);
        ul = 1;
        } /* endif */
      }
    // if it is a file matching the required specifications calls pfFile
    else if ( (((pst->ff.attrFile & (pst->flag >> 8)) == (pst->flag >> 8)) && !(pst->ff.attrFile & ~(pst->flag & SCANTREE_ATTRMASK))
               && szWildCardMatchFileName(pst->achMask, pst->ff.achName)) )
      {
      if ( pst->pfFile )
        {
        if ( (res = pst->pfFile(pst)) < 0 ) goto error;
        cfile += res;
        } /* endif */
      } /* endif */

    if ( g.setting & OPT_FIND_DIR &&  pst->ff.attrFile & FILE_DIRECTORY && szWildCardMatchDirName(pst->ff, pst->achPath, pst->achMask) ) // check if directory match
      {
      TRACE1("------------- match dir=%s", pst->achPath);
      if ( pst->pfFile )    // if procedure for found files is specified then call it
        {
        if ( (res = pst->pfFile(pst)) < 0 ) goto error;
        cfile += res;
        } /* endif */
      }
    } while ( !xDosFindNext (hDir, &pst->ff, sizeof(pst->ff), &ul) );  // 20090615 LFS
  pst->cbPath = cbPath;
  goto end;
  error:
  cfile = res;
  end:
  DosFindClose(hDir);
  return cfile;
  }


//===========================================================================
//
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// VOID
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
// VOID
//===========================================================================
static
            LONG scanDir(PSCANTREE pst)
  {
  HDIR hDir = HDIR_CREATE;
  ULONG ul = 1;
  LONG cfile = 0;             // processed files count
  LONG res = 0;               // callback procedure result
  // inizia scansione
  TRACE1("scandir pst=0x%X", pst);
  res = -xDosFindFirst(pst->achPath, &hDir, pst->flag & 0xffff,
                       &pst->ff, sizeof(pst->ff), &ul, FIL_STANDARDL);  // 20090615 LFS
  if ( res ) return res;
  do
    {
    TRACE2("hDir: %d (at 0x%0X)", hDir, &hDir);
    if ( pst->hev &&
         (ERROR_TIMEOUT == DosWaitEventSem(pst->hev, SEM_IMMEDIATE_RETURN)) )
      break;
    // skip current (".") and parent ("..") directories
    if ( (!(pst->ff.attrFile & FILE_DIRECTORY) ||
          (fmIsNotCurDir(pst->ff.achName) && fmIsNotParDir(pst->ff.achName)))
         && szWildCardMatchFileName(pst->achMask, pst->ff.achName) )
      {
      if ( pst->pfFile )
        {
        TRACE1("found %s", pst->ff.achName);
        if ( (res = pst->pfFile(pst)) < 0 ) goto error;
        cfile += res;
        } /* endif */
      } /* endif */
    } while ( NO_ERROR == xDosFindNext (hDir, &pst->ff, sizeof(pst->ff), &ul) );  // 20090615 LFS
  goto end;
  error:
  cfile = res;
  end:
  DosFindClose(hDir);
  return cfile;
  }


//===========================================================================
// DosOpen() wrapper. It just admits the most common flags.
// Parameters --------------------------------------------------------------
// PSZ pszFile : file to be opened.
// ULONG flag  : open flag.
//    Creation flags (mutually exclusive)
//      FMO_FAILIFNEW   return FMO_ERROR if the file does not exist.
//      FMO_CREATE      create the file if it does not exist.
//    Opening flags (mutually exclusive)
//      FMO_NOTIFEXIST  return FMO_ERROR if the file already exists.
//      FMO_IFEXIST     open the file if it exists.
//      FMO_REPLACE     replace the file if it exists.
//    Sharing flags (mutually exclusive)
//      FMO_SHARENONE   do not allow other processes to access the file.
//      FMO_SHAREREAD   allow other processes to access the file for reading only.
//      FMO_SHAREWRITE  allow other processes to access the file for writing only.
//      FMO_SHAREALL    allow other processes unrestricted access to the file.
//    Access flags (mutually exclusive)
//      FMO_READ        open the file for reading.
//      FMO_WRITE       open the file for writing.
//      FMO_READWRITE   open the file for reading and writing.
//      FMO_APPEND      open the file and move the file pointer to the file end.
// Return value ------------------------------------------------------------
// HFILE : file handle, FMO_ERROR (-1) in case of error.
//===========================================================================

HFILE fmOpen(PSZ pszFile, ULONG flag)
  {     // used only for .ini file, so LSF is not necessary
  HFILE hf;                                // ################################################
  ULONG ul;
  if ( xDosOpen(pszFile, &hf, &ul, 0, 0, MKOPENFLAG(flag), MKOPENMODE(flag), NULL) ) // 20090615 LFS
    return FMO_ERROR;
  if ( (flag & FMO_APPEND) && DosSetFilePtr(hf, 0, FILE_END, &ul) )
    {                // not LFS 20090615 LFS
    DosClose(hf);
    return FMO_ERROR;
    } /* endif */
  return hf;
  }


//===========================================================================
// Close a file opened via DosOpen() or fmOpen().
// Parameters --------------------------------------------------------------
// HFILE hf : file handle.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL fmClose(HFILE hf)
  {
  return !DosClose(hf);
  }


//===========================================================================
// DosSetFilePtr() wrapper. Change the position of a file read/write pointer.
// Parameters --------------------------------------------------------------
// HFILE hf      file handle.
// LONG offset   new pointer position.
// ULONG flag    offset relativity: FILE_BEGIN (beginning of the file),
//               FILE_CURRENT (current pointer position), FILE_END
//               (end of file).
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

// ToDo: LFS        // used only for .ini file, so LSF is not necessary
// ################################################
BOOL fmPtrSet(HFILE hf, LONG offset, ULONG flag)
  {
  return !DosSetFilePtr(hf, offset, flag, (PULONG)&offset);    // 20090615 not LFS
  }


//===========================================================================
// Read data from a file to a buffer.
// Parameters --------------------------------------------------------------
// HFILE hf      : file handle.
// PVOID pBuffer : output buffer.
// ULONG cb      : buffer size.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

// ToDo: LFS        // used only for .ini file, so LSF is not necessary
// ################################################
BOOL fmRead(HFILE hf, PVOID pBuffer, ULONG cb)
  {
  return !DosRead(hf, pBuffer, cb, &cb);                       // not LFS
  }


//===========================================================================
// Write data from a buffer to file.
// Parameters --------------------------------------------------------------
// HFILE hf      : file handle.
// PVOID pBuffer : output buffer.
// ULONG cb      : buffer size.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

// ToDo: LFS        // used only for .ini file, so LSF is not necessary
// ################################################
BOOL fmWrite(HFILE hf, PVOID pBuffer, ULONG cb)
  {
  return !DosWrite(hf, pBuffer, cb, &cb);                  // not LFS // 20090615 LFS
  }


//===========================================================================
// Find the name of a file/directory in a text string optionally containing
// specifications of the file path. ':', '\' and '/' are accepted as valid
// path separators.
// Parameters --------------------------------------------------------------
// PSZ pszFullFileName : full file name.
// Return value ------------------------------------------------------------
// PSZ : address of the file name.
//===========================================================================

PSZ fmFileNameFromPath(PSZ pszFullFileName)
  {
  PSZ p = pszFullFileName + strlen(pszFullFileName);
  while ( p-- > pszFullFileName )
    {
    if ( (*p == '\\') || (*p == '/') || (*p == ':') )
      return p + 1;
    } /* endwhile */
  // pszFullFileName does not contain any path specification
  return pszFullFileName;
  }


//===========================================================================
// Replaces the extension of 'pszFileName' with 'pszExt'. If 'pszFileName'
// has no extension, .'pszExt' is appended at the end of 'pszFileName'.
// Parameters --------------------------------------------------------------
// PSZ pszFileName : name of file whose extension hat to be changed.
// PSZ pszExt      : new file extension.
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID fmFileNameEditExt(PSZ pszFileName, PSZ pszExt)
  {
  ULONG cb;
  PSZ p;
  cb = strlen(pszFileName);
  p = pszFileName + cb;
  if ( *pszExt == '.' ) ++pszExt;
  while ( p-- > pszFileName )
    {
    if ( *p == '.' ) goto copyExt;
    if ( (*p == '\\') || (*p == '/') || (*p == ':') )
      break;
    } /* endwhile */
  pszFileName[cb] = '.';
  p = pszFileName + cb;
  copyExt:
  strcpy(p + 1, pszExt);
  }


//===========================================================================
// Compare case insensitively the file name specified in 'pszPath' (which
// can optionally include the full path) to the file name (not including
// the path) specified in 'pszName'.
// Parameters --------------------------------------------------------------
// PSZ pszPath : file name (optionally including path) to be checked.
// PSZ pszName : file name to check for.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (match/mismatch)
//===========================================================================

BOOL fmFileNameComp(PSZ pszPath, PSZ pszName)
  {
  TRACE2("path:'%s' name:'%s'", pszPath, pszName);
  return !szCompI(fmFileNameFromPath(pszPath), pszName);
  }

