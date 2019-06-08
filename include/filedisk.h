#ifndef _FILE_DISK_
#define _FILE_DISK_
#include <Windows.h>
#define FILE_DISK_POOL_TAG 'ksiD'

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

#define DEVICE_BASE_NAME    _T("\\FileDisk")
#define DEVICE_DIR_NAME     _T("\\Device")      DEVICE_BASE_NAME
#define DEVICE_NAME_PREFIX  DEVICE_DIR_NAME     DEVICE_BASE_NAME

#define IOCTL_FILE_DISK_OPEN_FILE   CTL_CODE(FILE_DEVICE_DISK, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_CLOSE_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_QUERY_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _OPEN_FILE_INFORMATION {
	LARGE_INTEGER   FileSize;
	BOOLEAN         ReadOnly;
	UCHAR           DriveLetter;
	USHORT          FileNameLength;
	USHORT			PasswordLength;
	CHAR			Password[17];
	CHAR            FileName[1];
} OPEN_FILE_INFORMATION, *POPEN_FILE_INFORMATION;

int FileDiskSyntax(void);
void PrintLastError(char* Prefix);

int FileDiskMount(
    int                     DeviceNumber,
    POPEN_FILE_INFORMATION  OpenFileInformation,
    BOOLEAN                 CdImage
);

int FileDiskUmount(char DriveLetter);
OPEN_FILE_INFORMATION* FileDiskStatus(char DriveLetter);
#endif
