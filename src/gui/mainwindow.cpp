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

#include "mainwindow.h"

/**
 * @brief      Class for main window.
 */
MainWindow::MainWindow(const std::shared_ptr<QStringList> _log_messages,
                       QWidget *parent)
    : QMainWindow(parent),
    log_messages(_log_messages) {

    qDebug() << "Constructing Main Window";

    // log window
    this->log_window = std::make_unique<LogWindow>(this->log_messages);

    // dropdown menu bar
    QMenuBar *menuBar = new QMenuBar;

    // add drop-down menus
    QMenu *menu_file = menuBar->addMenu(tr("&File"));
    QMenu *menu_help = menuBar->addMenu(tr("&Help"));

    // actions for file menu
    QAction *action_quit = new QAction(menu_file);

    // actions for help menu
    QAction *action_about = new QAction(menu_help);

    // debug log
    QAction *action_debug_log = new QAction(menu_help);
    action_debug_log->setText(tr("Debug Log"));
    action_debug_log ->setShortcut(Qt::Key_F2);
    menu_help->addAction(action_debug_log);
    connect(action_debug_log, &QAction::triggered, this, &MainWindow::slot_debug_log);

    // create actions for file menu
    action_quit->setText(tr("Quit"));
    action_quit->setShortcuts({
        QKeySequence::Quit,                     // platform default (Alt+F4 on Windows)
        QKeySequence(Qt::CTRL | Qt::Key_Q),     // common on Linux / terminals
        QKeySequence(Qt::CTRL | Qt::Key_W)      // Windows-friendly close
    });
    // action_quit->setIcon(QIcon(":/assets/icon/close.png"));

    // create actions for about menu
    action_about->setText(tr("About"));
    // action_about->setIcon(QIcon(":/assets/icon/info.png"));

    // add actions to file menu
    menu_file->addAction(action_quit);

    // add actions to help menu
    menu_help->addAction(action_about);

    // connect actions file menu
    connect(action_quit, &QAction::triggered, this, &MainWindow::exit);

    // connect actions about menu
    connect(action_about, &QAction::triggered, this, &MainWindow::about);

    setMenuBar(menuBar);
    this->interface_window = new InterfaceWindow(this);
    setCentralWidget(this->interface_window);
    connect(this->interface_window, SIGNAL(signal_message_statusbar(const QString&)), this, SLOT(show_message_statusbar(const QString&)));
    this->statusbar_timer = new QTimer(this);
    connect(this->statusbar_timer, SIGNAL(timeout()), this, SLOT(statusbar_timeout()));

    // add projection icon to status bar
    this->statusbar_projection_icon = new QLabel();
    this->statusbar_projection_icon->setFixedSize(16, 16);
    this->statusbar_projection_icon->setPixmap(QPixmap(":/assets/icon/two_dimensional_32.png").scaled(16, 16));
    statusBar()->addPermanentWidget(this->statusbar_projection_icon);

    // display status message
    statusBar()->showMessage(QString(PROGRAM_NAME) + " " + QString(PROGRAM_VERSION));
    this->statusbar_timer->start(1000);

    // set icon
    setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));

    // allow dropping of data files
    setAcceptDrops(true);

    // load UI theme
    // this->load_theme();

    // set Window properties
    this->setWindowTitle(QString(PROGRAM_NAME) + " " + QString(PROGRAM_VERSION));
    this->resize(1280,960);

    qDebug() << "Done building MainWindow";
}

/**
 * @brief Parse command line arguments
 */
void MainWindow::set_cli_parser(const QCommandLineParser& cli_parser) {
    if(!cli_parser.value("o").isEmpty()) {
        QString filename = cli_parser.value("o");
        qDebug() << "Received CLI '-o': " << filename;
        this->interface_window->open_file(filename);
        statusBar()->showMessage("Loaded " + filename + ".");
        this->setWindowTitle(QFileInfo(filename).fileName() + " - " + QString(PROGRAM_NAME));
    }
}

/**
 * @brief      Open a new object file
 */
void MainWindow::open()
{
    QSettings settings;

    // Default to last directory, or user's home directory
    const QString startDir = settings.value(
        "ui/lastOpenDir",
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
    ).toString();

    const QString filters =
        tr("All supported files (*.geo *.xyz *.yaml *.yml *.vasp OUTCAR* CONTCAR* POSCAR*);;"
           "VASP files (*.vasp POSCAR* CONTCAR* OUTCAR*);;"
           "YAML frequency files (*.yaml *.yml);;"
           "ADF geometry files (*.geo);;"
           "XYZ files (*.xyz)");

    const QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open file"),
        startDir,
        filters
    );

    if (filename.isEmpty()) {
        return;
    }

    // Remember directory for next time
    settings.setValue(
        "ui/lastOpenDir",
        QFileInfo(filename).absolutePath()
    );

    interface_window->open_file(filename);

    statusBar()->showMessage(
        tr("Loaded %1.").arg(QFileInfo(filename).fileName())
    );

    setWindowTitle(
        QString("%1 - %2")
            .arg(QFileInfo(filename).fileName())
            .arg(PROGRAM_NAME)
    );
}

/**
 * @brief      Open a new object file
 */
void MainWindow::save() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("POSCAR"));

    if(filename.isEmpty()) {
        return;
    }

    // display load time
    this->interface_window->save_file(filename);
    statusBar()->showMessage("Save to " + filename + ".");
}

