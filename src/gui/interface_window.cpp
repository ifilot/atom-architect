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
#include "interface_window.h"

/**
 * @brief      Constructs the object.
 *
 * @param      mw    pointer to MainWindow object
 */
InterfaceWindow::InterfaceWindow(MainWindow *mw)
    : mainWindow(mw)
{
    // ============================================================
    // Root layout
    // ============================================================
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    setLayout(rootLayout);

    // ============================================================
    // Toolbar
    // ============================================================
    toolbar = new ToolBarWidget(this);
    toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    rootLayout->addWidget(toolbar);

    // ============================================================
    // Main horizontal splitter
    // ============================================================
    QSplitter *hSplitter = new QSplitter(Qt::Horizontal, this);
    rootLayout->addWidget(hSplitter);

    // ============================================================
    // LEFT COLUMN (Editor + Geometry Optimization Viewer)
    // ============================================================
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
    hSplitter->addWidget(leftSplitter);

    // ------------------------------------------------------------
    // Editor panel (TOP-LEFT)
    // ------------------------------------------------------------
    QWidget *editorPanel = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);

    QLabel *editorLabel = new QLabel("EDITOR", editorPanel);
    editorLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    editorLayout->addWidget(editorLabel);

    anaglyph_widget = new AnaglyphWidget(this);
    anaglyph_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    editorLayout->addWidget(anaglyph_widget);

    // Interaction / selection labels
    QWidget *labelWidget = new QWidget(this);
    QHBoxLayout *labelLayout = new QHBoxLayout(labelWidget);

    selection_label = new QLabel("<br>", labelWidget);
    interaction_label = new QLabel("", labelWidget);

    labelLayout->addWidget(selection_label);
    labelLayout->addWidget(interaction_label);
    labelLayout->addStretch();

    editorLayout->addWidget(labelWidget);

    leftSplitter->addWidget(editorPanel);

    // ------------------------------------------------------------
    // Geometry optimization viewer (BOTTOM-LEFT)
    // ------------------------------------------------------------
    geometryOptimization = new AnalysisGeometryOptimization(this);

    leftSplitter->addWidget(geometryOptimization->viewer());

    // ============================================================
    // RIGHT COLUMN (Structure info + Optimization graph)
    // ============================================================
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);
    hSplitter->addWidget(rightSplitter);

    // ------------------------------------------------------------
    // Structure info (TOP-RIGHT)
    // ------------------------------------------------------------
    structure_info_widget = new StructureInfoWidget(this);
    structure_info_widget->set_anaglyph_widget(anaglyph_widget);
    structure_info_widget->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);

    rightSplitter->addWidget(structure_info_widget);

    // ------------------------------------------------------------
    // Optimization graph (BOTTOM-RIGHT)
    // ------------------------------------------------------------
    rightSplitter->addWidget(geometryOptimization->graph());

    // ============================================================
    // Initial 50/50/50/50 layout (relative to MainWindow size)
    // ============================================================

    const int totalWidth  = width();
    const int totalHeight = height();

    // Left / Right
    hSplitter->setSizes({ totalWidth / 2, totalWidth / 2 });

    // Top / Bottom (both columns)
    leftSplitter->setSizes({ totalHeight / 2, totalHeight / 2 });
    rightSplitter->setSizes({ totalHeight / 2, totalHeight / 2 });

    // ============================================================
    // Connections (unchanged behavior)
    // ============================================================
    connect(this, SIGNAL(new_file_loaded()),
            structure_info_widget, SLOT(reset()));

    connect(anaglyph_widget, SIGNAL(opengl_ready()),
            this, SLOT(load_default_file()));

    connect(toolbar->get_action("toggle_periodicity_xy"),
            SIGNAL(triggered()),
            anaglyph_widget, SLOT(toggle_periodicity_xy()));

    connect(toolbar->get_action("toggle_periodicity_z"),
            SIGNAL(triggered()),
            anaglyph_widget, SLOT(toggle_periodicity_z()));

    connect(toolbar->get_action("add_fragment"),
            SIGNAL(triggered()),
            this, SLOT(add_fragment()));

    connect(anaglyph_widget, SIGNAL(signal_interaction_message(const QString&)),
            this, SLOT(update_interaction_label(const QString&)));

    connect(anaglyph_widget, SIGNAL(signal_selection_message(const QString&)),
            this, SLOT(update_selection_label(const QString&)));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_selection_message(const QString&)),
            this, SLOT(update_selection_label(const QString&)));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_message_statusbar(const QString&)),
            this, SLOT(propagate_message_statusbar(const QString&)));

    connect(structure_info_widget->get_fragment_selector(),
            SIGNAL(signal_new_fragment(const Fragment&)),
            anaglyph_widget->get_user_action().get(),
            SLOT(set_fragment(const Fragment&)));

    connect(geometryOptimization->viewer(), SIGNAL(edit_requested()),
            this, SLOT(load_structure_from_geometry_analysis()));

    // ============================================================
    // Default fragment
    // ============================================================
    anaglyph_widget->get_user_action()->set_fragment(
        structure_info_widget->get_fragment_selector()->get_current_fragment());

    // ============================================================
    // Structure stack
    // ============================================================
    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_push_structure()),
            this, SLOT(push_structure()));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_increment_structure_stack_pointer()),
            this, SLOT(increment_structure_stack_pointer()));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_decrement_structure_stack_pointer()),
            this, SLOT(decrement_structure_stack_pointer()));
}

