#ifndef DRIVEROPERATION_H
#define DRIVEROPERATION_H

#include <windows.h>
#include "common.h"

#define SSDT_NAME	"\\\\.\\SSDTProcess"
#define WFP_NAME   "\\\\.\\wfp_device"
#define NDIS_NAME  "\\\\.\\NdisFilterTEST"

//SSDT
//发送用户输入的用户名和密码
#define IOCTL_SSDT_SEND_UP\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x850, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//从注册表读出安装信息：Register->CP
#define IOCTL_SSDT_GET_REG\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x853, METHOD_BUFFERED, \
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

//NDIS clear
#define FILE_DEVICE_COMM_DRIVER     0x00008811
#define IOCTL_CLEAR_BLACK_LIST \
	CTL_CODE(\
	FILE_DEVICE_COMM_DRIVER,\
	0x805, METHOD_BUFFERED, \
	FILE_ANY_ACCESS)
//WFP clear
#define IOCTL_WFP_CLEAR_BLACK_LIST\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x824, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)
//SSDT clear
#define IOCTL_SSDT_CLAER_BLACK_ILST\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x832, METHOD_BUFFERED, \
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)


//CP用SSDT从注册表读的数据
typedef struct _REG_TO_CP{
	CHAR dbName[10];
	CHAR dbHost[10];
	CHAR sysAdmin[10];
	CHAR sysPassword[20];
}REG_TO_CP, *PREG_TO_CP;

class DriverOperation{
private:
	HANDLE SSDT;

	HANDLE WFP;

	HANDLE NDIS;
public:

	DriverOperation(){ 
		SSDT = NULL; 
		NDIS = NULL;
		WFP = NULL;
	}
	~DriverOperation(){}

	bool openSSDT();

	bool openWFP();

	bool openNDIS();

	bool clearAll();

	bool sendSSDT_userAndpass(USER_PASSWORD up);

	REG_TO_CP getSSDT_userAndpass();
};

#endif