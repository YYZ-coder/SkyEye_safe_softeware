
#include "mainwindow.h"

#include <QHBoxLayout>
#include <QCloseEvent>
#include <QHeaderView>
#include <psapi.h>
#include <SDKDDKVer.h>
#include <string.h>
#include <QToolBar>
#include <QDebug>

//---wfp监控---
//事件对象
HANDLE g_hKernelEvent = NULL;
//判断是否启动
BOOLEAN g_bIsRunning = false;
//表示DataLen被设置过成KB
BOOLEAN isKB = false;
//表示DataLen被设置过成MB
BOOLEAN isMB = false;
//用此来设置是否可以关闭该软件
BOOLEAN isAdmin = false;
//写入注册表自动启动路径
#define REG_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
//------------

//初始化MonitorTable
void MainWindow::InitialMonitorTable(){
    //监控表
    monitorTable = new QTableWidget(0,8);
    QStringList header;
    header<<tr("ProcessID")<<tr("ProcessName")
         <<tr("localIp")<<tr("Destination Ip")
        <<tr("localPort")<<tr("DestinationPort")
       <<tr("DataLength")<<tr("Processpath");
    monitorTable->setHorizontalHeaderLabels(header);
    monitorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    monitorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    monitorTable->horizontalHeader()->setHighlightSections(false);

    rowData = new QVector<QString>();
    PIDs = new QVector<QString>();
    PROCESPaths = new QVector<QString>();
}

//初始化Timer类：实时查询短期不进行流量传送的程序，并进行monitorTable清理
void MainWindow::InitialTimer()
{
    //当前正在执行的进程ID
    currentProcessID = new QVector<QString>();

    processTimer = new QTimer();
    connect(processTimer,SIGNAL(timeout()),this,SLOT(clearUnusedProcess()));
    //processTimer->start(3000);
}