/**
 * @brief      Button press event
 *
 * @param      event  The event
 */
void InterfaceWindow::keyPressEvent(QKeyEvent* event) {
    // Only forward when the 3D viewport is active
    if(this->anaglyph_widget->hasFocus()) {
        if(this->anaglyph_widget
               ->get_user_action()
               ->handle_key(event)) {

            event->accept();
            return;
        }
    }

    // Let Qt handle it (menus, shortcuts, text input, etc.)
    QWidget::keyPressEvent(event);
}

/**
 * @brief      Opens a file.
 *
 * @param[in]  filename  The filename
 *
 * @return     loading time of object in seconds
 */
void InterfaceWindow::open_file(const QString& filename)
{
    qDebug() << "Opening file:" << filename;

    // ------------------------------------------------------------
    // Case 1: Geometry optimization (OUTCAR)
    // ------------------------------------------------------------
    if (filename.contains("OUTCAR", Qt::CaseInsensitive)) {

        StructureLoader sl;
        std::vector<std::shared_ptr<Structure>> structures;

        try {
            structures = sl.load_outcar(filename.toStdString());
        } catch (const std::exception& e) {
            QMessageBox::critical(this,
                tr("Exception encountered"),
                tr(e.what()));
            return;
        }

        if (structures.empty()) {
            QMessageBox::warning(this,
                tr("Empty optimization"),
                tr("No structures found in OUTCAR."));
            return;
        }

        // ---- Update geometry optimization panels ----
        std::vector<std::shared_ptr<Structure>> geometry_structures;
        geometry_structures.reserve(structures.size());

        for (const auto& s : structures) {
            geometry_structures.push_back(s->clone_for_view());
        }

        geometryOptimization->set_structures(geometry_structures);

        // ---- Also sync editor + info to first structure ----
        structure_stack.clear();
        structure_stack.push_back(structures.front());
        structure_stack_pointer = 0;

        emit new_file_loaded();

        anaglyph_widget->set_structure(structures.front());
        structure_info_widget->set_structure(structures.front());

        return;
    }

    // ------------------------------------------------------------
    // Case 2: Single structure file (original behavior)
    // ------------------------------------------------------------
    std::shared_ptr<Structure> structure;

    try {
        structure = structure_loader.load_file(filename.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this,
            tr("Exception encountered"),
            tr(e.what()));
        return;
    }

    structure_stack.clear();
    structure_stack.push_back(structure);
    structure_stack_pointer = 0;

    emit new_file_loaded();

    anaglyph_widget->set_structure(structure);
    structure_info_widget->set_structure(structure);
}

/**
 * @brief      Saves a file.
 *
 * @param[in]  filename  The filename
 */
void InterfaceWindow::save_file(const QString& filename) {
    this->structure_saver.save_poscar(this->anaglyph_widget->get_structure(), filename.toStdString());
}

/**
 * @brief      Sets the camera align.
 *
 * @param      action  The action
 */
void InterfaceWindow::set_camera_align(QAction* action) {
    this->anaglyph_widget->get_user_action()->set_camera_alignment(action->data().toInt());
}

/**
 * @brief      Sets the camera mode (orthogonal or perspective).
 *
 * @param      action  The action
 */
void InterfaceWindow::set_camera_mode(QAction* action) {
    this->anaglyph_widget->get_user_action()->set_camera_mode(action->data().toInt());
}

/**
 * @brief      Loads a default structure file.
 */
