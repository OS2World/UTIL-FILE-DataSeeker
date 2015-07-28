/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (c) 2014 Andreas Buchinger

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

#define REFRESH_DRIVES_TIME_IN_MS   3000

#include "pmseek.h"

// variables -----------------------------------------------------------------
static HAB hab;

// prototypes ---------------------------------------------------------------

int IsMediaRemoveable(ULONG drive);
int IsACdRomDrive ( unsigned long drive );
int FillDriveInfo(VOID);
void DriveInfoThread (void *arg);
MRESULT EXPENTRY DriveInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// code ---------------------------------------------------------------

int StartFillInfoThread(void)
  {

  // 20100415 only if MM dlls are found
  if ( !g.iHaveMMdlls ) return -1;

  g.DriveInfoTid = _beginthread(DriveInfoThread, NULL, 8192, NULL);   // 20091118 AB changed to _beginthread

  return TRUE;
  }


void DriveInfoThread (void *arg)
  {
  EXCEPTIONREGISTRATIONRECORD exRegRec;
  QMSG qmsg;
  ERRORID  erridErrorCode;/* last error id code                   */
  PERRINFO  perriErrorInfo;/* error info structure                */
  HMQ hmq;

  // 20100415 only if MM dlls are found
  if ( !g.iHaveMMdlls ) return;

  LoadExceptq(&exRegRec, NULL, NULL);

  TRACE("DriveInfo thread");

  hmq = WinCreateMsgQueue(hab = WinInitialize(0), 0);

  if ( !hmq )
    {
    WinTerminate(hab);
    TRACE("WinTerminate");
    }


  if ( !WinRegisterClass(  hab,
                           "DriveInfoThread",
                           DriveInfoProc,
                           0L,         //CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                           0           // extra memory to reserve for QWL_USER
                        ) )
    {
    TRACE("Error WinRegisterClass");
    }
  else
    {
    //      TRACE("WinRegisterClass okay");
    }




  g.hwndDriveInfo = WinCreateWindow (HWND_OBJECT,
                                     "DriveInfoThread", "",
                                     0, 0, 0, 0, 0,
                                     HWND_OBJECT,
                                     HWND_BOTTOM,
                                     0, NULL, NULL );

  TRACE1( "Starting DriveInfo message handler 0x%0X", g.hwndDriveInfo);
  while ( WinGetMsg ( hab, &qmsg, 0, 0, 0 ) ) WinDispatchMsg ( hab, &qmsg );

  TRACE( "Destroy DriveInfo window & message queue");
  WinDestroyWindow( g.hwndDriveInfo );
  WinDestroyMsgQueue( hmq );
  WinTerminate (hab);

  UninstallExceptq(&exRegRec);
  return;
  }

MRESULT EXPENTRY DriveInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {

  MRESULT mrc = 0;

  switch ( msg )
    {
    static ULONG ulTimerId;

    case WM_CREATE:
      TRACE("WM_CREATE DriveInfo");
      WinPostMsg(hwnd, WM_DRIVE_UPDATE, 0, 0);
      break;

    case WM_DRIVE_UPDATE:
      TRACE("WM_DRIVE_UPDATE");
      FillDriveInfo();
      WinPostMsg(g.hwndWinDrives, WM_NEW_DRIVE_INFO, 0, 0);
      break;

    case WM_TIMER:
      TRACE("DriveInfo WM_TIMER");
      ulTimerId = WinStartTimer(hab, hwnd, 0, REFRESH_DRIVES_TIME_IN_MS);
      WinPostMsg(hwnd, WM_DRIVE_UPDATE, 0, 0);
      break;

    case WM_DRIVE_START_TIMER:
      ulTimerId = WinStartTimer(hab, hwnd, 0, REFRESH_DRIVES_TIME_IN_MS);
      break;

    case WM_DRIVE_STOP_TIMER:
      TRACE("DriveInfo WM_DRIVE_STOP_TIMER");
      WinStopTimer(hab, hwnd, ulTimerId);
      break;

    case WM_TEST:
      TRACE( "WM_TEST received");
      break;

    case WM_DESTROY:
      WinStopTimer(hab, hwnd, ulTimerId);
      TRACE( "WM_DESTROY");
      mrc = 0;
      TRACE("done");
      break;

    default:
      mrc = WinDefWindowProc (hwnd, msg, mp1, mp2);
    }

  return mrc; //mrc;
  }


