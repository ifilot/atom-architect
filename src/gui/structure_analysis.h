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

#include <QObject>
#include <QTimer>
#include <QAction>
#include <memory>
#include <vector>

#include "structure_analysis_viewer.h"
#include "structure_analysis_graph.h"
#include "../data/structure_loader.h"

/**
 * @brief StructureAnalysis class.
 */
class StructureAnalysis : public QObject {
    Q_OBJECT

public:
/**
 * @brief StructureAnalysis.
 *
 * @param parent Parameter parent.
 */
    explicit StructureAnalysis(QObject *parent = nullptr);

    /**
     * @brief viewer.
     *
     */
    StructureAnalysisViewer* viewer() const { return viewer_; }
    /**
     * @brief graph.
     *
     */
    StructureAnalysisGraph* graph() const { return graph_; }

    /**
     * @brief set_structures.
     *
     * @param structures Parameter structures.
     * @param series_kind Parameter series_kind.
     */
    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures,
                        StructureAnalysisViewer::SeriesKind series_kind = StructureAnalysisViewer::SeriesKind::GEOMETRY_OPTIMIZATION);
/**
 * @brief set_frequency_structure.
 *
 * @param structure Parameter structure.
 */
    void set_frequency_structure(const std::shared_ptr<Structure>& structure);

public slots:
/**
 * @brief load_file.
 *
 * @param filename Parameter filename.
 */
    void load_file(const QString& filename);
/**
 * @brief set_camera_align.
 *
 * @param action Parameter action.
 */
    void set_camera_align(QAction* action);
/**
 * @brief set_camera_mode.
 *
 * @param action Parameter action.
 */
    void set_camera_mode(QAction* action);
/**
 * @brief set_stereo.
 *
 * @param stereo_name Parameter stereo_name.
 */
    void set_stereo(const QString& stereo_name);

private slots:
/**
 * @brief first.
 *
 */
    void first();
/**
 * @brief prev.
 *
 */
    void prev();
/**
 * @brief next.
 *
 */
    void next();
/**
 * @brief last.
 *
 */
    void last();
/**
 * @brief tick_frequency_animation.
 *
 */
    void tick_frequency_animation();
/**
 * @brief select_frequency_mode.
 *
 * @param index Parameter index.
 */
    void select_frequency_mode(size_t index);

private:
    enum class AnalysisMode {
        NONE,
        STRUCTURE_SERIES,
        FREQUENCY
    };

/**
 * @brief update_current.
 *
 */
    void update_current();
/**
 * @brief update_frequency_mode.
 *
 */
    void update_frequency_mode();
/**
 * @brief build_frequency_frame_structure.
 *
 * @param mode_index Parameter mode_index.
 * @param phase Parameter phase.
 */
    std::shared_ptr<Structure> build_frequency_frame_structure(size_t mode_index, double phase) const;

private:
    StructureAnalysisViewer *viewer_;
    StructureAnalysisGraph *graph_;

    AnalysisMode mode_ = AnalysisMode::NONE;
    StructureAnalysisViewer::SeriesKind current_series_kind_ = StructureAnalysisViewer::SeriesKind::GEOMETRY_OPTIMIZATION;
    std::vector<std::shared_ptr<Structure>> structures_;
    std::shared_ptr<Structure> frequency_structure_;

    size_t current_index_ = 0;

    QTimer frequency_animation_timer_;
    double animation_phase_ = 0.0;
    static constexpr double animation_phase_increment_ = 0.22;
    static constexpr double animation_amplitude_ = 0.35;
};
