#include "processopera_setting.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QDebug>

bool pmoving = false;

//初始化表格
void ProcessOpera_Setting::InitialBlackWidget()
{
    blacktw = new BlackTableList(0,4,0);
    QStringList header;
    header<<"ProcessName"<<"ProcessPath"<<"ForbidOpen"<<"ForbidNet";
    blacktw->setHorizontalHeaderLabels(header);
    blacktw->setSelectionBehavior(QAbstractItemView::SelectRows);
    blacktw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blacktw->horizontalHeader()->setHighlightSections(false);

    //实例化一个信号映射对象
    mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(QString)),this,SLOT(comboboxChanged(QString)));

    connect(blacktw,SIGNAL(deleteDt()),this,SLOT(allowProcess()));
    connect(blacktw,SIGNAL(showDetail()),this,SLOT(Detail()));

    //初始化界面完后还需要读取数据库来将当前用户的禁止信息显示在表格中
    //************************************************************
}

//构造函数
ProcessOpera_Setting::ProcessOpera_Setting(QWidget *parent,ConMySQL *sql,int userID,QVector<int> *justice) : QWidget(parent)
{
    //获得当前用户的权限集合
    justices = justice;

    this->userID = userID;
    mysql = sql;

    closeIt = new MultiBtn(this);
    closeIt->setMulti("-");
    closeIt->setFixedSize(30,20);
    closeIt->setStyleSheet("QPushButton{background : transparent;}");
    connect(closeIt,SIGNAL(clicked()),this,SLOT(close()));

    Data = new QVector<QString>();

    //驱动操作对象
    ssdt = new SSDT_Operate();
    wfp = new WFP_Operate();

    //初始化表格
    InitialBlackWidget();

    lab_processName = new QLabel("ProcessName:");
    lne_processName = new QLineEdit();
    lne_processName->setMaxLength(64);
    lne_processName->setToolTip("within 64");
    lab_processPath = new QLabel("ProcessPath");
    lne_processPath = new QLineEdit();
    lne_processPath->setMaxLength(1024);
    lne_processPath->setToolTip("within 1024");

    btn_forbidNet = new QPushButton("Forbid Net");
    btn_forbidOpen = new QPushButton("Forbid Open");

    /*
     *  #define PROCESS_CREATE  7
        #define PROCESS_NET     9
    */
    //用角色权限来决定按钮状态
    if(!justices->contains(7))
        btn_forbidOpen->setEnabled(false);
    if(!justices->contains(9))
        btn_forbidNet->setEnabled(false);

    connect(btn_forbidNet,SIGNAL(clicked()),this,SLOT(forbidProcessNet()));
    connect(btn_forbidOpen,SIGNAL(clicked()),this,SLOT(forbidProcessOpen()));

    QHBoxLayout *h_head = new QHBoxLayout();
    h_head->addSpacing(this->width()-30);
    h_head->addWidget(closeIt);
    QHBoxLayout *h_processname = new QHBoxLayout();
    h_processname->addWidget(lab_processName);
    h_processname->addWidget(lne_processName);
    QHBoxLayout *h_processpath = new QHBoxLayout();
    h_processpath->addWidget(lab_processPath);
    h_processpath->addWidget(lne_processPath);
    QHBoxLayout *h_forbid = new QHBoxLayout();
    h_forbid->addWidget(btn_forbidNet);
    h_forbid->addWidget(btn_forbidOpen);

    QVBoxLayout *v_m = new QVBoxLayout();

    v_m->addLayout(h_head);
    v_m->addLayout(h_processname);
    v_m->addLayout(h_processpath);
    v_m->addLayout(h_forbid);
    v_m->addWidget(blacktw);

    setLayout(v_m);

    setFixedSize(500,300);
    setWindowIcon(QIcon(":/owl0.ico"));
    setAttribute(Qt::WA_TranslucentBackground);  //背景透明
    setWindowFlags(Qt::FramelessWindowHint);     //无边框
}

//析构函数
ProcessOpera_Setting::~ProcessOpera_Setting()
{

}

//获得bool值
bool getStatus(QString j)
{
    if(j == "Yes")
        return true;
    else
        return false;
}

//删除表格中的一行数据:并解除禁止
void ProcessOpera_Setting::allowProcess()
{
    QList<QTableWidgetItem *> Item = blacktw->selectedItems();
    QString processName = Item.at(0)->text();
    QString processPath = Item.at(1)->text();

    int row = Item.at(0)->row();
    //读取有下拉框的数据
    QComboBox *fo = (QComboBox *)blacktw->cellWidget(row,2);
    QComboBox *fn = (QComboBox *)blacktw->cellWidget(row,3);
    QString fos = fo->currentText();
    QString fns = fn->currentText();

    blacktw->removeRow(row);
    //给W驱动发送删除命令
    this->wfpAllowprocessnetwork(processName,processPath);
    this->ssdtAllowprocessopen(processName);

    bool forbidO = getStatus(fos);
    bool forbidN = getStatus(fns);

    //给数据库发送删除命令
    if(!mysql->deleteProcess(userID,processName,processPath,forbidO,forbidN))
        QMessageBox::warning(NULL,"warning","process delete failed!");
}

