#ifndef BLACKTABLELIST_H
#define BLACKTABLELIST_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述: BlackTableList是**_Setting窗口中黑名单的公共类,
 *      用来实现它们共有的属性和方法.
******************************************************/

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

class BlackTableList : public QTableWidget
{
    Q_OBJECT
public:
    BlackTableList(QWidget *parent=0);
    BlackTableList(int row,int col,QWidget *parent=0);
    ~BlackTableList();

private:

    QMenu   *popMenu;
    QAction *itemDetail;
    QAction *itemDelete;

private slots:

signals:

    //在其子类中进行删除判断，操作
    void deleteDt();

    //显示当前所选行的明细
    void showDetail();

protected:

    //右键菜单重写
    void contextMenuEvent(QContextMenuEvent *);

};

#endif // BLACKTABLELIST_H
