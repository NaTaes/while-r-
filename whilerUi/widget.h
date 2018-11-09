#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QLineSeries>

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class QTcpSocket;
//class QLineSeries;

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
    QTcpSocket *tcpSocket;
    QLineSeries *lineEMG1Series;
    QLineSeries *lineEMG2Series;
    QLineSeries *lineGyroSeries;
    int lineCount = 0;
    QStringList lineCategories;



private slots:
    void mouse_Pressed();
    void refresh_Pressed();
    void login_Pressed();
    void logout_Pressed();
    void joinLabel_Enter();
    void joinLabel_Left();
    void joinLabel_Pressed();

    void connectToServer(const QString &host, quint16 port);
    void receiveData();
    void error();
    void closeConnection();

};

#endif // WIDGET_H
