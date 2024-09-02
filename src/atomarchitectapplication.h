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

#include <QApplication>
#include <QMessageBox>
#include <QIcon>

#include "config.h"

class AtomArchitectApplication : public QApplication
{
public:
    /**
     * @brief Default constructor
     * @param argc number of command line argument
     * @param argv command line arguments
     */
    AtomArchitectApplication(int& argc, char** argv);

    /**
     * @brief notify
     * @param receiver
     * @param event
     * @return
     */
    bool notify(QObject* receiver, QEvent* event);

private:
    void throw_message_window(const QString& title, const QString& message);
};
