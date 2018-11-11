#include "wholefileopera_setting.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QFile>
#include <QDir>

//窗口移动状态
bool fmoving = false;

//构造函数
WholeFileOpera_Setting::WholeFileOpera_Setting(QWidget *parent,Pipe_Operation *pipe,int userid) : QWidget(parent)
{
    lab_ProcessID = new QLabel("processID:");
    lne_ProcessID = new QLineEdit();

    backFeed = new waitForBack();
    connect(backFeed,SIGNAL(haveData()),this,SLOT(gotReadFromBack()));
    //等待线程开始启动
    backFeed->start();
    ckb_List = new QVector<QString>();

    curComboText = "public";
    //当前登陆用户的ID
    this->userID = userid;
    //初始化管道操作类
    this->fpipe = pipe;
    model = new QSqlQueryModel();
    model->setQuery("select * from file");
    //多功能按钮
    closeIt = new MultiBtn(this);
    closeIt->setMulti("-");
    closeIt->setFixedSize(30,20);
    closeIt->setStyleSheet("QPushButton{background : transparent;}");
    connect(closeIt,SIGNAL(clicked()),this,SLOT(close()));
    //黑名单列表
    blacktl = new QTableView();
    blacktl->setModel(model);
    blacktl->show();
    /*QStringList list;
    list<<"FilePath"<<"FileName"<<"ProcessName"<<"Result"<<"Limit Type";
    blacktl->setHorizontalHeaderLabels(list);
    blacktl->setSelectionBehavior(QAbstractItemView::SelectRows);
    blacktl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blacktl->horizontalHeader()->setHighlightSections(false);*/
    //控件初始化
    lab_state = new QLabel("");
    lab_filePath = new QLabel("FilePath:");
    lne_filePath = new QLineEdit();
    lne_filePath->setToolTip("within 1024 length");
    lne_filePath->setMaxLength(1024);
    connect(lne_filePath,SIGNAL(textChanged(QString)),this,SLOT(func_lne_textChanged(QString)));

    btn_add = new QPushButton("Add");
    btn_add->setFixedWidth(50);
    connect(btn_add,SIGNAL(clicked()),this,SLOT(addBtn()));

    lab_securityLevel = new QLabel("File Level:");
    cob_securityLevel = new QComboBox();
    cob_securityLevel->setToolTip("public lowest");
    QStringList cob;
    cob<<"public"<<"nomal"<<"private"<<"senior";
    //倘若是管理员则加载"system"等级
    if(userid == 0){
        cob.push_back("system");
    }
    cob_securityLevel->addItems(cob);
    cob_securityLevel->setCurrentIndex(0);
    connect(cob_securityLevel,SIGNAL(currentIndexChanged(QString)),this,SLOT(cobChanged(QString)));

    initialDirControl();
    initialFileControl();

    QHBoxLayout *h_head = new QHBoxLayout();
    h_head->addSpacing(this->width()-30);
    h_head->addWidget(closeIt);
    QHBoxLayout *h_filepath = new QHBoxLayout();
    h_filepath->addWidget(lab_filePath);
    h_filepath->addWidget(lne_filePath);
    h_filepath->addWidget(lab_state);
    QHBoxLayout *h_add = new QHBoxLayout();
    QVBoxLayout *v_limit = new QVBoxLayout();
    v_limit->addWidget(lab_securityLevel);
    v_limit->addWidget(cob_securityLevel);
    h_add->addLayout(v_limit);
    h_add->addWidget(btn_add);
    QVBoxLayout *v_DF = new QVBoxLayout();
    v_DF->addLayout(h_File);
    v_DF->addLayout(h_Dir);
    QHBoxLayout *h_pid = new QHBoxLayout();
    h_pid->addWidget(lab_ProcessID);
    h_pid->addWidget(lne_ProcessID);
    QVBoxLayout *v_m = new QVBoxLayout();
    v_m->addLayout(h_head);
    v_m->addLayout(h_filepath);
    v_m->addLayout(h_add);
    v_m->addLayout(v_DF);
    v_m->addLayout(h_pid);
    v_m->addWidget(blacktl);

    //初始隐藏Dir / File控件集合
    hideDirControl();
    hideFileControl();
    lab_ProcessID->hide();
    lne_ProcessID->hide();

    setLayout(v_m);
    setFixedSize(500,300);
    setWindowIcon(QIcon(":/owl0.ico"));
    setAttribute(Qt::WA_TranslucentBackground);  //背景透明
    setWindowFlags(Qt::FramelessWindowHint);     //无边框
}

