#ifndef PROCESSFORBID_H
#define PROCESSFORBID_H

#include <ntddk.h>
#include <stdlib.h>

#define PROCESSNAME_LENGTH 50

//用户发送需要禁止打开的进程并插入禁止进程链
#define IOCTL_SSDT_FORBIDPROCESS_ADD\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x854, METHOD_BUFFERED, \
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

//用户发送需要禁止打开的进程名并移除禁止进程链
#define IOCTL_SSDT_FORBIDPROCESS_DELETE\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x855, METHOD_BUFFERED, \
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

//存储需要禁止的进程名链
typedef struct _ForbidProcess{
	LIST_ENTRY listEntry;
	WCHAR		ProcessName[PROCESSNAME_LENGTH];
}ForbidProcess,*PForbidProcess;



BOOLEAN InsertForbidProcessList(PForbidProcess fP);

BOOLEAN RemoveForbidProcessList(PForbidProcess fP);

#endif