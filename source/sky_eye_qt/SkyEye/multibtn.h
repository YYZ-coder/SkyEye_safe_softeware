#ifndef MULTIBTN_H
#define MULTIBTN_H

/*****************************************************
 * Copyright(c) 2017 CherryZhang , All rights reserverd
 * 项目: SkyEye
 * 版本: 1.0
 * 作者:CherryZhang
 * 创建时间:2017/3/19
 * 描述:多样式按钮类,用来设置不同的按钮样式,提高用户体验.
******************************************************/

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPaintEvent>

class MultiBtn : public QPushButton
{
    Q_OBJECT
public:
    MultiBtn(QWidget *parent);
    ~MultiBtn();

    void setMulti(QString);

    void setUsername(QString);

private:

    QString multi;

    QString username;

protected:

    void paintEvent(QPaintEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);

};

#endif // MULTIBTN_H
