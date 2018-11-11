#include "conmysql.h"
#include <QMessageBox>

ConMySQL::ConMySQL(){}

ConMySQL::~ConMySQL(){if(db.isOpen()) db.close();}

//打开数据库
bool ConMySQL::OpenDB(QString host,QString DBname,QString userName,QString password)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setDatabaseName(DBname);
    db.setUserName(userName);
    db.setPassword(password);
    bool ok = db.open();
    return ok;
}

//读取数据库中的users表并返回所有信息
QSqlQuery ConMySQL::getUserID()
{
    if(isOpen()){
        QSqlQuery sql;
        bool yes = false;
        yes = sql.exec(QString("SELECT * FROM users"));
       if(yes){
            return sql;
        }else
            return NULL;
    }else
        return NULL;
}

//获得所有角色ID
QSqlQuery ConMySQL::getRole()
{
    QSqlQuery sql;
    bool yes = false;
    yes = sql.exec(QString("SELECT * FROM role"));
    if(yes){
        return sql;
    }
    return NULL;
}

//获得所有角色对应的权限集
QSqlQuery ConMySQL::getJurisdiction()
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM role_jurisdiction"));
    if(yes)
        return sql;
    return NULL;
}

//获得当前角色的权限集合
QSqlQuery ConMySQL::getRoleJustice(int roleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM role_jurisdiction WHERE Role_ID = '%1'")
             .arg(roleID));
    if(yes)
        return sql;
    else
        return NULL;
}

//获得对应role_ID的userID
QSqlQuery ConMySQL::getUserIDuserrole(int roleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM user_role WHERE Role_ID = '%1'")
                   .arg(roleID));
    if(yes)
        return sql;
    else
        return NULL;
}

//获得进程表中所有信息
QSqlQuery ConMySQL::getProcess()
{
    bool yes;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM process"));
    if(yes){
        return sql;
    }
    return NULL;
}

//获得包头表中所有控制信息
QSqlQuery ConMySQL::getNetHead()
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM nethead"));
    if(yes)
        return sql;
    return NULL;
}

//插入users表中,并返回状态
bool ConMySQL::insertUser(QString username,int userID, int securityLevel)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("INSERT INTO users VALUES('%1','%2','%3','%4','%5','6','%7')")
                      .arg(userID).arg(securityLevel).arg(username).arg(" ").arg(0).arg("").arg(" "));
    return yes;
}

//插入user_role表中
bool ConMySQL::insertUserRole(int userID, int roleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("INSERT INTO  user_role VALUES('%1','%2')")
                   .arg(userID).arg(roleID));
    return yes;
}

//删除user_role内对应的一行where User_ID = xx
bool ConMySQL::deleteUserRole(int userID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM user_role WHERE User_ID = '%1'")
                   .arg(userID));
    return yes;
}

//删除user_role表中对应一行where Role_ID = xx
bool ConMySQL::deleteuserroleRoleId(int RoleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM user_role WHERE Role_ID = '%1'")
                   .arg(RoleID));
    return yes;
}

//插入用户ID和对应的权限表
bool ConMySQL::insertUserRoleJurisdiction(int roleID, int justice)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("INSERT INTO role_jurisdiction"
                     " VALUES('%1','%2')").arg(roleID).arg(justice));
    return yes;
}

//删除权限表中的权限集
bool ConMySQL::deleteUserRoleJurisdiction(int roleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM role_jurisdiction WHERE Role_ID = '%1'")
                   .arg(roleID));
    return yes;
}

//插入新的角色并命名
bool ConMySQL::insertRole(int roleID, QString roleName)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("INSERT INTO role VALUES('%1','%2')")
                   .arg(roleID).arg(roleName));
    return yes;
}

//删除对应角色ID的角色
bool ConMySQL::deleteRole(int roleID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM role WHERE Role_ID = '%1'")
                   .arg(roleID));
    return yes;
}

//获得用户角色ID
int ConMySQL::getRoleID(int userID)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM user_role WHERE User_ID = '%1'").arg(userID));
    if(yes){
        if(sql.next()){
            return sql.value(1).toInt();
        }
    }
    return -1;
}