int FillDriveInfo(VOID)
  {

  UCHAR       sTemp[20];
  ULONG       uDrvNo;
  APIRET      apirc, ret;

  BYTE        fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
  ULONG       cbBuffer;       /* Buffer length) */
  PFSQBUFFER2 p2 = (PFSQBUFFER2) fsqBuffer;
  FSINFO      fsinfo2;
  FSALLOCATE  fsinf;

  for ( uDrvNo = 'A'-'A' + 1; uDrvNo <= 'Z'-'A' + 1; uDrvNo++ )
    {
    DriveInfo[uDrvNo].szDisplayString[0] = '-';
    DriveInfo[uDrvNo].szDisplayString[1] = ' '; //0xB3; // vertical line
    DriveInfo[uDrvNo].szDisplayString[2] = ' ';
    DriveInfo[uDrvNo].szDisplayString[3] = uDrvNo + 'A' - 1;
    DriveInfo[uDrvNo].szDisplayString[4] = ':';
    DriveInfo[uDrvNo].szDisplayString[5] = ' ';
    DriveInfo[uDrvNo].szDisplayString[6] = ' ';
    DriveInfo[uDrvNo].szDisplayString[7] = '\0';
    sprintf (sTemp, "%c:", uDrvNo + 'A' - 1);
    cbBuffer = sizeof(fsqBuffer);
    DosError(FERR_DISABLEHARDERR);
    apirc = DosQueryFSAttach(sTemp, 0, FSAIL_QUERYNAME, p2, &cbBuffer);
    //TRACE1(" apirc=%4d ", apirc);
    if ( apirc == NO_ERROR )
      {
      //TRACE1("%-8s ", p2->szName + p2->cbName + 1);
      if ( p2->cbFSDName > sizeof(DriveInfo[uDrvNo].fsName)-1 ) p2->cbFSDName = sizeof(DriveInfo[uDrvNo].fsName)-1 ;
      memcpy(DriveInfo[uDrvNo].fsName, p2->szName + p2->cbName + 1, p2->cbFSDName);
      DriveInfo[uDrvNo].fsName[p2->cbFSDName] = '\0';

      switch ( p2->iType )
        {
        case FSAT_CHARDEV:   // Resident character device
          DriveInfo[uDrvNo].type = DT_FLOPPY;
          break;

        case FSAT_PSEUDODEV: // Pusedu-character device
          if ( IsMediaRemoveable(uDrvNo) )
            DriveInfo[uDrvNo].type = DT_REMOVABLE;
          else
            DriveInfo[uDrvNo].type = DT_RAMDRIVE;
          break;

        case FSAT_LOCALDRV:  // Local drive
          if ( !strncmp (DriveInfo[uDrvNo].fsName, "CDFS", 4) )
            DriveInfo[uDrvNo].type = DT_CDROM;
          else
            if ( IsMediaRemoveable(uDrvNo) )
            DriveInfo[uDrvNo].type = DT_FLOPPY;
          else
            DriveInfo[uDrvNo].type = DT_HARDDISK;
          break;

        case FSAT_REMOTEDRV: // Remote drive attached to FSD
          if ( !strncmp (DriveInfo[uDrvNo].fsName, "RAMFS", 5) )
            DriveInfo[uDrvNo].type = DT_RAMDRIVE;
          else if ( !strncmp (DriveInfo[uDrvNo].fsName, "CDWFS", 5) )
            DriveInfo[uDrvNo].type = DT_RSJ;
          else
            DriveInfo[uDrvNo].type = DT_NETWORK;
          break;
        }

      DosError(FERR_DISABLEHARDERR);
      apirc = DosQueryFSInfo(uDrvNo, FSIL_VOLSER, &fsinfo2, sizeof(fsinfo2));
      if ( apirc == NO_ERROR )
        {
        ULONG* date = (ULONG*) &fsinfo2.fdateCreation;
        ULONG* time = (ULONG*) &fsinfo2.ftimeCreation;
        DriveInfo[uDrvNo].serialNo = (*time << 16) | *date;
        strcpy (DriveInfo[uDrvNo].volumeName, fsinfo2.vol.szVolLabel);
        if ( !strncmp (DriveInfo[uDrvNo].volumeName, "OS2VDISK", 4) )
          DriveInfo[uDrvNo].type = DT_RAMDRIVE;

        DosError(FERR_DISABLEHARDERR);
        apirc = DosQueryFSInfo(uDrvNo, FSIL_ALLOC, (char*) &fsinf, sizeof(fsinf));
        if ( apirc == NO_ERROR )
          {
          DriveInfo[uDrvNo].isAvailable = TRUE;
          DriveInfo[uDrvNo].bytesPerSector = fsinf.cbSector;
          DriveInfo[uDrvNo].numSectorsPerUnit = fsinf.cSectorUnit;
          DriveInfo[uDrvNo].numUnits = fsinf.cUnit;
          DriveInfo[uDrvNo].numAvailUnits = fsinf.cUnitAvail;
          DriveInfo[uDrvNo].diskSize = (double) DriveInfo[uDrvNo].bytesPerSector * (double) DriveInfo[uDrvNo].numSectorsPerUnit * (double) DriveInfo[uDrvNo].numUnits;
          DriveInfo[uDrvNo].diskFree = (double) DriveInfo[uDrvNo].bytesPerSector * (double) DriveInfo[uDrvNo].numSectorsPerUnit * (double) DriveInfo[uDrvNo].numAvailUnits;

          if ( !strncmp (DriveInfo[uDrvNo].fsName, "FAT", 4) )
            DriveInfo[uDrvNo].bytesPerCluster = (unsigned int)(DriveInfo[uDrvNo].diskSize / 0xFFFF);
          else
            if ( !strncmp (DriveInfo[uDrvNo].fsName, "HPFS", 4) )
            DriveInfo[uDrvNo].bytesPerCluster = 512;
          else
            DriveInfo[uDrvNo].bytesPerCluster = 512; // TODO: ???
          }
        }
      }
    else
      {
      /*            //printf("%8s ", " ");
            if ( IsACdRomDrive(uDrvNo) )
                      {
                      DriveInfo[uDrvNo].type = DT_CDROM;
                      ret = NO_ERROR;
                      }
                  else
                      if ( apirc == ERROR_NOT_READY  || apirc == ERROR_INVALID_DRIVE || apirc == ERROR_BAD_UNIT )
                      {
      */
      DriveInfo[uDrvNo].type = DT_UNKNOWN; // 20100314 do not show invalid drives anymore //FLOPPY; // Assume floppy with no drive in it
      }
    /*            else
                    {
                    ret = apirc;
                    }
                }
    */
    //      if ( !DriveInfo[uDrvNo].type )
    //            {
    //            if ( IsANetworkDrive( (int) uDrvNo) )
    //                {
    //                printf(" * ");
    //                }
    //            }
    //sprintf (sTemp, "%c:\\*", (char) (uDrvNo + 'A'));
    DriveInfo[uDrvNo].szDisplayString[0] = caDriveTypeShort[DriveInfo[uDrvNo].type];
    strcat(DriveInfo[uDrvNo].szDisplayString, DriveInfo[uDrvNo].volumeName);
    //TRACE3("Drv:%3d, type=%d, %s",uDrvNo, p2->iType, DriveInfo[uDrvNo].szDisplayString);
    }

  DosError(FERR_ENABLEHARDERR);
  return ret;
  }


