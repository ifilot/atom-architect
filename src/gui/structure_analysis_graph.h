#pragma once

#include <QWidget>
#include <QtCharts>
#include <QTableWidget>
#include <QStackedWidget>
#include <QLabel>

#include "../data/structure.h"

class StructureAnalysisGraph : public QWidget {
    Q_OBJECT

public:
    enum class SeriesKind {
        GEOMETRY_OPTIMIZATION,
        NEB
    };

    explicit StructureAnalysisGraph(QWidget *parent = nullptr);

    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures,
                        SeriesKind kind = SeriesKind::GEOMETRY_OPTIMIZATION);
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
    QChart *chart = nullptr;

    QTableWidget *frequency_table;

    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    QValueAxis *axisY2 = nullptr;

    std::vector<std::shared_ptr<Structure>> structures;
    size_t current_index = 0;
    SeriesKind series_kind_ = SeriesKind::GEOMETRY_OPTIMIZATION;

    static constexpr double THZ_TO_WAVENUMBER = 33.35640951981521;
};
