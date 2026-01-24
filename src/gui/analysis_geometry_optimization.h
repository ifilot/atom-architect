#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "geometry_optimization_viewer.h"
#include "geometry_optimization_graph.h"
#include "../data/structure_loader.h"

class AnalysisGeometryOptimization : public QObject {
    Q_OBJECT

public:
    explicit AnalysisGeometryOptimization(QObject *parent = nullptr);

    GeometryOptimizationViewer* viewer() const { return viewer_; }
    GeometryOptimizationGraph* graph() const { return graph_; }

    void set_structures(const std::vector<std::shared_ptr<Structure>>& structures);

public slots:
    void load_file(const QString& filename);

private slots:
    void first();
    void prev();
    void next();
    void last();

private:
    void update_current();

private:
    GeometryOptimizationViewer *viewer_;
    GeometryOptimizationGraph *graph_;

    std::vector<std::shared_ptr<Structure>> structures_;
    size_t current_index_ = 0;
};