//析构函数
WholeFileOpera_Setting::~WholeFileOpera_Setting()
{

}

//初始化文件夹权限控件
void WholeFileOpera_Setting::initialDirControl()
{
    h_Dir = new QHBoxLayout();
    h_d1 = new QHBoxLayout();
    h_d2 = new QHBoxLayout();
    ckb_DeleteDir = new QCheckBox("Dir:Delete");
    ckb_MoveDir = new QCheckBox("Dir:Move");
    ckb_newDir = new QCheckBox("Dir:New one");
    ckb_RenameDir = new QCheckBox("Dir:Rename");
    ckb_ReadDir = new QCheckBox("Dir:Read");
    ckb_WriteDir = new QCheckBox("Dir:Write");
    ckb_AllDir = new QCheckBox("Dir:All");

    h_d1->addWidget(ckb_DeleteDir);
    h_d1->addWidget(ckb_MoveDir);
    h_d1->addWidget(ckb_newDir);
    h_d1->addWidget(ckb_RenameDir);
    h_d2->addWidget(ckb_ReadDir);
    h_d2->addWidget(ckb_WriteDir);
    h_d2->addWidget(ckb_AllDir);
    h_Dir->addLayout(h_d1);
    h_Dir->addLayout(h_d2);

    connect(ckb_DeleteDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_DeleteDir(int)));
    connect(ckb_MoveDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_MoveDir(int)));
    connect(ckb_newDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_newDir(int)));
    connect(ckb_RenameDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_RenameDir(int)));
    connect(ckb_ReadDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_ReadDir(int)));
    connect(ckb_WriteDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_WriteDir(int)));
    connect(ckb_AllDir,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_AllDir(int)));
}

//显示Dir控件
void WholeFileOpera_Setting::showDirControl()
{
    ckb_DeleteDir->show();
    ckb_MoveDir->show();
    ckb_newDir->show();
    ckb_RenameDir->show();
    ckb_ReadDir->show();
    ckb_WriteDir->show();
    ckb_AllDir->show();

    ckb_DeleteFile->setChecked(false);
    ckb_MoveFile->setChecked(false);
    ckb_RenameFile->setChecked(false);
    ckb_ReadFile->setChecked(false);
    ckb_WriteFile->setChecked(false);
    ckb_AllFile->setChecked(false);
}

//隐藏Dir控件
void WholeFileOpera_Setting::hideDirControl()
{
    ckb_DeleteDir->hide();
    ckb_MoveDir->hide();
    ckb_newDir->hide();
    ckb_RenameDir->hide();
    ckb_ReadDir->hide();
    ckb_WriteDir->hide();
    ckb_AllDir->hide();
}

//初始化文件权限控件
void WholeFileOpera_Setting::initialFileControl()
{
    h_File = new QHBoxLayout();
    h_f1 = new QHBoxLayout();
    h_f2 = new QHBoxLayout();

    ckb_DeleteFile = new QCheckBox("File:Delete");
    ckb_MoveFile = new QCheckBox("File:Move");
    ckb_RenameFile = new QCheckBox("File:Rename");
    ckb_ReadFile = new QCheckBox("File:Read");
    ckb_WriteFile = new QCheckBox("File:Write");
    ckb_AllFile = new QCheckBox("File:All");

    h_f1->addWidget(ckb_DeleteFile);
    h_f1->addWidget(ckb_MoveFile);
    h_f1->addWidget(ckb_RenameFile);
    h_f2->addWidget(ckb_ReadFile);
    h_f2->addWidget(ckb_AllFile);
    h_f2->addWidget(ckb_WriteFile);
    h_File->addLayout(h_f1);
    h_File->addLayout(h_f2);

    connect(ckb_DeleteFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_DeleteFile(int)));
    connect(ckb_MoveFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_MoveFile(int)));;
    connect(ckb_RenameFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_RenameFile(int)));
    connect(ckb_ReadFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_ReadFile(int)));
    connect(ckb_WriteFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_WriteFile(int)));
    connect(ckb_AllFile,SIGNAL(stateChanged(int)),this,SLOT(func_ckb_AllFile(int)));
}

