#include "ndis_operate.h"
QString ndis_symbol = "\\\\.\\NdisFilterTEST";

NDIS_Operate::NDIS_Operate()
{
    DriverNDIS = NULL;
}

NDIS_Operate::~NDIS_Operate()
{
    CloseHandle(DriverNDIS);
}

//打开驱动
bool NDIS_Operate::openDriver()
{
    DriverNDIS = CreateFileW( ndis_symbol.toStdWString().data(),
                             GENERIC_READ | GENERIC_WRITE,
                             0, 0, OPEN_EXISTING,
                             FILE_ATTRIBUTE_SYSTEM, 0);

    if(DriverNDIS == INVALID_HANDLE_VALUE){
        return FALSE;
    }
    return TRUE;
}

//关闭驱动
void NDIS_Operate::closeNDIS()
{
    CloseHandle(DriverNDIS);
}

//清除底层黑名单
bool NDIS_Operate::clear()
{
    bool send = false;
    DWORD ret_len = 0;
    if (!DeviceIoControl(DriverNDIS, IOCTL_CLEAR_BLACK_LIST, NULL, 0, NULL, 0, (DWORD*)&ret_len, 0)){
        printf("NDIS Forbidden: Send message failed.\r\n");
    }else{
        printf("NDIS Forbidden: Send message successfully.\r\n");
        send = true;
    }
    CloseHandle(DriverNDIS);
    return send;
}

//禁止网络信息头
bool NDIS_Operate::forbiddenipPortMacUrlProto(BLACK_DATA BL)
{
    bool send = false;
    int ret_len = 0;

    if (!DeviceIoControl(DriverNDIS, IOCTL_SET_BLACK_LIST, &BL, sizeof(BLACK_DATA), NULL, 0, (DWORD*)&ret_len, 0)){
        printf("NDIS Forbidden: Send message failed.\r\n");
    }else{
        printf("NDIS Forbidden: Send message successfully.\r\n");
        send = true;
    }
    CloseHandle(DriverNDIS);
    return send;
}

//运行网络信息头
bool NDIS_Operate::deleteforbiddenipPortMacUrlProto(BLACK_DATA BL1)
{
    bool send = false;
    int ret_len = 0;

    if (!DeviceIoControl(DriverNDIS, IOCTL_DELETE_BLACK_LIST, &BL1, sizeof(BLACK_DATA), NULL, 0, (DWORD*)&ret_len, 0)){
        printf("NDIS Forbidden: Send message failed.\r\n");
    }else{
        send = true;
    }
    CloseHandle(DriverNDIS);
    return send;
}