//插入一条进程控制信息
bool ConMySQL::insertProcess(int owner, QString processName, QString processPath, bool forbidOpen, bool forbidNet)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("INSERT INTO process VALUES('%1','%2','%3','%4','%5')")
                   .arg(owner).arg(processName).arg(processPath).arg(forbidOpen).arg(forbidNet));
    return yes;
}

//删除指定进程控制信息
bool ConMySQL::deleteProcess(int owner, QString processName, QString processPath, bool forbidOpen, bool forbidNet)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM process WHERE Owner = '%1' "
                           "AND processName = '%2' "
                           "AND processPath = '%3' "
                           "AND forbidOpen = '%4' "
                           "AND forbidNet = '%5'")
                   .arg(owner).arg(processName).arg(processPath).arg(forbidOpen).arg(forbidNet));
    return yes;
}

//更新进程打开规则
bool ConMySQL::updateProcessOpen(int owner, QString processName, QString processPath, bool forbidOpen)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("update process set forbidOpen = '%1' where Owner = '%2' AND processName = '%3' AND processPath = '%4'")
                   .arg(forbidOpen).arg(owner).arg(processName).arg(processPath));
    return yes;
}

//更新进程网络规则
bool ConMySQL::updateProcessNet(int owner, QString processName, QString processPath, bool forbidNet)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("update process set forbidNet = '%1' where Owner = '%2' AND processName = '%3' AND processPath = '%4'")
                   .arg(forbidNet).arg(owner).arg(processName).arg(processPath));
    return yes;
}

//插入包头控制信息
bool ConMySQL::insertNetHead(QString ip, QString localport, QString remotport, QString url, int owner, int protocol)
{
    bool yes = false;
    QSqlQuery sql;
    if(isOpen()){
        yes = sql.exec(QString("INSERT INTO nethead VALUES('%1','%2','%3','%4','%5','%6')")
                       .arg(ip).arg(localport).arg(remotport).arg(url).arg(owner).arg(protocol));
    }else
        QMessageBox::information(NULL,"attention","DB not OPEN ");

    return yes;
}

//删除包头信息指定行
bool ConMySQL::deleteNetHead(QString ip, QString localport, QString remotport, QString url, int owner, int protocol)
{
    bool yes = false;
    QSqlQuery sql;
    if(isOpen()){
        yes = sql.exec(QString("DELETE FROM nethead WHERE IP = '%1' "
                              "AND localPort = '%2' AND remotePort = '%3'"
                              " AND url = '%4' AND owner = '%5' AND proto = '%6'")
                 .arg(ip).arg(localport).arg(remotport).arg(url).arg(owner).arg(protocol));
    }else
        QMessageBox::information(NULL,"attention","DB not OPEN ");

    return yes;
}

//插入进程监控的数据并判断是否要更新
bool ConMySQL::insertProcessMonitor(int owner, int processid, QString processName, QString datalength)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("SELECT * FROM processmonitor WHERE owner_id='%1' AND process_id='%2' AND process_name='%3'")
                   .arg(owner).arg(processid).arg(processName));
    //存在当前信息
    if(yes){
        //更新操作
        yes = updateProcessMonitor(owner,processid,processName,datalength);
    }else{//不存在此用户对此进程的操作
        //插入操作
        yes = sql.exec(QString("INSERT INTO processmonitor VALUES('%1','%2','%3','%4')")
                       .arg(owner).arg(processid).arg(processName).arg(datalength));
    }
    return yes;
}

//更新操作
bool ConMySQL::updateProcessMonitor(int owner, int processid, QString processName, QString datalength)
{
    bool yes = false;
    QSqlQuery sql;
    yes= sql.exec(QString("update processmonitor set datalength = '%1' where owner_id = '%2' and process_id = '%3' and process_name ='%4'")
                  .arg(datalength).arg(owner).arg(processid).arg(processName));
    return yes;
}