//程序托盘初始化
void MainWindow::InitialSysTray()
{
    sysTray = new QSystemTrayIcon();

    trayMenu = new QMenu();
    setting = new QMenu("Setting");

    action_show = new QAction(trayMenu);
    action_quit = new QAction(trayMenu);
    action_help = new QAction(trayMenu);
    action_about = new QAction(trayMenu);
    action_check_update = new QAction(trayMenu);
    action_processOperation = new QAction(trayMenu);
    action_netDataHeadOperation = new QAction(trayMenu);
    action_fileOperation = new QAction(trayMenu);

    //设置托盘显示文本
    action_show->setText(QString("Show"));
    action_quit->setText(QString("Exit"));
    action_help->setText(QString("Help"));
    action_about->setText(QString("About"));
    action_check_update->setText(QString("update"));
    action_processOperation->setText(QString("Process-Setting"));
    action_netDataHeadOperation->setText(QString("NetHead-Setting"));
    action_fileOperation->setText(QString("File-Setting"));
    //添加菜单项
    setting->addAction(action_processOperation);
    setting->addAction(action_netDataHeadOperation);
    setting->addAction(action_fileOperation);
    trayMenu->addAction(action_show);
    trayMenu->addMenu(setting);
    trayMenu->addSeparator();
    trayMenu->addAction(action_help);
    trayMenu->addAction(action_about);
    trayMenu->addAction(action_check_update);
    trayMenu->addSeparator();
    trayMenu->addAction(action_quit);

    //点击托盘执行的事件
    connect(sysTray , SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
    connect(action_quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(action_show,SIGNAL(triggered()),this,SLOT(show()));

    connect(action_netDataHeadOperation,SIGNAL(triggered()),this,SLOT(NetheadSettingShow()));
    connect(action_processOperation,SIGNAL(triggered()),this,SLOT(processSettingShow()));
    connect(action_fileOperation,SIGNAL(triggered()),this,SLOT(FileSettingShow()));
}

//点击托盘图标出现的动作
void MainWindow::iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    { //点击托盘显示窗口
    case QSystemTrayIcon::Trigger:
    {
        this->show();
        break;
    }
    default:
    break;
    }
}

//初始化窗口头
void MainWindow::InitialHeader()
{
    //窗口左上角显示
    currentUsername = new MultiBtn(0);
    currentUsername->setToolTip("Current logon User");
    currentUsername->setUsername(getUser);
    currentUsername->setMulti("user");

    //管理员角色和用户管理界面
    btn_sqlManager = new QPushButton();
    btn_sqlManager->setToolTip("DataBase Managment");
    btn_sqlManager->setText("Management");

    //注册新用户按钮
    addUser = new MultiBtn(0);
    addUser->setFixedSize(30,20);
    addUser->setToolTip("Add User");
    addUser->setMulti("");

    //软件图标
    owlIcon = new QLabel();
    owlIcon->setFixedSize(30,20);
    owl = new QPixmap(":/owl_sysTray.ico");
    QPixmap owled = owl->scaled(30,20,Qt::KeepAspectRatio);
    owlIcon->setPixmap(owled);

    //关闭按钮
    closeIt = new MultiBtn(this);
    closeIt->setFixedWidth(30);
    closeIt->setFixedHeight(20);
    closeIt->setToolTip("To Tray");
    closeIt->setMulti("X");

    //设置按钮透明
    currentUsername->setStyleSheet("QPushButton{background : transparent;}");
    closeIt->setStyleSheet("QPushButton{background : transparent;}");
    addUser->setStyleSheet("QPushButton{background : transparent;}");
    btn_sqlManager->setStyleSheet("QPushButton{background : transparent;}");
    //打开数据库管理窗口
    connect(btn_sqlManager,SIGNAL(clicked()),this,SLOT(showSqlManager()));
    //点击按钮进行隐藏
    connect(closeIt,SIGNAL(clicked()),this,SLOT(hide()));
    //注册用户按钮
    connect(addUser,SIGNAL(clicked()),this,SLOT(showRegister()));
    //显示当前用户信息
    connect(currentUsername,SIGNAL(clicked()),this,SLOT(showCurrentUserInfo()));
}

//权限ID转字符串
QString JusticeChanging(int justID){
    QString JusticeString;
    switch(justID){
    case 1:
        JusticeString = "File Read";
        break;
    case 2:
        JusticeString = "File Write";
        break;
    case 3:
        JusticeString = "File Execute";
        break;
    case 4:
        JusticeString = "File Copy";
        break;
    case 5:
        JusticeString = "File Modify";
        break;
    case 6:
        JusticeString = "File All";
        break;
    case 7:
        JusticeString = "Process Open";
        break;
    case 8:
        JusticeString = "";
        break;
    case 9:
        JusticeString = "Process Net";
        break;
    case 11:
        JusticeString = "Net Head";
        break;
    }
    return JusticeString;
}

//显示当前用户信息
void MainWindow::showCurrentUserInfo()
{
    QString allJustices;
    if(justice->size() == 0){
        allJustices = "Sorry,U don't have any jurisdiction!";
    }else{
        for(int i=0;i<justice->size();i++){
            allJustices += JusticeChanging(justice->at(i));
            allJustices += "<br>";
        }
    }

    QString list = "<b>UserName</b>:" + getUser + "<br>"
            + "<b>UserID</b>:" + QString::number(currentUserID) + "<br>"
            + "<b>Jurisdiction</b>:<br>"
            + allJustices;
    QMessageBox::information(this,"User-Info",list);
}

//显示SqlManagement
void MainWindow::showSqlManager()
{
    sqlManager->show();
}

//初始化工具控件
void MainWindow::InitialSetting()
{
    btn_processSetting = new QPushButton("ProcessSet");
    btn_processSetting->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    btn_processSetting->clearMask();
    btn_processSetting->setFixedHeight(17);
    btn_processSetting->setToolTip("ProcessSet");

    btn_netheadSetting = new QPushButton("NetHeadSet");
    btn_netheadSetting->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    btn_netheadSetting->setToolTip("NetheadSet");
    btn_netheadSetting->setFixedHeight(17);
    btn_netheadSetting->clearMask();

    btn_fileSetting = new QPushButton("WholeFileSet");
    btn_fileSetting->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    btn_fileSetting->setToolTip("WholeFileSet");
    btn_fileSetting->setFixedHeight(17);
    btn_fileSetting->clearMask();
    //当前用户角色权限集合中含有 PROCESS_CREATE 和 PROCESS_NET
    if(!justice->contains(7) && !justice->contains(9))
        btn_processSetting->setEnabled(false);
    else{
        proSetting = new ProcessOpera_Setting(0,MySql,currentUserID,justice);
        connect(btn_processSetting,SIGNAL(clicked()),this,SLOT(processSettingShow()));
    }

    if(!justice->contains(11))
        btn_netheadSetting->setEnabled(false);
    else{
        //同时还得传入当前用户角色的权限集
        netoSetting = new NetHeadOpera_Setting(0,MySql,currentUserID);
        connect(btn_netheadSetting,SIGNAL(clicked()),this,SLOT(NetheadSettingShow()));
    }

    if(!justice->contains(2) &&
            !justice->contains(3) &&
            !justice->contains(4) &&
            !justice->contains(5) &&
            !justice->contains(6) ){
        btn_fileSetting->setEnabled(false);
    }else{
        fileSetting = new WholeFileOpera_Setting(0,pipe,currentUserID);
        connect(btn_fileSetting,SIGNAL(clicked()),this,SLOT(FileSettingShow()));
    }

    //右键菜单
    popMenu = new QMenu();
    action_showtoolWidget = new QAction(this);
    action_showtoolWidget->setText("showTool");
    action_hidetoolWidget = new QAction(this);
    action_hidetoolWidget->setText("hideTool");
    action_startMonitornet = new QAction(this);
    action_startMonitornet->setText("StartMonitor");
    action_Initialize = new QAction(this);
    action_Initialize->setText("Initialize");
    popMenu->addAction(action_startMonitornet);
    popMenu->addAction(action_Initialize);
    popMenu->addSeparator();
    popMenu->addAction(action_hidetoolWidget);
    popMenu->addAction(action_showtoolWidget);

    connect(action_hidetoolWidget,SIGNAL(triggered()),this,SLOT(hideTool()));
    connect(action_showtoolWidget,SIGNAL(triggered()),this,SLOT(showTool()));
    connect(action_startMonitornet,SIGNAL(triggered()),this,SLOT(startMonitor()));
}

//进程操作设置窗口显示
void MainWindow::processSettingShow()
{
    proSetting->show();
}

//网络包头操作设置显示
void MainWindow::NetheadSettingShow()
{
    netoSetting->show();
}

//文件操作设置显示
void MainWindow::FileSettingShow()
{
    fileSetting->show();
}

//实时插入监控表中
void MainWindow::insertTableWidget()
{
    bool fa = false;
    do{
        //倘若当前为回环地址，则进行屏蔽
        if(rowData->at(2) == "127.0.0.1"){
            break;
        }

        //插入行
        int row_count = monitorTable->rowCount();
        //检测在PIDs中是否含有当前接受到网络信息的PID
        int status = PIDs->indexOf(rowData->at(0),1);
        if(PIDs->isEmpty() || status == -1){
            monitorTable->insertRow(row_count);
            //将当前获取到的PID插入PIDs链中
            PIDs->append(rowData->at(0));
            row_count--;
            for(int i =0;i<8;i++){
                QTableWidgetItem *item = new QTableWidgetItem();
                item->setText(rowData->at(i));
                monitorTable->setItem(row_count,i,item);
            }
        }else{//更新表
            //获得Item所在的行数
            QList<QTableWidgetItem *> li = monitorTable->findItems(rowData->at(0),Qt::MatchExactly);
            int row = li.at(0)->row();

            //-----------------更新LocalIP数据----------------------------
            //新建TableWidgetItem
            QTableWidgetItem *update_LIP = new QTableWidgetItem();
            update_LIP->setText(rowData->at(2));
            monitorTable->setItem(row,2,update_LIP);
            //------------------------------------------------------------

            //-----------------更新RemoteIP数据----------------------------
            //新建TableWidgetItem
            QTableWidgetItem *update_RIP = new QTableWidgetItem();
            update_RIP->setText(rowData->at(3));
            monitorTable->setItem(row,3,update_RIP);
            //------------------------------------------------------------

            //-----------------更新LocalPORT数据----------------------------
            //新建TableWidgetItem
            QTableWidgetItem *update_LPO = new QTableWidgetItem();
            update_LPO->setText(rowData->at(4));
            monitorTable->setItem(row,4,update_LPO);
            //------------------------------------------------------------

            //-----------------更新RemotePORT数据--------------------------
            //新建TableWidgetItem
            QTableWidgetItem *update_RPO = new QTableWidgetItem();
            update_RPO->setText(rowData->at(5));
            monitorTable->setItem(row,5,update_RPO);
            //------------------------------------------------------------

            //-----------------更新DataLength的当前数据---------------------
            //新建TableWidgetItem
            QTableWidgetItem *update_DataLen = new QTableWidgetItem();
            //获得原位置DataLength的数据
            QTableWidgetItem * dataLen = monitorTable->item(row,6);

            //将QString => char*
            QString result;
            QByteArray lastDataChar = dataLen->text().toLatin1();
            char * dat = lastDataChar.data();
            for(int i = 0;i<lastDataChar.length();i++)
            {
                if(dat[i] >= '0' && dat[i] <= '9'){
                    result = result + QString(QChar(dat[i]));
                }
            }
            //转换后，提取其中的数字并进行计算
            int lastDataLen = result.toInt();
            int currentDataLen = rowData->at(6).toInt();

            if(isKB){
                currentDataLen  = currentDataLen/1024;
                currentDataLen += lastDataLen;
            }else
                currentDataLen += lastDataLen;
            QString dtl;
            if(currentDataLen > 1024 && isKB == false){//当B为单位的大于1024
                currentDataLen = currentDataLen / 1024;
                update_DataLen->setText(QString::number(currentDataLen)+"KB");
                dtl = QString::number(currentDataLen)+"KB";
                isKB = true;
            }else if(isKB == true){//当前数据小于1024 并且单位为KB
                update_DataLen->setText(QString::number(currentDataLen)+"KB");
                QString::number(currentDataLen)+"KB";
            }else{
                update_DataLen->setText(QString::number(currentDataLen)+"B");
                QString::number(currentDataLen)+"B";
            }

            monitorTable->setItem(row,6,update_DataLen);
            //------------------------------------------------------------
            //更新至数据库
            setToPorcessMonitor(dtl);
        }
    }while(fa);

}

//初始化整个程序
void MainWindow::InitializeSkyEye()
{
    //主layout
    QVBoxLayout * vb = new QVBoxLayout();
    //head部分
    QHBoxLayout *xx = new QHBoxLayout();
    xx->addWidget(owlIcon);
    xx->addWidget(currentUsername);
    xx->addSpacing(this->width()-20);
    if(isAdmin){
        xx->addWidget(btn_sqlManager);
        xx->addWidget(addUser);
    }
    xx->addWidget(closeIt);
    //工具部分
    QHBoxLayout *tool = new QHBoxLayout();
    tool->addWidget(btn_processSetting);
    tool->addWidget(btn_netheadSetting);
    tool->addWidget(btn_fileSetting);
    //打开进程错误
    if(!openedProcess)btn_fileSetting->setEnabled(false);
    tool->setContentsMargins(0,0,0,0);
    tool->setSizeConstraint(QLayout::SetNoConstraint);
    vb->addLayout(xx);
    vb->addLayout(tool);
    vb->addWidget(monitorTable);
    Main->setLayout(vb);
    this->setCentralWidget(Main);
    //按动鼠标对窗口进行拖动
    moving = false;

    connect(this,SIGNAL(tableShow()),this,SLOT(insertTableWidget()));               //实时显示展示在table
    //Monitor
    connect(monitor,SIGNAL(haveData()),this,SLOT(getMonitorData()));                //主窗体获得网络数据包包头
}

//构造函数
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    do{
        openedProcess = false;
        //存储权限集合
        justice = new QVector<int>();
        //子进程ID
        subPid = 0;
        //初始化当前用户ID
        currentUserID = 0;
        //默认不是自动启动
        isAutoStart = false;
        //初始不是管理员
        isAdmin = false;
        //创建事件
        g_hKernelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        //主窗口设置
        setWindowIcon(QIcon(":/owl0.ico"));
        setFixedSize(500,300);
        setAttribute(Qt::WA_TranslucentBackground);//背景透明
        setWindowFlags(Qt::FramelessWindowHint);   //无边框
        //字符串初始化
        getUser = "";
        getPassword = "";
        getDbhost = "";
        getDbname = "";
        //驱动操作类
        Ndis = new NDIS_Operate();
        Wfp = new WFP_Operate();
        Ssdt = new SSDT_Operate();
        //数据库操作类
        MySql = new ConMySQL();
        //主窗口中心部件
        Main = new QWidget();
        //初始化Monitor线程
        monitor = new Monitor(Wfp);
        //执行获取注册表信息操作
        this->getUserPassword();
        //注册表配置界面
        if(getDbhost == "" && getDbname == ""){
            QMessageBox::information(this,"attention",
                                     "u need initial config!");
            Verify *v = new Verify;
            connect(v,SIGNAL(autologinRoot(ForDB)),this,SLOT(getFromVerify(ForDB)));
            int t = v->exec();
            if( t == 0){
                //初始化processTimer
                InitialTimer();
                //初始化MonitorTable
                InitialMonitorTable();
                //初始化程序托盘
                InitialSysTray();
                //初始化主窗口头部
                InitialHeader();
                //初始化主窗口工具部分
                InitialSetting();
                QTimer::singleShot(1,qApp,SLOT(quit()));
                break;
            }else{
                //开机自启
                AutoStart();
                isAdmin = true;
            }
        }
        //开机自启
        AutoStart();
        //初始化processTimer
        InitialTimer();
        //初始化MonitorTable
        InitialMonitorTable();
        //初始化程序托盘
        InitialSysTray();
        //初始化主窗口头部
        InitialHeader();
        //托盘设置
        sysTray ->setIcon(QIcon(":/owl_sysTray.ico"));
        sysTray->setContextMenu(trayMenu);
        //托盘显示提示信息
        sysTray->showMessage(QString("Hello"), "Hello ~~ \nWelcome "+getUser+" ~\nI'm Here :-)",QSystemTrayIcon::NoIcon,3000);
        sysTray->setToolTip("USER:"+getUser+"\n");
        //显示托盘提示信息
        sysTray->show();
        //判断:WFP,NDIS驱动是否安装正确
        //Wfp && Ndis
        if(Wfp->openDriver() && !Ndis->openDriver()){
            //关闭驱动
            Wfp->closeWFP();
            Ndis->closeNDIS();
            //打开数据库进行验证当前用户身份
            if(MySql->OpenDB(getDbhost,getDbname,getUser,getPassword)){
                //打开外部进程
                openProcess();
                //初始化管道
                pipe = new Pipe_Operation();
                if(!pipe->createPipe()) QMessageBox::information(this,"","CreatePipe failed");
                //初始化注册界面:此时数据库已经打开
                regNew = new Register(0,MySql,getDbhost,getDbname,getUser,getPassword);
                //初始化管理界面
                sqlManager = new SqlManager(0,MySql,getDbhost);
                //获得并存储当前用户的UserID
                savecurrentUserID();
                //加载当前用户拥有的角色权限
                getCurrentJustice();
                if(MySql->verifyDB(getDbname,getDbhost)){
                    QMessageBox::information(this,"welcome","Welcome admine back :-)");
                    //说明当前用户是管理员
                    isAdmin = true;
                }
                //初始化主窗口工具部分
                InitialSetting();
                //初始化程序：给其进行布局
                InitializeSkyEye();
            }else{
                //初始化注册:以防止程序异常
                regNew = new Register(0);
                QMessageBox::information(this,"Warning","Login Failed!");
                QTimer::singleShot(1,qApp,SLOT(quit()));
                break;
            }
             //1-给SSDT发送当前进程的PID进行保护和隐藏
            if(!isAdmin)
                this->sendSSDTProcessID();
            //2-给NDIS发送初始的禁止网络规则
            this->sendNDISDriverMess();
            //3-给WFP发送初始网络规则
            this->sendWFPDriverMess();
            //4-ssdt添加当前用户的初始的禁止进程规则
            this->addForbiddenProcess();
            //5-wfp开始监视网络进程的网络活动
            this->startMonitor();
            //6-发送比当前用户安全等级高的规则信息
            this->loadSeniorSLevel();
        }else{
            QMessageBox::warning(this,"Warning","WFP Driver or NDIS Driver install Error!!");
            //初始化注册
            regNew = new Register(0);
            //初始化程序：给其进行布局
            InitializeSkyEye();
            QTimer::singleShot(10,qApp,SLOT(quit()));
        }
    }while(false);
}

