#include "register.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QTime>
#include <QDebug>
#include <windows.h>
#include <QRegExp>
#include <QMessageBox>
#include <QInputDialog>

bool rmoving = false;
bool RightShow = false;

//构造函数
Register::Register(QWidget *parent,ConMySQL *Mysql,QString Localhost,QString Dbname,QString Admin,QString Password):QDialog(parent)
{
    //注册用户所选的权限集
    registerJust = new QVector<int>();
    //获得数据库操作类的指针
    mysql = Mysql;
    //localhost
    localhost = Localhost;
    //dbname
    dbname = Dbname;
    //admin
    Adminname = Admin;
    //password
    Adminpassword = Password;
    //当前用户ID
    curIDset = 0;

    lib<<"0"<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<
       "a"<<"b"<<"c"<<"d"<<"e"<<"f"<<"g"<<"h"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"<<"o"<<"p"<<"q"<<"r"<<"s"<<"t"<<"u"<<"v"<<"w"<<"x"<<"y"<<"z"<<
       "A"<<"B"<<"C"<<"D"<<"E"<<"F"<<"G"<<"H"<<"I"<<"J"<<"K"<<"L"<<"M"<<"N"<<"O"<<"P"<<"Q"<<"R"<<"S"<<"T"<<"U"<<"V"<<"W"<<"X"<<"Y"<<"Z";

    t = QTime::currentTime();

    btn_Cancel = new QPushButton("Cancel");
    btn_Create = new QPushButton("Create");
    connect(btn_Cancel,SIGNAL(clicked()),this,SLOT(close()));
    connect(btn_Create,SIGNAL(clicked()),this,SLOT(createAccount()));

    btn_randomGener = new QPushButton("Random_Create");
    connect(btn_randomGener,SIGNAL(clicked()),this,SLOT(GenerateAccount()));

    btn_user_defined = new QPushButton("Role-Define");
    connect(btn_user_defined,SIGNAL(clicked()),this,SLOT(showRightgather()));

    lab_userName = new QLabel("username:");
    lab_passWord = new QLabel("password");
    lne_passWord = new QLineEdit();
    lne_passWord->setMaxLength(20);
    lne_passWord->setToolTip("within 20");
    lne_userName = new QLineEdit();
    lne_userName->setMaxLength(10);
    lne_userName->setToolTip("within 10");

    lab_level = new QLabel("SecurityLevel:");
    cob_securityLevel = new QComboBox();
    QStringList levels;
    levels<<"---level---"
         <<"   advance"
        <<"   common"
       <<"   guest";
    cob_securityLevel->addItems(levels);

    QGridLayout *v_m = new QGridLayout();

    QHBoxLayout *h_username = new QHBoxLayout();
    h_username->addWidget(lab_userName);
    h_username->addWidget(lne_userName);

    QHBoxLayout *h_password = new QHBoxLayout();
    h_password->addWidget(lab_passWord);
    h_password->addWidget(lne_passWord);

    QHBoxLayout *h_level = new QHBoxLayout();
    h_level->addWidget(lab_level);
    h_level->addWidget(cob_securityLevel);

    QHBoxLayout *h_random = new QHBoxLayout();
    h_random->addWidget(btn_randomGener);
    h_random->addWidget(btn_user_defined);

    QHBoxLayout *h_judge = new QHBoxLayout();
    h_judge->addWidget(btn_Create);
    h_judge->addWidget(btn_Cancel);


    v_m->addWidget(new QLabel(" "),0,1,1,1);
    v_m->addWidget(new QLabel(" "),0,1,1,1);
    v_m->addLayout(h_username,2,1,1,1);
    v_m->addLayout(h_password,3,1,1,1);
    v_m->addLayout(h_level,4,1,1,1);
    v_m->addLayout(h_random,6,1,1,1);
    v_m->addWidget(new QLabel(""));
    v_m->addLayout(h_judge,8,1,1,1);

    QString *title = new QString("--Right Gather--");
    rightGather = new QGroupBox(*title);

    //NetHead
    ckb_NetHeadForbidden = new QCheckBox("NetProcessForbidden");
    connect(ckb_NetHeadForbidden,SIGNAL(stateChanged(int)),this,SLOT(netheadForbidden(int)));

    //Process
    ckb_ProcessForbiddenOpen = new QCheckBox("ProcessForbiddengOpen");
    connect(ckb_ProcessForbiddenOpen,SIGNAL(stateChanged(int)),this,SLOT(processCreate(int)));
    ckb_NetprocessForbidden = new QCheckBox("ProcessForbiddenNet");
    connect(ckb_NetprocessForbidden,SIGNAL(stateChanged(int)),this,SLOT(processNet(int)));

    //File
    ckb_Fileall = new QCheckBox("FileAll");
    connect(ckb_Fileall,SIGNAL(stateChanged(int)),this,SLOT(fileAll(int)));
    ckb_Fileexcute = new QCheckBox("FileExcute");
    connect(ckb_Fileexcute,SIGNAL(stateChanged(int)),this,SLOT(fileExcute(int)));
    ckb_Filemodify = new QCheckBox("FileModify");
    connect(ckb_Filemodify,SIGNAL(stateChanged(int)),this,SLOT(fileModify(int)));
    ckb_Fileread = new QCheckBox("FileRead");
    connect(ckb_Fileread,SIGNAL(stateChanged(int)),this,SLOT(fileRead(int)));
    ckb_Filewrite = new QCheckBox("FileWrite");
    connect(ckb_Filewrite,SIGNAL(stateChanged(int)),this,SLOT(fileWrite(int)));
    ckb_Filecopy = new QCheckBox("FileCopy");
    connect(ckb_Filecopy,SIGNAL(stateChanged(int)),this,SLOT(fileCopy(int)));

    QVBoxLayout *v_f = new QVBoxLayout();
    v_f->addWidget(ckb_Fileall);
    v_f->addWidget(ckb_Fileexcute);
    v_f->addWidget(ckb_Filemodify);
    v_f->addWidget(ckb_Filecopy);
    v_f->addWidget(ckb_Fileread);
    v_f->addWidget(ckb_Filewrite);

    QVBoxLayout *v_np = new QVBoxLayout();
    v_np->addWidget(ckb_NetHeadForbidden);
    v_np->addWidget(ckb_NetprocessForbidden);
    v_np->addWidget(ckb_ProcessForbiddenOpen);

    QHBoxLayout *h_gb = new QHBoxLayout();
    h_gb->addLayout(v_f);
    h_gb->addLayout(v_np);

    rightGather->setLayout(h_gb);

    rightGather->hide();

    QHBoxLayout *h_m = new QHBoxLayout();
    h_m->addLayout(v_m);
    h_m->addWidget(rightGather);

    this->setLayout(h_m);

    this->setFixedSize(300,250);
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint);   //无边框
}

