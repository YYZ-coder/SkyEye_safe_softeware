
#include "DriverOperation.h"

#define SERVICE_NAME "WFP_Filter"

//因为wfp驱动即便设置成自启也不自启，所以就用这种方式了
//每次开机打开一次服务
BOOL loadWFPService(){
	BOOL yes = false;
	SC_HANDLE hServiceMgr;
	SC_HANDLE hServiceDDK;
	do{
		//打开SCM
		hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (hServiceMgr == NULL){
			yes = false;
			break;
		}
		//打开服务
		hServiceDDK = OpenServiceA(hServiceMgr, SERVICE_NAME, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL){
			yes = false;
			break;
		}
		//开启服务
		BOOL s = StartServiceA(hServiceDDK, NULL, NULL);
		if (!s){
			yes = false;
			break;
		}
		yes = true;
	} while (FALSE);
	return yes;
}

//打开SSDT驱动
bool DriverOperation::openSSDT(){
	SSDT = CreateFile(SSDT_NAME,
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (SSDT == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	return TRUE;
}

//打开NDIS
bool DriverOperation::openNDIS(){
	NDIS = CreateFile(NDIS_NAME,
		GENERIC_READ | GENERIC_WRITE,
		0, 0, OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM, 0);
	if (NDIS == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	return TRUE;
}

//打开WFP
bool DriverOperation::openWFP(){
	do{
		//刚开始若打开成功则不需要再次加载Driver
		WFP = CreateFile(WFP_NAME,
			GENERIC_READ | GENERIC_WRITE,
			0, 0, OPEN_EXISTING,
			FILE_ATTRIBUTE_SYSTEM, 0);
		if (WFP != INVALID_HANDLE_VALUE){
			return TRUE;
		}
		//打开失败,说明刚开机,进行loadDriver
		if (loadWFPService()){
			WFP = CreateFile(WFP_NAME,
				GENERIC_READ | GENERIC_WRITE,
				0, 0, OPEN_EXISTING,
				FILE_ATTRIBUTE_SYSTEM, 0);
			if (WFP == INVALID_HANDLE_VALUE){
				return FALSE;
			}
			return TRUE;
		}
		else
			return FALSE;
	} while (FALSE);
}

//SSDT发送用户输入的用户名和密码
bool DriverOperation::sendSSDT_userAndpass(USER_PASSWORD up){
	int ret_len = 0;
	if (&up != NULL){
		if (!DeviceIoControl(SSDT, IOCTL_SSDT_SEND_UP, &up, sizeof(USER_PASSWORD), NULL, 0, (DWORD*)&ret_len, 0)){
			CloseHandle(SSDT);
			return FALSE;
		}
	}
	CloseHandle(SSDT);
	return TRUE;
}

//SSDT从驱动获得管理员用户名和密码
REG_TO_CP DriverOperation::getSSDT_userAndpass(){
	REG_TO_CP up;
	int ret_len = 0;
	//ZeroMemory(&up, sizeof(REG_TO_CP));
	ZeroMemory(&up.dbHost,sizeof(10));
	ZeroMemory(&up.dbName,sizeof(10));
	ZeroMemory(&up.sysAdmin,sizeof(10));
	ZeroMemory(&up.sysPassword,sizeof(20));

	if (openSSDT()){
		if (DeviceIoControl(SSDT, IOCTL_SSDT_GET_REG, NULL, 0, &up, sizeof(REG_TO_CP), (DWORD*)&ret_len, 0)){
			CloseHandle(SSDT);
			return up;
		}
		else
			MessageBoxA(NULL, "Info", "Get Data Failed!", 0);
		CloseHandle(SSDT);
	}
	else
		MessageBoxA(NULL,"Open Driver failed!","Info", 0);
	return up;
}

//清除三个驱动所有的驱动内规则信息
bool DriverOperation::clearAll(){
	DWORD ret_len = 0;
	//清空SSDT
	if (openSSDT()){
		if (!DeviceIoControl(SSDT, IOCTL_SSDT_CLAER_BLACK_ILST, NULL, 0, NULL, 0, (DWORD*)&ret_len, 0))
			MessageBoxA(NULL,"Clear SSDT Failed!","warning",0);
		CloseHandle(SSDT);
	}
	//清空WFP
	if (openWFP()){
		if (!DeviceIoControl(WFP, IOCTL_WFP_CLEAR_BLACK_LIST, NULL, 0, NULL, 0, (DWORD *)&ret_len, 0))
			MessageBoxA(NULL, "Clear WFP Failed!", "warning", 0);
		CloseHandle(WFP);
	}
	else{
		MessageBoxA(NULL, "Open WFP Failed!", "warning", 0);
	}
	//清空NDIS
	if (openNDIS()){
		if (!DeviceIoControl(NDIS, IOCTL_CLEAR_BLACK_LIST, NULL, 0, NULL, 0, (DWORD*)&ret_len, 0)){
			MessageBoxA(NULL, "Clear NDIS Failed!", "warning", 0);
		}
		CloseHandle(NDIS);
	}
	return true;
}

