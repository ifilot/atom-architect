/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2024 Ivo Filot <i.a.w.filot@tue.nl>                 *
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

#include "analysis_neb.h"

AnalysisNEB::AnalysisNEB(QWidget* parent) : QWidget(parent) {
    this->setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));

    // set layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);

    this->chartview = new QChartView();
    this->chartview->setRenderHint(QPainter::Antialiasing);

    // add Splitter
    QSplitter *splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(splitter);

    // add widget to splitter
    QWidget* anaglyph_hypercontainer = new QWidget();
    splitter->addWidget(anaglyph_hypercontainer);
    this->anaglyph_hypergrid = new QGridLayout();
    anaglyph_hypercontainer->setLayout(this->anaglyph_hypergrid);

    QWidget* graph_hypercontainer = new QWidget();
    splitter->addWidget(graph_hypercontainer);
    graph_hypercontainer->setLayout(new QVBoxLayout());
    graph_hypercontainer->layout()->addWidget(this->chartview);

    // add widget with cycle buttons
    QWidget* button_widget = new QWidget(this);
    layout->addWidget(button_widget);
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_widget->setLayout(button_layout);
    this->button_previous = new QPushButton("<");
    this->button_previous->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->button_next = new QPushButton(">");
    this->button_next->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    button_layout->addWidget(this->button_previous);
    button_layout->addWidget(this->button_next);
    this->label_structure_id = new QLabel("");
    button_layout->addWidget(this->label_structure_id);
    this->label_current_energy = new QLabel("");
    button_layout->addWidget(this->label_current_energy);
    button_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // build drop-down menu
    this->build_menu();

    this->resize(800,400);
}

/**
 * @brief      Update the graph
 */
void AnalysisNEB::update_graph() {
    this->chart = new QChart();

    QFont font;
    font.setPixelSize(12);
    this->chart->setTitleFont(font);
    this->chart->setTitleBrush(QBrush(QRgb(0x909090)));
    this->chart->setTitle("Nudged Elastic Band Energies");
    this->chartview->setChart(chart);

    this->axisX = new QValueAxis();
    this->axisY = new QValueAxis();

    // set background
    QLinearGradient background_gradient;
    background_gradient.setStart(QPointF(0, 0));
    background_gradient.setFinalStop(QPointF(0, 1));
    background_gradient.setColorAt(0.0, QRgb(0x151515));
    background_gradient.setColorAt(1.0, QRgb(0x151515));
    background_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    chart->setBackgroundBrush(background_gradient);

    // Customize axis label font
    QFont labelsFont;
    labelsFont.setPixelSize(12);
    this->axisX->setLabelsFont(labelsFont);
    this->axisY->setLabelsFont(labelsFont);

    auto energycol = QRgb(0x377eb8);

    // Customize axis colors
    QPen axisPen(QRgb(0x909090));
    axisPen.setWidth(2);
    this->axisX->setLinePen(axisPen);
    QPen axisPenY(energycol);
    this->axisY->setLinePen(axisPenY);

    // Customize axis label colors
    QBrush axisBrush(QRgb(0x909090));
    this->axisX->setLabelsBrush(axisBrush);
    QBrush axisBrushY(energycol);
    this->axisY->setLabelsBrush(axisBrushY);

    // Customize grid lines and shades
    this->axisX->setGridLineVisible(false);
    this->axisY->setGridLineVisible(true);
    this->axisY->setShadesPen(Qt::NoPen);
    this->axisY->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
    this->axisY->setShadesVisible(false);

    this->axisX->setRange(0, this->structures.size()+1);
    this->chart->addAxis(axisX, Qt::AlignBottom);
    this->chart->addAxis(axisY, Qt::AlignLeft);

    // remove all previous series
    for(int i=0; i<this->chart->series().size(); i++) {
        this->chart->removeSeries(this->chart->series().last());
    }

    // create new series
    double emin = 1e6;
    double emax = -1e6;
    this->spline_series.clear();
    for(unsigned int i=0; i<this->structures.front().size(); i++) {
        QSplineSeries *series = new QSplineSeries();
        this->spline_series.push_back(series);
        QScatterSeries *series_symbols = new QScatterSeries();
        series->setName(tr("Iteration %1").arg(i));
        series_symbols->setName(tr("Iteration %1").arg(i));
        series_symbols->setMarkerSize(3);
        series_symbols->setMarkerShape(QScatterSeries::MarkerShapeCircle);

        for(unsigned int j=0; j<this->structures.size(); j++) {
            const double energy = this->structures[j][i]->get_energy();
            *series << QPointF(j+1, energy);
            *series_symbols << QPointF(j+1, energy);
            emin = std::min(energy, emin);
            emax = std::max(energy, emax);
        }

        this->chart->addSeries(series);
        this->chart->addSeries(series_symbols);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        series_symbols->attachAxis(axisX);
        series_symbols->attachAxis(axisY);
        this->chart->legend()->markers(series)[0]->setVisible(false);
        this->chart->legend()->markers(series_symbols)[0]->setVisible(false);

        // set color
        QPen pen = series->pen();
        pen.setWidth(2);
        int cval = (int)(255.0 / (double)this->structures.front().size() * (double)i);
        pen.setBrush(QBrush(QColor(cval, 0, 255 - cval)));
        series->setPen(pen);
        pen = series_symbols->pen();
        pen.setBrush(QBrush(QColor(cval, 0, 255 - cval)));
        series_symbols->setPen(pen);
    }
    const double emargin = (emax - emin) * 0.1;

    this->axisX->setTitleText("Image number");
    this->axisX->setTickCount(this->structures.size()+1);
    this->axisX->setMinorTickCount(0);
    this->axisX->setLabelFormat("%i");

    this->axisY->setRange(emin - emargin, emax + emargin);
    this->axisY->setTitleText("Energy [eV]");

    this->current_structure_id = 0;
    this->update_labels();
    this->update_chart_highlight();
}