//析构函数
Register::~Register()
{

}

//生成随机字符串
QString Register::randomGenerate()
{
    QTime time = QTime::currentTime();
    int s = t.msecsTo(time);
    //设置随机种子
    qsrand(s);
    int random = qrand()%61;
    //t用来记录前一次的时间
    t = time;
    return lib.at(random);
}

//从数据库读取用户列表，为了避免重复，获取到最大的用户ID
void Register::readUserfromDB()
{
    QSqlQuery q = mysql->getUserID();
    //将用户名列表存入users
    while(q.next()){
        users.append(q.value(2).toString());
        //使curIDset设置为当前已有用户的最大ID
        if(q.value(0).toInt() > curIDset)
            curIDset = q.value(0).toInt();
    }
}

//清空当前所有控件数据
void Register::clearAll()
{
    lne_userName->clear();
    lne_passWord->clear();
    cob_securityLevel->setCurrentIndex(0);
}

//利用当前选中的权限查找权限集合----角色
int Register::checkRoleJurisdiction()
{
    //匹配上的角色ID
    int MatchRoleID = -1;
    //建立一个一对多key->justice结构
    QMap<int,QVector<int> > map;
    do{
        //先读取role表
        QSqlQuery s = mysql->getRole();
        if(&s == NULL)
            break;
        //将读取到的roleID读取至roleID集合中
        //并初始化map的key-value对
        while(s.next())
            map.insert(s.value(0).toInt(),*(new QVector<int>()));
        //读取整个权限表
        QSqlQuery sql = mysql->getJurisdiction();
        /*
         * 这儿利用的思想是:
         *  因为之前已经初始化了map一对多集合，也对每一个Key初始化了value
         *  再利用map的Key不能重复的特点，也就是可以不断insert集合中已有
         *  的key值，不断更新key对应的value.
         *  所以，在这儿就直接可以利用重复insert来更新map表中roleID对应一个
         *  jurisdiction集合的情况。
        */
        if(&sql != NULL){
            while(sql.next()){
                //当前角色ID
                int roleID = sql.value(0).toInt();
                //将当前行的角色值对应的已读取的权限集合读出
                QVector<int> jurisdictions = map.value(roleID);
                //push进当前行中角色ID的权限ID
                jurisdictions.push_back(sql.value(1).toInt());
                //更新不能key重复的map
                map.insert(roleID,jurisdictions);
            }
        }else break;

        //比较当前拥有的角色对应的角色权限集合是否存在
        QMutableMapIterator<int,QVector<int> > iterator(map);
        while(iterator.hasNext()){
            iterator.next();
            //用来记录当前角色与已选权限集合的权限匹配数目
            int count = 0;
            for(int i=0;i<iterator.value().size();i++){
                //检测当前“已选的权限集合”是否包含当前角色的单一权限
                if(registerJust->contains(iterator.value().at(i)))
                    count++;
            }
            //利用得到的包含数目比较
            //当前角色权限数:   iterator.value().size()
            //当前已选权限数:   registerJust.size()
            //两者匹配上的数目:  count
            //匹配数 == 当前已选权限数 && 当前已选权限数 == 当前角色权限数
            //表示已经匹配上
            if(count == registerJust->size() &&
                    registerJust->size() == iterator.value().size()){
                //当前角色ID
                MatchRoleID = iterator.key();
                QMessageBox::information(this,"info","matchRoleID:"+QString::number(MatchRoleID));
                break;
            }
        }
    }while(false);
    return MatchRoleID;
}

