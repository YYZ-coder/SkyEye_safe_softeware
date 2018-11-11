#ifndef NETHEADOPERA_SETTING_H
#define NETHEADOPERA_SETTING_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:网络信息头操作设置窗口,用来设置对驱动规则的添加删除操作》
******************************************************/

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>

#include "ndis_operate.h"
#include "multibtn.h"
#include "blacktablelist.h"
#include "conmysql.h"

class NetHeadOpera_Setting : public QWidget
{
    Q_OBJECT
public:

    //传入参数:sql操作类, userID
    explicit NetHeadOpera_Setting(QWidget *parent = 0,ConMySQL *sql = 0,int userID = 0);
    ~NetHeadOpera_Setting();

private:
    //数据库操作类
    ConMySQL *mysql;
    //当前用户ID
    int userID;

    //头部按钮
    MultiBtn  *closeIt;
    //驱动操作对象
    NDIS_Operate *ndis;
    //Ip
    QLabel    *label_Ip;
    QLineEdit *lne_Ip1edit;
    QLineEdit *lne_Ip2edit;
    QLineEdit *lne_Ip3edit;
    QLineEdit *lne_Ip4edit;
    //Protocol
    QLabel    *label_Protocol;
    QComboBox *cob_Protocoledit;
    //Port
    QLabel    *label_Localport;
    QLineEdit *lne_Localedit;
    QLabel    *label_Remoteport;
    QLineEdit *lne_Remoteedit;
    //Url
    QLabel    *label_Url;
    QLineEdit *lne_Urledit;
    //删除列表：删除黑名单
    QPushButton *addItem;

    //禁止记录列表
    BlackTableList *blacktw;

    //当前位置
    QPoint    CurrPosition;
    //用来记录当前用户输入的信息
    QVector<QString> *Data;

    void InitialBlackWidget();

    //显示在tableWidget上
    bool addBlacktw();
    //禁止网络信息头
    bool ndisForbidnethead();
    //由comboBox获得的协议字符串进行转化
    int getProtocol(QString proto);

signals:

public slots:

    //点击ADD按钮触发的动作
    void forbidNethead();
    //点击DELETE按钮触发的动作
    void allowNethead();

    //显示此行明细
    void showDetail();

protected:

    void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

};

#endif // NETHEADOPERA_SETTING_H
