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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTabWidget>

#include "anaglyph_widget.h"
#include "structure_info_basic_tab.h"
#include "fragment_selector.h"

class StructureInfoWidget : public QWidget {
    Q_OBJECT

private:
    QTabWidget *tabs;

    AnaglyphWidget *anaglyph_widget;

    StructureInfoBasicTab *structure_basic_info_tab;
    FragmentSelector *fragment_selector;

public:
    /**
     * @brief      Constructs the object.
     *
     * @param      mw    pointer to MainWindow object
     */
    StructureInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief      Sets the structure colorizer.
     *
     * @param[in]  _structure_colorizer  The structure colorizer
     */
    inline void set_anaglyph_widget(AnaglyphWidget *_anaglyph_widget) {
        this->anaglyph_widget = _anaglyph_widget;
    }

    /**
     * @brief      Gets the fragment selector.
     *
     * @return     The fragment selector.
     */
    inline FragmentSelector* get_fragment_selector() const {
        return this->fragment_selector;
    }

    /**
     * @brief      Sets the structure.
     *
     * @param[in]  _structure  The structure
     */
    void set_structure(const std::shared_ptr<Structure>& _structure);

private:


protected:


public slots:
    /**
     * @brief      Updates the object.
     */
    void update();

    /**
     * @brief      Resets the object.
     */
    void reset();

private slots:

};
