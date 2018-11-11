#ifndef PROCESSOPERA_SETTING_H
#define PROCESSOPERA_SETTING_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:进程操作设置窗口,用来设置对定向进程的禁止打开,禁止联网或反
 *     之的操作.
******************************************************/

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSignalMapper>

#include "ssdt_operate.h"
#include "wfp_operate.h"
#include "multibtn.h"
#include "blacktablelist.h"
#include "conmysql.h"

class ProcessOpera_Setting : public QWidget
{
    Q_OBJECT
public:
    explicit ProcessOpera_Setting(QWidget *parent = 0,ConMySQL *sql = 0,int userID = 0,QVector<int> *justice = 0);
    ~ProcessOpera_Setting();
private:
    //权限集合
    QVector<int> *justices;
    //数据库操作类
    ConMySQL   *mysql;
    //userID
    int userID;
    //关闭按钮
    MultiBtn    *closeIt;
    //驱动操作对象
    SSDT_Operate   *ssdt;
    WFP_Operate    *wfp;

    //被禁止的进程表格
    BlackTableList *blacktw;
    //用来记录用户输入
    QVector<QString> *Data;

    QLabel    *lab_processName;
    QLineEdit *lne_processName;
    QLabel    *lab_processPath;
    QLineEdit *lne_processPath;

    QPushButton *btn_forbidNet;
    QPushButton *btn_forbidOpen;

    //用mapper来解决QComboBox值交互
    QSignalMapper *mapper;

    //当前位置
    QPoint CurrPosition;
    //初始化黑名单表
    void InitialBlackWidget();
    //ForbidOpenProcess:插入表格
    bool InsertForbidOpenprocess();
    //ForbidNetProcess:插入表格
    bool InsertForbidNetprocess();
    //创建一个下拉框
    QComboBox *createComboBox();
    //ProcessOpen 是否重复
    bool processOpenRepeated();
    //ProcessNet 是否重复
    bool processNetRepeated();

signals:

public slots:
    //当前下拉框被更改所触发的操作
    void comboboxChanged(QString );

    //删除表格中的禁止项
    void allowProcess();
    //给表格添加禁止打开项
    void forbidProcessOpen();
    //给表格添加禁止访问网络项
    void forbidProcessNet();
    //显示详细内容
    void Detail();

    //禁止进程被打开
    bool ssdtForbidprocessopen(QString pn);
    //允许进程被打开
    bool ssdtAllowprocessopen(QString pn);

    //禁止进程网络活动
    bool wfpForbidprocessnetwork(QString pn,QString pp);
    //允许进程网络活动
    bool wfpAllowprocessnetwork(QString pn,QString pp);

protected:

    void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);
};

#endif // PROCESSOPERA_SETTING_H