//加载当前用户拥有的角色权限
void MainWindow::getCurrentJustice()
{
    //获得当前角色ID
    currentUserRole = MySql->getRoleID(currentUserID);
    //获得失败
    if(currentUserRole == -1){
        QMessageBox::warning(this,"warning","getUserRoleID Failed!");
    }else{
        //由角色ID获得权限集合
        QSqlQuery sql = MySql->getRoleJustice(currentUserRole);
        if(&sql != NULL){
            while(sql.next()){
                justice->push_back(sql.value(1).toInt());
            }
        }else
            QMessageBox::warning(this,"warning","no query!");
    }
}

//加载比当前用户等级高的用户规则限制信息
void MainWindow::loadSeniorSLevel()
{
    //存储比当前用户安全等级高的userID
    QVector<int> seniorUserID;
    //BLACK_DATA属性
    QString ip1,ip2,ip3,ip4;
    QString url;
    QString lport,rport;
    int proto = 0;

    do{
        //倘若是管理员登陆，省去步骤
        if(isAdmin)
            break;
        //users表中获取比自己权限等级高的用户的userID
        QSqlQuery sql = MySql->getUserID();
        if(&sql == NULL)
            break;
        //遍历整个user表,寻找符合条件的信息
        while(sql.next()){
            //安全等级ID比当前用户低的即为安全等级比当前用户安全等级高
            if(sql.value(1).toInt() < currentUserSecurityLevel)
                seniorUserID.push_back(sql.value(0).toInt());
        }
        //依靠获得的userID查找process,nethead表并下发至驱动
        QSqlQuery s = MySql->getProcess();
        if(&s == NULL)
            break;
        //遍历process表
        while(s.next()){
            //seniorUserID集合中含有当前行的userID
            if(seniorUserID.contains(s.value(0).toInt())){
                //forbidOpen:分发给SSDT驱动
                if(s.value(3).toBool()){
                    if(Ssdt->openDriver()){
                        if(!Ssdt->addForbidProcessName(s.value(1).toString())){
                            QMessageBox::warning(this,"Senior Process(ssdt) tbl","Please contact Admin");
                        }
                    }
                }

                //forbidNet:分发给WFP驱动
                if(s.value(4).toBool()){
                    if(Wfp->openDriver()){
                        if(!Wfp->forbiddenProcess(s.value(1).toString(),s.value(2).toString())){
                            QMessageBox::warning(this,"Senior Process(wfp) tbl","Please contact Admin");
                        }
                    }
                }
            }
        }
        //遍历nethead表
        QSqlQuery sn = MySql->getNetHead();
        if(&sn == NULL)
            break;
        //遍历nethead表
        while(sn.next()){
            if(seniorUserID.contains(sn.value(4).toInt())){
                //获得IP
                QStringList IpList = sn.value(0).toString().split(".");
                ip1 = IpList.at(0);
                ip2 = IpList.at(1);
                ip3 = IpList.at(2);
                ip4 = IpList.at(3);
                //获得URL
                url = sn.value(3).toString();
                //获得port
                lport = sn.value(1).toString();
                rport = sn.value(2).toString();
                //获得协议
                proto = sn.value(5).toInt();
                //进行结构包装并且发送给驱动
                bool ok;
                BLACK_DATA BL;
                ZeroMemory(&BL, sizeof(BLACK_DATA));
                //IP
                BL.Ip[0] = ip1.toInt();
                BL.Ip[1] = ip2.toInt();
                BL.Ip[2] = ip3.toInt();
                BL.Ip[3] = ip4.toInt();
                //URL  (QString 转 CHAR[])
                char *curl = url.toLatin1().data();
                for(int i=0;i<url.length();i++)
                    BL.URL[i] = curl[i];
                //Port
                BL.SourceProt = lport.toShort(&ok,10);
                BL.DestinationProt = rport.toShort(&ok,10);
                //Protocol
                BL.ProtoType = proto;
                if(Ndis->openDriver())
                    if(!Ndis->forbiddenipPortMacUrlProto(BL)){
                        QMessageBox::warning(this,"Senior nethead tbl","Please contact Admin");
                    }
            }
        }

    }while(false);
}

