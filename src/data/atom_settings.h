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

// qt headers
#include <QFile>
#include <QTemporaryDir>
#include <QColor>
#include <QVector3D>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

// glm headers
#include <glm/glm.hpp>

// stl headers
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

/**
 * @brief      Class holding information about atoms in the periodic table
 */
class AtomSettings {
private:
    std::string settings_file;
    QJsonDocument root;
    QFile qf;

    std::vector<std::vector<double>> bond_distances;
    std::vector<float> radii;

public:

    /**
     * @brief      Get AtomSettings Class
     *
     * Default singleton pattern
     *
     * @return     return instance of the atomsettings class
     */
    static AtomSettings& get() {
        static AtomSettings settings_instance;
        return settings_instance;
    }

    /**
     * @brief      Get the default color for an atom
     *
     * @param[in]  elname  Element name
     *
     * @return     Color of the atom
     */
    glm::vec3 get_atom_color(const std::string& elname);

    /**
     * @brief      Get the default color for an atom
     *
     * @param[in]  elname  Element name
     *
     * @return     Color of the atom
     */
    QVector3D get_atom_color_qvector(const std::string& elname);

    /**
     * @brief      Get the atomic radius of an element
     *
     * @param[in]  elname  Element name
     *
     * @return     atomic radius
     */
    float get_atom_radius(const std::string& elname);

    /**
     * @brief      Get the atomic radius of an element
     *
     * @param[in]  elname  Element name
     *
     * @return     atomic radius
     */
    float get_atom_radius_from_elnr(unsigned int elnr);

    /**
     * @brief      Get element number of an element
     *
     * @param[in]  elname  Element name
     *
     * @return     The atom elnr.
     */
    unsigned int get_atom_elnr(const std::string& elname);

    /**
     * @brief      Get the maximum bond distance between two atoms
     *
     * @param[in]  atoma  The atoma
     * @param[in]  atomb  The atomb
     *
     * @return     The bond distance.
     */
    double get_bond_distance(int atoma, int atomb);

    /**
     * @brief      Gets the name from element number.
     *
     * @param[in]  elnr  The elnr
     *
     * @return     The name from elnr.
     */
    std::string get_name_from_elnr(unsigned int elnr);

private:
    /**
     * @brief      Constructs a new instance.
     */
    AtomSettings();

    /**
     * @brief      Load the JSON file and parse its contents
     */
    void load();

    // delete copy constructor
    AtomSettings(AtomSettings const&)          = delete;
    void operator=(AtomSettings const&)    = delete;
};
