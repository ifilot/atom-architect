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

#include "toolbar.h"

/**
 * @brief      Constructs a new instance.
 *
 * @param      parent  The parent
 */
ToolBarWidget::ToolBarWidget(QWidget* parent) :
QToolBar(parent) {
    // set vertical orientation
    this->setOrientation(Qt::Vertical);

    // add actions to toolbar
    setIconSize(QSize(32, 32));
    this->add_action("toggle_periodicity_xy", "expand_xy");
    this->add_action("toggle_periodicity_z", "expand_z");
    // this->add_action("---");
    this->add_action("add_fragment", "add_fragment");
}

/**
 * @brief      Get an action by action name
 *
 * @param[in]  action_name  The action name
 *
 * @return     The action.
 */
QAction* ToolBarWidget::get_action(const std::string& action_name) {
    auto got = this->actions.find(action_name);
    if(got != this->actions.end()) {
        return got->second;
    } else {
        throw std::runtime_error("Invalid action requested: " + action_name);
    }
}

/**
 * @brief      Add an action to the toolbar
 *
 * @param[in]  action_name  The action name
 */
void ToolBarWidget::add_action(const std::string& action_name, const QString& icon) {
    if(action_name == "---") {
        this->addSeparator();
    } else {
        auto action = this->addAction(QIcon(":/assets/icon/" + icon + ".png"), action_name.c_str());
        this->actions.emplace(action_name, action);
    }
}
