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

    ui->graphView->hide();
    //ui->graphView->show();
    //ui->loginView->hide();

    ui->logoutButton->hide();

    tcpSocket = new QTcpSocket(this);

    lineEMG1Series = new QLineSeries();
    lineEMG2Series = new QLineSeries();
    lineGyroSeries = new QLineSeries();

    connect(ui->logoutIcon, SIGNAL(mouse_Pressed()), this, SLOT(mouse_Pressed()));
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
    //createLineChart();

    ui->pieChartLayout->addWidget(pieChartView);
    ui->barChartLayout->addWidget(barChartView);
    //ui->lineChartLayout->addWidget(lineChartView);

    //ui->logoutLabel->setText(ui->dateTimeEdit->text());
}

Widget::~Widget()
{
    delete ui;
}

void Widget::mouse_Pressed()
{
    tcpSocket->close();
    qApp->exit();
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
    QString link = "http://192.168.1.33:8001";
    QDesktopServices::openUrl(QUrl(link));
}

void Widget::login_Pressed()
{
    QString addr = "192.168.1.33";
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
   //qDebug("rcv: start");

   QByteArray rcv_bytes = tcpSocket->readAll();

   QString rcv_data;
   if (rcv_bytes.length() == 0) rcv_data = "[no data]";
   else rcv_data = QString::fromUtf8(rcv_bytes);

   //ui->textEdit->setText(rcv_data);
   qDebug("rcv: '%s'", rcv_data.toUtf8().constData());

   QString condition = rcv_data.section(" ", 0, 0);
   QString result = rcv_data.section(" ", 1, 1);

   QString q_str;
   QByteArray utf8_str;

   if(condition == "whiler")
   {
       ui->loginView->hide();
       ui->graphView->show();
       ui->logoutButton->show();

       q_str = "line " + result;
       utf8_str = q_str.toUtf8();

       tcpSocket->write(utf8_str);
   }
   else if(condition == "bar")
   {
       QString EMG1 = rcv_data.section(" ", 1, 1);
       QString EMG2 = rcv_data.section(" ", 2, 2);
       QString Gyro = rcv_data.section(" ", 3, 3);
       QString Day = rcv_data.section(" ", 4, 4);
       QString End = rcv_data.section(" ", 5, 5);

       qreal EMG1y = EMG1.toDouble();
       qreal EMG2y = EMG2.toDouble();
       qreal Gyroy = Gyro.toDouble();


   }
   else if(condition == "line")
   {
       QString EMG1;
       QString EMG2;
       QString Gyro;
       QString Day;
       QString Month;
       QString Date;
       QString Year;
       QString Time;
       QString End;

       qreal EMG1y;
       qreal EMG2y;
       qreal Gyroy;

       for(int i=0; ; i+=14)
       {
           EMG1 = rcv_data.section(" ", i+1, i+1);
           EMG2 = rcv_data.section(" ", i+2, i+2);
           Gyro = rcv_data.section(" ", i+3, i+3);
           Day = rcv_data.section(" ", i+4, i+4).section("", 1, 1);
           Month = rcv_data.section(" ", i+5, i+5).section("", 1, 1);
           Date = rcv_data.section(" ", i+6, i+6);
           Year = rcv_data.section(" ", i+7, i+7).section("", 3, 4);
           Time = rcv_data.section(" ", i+12, i+12).section("", 1, 13);
           End = rcv_data.section(" ", i+13, i+13);
/*
           qDebug("EMG1: %s", EMG1.toUtf8().constData());
           qDebug("EMG2: %s", EMG2.toUtf8().constData());
           qDebug("Gyro: %s", Gyro.toUtf8().constData());
           qDebug("Day: %s", Day.toUtf8().constData());
           qDebug("Month: %s", Month.toUtf8().constData());
           qDebug("Date: %s", Date.toUtf8().constData());
           qDebug("Year: %s", Year.toUtf8().constData());
           qDebug("Time: %s", Time.toUtf8().constData());
           qDebug("End: %s", End.toUtf8().constData());
*/
           EMG1y = EMG1.toDouble();
           EMG2y = EMG2.toDouble();
           Gyroy = Gyro.toDouble();

           lineEMG1Series->append(lineCount, EMG1y);
           lineEMG2Series->append(lineCount, EMG2y);
           lineGyroSeries->append(lineCount, Gyroy);

           lineCount++;

           lineCategories.append(Year + Month + Date + Day + Time);

           if(End == "00")
           {
               break;
           }
           if(End == "11")
           {
               qDebug("End: %s", End.toUtf8().constData());
               createLineChart();
               break;
           }
       }
   }
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

    tcpSocket->close();
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

    //*set0 << 1 << 2 << 3 << 4 << 5 << 6;
    set0->append(1);
    set0->append(2);
    set0->append(3);
    set0->append(4);
    set0->append(5);
    set0->append(6);

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
    lineCount = 0;
/*
    QLineSeries *series = new QLineSeries();

    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);

    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);



    QStringList categories;
    categories.append("Monday");
    categories.append("Tuesday");
    categories.append("Wednesday");
    categories.append("Thursday");
    categories.append("Friday");
    categories.append("Saturday");
    categories.append("Sunday");

    //categories << "Monday" << "Tuesday" << "Wednesday" << "Thursday" << "Friday" << "Saturday" << "Sunday";
*/
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(lineCategories);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(lineEMG1Series);
    chart->addSeries(lineEMG2Series);
    chart->addSeries(lineGyroSeries);
    chart->createDefaultAxes();
    chart->setAxisX(axis, lineEMG1Series);
    chart->setAxisX(axis, lineEMG2Series);
    chart->setAxisX(axis, lineGyroSeries);
    //chart->setTitle("Simple line chart example");

    lineChartView = new QChartView(chart);

    lineChartView->setRenderHint(QPainter::Antialiasing);

    ui->lineChartLayout->addWidget(lineChartView);
}

void Widget::refresh_Pressed()
{

}
