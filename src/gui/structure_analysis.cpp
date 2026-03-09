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

#include "structure_analysis.h"

#include <cmath>

/**
 * @brief StructureAnalysis.
 *
 * @param parent Parameter parent.
 */
StructureAnalysis::StructureAnalysis(QObject *parent)
    : QObject(parent)
{
    viewer_ = new StructureAnalysisViewer();
    graph_  = new StructureAnalysisGraph();

    connect(viewer_, &StructureAnalysisViewer::first_requested, this, &StructureAnalysis::first);
    connect(viewer_, &StructureAnalysisViewer::prev_requested, this, &StructureAnalysis::prev);
    connect(viewer_, &StructureAnalysisViewer::next_requested, this, &StructureAnalysis::next);
    connect(viewer_, &StructureAnalysisViewer::last_requested, this, &StructureAnalysis::last);
    connect(viewer_, &StructureAnalysisViewer::file_dropped, this, &StructureAnalysis::load_file);
    connect(graph_, &StructureAnalysisGraph::frequency_selected,
            this, &StructureAnalysis::select_frequency_mode);

    frequency_animation_timer_.setInterval(40);
    connect(&frequency_animation_timer_, &QTimer::timeout,
            this, &StructureAnalysis::tick_frequency_animation);
}

/**
 * @brief set_structures.
 *
 * @param s Parameter s.
 * @param series_kind Parameter series_kind.
 */
void StructureAnalysis::set_structures(const std::vector<std::shared_ptr<Structure>>& s,
                                       StructureAnalysisViewer::SeriesKind series_kind)
{
    if(s.empty()) {
        return;
    }

    mode_ = AnalysisMode::STRUCTURE_SERIES;
    current_series_kind_ = series_kind;
    structures_ = s;
    frequency_structure_.reset();
    current_index_ = 0;
    animation_phase_ = 0.0;
    frequency_animation_timer_.stop();
    Structure::set_debug_logging_enabled(true);

    graph_->setVisible(true);
    viewer_->set_mode(StructureAnalysisViewer::ViewerMode::STRUCTURE_SERIES);
    viewer_->set_series_kind(series_kind);

    const auto graph_kind = (series_kind == StructureAnalysisViewer::SeriesKind::NEB)
        ? StructureAnalysisGraph::SeriesKind::NEB
        : StructureAnalysisGraph::SeriesKind::GEOMETRY_OPTIMIZATION;
    graph_->set_structures(structures_, graph_kind);

    update_current();
    viewer_->set_structure(structures_[current_index_]);
}

/**
 * @brief set_frequency_structure.
 *
 * @param structure Parameter structure.
 */
void StructureAnalysis::set_frequency_structure(const std::shared_ptr<Structure>& structure)
{
    if(!structure || structure->get_nr_eigenmodes() == 0) {
        return;
    }

    mode_ = AnalysisMode::FREQUENCY;
    structures_.clear();
    frequency_structure_ = structure;
    current_index_ = 0;
    animation_phase_ = 0.0;
    Structure::set_debug_logging_enabled(false);

    graph_->setVisible(true);
    graph_->set_frequency_modes(frequency_structure_->get_eigenmodes());
    graph_->set_current_index(current_index_);
    viewer_->set_mode(StructureAnalysisViewer::ViewerMode::FREQUENCY);

    update_current();
    frequency_animation_timer_.start();
}

/**
 * @brief update_current.
 *
 */
void StructureAnalysis::update_current()
{
    if(mode_ == AnalysisMode::STRUCTURE_SERIES) {
        if(structures_.empty()) {
            return;
        }

        viewer_->set_structure_conservative(structures_[current_index_]);
        viewer_->set_index(current_index_, structures_.size());
        graph_->set_current_index(current_index_);
        return;
    }

    if(mode_ == AnalysisMode::FREQUENCY) {
        update_frequency_mode();
    }
}

/**
 * @brief update_frequency_mode.
 *
 */
void StructureAnalysis::update_frequency_mode()
{
    if(!frequency_structure_) {
        return;
    }

    if(current_index_ >= frequency_structure_->get_nr_eigenmodes()) {
        current_index_ = 0;
    }

    auto display = build_frequency_frame_structure(current_index_, animation_phase_);
    viewer_->set_structure_conservative(display);
    viewer_->set_index(current_index_, frequency_structure_->get_nr_eigenmodes());
}

/**
 * @brief build_frequency_frame_structure.
 *
 * @param mode_index Parameter mode_index.
 * @param phase Parameter phase.
 */
std::shared_ptr<Structure> StructureAnalysis::build_frequency_frame_structure(size_t mode_index,
                                                                              double phase) const
{
    auto display = std::make_shared<Structure>(frequency_structure_->get_unitcell());

    const auto& base_atoms = frequency_structure_->get_atoms();
    const auto& mode = frequency_structure_->get_eigenmodes()[mode_index];
    const double oscillation = std::sin(phase) * animation_amplitude_;

    for(size_t i = 0; i < base_atoms.size(); ++i) {
        const auto& atom = base_atoms[i];
        QVector3D displaced = atom.get_pos_qtvec() + mode.eigenvectors[i] * oscillation;
        display->add_atom(atom.atnr, displaced.x(), displaced.y(), displaced.z());
    }

    display->set_energy(mode.eigenvalue);
    display->update();

    return display;
}

