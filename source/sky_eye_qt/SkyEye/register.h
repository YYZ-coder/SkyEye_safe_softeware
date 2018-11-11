#ifndef REGISTER_H
#define REGISTER_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:管理员创建新用户的注册窗口
******************************************************/

#define FILE_READ       1
#define FILE_WRITE      2
#define FILE_EXCUTE     3

#define FILE_COPY       4

#define FILE_MODIFY     5
#define FILE_ALL        6

#define PROCESS_CREATE  7
#define PROCESS_NET     9
#define NETHEAD_OPEN    11

#define RECORD_WATCH    10
#define FILE_LOCK       13
#define FILE_UNLOCK     14

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QTime>
#include <QGroupBox>
#include <QCheckBox>

#include "conmysql.h"

class Register : public QDialog
{
    Q_OBJECT
public:

    Register(QWidget *parent=0,ConMySQL *mysql = 0,
             QString Localhost = 0,QString Dbname = 0,
             QString Username = 0,QString Password = 0);
    ~Register();

private:
    //当前用户所选的权限集
    QVector<int> *registerJust;

    //权限集合控件块
    QGroupBox *rightGather;

    //数据库操作类
    ConMySQL   *mysql;
    //localhost
    QString    localhost;
    //dbname
    QString    dbname;
    //Admin
    QString    Adminname;
    QString    Adminpassword;

    //字典库
    QVector<QString> lib;

    //从数据库读出现有的用户名:用来进行重复比较
    QVector<QString> users;
    int curIDset;

    QPushButton *btn_Create;
    QPushButton *btn_Cancel;
    //随机生成按钮
    QPushButton *btn_randomGener;

    //角色权限集合设置
    QPushButton *btn_user_defined;
    QCheckBox   *ckb_Fileall;   //all
    QCheckBox   *ckb_Filecopy;  //copy
    QCheckBox   *ckb_Fileread;  //read
    QCheckBox   *ckb_Filewrite; //write
    QCheckBox   *ckb_Fileexcute;//excute
    QCheckBox   *ckb_Filemodify;//modify
    QCheckBox   *ckb_NetprocessForbidden;
    QCheckBox   *ckb_NetHeadForbidden;
    QCheckBox   *ckb_ProcessForbiddenOpen;

    QLabel      *lab_userName;
    QLineEdit   *lne_userName;
    QLabel      *lab_passWord;
    QLineEdit   *lne_passWord;
    QLabel      *lab_level;
    QComboBox   *cob_securityLevel;
    //表示当前位置的点
    QPoint      CurrPosition;
    //存储当前事件的毫秒数
    QTime t;
    //随机生成用户名和密码
    QString randomGenerate();
    //从数据库读取当前存在用户列表
    void readUserfromDB();
    //清空当前控件的数据
    void clearAll();
    //查找当前所选权限组成的角色是否存在
    int checkRoleJurisdiction();

public slots:
    //processNet
    void processNet(int);
    //processCreate
    void processCreate(int);
    //netheadForbidden
    void netheadForbidden(int);

    //fileModify
    void fileModify(int);
    //fileWrite
    void fileWrite(int);
    //fileRead
    void fileRead(int);
    //fileCopy
    void fileCopy(int);
    //fileExecute
    void fileExcute(int);
    //filrAll
    void fileAll(int);

    void showRightgather();

    void GenerateAccount();

    void createAccount();

protected:

    void closeEvent(QCloseEvent *);

    void paintEvent(QPaintEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

};

#endif // REGISTER_H
