#ifndef WHOLEFILEOPERA_SETTING_H
#define WHOLEFILEOPERA_SETTING_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:全局的文件系统操作类,实现对系统的文件进行禁止/允许,权限限制
 *     等操作.
******************************************************/

#include <QObject>
#include <QWidget>
#include <QPoint>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QThread>
#include <QTableView>
#include <QSqlQueryModel>

#include "multibtn.h"
#include "blacktablelist.h"
#include "pipe_operation.h"

//等待线程
class waitForBack : public QThread
{
    Q_OBJECT
public:
    waitForBack();
    void run();
    QString getRead();
    bool succ;
signals:
    void haveData();
private:
    QString readData;
    HANDLE readPipe;
    bool openPipe();
};

class WholeFileOpera_Setting : public QWidget
{
    Q_OBJECT
public:
    explicit WholeFileOpera_Setting(QWidget *parent = 0,Pipe_Operation *pipe = 0,int userid = 0);
    ~WholeFileOpera_Setting();

private:
    //等待后台线程回馈操作
    waitForBack *backFeed;
    //记录用户选中checkBox的字符
    QVector<QString> *ckb_List;
    //记录当前下拉框选中的字符串
    QString curComboText;
    //当前登陆的用户ID
    int userID;
    //管道的操作类
    Pipe_Operation *fpipe;
    //进程ID
    QLabel *lab_ProcessID;
    QLineEdit *lne_ProcessID;
    //多样式按钮
    MultiBtn    *closeIt;
    QLabel      *lab_state;
    QLabel      *lab_filePath;
    QLineEdit   *lne_filePath;//文件路径
    QLabel      *lab_securityLevel;
    QComboBox   *cob_securityLevel;//文件等级
    //当当前添加的为私人文件时，会出现该权限选择部件块
    //Dir --- 文件夹
    QHBoxLayout *h_Dir;
    QHBoxLayout *h_d1;
    QHBoxLayout *h_d2;
    QCheckBox   *ckb_newDir;
    QCheckBox   *ckb_RenameDir;
    QCheckBox   *ckb_DeleteDir;
    QCheckBox   *ckb_MoveDir;
    QCheckBox   *ckb_ReadDir;
    QCheckBox   *ckb_WriteDir;
    QCheckBox   *ckb_AllDir;
    //初始化文件夹权限控件
    void initialDirControl();
    void showDirControl();//显示
    void hideDirControl();//隐藏

    //File --- 文件
    QHBoxLayout *h_File;
    QHBoxLayout *h_f1;
    QHBoxLayout *h_f2;
    QCheckBox   *ckb_RenameFile;
    QCheckBox   *ckb_DeleteFile;
    QCheckBox   *ckb_MoveFile;
    QCheckBox   *ckb_ReadFile;
    QCheckBox   *ckb_WriteFile;
    QCheckBox   *ckb_AllFile;
    //初始化文件权限控件
    void initialFileControl();
    void showFileControl();//显示
    void hideFileControl();//隐藏

    //添加规则按钮
    QPushButton *btn_add;
    //黑名单列表
    QTableView *blacktl;
    QSqlQueryModel *model;
    //当前窗口位置
    QPoint CurrPosition;

    int fileSecurityLevel(QString);

signals:

public slots:

    void clearCkb();

    void gotReadFromBack();

    void addBtn();

    void cobChanged(QString);

    void func_ckb_ReadDir(int);
    void func_ckb_ReadFile(int);

    void func_ckb_WriteDir(int);
    void func_ckb_WriteFile(int);

    void func_ckb_AllDir(int);
    void func_ckb_AllFile(int);

    void func_ckb_DeleteDir(int);
    void func_ckb_DeleteFile(int);

    void func_ckb_MoveDir(int);
    void func_ckb_MoveFile(int);

    void func_ckb_newDir(int);

    void func_ckb_RenameDir(int);
    void func_ckb_RenameFile(int);

    void func_lne_textChanged(QString);

protected:
    void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);
};


#endif // WHOLEFILEOPERA_SETTING_H