//close subprocess and clear drivers
void MainWindow::closesub()
{
    if(pipe->closeSubProcess()){
        QMessageBox::information(this,"","close process success");
    }else
        QMessageBox::information(this,"","close process failed");
}

//析构函数
MainWindow::~MainWindow()
{
    //关闭监控句柄
    CloseHandle(g_hKernelEvent);
    //数据库关闭
    MySql->closeDB();
    //关闭进程管道
    pipe->closePipe();
    //给各个驱动发送命令:清空当前用户在底层的黑名单列表
    Ssdt->clear();
    Wfp->clear();
    Ndis->clear();
}

//打开外部进程
void MainWindow::openProcess()
{
    QString cmdpath = "sfilter_App.exe";
    QString path = QApplication::applicationFilePath();
    path.replace("/","\\");
    QStringList plist = path.split("\\");
    QString absolutePath;
    plist.removeLast();
    for(int i=0;i<plist.size();i++){
        absolutePath += plist.at(i);
        absolutePath +=  + "\\";
    }
    absolutePath += cmdpath;

    LPCWSTR subpath = absolutePath.toStdWString().c_str();
    //子进程打开
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &pi, sizeof(pi) );
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    bool subProcessCreate = false;

    QString uid = QString::number(currentUserID);

    WCHAR *sendbuffer = (WCHAR *)malloc(4048);

    WCHAR *host = (WCHAR *)malloc(10);
    wcscpy(host,getDbhost.toStdWString().c_str());
    WCHAR *dbname = (WCHAR *)malloc(10);
    wcscpy(dbname, getDbname.toStdWString().c_str());
    WCHAR *dbpass = (WCHAR *)malloc(10);
    wcscpy(dbpass,getPassword.toStdWString().c_str());
    WCHAR *username = (WCHAR *)malloc(10);
    wcscpy(username,getUser.toStdWString().c_str());
    WCHAR *userid = (WCHAR *)malloc(2);
    wcscpy(userid,uid.toStdWString().c_str());

    wsprintf(sendbuffer,L"*hostname:%s|dbname:%s|userid:%s|dbpass:%s|username:%s?",
        host,
        dbname,
        userid,
        dbpass,
        username);

    //打开外部文件驱动交互程序:并传递userID
    subProcessCreate = CreateProcess(subpath,sendbuffer,NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    //子进程ID获取
    DWORD sid = pi.dwProcessId;
    subPid = (int)sid;
    //主窗口显示
    if(subProcessCreate == false){
        char message[120];
        ::sprintf(message,"error code:%d",GetLastError());
        QMessageBox::about(NULL,message,"Create Process failed!\n");
    }else
        openedProcess = true;
}

