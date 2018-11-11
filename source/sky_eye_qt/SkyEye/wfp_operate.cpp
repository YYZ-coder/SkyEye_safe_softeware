#include "wfp_operate.h"

QString wfp_symbol = "\\\\.\\wfp_device";

WFP_Operate::WFP_Operate()
{
    DriverWFP = NULL;
}

WFP_Operate::~WFP_Operate()
{
}

//打开驱动
bool WFP_Operate::openDriver()
{
    DriverWFP = CreateFileW( wfp_symbol.toStdWString().data(),
                            GENERIC_READ | GENERIC_WRITE,
                            0, 0, OPEN_EXISTING,
                            FILE_ATTRIBUTE_SYSTEM, 0);
    if(DriverWFP == INVALID_HANDLE_VALUE){
        return FALSE;
    }
    return TRUE;
}

//关闭驱动
void WFP_Operate::closeWFP()
{
    CloseHandle(DriverWFP);
}

//清除黑名单链表
bool WFP_Operate::clear()
{
    bool send = false;
    int ret_len = 0;
    if (DeviceIoControl(DriverWFP, IOCTL_WFP_CLEAR_BLACK_LIST, NULL, 0, NULL, 0, (DWORD *)&ret_len, 0))
         send = true;
    CloseHandle(DriverWFP);
    return send;
}

//发送并开始监控事件
bool WFP_Operate::sendEvent(HANDLE g_hKernelEvent)
{
    bool send = false;
    int ret_len = 0;
    if (!DeviceIoControl(DriverWFP, IOCTL_WFP_START_EVENT, &g_hKernelEvent, sizeof(g_hKernelEvent), NULL, 0, (DWORD *)&ret_len, 0)){
        CloseHandle(g_hKernelEvent);
    }else send = true;
    CloseHandle(DriverWFP);
    return send;
}

//停止监控事件
bool WFP_Operate::stopEvent()
{
    bool send = false;
    int ret_len = 0;
    if (DeviceIoControl(DriverWFP, IOCTL_WFP_STOP_EVENT, NULL, 0, NULL, 0, (DWORD *)&ret_len, 0))
        send = true;
    CloseHandle(DriverWFP);
    return send;
}

//监视调用并返回数据
TRANSFER_R3 WFP_Operate::monitorWFPData()
{
    int ret_len = 0;
    TRANSFER_R3 tr;
    ZeroMemory(&tr,sizeof(TRANSFER_R3));

    if(openDriver()){
        if (DeviceIoControl(DriverWFP, IOCTL_WFP_MONITOR, NULL, 0, &tr, sizeof(TRANSFER_R3), (DWORD*)&ret_len, 0)){
        }
    }
    CloseHandle(DriverWFP);
    return tr;
}

//禁止进程
bool WFP_Operate::forbiddenProcess(
        QString processName, QString processPath)
{
    bool send = false;
    int ret_len = 0;
    RULE_FROM_R3 rfr;
    ZeroMemory(&rfr,sizeof(RULE_FROM_R3));

    //QString 转 BYTE[]
    QByteArray tmp = processName.toLatin1();
    char *mm = (char*)tmp.data();
    char nn[64];
    strcpy(nn,mm);
    memcpy(rfr.ImageName,nn,64);

    //QString 转 WCHAR[]
    processPath.toWCharArray(rfr.ProcessPath);

    if (!DeviceIoControl(DriverWFP, IOCTL_WFP_FORBIDDEN, &rfr, sizeof(RULE_FROM_R3), NULL, 0, (DWORD*)&ret_len, 0))
        printf("Forbidden: Send message failed.\r\n");
    else{
            printf("Forbidden: Send message successfully.\r\n");
            send = true;
    }
    CloseHandle(DriverWFP);
    return send;
}

//取消禁止进程
bool WFP_Operate::deleteForbiddenProcess(
        QString processName, QString processPath)
{
    bool send = false;
    int ret_len = 0;
    RULE_FROM_R3 rr;
    ZeroMemory(&rr,sizeof(RULE_FROM_R3));
    //QString 转 BYTE[]
    QByteArray tmp = processName.toLatin1();
    char *mm = (char*)tmp.data();
    char nn[64];
    strcpy(nn,mm);
    memcpy(rr.ImageName,nn,64);

    //QString 转 WCHAR[]
    processPath.toWCharArray(rr.ProcessPath);

    if (DeviceIoControl(DriverWFP, IOCTL_WFP_REMOVE_RULE, &rr, sizeof(RULE_FROM_R3), NULL, 0, (DWORD*)&ret_len, 0))
        send = true;
    CloseHandle(DriverWFP);
    return send;
}