//processNet
void Register::processNet(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(PROCESS_NET);
    }else if(registerJust->contains(PROCESS_NET)){
        int index = registerJust->indexOf(PROCESS_NET);
        registerJust->remove(index);
    }
}

//processCreate
void Register::processCreate(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(PROCESS_CREATE);
    }else if(registerJust->contains(PROCESS_CREATE)){
        int index = registerJust->indexOf(PROCESS_CREATE);
        registerJust->remove(index);
    }
}

//netheadForbidden
void Register::netheadForbidden(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(NETHEAD_OPEN);
    }else if(registerJust->contains(NETHEAD_OPEN)){
        int index = registerJust->indexOf(NETHEAD_OPEN);
        registerJust->remove(index);
    }
}

//文件:修改
void Register::fileModify(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(FILE_MODIFY);
    }else if(registerJust->contains(FILE_MODIFY)){
        int index = registerJust->indexOf(FILE_MODIFY);
        registerJust->remove(index);
    }
}

//文件:写
void Register::fileWrite(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(FILE_WRITE);
    }else if(registerJust->contains(FILE_WRITE)){
        int index = registerJust->indexOf(FILE_WRITE);
        registerJust->remove(index);
    }
}

//文件:读
void Register::fileRead(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(FILE_READ);
    }else if(registerJust->contains(FILE_READ)){
        int index = registerJust->indexOf(FILE_READ);
        registerJust->remove(index);
    }
}

//文件:复制
void Register::fileCopy(int sta)
{
    if(sta == Qt::Checked){
        registerJust->push_back(FILE_COPY);
    }else if(registerJust->contains(FILE_COPY)){
        int index = registerJust->indexOf(FILE_COPY);
        registerJust->remove(index);
    }
}

//文件:执行
void Register::fileExcute(int s)
{
    if(s == Qt::Checked){
        registerJust->push_back(FILE_EXCUTE);
    }else if(registerJust->contains(FILE_EXCUTE)){
        int index = registerJust->indexOf(FILE_EXCUTE);
        registerJust->remove(index);
    }
}

//文件:权限全选
void Register::fileAll(int stat)
{
    bool c = true;
    if(stat == Qt::Checked){
        c=true;
    }else if(stat == Qt::Unchecked){
        c=false;
    }
    //选中所有文件权限
    ckb_Filecopy->setChecked(c);
    ckb_Fileexcute->setChecked(c);
    ckb_Filemodify->setChecked(c);
    ckb_Fileread->setChecked(c);
    ckb_Filewrite->setChecked(c);
}

//显示权限集合
void Register::showRightgather()
{
    if(!RightShow){
         rightGather->show();
         RightShow = true;
         this->setFixedSize(300+rightGather->width()*2,250);
    }else{
        rightGather->hide();
        RightShow = false;
        this->setFixedSize(300,250);
    }
}

//随机生成账户
void Register::GenerateAccount()
{
    QString user,password;
    //随机生成用户名
    for(int i=0;i<10;i++){
         user+=randomGenerate();
    }

    //随机生成密码
    for(int j=0;j<20;j++){
        password+=randomGenerate();
    }

    lne_userName->setText(user);
    lne_passWord->setText(password);
}

