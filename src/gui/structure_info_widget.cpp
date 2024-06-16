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

#include "structure_info_widget.h"

StructureInfoWidget::StructureInfoWidget(QWidget *parent) :
    QWidget(parent)
{
    // create parent layout of this window
    QVBoxLayout *parentLayout = new QVBoxLayout;
    this->setLayout(parentLayout);

    // add tabs
    this->tabs = new QTabWidget();
    this->tabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    parentLayout->addWidget(this->tabs);

    // add a couple of tabs
    this->structure_basic_info_tab = new StructureInfoBasicTab();
    this->tabs->addTab(this->structure_basic_info_tab, tr("Basic info"));
    this->fragment_selector = new FragmentSelector();
    this->tabs->addTab(this->fragment_selector, tr("Fragments"));
}

/**
 * @brief      Sets the structure.
 *
 * @param[in]  _structure  The structure
 */
void StructureInfoWidget::set_structure(const std::shared_ptr<Structure>& _structure) {
    this->structure_basic_info_tab->set_structure(_structure);
}

/**
 * @brief      Updates the object.
 */
void StructureInfoWidget::update() {
    this->structure_basic_info_tab->update_data();
}

/**
 * @brief      Resets the object.
 */
void StructureInfoWidget::reset() {
    this->structure_basic_info_tab->reset();
}