/**
 * @brief      Open new OUTCAR file
 */
void AnalysisNEB::open() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("VASP NEB binary (*.bin)"));

    if(filename.isEmpty()) {
        return;
    }

    this->load_file(filename);
}

/**
 * @brief load_file
 * @param filename
 */
void AnalysisNEB::load_file(const QString& filename) {
    StructureLoader sl;
    this->set_structures(sl.load_neb_bin(filename.toStdString()));

    // expand anaglyph widgets
    static const unsigned int nr_cols = 4;

    qDebug() << "Loading " << this->structures.size() << " images.";
    for(unsigned int i=0; i<this->structures.size(); i++) {
        this->anaglyph_widgets.push_back(new AnaglyphWidget());
        this->anaglyph_hypergrid->addWidget(this->anaglyph_widgets.back(), (i / nr_cols) * 2, i % nr_cols);

        this->image_labels.push_back(new QLabel(tr("Image %1").arg(i+1)));
        this->anaglyph_hypergrid->addWidget(this->image_labels.back(), (i / nr_cols) * 2 + 1, i % nr_cols);

        this->structures[i].front()->update();
        this->anaglyph_widgets.back()->set_structure(this->structures[i].front());
    }

    this->update_graph();
}

/**
 * @brief      Close window
 */
void AnalysisNEB::exit() {
    this->close();
}

/**
 * @brief      Cycle to previous structure
 */
void AnalysisNEB::prev() {
    if(this->structures.size() == 0) {
        return;
    }

    if(this->current_structure_id == 0) {
        this->current_structure_id = this->structures.front().size() - 1;
    } else {
        this->current_structure_id--;
    }

    this->update_labels();

    // set image
    for(unsigned int i=0; i<this->anaglyph_widgets.size(); i++) {
        this->anaglyph_widgets[i]->set_structure_conservative(this->structures[i][this->current_structure_id]);
        this->structures[i][this->current_structure_id]->update();
    }

    this->update_chart_highlight();
}

/**
 * @brief      Cycle to next structure
 */
void AnalysisNEB::next() {
    if(this->structures.size() == 0) {
        return;
    }

    this->current_structure_id++;
    if(this->current_structure_id == this->structures.front().size()) {
        this->current_structure_id = 0;
    }

    this->update_labels();

    // set image
    for(unsigned int i=0; i<this->anaglyph_widgets.size(); i++) {
        this->anaglyph_widgets[i]->set_structure_conservative(this->structures[i][this->current_structure_id]);
        this->structures[i][this->current_structure_id]->update();
    }

    this->update_chart_highlight();
}

/**
 * @brief      Update labels based on current structure
 */
void AnalysisNEB::update_labels() {
    for(unsigned int i=0; i<this->anaglyph_widgets.size(); i++) {
        this->image_labels[i]->setText(tr("Image: %1 (%2 eV)").arg(i+1).arg(this->structures[i][this->current_structure_id]->get_energy()));
    }
    this->label_structure_id->setText(tr("<b>Image:</b> %1 / %2").arg(this->current_structure_id+1).arg(this->structures.front().size()));
}


/**
 * @brief      Highlight point of interest on chart
 */
void AnalysisNEB::update_chart_highlight() {
    for(unsigned int i=0; i<this->structures.front().size(); i++) {
        QPen pen = this->spline_series[i]->pen();
        pen.setWidth(2);
        if(i == this->current_structure_id) {
            pen.setBrush(QBrush(QColor(255, 255, 255)));
        } else {
            int cval = (int)(255.0 / (double)this->structures.front().size() * (double)i);
            pen.setBrush(QBrush(QColor(cval, 0, 255 - cval)));
        }
        this->spline_series[i]->setPen(pen);
    }
}

