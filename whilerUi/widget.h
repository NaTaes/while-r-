#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCharts/QChartGlobal>

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();



private:
    void createPieChart();
    void createBarChart();
    void createLineChart();
    QChartView *pieChartView;
    QChartView *barChartView;
    QChartView *lineChartView;
    Ui::Widget *ui;

private slots:
    void mouse_Pressed();
    void mouse_Enter();
    void mouse_Left();
    void refresh_Pressed();
};

#endif // WIDGET_H