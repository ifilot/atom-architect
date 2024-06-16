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

#include "analysis_geometry_optimization.h"

AnalysisGeometryOptimization::AnalysisGeometryOptimization(QWidget* parent) : QWidget(parent) {
    this->setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));

    // add drop down menu
    QMenuBar *menuBar = new QMenuBar;
    QMenu *menu_file = menuBar->addMenu(tr("&File"));
    QMenu *menu_images = menuBar->addMenu(tr("&Images"));
    QAction *action_open = new QAction(menu_file);
    QAction *action_quit = new QAction(menu_file);
    action_open->setText(tr("Open"));
    action_open->setShortcuts(QKeySequence::Open);
    action_open->setIcon(QIcon(":/assets/icon/open.png"));
    action_quit->setText(tr("Quit"));
    action_quit->setShortcuts(QKeySequence::Quit);
    action_quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    action_quit->setIcon(QIcon(":/assets/icon/close.png"));
    menu_file->addAction(action_open);
    menu_file->addAction(action_quit);
    connect(action_open, SIGNAL(triggered()), this, SLOT(open()));
    connect(action_quit, SIGNAL(triggered()), this, SLOT(exit()));

    // accept drops
    this->setAcceptDrops(true);

    // set layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    this->anaglyph_widget = new AnaglyphWidget();
    this->layout()->setMenuBar(menuBar);

    this->chartview = new QChartView();
    this->chartview->setRenderHint(QPainter::Antialiasing);

    // add Splitter
    QSplitter *splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(splitter);

    // add widget to splitter
    splitter->addWidget(this->anaglyph_widget);
    splitter->addWidget(this->chartview);

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

    // cycle left
    QAction *action_image_prev = new QAction(menu_images);
    action_image_prev->setShortcut(Qt::Key_Left);
    QObject::connect(action_image_prev, SIGNAL(triggered()), this, SLOT(prev()));
    QObject::connect(this->button_previous, SIGNAL(released()), this, SLOT(prev()));

    // cycle right
    QAction *action_image_next = new QAction(menu_images);
    action_image_next->setShortcut(Qt::Key_Right);
    QObject::connect(action_image_next, SIGNAL(triggered()), this, SLOT(next()));
    QObject::connect(this->button_next, SIGNAL(released()), this, SLOT(next()));

    // set images menu
    menu_images->addAction(action_image_prev);
    menu_images->addAction(action_image_next);

    this->resize(800,400);
}

/**
 * @brief      Update the graph
 */
void AnalysisGeometryOptimization::update_graph() {
    this->chart = new QChart();
    auto *energy_series = new QLineSeries();
    auto *force_series = new QLineSeries();

    for(unsigned int i=0; i<this->structures.size(); i++) {
        energy_series->append((double)(i+1), this->structures[i]->get_energy());
        force_series->append((double)(i+1), this->structures[i]->get_rms_force());
    }
    energy_series->setName("Energy");
    force_series->setName("Force");

    QLinearGradient background_gradient;
    background_gradient.setStart(QPointF(0, 0));
    background_gradient.setFinalStop(QPointF(0, 1));
    background_gradient.setColorAt(0.0, QRgb(0x151515));
    background_gradient.setColorAt(1.0, QRgb(0x151515));
    background_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    chart->setBackgroundBrush(background_gradient);

    QFont font;
    font.setPixelSize(12);
    this->chart->setTitleFont(font);
    this->chart->setTitleBrush(QBrush(QRgb(0x909090)));
    this->chart->setTitle("Geometry optimization analysis");

    this->chart->addSeries(energy_series);
    this->chart->addSeries(force_series);
    this->chartview->setChart(chart);

    this->axisX = new QValueAxis();
    this->axisY = new QValueAxis();
    this->axisY2 = new QValueAxis();

    // Customize axis label font
    QFont labelsFont;
    labelsFont.setPixelSize(12);
    this->axisX->setLabelsFont(labelsFont);
    this->axisY->setLabelsFont(labelsFont);
    this->axisY2->setLabelsFont(labelsFont);

    auto energycol = QRgb(0x377eb8);
    auto forcecol = QRgb(0xe41a1c);
    energy_series->setColor(energycol);
    force_series->setColor(forcecol);

    // Customize axis colors
    QPen axisPen(QRgb(0x909090));
    axisPen.setWidth(2);
    this->axisX->setLinePen(axisPen);
    QPen axisPenY(energycol);
    this->axisY->setLinePen(axisPenY);
    QPen axisPenY2(forcecol);
    this->axisY2->setLinePen(axisPenY2);

    // Customize axis label colors
    QBrush axisBrush(QRgb(0x909090));
    this->axisX->setLabelsBrush(axisBrush);
    QBrush axisBrushY(energycol);
    this->axisY->setLabelsBrush(axisBrushY);
    QBrush axisBrushY2(forcecol);
    this->axisY2->setLabelsBrush(axisBrushY2);

    // Customize grid lines and shades
    this->axisX->setGridLineVisible(false);
    this->axisY->setGridLineVisible(true);
    this->axisY->setShadesPen(Qt::NoPen);
    this->axisY->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
    this->axisY->setShadesVisible(false);

    this->axisY2->setGridLineVisible(false);
    this->axisY2->setShadesPen(Qt::NoPen);
    this->axisY2->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
    this->axisY2->setShadesVisible(false);

    this->axisX->setRange(1, this->structures.size());

    // determine lowest and highest energy
    double emin = 1e6;
    double emax = -1e6;
    double fmin = 1e6;
    double fmax = -1e6;
    for(const auto& structure : this->structures) {
        emin = std::min(structure->get_energy(), emin);
        emax = std::max(structure->get_energy(), emax);

        fmin = std::min(structure->get_rms_force(), fmin);
        fmax = std::max(structure->get_rms_force(), fmax);
    }
    const double emargin = (emax - emin) * 0.1;
    const double fmargin = (fmax - fmin) * 0.1;

    this->axisX->setTitleText("Frame number");
    this->axisX->setTickCount(std::min(this->structures.size(), (size_t)10));
    this->axisX->setMinorTickCount(0);
    this->axisX->setLabelFormat("%i");

    this->axisY->setRange(emin - emargin, emax + emargin);
    this->axisY2->setRange(fmin - fmargin, fmax + fmargin);
    this->axisY->setTitleText("Energy [eV]");
    this->axisY2->setTitleText("Force [eV/A]");

    this->chart->addAxis(axisX, Qt::AlignBottom);
    this->chart->addAxis(axisY, Qt::AlignLeft);
    this->chart->addAxis(axisY2, Qt::AlignRight);
    energy_series->attachAxis(axisX);
    energy_series->attachAxis(axisY);
    force_series->attachAxis(axisX);
    force_series->attachAxis(axisY2);

    this->current_structure_id = 0;
    this->update_labels();
    this->update_chart_highlight();
}

