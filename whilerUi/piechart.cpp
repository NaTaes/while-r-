#include "piechart.h"

piechart::piechart(QString label, qreal value)
    : QPieSlice(label, value)
{
    connect(this, &piechart::hovered, this, &piechart::showHighlight);
}

QBrush piechart::originalBrush()
{
    return m_originalBrush;
}

void piechart::showHighlight(bool show)
{
    if (show) {
        QBrush brush = this->brush();
        m_originalBrush = brush;
        brush.setColor(brush.color().lighter());
        setBrush(brush);
    } else {
        setBrush(m_originalBrush);
    }
}
