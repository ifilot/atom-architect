#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>

#include "anaglyph_widget.h"
#include "../data/structure.h"

class GeometryOptimizationViewer : public QWidget {
    Q_OBJECT

public:
    enum class ViewerMode {
        GEOMETRY_OPTIMIZATION,
        FREQUENCY
    };

    explicit GeometryOptimizationViewer(QWidget *parent = nullptr);

    void set_structure(const std::shared_ptr<Structure>& structure);
    void set_structure_conservative(const std::shared_ptr<Structure>& structure);
    void set_index(size_t index, size_t total);
    void set_mode(ViewerMode mode);
    inline const auto* get_anaglyph_widget() const {
        return this->anaglyph_widget;
    }

signals:
    void first_requested();
    void prev_requested();
    void next_requested();
    void last_requested();
    void edit_requested();

    void file_dropped(const QString& filename);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    AnaglyphWidget *anaglyph_widget;

    QPushButton *button_first;
    QPushButton *button_previous;
    QPushButton *button_next;
    QPushButton *button_last;
    QPushButton *button_edit;

    QLabel *label_title;
    QLabel *label_structure_id;
    QLabel *label_current_energy;

    ViewerMode viewer_mode = ViewerMode::GEOMETRY_OPTIMIZATION;
};
