#include "sqlmanager.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>

bool smoving = false;

//构造函数
SqlManager::SqlManager(QWidget *parent,ConMySQL *mysql,QString host) : QWidget(parent)
{
    localhost = host;
    msql = mysql;
    //显示users和role表
    URtable = new BlackTableList(0,7,0);
    URtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    URtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    URtable->horizontalHeader()->setHighlightSections(false);
    connect(URtable,SIGNAL(deleteDt()),this,SLOT(deleteCurrentRow()));
    //显示普通表
    tableView = new QTableView;
    //只读Model
    queryModel = new QSqlQueryModel();
    //多功能按钮
    closeIt = new MultiBtn(this);
    closeIt->setMulti("-");
    closeIt->setFixedSize(30,20);
    closeIt->setStyleSheet("QPushButton{background : transparent;}");
    connect(closeIt,SIGNAL(clicked()),this,SLOT(close()));

    //选择显示的表索引下拉框
    cob_tables = new QComboBox();
    QStringList ts;
    ts<<"users"<<"role"<<"process"<<"nethead"<<"processmonitor"<<"file";
    cob_tables->addItems(ts);
    cob_tables->setCurrentIndex(0);
    connect(cob_tables,SIGNAL(currentIndexChanged(int)),this,SLOT(checkIndex(int)));

    queryModel->setQuery("SELECT * FROM users");

    tableView->setModel(queryModel);
    URtable->show();

    QHBoxLayout *h_head = new QHBoxLayout();
    h_head->addSpacing(this->width()-30);
    h_head->addWidget(closeIt);
    QHBoxLayout *h_tables = new QHBoxLayout();
    h_tables->addWidget(cob_tables);
    QVBoxLayout *v_m = new QVBoxLayout();
    v_m->addLayout(h_head);
    v_m->addLayout(h_tables);
    v_m->addWidget(URtable);
    v_m->addWidget(tableView);

    tableView->hide();

    setLayout(v_m);
    setWindowIcon(QIcon(":/owl0.ico"));
    setAttribute(Qt::WA_TranslucentBackground);  //背景透明
    setWindowFlags(Qt::FramelessWindowHint);     //无边框
}

//析构函数
SqlManager::~SqlManager()
{

}

//显示users表
void SqlManager::showUsersTbl()
{
    //先清空当前列表的数据
    int allRs = URtable->rowCount();
    for(int i=0;i<allRs;i++){
        for(int j=0;j<URtable->columnCount();j++){
            delete URtable->item(i,j);
        }
    }
    URtable->setRowCount(0);

    //存储当前显示的表名
    currentChangeTable = "users";

    //读取数据库进行数据写入
    QSqlQuery sql = msql->getUserID();
    QStringList list;
    list<<"User_ID"<<"Security_Level"<<"User_Name";
    URtable->setHorizontalHeaderLabels(list);
    //行数
    int row = 0;
    if(&sql != NULL){
        while(sql.next()){
            //插入新的一行
            URtable->insertRow(row);
            for(int i=0;i<7;i++){
                QTableWidgetItem *ite = new QTableWidgetItem();
                ite->setText(sql.value(i).toString());
                URtable->setItem(row,i,ite);
            }

        }
    }
}

//显示角色表
void SqlManager::showRoleTbl()
{
    //先清空当前列表的数据
    URtable->clearFocus();
    int allRs = URtable->rowCount();
    for(int i=0;i<allRs;i++){
        for(int j=0;j<URtable->columnCount();j++){
            delete URtable->item(i,j);
        }
    }
    URtable->setRowCount(0);

    //存储当前显示的表名
    currentChangeTable = "role";

    //读取数据库中的信息并写入表格中
    QSqlQuery sql = msql->getRole();
    QStringList list;
    list<<"Role_ID"<<"Role_Type";
    URtable->setHorizontalHeaderLabels(list);
    //行数
    int row = 0;
    if(&sql != NULL){
        while(sql.next()){
            //插入新的一行
            URtable->insertRow(row);
            for(int i=0;i<5;i++){
                QTableWidgetItem *ite = new QTableWidgetItem();
                ite->setText(sql.value(i).toString());
                URtable->setItem(row,i,ite);
            }
        }
    }
}

