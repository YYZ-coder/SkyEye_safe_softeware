#ifndef SQLMANAGER_H
#define SQLMANAGER_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:管理员对用户表,角色表的访问，删除操作，对其它表的查看操作
******************************************************/

#include <QWidget>
#include <QWidget>
#include <QTableView>
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QComboBox>
#include <QLabel>

#include "blacktablelist.h"
#include "conmysql.h"
#include "multibtn.h"

/*
 * 实现思想:
 *
 * 删除一个用户:
 *      删除users表的user行;
 *      同时检测user_role表中有没有对应的(userID-roleID),有则删除即可.
 * 删除一个角色:
 *      删除role表的行;
 *      同时检测user_role表中有无对应的userID;
 *      user_role表中有对应的userID,则读取users表进行删除操作.
*/

class SqlManager : public QWidget
{
    Q_OBJECT
public:
    explicit SqlManager(QWidget *parent = 0,ConMySQL *mysql = 0,QString host = 0);
    ~SqlManager();

private:
    //表索引
    QComboBox *cob_tables;
    //可删除的tableview：用于显示users表，role
    BlackTableList *URtable;
    QTableView     *tableView;
    QSqlQueryModel *queryModel;

    //host
    QString localhost;
    //数据库操作对象
    ConMySQL *msql;
    //按钮
    MultiBtn    *closeIt;
    //当前窗口位置
    QPoint     CurrPosition;
    //当前blacktableList显示的table名
    QString currentChangeTable;

    //显示users表信息
    void showUsersTbl();
    //显示user_role表信息
    void showRoleTbl();

signals:

public slots:

    //删除当前选中行
    void deleteCurrentRow();

    //当前显示
    void checkIndex(int);

protected:

    void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

};

#endif // SQLMANAGER_H