//显示File控件
void WholeFileOpera_Setting::showFileControl()
{
    ckb_MoveFile->show();
    ckb_DeleteFile->show();
    ckb_RenameFile->show();
    ckb_WriteFile->show();
    ckb_ReadFile->show();
    ckb_AllFile->show();

    ckb_DeleteDir->setChecked(false);
    ckb_MoveDir->setChecked(false);
    ckb_newDir->setChecked(false);
    ckb_RenameDir->setChecked(false);
    ckb_ReadDir->setChecked(false);
    ckb_WriteDir->setChecked(false);
    ckb_AllDir->setChecked(false);
}

//隐藏File控件
void WholeFileOpera_Setting::hideFileControl()
{
    ckb_DeleteFile->hide();
    ckb_MoveFile->hide();
    ckb_RenameFile->hide();
    ckb_AllFile->hide();
    ckb_WriteFile->hide();
    ckb_ReadFile->hide();
}

//提取文件安全等级
int WholeFileOpera_Setting::fileSecurityLevel(QString fl)
{
    //"public"<<"nomal"<<"private"<<"senior"<<"system"
    if(fl == "public")
        return 5;
    else if(fl == "nomal")
        return 4;
    else if(fl == "senior")
        return 2;
    else if(fl == "system")
        return 1;
    else if(fl == "private")
        return 3;
}

//清空checkBox
void WholeFileOpera_Setting::clearCkb()
{
}

//从后台线程获取反馈信息
void WholeFileOpera_Setting::gotReadFromBack()
{
    QString readData = backFeed->getRead();
    QMessageBox::information(this,"","read Data from back :"+readData);
    //判断返回的数据
    //决定是否读取数据库并显示在表中
    if(readData == "1")
        model->setQuery("select * from file");
}

//添加规则
void WholeFileOpera_Setting::addBtn()
{
    //首先发给后台进程进行private验证
    //发送文件路径和当前所选权限，发送给后台进程
    //等待验证状态，并进行进一步操作
    QString filePath = lne_filePath->text();
    int fps = filePath.size();
    QFile *f = new QFile(filePath);
    QDir  *d = new QDir(filePath);
    bool fa = false;
    bool isDir = false;
    bool isFile = false;
    do{ //判断路径输入是否为空
        if(filePath.isEmpty()){
            lab_state->setText(QString("<font color = 'RED'>Empty</font>"));
            hideDirControl();
            hideFileControl();
            break;
        }
        //是文件夹
        if(d->exists()){
            lab_state->setText("");
            //确定是文件夹
            isDir = true;
        }
        //不是文件夹
        if(!isDir){
            //文件不存在
            if(!f->exists()){
                lab_state->setText(QString("<font color = 'RED'>noExist</font>"));
                lne_filePath->clear();
                break;
            }else//文件存在
                isFile = true;
        }
        //提取所有已知权限
        QString pid = lne_ProcessID->text();
        int processID = 0;
        if(pid.trimmed() != "") processID = pid.toInt();

        QStringList sl;
        sl = QStringList::fromVector(*ckb_List);
        QString a = sl.join(",");
        QMessageBox::information(this,"information",a);
        //提取FileName
        QStringList list = filePath.split("\\");
        QString filename = list.last();
        WCHAR fn[64];
        int fns = filename.size();
        WCHAR fp[1024];
        //转换成宽字符
        filePath.toWCharArray(fp);
        filename.toWCharArray(fn);
        CareFile_T cft;
        ZeroMemory(&cft,sizeof(CareFile_T));
        cft.access_times = 0;
        cft.Owner = userID;
        cft.refused_times = 0;
        cft.secutiry_level = fileSecurityLevel(curComboText);
        wcsncpy(cft.filepath,fp,fps);
        wcsncpy(cft.filename,fn,fns);

        QMessageBox::information(this,"Info",QString::fromWCharArray(cft.filepath)+"\n filename:"+QString::fromWCharArray(cft.filename));

        if(fpipe != NULL){
            //发送文件属性层级控制
            if(fpipe->transferDate(&cft)){
                //读取并传递细节控制
                if(curComboText == "private"){
                    if(fpipe->getNext()){
                        QMessageBox::information(this,"warning","getNext!!!");
                        if(isFile){//传递文件细节控制
                            if(!fpipe->transferFileDetail(a,filePath,userID,processID)){
                                QMessageBox::warning(this,"","transfer File Info Failed!");
                                break;
                            }
                        }
                        if(isDir){//传递目录细节控制
                            if(!fpipe->transferDirDetail(a,filePath,userID,processID)){
                                QMessageBox::warning(this,"","transfer Dir Info Failed!");
                                break;
                            }
                        }
                        model->setQuery("select * from file");
                        QMessageBox::information(this,"Congrantulations","Add file/Dir successful!");
                    }else{
                        QMessageBox::warning(this,"warning","out of Time!");
                    }
                }
            }else
                QMessageBox::warning(this,"warning","add file failed!");
        }else
            QMessageBox::warning(this,"warning","please contact developerteam");
    }while(fa);
}

