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
    : mainWindow(mw) {

    // set layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *container = new QHBoxLayout;

    // add toolbar
    this->toolbar = new ToolBarWidget();
    container->addWidget(this->toolbar);
    this->toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // add Splitter
    qDebug() << "Add splitter";
    QSplitter *splitter = new QSplitter();
    container->addWidget(splitter);

    // add anaglyph widget
    qDebug() << "Add Anaglyph widget";
    QWidget *container2 = new QWidget();
    splitter->addWidget(container2);
    QVBoxLayout* layout = new QVBoxLayout();
    container2->setLayout(layout);
    this->anaglyph_widget = new AnaglyphWidget();
    this->interaction_label = new QLabel("");
    this->selection_label = new QLabel("<br>");
    layout->addWidget(this->anaglyph_widget);
    QWidget* labelwidget = new QWidget(this);
    layout->addWidget(labelwidget);
    QHBoxLayout* labelwidgetlayout = new QHBoxLayout(labelwidget);
    labelwidget->setLayout(labelwidgetlayout);
    labelwidgetlayout->addWidget(this->selection_label);
    labelwidgetlayout->addWidget(this->interaction_label);
    this->anaglyph_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // add structure info widget
    qDebug() << "Add Structure Info widget";
    this->structure_info_widget = new StructureInfoWidget();
    this->structure_info_widget->set_anaglyph_widget(this->anaglyph_widget);
    this->structure_info_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    splitter->addWidget(this->structure_info_widget);

    // put everything as central widget
    qDebug() << "Put everything as central widget";
    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);
    this->setLayout(mainLayout);

    // connect functions
    connect(this, SIGNAL(new_file_loaded()), this->structure_info_widget, SLOT(reset()));
    connect(this->anaglyph_widget, SIGNAL(opengl_ready()), this, SLOT(load_default_file()));

    // connect toolbar
    qDebug() << "Connect toolbar";
    connect(this->toolbar->get_action("toggle_periodicity_xy"), SIGNAL(triggered()), this->anaglyph_widget, SLOT(toggle_periodicity_xy()));
    connect(this->toolbar->get_action("toggle_periodicity_z"), SIGNAL(triggered()), this->anaglyph_widget, SLOT(toggle_periodicity_z()));
    connect(this->toolbar->get_action("add_fragment"), SIGNAL(triggered()), this, SLOT(add_fragment()));
    connect(this->anaglyph_widget, SIGNAL(signal_interaction_message(const QString&)), this, SLOT(update_interaction_label(const QString&)));
    connect(this->anaglyph_widget, SIGNAL(signal_selection_message(const QString&)), this, SLOT(update_selection_label(const QString&)));
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_selection_message(const QString&)), this, SLOT(update_selection_label(const QString&)));
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_message_statusbar(const QString&)), this, SLOT(propagate_message_statusbar(const QString&)));
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_update_structure_info()), this->structure_info_widget, SLOT(update()));
    connect(this->structure_info_widget->get_fragment_selector(), SIGNAL(signal_new_fragment(const Fragment&)), this->anaglyph_widget->get_user_action().get(), SLOT(set_fragment(const Fragment&)));

    // set default fragment
    qDebug() << "Set default fragment";
    this->anaglyph_widget->get_user_action()->set_fragment(this->structure_info_widget->get_fragment_selector()->get_current_fragment());

    // connect structure stack functions
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_push_structure()), this, SLOT(push_structure()));
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_increment_structure_stack_pointer()), this, SLOT(increment_structure_stack_pointer()));
    connect(this->anaglyph_widget->get_user_action().get(), SIGNAL(signal_decrement_structure_stack_pointer()), this, SLOT(decrement_structure_stack_pointer()));
}

/**
 * @brief      Button press event
 *
 * @param      event  The event
 */