//获得当前登陆用户的userID并进行存储
void MainWindow::savecurrentUserID()
{
    //user表中每行信息索引依次为：
    //  userID 0
    //  securityLevel 1
    //  username   2
    QSqlQuery sql = MySql->getUserID();
    if(&sql != NULL){
        //getUser为当前登陆用户用户名
        //每行遍历
        while(sql.next()){
            //每行的第2个属性值
            QString name = sql.value(2).toString();
            //匹配上当前登陆的用户名
            if(name == getUser){
                //开始存储当前用户的userID
                currentUserID = sql.value(0).toInt();
                //存储当前用户的securityLevel
                currentUserSecurityLevel = sql.value(1).toInt();
                break;
            }
        }
    }else
        QMessageBox::warning(this,"Error","getUserID failed!");
}

//开机自启
void MainWindow::AutoStart()
{
    QString appName = QApplication::applicationName();
    QSettings *settings = new QSettings(REG_RUN,QSettings::NativeFormat);
    QString appPath = QApplication::applicationFilePath();
    settings->setValue(appName,appPath.replace("/","\\"));
}

//显示工具
void MainWindow::showTool()
{
    btn_processSetting->show();
    btn_fileSetting->show();
    btn_netheadSetting->show();
}

//隐藏工具
void MainWindow::hideTool()
{
    btn_processSetting->hide();
    btn_fileSetting->hide();
    btn_netheadSetting->hide();
}

