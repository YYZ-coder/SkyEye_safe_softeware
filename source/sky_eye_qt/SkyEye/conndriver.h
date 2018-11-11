#ifndef CONNDRIVER_H
#define CONNDRIVER_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:驱动操作的抽象类,声明基本常用协议,并且作为各个驱动继承的
 *     公共类.
******************************************************/

#include <QObject>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

//Protocol
#define PROTO_ARP	  8

#define PROTO_ICMP    1
#define PROTO_IGMP    2
#define PROTO_TCP     6
#define PROTO_UDP     17

//数据库登陆记录结构
class ForDB{
public:
    QString username;
    QString password;
    QString host;
    QString dbname;
};

class ConnDriver{

public:
    ConnDriver(){}
    ~ConnDriver(){}

    virtual bool openDriver() = 0;
};

#endif // CONNDRIVER_H