//创建一个下拉框
QComboBox *ProcessOpera_Setting::createComboBox()
{
    QComboBox *com = new QComboBox();
    QStringList status;
    status<<"Yes"<<"No";
    com->addItems(status);
    return com;
}

//规则存在且下拉框更改操作:rc 存储 "row,col"
void ProcessOpera_Setting::comboboxChanged(QString rc)
{
    QStringList sl = rc.split(",");
    int row = sl.at(0).toInt();
    int col = sl.at(1).toInt();

    if(col == 2){//当前改变的为ProcessOpen的ComboBox对象
        bool processOpenstatus = false;
        QComboBox *PO = (QComboBox *)blacktw->cellWidget(row,col);
        //获得第0个Item的值:ProcessName
        QTableWidgetItem *cuio = blacktw->item(row,0);
        QTableWidgetItem *cuiop = blacktw->item(row,1);
        if("Yes" == PO->currentText()){
            //给驱动发ForbidOpen命令
            this->ssdtForbidprocessopen(cuio->text());
            //禁止进程打开
            processOpenstatus = true;
        }else{
            //给驱动发送AllowOpen命令
            this->ssdtAllowprocessopen(cuio->text());
            //允许进程打开
            processOpenstatus = false;
        }
        //更新数据库规则信息
        if(!mysql->updateProcessOpen(userID,cuio->text(),cuiop->text(),processOpenstatus)){
            QMessageBox::warning(this,"warning","update failed!");
        }
    }else if(col == 3){//当前改变的为ProcessNet的ComboBox对象
        bool processNetstatus = false;
        QComboBox *PN = (QComboBox *)blacktw->cellWidget(row,col);
        //获得第0个Item的值:ProcessName
        QTableWidgetItem *cuin = blacktw->item(row,0);
        //获得第1隔Item的值:ProcessPath
        QTableWidgetItem *cuinp = blacktw->item(row,1);
        if("Yes" == PN->currentText()){
            //给驱动发送ForbidNet命令
            this->wfpForbidprocessnetwork(cuin->text(),cuinp->text());
            //禁止进程网络
            processNetstatus = true;
        }else{
            //给驱动发送AllowNet命令
            this->wfpAllowprocessnetwork(cuin->text(),cuinp->text());
            //允许进程网络
            processNetstatus = false;
        }
        //更新数据库对应规则信息
        if(!mysql->updateProcessNet(userID,cuin->text(),cuinp->text(),processNetstatus)){
            QMessageBox::warning(this,"warning","update failed!");
        }
    }
}

//ProcessOpen是否重复
bool ProcessOpera_Setting::processOpenRepeated()
{
    bool repetition = false;
    //先判断是否有重复:规则不能重复
    int row = blacktw->rowCount();
    //每行判断
    if(row != 0){
        //遍历整个表,判断是否有重复
        for(int r=0;r<row;r++){
            QTableWidgetItem *item0 = blacktw->item(r,0);
            QTableWidgetItem *item1 = blacktw->item(r,1);
            QComboBox *item3 = (QComboBox *)blacktw->cellWidget(r,2);
            //processName processPath forbidOpen匹配上
            //Data指当前添加的规则信息行
            if(item0->text() == Data->at(0) && item1->text() == Data->at(1) && item3->currentText() == Data->at(2))
                return true;
            else if(item0->text() == Data->at(0) && item1->text() == Data->at(1)){
                //此时没有匹配上，则利用QComboBox的textChange机制进行更进一步的操作
                item3->setCurrentIndex(0);
                return true;
            }
        }
    }
    return repetition;
}

