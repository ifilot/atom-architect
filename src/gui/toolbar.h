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

#include <stdexcept>
#include <string>
#include <unordered_map>

#include <QToolBar>
#include <QIcon>

class ToolBarWidget : public QToolBar {
private:
    std::unordered_map<std::string, QAction*> actions;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      parent  The parent
     */
    ToolBarWidget(QWidget* parent = nullptr);

    /**
     * @brief      Get an action by action name
     *
     * @param[in]  action_name  The action name
     *
     * @return     The action.
     */
    QAction* get_action(const std::string& action_name);

private:
    /**
     * @brief      Add an action to the toolbar
     *
     * @param[in]  action_name  The action name
     */
    void add_action(const std::string& action_name, const QString& icon = "tools");
};
