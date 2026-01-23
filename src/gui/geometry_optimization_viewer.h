#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>

#include "anaglyph_widget.h"
#include "../data/structure.h"

class GeometryOptimizationViewer : public QWidget {
    Q_OBJECT

public:
    explicit GeometryOptimizationViewer(QWidget *parent = nullptr);

    void set_structure(const std::shared_ptr<Structure>& structure);
    void set_structure_conservative(const std::shared_ptr<Structure>& structure);
    void set_index(size_t index, size_t total);

signals:
    void prev_requested();
    void next_requested();
    void file_dropped(const QString& filename);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    AnaglyphWidget *anaglyph_widget;

    QPushButton *button_previous;
    QPushButton *button_next;

    QLabel *label_structure_id;
    QLabel *label_current_energy;
};
