#include "blacktablelist.h"
#include <QDebug>
#include <QMessageBox>

BlackTableList::BlackTableList(QWidget *parent):QTableWidget(parent)
{

}

BlackTableList::BlackTableList(int row, int col,QWidget *parents):QTableWidget(row,col,parents)
{
    popMenu = new QMenu();
    itemDetail = new QAction(this);
    itemDelete = new QAction(this);
    itemDetail->setText("Detail");
    itemDelete->setText("Delete");
    popMenu->addAction(itemDetail);
    popMenu->addAction(itemDelete);

    connect(itemDetail,SIGNAL(triggered()),this,SIGNAL(showDetail()));
    connect(itemDelete,SIGNAL(triggered()),this,SIGNAL(deleteDt()));
}

BlackTableList::~BlackTableList()
{

}

//右键菜单重写
void BlackTableList::contextMenuEvent(QContextMenuEvent *p)
{
    popMenu->clear();
    popMenu->addAction(itemDetail);
    popMenu->addSeparator();
    popMenu->addAction(itemDelete);
    popMenu->exec(QCursor::pos());

    p->accept();
}

