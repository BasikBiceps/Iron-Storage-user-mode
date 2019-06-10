#ifndef _IRON_STORAGE_
#define _IRON_STORAGE_

#include "Windows.h"

#define IRON_STORAGE_POOL_TAG 'ksiD'

#ifndef __T
#ifdef _NTDDK_
#define __T(x)  L ## x
#else
#define __T(x)  x
#endif
#endif

#ifndef _T
#define _T(x)   __T(x)
#endif

#define DEVICE_BASE_NAME    _T("\\IronStorage")
#define DEVICE_DIR_NAME     _T("\\Device")      DEVICE_BASE_NAME
#define DEVICE_NAME_PREFIX  DEVICE_DIR_NAME     DEVICE_BASE_NAME

#define IOCTL_IRON_STORAGE_OPEN_FILE   CTL_CODE(FILE_DEVICE_DISK, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_IRON_STORAGE_CLOSE_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_IRON_STORAGE_QUERY_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)

#define PASSWORD_LENGTH 32

typedef struct _OPEN_FILE_INFORMATION {
	LARGE_INTEGER   FileSize;
	BOOLEAN         ReadOnly;
	UCHAR           DriveLetter;
	USHORT          FileNameLength;
	USHORT			PasswordLength;
	CHAR			Password[PASSWORD_LENGTH];
	CHAR            FileName[1];
} OPEN_FILE_INFORMATION, *POPEN_FILE_INFORMATION;

int IronStorageDiskMount(int DeviceNumber, POPEN_FILE_INFORMATION  OpenFileInformation);
int IronStorageDiskUnMount(char DriveLetter);
POPEN_FILE_INFORMATION IronStorageDiskStatus(char DriveLetter);

#endif