//插入表格:ForbidProcessOpen
bool ProcessOpera_Setting::InsertForbidOpenprocess()
{
    bool repetition = false;
    QString pn = lne_processName->text();

    do{
        //没重复:这里判断的是完全重复(processName,processPath,forbidNet)
        if(!processOpenRepeated()){
            //先判断是否有重复:规则不能重复
            int row = blacktw->rowCount();
            //没有重复,便新添一行:传递给驱动命令
            if(ssdtForbidprocessopen(pn)){
                //新建表插入
                blacktw->insertRow(row);
                for(int i=0;i<4;i++){
                    //将这个设置为开：Yes
                    QString r = QString::number(row);
                    if(i==2){
                        QComboBox *com = createComboBox();
                        com->setCurrentIndex(0);
                        QString c = QString::number(i);
                        connect(com,SIGNAL(currentIndexChanged(int)),mapper,SLOT(map()));
                        mapper->setMapping(com,r+","+c);
                        blacktw->setCellWidget(row,i,com);
                    }else if(i==3){//这个设置为No
                        QComboBox *net = createComboBox();
                        net->setCurrentIndex(1);
                        QString c = QString::number(i);
                        connect(net,SIGNAL(currentIndexChanged(int)),mapper,SLOT(map()));
                        mapper->setMapping(net,r+","+c);
                        blacktw->setCellWidget(row,i,net);
                    }else{
                        QTableWidgetItem *ite = new QTableWidgetItem();
                        ite->setText(Data->at(i));
                        blacktw->setItem(row,i,ite);
                    }
                }
            }
        }else
            return true;

    }while(false);

    return repetition;
}

//点击ForbidOpen:增加表格中的禁止打开项
void ProcessOpera_Setting::forbidProcessOpen()
{
    QString processname = lne_processName->text();
    QString processpath = lne_processPath->text();
    do{
        //判断用户输入是否为空
        if(processname.isEmpty() || processpath.isEmpty()){
            QMessageBox::warning(this,"Warning","ProcessName or ProcessPath is Empty!");
            break;
        }

        //将当前操作记录在Data中
        Data->clear();
        Data->append(processname);
        Data->append(lne_processPath->text());
        Data->append("Yes");
        Data->append("No");

        //传递给驱动并插入表格中
        if(InsertForbidOpenprocess()){
            QMessageBox::information(this,"Warning","Process has repeated!");
        }else{
            //若已经存在项
            //向数据库表插入数据项
            if(!mysql->insertProcess(userID,processname,processpath,true,false)){
                QMessageBox::warning(this,"warning","insert process failed!");
            }
        }

    }while(false);
}

//ProcessNet是否重复
bool ProcessOpera_Setting::processNetRepeated()
{
    bool repetition = false;
    //先判断是否有重复:规则不能重复
    int row = blacktw->rowCount();
    //每行判断
    if(row != 0){
        for(int r=0;r<row;r++){
            //获取进程名和进程路径Item对象
            QTableWidgetItem *item0 = blacktw->item(r,0);
            QTableWidgetItem *item1 = blacktw->item(r,1);
            QComboBox *item3 = (QComboBox *)blacktw->cellWidget(r,3);
            //同进程名,同进程路径,同网络状态
            if(item0->text() == Data->at(0) && item1->text() == Data->at(1) && item3->currentText() == Data->at(3))
                return true;
            else if(item0->text() == Data->at(0) && item1->text() == Data->at(1)){//同进程名,同进程路径,不同网络状态
                //将其设置为Yes,并返回true
                //这里利用的是改变QComboBox后进行操作的机制
                item3->setCurrentIndex(0);
                return true;
            }
        }
    }
    return repetition;
}

//插入表格:ForbidProcessNet
bool ProcessOpera_Setting::InsertForbidNetprocess()
{
    bool repetition = false;
    QString pn,pp;
    pn = lne_processName->text();
    pp = lne_processPath->text();

    do{
        //没重复:这里判断的是完全重复(processName,processPath,forbidNet)
        if(!processNetRepeated()){
            //先判断是否有重复:规则不能重复
            int row = blacktw->rowCount();
            //新增一行数据
            //向驱动发送增添指令
            if(wfpForbidprocessnetwork(pn,pp)){
                //新建规则行插入
                blacktw->insertRow(row);
                for(int i=0;i<4;i++){
                    QString r = QString::number(row);
                    if(i==2){//将这个设置为No:ProcessOpen
                        QComboBox *com = createComboBox();
                        com->setCurrentIndex(1);
                        QString c = QString::number(i);
                        connect(com,SIGNAL(currentIndexChanged(int)),mapper,SLOT(map()));
                        mapper->setMapping(com,r+","+c);
                        blacktw->setCellWidget(row,i,com);
                    }else if(i==3){//这个设置为Yes:ProcessNet
                        QComboBox *net = createComboBox();
                        net->setCurrentIndex(0);
                        QString c = QString::number(i);
                        connect(net,SIGNAL(currentIndexChanged(int)),mapper,SLOT(map()));
                        mapper->setMapping(net,r+","+c);
                        blacktw->setCellWidget(row,i,net);
                    }else{//只创建item不更改:新插入规则行
                        QTableWidgetItem *ite = new QTableWidgetItem();
                        ite->setText(Data->at(i));
                        blacktw->setItem(row,i,ite);
                    }
                }
            }
        }else//有重复
            return true;

    }while(false);

    return repetition;
}