//combobox 改变,并记录
void WholeFileOpera_Setting::cobChanged(QString in)
{
    curComboText = in;
    if(in == "private"){
        lab_ProcessID->show();
        lne_ProcessID->show();
        QString filePath = lne_filePath->text();
        QFile *f = new QFile(filePath);
        QDir  *d = new QDir(filePath);
        bool isDir = false;
        //行是否输入为空
        if(filePath.trimmed() == "" || filePath == ""){
            lab_state->setText("<font color = 'RED'>Empty</font>");
            hideDirControl();
            hideFileControl();
        }
        //是文件夹
        if(d->exists()){
            lab_state->setText("");
            //隐藏
            hideFileControl();
            //展示Dir控件
            showDirControl();
            //确定是文件夹
            isDir = true;
        }
        //不是文件夹
        if(!isDir){
            //文件不存在
            if(!f->exists()){
                lab_state->setText(QString("<font color = 'RED'>noExist</font>"));
                lne_filePath->clear();
            }else{//文件存在
                lab_state->setText("");
                hideDirControl();
                showFileControl();
            }
        }
    }else{
        this->hideDirControl();
        this->hideFileControl();
        lab_ProcessID->hide();
        lne_ProcessID->hide();
        this->clearCkb();
    }
}

void WholeFileOpera_Setting::func_ckb_ReadDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_READ");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_READ"));
    }
}

void WholeFileOpera_Setting::func_ckb_ReadFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_READ");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_READ"));
    }
}

void WholeFileOpera_Setting::func_ckb_WriteDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_WRITE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_WRITE"));
    }
}

void WholeFileOpera_Setting::func_ckb_WriteFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_WRITE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_WRITE"));
    }
}

void WholeFileOpera_Setting::func_ckb_AllDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_ALL");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_ALL"));
    }
}

void WholeFileOpera_Setting::func_ckb_AllFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_ALL");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_ALL"));
    }
}

void WholeFileOpera_Setting::func_ckb_DeleteDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_DELETE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_DELETE"));
    }
}

void WholeFileOpera_Setting::func_ckb_DeleteFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_DELETE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_DELETE"));
    }
}

void WholeFileOpera_Setting::func_ckb_MoveDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_MOVE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_MOVE"));
    }
}

void WholeFileOpera_Setting::func_ckb_MoveFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_MOVE");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_MOVE"));
    }
}

void WholeFileOpera_Setting::func_ckb_newDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_NEW");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_NEW"));
    }
}

void WholeFileOpera_Setting::func_ckb_RenameDir(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("DIR_RENAME");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("DIR_RENAME"));
    }
}

