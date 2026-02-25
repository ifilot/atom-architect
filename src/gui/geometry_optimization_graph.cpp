#include "geometry_optimization_graph.h"

#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QHeaderView>

GeometryOptimizationGraph::GeometryOptimizationGraph(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    title = new QLabel("OPTIMIZATION GRAPH", this);
    title->setStyleSheet("font-weight: bold;");
    layout->addWidget(title);

    stack = new QStackedWidget(this);
    layout->addWidget(stack);

    chartview = new QChartView(this);
    chartview->setRenderHint(QPainter::Antialiasing);
    stack->addWidget(chartview);

    frequency_table = new QTableWidget(this);
    frequency_table->setColumnCount(2);
    frequency_table->setHorizontalHeaderLabels({tr("Mode"), tr("Frequency (cm⁻¹)")});
    frequency_table->horizontalHeader()->setStretchLastSection(true);
    frequency_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    frequency_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    frequency_table->setSelectionMode(QAbstractItemView::SingleSelection);
    frequency_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    frequency_table->setAlternatingRowColors(true);
    frequency_table->verticalHeader()->setVisible(false);
    frequency_table->setStyleSheet(
        "QTableWidget::item:selected {"
        "background-color: #6ea8ff;"
        "color: #0b1f44;"
        "}"
    );
    stack->addWidget(frequency_table);

    connect(frequency_table, &QTableWidget::currentCellChanged,
            this, [this](int currentRow, int, int, int) {
                if(currentRow >= 0) {
                    emit frequency_selected((size_t)currentRow);
                }
            });
}

void GeometryOptimizationGraph::set_structures(
    const std::vector<std::shared_ptr<Structure>>& s)
{
    title->setText("OPTIMIZATION GRAPH");
    stack->setCurrentWidget(chartview);

    structures = s;
    current_index = 0;
    rebuild_chart();
}

void GeometryOptimizationGraph::set_frequency_modes(const std::vector<Structure::Eigenmode>& modes)
{
    title->setText("FREQUENCIES");
    stack->setCurrentWidget(frequency_table);

    QSignalBlocker blocker(frequency_table);
    frequency_table->setRowCount(0);

    for(size_t i = 0; i < modes.size(); ++i) {
        const double cm1 = modes[i].eigenvalue * THZ_TO_WAVENUMBER;
        const int row = frequency_table->rowCount();
        frequency_table->insertRow(row);

        auto* mode_item = new QTableWidgetItem(tr("Mode %1").arg(i + 1));
        mode_item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        frequency_table->setItem(row, 0, mode_item);

        auto* freq_item = new QTableWidgetItem(QString::number(cm1, 'f', 2));
        freq_item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        frequency_table->setItem(row, 1, freq_item);
    }

    if(frequency_table->rowCount() > 0) {
        frequency_table->selectRow(0);
    }
}

void GeometryOptimizationGraph::set_current_index(size_t idx)
{
    current_index = idx;

    if(stack->currentWidget() == chartview) {
        update_highlight();
        return;
    }

    QSignalBlocker blocker(frequency_table);
    if((int)current_index < frequency_table->rowCount()) {
        frequency_table->selectRow((int)current_index);
    }
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
