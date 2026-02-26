#pragma once

#include <QObject>
#include <QTimer>
#include <QAction>
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
    void set_frequency_structure(const std::shared_ptr<Structure>& structure);

public slots:
    void load_file(const QString& filename);
    void set_camera_align(QAction* action);
    void set_camera_mode(QAction* action);
    void set_stereo(const QString& stereo_name);

private slots:
    void first();
    void prev();
    void next();
    void last();
    void tick_frequency_animation();
    void select_frequency_mode(size_t index);

private:
    enum class AnalysisMode {
        NONE,
        GEOMETRY_OPTIMIZATION,
        FREQUENCY
    };

    void update_current();
    void update_frequency_mode();
    std::shared_ptr<Structure> build_frequency_frame_structure(size_t mode_index, double phase) const;

private:
    GeometryOptimizationViewer *viewer_;
    GeometryOptimizationGraph *graph_;

    AnalysisMode mode_ = AnalysisMode::NONE;
    std::vector<std::shared_ptr<Structure>> structures_;
    std::shared_ptr<Structure> frequency_structure_;

    size_t current_index_ = 0;

    QTimer frequency_animation_timer_;
    double animation_phase_ = 0.0;
    static constexpr double animation_phase_increment_ = 0.22;
    static constexpr double animation_amplitude_ = 0.35;
};