//创建用户
bool ConMySQL::CreateUser(int id,int sl,QString dbname,QString localhost, QString username, QString password,QVector<int> justs)
{
    QSqlQuery sq;
    if(sq.exec(QString("grant select,insert,update,delete,create,drop on %1 to '%2'@'%3' IDENTIFIED BY '%4' ")
               .arg(dbname).arg(username).arg(localhost).arg(password))){
        //赋权给注册用户
        //users表:select
        sq.exec(QString("grant select on users to '%1'@'%2'")
                .arg(username).arg(localhost));
        //role_jurisdiction表:select
        sq.exec(QString("grant select on role_jurisdiction to '%1'@'%2'")
                 .arg(username)
                 .arg(localhost));
        //user_role表:select 用于查找用户角色
        sq.exec(QString("grant select on user_role to '%1'@'%2' ")
                .arg(username)
                .arg(localhost));
        //processmonitor表:insert，插入
        sq.exec(QString("grant insert on processmonitor to '%1'@'%2'")
                .arg(username).arg(localhost));

        //判断该角色是否有权限对process表操作
        if(justs.contains(7) || justs.contains(9)){
            //process表:select & insert & delete,查询，插入，删除
            sq.exec(QString("grant select,insert,delete on process to '%1'@'%2'")
                    .arg(username).arg(localhost));
        }
        //判断该角色是否有权限对nethead表操作
        if(justs.contains(11)){
            //nethead表：select & insert & delete,查询，插入，删除
            sq.exec(QString("grant select,insert,delete on nethead to '%1'@'%2'")
                    .arg(username).arg(localhost));
        }
        //判断该角色是否有权限对文件进行操作
        if(justs.contains(1) || justs.contains(2) || justs.contains(3)
                || justs.contains(4) || justs.contains(5) || justs.contains(6)){
            //record表:插入
            sq.exec(QString("grant insert,select,update,delete on record to '%1'@'%2'")
                    .arg(username).arg(localhost));
            //resource表
            sq.exec(QString("grant insert,select,update,delete on resource to '%1'@'%2'")
                    .arg(username).arg(localhost));
            //operation_jurisdiction表
            sq.exec(QString("grant insert,select,update,delete on operation_jurisdiction to '%1'@'%2'")
                    .arg(username).arg(localhost));
        }
        //将用户插入users表中
        if(insertUser(username,id,sl))
            return true;
    }
    return false;
}

//删除用户并删除users表中的用户信息
bool ConMySQL::DeleteUser(int id,QString localhost, QString username)
{
    if(deleteUserid(id)){
        if(deleteUserlogon(username,localhost)){
            return true;
        }
    }
    return false;
    /*QSqlQuery sq;
    if(sq.exec(QString("DROP USER '%1'@'%2'").arg(username).arg(localhost))){
        if(sq.exec(QString("DELETE FROM users WHERE User_ID = '%1'").arg(id)))
            return true;
        else
            qDebug()<<"delete users failed";
    }
    return false;*/
}

//依靠userid删除user信息
bool ConMySQL::deleteUserid(int userid)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DELETE FROM users WHERE User_ID = '%1'").arg(userid));
    return yes;
}

//移除登陆用户
bool ConMySQL::deleteUserlogon(QString username, QString localhost)
{
    bool yes = false;
    QSqlQuery sql;
    yes = sql.exec(QString("DROP USER '%1'@'%2'")
                  .arg(username).arg(localhost));
    return yes;
}

//验证方式：当前用户可以创建并删除另一个用户就算是owner用户
bool ConMySQL::verifyDB(QString dbname,QString lh)
{
    QString user = "123456789012";
    QVector<int> ju;
    ju.append(0);
    bool yes = false;
    if(CreateUser(100,10,dbname,lh,user,"123",ju)){
        if(DeleteUser(100,lh,user)){
             yes = true;
        }else
             QMessageBox::information(NULL,"","Verify: Drop user Failed");
    }
    return yes;
}

//关闭数据库
void ConMySQL::closeDB(){db.close();}

//判断当前是否已经打开数据库
bool ConMySQL::isOpen()
{
    if(db.isOpen())
        return true;
    return false;
}
