#ifndef VERIFY_H
#define VERIFY_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:对注册表信息的初始化操作
******************************************************/

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QDialog>

#include "conmysql.h"
#include "ssdt_operate.h"


class Verify : public QDialog
{
    Q_OBJECT
public:
    explicit Verify(QWidget *parent = 0);
    ~Verify();

private:

    QPushButton *verify;
    QPushButton *cancel;

    QLineEdit *dbAdmin;
    QLineEdit *dbPass;
    QLineEdit *dbHostname;
    QLineEdit *dbDBname;
    QLineEdit *SysAdmin;
    QLineEdit *SysPass;

    QLabel *dba;
    QLabel *dbp;
    QLabel *dbhost;
    QLabel *dbname;
    QLabel *horiLine;
    QLabel *sysa;
    QLabel *sysp;

    ConMySQL *sql;
    SSDT_Operate *ssdt;

    QPoint CurrPosition;

signals:

    void autologinRoot(ForDB fdb);

public slots:

     void VerifyIt();

     void accept();

protected:

     void closeEvent(QCloseEvent *);

     void paintEvent(QPaintEvent *);

     void mousePressEvent(QMouseEvent *);

     void mouseMoveEvent(QMouseEvent *);

     void mouseReleaseEvent(QMouseEvent *);
};

#endif // VERIFY_H