/**
 * @brief      Close the application
 */
void MainWindow::exit() {
    QMessageBox msgBox;
    msgBox.setText("Exit program.");
    msgBox.setStyleSheet("QLabel{min-width: 350px; font-weight: normal;}");
    msgBox.setInformativeText("Are you sure you want to quit? Your progress will be <b>unsaved</b>.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));
    int ret = msgBox.exec();

    switch (ret) {
      case QMessageBox::Ok:
          QApplication::quit();
          return;
      case QMessageBox::Cancel:
          return;
      default:
          // should never be reached
          return;
    }
}

/**
 * @brief      Display about menu
 */
void MainWindow::about() {
    QMessageBox message_box;
    message_box.setStyleSheet("QLabel{min-width: 250px; font-weight: normal;}");
    message_box.setText(PROGRAM_NAME
                        " version "
                        PROGRAM_VERSION
                        ".\n\nMaintainer:\n"
                        "Ivo Filot <i.a.w.filot@tue.nl>\n\n"
                        "Authors:\n"
                        "Ivo Filot\n"
                        "Joeri van Limpt\n\n"
                        PROGRAM_NAME " is licensed under the GPLv3 license.\n\n"
                        PROGRAM_NAME " is dynamically linked to Qt, which is licensed under LGPLv3.\n");
    message_box.setIcon(QMessageBox::Information);
    message_box.setWindowTitle("About " PROGRAM_NAME);
    message_box.setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));
    message_box.setIconPixmap(QPixmap(":/assets/icon/atom_architect_256.ico"));
    message_box.exec();
}

/**
 * @brief      Set stereo projection
 */
void MainWindow::set_stereo(QString stereo_name) {
    this->interface_window->get_anaglyph_widget()->set_stereo(stereo_name);

    // set icon
    QString icon_name;
    if (stereo_name.startsWith("stereo")) {
        icon_name = ":/assets/icon/" + stereo_name.replace("stereo_", "") + "_32.png";
    } else {
        icon_name = ":/assets/icon/two_dimensional_32.png";
    }

    QPixmap pixmap(icon_name);
    this->statusbar_projection_icon->setPixmap(pixmap.scaled(16, 16));
}

/**
 * @brief moveEvent.
 *
 * @param QMoveEvent Parameter QMoveEvent.
 */
void MainWindow::moveEvent(QMoveEvent* event) {
    this->interface_window->get_anaglyph_widget()->window_move_event();

    // this is just to silence a warning message, we are not using event here, but this is the callback form
    (void)event;
}

/**
 * @brief dragEnterEvent.
 *
 * @param event Parameter event.
 */
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief dropEvent.
 *
 * @param event Parameter event.
 */
void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        const QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size() && i < 32; ++i) {
            QString filepath = urlList.at(i).toLocalFile();

            // Fallback for malformed URL payloads that do not expose local file directly.
            if(filepath.isEmpty()) {
                filepath = urlList.at(i).path();
            }

            #ifdef Q_OS_WIN
            // remove leading / on Windows path; this sometimes causes problems
            QFileInfo check_file_win(filepath);
            if(!check_file_win.exists()) {
                if(!filepath.isEmpty() && filepath[0] == '/') {
                    filepath = filepath.remove(0,1);
                }
            }
            #endif

            // check if file exists, else show error message
            QFileInfo check_file(filepath);
            if(check_file.exists() && check_file.isFile()) {
                this->interface_window->open_file(filepath);

                if(i == 0) {
                    this->setWindowTitle(
                        QString("%1 - %2")
                            .arg(check_file.fileName())
                            .arg(PROGRAM_NAME)
                    );
                }
            } else {
                QMessageBox::critical(this, tr("Failed to load file"), tr("Could not load file. Did you try to load this file from a network drive? This is not supported.") );
                statusBar()->showMessage("Error loading file.");
                return;
            }
        }

        // show message after loading
        if(urlList.size() == 1) {
            const QString loaded_file = !urlList.at(0).toLocalFile().isEmpty()
                ? QFileInfo(urlList.at(0).toLocalFile()).fileName()
                : urlList.at(0).path();
            statusBar()->showMessage("Loaded " + loaded_file + ".");
        } else {
            statusBar()->showMessage("Loaded files.");
        }
    } else {
        statusBar()->showMessage("Could not identify dropped format. Ignoring...");
    }
}

/**
 * @brief dragMoveEvent.
 *
 * @param event Parameter event.
 */
void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief dragLeaveEvent.
 *
 * @param event Parameter event.
 */
void MainWindow::dragLeaveEvent(QDragLeaveEvent *event) {
    event->accept();
}

/**
 * @brief      Clear statusbar when timer runs out
 */
void MainWindow::statusbar_timeout() {
    statusBar()->showMessage("");
}

/**
 * @brief      Loads a theme.
 */
void MainWindow::load_theme() {
    // load theme
    QFile f(":/assets/themes/darkorange/darkorange.qss");
    if (!f.exists())   {
        throw std::runtime_error("Cannot open theme file.");
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
}

/**
* @brief      Show a message on the statusbar
*
* @param[in]  message  The message
*/
void MainWindow::show_message_statusbar(const QString& message) {
    statusBar()->showMessage(message);
    this->statusbar_timer->start(1000);
}

/**
 * @brief Show an about window
 */
void MainWindow::slot_debug_log() {
    this->log_window->show();
}