/**
 * @brief first.
 *
 */
void StructureAnalysis::first()
{
    if(mode_ == AnalysisMode::STRUCTURE_SERIES) {
        current_index_ = 0;
    } else if(mode_ == AnalysisMode::FREQUENCY && frequency_structure_) {
        current_index_ = 0;
        animation_phase_ = 0.0;
    }

    update_current();
    if(mode_ == AnalysisMode::FREQUENCY) {
        graph_->set_current_index(current_index_);
    }
}

/**
 * @brief prev.
 *
 */
void StructureAnalysis::prev()
{
    if(mode_ == AnalysisMode::STRUCTURE_SERIES) {
        current_index_ = (current_index_ == 0) ? structures_.size() - 1 : current_index_ - 1;
    } else if(mode_ == AnalysisMode::FREQUENCY && frequency_structure_) {
        const size_t n = frequency_structure_->get_nr_eigenmodes();
        current_index_ = (current_index_ == 0) ? n - 1 : current_index_ - 1;
        animation_phase_ = 0.0;
    }

    update_current();
    if(mode_ == AnalysisMode::FREQUENCY) {
        graph_->set_current_index(current_index_);
    }
}

/**
 * @brief next.
 *
 */
void StructureAnalysis::next()
{
    if(mode_ == AnalysisMode::STRUCTURE_SERIES) {
        current_index_ = (current_index_ + 1) % structures_.size();
    } else if(mode_ == AnalysisMode::FREQUENCY && frequency_structure_) {
        const size_t n = frequency_structure_->get_nr_eigenmodes();
        current_index_ = (current_index_ + 1) % n;
        animation_phase_ = 0.0;
    }

    update_current();
    if(mode_ == AnalysisMode::FREQUENCY) {
        graph_->set_current_index(current_index_);
    }
}

/**
 * @brief last.
 *
 */
void StructureAnalysis::last()
{
    if(mode_ == AnalysisMode::STRUCTURE_SERIES) {
        current_index_ = structures_.size() - 1;
    } else if(mode_ == AnalysisMode::FREQUENCY && frequency_structure_) {
        current_index_ = frequency_structure_->get_nr_eigenmodes() - 1;
        animation_phase_ = 0.0;
    }

    update_current();
    if(mode_ == AnalysisMode::FREQUENCY) {
        graph_->set_current_index(current_index_);
    }
}

/**
 * @brief tick_frequency_animation.
 *
 */
void StructureAnalysis::tick_frequency_animation()
{
    if(mode_ != AnalysisMode::FREQUENCY) {
        return;
    }

    animation_phase_ += animation_phase_increment_;
    update_frequency_mode();
}

/**
 * @brief select_frequency_mode.
 *
 * @param index Parameter index.
 */
void StructureAnalysis::select_frequency_mode(size_t index)
{
    if(mode_ != AnalysisMode::FREQUENCY || !frequency_structure_) {
        return;
    }

    if(index >= frequency_structure_->get_nr_eigenmodes()) {
        return;
    }

    current_index_ = index;
    animation_phase_ = 0.0;
    graph_->set_current_index(current_index_);
    update_current();
}

    /**
     * @brief      Sets the camera align.
     *
     * @param      action  The action
     */
void StructureAnalysis::set_camera_align(QAction* action)
{
    if(!action || !viewer_ || !viewer_->get_anaglyph_widget()) {
        return;
    }

    viewer_->get_anaglyph_widget()->get_user_action()->set_camera_alignment(action->data().toInt());
}

    /**
     * @brief      Sets the camera mode (orthogonal or perspective).
     *
     * @param      action  The action
     */
void StructureAnalysis::set_camera_mode(QAction* action)
{
    if(!action || !viewer_ || !viewer_->get_anaglyph_widget()) {
        return;
    }

    viewer_->get_anaglyph_widget()->get_user_action()->set_camera_mode(action->data().toInt());
}

/**
 * @brief set_stereo.
 *
 * @param stereo_name Parameter stereo_name.
 */
void StructureAnalysis::set_stereo(const QString& stereo_name)
{
    if(!viewer_ || !viewer_->get_anaglyph_widget()) {
        return;
    }

    viewer_->get_anaglyph_widget()->set_stereo(stereo_name);
}

/**
 * @brief load_file.
 *
 * @param filename Parameter filename.
 */
void StructureAnalysis::load_file(const QString &filename)
{
    StructureLoader sl;
    std::vector<std::shared_ptr<Structure>> loaded;

    if(filename.contains("OUTCAR", Qt::CaseInsensitive)) {
        loaded = sl.load_outcar(filename.toStdString());
    } else if(filename.endsWith(".yaml", Qt::CaseInsensitive) ||
              filename.endsWith(".yml", Qt::CaseInsensitive)) {
        loaded = sl.load_yaml(filename.toStdString());
    } else {
        return;
    }

    if(loaded.empty()) {
        return;
    }

    if(loaded.size() == 1 && loaded.front()->get_nr_eigenmodes() > 0) {
        set_frequency_structure(loaded.front());
    } else {
        set_structures(loaded, StructureAnalysisViewer::SeriesKind::GEOMETRY_OPTIMIZATION);
    }
}
