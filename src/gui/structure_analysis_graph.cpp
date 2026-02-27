/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2026 Ivo Filot <i.a.w.filot@tue.nl>                 *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

#include "structure_analysis_graph.h"

#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QHeaderView>

/**
 * @brief StructureAnalysisGraph.
 *
 * @param parent Parameter parent.
 */
StructureAnalysisGraph::StructureAnalysisGraph(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    title = new QLabel("STRUCTURE ENERGY PROFILE", this);
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

/**
 * @brief set_structures.
 *
 * @param s Parameter s.
 * @param kind Parameter kind.
 */
void StructureAnalysisGraph::set_structures(const std::vector<std::shared_ptr<Structure>>& s,
                                            SeriesKind kind)
{
    series_kind_ = kind;
    title->setText(series_kind_ == SeriesKind::NEB ? "NEB ENERGY PROFILE" : "OPTIMIZATION GRAPH");
    stack->setCurrentWidget(chartview);

    structures = s;
    current_index = 0;
    rebuild_chart();
}

/**
 * @brief set_frequency_modes.
 *
 * @param modes Parameter modes.
 */
void StructureAnalysisGraph::set_frequency_modes(const std::vector<Structure::Eigenmode>& modes)
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

/**
 * @brief set_current_index.
 *
 * @param index Parameter index.
 */
void StructureAnalysisGraph::set_current_index(size_t idx)
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

/**
 * @brief rebuild_chart.
 *
 */
void StructureAnalysisGraph::rebuild_chart()
{
    chart = new QChart();
    auto *energy = new QLineSeries();
    auto *force = new QLineSeries();

    for(size_t i = 0; i < structures.size(); ++i) {
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

    axisX->setRange(1, (double)structures.size());
    chartview->setChart(chart);

    update_highlight();
}

/**
 * @brief update_highlight.
 *
 */
void StructureAnalysisGraph::update_highlight()
{
    if(!chart || structures.empty() || current_index >= structures.size()) {
        return;
    }

    while(chart->series().size() > 2) {
        chart->removeSeries(chart->series().last());
    }

    auto *s1 = new QScatterSeries();
    *s1 << QPointF(current_index + 1, structures[current_index]->get_energy());

    chart->addSeries(s1);
    s1->attachAxis(axisX);
    s1->attachAxis(axisY);
}
