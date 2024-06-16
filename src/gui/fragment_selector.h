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

#include <unordered_map>

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QJsonObject>

#include "../data/fragment.h"
#include "../data/atom_settings.h"
#include "structure_info_tab.h"
#include "anaglyph_widget.h"
#include "periodic_table.h"

class FragmentSelector : public StructureInfoTab {
    Q_OBJECT

private:
    std::unordered_map<std::string, Fragment> fragments;

    QVBoxLayout *layout;
    QLineEdit *searchbox;
    QListWidget *fragment_list;
    AnaglyphWidget *anaglyph_widget;
    QLabel *label_current_selection;
    QPushButton *button_periodic_table;

public:
    FragmentSelector(QWidget *parent = nullptr);

    /**
     * @brief      Gets the current fragment.
     *
     * @return     The current fragment.
     */
    const Fragment& get_current_fragment() const;

private:
    /**
     * @brief      Add a fragment
     *
     * @param[in]  fragment  The fragment
     */
    void add_fragment(const Fragment& fragment);

    /**
     * @brief      Add series of fragments from file
     *
     * @param[in]  filename  The filename
     */
    void add_fragments_from_file(const QString& filename);

    /**
     * @brief      Calculate distance between two strings
     *
     * @param[in]  str1  String 1
     * @param[in]  str2  String 2
     *
     * @return     distance between strings
     */
    size_t string_levenshtein_distance(const std::string& str1, const std::string& str2);

signals:
    void signal_new_fragment(const Fragment& fragment);

private slots:
    /**
     * @brief      Perform fuzzy search
     */
    void perform_fuzzy_search(const QString& _source);

    /**
     * @brief      Update the anaglyph widget with the selected molecule
     *
     * @param      current   Currently selected fragment
     * @param      previous  Previously selected fragments
     */
    void update_display(QListWidgetItem *current, QListWidgetItem *previous);

    /**
     * @brief      Select an atom from the periodic table
     */
    void select_atom_periodic_table();

public slots:
    /**
     * @brief      Update data in tab based on current structure
     */
    void update_data();

    /**
     * @brief      Resets the object.
     */
    void reset();

};
