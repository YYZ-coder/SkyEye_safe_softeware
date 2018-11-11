#include "netheadopera_setting.h"

#include <QMessageBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QHeaderView>

//窗口是否移动
bool moving = false;

//初始化表格
void NetHeadOpera_Setting::InitialBlackWidget()
{
    blacktw = new BlackTableList(0,5,0);
    QStringList header;
    header<<"Protocol"<<"Ip"<<"LocalPort"<<"RemotePort"<<"URL";
    blacktw->setHorizontalHeaderLabels(header);
    blacktw->setSelectionBehavior(QAbstractItemView::SelectRows);
    blacktw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blacktw->horizontalHeader()->setHighlightSections(false);

    connect(blacktw,SIGNAL(deleteDt()),this,SLOT(allowNethead()));
    connect(blacktw,SIGNAL(showDetail()),this,SLOT(showDetail()));

    //初始化界面完后还需要读取数据库来将当前用户的禁止信息显示在表格中
    //************************************************************
    QSqlQuery sql = mysql->getNetHead();
    /*while(sql.next()){
        //将表中已存在的数据显示在界面表格中
        if(sql.value(0).toInt() == userID){
            qDebug()<<"Have one!";
        }
    }*/
}

//构造函数
NetHeadOpera_Setting::NetHeadOpera_Setting(QWidget *parent,ConMySQL* sql,int userId) : QWidget(parent)
{
    //记录数据库操作对象和useID
    mysql = sql;
    userID = userId;

    closeIt = new MultiBtn(this);
    closeIt->setMulti("-");
    closeIt->setFixedSize(30,20);
    closeIt->setStyleSheet("QPushButton{background : transparent;}");
    connect(closeIt,SIGNAL(clicked()),this,SLOT(close()));

    //用来记录当前用户输入的信息
    Data = new QVector<QString>();
    ndis = new NDIS_Operate();

    //Ip
    label_Ip = new QLabel("IP:");
    lne_Ip1edit = new QLineEdit();
    lne_Ip1edit->setMaxLength(3);

    lne_Ip2edit = new QLineEdit();
    lne_Ip2edit->setMaxLength(3);

    lne_Ip3edit = new QLineEdit();
    lne_Ip3edit->setMaxLength(3);

    lne_Ip4edit = new QLineEdit();
    lne_Ip4edit->setMaxLength(3);
    //Protocol
    label_Protocol = new QLabel("Protocol:");
    cob_Protocoledit = new QComboBox();
    QStringList cobList;
    cobList<<"--proto--"<<"TCP"<<"ARP"<<"ICMP"<<"IGMP"<<"UDP";
    cob_Protocoledit->addItems(cobList);
    //Port
    label_Localport = new QLabel("LocalPort");
    lne_Localedit = new QLineEdit();
    lne_Localedit->setMaxLength(4);
    label_Remoteport = new QLabel("RemotePort:");
    lne_Remoteedit = new QLineEdit();
    lne_Remoteedit->setMaxLength(4);
    //Url
    label_Url = new QLabel("URL:");
    lne_Urledit = new QLineEdit();
    lne_Urledit->setMaxLength(20);
    lne_Urledit->setToolTip("within in 20");
    //删除黑名单
    addItem = new QPushButton("add");

    connect(addItem,SIGNAL(clicked()),this,SLOT(forbidNethead()));
    //记录表
    InitialBlackWidget();

    QHBoxLayout *hb_head = new QHBoxLayout();
    hb_head->addSpacing(this->width()-30);
    hb_head->addWidget(closeIt);

    QHBoxLayout *hb_bodyIp = new QHBoxLayout();
    hb_bodyIp->addWidget(label_Ip);
    hb_bodyIp->addWidget(lne_Ip1edit);
    hb_bodyIp->addWidget(lne_Ip2edit);
    hb_bodyIp->addWidget(lne_Ip3edit);
    hb_bodyIp->addWidget(lne_Ip4edit);
    QHBoxLayout *hb_bodyport = new QHBoxLayout();
    hb_bodyport->addWidget(label_Localport);
    hb_bodyport->addWidget(lne_Localedit);
    hb_bodyport->addWidget(label_Remoteport);
    hb_bodyport->addWidget(lne_Remoteedit);
    QHBoxLayout *hb_bodyurl = new QHBoxLayout();
    hb_bodyurl->addWidget(label_Url);
    hb_bodyurl->addWidget(lne_Urledit);

    QHBoxLayout *hb_bodyprotocol = new QHBoxLayout();
    hb_bodyprotocol->addWidget(label_Protocol);
    hb_bodyprotocol->addWidget(cob_Protocoledit);
    hb_bodyprotocol->addSpacing(7);

    QVBoxLayout *hb_bodyleft = new QVBoxLayout();
    hb_bodyleft->addLayout(hb_bodyprotocol);
    hb_bodyleft->setAlignment(hb_bodyprotocol,Qt::AlignLeft);
    hb_bodyleft->addLayout(hb_bodyIp);
    hb_bodyleft->addLayout(hb_bodyport);
    hb_bodyleft->addLayout(hb_bodyurl);

    QVBoxLayout *hb_bodyright = new QVBoxLayout();
    hb_bodyright->addWidget(addItem);

    QHBoxLayout *hb_body = new QHBoxLayout();
    hb_body->addLayout(hb_bodyleft);
    hb_body->addLayout(hb_bodyright);

    QVBoxLayout *vb_main = new QVBoxLayout();
    vb_main->addLayout(hb_head);
    vb_main->addStretch(2);
    vb_main->addLayout(hb_body);
    vb_main->addWidget(blacktw);

    setLayout(vb_main);
    setFixedSize(500,300);
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint);   //无边框
}