//打开注册窗口开始注册
void MainWindow::showRegister()
{
    regNew->show();
}

//从Verify获得信息:获得验证时输入的信息
void MainWindow::getFromVerify(ForDB fdb)
{
    getUser = fdb.username;
    getPassword = fdb.password;
    getDbhost = fdb.host;
    getDbname = fdb.dbname;
}

//-------------------------ProcessID----Show---------------------
//输出当前进程表(获得单一的,在下面的循环中)
void MainWindow::ProcessNameAndID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    //获得进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);
    //获得进程名
    if (NULL != hProcess){
        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),&cbNeeded)){
            GetModuleBaseName(hProcess, hMod, szProcessName , sizeof(szProcessName) / sizeof(TCHAR));
        }
    }
    //TCHAR [] => QString
    QString sum;
    for(int i=0;i<260;i++)
        sum+=QString::fromUtf8((char *)szProcessName+i);
    //DWORD => QString
    QString pid = QString::number((int)processID);

    //将PID加入当前进行ID集
    currentProcessID->append(pid);

    // 释放句柄
    CloseHandle(hProcess);
}

//获得当前的进程,获得全部的(循环)
bool MainWindow::getCurrentProcressID()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
        return false;
    }
    cProcesses = cbNeeded / sizeof(DWORD);
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0){
            currentProcessID->clear();
            //开始进程ID入列
            ProcessNameAndID(aProcesses[i]);
        }
    }
    return true;
}

//实时清理短期不使用的当前进程(在monitorTable内显示的)-------------undone-
void MainWindow::clearUnusedProcess()
{
    //--------清理之前进行数据库写入---------

    //判断当前monitorTable是否为空
    if(monitorTable->item(0,0) != NULL){
        //开始进行当前系统总的进程ID收集
        getCurrentProcressID();
        //将收集到的和当前在monitorTable中显示的进行判断，倘若有在进行，就不删除，不在进行，就进行删除操作
        for(int i=0;i<monitorTable->rowCount();i++){
            //获得每行第一个Item，并进行字符串提取,之后进行比对
            QTableWidgetItem *item =  monitorTable->item(i,0);
            //若含有当前数据的ID,则不进行处理;
            //若不含有,则进行删除显示处理。
            if(!currentProcessID->contains(item->text())){
                //QMessageBox::information(this,"");
               // monitorTable->removeRow(i);
            }
        }
    }//else QMessageBox::information(this,"Attention","monitor is Empty!");
}

//----------------------------ADD---------------------------------
//NDIS send
void MainWindow::sendNDISDriverMess()
{
    //---------------
    QString ip1 = "139";
    QString ip2 = "118";
    QString ip3 = "173";
    QString ip4 = "36";
    QString url = "baidu";
    QString lport = "10";
    QString rport = "10";
    int proto = 0;
    bool fal = FALSE;
    do{
        //读取Nethead表中的数据，并进行遍历，包装分发给驱动
        if(MySql->isOpen()){
            QSqlQuery sql = MySql->getNetHead();
            if(&sql == NULL)
                break;
            while(sql.next()){
                //寻找匹配的userID
                if(currentUserID == sql.value(4).toInt()){
                    //获得IP
                    QStringList IpList = sql.value(0).toString().split(".");
                    ip1 = IpList.at(0);
                    ip2 = IpList.at(1);
                    ip3 = IpList.at(2);
                    ip4 = IpList.at(3);
                    //获得URL
                    url = sql.value(3).toString();
                    //获得port
                    lport = sql.value(1).toString();
                    rport = sql.value(2).toString();
                    //获得协议
                    proto = sql.value(5).toInt();
                    //进行结构包装并且发送给驱动
                    bool ok;
                    BLACK_DATA BL;
                    ZeroMemory(&BL, sizeof(BLACK_DATA));
                    //IP
                    BL.Ip[0] = ip1.toInt();
                    BL.Ip[1] = ip2.toInt();
                    BL.Ip[2] = ip3.toInt();
                    BL.Ip[3] = ip4.toInt();
                    //URL  (QString 转 CHAR[])
                    char *curl = url.toLatin1().data();
                    for(int i=0;i<url.length();i++)
                        BL.URL[i] = curl[i];
                    //Port
                    BL.SourceProt = lport.toShort(&ok,10);
                    BL.DestinationProt = rport.toShort(&ok,10);
                    //Protocol
                    BL.ProtoType = proto;
                    //MAC
                    //暂定忽略
                    if(Ndis->openDriver()){
                        if(!Ndis->forbiddenipPortMacUrlProto(BL))
                            QMessageBox::warning(this,"Attention","NDIS Send Failed");
                    }else
                        QMessageBox::warning(this,"Warning","Open NDIS Failed!");
                }
            }
        }
    }while(fal);
}