//依照当前用户选取的字符串进行整数匹配
int SecurityMatch(QString level){

    if(level == "---level---"){
        return 0;
    }else if(level == "   advance"){
        return 1;
    }else if(level == "   common"){
        return 2;
    }else if(level == "   guest"){
        return 3;
    }
    return 0;
}

//创建账户
void Register::createAccount()
{
    QString username = lne_userName->text();
    QString password = lne_passWord->text();
    QString secl = cob_securityLevel->currentText();
    //用户权限等级匹配
    int sl = SecurityMatch(secl);
    if(sl == 0){
        QMessageBox::warning(this,"warning","U Must select security level!");
        return;
    }
    //将当前users里面存储的用户信息(username,ID)读取出来
    //获取当前最大的userID
    readUserfromDB();
    //与数据库中读取到的账户名进行比较，是否重复
    if(users.contains(lne_userName->text())){
        //提示用户名重复
        QMessageBox::warning(this,"Warning","UserName have repeated!");
        //清空控件数据
        this->clearAll();
    }else{//用户不重复则继续进行注册操作
        do{
            //开始进行创建数据库的用户创建
            //++curIDset是为了获得新建用户的userID
            if(mysql->isOpen()){
                ++curIDset;
            }else{//数据库没有打开则进行打开并创建用户操作
                if(mysql->OpenDB(localhost,dbname,Adminname,Adminpassword)){
                    ++curIDset;
                }else break;
            }
            //创建用户并插入users表中
            if(mysql->CreateUser(curIDset,sl,dbname,localhost,username,password,*registerJust)){
                QMessageBox::information(this,"Congratulations","Create User successful!");
                //将用户权限集合写入user_role表中,并对应UserID
                //说明没有选中权限
                //默认：全选权限;安全等级必选
                //此处权限未选,用的是默认角色，即用户安全等级自动生成的角色
                if(registerJust->isEmpty()){
                    //安全等级非guest,没选权限
                    if(sl != 3){
                        //若注册用户选项不是guest安全等级
                        //其它安全等级则是权限全选
                        //只需插入user_role表
                        mysql->insertUserRole(curIDset,sl);
                    }else//安全等级为guest,没选权限
                        QMessageBox::information(this,"in-fo","guest don't have any jurisdictions on default!");
                }else{
                    //已选权限
                    //判断是否为新的role
                    int existRoleID = this->checkRoleJurisdiction();
                    //旧的role
                    if(existRoleID != -1){
                        //插入user_role表即可
                        if(!mysql->insertUserRole(curIDset,existRoleID)){
                            QMessageBox::warning(this,"warning","insert user_role failed!");
                        }
                    }else{//新role,插入权限表并让用户命名
                        //求出最大的roleID
                        QSqlQuery sql = mysql->getRole();
                        int newRoleID = 0;
                        while(sql.next()){
                            if(sql.value(0).toInt() >= newRoleID)
                                newRoleID = sql.value(0).toInt();
                        }
                        newRoleID++;
                        //让用户命名一个新的角色名
                        QString roleName =
                                QInputDialog::getText(this,"Naming a role","Please Naming a role");
                        //插入role表
                        mysql->insertRole(newRoleID,roleName);
                        //插入role_jurisdiction表
                        foreach(int a,*registerJust){
                            if(!mysql->insertUserRoleJurisdiction(newRoleID,a)){
                                QMessageBox::warning(this,"warning","set role failed!");
                                break;
                            }
                        }
                        //插入user_role表
                        mysql->insertUserRole(curIDset,newRoleID);
                    }
                }
            }else//创建用户失败
                QMessageBox::warning(this,"Error","Create User Failed!");
        }while(false);
    }
}

//---------------------WidgetEvent------------------------------
//paint事件重写
void Register::paintEvent(QPaintEvent *p)
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
//press事件重写
void Register::mousePressEvent(QMouseEvent *e)
{
    rmoving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QDialog::mousePressEvent(e);
}
//move事件重写
void Register::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(rmoving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QDialog::mouseMoveEvent(e);
}
//release事件重写
void Register::mouseReleaseEvent(QMouseEvent *e)
{
    rmoving = false;
    return QDialog::mouseReleaseEvent(e);
}
//close事件重写
void Register::closeEvent(QCloseEvent *e)
{
    clearAll();
    QWidget::closeEvent(e);
}

