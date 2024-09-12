/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2024 Ivo Filot <i.a.w.filot@tue.nl>                 *
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
#include <QIcon>
#include <QtCharts>
#include <QSplitter>

#include "anaglyph_widget.h"
#include "../data/structure_loader.h"

class AnalysisNEB : public QWidget {
    Q_OBJECT

private:
    std::vector<AnaglyphWidget*> anaglyph_widgets;
    std::vector<QLabel*> image_labels;
    QChartView *chartview;

    std::vector<std::vector<std::shared_ptr<Structure>> > structures;
    QGridLayout* anaglyph_hypergrid;

    QPushButton* button_previous;
    QPushButton* button_next;

    QLabel* label_structure_id;
    QLabel* label_current_energy;

    size_t current_structure_id = 0;

    QChart* chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY2;

    std::vector<QSplineSeries*> spline_series;

public:
    AnalysisNEB(QWidget* parent = nullptr);

    inline void set_structures(const std::vector<std::vector<std::shared_ptr<Structure>>>& _structures) {
        this->structures = _structures;
    }

    /**
     * @brief load_file
     * @param filename
     */
    void load_file(const QString& filename);

private:
    /**
     * @brief      Update the graph
     */
    void update_graph();

    /**
     * @brief      Update labels based on current structure
     */
    void update_labels();

    /**
     * @brief      Highlight point of interest on chart
     */
    void update_chart_highlight();

    /**
     * @brief      Builds a menu.
     */
    void build_menu();

private slots:
    /**
     * @brief      Open new OUTCAR file
     */
    void open();

    /**
     * @brief      Close window
     */
    void exit();

    /**
     * @brief      Cycle to previous structure
     */
    void prev();

    /**
     * @brief      Cycle to next structure
     */
    void next();

    /**
     * @brief      Align cameras
     */
    void set_camera_align(QAction* action);
};