//WFP send forbid process and processPath
void MainWindow::sendWFPDriverMess()
{
    QString pn = "qq.exe";
    QString pp = "xxx";

    bool fal = false;
    do{
        QSqlQuery sql = MySql->getProcess();
        if(&sql == NULL)
            break;
        while(sql.next()){
            //匹配上用户账户ID
            if(currentUserID == sql.value(0).toInt()){
                //当前行为禁止该条进程网络链接
                if(sql.value(4).toBool()){
                    pn = sql.value(1).toString();
                    pp = sql.value(2).toString();
                    if(Wfp->openDriver()){
                        if(!Wfp->forbiddenProcess(pn,pp))
                            QMessageBox::information(this,"add Wfp","Wfp Send Failed!");
                    }else
                        QMessageBox::warning(this,"Warning","Open WFP Failed");
                }
            }
        }
    }while(fal);
}

//WFP monitor,在这里启动线程
void MainWindow::startMonitor()
{
    if(Wfp->openDriver()){
        //发送同步事件成功
        if(Wfp->sendEvent(g_hKernelEvent)){
            g_bIsRunning = true;
            Sleep(1);
            monitor->start();
        }else
            QMessageBox::information(this,"Attention","Send Event Faild!");
    }else
        QMessageBox::warning(this,"Warning","Start Monitor:Open WFP Failed");
}

//WFP monitor,更新至数据库
void MainWindow::setToPorcessMonitor(QString dtlength)
{
    int processID = rowData->at(0).toInt();
    QString processName = rowData->at(1);
    MySql->insertProcessMonitor(currentUserID,processID,processName,dtlength);
}

//WFP monitor,获得从线程读出的网络信息
void MainWindow::getMonitorData()
{
    //此处将右键startMonitor设置为disabled
    if(action_startMonitornet->isEnabled())
        action_startMonitornet->setEnabled(false);
    //开始获得数据
    TRANSFER_R3 tr3 = monitor->returnData();

    //获得imageName
    char *tranferedByte = (char *)tr3.imageName;
    QString *s = new QString(tranferedByte);

    //获得Ip()
    UINT8 *lA = (UINT8 *)&tr3.SourceIp;
    UINT8 *rA = (UINT8 *)&tr3.RemoteIp;
    QString sIp1 = QString::number(lA[0]);
    QString sIp2 = QString::number(lA[1]);
    QString sIp3 = QString::number(lA[2]);
    QString sIp4 = QString::number(lA[3]);

    QString rIp1 = QString::number(rA[0]);
    QString rIp2 = QString::number(rA[1]);
    QString rIp3 = QString::number(rA[2]);
    QString rIp4 = QString::number(rA[3]);

    //获得Port(SHORT 转 QString)
    QString a,b;
    QString lp = a.setNum(tr3.uLocalPort);
    QString rp = b.setNum(tr3.uRemotePort);

    //获得进程ID(UINT64 转 QString)
    QString processID = QString::number(tr3.uProcessID);
    //获得进程路径
    QString processPath = QString::fromWCharArray(tr3.wProcessPath);
    //获得数据长度
    QString DataLength;
    DataLength.setNum(tr3.uDataLength);
    //rowData不为空，则清除
    if(!rowData->isEmpty())
          rowData->clear();
    rowData->append(processID);                         //ProcessID
    rowData->append(*s);                                //ProcessName
    rowData->append(sIp4+"."+sIp3+"."+sIp2+"."+sIp1);   //Source IP
    rowData->append(rIp4+"."+rIp3+"."+rIp2+"."+rIp1);   //Destination IP
    rowData->append(lp);                                //Local Port
    rowData->append(rp);                                //Remote Port
    rowData->append(DataLength);                        //DataLength
    rowData->append(processPath);                       //ProcessPath

    //发送表格显示的信号，调用insertTableWidget()函数
    emit tableShow();
}

//SSDT,增加SSDT:发送当前进程信息进行保护和隐藏(保护和隐藏当前进程)
void MainWindow::sendSSDTProcessID()
{
    //当前进程ID
    int pid = ::getpid();
    if(Ssdt->openDriver()){
        //保护当前主进程PID
        if(!Ssdt->addHideAndProtected(pid))
            QMessageBox::information(this,"Attention","Protect PID Failed!");
        //保护子进程SubPID
        if(!Ssdt->addHideAndProtected(subPid))
            QMessageBox::information(this,"Attention","Protect SubPID Failed!");
    }else
        QMessageBox::warning(this,"Warning","Open Driver(ssdt) Failed");
}

//SSDT,获得用户名和密码-
void MainWindow::getUserPassword()
{
    if(Ssdt->openDriver()){
        ForDB s2s = Ssdt->getData();
        getUser = s2s.username;
        getPassword = s2s.password;
        getDbhost = s2s.host;
        getDbname = s2s.dbname;
    }else
        QMessageBox::warning(this,"warning","Get Data from Regedit failed!(open failed!)");
}