/**
 * @brief      Builds a menu.
 */
void AnalysisNEB::build_menu() {
    // add drop down menu
    QMenuBar *menuBar = new QMenuBar;

    QMenu *menu_file = menuBar->addMenu(tr("&File"));
    QAction *action_open = new QAction(menu_file);
    QAction *action_quit = new QAction(menu_file);

    QMenu *menu_images = menuBar->addMenu(tr("&Images"));
    QAction *action_image_prev = new QAction(menu_images);
    action_image_prev->setShortcut(Qt::Key_Left);
    QObject::connect(action_image_prev, SIGNAL(triggered()), this, SLOT(prev()));
    QObject::connect(this->button_previous, SIGNAL(released()), this, SLOT(prev()));
    QAction *action_image_next = new QAction(menu_images);
    action_image_next->setShortcut(Qt::Key_Right);
    QObject::connect(action_image_next, SIGNAL(triggered()), this, SLOT(next()));
    QObject::connect(this->button_next, SIGNAL(released()), this, SLOT(next()));
    menu_images->addAction(action_image_prev);
    menu_images->addAction(action_image_next);

    QMenu *menu_camera = menuBar->addMenu(tr("&Camera"));
    QAction *action_camera_default = new QAction(menu_camera);
    QAction *action_camera_top = new QAction(menu_camera);
    QAction *action_camera_bottom = new QAction(menu_camera);
    QAction *action_camera_left = new QAction(menu_camera);
    QAction *action_camera_right = new QAction(menu_camera);
    QAction *action_camera_front = new QAction(menu_camera);
    QAction *action_camera_back = new QAction(menu_camera);

    action_open->setText(tr("Open"));
    action_open->setShortcuts(QKeySequence::Open);
    action_open->setIcon(QIcon(":/assets/icon/open.png"));
    action_quit->setText(tr("Quit"));
    action_quit->setShortcuts(QKeySequence::Quit);
    action_quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    action_quit->setIcon(QIcon(":/assets/icon/close.png"));

    // create actions for projection menu
    action_camera_default->setText(tr("Default"));
    action_camera_default->setData(QVariant((int)CameraAlignment::DEFAULT));
    action_camera_default->setShortcut(Qt::Key_0);
    action_camera_top->setText(tr("Top"));
    action_camera_top->setData(QVariant((int)CameraAlignment::TOP));
    action_camera_top->setShortcut(Qt::Key_7);
    action_camera_bottom->setText(tr("Bottom"));
    action_camera_bottom->setData(QVariant((int)CameraAlignment::BOTTOM));
    action_camera_bottom->setShortcut(Qt::CTRL | Qt::Key_7);
    action_camera_left->setText(tr("Left"));
    action_camera_left->setData(QVariant((int)CameraAlignment::LEFT));
    action_camera_left->setShortcut(Qt::Key_3);
    action_camera_right->setText(tr("Right"));
    action_camera_right->setData(QVariant((int)CameraAlignment::RIGHT));
    action_camera_right->setShortcut(Qt::CTRL | Qt::Key_3);
    action_camera_front->setText(tr("Front"));
    action_camera_front->setData(QVariant((int)CameraAlignment::FRONT));
    action_camera_front->setShortcut(Qt::Key_1);
    action_camera_back->setText(tr("Back"));
    action_camera_back->setData(QVariant((int)CameraAlignment::BACK));
    action_camera_back->setShortcut(Qt::CTRL | Qt::Key_1);

    menu_file->addAction(action_open);
    menu_file->addAction(action_quit);

    menu_camera->addAction(action_camera_default);
    menu_camera->addAction(action_camera_top);
    menu_camera->addAction(action_camera_bottom);
    menu_camera->addAction(action_camera_left);
    menu_camera->addAction(action_camera_right);
    menu_camera->addAction(action_camera_front);
    menu_camera->addAction(action_camera_back);

    // connect signals
    connect(action_open, SIGNAL(triggered()), this, SLOT(open()));
    connect(action_quit, SIGNAL(triggered()), this, SLOT(exit()));
    connect(menu_camera, SIGNAL(triggered(QAction*)), this, SLOT(set_camera_align(QAction*)));

    this->layout()->setMenuBar(menuBar);
}

/**
 * @brief      Align cameras
 */
void AnalysisNEB::set_camera_align(QAction* action) {
    for(AnaglyphWidget* aw : this->anaglyph_widgets) {
        aw->get_user_action()->set_camera_alignment(action->data().toInt());
    }
}
