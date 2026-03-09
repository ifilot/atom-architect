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
#include <QVBoxLayout>
#include <QScrollArea>

#include <memory>

#include "../data/structure.h"

/**
 * @brief StructureInfoTab class.
 */
class StructureInfoTab : public QWidget {
protected:
    std::shared_ptr<Structure> structure;

    QVBoxLayout *container_layout;
    QScrollArea *scrollarea;

public:
    /**
     * @brief StructureInfoTab.
     *
     * @param parent Parameter parent.
     */
    explicit StructureInfoTab(QWidget *parent = nullptr) : QWidget(parent) {
        this->container_layout = new QVBoxLayout();
        this->scrollarea = new QScrollArea();

        this->setLayout(container_layout);
        this->container_layout->addWidget(this->scrollarea);
    }

    /**
     * @brief set_structure.
     *
     * @param _structure Parameter _structure.
     */
    inline void set_structure(const std::shared_ptr<Structure>& _structure) {
        this->structure = _structure;
        this->update_data();
    }

/**
 * @brief update_data.
 *
 */
    virtual void update_data() = 0;

/**
 * @brief reset.
 *
 */
    virtual void reset() = 0;

protected:
};
