#ifndef PIECHART_H
#define PIECHART_H

#include <QtCharts/QPieSlice>

QT_CHARTS_USE_NAMESPACE

class piechart : public QPieSlice
{
    Q_OBJECT

public:
    piechart(QString label, qreal value);
    QBrush originalBrush();

public Q_SLOTS:
    void showHighlight(bool show);

private:
    QBrush m_originalBrush;
};

#endif // PIECHART_H
