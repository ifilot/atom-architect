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
#include <QVector>

#include "anaglyph_widget.h"
#include "structure_analysis.h"
#include "mainwindow.h"
#include "../data/structure_loader.h"
#include "../data/neb_calculation_loader.h"
#include "structure_info_widget.h"
#include "../data/structure_saver.h"
#include "toolbar.h"

QT_BEGIN_NAMESPACE
/**
 * @brief QSlider class.
 */
class QSlider;
/**
 * @brief QPushButton class.
 */
class QPushButton;
QT_END_NAMESPACE

/**
 * @brief MainWindow class.
 */
class MainWindow; // forward declaration to avoid circular dependencies

/**
 * @brief InterfaceWindow class.
 */
class InterfaceWindow : public QWidget {
    Q_OBJECT

private:
    MainWindow *mainWindow;

    AnaglyphWidget *anaglyph_widget;
    QLabel *interaction_label;
    QLabel *selection_label;
    StructureInfoWidget *structure_info_widget;
    StructureAnalysis *structureAnalysis;

    ToolBarWidget *editor_toolbar;
    ToolBarWidget *analysis_toolbar;

    StructureLoader structure_loader;
    StructureSaver structure_saver;

    size_t structure_stack_pointer = 0;
    std::vector<std::shared_ptr<Structure>> structure_stack;

    QWidget *editor_panel_ = nullptr;
    QWidget *analysis_panel_ = nullptr;
    bool editor_panel_active_ = true;
    QTimer *active_panel_timer_ = nullptr;

    QVector<QAction*> editor_shortcut_actions_;
    QVector<QAction*> analysis_shortcut_actions_;

/**
 * @brief set_active_panel.
 *
 * @param editor_active Parameter editor_active.
 */
    void set_active_panel(bool editor_active);
/**
 * @brief update_active_panel_from_cursor.
 *
 */
    void update_active_panel_from_cursor();

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
 * @brief open_editor_file.
 *
 */
    void open_editor_file();
/**
 * @brief open_analysis_file.
 *
 */
    void open_analysis_file();
/**
 * @brief open_analysis_neb_calculation.
 *
 */
    void open_analysis_neb_calculation();

    /**
     * @brief      Saves a file.
     *
     * @param[in]  filename  The filename
     */
    void save_file(const QString& filename);
/**
 * @brief save_editor_file.
 *
 */
    void save_editor_file();

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
/**
 * @brief emit.
 *
 * @param param Parameter param.
 */
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

    /**
     * @brief Copy structure from GeometryAnalysis window to Editor
     */
    void load_structure_from_geometry_analysis();

signals:
    /**
     * @brief A new file is loaded into the window
     */
    void new_file_loaded();

    /**
     * @brief      Message statusbar
     *
     * @param[in]  message  The message
     */
    void signal_message_statusbar(const QString& message);
};
