#ifndef SSDT_OPERATE_H
#define SSDT_OPERATE_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:SSDT驱动操作类,实现通过驱动对进程的保护/隐藏,禁止/允许打开
 *     ,注册表的读写操作
******************************************************/

#include "conndriver.h"

//SkyEye发送SSDT当前进程ID并保护
#define IOCTL_SSDT_ADD \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x830, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//SkyEye删除SSDT当前受保护进程ID
#define IOCTL_SSDT_DELETE \
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x831, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//获得用户输入的用户名,密码,和注册表中DBname,DBhost：CP+Regedit -> SkyEye
#define IOCTL_SSDT_GET_UP\
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x851, METHOD_BUFFERED, \
    FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//将安装信息写入注册表：SkyEye->Register
#define IOCTL_SSDT_SET_REG\
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x852,METHOD_BUFFERED,\
    FILE_WRITE_ACCESS | FILE_READ_ACCESS)

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

//清除驱动黑名单
#define IOCTL_SSDT_CLAER_BLACK_ILST\
    CTL_CODE(\
    FILE_DEVICE_UNKNOWN, \
    0x832, METHOD_BUFFERED, \
    FILE_WRITE_ACCESS | FILE_READ_ACCESS)


//SkyEye发送给SSDT，并让其存入注册表中的数据
typedef struct _R3_TO_REG{
    //数据库Host名,数据库名
    WCHAR dbHost[10];
    WCHAR dbName[10];
    //系统管理员账户
    WCHAR sysAdmin[10];
    WCHAR sysPassword[20];
}R3_TO_REG,*PR3_TO_REG;

//将这四个参数传递给R3
//USER,PASSWORD	 从CP获得;
//HOST,DBNAME	 从注册表读得.
typedef struct _CP_TO_R3{
    int  userLen;
    int  passLen;
    CHAR USER[10];
    CHAR PASSWORD[20];
    CHAR HOST[10];
    CHAR DBNAME[10];
}CP_TO_R3,PCP_TO_R3;

//要传进去的禁止进程名和字符串长度
typedef struct _ForbidProcessName{
    int length;
    WCHAR ProcessName[50];
}ForbidProcessName,*PForbidProcessName;

class SSDT_Operate : public ConnDriver
{
public:
    SSDT_Operate();
    ~SSDT_Operate();

    bool openDriver();

    void closeSSDT();

    bool clear();

    bool addHideAndProtected(ULONG pid);

    bool deleteHideAndProtected();

    bool addForbidProcessName(QString pn);

    bool deleteForbidProcessName(QString pn);

    bool sendRegistryData(QString sysAdmin,QString sysPassword,QString dbName,QString dbHost);

    ForDB getData();

private:

    HANDLE DriverSSDT;
};

#endif // SSDT_OPERATE_H
