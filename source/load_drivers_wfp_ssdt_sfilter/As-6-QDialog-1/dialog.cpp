#include "dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{

    
    Main=new QGridLayout;

    Push_1=new QPushButton("FileDialog_Open_Btn");
    Push_2=new QPushButton("ColorDialog_palette_set");
    Push_3=new QPushButton("FontDialog_set");
    Push_4=new QPushButton("EditDialog");
    Line_1=new QLineEdit;
    Line_2=new QLineEdit;
    Color=new QFrame;

    Color->setFrameShape(QFrame::Box);
    Color->setAutoFillBackground(true);

    Main->addWidget(Push_1,0,0);
    Main->addWidget(Line_1,0,1);

    Main->addWidget(Push_2,1,0);
    Main->addWidget(Color,1,1);

    Main->addWidget(Push_3,2,0);
    Main->addWidget(Line_2,2,1);
    
    Main->addWidget(Push_4,3,0,1,2);

    setLayout(Main);

    QObject::connect(Push_1,SIGNAL(clicked()),this,SLOT(showFile()));
    QObject::connect(Push_2,SIGNAL(clicked()),this,SLOT(showColor()));
    QObject::connect(Push_3,SIGNAL(clicked()),this,SLOT(showFont()));
    QObject::connect(Push_4,SIGNAL(clicked()),this,SLOT(showEdit()));

}

Dialog::~Dialog()
{

}

void Dialog::showFile()//____FileDialog
{
    QString S=QFileDialog::getOpenFileName(this,"FileDialog","F:\\1.txt","c++ file(*.cpp);;c file(*.c);;txt file(*.txt);;png file(*.png)");
    Line_1->setText(S);
}

void Dialog::showColor()//____ColorDialog
{
    QColor a=QColorDialog::getColor(Qt::red);
    if(a.isValid())
    {
        Color->setPalette(QPalette(a));
    }
}

void Dialog::showFont()//_____FontDialog
{
    bool ok;
    QFont b=QFontDialog::getFont(&ok);
    if(ok)
    {
        Line_2->setFont(b);
    }
}