int IsACdRomDrive ( unsigned long drive )
  {

#pragma pack(1)
  struct
    {
    UCHAR ucCommand;
    UCHAR ucDrive;
    } parms;
#pragma pack()

  BIOSPARAMETERBLOCK pb;
  ULONG cbParams = sizeof(parms);
  ULONG cbData = sizeof(pb);
  APIRET rc;

  parms.ucCommand = 0;    // 0 = return standard media
  parms.ucDrive = (UCHAR) (drive-1);

  memset(&pb, 0, sizeof(pb));
  rc = DosDevIOCtl((HFILE) -1, IOCTL_DISK, DSK_GETDEVICEPARAMS,
                   &parms, cbParams, &cbParams,
                   &pb, cbData, &cbData);


  if ( (drive < 'A' - 'A' + 1) || (drive > 'Z' - 'A' + 1) )
    {
    //printf(" wrong drive number (%d)", drive);
    return FALSE;
    }

  if ( rc != NO_ERROR ) return FALSE;
  else
    if ( pb.bDeviceType == 7 && // "other"
         pb.usBytesPerSector == 2048 &&
         pb.usSectorsPerTrack == (USHORT) -1 )
    return TRUE;
  else
    return FALSE;
  }

// ------ below not used
int IsANetworkDrive ( int drive )
  {
  #define BUFLEN  4096

  char szName[32];
  BOOL ret = FALSE;
  ULONG ulLen = BUFLEN;
  char buf[BUFLEN];
  APIRET apirc;

  DosError(FERR_DISABLEHARDERR);
  sprintf(szName, "%c:", drive + 'A' - 1);
  apirc = DosQueryFSAttach(szName, 0, FSAIL_QUERYNAME, (FSQBUFFER2*) buf, &ulLen);
  if ( apirc == NO_ERROR )
    {
    FSQBUFFER2 *p2 = (FSQBUFFER2 *) buf;
    ret = (p2->iType == FSAT_REMOTEDRV);
    }
  DosError(FERR_ENABLEHARDERR);
  return ret;
  }

