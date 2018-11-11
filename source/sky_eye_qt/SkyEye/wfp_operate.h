#ifndef WFP_OPERATE_H
#define WFP_OPERATE_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:WFP驱动操作类,实现进程对应的网络信息的禁止/允许,监控操作.
******************************************************/

#include "conndriver.h"

//WFP
#define MAX_NAME                    16*4
#define PROCESSNAME_LENG            1024

//监控
#define IOCTL_WFP_MONITOR \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x802, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//增加指定防火墙规则
#define IOCTL_WFP_FORBIDDEN \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x803, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//移除指定防火墙规则
#define IOCTL_WFP_REMOVE_RULE \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x804, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//传递事件
#define IOCTL_WFP_START_EVENT \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x822, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//事件停止
#define IOCTL_WFP_STOP_EVENT \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x823, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//清除黑名单
#define IOCTL_WFP_CLEAR_BLACK_LIST\
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x824, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//监视反馈信息
typedef struct _TRANSFER_R3
{
    ULONG      RemoteIp;         //远端IP
    ULONG      SourceIp;         //源IP

    UINT64	   uProcessID;			//进程ID
    UINT64      uDataLength;		//数据长度
    USHORT     uRemotePort;			//连接端口
    USHORT     uLocalPort;			//本地端口
    BOOLEAN	   bIsSend;				//是否是发送
    WCHAR	   wProcessPath[PROCESSNAME_LENG]; //进程全路径
    BYTE		imageName[MAX_NAME];//进程名称
}TRANSFER_R3, *PTRANSFER_R3;
//传进程黑名单
typedef struct _RULE_FROM_R3{
    WCHAR ProcessPath[1024];
    BYTE  ImageName[64];
}RULE_FROM_R3, *PRULE_FROM_R3;


class WFP_Operate : public ConnDriver
{
private:
    //驱动句柄
    HANDLE DriverWFP;

public:

    WFP_Operate();
    ~WFP_Operate();

    //打开WFP驱动
    bool openDriver();
    //关闭WFP驱动
    void closeWFP();

    //清除黑名单链表
    bool clear();

    //WFP 开始同步
    bool sendEvent(HANDLE);
    //WFP 停止同步
    bool stopEvent();
    //WFP data驱动发送给程序的
    TRANSFER_R3 monitorWFPData();
    //WFP process传进黑名单
    bool forbiddenProcess(QString processName,QString processPath);
    //WFP 删除黑名单具体一项
    bool deleteForbiddenProcess(QString processName,QString processPath);


};

#endif // WFP_OPERATE_H