//SSDT,增加禁止进程
void MainWindow::addForbiddenProcess()
{ 
    bool fal = false;
    do{
        QSqlQuery sql = MySql->getProcess();
        if(&sql == NULL) break;
        while(sql.next()){
            if(currentUserID == sql.value(0).toInt()){
                //当前行为禁止该条进程开启
                if(sql.value(3).toBool()){
                    QString pn = sql.value(1).toString();
                    if(!Ssdt->addForbidProcessName(pn))
                        QMessageBox::information(this,"Add Process","Add failed!");
                }
            }
        }
    }while(fal);
}

//--------------------------DELETE-------------------------------
//WFP delete--
void MainWindow::deleteWFPDriverMess()
{
    QString pn,pp;

    if(Wfp->openDriver()){
        if(Wfp->deleteForbiddenProcess(pn,pp))
            QMessageBox::information(this,"Attention","Wfp Delete Success!");
        else
            QMessageBox::information(this,"Attention","Wfp Delete Failed!");
    }else
        QMessageBox::warning(this,"Warning","Open WFP Failed");
}

//WFP monitor,停止监视--
void MainWindow::stopMonitor()
{
    if(Wfp->openDriver()){
        if(Wfp->stopEvent()){
            g_bIsRunning = false;
            QMessageBox::information(this,"Attention","Stop Monitor Success!");
        }else
            QMessageBox::information(this,"Attention","Stop Monitor Failed!");
    }
    else
        QMessageBox::warning(this,"Warning","Open WFP Failed");
}

//SSDT,删除SSDT--
void MainWindow::deleteSSDTProcessID()
{
    if(Ssdt->openDriver()){
        if(Ssdt->deleteHideAndProtected())
            QMessageBox::information(this,"Attention","delete SSDT success");
        else
            QMessageBox::information(this,"Attention","delete SSDT Failed");
    }else
        QMessageBox::warning(this,"Warning","Open SSDT Failed");
}

//SSDT,删除禁止进程
void MainWindow::deleteForbiddenProcess()
{
    if(!Ssdt->deleteForbidProcessName(QString("regedit.exe")))
        QMessageBox::information(this,"Delete Process","Delete Success");
    else
        QMessageBox::information(this,"Delete Process","Delte Failed!");
}

//NDIS delete--
void MainWindow::deleteNDISDriverMess()
{
    //---------------

    QString ip1,ip2,ip3,ip4;
    QString url,lport,rport;

    //---------------

    bool ok;
    BLACK_DATA BL;
    ZeroMemory(&BL, sizeof(BLACK_DATA));
    //IP
    BL.Ip[0] = ip1.toInt();
    BL.Ip[1] = ip2.toInt();
    BL.Ip[2] = ip3.toInt();
    BL.Ip[3] = ip4.toInt();
    //URL  (QString 转 CHAR[])
    char *curl = url.toLatin1().data();
    for(int i=0;i<url.length();i++)
        BL.URL[i] = curl[i];
    //Port
    BL.SourceProt = lport.toShort(&ok,10);
    BL.DestinationProt = rport.toShort(&ok,10);
    //Protocol
    //BL.ProtoType = protocol;
    //MAC
    //暂定忽略
    if(Ndis->openDriver()){
        if(Ndis->deleteforbiddenipPortMacUrlProto(BL)){
            QMessageBox::information(this,"Info","NDIS Delete Success");
        }else
            QMessageBox::warning(this,"Attention","NDIS Delete Failed");
    }else
        QMessageBox::warning(this,"Warning","Open NDIS Failed!");
}

//-------------------------WidgetEvent---------------------------
//窗口关闭事件函数
void MainWindow::closeEvent(QCloseEvent *e)
{
    //还需判断管理员----通过读取数据库中的记录表
    if(isAdmin){
       //关闭子进程
       closesub();
       e->accept();
       this->close();
       QMainWindow::closeEvent(e);
    }else{//不是管理员
        e->ignore();
        this->hide();
    }
}

//鼠标按压事件重写
void MainWindow::mousePressEvent(QMouseEvent *e)
{
    moving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QMainWindow::mousePressEvent(e);
}

//鼠标移动事件重写
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(moving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QMainWindow::mouseMoveEvent(e);
}

//鼠标释放事件重写
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    moving = false;
    return QMainWindow::mouseReleaseEvent(e);
}

//右键菜单
void MainWindow::contextMenuEvent(QContextMenuEvent *e)
{
    popMenu->clear();
    popMenu->addAction(action_startMonitornet);
    popMenu->addSeparator();
    popMenu->addAction(action_hidetoolWidget);
    popMenu->addAction(action_showtoolWidget);
    popMenu->exec(QCursor::pos());

    e->accept();
}

//paint事件函数重写
void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);//反锯齿
    painter.setBrush(QBrush(QColor(232,232,232)));
    painter.setPen(QColor(Qt::black));
    QRect rect = this->rect();
    rect.setWidth(rect.width()-1);
    rect.setHeight(rect.height()-1);
    painter.drawRoundedRect(rect,15,15);
    painter.drawLine(QPoint(0,closeIt->pos().y()+closeIt->height()),QPoint(rect.width(),closeIt->pos().y()+closeIt->height()));

    QMainWindow::paintEvent(e);
}

Monitor::~Monitor(){}

//进程Monitor线程中的run()
void Monitor::run()
{
    while (g_bIsRunning){
        //等待同步事件信号
        WaitForSingleObject(g_hKernelEvent, INFINITE);
        TRANSFER_R3 tr3 = Wfp->monitorWFPData();
        this->Data = tr3;
        emit haveData();
        //设置同步事件为无信号，等待下一次通知
        ResetEvent(g_hKernelEvent);
    }
}

