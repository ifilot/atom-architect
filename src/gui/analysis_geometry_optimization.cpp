#include "analysis_geometry_optimization.h"

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
}

void AnalysisGeometryOptimization::set_structures(
    const std::vector<std::shared_ptr<Structure>>& s)
{
    structures_ = s;
    current_index_ = 0;

    graph_->set_structures(structures_);
    update_current();
    viewer_->set_structure(structures_[current_index_]);
}

void AnalysisGeometryOptimization::update_current()
{
    viewer_->set_structure_conservative(structures_[current_index_]);
    viewer_->set_index(current_index_, structures_.size());
    graph_->set_current_index(current_index_);
}

void AnalysisGeometryOptimization::first()
{
    current_index_ = 0;
    update_current();
}

void AnalysisGeometryOptimization::prev()
{
    current_index_ = (current_index_ == 0) ? structures_.size() - 1 : current_index_ - 1;
    update_current();
}

void AnalysisGeometryOptimization::next()
{
    current_index_ = (current_index_ + 1) % structures_.size();
    update_current();
}

void AnalysisGeometryOptimization::last()
{
    current_index_ = structures_.size() - 1;
    update_current();
}

void AnalysisGeometryOptimization::load_file(const QString &filename)
{
    StructureLoader sl;
    set_structures(sl.load_outcar(filename.toStdString()));
}
