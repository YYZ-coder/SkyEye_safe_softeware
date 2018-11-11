#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTextCodec>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>

class Dialog : public QDialog
{
    Q_OBJECT

public:

    Dialog(QWidget *parent = 0);
    ~Dialog();

private:

    QGridLayout *Main;

    QPushButton *Push_1;
    QPushButton *Push_2;
    QPushButton *Push_3;
    QPushButton *Push_4;
    QPushButton *Push_5;

    QLineEdit *Line_1;
    QLineEdit *Line_2;

    QFrame *Color;

private slots:

    void showFile();
    void showColor();
    void showFont();
};

#endif // DIALOG_H
