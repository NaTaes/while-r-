#include "widget.h"
#include "ui_widget.h"
#include "my_qlabel.h"
#include <QApplication>
#include <QPushButton>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>

#include <QtCharts/QLineSeries>
#include <QFont>

#include <QTcpSocket>
#include <QDesktopServices>
#include <QUrl>

#include <QString>
#include <QStringList>

#include <QPixmap>

#include "piechart.h"

QT_CHARTS_USE_NAMESPACE

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //setWindowTitle("while(r);");

    ui->graphView->show();//hide();
    ui->loginView->hide();

    ui->logoutButton->hide();

    tcpSocket = new QTcpSocket(this);

    connect(ui->logoutIcon, SIGNAL(mouse_Pressed()), this, SLOT(mouse_Pressed()));
    connect(ui->logoutIcon, SIGNAL(mouse_Left()), this, SLOT(mouse_Left()));
    connect(ui->logoutIcon, SIGNAL(mouse_Enter()), this, SLOT(mouse_Enter()));
    connect(ui->loginButton, SIGNAL(clicked(bool)), this, SLOT(login_Pressed()));
    connect(ui->logoutButton, SIGNAL(clicked(bool)), this, SLOT(logout_Pressed()));

    connect(ui->joinLabel, SIGNAL(mouse_Enter()), this, SLOT(joinLabel_Enter()));
    connect(ui->joinLabel, SIGNAL(mouse_Left()), this, SLOT(joinLabel_Left()));
    connect(ui->joinLabel, SIGNAL(mouse_Pressed()), this, SLOT(joinLabel_Pressed()));

    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(closeConnection()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error()));

    connect(ui->refreshIcon, SIGNAL(mouse_Pressed()), this, SLOT(refresh_Pressed()));

    createPieChart();
    createBarChart();
    createLineChart();

    ui->pieChartLayout->addWidget(pieChartView);
    ui->barChartLayout->addWidget(barChartView);
    ui->lineChartLayout->addWidget(lineChartView);

    //ui->logoutLabel->setText(ui->dateTimeEdit->text());
}

Widget::~Widget()
{
    delete ui;
}

void Widget::mouse_Pressed()
{
    qApp->exit();
}

void Widget::mouse_Enter()
{
    //ui->graphView->hide();
    //ui->loginView->show();
}

void Widget::mouse_Left()
{
    //ui->loginView->hide();
    //ui->graphView->show();
}

void Widget::joinLabel_Enter()
{
    QFont f = font();
    f.setUnderline(true);

    ui->joinLabel->setFont(f);
}

void Widget::joinLabel_Left()
{
    QFont f = font();
    f.setUnderline(false);

    ui->joinLabel->setFont(f);
}

void Widget::joinLabel_Pressed()
{
    QString link = "http://172.30.1.56:8001";
    QDesktopServices::openUrl(QUrl(link));
}

void Widget::login_Pressed()
{
    QString addr = "172.30.1.56";
    QString port_str = "8004";  //LineEdit

    bool is_ok;
    int port = port_str.toInt(&is_ok, 10);
    if (!is_ok || port <= 0) return;

    this->connectToServer(addr, port);

    QString q_str = "login " + ui->idEdit->text() + " " + ui->pwEdit->text();

    QByteArray utf8_str = q_str.toUtf8();

    //qDebug("send: '%s'", utf8_str.constData());

    tcpSocket->write(utf8_str);
}

void Widget::connectToServer(const QString &host, quint16 port)
{
   qDebug("conn: host=%s, port=%d", host.toUtf8().constData(), port);
   tcpSocket->connectToHost(host, port);
}

void Widget::receiveData()
{
   qDebug("rcv: start");

   QByteArray rcv_bytes = tcpSocket->readAll();

   QString rcv_data;
   if (rcv_bytes.length() == 0) rcv_data = "[no data]";
   else rcv_data = QString::fromUtf8(rcv_bytes);

   //ui->textEdit->setText(rcv_data);
   //qDebug("rcv: '%s'", rcv_data.toUtf8().constData());

   QString condition = rcv_data.section(" ", 0, 0);
   QString result = rcv_data.section(" ", 1, 1);

   QString q_str;
   QByteArray utf8_str;

   if(condition == "whiler")
   {
       ui->loginView->hide();
       ui->graphView->show();
       ui->logoutButton->show();

       q_str = "line" + " " + result;
       utf8_str = q_str.toUtf8();

       tcpSocket->write(utf8_str);
   }
   else if(condition == "bar")
   {

   }
   else if(condition == "line")
   {
       qDebug("line: '%s'", rcv_data.toUtf8().constData());
   }

   //qDebug("%s", result.toUtf8().constData());

   //tcpSocket->close();
}

void Widget::closeConnection()
{
   qDebug("close: close conn");
   tcpSocket->close();
}

void Widget::error()
{
   QString mess = tcpSocket->errorString();
   qDebug("error: %s", mess.toUtf8().constData());

   tcpSocket->close();
}

void Widget::logout_Pressed()
{
    ui->idEdit->clear();
    ui->pwEdit->clear();
    ui->logoutButton->hide();
    ui->graphView->hide();
    ui->loginView->show();
}

void Widget::createPieChart()
{
    QChart *chart = new QChart;
    chart->setTitle("Piechart customization");
    chart->setAnimationOptions(QChart::AllAnimations);

    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35);

    *series << new piechart("biceps brachii 4.2%", 4.2);
    //series->append("Jeon Wan Geun 4.2%", 4.2);
    *series << new piechart("wrist 15.6%", 15.6);
    //testslice->set
    //slice->setExploded();
    //slice->setLabelVisible();
    *series << new piechart("Jeon Wan Geun 23.8%", 23.8);

    //series->append("Other 23.8%", 23.8);
    //series->append("Carbs 56.4%", 56.4);

    pieChartView = new QChartView();

    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->chart()->setTitle("Exercise Site Ratio");
    pieChartView->chart()->addSeries(series);

    pieChartView->chart()->resize(700, 700);
    pieChartView->chart()->legend()->setAlignment(Qt::AlignBottom);
    //chartView->chart()->setTheme(QChart::ChartThemeBlueCerulean);
    pieChartView->chart()->legend()->setFont(QFont("Arial", 7));
}

void Widget::createBarChart()
{
    QBarSet *set0 = new QBarSet("EMG1");
    QBarSet *set1 = new QBarSet("EMG2");
    QBarSet *set2 = new QBarSet("Gyro");

    *set0 << 1 << 2 << 3 << 4 << 5 << 6;
    *set1 << 5 << 0 << 0 << 4 << 0 << 7;
    *set2 << 3 << 5 << 8 << 13 << 8 << 5;

    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->append(set1);
    series->append(set2);

    QChart *chart = new QChart();
    chart->addSeries(series);
    //chart->setTitle("Weekend Exercise");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "Monday" << "Tuesday" << "Wednesday" << "Thursday" << "Friday" << "Saturday" << "Sunday";
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    barChartView = new QChartView(chart);
    barChartView->setRenderHint(QPainter::Antialiasing);
}

void Widget::createLineChart()
{
    QLineSeries *series = new QLineSeries();

    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    //chart->setTitle("Simple line chart example");

    lineChartView = new QChartView(chart);
    lineChartView->setRenderHint(QPainter::Antialiasing);
}

void Widget::refresh_Pressed()
{

}