void InterfaceWindow::load_default_file() {
    // do not load default file if a file is already loaded (via CLI)
    if(this->structure_stack.size() != 0) {
        return;
    }

    qDebug() << "Opening default file";
    const std::string filename = "OUTCAR";
    QTemporaryDir tmp_dir;
    QFile::copy(":/assets/structures/" + tr(filename.c_str()), tmp_dir.path() + "/" + filename.c_str());
    this->open_file(tmp_dir.path() + "/" + filename.c_str());
}

/**
 * @brief      Add a fragment to the selection
 */
void InterfaceWindow::add_fragment() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_add_fragment();
}

/**
 * @brief      Select all atoms
 */
void InterfaceWindow::select_all_atoms() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_select_all();
}

/**
 * @brief      Deselect all atoms
 */
void InterfaceWindow::deselect_all_atoms() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_deselect_all();
}

/**
 * @brief      Invert selection
 */
void InterfaceWindow::invert_selection() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_invert_selection();
}

/**
 * @brief      Set selected atoms to frozen state
 */
void InterfaceWindow::set_frozen() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_set_frozen();
}
/**
 * @brief      Set selected atoms to unfrozen state
 */
void InterfaceWindow::set_unfrozen() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_set_unfrozen();
}

/**
 * @brief      Update the inform label
 *
 * @param[in]  text  The text
 */
void InterfaceWindow::update_interaction_label(const QString& text) {
    this->interaction_label->setText(text);
}

/**
 * @brief      Update the selection label
 *
 * If the list of atoms exceeds 120 characters, truncate the list and show
 * cdots
 *
 * @param[in]  text  The text
 */
void InterfaceWindow::update_selection_label(const QString& text) {
    QStringList atomlists = text.split("<br>");
    QStringList pieces1 = atomlists[0].split(";");
    QStringList pieces2 = atomlists[1].split(";");

    // maximum size of the atomlist string
    static const unsigned int sz = 120;

    if(pieces1[0].length() > sz) {
        pieces1[0] = pieces1[0].left(sz) + "...";

    }

    if(pieces2[0].length() > sz) {
        pieces2[0] = pieces2[0].left(sz) + "...";

    }

    atomlists[0] = pieces1.join(";");
    atomlists[1] = pieces2.join(";");

    this->selection_label->setText(atomlists.join("<br>"));
}

/**
 * @brief Grab the latest structure and push it to the stack and
 *        create a new copy on the stack
 */
void InterfaceWindow::push_structure() {
    // Current structure as used by the renderer/user action
    auto current = this->anaglyph_widget->get_structure();
    if(!current) return;

    // If we undid before, drop redo history
    this->structure_stack.resize(this->structure_stack_pointer + 1);

    // Push a NEW snapshot of the current state
    this->structure_stack.push_back(std::make_shared<Structure>(*current));
    this->structure_stack_pointer++;

    // Make the newly pushed snapshot the active one everywhere
    this->anaglyph_widget->set_structure_conservative(this->structure_stack[this->structure_stack_pointer]);
    this->structure_info_widget->set_structure(this->structure_stack[this->structure_stack_pointer]);

    // Also ensure UserAction uses the same shared_ptr (critical!)
    this->anaglyph_widget->get_user_action()->set_structure(this->structure_stack[this->structure_stack_pointer]);
}

/**
 * @brief Increment the stack pointer
 */
void InterfaceWindow::increment_structure_stack_pointer() {
    if(this->structure_stack_pointer < (this->structure_stack.size() - 1)) {
        this->structure_stack_pointer++;

        auto s = this->structure_stack[this->structure_stack_pointer];
        this->anaglyph_widget->set_structure_conservative(s);
        this->structure_info_widget->set_structure(s);
        this->anaglyph_widget->get_user_action()->set_structure(s);
    }
}

/**
 * @brief Decrement the stack pointer
 */
void InterfaceWindow::decrement_structure_stack_pointer() {
    if(this->structure_stack_pointer > 0) {
        this->structure_stack_pointer--;

        auto s = this->structure_stack[this->structure_stack_pointer];
        this->anaglyph_widget->set_structure_conservative(s);
        this->structure_info_widget->set_structure(s);
        this->anaglyph_widget->get_user_action()->set_structure(s);
    }
}

/**
 * @brief Copy structure from GeometryAnalysis window to Editor
 */
void InterfaceWindow::load_structure_from_geometry_analysis() {
    this->anaglyph_widget->set_structure(
        this->geometryOptimization->viewer()->get_anaglyph_widget()->get_structure()->clone_for_view()
    );
}