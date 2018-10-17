#include "my_qlabel.h"

my_qlabel::my_qlabel(QWidget *parent) :
    QLabel(parent)
{

}

void my_qlabel::mousePressEvent(QMouseEvent *e)
{
    emit mouse_Pressed();
}

void my_qlabel::leaveEvent(QEvent *)
{
    emit mouse_Left();
}

void my_qlabel::enterEvent(QEvent *)
{
    emit mouse_Enter();
}