//删除当前选中行
void SqlManager::deleteCurrentRow()
{
    //读取当前选中行的所有items信息
    QList<QTableWidgetItem*> items = URtable->selectedItems();
    //用来存储user_ID,如果当前的表是user_role表的话
    QVector<int> user_IDs;
    do{
        //当前显示为users表
        if(currentChangeTable == "users"){
            //若当前选中为管理员账户
            if(items.at(0)->text().toInt() == 0){
                QMessageBox::warning(this,"warning","U don't delete yourself!");
                break;
            }
            //提醒用户
            int ret = QMessageBox::warning(this,"warning","U will delete all this user's Info!",QMessageBox::Ok|QMessageBox::Cancel);
            if(ret != QMessageBox::Ok)
                break;
            //删除一个用户
            //1.在users表中删除并移除登录用户
            msql->DeleteUser(items.at(0)->text().toInt(),localhost,items.at(2)->text());
            //2.在user_role表中删除userID-RoleID
            msql->deleteUserRole(items.at(0)->text().toInt());
        }//当前显示为role表
        else if(currentChangeTable == "role"){
            //提醒用户
            int ret = QMessageBox::warning(this,"warning","U will delete all this role's Info!\nInclude users Info and role's Jurisdiction!",
                                           QMessageBox::Ok|QMessageBox::Cancel);
            if(ret != QMessageBox::Ok)
                break;
            //删除一个角色
            //1.删除role表中的一个角色
            msql->deleteRole(items.at(0)->text().toInt());
            //2.删除role_jurisdiction表中的相关角色权限集
            msql->deleteUserRoleJurisdiction(items.at(0)->text().toInt());
            //获得user_role中对应的userID
            QSqlQuery sql = msql->getUserIDuserrole(items.at(0)->text().toInt());
            if(&sql == NULL){
                QMessageBox::warning(this,"ERROR","DataBase Error ,please Contact Admin!");
                break;
            }
            //遍历user_role表,获得user_Id
            while(sql.next()){
                user_IDs.push_back(sql.value(0).toInt());
            }
            //3.删除user_role的对应userID-RoleID
            msql->deleteuserroleRoleId(items.at(0)->text().toInt());
            //4.删除users表中所有为该角色的用户信息,并删除用户登录drop user xxx
            QSqlQuery sqluser;
            sqluser = msql->getUserID();
            if(&sqluser == NULL)
                break;
            //遍历users表
            while(sqluser.next()){
                //包含当前检索到的user_ID
                if(user_IDs.contains(sqluser.value(0).toInt())){
                    msql->DeleteUser(sqluser.value(0).toInt(),
                                     localhost,
                                     sqluser.value(2).toString());
                }
            }
        }else break;
        //获得当前行号
        int row = items.at(0)->row();
        //删除表中选中的一行数据
        URtable->removeRow(row);
    }while(false);
}

//检测当前combobox的索引
void SqlManager::checkIndex(int index)
{
    switch(index){
    case 0://users
        tableView->hide();
        URtable->show();
        showUsersTbl();
        break;
    case 1://role
        tableView->hide();
        URtable->show();
        showRoleTbl();
        break;
    case 2://process
        URtable->hide();
        queryModel->setQuery("SELECT * FROM process");
        tableView->show();
        break;
    case 3://nethead
        URtable->hide();
        queryModel->setQuery("SELECT * FROM nethead");
        tableView->show();
        break;
    case 4://processmonitor
        URtable->hide();
        queryModel->setQuery("SELECT * FROM processmonitor");
        tableView->show();
        break;
    case 5://file
        URtable->hide();
        queryModel->setQuery("SELECT * FROM file");
        tableView->show();
        break;
    default://默认显示users表
        URtable->hide();
        queryModel->setQuery("SELECT * FROM users");
        tableView->show();
        break;
    }
}

//paint事件重写
void SqlManager::paintEvent(QPaintEvent *p)
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
void SqlManager::closeEvent(QCloseEvent *c)
{
    this->hide();
    c->ignore();
}

//press事件重写
void SqlManager::mousePressEvent(QMouseEvent *e)
{
    smoving = true;
    //当前位置等于鼠标相对于桌面的位置减去相对于窗口的位置，也就得到了当前窗口的(0,0)处在桌面的位置
    CurrPosition = e->globalPos() - pos();
    return QWidget::mousePressEvent(e);
}

//move事件重写
void SqlManager::mouseMoveEvent(QMouseEvent *e)
{
    //条件:正在移动,左键按动,当前窗口的全局增量的曼哈顿长度大于拖动的长度
    //(manhattanLength是指直角三角形中的两个直角边之和)
    if(smoving && e->buttons() && Qt::LeftButton
            &&(e->globalPos()-CurrPosition).manhattanLength() > QApplication::startDragDistance()){
        //满足条件后开始移动相对距离,并刷新CurrPosition
        move(e->globalPos()-CurrPosition);
        CurrPosition = e->globalPos()-pos();
    }
    return QWidget::mouseMoveEvent(e);
}

//release事件重写
void SqlManager::mouseReleaseEvent(QMouseEvent *e)
{
    smoving = false;
    return QWidget::mouseReleaseEvent(e);
}

