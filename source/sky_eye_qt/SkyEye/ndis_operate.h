#ifndef NDIS_OPERATE_H
#define NDIS_OPERATE_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:NDIS驱动的操作类,实现了打开驱动句柄,关闭驱动句柄,添加驱动
 *     过滤规则,删除驱动过滤规则.
******************************************************/

#include "conndriver.h"

//NDIS
#define FILE_DEVICE_COMM_DRIVER     0x00008811
#define IOCTL_COMM_SEND_NBL			CTL_CODE(FILE_DEVICE_COMM_DRIVER, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_COMM_SET_SELFIP		CTL_CODE(FILE_DEVICE_COMM_DRIVER, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DELETE_BLACK_LIST     CTL_CODE(FILE_DEVICE_COMM_DRIVER, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_BLACK_LIST		CTL_CODE(FILE_DEVICE_COMM_DRIVER, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLEAR_BLACK_LIST		CTL_CODE(FILE_DEVICE_COMM_DRIVER, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

//传IP,PORT,PROTOCOL,MAC,URL黑名单
typedef struct _BLACK_DATA{
    //Mac地址过滤
    UCHAR      Mac[6];

    //IP地址过滤
    UCHAR	   Ip[4];

    //端口过滤
    USHORT     DestinationProt;
    USHORT	   SourceProt;

    //需要过滤的URL网址
    UCHAR	   URL[20];

    //需要过滤的协议类型
    UCHAR     ProtoType;
}BLACK_DATA, *PBLACK_DATA;

//NDIS操作类
class NDIS_Operate : public ConnDriver
{
private:

    HANDLE DriverNDIS;

public:

    NDIS_Operate();
    ~NDIS_Operate();

    //打开NDIS驱动
    bool openDriver();

    //关闭NDIS驱动
    void closeNDIS();

    //清除底层黑名单
    bool clear();

    //NDIS 添加黑名单
    bool forbiddenipPortMacUrlProto(BLACK_DATA bl);

    //NDIS 删除黑名单具体一项
    bool deleteforbiddenipPortMacUrlProto(BLACK_DATA bl);


};

#endif // NDIS_OPERATE_H
