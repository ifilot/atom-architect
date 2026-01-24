#include "geometry_optimization_graph.h"

#include <QVBoxLayout>
#include <QLinearGradient>

GeometryOptimizationGraph::GeometryOptimizationGraph(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QLabel *title = new QLabel("OPTIMIZATION GRAPH", this);
    title->setStyleSheet("font-weight: bold;");
    layout->addWidget(title);

    chartview = new QChartView(this);
    chartview->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartview);
}

void GeometryOptimizationGraph::set_structures(
    const std::vector<std::shared_ptr<Structure>>& s)
{
    structures = s;
    current_index = 0;
    rebuild_chart();
}

void GeometryOptimizationGraph::set_current_index(size_t idx)
{
    current_index = idx;
    update_highlight();
}

void GeometryOptimizationGraph::rebuild_chart()
{
    chart = new QChart();
    auto *energy = new QLineSeries();
    auto *force = new QLineSeries();

    for (size_t i = 0; i < structures.size(); ++i) {
        energy->append(i + 1, structures[i]->get_energy());
        force->append(i + 1, structures[i]->get_rms_force());
    }

    chart->addSeries(energy);
    chart->addSeries(force);

    axisX = new QValueAxis();
    axisY = new QValueAxis();
    axisY2 = new QValueAxis();

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addAxis(axisY2, Qt::AlignRight);

    energy->attachAxis(axisX);
    energy->attachAxis(axisY);
    force->attachAxis(axisX);
    force->attachAxis(axisY2);

    axisX->setRange(1, structures.size());
    chartview->setChart(chart);

    update_highlight();
}

void GeometryOptimizationGraph::update_highlight()
{
    while (chart->series().size() > 2)
        chart->removeSeries(chart->series().last());

    auto *s1 = new QScatterSeries();
    *s1 << QPointF(current_index + 1,
                   structures[current_index]->get_energy());

    chart->addSeries(s1);
    s1->attachAxis(axisX);
    s1->attachAxis(axisY);
}
