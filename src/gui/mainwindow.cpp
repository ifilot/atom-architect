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
    QMenu *menu_view = menuBar->addMenu(tr("&View"));
    QMenu *menu_select = menuBar->addMenu(tr("&Select"));
    QMenu *menu_analysis = menuBar->addMenu(tr("&Analysis"));
    QMenu *menu_help = menuBar->addMenu(tr("&Help"));

    // actions for file menu
    QAction *action_open = new QAction(menu_file);
    QAction *action_save = new QAction(menu_file);
    QAction *action_quit = new QAction(menu_file);

    // actions for select menu
    QAction *action_select_all = new QAction(menu_select);
    QAction *action_deselect_all = new QAction(menu_select);
    QAction *action_invert_selection = new QAction(menu_select);
    QAction *action_set_frozen = new QAction(menu_select);
    QAction *action_set_unfrozen = new QAction(menu_select);

    // actions for projection menu
    QMenu *menu_camera = new QMenu(tr("Camera"));

    // camera alignment
    QMenu *menu_camera_align = new QMenu(tr("Align"));
    QAction *action_camera_default = new QAction(menu_camera_align);
    QAction *action_camera_top = new QAction(menu_camera_align);
    QAction *action_camera_bottom = new QAction(menu_camera_align);
    QAction *action_camera_left = new QAction(menu_camera_align);
    QAction *action_camera_right = new QAction(menu_camera_align);
    QAction *action_camera_front = new QAction(menu_camera_align);
    QAction *action_camera_back = new QAction(menu_camera_align);

    // camera modes
    QMenu *menu_camera_mode = new QMenu(tr("Mode"));
    QAction *action_camera_perspective = new QAction(menu_camera_mode);
    QAction *action_camera_orthographic = new QAction(menu_camera_mode);

    // camera projections
    QMenu *menu_projection = new QMenu(tr("Projection"));
    QAction *action_projection_two_dimensional = new QAction(menu_projection);
    QAction *action_projection_anaglyph_red_cyan = new QAction(menu_projection);

    // interlaced projections
    QMenu *menu_projection_interlaced = new QMenu(tr("Interlaced"));
    QAction *action_projection_interlaced_rows_lr = new QAction(menu_projection_interlaced);
    QAction *action_projection_interlaced_rows_rl = new QAction(menu_projection_interlaced);
    QAction *action_projection_interlaced_columns_lr = new QAction(menu_projection_interlaced);
    QAction *action_projection_interlaced_columns_rl = new QAction(menu_projection_interlaced);
    QAction *action_projection_interlaced_checkerboard_lr = new QAction(menu_projection_interlaced);
    QAction *action_projection_interlaced_checkerboard_rl = new QAction(menu_projection_interlaced);

    // actions for analysis menu
    QAction *action_analysis_optimization = new QAction(menu_analysis);
    QAction *action_analysis_neb = new QAction(menu_analysis);

    // actions for help menu
    QAction *action_about = new QAction(menu_help);

    // debug log
    QAction *action_debug_log = new QAction(menu_help);
    action_debug_log->setText(tr("Debug Log"));
    action_debug_log ->setShortcut(Qt::Key_F2);
    menu_help->addAction(action_debug_log);
    connect(action_debug_log, &QAction::triggered, this, &MainWindow::slot_debug_log);

    // create actions for file menu
    action_open->setText(tr("Open"));
    action_open->setShortcuts(QKeySequence::Open);
    // action_open->setIcon(QIcon(":/assets/icon/open.png"));
    action_save->setText(tr("Save"));
    action_save->setShortcuts(QKeySequence::Save);
    // action_save->setIcon(QIcon(":/assets/icon/save.png"));
    action_quit->setText(tr("Quit"));
    action_quit->setShortcuts({
        QKeySequence::Quit,                     // platform default (Alt+F4 on Windows)
        QKeySequence(Qt::CTRL | Qt::Key_Q),     // common on Linux / terminals
        QKeySequence(Qt::CTRL | Qt::Key_W)      // Windows-friendly close
    });
    // action_quit->setIcon(QIcon(":/assets/icon/close.png"));

    action_select_all->setText(tr("Select all atoms"));
    action_select_all->setShortcut(Qt::CTRL | Qt::Key_A);
    action_deselect_all->setText(tr("Deselect all atoms"));
    action_deselect_all->setShortcut(Qt::CTRL | Qt::Key_D);
    action_invert_selection->setText(tr("Invert selection"));
    action_invert_selection->setShortcut(Qt::CTRL | Qt::Key_I);
    action_set_frozen->setText(tr("Set frozen"));
    action_set_frozen->setShortcut(Qt::CTRL | Qt::Key_F);
    action_set_unfrozen->setText(tr("Set unfrozen"));
    action_set_unfrozen->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_F);

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
    action_camera_perspective->setText(tr("Perspective"));
    action_camera_perspective->setData(QVariant((int)CameraMode::PERSPECTIVE));
    action_camera_perspective->setShortcut(Qt::Key_5);
    action_camera_orthographic->setText(tr("Orthographic"));
    action_camera_orthographic->setData(QVariant((int)CameraMode::ORTHOGRAPHIC));
    action_camera_orthographic->setShortcut(Qt::CTRL | Qt::Key_5);
    action_projection_two_dimensional->setText(tr("Two-dimensional"));
    action_projection_anaglyph_red_cyan->setText(tr("Anaglyph (red/cyan)"));
    action_projection_interlaced_rows_lr->setText(tr("Interlaced rows (left first)"));
    action_projection_interlaced_rows_rl->setText(tr("Interlaced rows (right first)"));
    action_projection_interlaced_columns_lr->setText(tr("Interlaced columns (left first)"));
    action_projection_interlaced_columns_rl->setText(tr("Interlaced columns (right first)"));
    action_projection_interlaced_checkerboard_lr->setText(tr("Checkerboard (left first)"));
    action_projection_interlaced_checkerboard_rl->setText(tr("Checkerboard (right first)"));
    menu_projection_interlaced->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    action_projection_two_dimensional->setIcon(QIcon(":/assets/icon/two_dimensional_32.png"));
    action_projection_anaglyph_red_cyan->setIcon(QIcon(":/assets/icon/anaglyph_red_cyan_32.png"));
    action_projection_interlaced_rows_lr->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    action_projection_interlaced_rows_rl->setIcon(QIcon(":/assets/icon/interlaced_rows_rl_32.png"));
    action_projection_interlaced_columns_lr->setIcon(QIcon(":/assets/icon/interlaced_columns_lr_32.png"));
    action_projection_interlaced_columns_rl->setIcon(QIcon(":/assets/icon/interlaced_columns_rl_32.png"));
    action_projection_interlaced_checkerboard_lr->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_lr_32.png"));
    action_projection_interlaced_checkerboard_rl->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_rl_32.png"));

    // create actions for analysis menu
    action_analysis_optimization->setText(tr("Geometry optimization analysis"));
    action_analysis_optimization->setShortcut(Qt::Key_F5);
    action_analysis_neb->setText(tr("NEB analysis"));
    action_analysis_neb->setShortcut(Qt::Key_F6);

    // create actions for about menu
    action_about->setText(tr("About"));
    // action_about->setIcon(QIcon(":/assets/icon/info.png"));

    // add actions to file menu
    menu_file->addAction(action_open);
    menu_file->addAction(action_save);
    menu_file->addAction(action_quit);

    // add actions to select menu
    menu_select->addAction(action_select_all);
    menu_select->addAction(action_deselect_all);
    menu_select->addAction(action_invert_selection);
    menu_select->addSeparator();
    menu_select->addAction(action_set_frozen);
    menu_select->addAction(action_set_unfrozen);

    // add actions to projection menu
    menu_view->addMenu(menu_projection);
    menu_view->addMenu(menu_camera);
    menu_camera->addMenu(menu_camera_align);
    menu_camera_align->addAction(action_camera_default);
    menu_camera_align->addAction(action_camera_top);
    menu_camera_align->addAction(action_camera_bottom);
    menu_camera_align->addAction(action_camera_left);
    menu_camera_align->addAction(action_camera_right);
    menu_camera_align->addAction(action_camera_front);
    menu_camera_align->addAction(action_camera_back);
    menu_camera->addMenu(menu_camera_mode);
    menu_camera_mode->addAction(action_camera_perspective);
    menu_camera_mode->addAction(action_camera_orthographic);
    menu_projection->addAction(action_projection_two_dimensional);
    menu_projection->addAction(action_projection_anaglyph_red_cyan);
    menu_projection->addMenu(menu_projection_interlaced);
    menu_projection_interlaced->addAction(action_projection_interlaced_rows_lr);
    menu_projection_interlaced->addAction(action_projection_interlaced_rows_rl);
    menu_projection_interlaced->addAction(action_projection_interlaced_columns_lr);
    menu_projection_interlaced->addAction(action_projection_interlaced_columns_rl);
    menu_projection_interlaced->addAction(action_projection_interlaced_checkerboard_lr);
    menu_projection_interlaced->addAction(action_projection_interlaced_checkerboard_rl);

    // add actions for analysis menu
    menu_analysis->addAction(action_analysis_optimization);
    menu_analysis->addAction(action_analysis_neb);

    // add actions to help menu
    menu_help->addAction(action_about);

    // connect actions file menu
    connect(action_open, &QAction::triggered, this, &MainWindow::open);
    connect(action_save, &QAction::triggered, this, &MainWindow::save);
    connect(action_quit, &QAction::triggered, this, &MainWindow::exit);

    // connect actions projection menu (note; [this]{} is the idiomatic way by providing a functor - "this is the way")
    connect(action_projection_two_dimensional, &QAction::triggered, this, [this]{ MainWindow::set_stereo("no_stereo_flat"); });
    connect(action_projection_anaglyph_red_cyan, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_anaglyph_red_cyan"); });
    connect(action_projection_interlaced_rows_lr, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_rows_lr"); });
    connect(action_projection_interlaced_rows_rl, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_rows_rl"); });
    connect(action_projection_interlaced_columns_lr, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_columns_lr"); });
    connect(action_projection_interlaced_columns_rl, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_columns_rl"); });
    connect(action_projection_interlaced_checkerboard_lr, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_checkerboard_lr"); });
    connect(action_projection_interlaced_checkerboard_rl, &QAction::triggered, this, [this]{ MainWindow::set_stereo("stereo_interlaced_checkerboard_rl"); });

    // connect actions analysis window
    connect(action_analysis_optimization, &QAction::triggered, this, &MainWindow::open_analysis_geometry_optimization_window);
    connect(action_analysis_neb, &QAction::triggered, this, &MainWindow::open_analysis_neb_window);

    // connect actions about menu
    connect(action_about, &QAction::triggered, this, &MainWindow::about);

    setMenuBar(menuBar);
    this->interface_window = new InterfaceWindow(this);
    setCentralWidget(this->interface_window);
    connect(this->interface_window, SIGNAL(signal_message_statusbar(const QString&)), this, SLOT(show_message_statusbar(const QString&)));
    this->statusbar_timer = new QTimer(this);
    connect(this->statusbar_timer, SIGNAL(timeout()), this, SLOT(statusbar_timeout()));

    // connect actions select menu
    connect(action_select_all, SIGNAL(triggered()), this->interface_window, SLOT(select_all_atoms()));
    connect(action_deselect_all, SIGNAL(triggered()), this->interface_window, SLOT(deselect_all_atoms()));
    connect(action_invert_selection, SIGNAL(triggered()), this->interface_window, SLOT(invert_selection()));
    connect(action_set_frozen, SIGNAL(triggered()), this->interface_window, SLOT(set_frozen()));
    connect(action_set_unfrozen, SIGNAL(triggered()), this->interface_window, SLOT(set_unfrozen()));

    // connect actions camera menu
    connect(menu_camera_align, SIGNAL(triggered(QAction*)), this->interface_window, SLOT(set_camera_align(QAction*)));
    connect(menu_camera_mode, SIGNAL(triggered(QAction*)), this->interface_window, SLOT(set_camera_mode(QAction*)));

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
    this->resize(1280,640);

    qDebug() << "Done building MainWindow";
}

/**
 * @brief Parse command line arguments
 */
void MainWindow::set_cli_parser(const QCommandLineParser& cli_parser) {
    if(!cli_parser.value("n").isEmpty()) {
        qDebug() << "Received CLI '-n': " << cli_parser.value("n");
        auto* neb_widget = new AnalysisNEB();
        neb_widget->load_file(cli_parser.value("n"));
        neb_widget->show();
    }

    if(!cli_parser.value("g").isEmpty()) {
        qDebug() << "Received CLI '-g': " << cli_parser.value("g");
        auto* ago_widget = new AnalysisGeometryOptimization();
        auto sl = StructureLoader();
        ago_widget->set_structures(sl.load_outcar(cli_parser.value("g").toStdString()));
        ago_widget->show();
    }

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
        tr("All supported files (*.geo *.xyz *.vasp OUTCAR* CONTCAR* POSCAR*);;"
           "VASP files (*.vasp POSCAR* CONTCAR* OUTCAR*);;"
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
                        ".\n\nAuthor:\nIvo Filot <i.a.w.filot@tue.nl>\n\n"
                        PROGRAM_NAME " is licensed under the GPLv3 license.\n\n"
                        PROGRAM_NAME " is dynamically linked to Qt, which is licensed under LGPLv3.\n");
    message_box.setIcon(QMessageBox::Information);
    message_box.setWindowTitle("About " PROGRAM_NAME);
    message_box.setWindowIcon(QIcon(":/assets/icon/atom_architect_256.ico"));
    message_box.setIconPixmap(QPixmap(":/assets/icon/atom_architect_256.ico"));
    message_box.exec();
}

/**
 * @brief      Opens an analysis geometry optimization window.
 */
void MainWindow::open_analysis_geometry_optimization_window() {
    auto* ago_widget = new AnalysisGeometryOptimization();
    const std::string filename = "OUTCAR";
    QTemporaryDir tmp_dir;
    QFile::copy(":/assets/structures/" + tr(filename.c_str()), tmp_dir.path() + "/" + filename.c_str());
    auto sl = StructureLoader();
    ago_widget->set_structures(sl.load_outcar((tmp_dir.path() + "/" + filename.c_str()).toStdString()));
    ago_widget->show();
}

/**
 * @brief      Opens an analysis geometry optimization window.
 */
void MainWindow::open_analysis_neb_window() {
    auto* anw = new AnalysisNEB();
    anw->show();
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
 * @brief      Handle windows move event
 *
 * Updates anaglyph window
 *
 * @param      event  MoveEvent
 */
void MainWindow::moveEvent(QMoveEvent* event) {
    this->interface_window->get_anaglyph_widget()->window_move_event();

    // this is just to silence a warning message, we are not using event here, but this is the callback form
    (void)event;
}

/**
 * @brief      Handles drag Enter event
 *
 * @param      event  The event
 */
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief      Handles file drop event
 *
 * @param      event  The event
 */
void MainWindow::dropEvent(QDropEvent *event) {
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
                this->interface_window->open_file(url);
            } else {
                QMessageBox::critical(this, tr("Failed to load file"), tr("Could not load file. Did you try to load this file from a network drive? This is not supported.") );
                statusBar()->showMessage("Error loading file.");
                return;
            }
        }

        // show message after loading
        if(urlList.size() == 1) {
            statusBar()->showMessage("Loaded " + urlList.at(0).path() + ".");
        } else {
            statusBar()->showMessage("Loaded files.");
        }
    } else {
        statusBar()->showMessage("Could not identify dropped format. Ignoring...");
    }
}

/**
 * @brief      Handles drag move event
 *
 * @param      event  The event
 */
void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief      Handles event when object is dragged outside window
 *
 * @param      event  The event
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
