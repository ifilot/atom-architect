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

#pragma once

#include <QWidget>
#include <QSlider>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QStyle>
#include <QKeyEvent>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QSplitter>
#include <QInputDialog>

#include "anaglyph_widget.h"
#include "mainwindow.h"
#include "../data/structure_loader.h"
#include "structure_info_widget.h"
#include "../data/structure_saver.h"
#include "toolbar.h"

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE

class MainWindow; // forward declaration to avoid circular dependencies

class InterfaceWindow : public QWidget {
    Q_OBJECT

private:
    MainWindow *mainWindow;

    AnaglyphWidget *anaglyph_widget;
    QLabel *interaction_label;
    QLabel *selection_label;
    StructureInfoWidget *structure_info_widget;

    ToolBarWidget *toolbar;

    StructureLoader structure_loader;
    StructureSaver structure_saver;

    size_t structure_stack_pointer = 0;
    std::vector<std::shared_ptr<Structure>> structure_stack;

public:
    /**
     * @brief      Constructs the object.
     *
     * @param      mw    pointer to MainWindow object
     */
    InterfaceWindow(MainWindow *mw);

    /**
     * @brief      Get pointer to anaglyph widget
     *
     * @return     The anaglyph widget.
     */
    inline AnaglyphWidget* get_anaglyph_widget() {
        return this->anaglyph_widget;
    }

private:


protected:
    /**
     * @brief      Button press event
     *
     * @param      event  The event
     */
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

public slots:
    /**
     * @brief      Opens a file.
     *
     * @param[in]  filename  The filename
     *
     * @return     loading time of object in seconds
     */
    void open_file(const QString& filename);

    /**
     * @brief      Saves a file.
     *
     * @param[in]  filename  The filename
     */
    void save_file(const QString& filename);

    /**
     * @brief      Sets the camera align.
     *
     * @param      action  The action
     */
    void set_camera_align(QAction* action);

    /**
     * @brief      Sets the camera mode (orthogonal or perspective).
     *
     * @param      action  The action
     */
    void set_camera_mode(QAction* action);

    /**
     * @brief      Select all atoms
     */
    void select_all_atoms();

    /**
     * @brief      Deselect all atoms
     */
    void deselect_all_atoms();

    /**
     * @brief      Invert selection
     */
    void invert_selection();

    /**
     * @brief      Set selected atoms to frozen state
     */
    void set_frozen();

    /**
     * @brief      Set selected atoms to unfrozen state
     */
    void set_unfrozen();

private slots:
    /**
     * @brief      Loads a default structure file.
     */
    void load_default_file();

    /**
     * @brief      Add a fragment to the selection
     */
    void add_fragment();

    /**
     * @brief      Update the inform label
     *
     * @param[in]  text  The text
     */
    void update_interaction_label(const QString& text);

    /**
     * @brief      Update the inform label
     *
     * @param[in]  text  The text
     */
    void update_selection_label(const QString& text);

    /**
     * @brief      Propagate a message to the statusbar
     *
     * @param[in]  message  The message
     */
    void propagate_message_statusbar(const QString& message) {
        emit(signal_message_statusbar(message));
    }

    /**
     * @brief Grab the latest structure and push it to the stack and
     *        create a new copy on the stack
     */
    void push_structure();

    /**
     * @brief Increment the stack pointer
     */
    void increment_structure_stack_pointer();

    /**
     * @brief Decrement the stack pointer
     */
    void decrement_structure_stack_pointer();

signals:
    void new_file_loaded();

    /**
     * @brief      Message statusbar
     *
     * @param[in]  message  The message
     */
    void signal_message_statusbar(const QString& message);
};
