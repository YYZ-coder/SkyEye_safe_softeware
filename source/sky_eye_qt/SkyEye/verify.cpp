#include "verify.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>

bool vmoving = false;

Verify::Verify(QWidget *parent) : QDialog(parent)
{
    sql = new ConMySQL();
    ssdt = new SSDT_Operate();

    verify = new QPushButton("verify");
    verify->setDefault(true);
    cancel = new QPushButton("cancel");

    dbAdmin = new QLineEdit();
    dbAdmin->setMaxLength(10);
    dbAdmin->setToolTip("lengths within 10");
    dbPass = new QLineEdit();
    dbPass->setMaxLength(20);
    dbPass->setEchoMode(QLineEdit::Password);
    dbPass->setToolTip("lengths within 20");
    dbHostname = new QLineEdit();
    dbHostname->setMaxLength(10);
    dbHostname->setToolTip("lengths within 10");
    dbDBname = new QLineEdit();
    dbDBname->setMaxLength(10);
    dbDBname->setToolTip("lengths within 10");
    SysAdmin = new QLineEdit();
    SysAdmin->setToolTip("lengths within 10");
    SysAdmin->setMaxLength(10);
    SysPass = new QLineEdit();
    SysPass->setToolTip("lengths within 20");
    SysPass->setMaxLength(20);

    dba = new QLabel("<b>DataBase</b>|ownerUser:");
    dbp = new QLabel("<b>DataBase</b>|ownerPassword:");
    dbhost = new QLabel("<b>DataBase</b>|Hostname:");
    dbname = new QLabel("<b>DataBase</b>|DBname:");
    horiLine = new QLabel();
    horiLine->setFrameStyle(QFrame::HLine|QFrame::Raised);
    sysa = new QLabel("<b>Sys</b>|Admin:");
    sysp = new QLabel("<b>Sys</b>|Password:");

    QHBoxLayout *h_dbn = new QHBoxLayout();
    h_dbn->addWidget(dba);
    h_dbn->addWidget(dbAdmin);
    QHBoxLayout *h_dbp = new QHBoxLayout();
    h_dbp->addWidget(dbp);
    h_dbp->addWidget(dbPass);
    QHBoxLayout *h_dbh = new QHBoxLayout();
    h_dbh->addWidget(dbhost);
    h_dbh->addWidget(dbHostname);
    QHBoxLayout *h_dbnn = new QHBoxLayout();
    h_dbnn->addWidget(dbname);
    h_dbnn->addWidget(dbDBname);

    QHBoxLayout *h_syn = new QHBoxLayout();
    h_syn->addWidget(sysa);
    h_syn->addWidget(SysAdmin);
    QHBoxLayout *h_syp = new QHBoxLayout();
    h_syp->addWidget(sysp);
    h_syp->addWidget(SysPass);
    QHBoxLayout *h_btn = new QHBoxLayout();
    h_btn->addWidget(verify);
    h_btn->addWidget(cancel);

    QVBoxLayout *v_m = new QVBoxLayout();
    v_m->addWidget(new QLabel(""));
    v_m->addWidget(new QLabel(""));
    v_m->addLayout(h_dbh);
    v_m->addLayout(h_dbnn);
    v_m->addLayout(h_dbn);
    v_m->addLayout(h_dbp);
    v_m->addWidget(horiLine);
    v_m->addLayout(h_syn);
    v_m->addLayout(h_syp);
    v_m->addWidget(new QLabel(""));
    v_m->addLayout(h_btn);

    setLayout(v_m);
    setFixedSize(300,250);
    setWindowIcon(QIcon(":/owl0.ico"));
    setToolTip("Setting");
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint);   //无边框

    connect(verify,SIGNAL(clicked()),this,SLOT(VerifyIt()));
    connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
}

Verify::~Verify()
{

}

//进行数据库拥有者验证，并且发送数据给注册表
void Verify::VerifyIt()
{
    do{
        QRegExp re("^[A-z0-9]*$");
        //字符串验证
        if((!re.exactMatch(dbAdmin->text()))||(!re.exactMatch(dbPass->text()))){
            QMessageBox::information(this,"verify char","You have illegal characters!\n"
                                                        "Attention : just have '0'->'z' in Ascii!!");
            dbAdmin->setText("");
            dbPass->setText("");
        }else{
            //打开数据库验证
            if(sql->OpenDB(dbHostname->text(),dbDBname->text(),dbAdmin->text(),dbPass->text())){
                    //创建用户并删除用户验证
                     if(sql->verifyDB(dbDBname->text(),dbHostname->text())){
                         //插入users表
                         if(sql->insertUser(dbAdmin->text(),0,0)){
                             //创建新的(user<->role)插入user_role表
                             sql->insertUserRole(0,1);
                             //此处不需要再插入角色权限表rolr_jurisdiction
                             QMessageBox::information(NULL,"Info","Insert admin success");
                         }
                         //发送给SSDT并写入注册表
                         if(ssdt->openDriver()){
                             if(ssdt->sendRegistryData(SysAdmin->text(),SysPass->text(),dbDBname->text(),dbHostname->text())){
                                 QMessageBox::information(this,"Congratulations","Init Parameters setting success!");
                                accept();
                                //关闭数据库
                                sql->closeDB();
                                this->close();
                                //为了写入注册表后可以正常安照当前管理员账户登陆,所以需要发送信号带参数给主窗口
                                ForDB fdb;
                                fdb.dbname = dbDBname->text();
                                fdb.host = dbHostname->text();
                                fdb.username = dbAdmin->text();
                                fdb.password = dbPass->text();

                                emit autologinRoot(fdb);
                             }else
                                 QMessageBox::information(this,"Status","Send registry failed!");
                         }else
                             QMessageBox::warning(this,"Driver open status","Driver open Failed!");
                     }
                     else
                         QMessageBox::information(this,"Db Status","verified failed !\nU are not DB owner!");
            }
            else
                QMessageBox::information(this,"Db Status","Input Error!");
        }

    }while(false);

}

void Verify::accept()
{
    QDialog::accept();
}

//---------------------WidgetEvent------------------------------
//鼠标按压事件重写
void Verify::mousePressEvent(QMouseEvent *e)
{
    vmoving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QDialog::mousePressEvent(e);
}
//鼠标移动事件重写
void Verify::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(vmoving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QDialog::mouseMoveEvent(e);
}
//鼠标释放事件重写
void Verify::mouseReleaseEvent(QMouseEvent *e)
{
    vmoving = false;
    return QDialog::mouseReleaseEvent(e);
}
//paint事件重写
void Verify::paintEvent(QPaintEvent *p)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);//反锯齿
    painter.setBrush(QBrush(QColor(232,232,232)));
    painter.setPen(QColor(Qt::black));
    QRect rect = this->rect();
    rect.setWidth(rect.width()-1);
    rect.setHeight(rect.height()-1);
    painter.drawRoundedRect(rect,15,15);
    QWidget::paintEvent(p);
}
//关闭事件重写
void Verify::closeEvent(QCloseEvent *e)
{
    qDebug()<<"verify close";
    e->accept();
}