/**
 * @brief      Open new OUTCAR file
 */
void AnalysisGeometryOptimization::open() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("VASP OUTCAR (OUTCAR)"));

    if(filename.isEmpty()) {
        return;
    }

    StructureLoader sl;
    this->set_structures(sl.load_outcar(filename.toStdString()));
    this->update_graph();
}

/**
 * @brief      Close window
 */
void AnalysisGeometryOptimization::exit() {
    this->close();
}

/**
 * @brief      Cycle to previous structure
 */
void AnalysisGeometryOptimization::prev() {
    if(this->current_structure_id == 0) {
        this->current_structure_id = this->structures.size() - 1;
    } else {
        this->current_structure_id--;
    }

    this->update_labels();
    this->structures[this->current_structure_id]->update();
    this->anaglyph_widget->set_structure_conservative(this->structures[this->current_structure_id]);
    this->update_chart_highlight();
}

/**
 * @brief      Cycle to next structure
 */
void AnalysisGeometryOptimization::next() {
    this->current_structure_id++;
    if(this->current_structure_id == this->structures.size()) {
        this->current_structure_id = 0;
    }

    this->update_labels();
    this->structures[this->current_structure_id]->update();
    this->anaglyph_widget->set_structure_conservative(this->structures[this->current_structure_id]);
    this->update_chart_highlight();
}

/**
 * @brief      Update labels based on current structure
 */
void AnalysisGeometryOptimization::update_labels() {
    this->label_structure_id->setText(tr("<b>Image:</b> %1 / %2").arg(this->current_structure_id+1).arg(this->structures.size()));
    this->label_current_energy->setText(tr("<b>Energy:</b> %1").arg(this->structures[this->current_structure_id]->get_energy(), 0, 'f', 6));
}


/**
 * @brief      Highlight point of interest on chart
 */
void AnalysisGeometryOptimization::update_chart_highlight() {
    if(this->chart->series().size() > 2) {
        this->chart->removeSeries(this->chart->series().last());
        this->chart->removeSeries(this->chart->series().last());
    }

    QScatterSeries *highlight_energy = new QScatterSeries();
    highlight_energy->setName("Current energy");
    highlight_energy->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    highlight_energy->setMarkerSize(5.0);
    *highlight_energy << QPointF(this->current_structure_id+1, this->structures[this->current_structure_id]->get_energy());

    this->chart->addSeries(highlight_energy);
    highlight_energy->attachAxis(this->axisX);
    highlight_energy->attachAxis(this->axisY);

    QScatterSeries *highlight_force = new QScatterSeries();
    highlight_force->setName("Current energy");
    highlight_force->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    highlight_force->setMarkerSize(5.0);
    *highlight_force << QPointF(this->current_structure_id+1, this->structures[this->current_structure_id]->get_rms_force());

    this->chart->addSeries(highlight_force);
    highlight_force->attachAxis(this->axisX);
    highlight_force->attachAxis(this->axisY2);

    this->chart->legend()->markers(highlight_energy)[0]->setVisible(false);
    this->chart->legend()->markers(highlight_force)[0]->setVisible(false);
}

/**
 * @brief      Handles drag Enter event
 *
 * @param      event  The event
 */
void AnalysisGeometryOptimization::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief      Handles file drop event
 *
 * @param      event  The event
 */
void AnalysisGeometryOptimization::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString text;
        for (int i = 0; i < urlList.size() && i < 32; ++i) {
            QString url = urlList.at(i).path();

            #ifdef Q_OS_WIN
            // remove leading / on Windows path; this sometimes causes problems
            QFileInfo check_file_win(url);
            if(!check_file_win.exists()) {
                if(url[0] == '/') {
                    url = url.remove(0,1);
                }
            }
            #endif

            // check if file exists, else show error message
            QFileInfo check_file(url);
            if(check_file.exists() && check_file.isFile()) {
                StructureLoader sl;
                this->set_structures(sl.load_outcar(url.toStdString()));
                this->update_graph();
            } else {
                QMessageBox::critical(this, tr("Failed to load file"), tr("Could not load file. Did you try to load this file from a network drive? This is not supported.") );
                //statusBar()->showMessage("Error loading file.");
                return;
            }
        }
    } else {
        //statusBar()->showMessage("Could not identify dropped format. Ignoring...");
    }
}

/**
 * @brief      Handles drag move event
 *
 * @param      event  The event
 */
void AnalysisGeometryOptimization::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief      Handles event when object is dragged outside window
 *
 * @param      event  The event
 */
void AnalysisGeometryOptimization::dragLeaveEvent(QDragLeaveEvent *event) {
    event->accept();
}
