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
#include <QDebug>

#include <iostream>
#include <iomanip>
#include <ctime>

#include "gui/mainwindow.h"
#include "config.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // write boot of program
    std::ofstream lf("execution.log");
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    lf << "Start application: " << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;

    qDebug() << "Start application";

    // set OpenGL surface settings
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

    qDebug() << "Creating MainWindow";

    std::unique_ptr<MainWindow> mainWindow;

    try {
        // build main window
        qDebug() << "Creating MainWindow pointer";
        mainWindow = std::make_unique<MainWindow>();
        mainWindow->setWindowTitle(QString(PROGRAM_NAME) + " " + QString(PROGRAM_VERSION));
        mainWindow->resize(1280,640);
        qDebug() << "Done setting MainWindow pointer";
    } catch(const std::exception& e) {
        // if any errors are caught in the process of starting up the application,
        // they will be printed in the execution.log file
        qDebug() << "Error detected!";
        qDebug() << e.what();
        lf << "Error detected!" << std::endl;
        lf << e.what() << std::endl;
        auto texc = std::time(nullptr);
        auto tmexc = *std::localtime(&texc);
        lf << "Abnormal closing of program: " << std::put_time(&tmexc, "%d-%m-%Y %H-%M-%S") << std::endl;
        lf.close();
    }

    qDebug() << "Perform show() call";
    mainWindow->show();

    lf.close();

    qDebug() << "Launching application";
    return app.exec();
}
