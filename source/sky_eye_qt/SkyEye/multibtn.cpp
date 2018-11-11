#include "multibtn.h"

#include <QPainter>

bool Pressed = false;

bool hovered = false;

MultiBtn::MultiBtn(QWidget *parent):QPushButton(parent)
{
    username = "";
}

//设置按钮显示类型
void MultiBtn::setMulti(QString a)
{
    multi = a;
}

//设置显示的用户名
void MultiBtn::setUsername(QString u)
{
    username = u;
}

//析构函数
MultiBtn::~MultiBtn()
{

}

//paint函数重写
void MultiBtn::paintEvent(QPaintEvent *p)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);//反锯齿
    if(multi == "X"){//对于MainWindow主窗口，在按钮上划横线
        if(Pressed){
            painter.setPen(QPen(Qt::gray,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            painter.drawLine(7,this->height()/2-5,this->width()-7,this->height()/2-5);
            painter.drawLine(7,this->height()/2-5,this->width()/2,this->height()-4);
            painter.drawLine(this->width()/2,this->height()-4,this->width()-7,this->height()/2-5);
        }else if(hovered){
            painter.setPen(QPen(QColor(199,80,80),3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            painter.drawLine(7,this->height()/2-3,this->width()-7,this->height()/2-3);
        }else{
            painter.setPen(QPen(Qt::black,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            painter.drawLine(7,this->height()/2-3,this->width()-7,this->height()/2-3);
        }

        painter.drawLine(7,this->height()/2-3,this->width()-7,this->height()/2-3);
    }else if(multi == "-"){//对于各个Setting子窗口，在按钮上画个圈
        if(Pressed){
            painter.setPen(QPen(Qt::gray,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            QRect *rec = new QRect(3,1,this->rect().width()-7,this->rect().height()-2);
            painter.drawEllipse(*rec);
        }else if(hovered){
            painter.setPen(QPen(QColor(199,80,80),2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            QRect *rec = new QRect(3,1,this->rect().width()-7,this->rect().height()-2);
            painter.drawEllipse(*rec);
        }else{
            painter.setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            QRect *rec = new QRect(3,1,this->rect().width()-7,this->rect().height()-2);
            painter.drawEllipse(*rec);
        }
    }else if(multi == "user"){//对于主界面显示当前用户
        if(Pressed){
            painter.setPen(QPen(Qt::gray,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        }else if(hovered){
            painter.setPen(QPen(QColor(0,0,255),3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        }else{
            painter.setPen(QPen(Qt::black,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        }
        painter.drawText(QPointF(0,11),username);
    }else{//当需要创建用户时，点击它弹出注册对话框
        if(Pressed)
            painter.setPen(QPen(Qt::gray,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        else if(hovered)
            painter.setPen(QPen(QColor(199,80,80),2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        else
            painter.setPen(QPen(Qt::black,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));

        painter.drawEllipse(this->width()/2-6,2,12,12);
        QRectF rec(0,11,30,15);
        painter.drawArc(rec,0*16,180*16);
        painter.setBrush(QBrush(Qt::black,Qt::SolidPattern));
    }

    QPushButton::paintEvent(p);
}

//MousePress函数重写
void MultiBtn::mousePressEvent(QMouseEvent *e){
    Pressed = true;
    repaint();
    QPushButton::mousePressEvent(e);
}

//MouseRelease函数重写
void MultiBtn::mouseReleaseEvent(QMouseEvent *e)
{
    Pressed = false;
    repaint();
    QPushButton::mouseReleaseEvent(e);
}

//鼠标进入窗口函数重写
void MultiBtn::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    hovered = true;
}

//鼠标出窗口函数重写
void MultiBtn::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);
    hovered = false;
}

