#pragma once

#include <QWidget>
#include <QtCharts>

#include "../data/structure.h"

class GeometryOptimizationGraph : public QWidget {
    Q_OBJECT

public:
    explicit GeometryOptimizationGraph(QWidget *parent = nullptr);

    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures);
    void set_current_index(size_t index);

private:
    void rebuild_chart();
    void update_highlight();

private:
    QChartView *chartview;
    QChart *chart;

    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY2;

    std::vector<std::shared_ptr<Structure>> structures;
    size_t current_index = 0;
};