void WholeFileOpera_Setting::func_ckb_RenameFile(int c)
{
    if(c == Qt::Checked){
        ckb_List->push_back("FILE_RENAME");
    }
    if(c == Qt::Unchecked){
        ckb_List->remove(ckb_List->indexOf("FILE_RENAME"));
    }
}

void WholeFileOpera_Setting::func_lne_textChanged(QString filePath)
{
    QFile *f = new QFile(filePath);
    QDir  *d = new QDir(filePath);
    bool isDir = false;
    //行是否输入为空
    if(filePath.trimmed() == "" || filePath == ""){
        lab_state->setText("<font color = 'RED'>Empty</font>");
        hideDirControl();
        hideFileControl();
    }
    //是文件夹
    if(d->exists()){
        lab_state->setText("");
        if(curComboText == "private"){
            hideFileControl();
            showDirControl();
        }
        //确定是文件夹
        isDir = true;
    }
    //不是文件夹
    if(!isDir){
        //文件不存在
        if(!f->exists()){
            lab_state->setText(QString("<font color = 'RED'>noExist</font>"));
            lne_filePath->clear();
            if(curComboText != "private"){
                hideDirControl();
                hideFileControl();
            }
        }else{//文件存在
            lab_state->setText("");
            if(curComboText == "private"){
                hideDirControl();
                showFileControl();
            }
        }
    }
}

//---------------------WidgetEvent------------------------------
//paint事件重写
void WholeFileOpera_Setting::paintEvent(QPaintEvent *p)
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

//close事件重写
void WholeFileOpera_Setting::closeEvent(QCloseEvent *c)
{
    this->hide();
    c->ignore();
}

//press事件重写
void WholeFileOpera_Setting::mousePressEvent(QMouseEvent *e)
{
    fmoving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QWidget::mousePressEvent(e);
}

//move事件重写
void WholeFileOpera_Setting::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(fmoving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QWidget::mouseMoveEvent(e);
}

//release事件重写
void WholeFileOpera_Setting::mouseReleaseEvent(QMouseEvent *e)
{
    fmoving = false;
    return QWidget::mouseReleaseEvent(e);
}

//等待线程
waitForBack::waitForBack()
{
    succ = true;
       BOOL b = WaitNamedPipeA("\\\\.\\pipe\\sendtoMainApp", 0);
       //打开管道
       readPipe = CreateFileA("\\\\.\\pipe\\sendtoMainApp",
           GENERIC_READ | GENERIC_WRITE,
           0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
       //检查是否连接成功
       if (!b || readPipe == INVALID_HANDLE_VALUE)
       {
           QMessageBox::information(NULL,"warning","Pipe CONNECT FAILED! "+QString::number(GetLastError()));
           succ = false;
       }
       else
       {
           QMessageBox::information(NULL,"warning","Pipe CONNECT SUCCESS!");
           emit haveData();
       }

}

void waitForBack::run()
{
    char message[4048];
    DWORD size;
    if(!succ){
        if(openPipe()){
            if(!ReadFile(readPipe,message,4048,&size,NULL)){
                message[size]='\0';
            }else{
                readData = QString::fromLatin1(message);
                emit haveData();
            }
        }
    }else{
        if(!ReadFile(readPipe,message,4048,&size,NULL)){
            message[size]='\0';
        }else{
            readData = QString::fromLatin1(message);
            emit haveData();
        }
    }
}

//返回读到的信息
QString waitForBack::getRead()
{
    return readData;
}

bool waitForBack::openPipe()
{
    BOOL b = WaitNamedPipeA("\\\\.\\pipe\\sendtoMainApp", 0);
    //打开管道
    readPipe = CreateFileA("\\\\.\\pipe\\sendtoMainApp",
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //检查是否连接成功
    if (!b || readPipe == INVALID_HANDLE_VALUE)
    {
        QMessageBox::information(NULL,"warning","Pipe CONNECT FAILED! "+QString::number(GetLastError()));
        succ = false;
    }
    else
    {
        QMessageBox::information(NULL,"warning","Pipe CONNECT SUCCESS!");
        emit haveData();
    }
}
