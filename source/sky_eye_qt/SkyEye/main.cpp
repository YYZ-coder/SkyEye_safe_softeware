#include <QApplication>
#include <QtSql>
#include <QMessageBox>
#include <stdio.h>
#include <windows.h>
#include <unistd.h>
#include <QDebug>
#include <QMessageBox>

#include "mainwindow.h"
#include "register.h"
#include "sqlmanager.h"

int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    QString strLibPath(QDir::toNativeSeparators(
                           QCoreApplication::applicationDirPath())+QDir::separator()+"plugins");
    a.addLibraryPath(strLibPath);

    MainWindow *w = new MainWindow();
    w->show();

    //WholeFileOpera_Setting *wfo = new WholeFileOpera_Setting();
    //wfo->show();
    //Register *r = new Register();
    //r->show();

    //SqlManager *sqlManager = new SqlManager();
    //sqlManager->show();

    return a.exec();
}
