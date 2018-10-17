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

#include "piechart.h"

QT_CHARTS_USE_NAMESPACE

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("while(r);");

    //ui->loginView->hide();

    connect(ui->logoutIcon, SIGNAL(mouse_Pressed()), this, SLOT(mouse_Pressed()));
    //connect(ui->logoutIcon, SIGNAL(mouse_Left()), this, SLOT(mouse_Left()));
    //connect(ui->logoutIcon, SIGNAL(mouse_Enter()), this, SLOT(mouse_Enter()));

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
    QBarSet *set2 = new QBarSet("Finger");

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