int IsMediaRemoveable(ULONG drive)
  {
#pragma pack(1)
  typedef struct _QRYINP
    {
    BYTE CmdInfo;
    BYTE DriveUnit;
    }
  QRYINP, *PQRYINP;
#pragma pack()

  QRYINP QueryInfo;
  BIOSPARAMETERBLOCK DiskData;
  ULONG ulParm;
  ULONG ulData;
  APIRET apirc;

  QueryInfo.CmdInfo = 0;
  QueryInfo.DriveUnit = (BYTE) (drive - 1);
  DosError (FERR_DISABLEHARDERR);
  apirc = DosDevIOCtl((HFILE)  -1,
                      (ULONG)  IOCTL_DISK,
                      (ULONG)  DSK_GETDEVICEPARAMS,
                      (void*)  &QueryInfo,
                      (ULONG)  sizeof(QRYINP),
                      (PULONG) &ulParm,
                      (void*)  &DiskData,
                      (ULONG)  sizeof(BIOSPARAMETERBLOCK),
                      (PULONG) &ulData);
  DosError(FERR_ENABLEHARDERR);
  if ( apirc != NO_ERROR )
    return FALSE;
  else
    if ( !(DiskData.fsDeviceAttr & 0x0001) )
    return TRUE;
  else
    return FALSE;
  }


int IsDriveValid(ULONG curDrive)
  {
  ULONG   ulDriveNum   = 0;      /* Drive number (A=1, B=2, C=3, ...)    */
  ULONG   ulDriveMap   = 0;      /* Mapping of valid drives              */
  ULONG   i            = 0;      /* A loop index                         */
  APIRET  rc           = NO_ERROR;  /* Return code                       */

  DosError (FERR_DISABLEHARDERR);
  rc = DosQueryCurrentDisk (&ulDriveNum, &ulDriveMap);

  DosError(FERR_ENABLEHARDERR);
  if ( rc != NO_ERROR )
    {
    //printf ("DosQueryCurrentDisk error : return code = %u\n", rc);
    return 1;
    }
  //  printf ("Current disk = %c\n", 'A' + ulDriveNum - 1);
  //  printf ("Logical disks: ");
  //  printf ("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n");
  //  printf ("        valid? ");

  /* Each bit in the ulDriveMap corresponds to a specific logical drive.
     bit 0 = A:, bit 1 = B:, ... , bit 24 = Y:, bit 25 = Z:
     For each drive, shift the bit string to the left to get rid of
     the bits before the one we want, then shift that result right 31
     bits to leave just the one we are interested in. */

  for ( i = 0; i < 26; i++ )
    {
    //      printf (( (ulDriveMap<<(31-i)) >> 31) ? "Y " : "- ");
    }
  //  printf ("\n");                          /* Print a newline character */
  //  printf(" curDrv: %d DrvMap: 0x%0X ", curDrive, (unsigned int) ulDriveMap);
  if ( (ulDriveMap<<(31-curDrive + 1)) >> 31 )
    {
    return FALSE;
    }
  return TRUE;
  }