//析构函数
NetHeadOpera_Setting::~NetHeadOpera_Setting()
{

}

//添加在tableWidget中，并发送给驱动规则
bool NetHeadOpera_Setting::addBlacktw()
{
    bool repetition = false;
    do{
        //先判断是否有重复:一个重复的都不行
        int row = blacktw->rowCount();
        if(row != 0){
            for(int j=0;j<5;j++)
                for(int i=0;i<row;i++){
                    QTableWidgetItem *item = blacktw->item(i,j);
                    if(Data->at(j)==item->text()){
                        repetition = true;
                        return repetition;
                    }
                }
        }

        //传递给驱动命令
        if(ndisForbidnethead()){
            //再插入表中,添加一行
            blacktw->insertRow(row);
            for(int i=0;i<5;i++){
                QTableWidgetItem *ite = new QTableWidgetItem();
                ite->setText(Data->at(i));
                blacktw->setItem(row,i,ite);
            }
        }
    }while(false);

    return repetition;
}

//添加禁止信息头
bool NetHeadOpera_Setting::ndisForbidnethead()
{
    //---------------
    bool send = false;

    QString ip1 = lne_Ip1edit->text();
    QString ip2 = lne_Ip2edit->text();
    QString ip3 = lne_Ip3edit->text();
    QString ip4 = lne_Ip4edit->text();
    QString url = lne_Urledit->text();
    QString lport = lne_Localedit->text();
    QString rport = lne_Remoteedit->text();
    QString proto = cob_Protocoledit->currentText();

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
    BL.ProtoType = getProtocol(proto);
    //MAC
    //暂定忽略
    if(ndis->openDriver()){
        if(ndis->forbiddenipPortMacUrlProto(BL)){
            QMessageBox::information(this,"Info","NetData rule add Success");
            send = true;
        }else
            QMessageBox::warning(this,"Warning","NetData rule add Failed");
    }else
        QMessageBox::warning(this,"Warning","Open Driver Failed!");
    //---------------
    return send;
}

//协议转化
int NetHeadOpera_Setting::getProtocol(QString proto)
{
    int p = 0;
    if(proto == "TCP")
        p = PROTO_TCP;
    else if(proto == "UDP")
        p = PROTO_UDP;
    else if(proto == "ICMP")
        p = PROTO_ICMP;
    else if(proto == "IGMP")
        p = PROTO_IGMP;
    else if(proto == "ARP")
        p = PROTO_ARP;
    return p;
}

//点击Add按钮的操作
void NetHeadOpera_Setting::forbidNethead()
{
    QString ip1 = lne_Ip1edit->text();
    QString ip2 = lne_Ip2edit->text();
    QString ip3 = lne_Ip3edit->text();
    QString ip4 = lne_Ip4edit->text();
    QString ip = ip1 + "." + ip2 + "." +ip3 + "." +ip4;
    QString url = lne_Urledit->text();
    QString lport = lne_Localedit->text();
    QString rport = lne_Remoteedit->text();
    QString proto = cob_Protocoledit->currentText();
    Data->clear();
    do{
        if(ip1.isEmpty() || ip2.isEmpty() || ip3.isEmpty() || ip4.isEmpty() ||
                url.isEmpty() || lport.isEmpty() || rport.isEmpty()){
            QMessageBox::warning(this,"Warning","Don't have null edit!");
            break;
        }

        if(proto == "--proto--")
            proto = "------";
        Data->append(proto);
        Data->append(ip1+"."+ip2+"."+ip3+"."+ip4);
        Data->append(lport);
        Data->append(rport);
        Data->append(url);

        //1.显示在tablewidget上,并传给驱动
        if(addBlacktw()){
            QMessageBox::warning(this,"Warning","black list repeated!");
            break;
        }

        int p = getProtocol(proto);
        //2.刷新数据库
        if(!mysql->insertNetHead(ip,lport,rport,url,userID,p)){
            QMessageBox::warning(this,"warning","insert database failed!");
        }

    }while(false);

}

