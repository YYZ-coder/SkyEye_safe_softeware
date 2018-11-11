#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:主窗口类,绘制主窗口,创建操作实例,并进行一系列操作,以实现
 *     对操作模块的整合.
******************************************************/
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QApplication>

#include "conmysql.h"
#include "multibtn.h"
#include "conndriver.h"
#include "ndis_operate.h"
#include "wfp_operate.h"
#include "ssdt_operate.h"
#include "netheadopera_setting.h"
#include "processopera_setting.h"
#include "wholefileopera_setting.h"
#include "verify.h"
#include "register.h"
#include "pipe_operation.h"
#include "sqlmanager.h"

class WFP_Operate;
class MainWindow;

//进程网络监控线程
class Monitor:public QThread{
    Q_OBJECT
public:
    Monitor(WFP_Operate *w){Wfp = w;}
    ~Monitor();

private:

    WFP_Operate *Wfp;

    TRANSFER_R3 Data;

public:

    TRANSFER_R3 returnData(){return Data;}

protected:
    void run();

signals:
    void haveData();
};
//主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    //显示当前用户名
    MultiBtn *currentUsername;

    //将监控到的网络信息更新至数据库
    void setToPorcessMonitor(QString dtlength);

    //进程通信操作
    Pipe_Operation *pipe;
    //打开子进程
    bool openedProcess;
    void openProcess();

    //打开的子进程ID
    int subPid;

    //从user_role中读取当前用户角色
    int     currentUserRole;
    //从user表中查询userID
    int     currentUserID;
    //从user表读取当前用户权限等级
    int     currentUserSecurityLevel;
    //保存当前用户ID
    void    savecurrentUserID();

    //设置开机启动
    bool    isAutoStart;
    void    AutoStart();

    QWidget *Main;

    MultiBtn *addUser;
    MultiBtn *closeIt;
    QPushButton *btn_sqlManager;
    QLabel *owlIcon;
    QPixmap *owl;

    void InitialHeader();

    //-------------------------------------
    //主窗口工具界面和右键菜单
    QMenu *popMenu;
    QAction *action_showtoolWidget;
    QAction *action_hidetoolWidget;
    QAction *action_startMonitornet;
    QAction *action_Initialize;

    QPushButton *btn_processSetting;
    QPushButton *btn_netheadSetting;
    QPushButton *btn_fileSetting;

    ProcessOpera_Setting *proSetting ;
    NetHeadOpera_Setting *netoSetting ;
    WholeFileOpera_Setting *fileSetting ;

    void InitialSetting();

    //-------------------------------------
    //程序托盘操作
    QSystemTrayIcon *sysTray;
    QMenu   * trayMenu ;
    QAction * action_show ;
    QAction * action_quit;
    QAction * action_help;
    QAction * action_about;
    QAction * action_check_update;
    QMenu   * setting;
    QAction * action_processOperation ;
    QAction * action_netDataHeadOperation;
    QAction * action_fileOperation;

    void InitialSysTray();
    //-------------------------------------
    //操作对象
    NDIS_Operate  * Ndis;
    WFP_Operate   * Wfp;
    SSDT_Operate  * Ssdt;
    ConMySQL      * MySql;
    //WFP监视线程
    Monitor *monitor;
    //实时删除短期不占用流量的进程
    QTimer *processTimer;
    QVector<QString> *currentProcessID;//当前正在运行的进程ID

    //-------------------Menu--------------------------
    QMenu *menu;
    void InitialTimer();

    //-------------MonitorTable-------------------------

    //监控表
    QTableWidget *monitorTable;
    QPushButton *insertTable;
    QVector<QString> *rowData;          //行数据列(为了存储所有可显示的Process网络流量信息)
    QVector<QString> *PIDs;             //PID列  (为了避免显示多行同PID的数据行)
    QVector<QString> *PROCESPaths;      //ProcessPath列

    void InitialMonitorTable();

    //--------------UPHN---------------------------------

    //从SSDT获得的登陆用户名和密码
    QString getUser;
    QString getPassword;
    //从SSDT获得的管理员和密码
    QString getDbname;
    QString getDbhost;
    //---------------------------------------------------

    //获得当前进程ID,用来更新MonitorTable
    void ProcessNameAndID(DWORD processID);
    bool getCurrentProcressID();
    //获得当前用户的角色权限
    QVector<int> *justice;
    void getCurrentJustice();

    //当前鼠标位置
    QPoint CurrPosition;
    bool moving;
    //注册界面
    Register  *regNew;
    //数据库管理窗口
    SqlManager *sqlManager;
    //加载比自己安全等级高的用户规则信息
    void loadSeniorSLevel();

signals:

    //通知表格显示
    void tableShow();

private slots:

    void closesub();
    //显示当前用户信息窗口
    void showCurrentUserInfo();
    //显示管理界面
    void showSqlManager();

    //Verify窗口传值给主窗口
    void getFromVerify(ForDB fdb);
    //打开注册界面
    void showRegister();
    //初始化整个界面绘图
    void InitializeSkyEye();
    //show
    void processSettingShow();
    void NetheadSettingShow();
    void FileSettingShow();
    //processTimer
    void clearUnusedProcess();
    //monitorTable
    void insertTableWidget();
    //NDIS
    void sendNDISDriverMess();
    void deleteNDISDriverMess();
    //WFP
    void sendWFPDriverMess();
    void deleteWFPDriverMess();
    void getMonitorData();
    void startMonitor();
    void stopMonitor();
    //SSDT
    void sendSSDTProcessID();
    void deleteSSDTProcessID();
    void getUserPassword();
    void addForbiddenProcess();
    void deleteForbiddenProcess();
    //SysTray & ContextMenu
    void iconIsActived(QSystemTrayIcon::ActivationReason reason);
    void showTool();
    void hideTool();

protected:

     void contextMenuEvent(QContextMenuEvent *);

     void closeEvent(QCloseEvent *);

     void paintEvent(QPaintEvent *);

     void mousePressEvent(QMouseEvent *);

     void mouseMoveEvent(QMouseEvent *);

     void mouseReleaseEvent(QMouseEvent *);
};

#endif // MAINWINDOW_H
