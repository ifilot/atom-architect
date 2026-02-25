#include "geometry_optimization_viewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>

namespace {
constexpr double THZ_TO_WAVENUMBER = 33.35640951981521;
}

GeometryOptimizationViewer::GeometryOptimizationViewer(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    // ---- Header ----
    label_title = new QLabel("GEOMETRY OPTIMIZATION", this);
    label_title->setStyleSheet("font-weight: bold;");
    layout->addWidget(label_title);

    // ---- Anaglyph ----
    anaglyph_widget = new AnaglyphWidget(this);
    anaglyph_widget->disable_selection();
    anaglyph_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(anaglyph_widget);

    // ---- Controls ----
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

    connect(button_first, &QPushButton::clicked, this, &GeometryOptimizationViewer::first_requested);
    connect(button_previous, &QPushButton::clicked, this, &GeometryOptimizationViewer::prev_requested);
    connect(button_next, &QPushButton::clicked, this, &GeometryOptimizationViewer::next_requested);
    connect(button_last, &QPushButton::clicked, this, &GeometryOptimizationViewer::last_requested);
    connect(button_edit, &QPushButton::clicked, this, &GeometryOptimizationViewer::edit_requested);
}

void GeometryOptimizationViewer::set_structure_conservative(
    const std::shared_ptr<Structure>& structure)
{
    anaglyph_widget->set_structure_conservative(structure);
}

void GeometryOptimizationViewer::set_structure(
    const std::shared_ptr<Structure>& structure)
{
    anaglyph_widget->set_structure(structure);
}

void GeometryOptimizationViewer::set_mode(ViewerMode mode)
{
    viewer_mode = mode;

    if(viewer_mode == ViewerMode::GEOMETRY_OPTIMIZATION) {
        label_title->setText("GEOMETRY OPTIMIZATION");
    } else {
        label_title->setText("FREQUENCY ANALYSIS");
    }
}

void GeometryOptimizationViewer::set_index(size_t index, size_t total)
{
    if(viewer_mode == ViewerMode::GEOMETRY_OPTIMIZATION) {
        label_structure_id->setText(
            tr("<b>Image:</b> %1 / %2").arg(index + 1).arg(total));
        label_current_energy->setText(
            tr("<b>Energy:</b> %1").arg(
                anaglyph_widget->get_structure()->get_energy(), 0, 'f', 6));
    } else {
        label_structure_id->setText(
            tr("<b>Mode:</b> %1 / %2").arg(index + 1).arg(total));
        const double cm1 = anaglyph_widget->get_structure()->get_energy() * THZ_TO_WAVENUMBER;
        label_current_energy->setText(
            tr("<b>Frequency (cm<sup>-1</sup>):</b> %1").arg(cm1, 0, 'f', 2));
    }
}

// -------------------- Drag & Drop --------------------

void GeometryOptimizationViewer::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void GeometryOptimizationViewer::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void GeometryOptimizationViewer::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void GeometryOptimizationViewer::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls())
        return;

    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo fi(path);
        if (fi.exists() && fi.isFile()) {
            emit file_dropped(path);
            break;
        }
    }
}
