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

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QString>
#include <QtWidgets/QApplication>
#include <QFileInfo>
#include <QMimeData>
#include <QTimer>
#include <QStringList>

#include "interface_window.h"
#include "analysis_geometry_optimization.h"
#include "analysis_neb.h"
#include "logwindow.h"

#include "../config.h"

class InterfaceWindow; // forward declaration to avoid circular dependencies

/**
 * @brief      Class for main window.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    InterfaceWindow* interface_window;
    QLabel* statusbar_projection_icon;
    QTimer* statusbar_timer;

    // storage for log messages
    std::shared_ptr<QStringList> log_messages;

    // window for log messages
    std::unique_ptr<LogWindow> log_window;

public:
    /**
     * @brief      Constructs the object.
     */
    MainWindow(const std::shared_ptr<QStringList> _log_messages,
               QWidget *parent = nullptr);

    /**
     * @brief Parse command line arguments
     */
    void set_cli_parser(const QCommandLineParser& cli_parser);

protected:
    void moveEvent(QMoveEvent*) Q_DECL_OVERRIDE;

private slots:
    /**
     * @brief      Open a new object file
     */
    void open();

    /**
     * @brief      Open a new object file
     */
    void save();

    /**
     * @brief      Close the application
     */
    void exit();

    /**
     * @brief      Display about menu
     */
    void about();

    /**
     * @brief      Opens an analysis geometry optimization window.
     */
    void open_analysis_geometry_optimization_window();

    /**
     * @brief      Opens an analysis geometry optimization window.
     */
    void open_analysis_neb_window();

    /**
     * @brief      Set stereo projection
     */
    void set_stereo(QString fragment_shader);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    /**
     * @brief      Show a message on the statusbar
     *
     * @param[in]  message  The message
     */
    void show_message_statusbar(const QString& message);

    /**
     * @brief      Clear statusbar when timer runs out
     */
    void statusbar_timeout();

    /**
     * @brief Show an about window
     */
    void slot_debug_log();

private:
    /**
     * @brief      Loads a theme.
     */
    void load_theme();
};
