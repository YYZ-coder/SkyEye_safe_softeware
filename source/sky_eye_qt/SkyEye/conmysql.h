#ifndef CONMYSQL_H
#define CONMYSQL_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:MySql数据库操作类,实现对数据库的打开,关闭,增加数据,删除
 *     数据,更新数据,读取数据的基本操作
******************************************************/

#include <QObject>
#include <QtSql>
#include <QDebug>
#include <QMessageBox>

class ConMySQL
{
public:
    ConMySQL();
    ~ConMySQL();
    //打开数据库
    bool OpenDB(QString host,QString DBname,QString userName,QString password);
    /*
     * 读取/写入 数据库的表:
     *  user,
     *  process,
     *  processmonitor,
     *  role,
     *  nethead.
    */
    //从user表中获得所有用户信息
    QSqlQuery getUserID();
    //注册一个用户，并将其写入user表
    bool insertUser(QString username,int userID,int securityLevel);
    //创建用户
    bool CreateUser(int id,int sl,QString dbname,QString localhost,QString username,QString password,QVector<int> justs);
    //删除用户
    bool DeleteUser(int id,QString localhost,QString username);
    //从users表删除user信息,按照user_ID
    bool deleteUserid(int userid);
    //移除登录数据库用户
    bool deleteUserlogon(QString username,QString localhost);
    //插入user_role表
    bool insertUserRole(int userID,int roleID);
    //删除user_role表中一项(按照user_ID)
    bool deleteUserRole(int userID);
    //删除user_role表中一项(按照role_ID)
    bool deleteuserroleRoleId(int RoleID);
    //插入用户角色及权限信息(单个)
    bool insertUserRoleJurisdiction(int roleID,int justice);
    //删除角色的权限集(全部)
    bool deleteUserRoleJurisdiction(int roleID);
    //插入新建角色
    bool insertRole(int roleID,QString roleName);
    //删除角色
    bool deleteRole(int roleID);
    //获得用户的角色ID
    int getRoleID(int userID);
    //获得所有已有角色ID
    QSqlQuery getRole();
    //获得所有justice集合
    QSqlQuery getJurisdiction();
    //获得角色权限集
    QSqlQuery getRoleJustice(int roleID);
    //获得当前角色所有用户ID
    QSqlQuery getUserIDuserrole(int roleID);

    //获取process表所有信息
    QSqlQuery getProcess();
    //插入一条进程信息
    bool insertProcess(int owner,QString processName,QString processPath,bool forbidOpen,bool forbidNet);
    //删除指定信息
    bool deleteProcess(int owner,QString processName,QString processPath,bool forbidOpen,bool forbidNet);
    //更新进程打开规则信息
    bool updateProcessOpen(int owner,QString processName,QString processPath,bool forbidOpen);
    //更新进程网络规则信息
    bool updateProcessNet(int owner,QString processName,QString processPath,bool forbidNet);

    //获取nethead表中所有信息
    QSqlQuery getNetHead();
    //插入一条信息
    bool insertNetHead(QString ip,QString localport,QString remotport,QString url,int owner,int protocol);
    //删除指定信息
    bool deleteNetHead(QString ip,QString localport,QString remotport,QString url,int owner,int protocol);

    //processmonitor表操作
    //插入数据，并判断是否存在当前owner,processid,processname
    bool insertProcessMonitor(int owner,int processid,QString processName,QString datalength);
    //倘若存在owner,processid,processname对应的数据，则更新
    bool updateProcessMonitor(int owner, int processid, QString processName, QString datalength);

    //验证当前输入的用户是否为输入数据库的owner
    bool verifyDB(QString dbname,QString localhost);

    //关闭数据库
    void closeDB();

    //判断当前是否已经打开
    bool isOpen();

private:

    QSqlDatabase db;

};

#endif // CONMYSQL_H