//右键点击Delete按钮的操作
void NetHeadOpera_Setting::allowNethead()
{
    //---------------

    QString ip1 = lne_Ip1edit->text();
    QString ip2 = lne_Ip2edit->text();
    QString ip3 = lne_Ip3edit->text();
    QString ip4 = lne_Ip4edit->text();
    QString ip = ip1 + "." + ip2 + "." +ip3 + "." +ip4;
    QString url = lne_Urledit->text();
    QString lport = lne_Localedit->text();
    QString rport = lne_Remoteedit->text();
    QString proto = cob_Protocoledit->currentText();
    do{

        if(ip1.isEmpty() || ip2.isEmpty() || ip3.isEmpty() || ip4.isEmpty()){
            QMessageBox::warning(this,"Warning","Don't have null edit!");
            break;
        }
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
        BL.ProtoType = getProtocol(proto);
        int p = getProtocol(proto);
        //MAC
        //暂定忽略
        //先打开驱动
        if(ndis->openDriver()){
            //打开成功后进行驱动数据传输
            if(ndis->deleteforbiddenipPortMacUrlProto(BL)){
                //这儿进行删除TableWidget一行操作
                QList<QTableWidgetItem*> items = blacktw->selectedItems();
                int row = items.at(0)->row();
                blacktw->removeRow(row);
                //驱动传输完毕，这儿进行数据库操作
                if(!mysql->deleteNetHead(ip,lport,rport,url,userID,p))
                    QMessageBox::information(this,"Warning","nethead sql Operation Failed!");
                else
                    QMessageBox::information(this,"Info","NetData rule delete Success");
            }else
                QMessageBox::warning(this,"Warning","NetData rule delete Failed");
        }else
            QMessageBox::warning(this,"Warning","Open Driver Failed!");
    }while(false);
}

//显示此行明细
void NetHeadOpera_Setting::showDetail()
{
    QVector<QString> vs;
    QList<QTableWidgetItem *> curItems = blacktw->selectedItems();
    for(int i=0;i<curItems.length();i++)
         vs.append(curItems.at(i)->text());
    QMessageBox::information(this,"Item Detial","<b>Portocol:</b>"+vs.at(0)+
                             "<br><b>Ip:</b>"+vs.at(1)+
                             "<br><b>Localport:</b>"+vs.at(2)+
                             "<br><b>Remoteport:</b>"+vs.at(3)+
                             "<br><b>URL:</b>"+vs.at(4));
}

//---------------------WidgetEvent------------------------------
//关闭事件重写
void NetHeadOpera_Setting::closeEvent(QCloseEvent *e)
{
    this->hide();
    e->ignore();
}

//paint事件
void NetHeadOpera_Setting::paintEvent(QPaintEvent *p)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);//反锯齿
    painter.setBrush(QBrush(QColor(232,232,232)));
    painter.setPen(QColor(Qt::black));
    QRect rect = this->rect();
    rect.setWidth(rect.width()-1);
    rect.setHeight(rect.height()-1);
    painter.drawRoundedRect(rect,15,15);
    painter.drawLine(QPoint(0,closeIt->pos().y()+closeIt->height()+2),QPoint(rect.width(),closeIt->pos().y()+closeIt->height()+2));

    QWidget::paintEvent(p);
}

//Mousepress事件重写
void NetHeadOpera_Setting::mousePressEvent(QMouseEvent *e)
{
    moving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QWidget::mousePressEvent(e);
}

//Mousemove事件重写
void NetHeadOpera_Setting::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(moving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QWidget::mouseMoveEvent(e);
}

//Mouserelease事件重写
void NetHeadOpera_Setting::mouseReleaseEvent(QMouseEvent *e)
{
    moving = false;
    return QWidget::mouseReleaseEvent(e);
}
