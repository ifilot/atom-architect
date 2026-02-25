#include "analysis_geometry_optimization.h"

#include <cmath>

AnalysisGeometryOptimization::AnalysisGeometryOptimization(QObject *parent)
    : QObject(parent)
{
    viewer_ = new GeometryOptimizationViewer();
    graph_  = new GeometryOptimizationGraph();

    connect(viewer_, &GeometryOptimizationViewer::first_requested, this, &AnalysisGeometryOptimization::first);
    connect(viewer_, &GeometryOptimizationViewer::prev_requested, this, &AnalysisGeometryOptimization::prev);
    connect(viewer_, &GeometryOptimizationViewer::next_requested, this, &AnalysisGeometryOptimization::next);
    connect(viewer_, &GeometryOptimizationViewer::last_requested, this, &AnalysisGeometryOptimization::last);
    connect(viewer_, &GeometryOptimizationViewer::file_dropped, this, &AnalysisGeometryOptimization::load_file);
    connect(graph_, &GeometryOptimizationGraph::frequency_selected,
            this, &AnalysisGeometryOptimization::select_frequency_mode);

    frequency_animation_timer_.setInterval(40);
    connect(&frequency_animation_timer_, &QTimer::timeout,
            this, &AnalysisGeometryOptimization::tick_frequency_animation);
}

void AnalysisGeometryOptimization::set_structures(
    const std::vector<std::shared_ptr<Structure>>& s)
{
    if(s.empty()) {
        return;
    }

    mode_ = AnalysisMode::GEOMETRY_OPTIMIZATION;
    structures_ = s;
    frequency_structure_.reset();
    current_index_ = 0;
    animation_phase_ = 0.0;
    frequency_animation_timer_.stop();
    Structure::set_debug_logging_enabled(true);

    graph_->setVisible(true);
    viewer_->set_mode(GeometryOptimizationViewer::ViewerMode::GEOMETRY_OPTIMIZATION);
    graph_->set_structures(structures_);

    update_current();
    viewer_->set_structure(structures_[current_index_]);
}

void AnalysisGeometryOptimization::set_frequency_structure(const std::shared_ptr<Structure>& structure)
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
    viewer_->set_mode(GeometryOptimizationViewer::ViewerMode::FREQUENCY);

    update_current();
    frequency_animation_timer_.start();
}

void AnalysisGeometryOptimization::update_current()
{
    if(mode_ == AnalysisMode::GEOMETRY_OPTIMIZATION) {
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

void AnalysisGeometryOptimization::update_frequency_mode()
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

std::shared_ptr<Structure> AnalysisGeometryOptimization::build_frequency_frame_structure(
    size_t mode_index,
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

void AnalysisGeometryOptimization::first()
{
    if(mode_ == AnalysisMode::GEOMETRY_OPTIMIZATION) {
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

void AnalysisGeometryOptimization::prev()
{
    if(mode_ == AnalysisMode::GEOMETRY_OPTIMIZATION) {
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

void AnalysisGeometryOptimization::next()
{
    if(mode_ == AnalysisMode::GEOMETRY_OPTIMIZATION) {
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

void AnalysisGeometryOptimization::last()
{
    if(mode_ == AnalysisMode::GEOMETRY_OPTIMIZATION) {
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

void AnalysisGeometryOptimization::tick_frequency_animation()
{
    if(mode_ != AnalysisMode::FREQUENCY) {
        return;
    }

    animation_phase_ += animation_phase_increment_;
    update_frequency_mode();
}

void AnalysisGeometryOptimization::select_frequency_mode(size_t index)
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

void AnalysisGeometryOptimization::load_file(const QString &filename)
{
    StructureLoader sl;
    auto loaded = sl.load_outcar(filename.toStdString());

    if(loaded.empty()) {
        return;
    }

    if(loaded.size() == 1 && loaded.front()->get_nr_eigenmodes() > 0) {
        set_frequency_structure(loaded.front());
    } else {
        set_structures(loaded);
    }
}
