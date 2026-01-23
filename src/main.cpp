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
#include <QApplication>
#include <QSurfaceFormat>
#include <QStatusBar>
#include <QStringList>
#include <QDebug>
#include <QCommandLineParser>

#include <iostream>
#include <iomanip>
#include <ctime>
#include <memory>

#include "atomarchitectapplication.h"
#include "gui/mainwindow.h"
#include "config.h"

std::shared_ptr<QStringList> log_messages;

/**
 * @brief custom function for storing and display messages
 * @param type
 * @param context
 * @param msg
 */
void message_output(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString local_msg = QString(msg.toLocal8Bit());

    QDateTime date = QDateTime::currentDateTime();
    QString fmtime = date.toString("dd.MM.yyyy hh:mm:ss.zzz");

    switch (type) {
    case QtDebugMsg:
        log_messages->append(fmtime + " [DEBUG] " + local_msg);
        std::cout << "[DEBUG] " << msg.toStdString() << std::endl;
        break;
    case QtInfoMsg:
        log_messages->append(fmtime + " [INFO] " + local_msg);
        std::cout << "[INFO] " << msg.toStdString() << std::endl;
        break;
    case QtWarningMsg:
        log_messages->append(fmtime + " [WARNING] " + local_msg);
        std::cout << "[WARNING] " << msg.toStdString() << std::endl;
        break;
    case QtCriticalMsg:
        log_messages->append(fmtime + " [CRITICAL] " + local_msg);
        std::cerr << "[CRITICAL] " << msg.toStdString() << std::endl;
        break;
    case QtFatalMsg:
        log_messages->append(fmtime + " [FATAL] " + local_msg);
        std::cerr << "[FATAL] " << msg.toStdString() << std::endl;
        break;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("IMC");
    QCoreApplication::setOrganizationDomain("IMC");
    QCoreApplication::setApplicationName(PROGRAM_NAME);
    QCoreApplication::setApplicationVersion(PROGRAM_VERSION);
    QCoreApplication::setOrganizationName("Inorganic Materials & Catalysis");
    QCoreApplication::setApplicationName("AtomArchitect");

    // set command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Atomistic visualization and builder tool");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption loadNEB("n", "Load NEB analysis", "file");
    parser.addOption(loadNEB);

    QCommandLineOption loadGeoOpt("g", "Load geometry analysis", "file");
    parser.addOption(loadGeoOpt);

    QCommandLineOption openFile("o", "Open structure file", "file");
    parser.addOption(openFile);

    AtomArchitectApplication app(argc, argv);
    qRegisterMetaType<std::vector<uint8_t>>("stdvector_uint8_t");

    std::unique_ptr<MainWindow> mainWindow;
    log_messages = std::make_shared<QStringList>();

    // parse command line arguments
    parser.process(app);

    try {
        // build main window
        qInstallMessageHandler(message_output);
        mainWindow = std::make_unique<MainWindow>(log_messages);
        mainWindow->setWindowTitle(QString(PROGRAM_NAME) + " " + QString(PROGRAM_VERSION));
        mainWindow->set_cli_parser(parser);
    } catch(const std::exception& e) {
        std::cerr << "Error detected!" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "Abnormal closing of program." << std::endl;
    }

    mainWindow->show();

    int res = -1;
    try {
        res = app.exec();
    }  catch (const std::exception& e) {
        std::cerr << "Error detected!" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "Abnormal closing of program." << std::endl;
    }

    return res;
}