void InterfaceWindow::keyPressEvent(QKeyEvent *event) {
    // qDebug() << event->key();

    std::vector<unsigned int> key_listing = {
        Qt::Key_G, // grab atoms
        Qt::Key_R, // rotate atoms
        Qt::Key_X, // x-direction
        Qt::Key_Y, // y-direction
        Qt::Key_Z, // z-direction
        Qt::Key_F, // focus-direction
        Qt::Key_D, // deselect
        Qt::Key_I, // Invert
        Qt::Key_A, // add fragment
        Qt::Key_R, // replace atom
        Qt::Key_Delete // delete atoms
    };


    if(std::find(key_listing.begin(), key_listing.end(), event->key()) != key_listing.end()) {
        this->anaglyph_widget->get_user_action()->handle_key(event->key(), event->modifiers());
    }

}

/**
 * @brief      Opens a file.
 *
 * @param[in]  filename  The filename
 *
 * @return     loading time of object in seconds
 */
void InterfaceWindow::open_file(const QString& filename) {
    qDebug() << "Opening file: " << filename;

    std::shared_ptr<Structure> structure;
    try {
        structure = structure_loader.load_file(filename.toStdString());
    } catch(const std::exception& e) {
        QMessageBox::critical(this, tr("Exception encountered"), tr(e.what()) );
        return;
    }

    // clean the structure stack and set a new structure
    this->structure_stack.clear();
    this->structure_stack.push_back(structure);
    this->structure_stack_pointer = 0;

    emit(new_file_loaded());
    this->anaglyph_widget->set_structure(structure);
    this->structure_info_widget->set_structure(structure);
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
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_A, Qt::ShiftModifier);
}

/**
 * @brief      Select all atoms
 */
void InterfaceWindow::select_all_atoms() {
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_A, Qt::ControlModifier);
}

/**
 * @brief      Deselect all atoms
 */
void InterfaceWindow::deselect_all_atoms() {
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_D, Qt::ControlModifier);
}

/**
 * @brief      Invert selection
 */
void InterfaceWindow::invert_selection() {
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_I, Qt::ControlModifier);
}

/**
 * @brief      Set selected atoms to frozen state
 */
void InterfaceWindow::set_frozen() {
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_F, Qt::ControlModifier);
}

/**
 * @brief      Set selected atoms to unfrozen state
 */
void InterfaceWindow::set_unfrozen() {
    this->anaglyph_widget->get_user_action()->handle_key(Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
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
        pieces1[0] = pieces1[0].first(sz) + "...";

    }

    if(pieces2[0].length() > sz) {
        pieces2[0] = pieces2[0].first(sz) + "...";

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
    qDebug() << "Pushing latest structure to the stack ("
             << (size_t)this->structure_stack.back().get()
             << ")";

    // destroy everything "beyond" the stack pointer
    this->structure_stack.resize(this->structure_stack_pointer+1);

    this->structure_stack.push_back(
        std::make_shared<Structure>(*this->structure_stack.back()) // creates a copy
    );

    // increment the structure stack pointer
    this->structure_stack_pointer++;

    this->anaglyph_widget->set_structure_conservative(this->structure_stack.back());
    qDebug() << this->structure_stack.size() << ": " << (size_t)this->structure_stack.back().get();
}

/**
 * @brief Increment the stack pointer
 */
void InterfaceWindow::increment_structure_stack_pointer() {
    if(this->structure_stack_pointer < (this->structure_stack.size() - 1)) {
        this->structure_stack_pointer++;
        qDebug() << "Incrementing stack pointer";
        qDebug() << "New pointer value: " << this->structure_stack_pointer;
        this->anaglyph_widget->set_structure_conservative(this->structure_stack[this->structure_stack_pointer]);
    } else {
        qDebug() << "Ignoring stack pointer request; structure stack exchausted.";
    }
}

/**
 * @brief Decrement the stack pointer
 */
void InterfaceWindow::decrement_structure_stack_pointer() {
    if(this->structure_stack_pointer > 0) {
        this->structure_stack_pointer--;
        qDebug() << "Decrementing stack pointer";
        qDebug() << "New pointer value: " << this->structure_stack_pointer;
        this->anaglyph_widget->set_structure_conservative(this->structure_stack[this->structure_stack_pointer]);
    } else {
        qDebug() << "Ignoring stack pointer request; structure stack exchausted.";
    }
}
