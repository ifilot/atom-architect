#pragma once

#include <QWidget>
#include <QtCharts>
#include <QTableWidget>
#include <QStackedWidget>
#include <QLabel>

#include "../data/structure.h"

class GeometryOptimizationGraph : public QWidget {
    Q_OBJECT

public:
    explicit GeometryOptimizationGraph(QWidget *parent = nullptr);

    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures);
    void set_current_index(size_t index);
    void set_frequency_modes(const std::vector<Structure::Eigenmode>& modes);

signals:
    void frequency_selected(size_t index);

private:
    void rebuild_chart();
    void update_highlight();

private:
    QLabel *title;
    QStackedWidget *stack;

    QChartView *chartview;
    QChart *chart;

    QTableWidget *frequency_table;

    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY2;

    std::vector<std::shared_ptr<Structure>> structures;
    size_t current_index = 0;

    static constexpr double THZ_TO_WAVENUMBER = 33.35640951981521;
};
