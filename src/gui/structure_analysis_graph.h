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

#pragma once

#include <QWidget>
#include <QtCharts>
#include <QTableWidget>
#include <QStackedWidget>
#include <QLabel>

#include "../data/structure.h"

/**
 * @brief StructureAnalysisGraph class.
 */
class StructureAnalysisGraph : public QWidget {
    Q_OBJECT

public:
    enum class SeriesKind {
        GEOMETRY_OPTIMIZATION,
        NEB
    };

/**
 * @brief StructureAnalysisGraph.
 *
 * @param parent Parameter parent.
 */
    explicit StructureAnalysisGraph(QWidget *parent = nullptr);

    /**
     * @brief set_structures.
     *
     * @param structures Parameter structures.
     * @param kind Parameter kind.
     */
    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures,
                        SeriesKind kind = SeriesKind::GEOMETRY_OPTIMIZATION);
/**
 * @brief set_current_index.
 *
 * @param index Parameter index.
 */
    void set_current_index(size_t index);
/**
 * @brief set_frequency_modes.
 *
 * @param modes Parameter modes.
 */
    void set_frequency_modes(const std::vector<Structure::Eigenmode>& modes);

signals:
/**
 * @brief frequency_selected.
 *
 * @param index Parameter index.
 */
    void frequency_selected(size_t index);

private:
/**
 * @brief rebuild_chart.
 *
 */
    void rebuild_chart();
/**
 * @brief update_highlight.
 *
 */
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
