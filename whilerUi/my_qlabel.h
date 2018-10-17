#ifndef MY_QLABEL_H
#define MY_QLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

class my_qlabel : public QLabel
{
    Q_OBJECT

public:
    explicit my_qlabel(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent *e);
    void leaveEvent(QEvent *);
    void enterEvent(QEvent *);

signals:
    void mouse_Pressed();
    void mouse_Enter();
    void mouse_Left();
};

#endif // MY_QLABEL_H