//点击ForbidNet:增加表格中的禁止网络项
void ProcessOpera_Setting::forbidProcessNet()
{
    QString processname = lne_processName->text();
    QString processpath = lne_processPath->text();

    do{
        //判断用户输入是否为空
        if(processname.isEmpty() || processpath.isEmpty()){
            QMessageBox::warning(this,"Warning","ProcessName or ProcessPath is Empty!");
            break;
        }

        Data->clear();
        Data->append(processname);
        Data->append(processpath);
        Data->append("No");
        Data->append("Yes");

        //插入表格和传递给驱动
        //当表格中存在当前插入的进程名和进程路径的数据
        if(InsertForbidNetprocess()){
            QMessageBox::warning(this,"Warning","Has Repeated");
        }else{
            //若已经存在项
            //向数据库表插入数据项
            if(!mysql->insertProcess(userID,processname,processpath,false,true)){
                QMessageBox::warning(this,"warning","insert process failed!");
            }
        }
    }while(false);
}

//显示详细内容
void ProcessOpera_Setting::Detail()
{
    QVector<QString> vs;
    QList<QTableWidgetItem *> curItems = blacktw->selectedItems();
    for(int i=0;i<curItems.length();i++)
         vs.append(curItems.at(i)->text());
    QMessageBox::information(this,"info","<b>PrcessName</b>:"+vs.at(0)+
                             "<b>ProcessPath</b>:"+vs.at(1)+
                             "<b>ForbiddenOpen</b>:"+vs.at(2)+
                             "<b>ForbiddenNet</b>:"+vs.at(3));
}

//禁止进程打开
bool ProcessOpera_Setting::ssdtForbidprocessopen(QString pn)
{
    bool send = false;

    if(!pn.isEmpty()){
        if(ssdt->openDriver()){
            if(ssdt->addForbidProcessName(pn)){
                QMessageBox::information(this,"Info","Send Forbidden Process Name success!");
                send = true;
            }
        }else
            QMessageBox::warning(this,"Warning","Open SSDT Failed!");
    }else
        QMessageBox::warning(this,"Warnig","Forbidden Process Name Area isEmpty!");
    return send;
}

//禁止进程有网络活动
bool ProcessOpera_Setting::wfpForbidprocessnetwork(QString pn,QString pp)
{
    bool send = false;
    if(!pn.isEmpty()){
        if(wfp->openDriver()){
            if(wfp->forbiddenProcess(pn,pp)){
                QMessageBox::information(this,"Info","Send Forbid Process Success!");
                send = true;
            }
        }else
            QMessageBox::warning(this,"Waring","Open WFP Failed!");
    }else
        QMessageBox::warning(this,"Warning","ProcessName Area isEmpty!");
    return send;
}

//允许进程打开
bool ProcessOpera_Setting::ssdtAllowprocessopen(QString pn)
{
    bool send = false;
    if(!pn.isEmpty()){
        if(ssdt->openDriver()){
            if(ssdt->deleteForbidProcessName(pn)){
                QMessageBox::information(this,"Info","Delete Process Name success!");
                send = true;
            }
        }else
            QMessageBox::warning(this,"Warning","Open SSDT Failed!");
    }else
        QMessageBox::warning(this,"Warning","Delete Process Name Area isEmpty!");
    return send;
}

//允许进程有网络活动
bool ProcessOpera_Setting::wfpAllowprocessnetwork(QString pn,QString pp)
{
    bool send = false;
    if(!pn.isEmpty()){
        if(wfp->openDriver()){
            if(wfp->deleteForbiddenProcess(pn,pp)){
                QMessageBox::information(this,"Info","Forbidden process Success!");
                   send = true;
            }
        }else
            QMessageBox::warning(this,"Warning","Open WFP Failed!");
    }else
        QMessageBox::warning(this,"Warning","ProcessName Area is Empty!");
    return send;
}

//---------------------WidgetEvent------------------------------
//paint事件
void ProcessOpera_Setting::paintEvent(QPaintEvent *p)
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

    QWidget::paintEvent(p);
}
//关闭事件重写
void ProcessOpera_Setting::closeEvent(QCloseEvent *c)
{
    this->hide();
    c->ignore();
}
//Mousepress事件重写
void ProcessOpera_Setting::mousePressEvent(QMouseEvent *e)
{
    pmoving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QWidget::mousePressEvent(e);

}
//Mousemove事件重写
void ProcessOpera_Setting::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(pmoving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QWidget::mouseMoveEvent(e);
}
//Mouserelease事件重写
void ProcessOpera_Setting::mouseReleaseEvent(QMouseEvent *e)
{
    pmoving = false;
    return QWidget::mouseReleaseEvent(e);
}
