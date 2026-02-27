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

#include "structure_analysis_viewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>

namespace {
constexpr double THZ_TO_WAVENUMBER = 33.35640951981521;
}

/**
 * @brief StructureAnalysisViewer.
 *
 * @param parent Parameter parent.
 */
StructureAnalysisViewer::StructureAnalysisViewer(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QWidget *header = new QWidget(this);
    header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(0, 0, 0, 0);
    header_layout->setSpacing(6);
    label_title = new QLabel("STRUCTURE ANALYSIS", header);
    label_title->setStyleSheet("font-weight: bold;");
    header_layout->addWidget(label_title);
    header_layout->addStretch();
    layout->addWidget(header);

    QWidget *viewportRow = new QWidget(this);
    viewport_layout = new QHBoxLayout(viewportRow);
    viewport_layout->setContentsMargins(0, 0, 0, 0);
    viewport_layout->setSpacing(6);

    anaglyph_widget = new AnaglyphWidget(viewportRow);
    anaglyph_widget->disable_selection();
    anaglyph_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewport_layout->addWidget(anaglyph_widget, 1);
    layout->addWidget(viewportRow, 1);

    QWidget *controls = new QWidget(this);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controls);

    button_first = new QPushButton("<<", controls);
    button_previous = new QPushButton("<", controls);
    button_next = new QPushButton(">", controls);
    button_last = new QPushButton(">>", controls);
    button_edit = new QPushButton("Send to editor", controls);

    label_structure_id = new QLabel("", controls);
    label_current_energy = new QLabel("", controls);

    controlsLayout->addWidget(button_first);
    controlsLayout->addWidget(button_previous);
    controlsLayout->addWidget(button_next);
    controlsLayout->addWidget(button_last);
    controlsLayout->addWidget(label_structure_id);
    controlsLayout->addWidget(label_current_energy);
    controlsLayout->addWidget(button_edit);
    controlsLayout->addStretch();

    layout->addWidget(controls);

    connect(button_first, &QPushButton::clicked, this, &StructureAnalysisViewer::first_requested);
    connect(button_previous, &QPushButton::clicked, this, &StructureAnalysisViewer::prev_requested);
    connect(button_next, &QPushButton::clicked, this, &StructureAnalysisViewer::next_requested);
    connect(button_last, &QPushButton::clicked, this, &StructureAnalysisViewer::last_requested);
    connect(button_edit, &QPushButton::clicked, this, &StructureAnalysisViewer::edit_requested);

    update_title();
}

/**
 * @brief set_header_widget.
 *
 * @param widget Parameter widget.
 */
void StructureAnalysisViewer::set_header_widget(QWidget *widget)
{
    if(widget == nullptr || header_layout == nullptr) {
        return;
    }

    widget->setParent(this);
    header_layout->insertWidget(0, widget, 0, Qt::AlignLeft);
}

/**
 * @brief set_side_toolbar.
 *
 * @param widget Parameter widget.
 */
void StructureAnalysisViewer::set_side_toolbar(QWidget *widget)
{
    if(widget == nullptr || viewport_layout == nullptr) {
        return;
    }

    widget->setParent(this);
    viewport_layout->insertWidget(0, widget, 0, Qt::AlignTop);
}

/**
 * @brief set_structure_conservative.
 *
 * @param structure Parameter structure.
 */
void StructureAnalysisViewer::set_structure_conservative(const std::shared_ptr<Structure>& structure)
{
    anaglyph_widget->set_structure_conservative(structure);
}

    /**
     * @brief      Sets the structure.
     *
     * @param[in]  _structure  The structure
     */
void StructureAnalysisViewer::set_structure(const std::shared_ptr<Structure>& structure)
{
    anaglyph_widget->set_structure(structure);
}

/**
 * @brief set_mode.
 *
 * @param mode Parameter mode.
 */
void StructureAnalysisViewer::set_mode(ViewerMode mode)
{
    viewer_mode = mode;
    update_title();
}

/**
 * @brief set_series_kind.
 *
 * @param kind Parameter kind.
 */
void StructureAnalysisViewer::set_series_kind(SeriesKind kind)
{
    series_kind_ = kind;
    update_title();
}

/**
 * @brief update_title.
 *
 */
void StructureAnalysisViewer::update_title()
{
    if(viewer_mode == ViewerMode::FREQUENCY) {
        label_title->setText("FREQUENCY ANALYSIS");
        return;
    }

    if(series_kind_ == SeriesKind::NEB) {
        label_title->setText("NEB ANALYSIS");
    } else {
        label_title->setText("GEOMETRY OPTIMIZATION");
    }
}

/**
 * @brief set_index.
 *
 * @param index Parameter index.
 * @param total Parameter total.
 */
void StructureAnalysisViewer::set_index(size_t index, size_t total)
{
    if(viewer_mode == ViewerMode::STRUCTURE_SERIES) {
        label_structure_id->setText(tr("<b>Image:</b> %1 / %2").arg(index + 1).arg(total));
        label_current_energy->setText(
            tr("<b>Energy:</b> %1").arg(anaglyph_widget->get_structure()->get_energy(), 0, 'f', 6));
    } else {
        label_structure_id->setText(tr("<b>Mode:</b> %1 / %2").arg(index + 1).arg(total));
        const double cm1 = anaglyph_widget->get_structure()->get_energy() * THZ_TO_WAVENUMBER;
        label_current_energy->setText(tr("<b>Frequency (cm<sup>-1</sup>):</b> %1").arg(cm1, 0, 'f', 2));
    }
}

/**
 * @brief dragEnterEvent.
 *
 * @param event Parameter event.
 */
void StructureAnalysisViewer::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief dragMoveEvent.
 *
 * @param event Parameter event.
 */
void StructureAnalysisViewer::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief dragLeaveEvent.
 *
 * @param event Parameter event.
 */
void StructureAnalysisViewer::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

/**
 * @brief dropEvent.
 *
 * @param event Parameter event.
 */
void StructureAnalysisViewer::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo fi(path);
        if (fi.exists() && fi.isFile()) {
            emit file_dropped(path);
            break;
        }
    }
}
