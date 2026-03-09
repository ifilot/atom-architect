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
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#include "anaglyph_widget.h"
#include "../data/structure.h"

/**
 * @brief StructureAnalysisViewer class.
 */
class StructureAnalysisViewer : public QWidget {
    Q_OBJECT

public:
    enum class ViewerMode {
        STRUCTURE_SERIES,
        FREQUENCY
    };

    enum class SeriesKind {
        GEOMETRY_OPTIMIZATION,
        NEB
    };

/**
 * @brief StructureAnalysisViewer.
 *
 * @param parent Parameter parent.
 */
    explicit StructureAnalysisViewer(QWidget *parent = nullptr);

/**
 * @brief set_structure.
 *
 * @param structure Parameter structure.
 */
    void set_structure(const std::shared_ptr<Structure>& structure);
/**
 * @brief set_structure_conservative.
 *
 * @param structure Parameter structure.
 */
    void set_structure_conservative(const std::shared_ptr<Structure>& structure);
/**
 * @brief set_index.
 *
 * @param index Parameter index.
 * @param total Parameter total.
 */
    void set_index(size_t index, size_t total);
/**
 * @brief set_mode.
 *
 * @param mode Parameter mode.
 */
    void set_mode(ViewerMode mode);
/**
 * @brief set_series_kind.
 *
 * @param kind Parameter kind.
 */
    void set_series_kind(SeriesKind kind);
/**
 * @brief set_header_widget.
 *
 * @param widget Parameter widget.
 */
    void set_header_widget(QWidget *widget);
/**
 * @brief set_side_toolbar.
 *
 * @param widget Parameter widget.
 */
    void set_side_toolbar(QWidget *widget);
    /**
     * @brief get_anaglyph_widget.
     *
     */
    inline AnaglyphWidget* get_anaglyph_widget() {
        return this->anaglyph_widget;
    }
    /**
     * @brief get_anaglyph_widget.
     *
     */
    inline const AnaglyphWidget* get_anaglyph_widget() const {
        return this->anaglyph_widget;
    }

signals:
/**
 * @brief first_requested.
 *
 */
    void first_requested();
/**
 * @brief prev_requested.
 *
 */
    void prev_requested();
/**
 * @brief next_requested.
 *
 */
    void next_requested();
/**
 * @brief last_requested.
 *
 */
    void last_requested();
/**
 * @brief edit_requested.
 *
 */
    void edit_requested();

/**
 * @brief file_dropped.
 *
 * @param filename Parameter filename.
 */
    void file_dropped(const QString& filename);

protected:
/**
 * @brief dragEnterEvent.
 *
 * @param event Parameter event.
 */
    void dragEnterEvent(QDragEnterEvent *event) override;
/**
 * @brief dragMoveEvent.
 *
 * @param event Parameter event.
 */
    void dragMoveEvent(QDragMoveEvent *event) override;
/**
 * @brief dragLeaveEvent.
 *
 * @param event Parameter event.
 */
    void dragLeaveEvent(QDragLeaveEvent *event) override;
/**
 * @brief dropEvent.
 *
 * @param event Parameter event.
 */
    void dropEvent(QDropEvent *event) override;

private:
/**
 * @brief update_title.
 *
 */
    void update_title();

    AnaglyphWidget *anaglyph_widget;

    QPushButton *button_first;
    QPushButton *button_previous;
    QPushButton *button_next;
    QPushButton *button_last;
    QPushButton *button_edit;

    QLabel *label_title;
    QLabel *label_structure_id;
    QLabel *label_current_energy;

    QHBoxLayout *header_layout = nullptr;
    QHBoxLayout *viewport_layout = nullptr;

    ViewerMode viewer_mode = ViewerMode::STRUCTURE_SERIES;
    SeriesKind series_kind_ = SeriesKind::GEOMETRY_OPTIMIZATION;
};
