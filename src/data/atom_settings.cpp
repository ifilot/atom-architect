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

#include "atom_settings.h"

/**
 * @brief      Constructs a new instance.
 */
AtomSettings::AtomSettings() {
    this->load();

    // set all bonds by default to 3.0
    this->bond_distances.resize(121);
    for(unsigned int i=0; i<121; i++) {
        this->bond_distances[i].resize(121, 3.0);
    }

    // loop over all atoms
    for(unsigned int i=0; i<121; i++) {
        if(i > 20) {
            for(unsigned int j=2; j<=20; j++) {
                // bonds for hydrogen
                this->bond_distances[i][1] = 2.0;
                this->bond_distances[1][i] = 2.0;

                // other atoms
                this->bond_distances[i][j] = 2.5;
                this->bond_distances[j][i] = 2.5;
            }
        } else {
            for(unsigned int j=2; j<=20; j++) {
                // bonds for hydrogen
                this->bond_distances[i][1] = 1.2;
                this->bond_distances[1][i] = 1.2;

                // other atoms
                this->bond_distances[i][j] = 2.0;
                this->bond_distances[j][i] = 2.0;
            }
        }
    }

    // add some special cases on the basis of user input
    this->bond_distances[6][13] = 3.5; // Al-C
    this->bond_distances[13][6] = 3.5;

    this->radii.resize(119);
    for(unsigned int i=1; i<=118; i++) {
        this->radii[i] = this->get_atom_radius(this->get_name_from_elnr(i));
    }
}

/**
 * @brief      Load the JSON file and parse its contents
 */
void AtomSettings::load() {
    // try to locate atoms.json
    QFile f(":/assets/configuration/atoms.json");
    if (!f.exists()) {
        qWarning() << "Cannot open atoms.json";
        exit(-1);
    }

    // open the file and read the data
    f.open(QIODevice::ReadOnly);
    QString jsondata = f.readAll();
    f.close();

    // try to parse the file
    QJsonParseError parseError;
    this->root = QJsonDocument::fromJson(jsondata.toUtf8(), &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
        exit(-1);
    }
}

/**
 * @brief      Get the default color for an atom
 *
 * @param[in]  elname  Element name
 *
 * @return     Color of the atom
 */
glm::vec3 AtomSettings::get_atom_color(const std::string& elname){
    QString hexColor = this->root["atoms"]["colors"][QString(elname.c_str())].toString();
    QColor *getColor = new QColor(hexColor);
    return glm::vec3((float)getColor->red()/255, (float)getColor->green()/255, (float)getColor->blue()/255);
}

/**
 * @brief      Get the default color for an atom
 *
 * @param[in]  elname  Element name
 *
 * @return     Color of the atom
 */
QVector3D AtomSettings::get_atom_color_qvector(const std::string& elname){
    QString hexColor = this->root["atoms"]["colors"][QString(elname.c_str())].toString();
    QColor *getColor = new QColor(hexColor);
    return QVector3D((float)getColor->red()/255, (float)getColor->green()/255, (float)getColor->blue()/255);
}

/**
 * @brief      Get the atomic radius of an element
 *
 * @param[in]  elname  Element name
 *
 * @return     atomic radius
 */
float AtomSettings::get_atom_radius(const std::string& elname){
    return this->root["atoms"]["radii"][QString(elname.c_str())].toString().toDouble();
}

/**
 * @brief      Get the atomic radius of an element
 *
 * @param[in]  elname  Element name
 *
 * @return     atomic radius
 */
float AtomSettings::get_atom_radius_from_elnr(unsigned int elnr) {
    return this->radii[elnr];
}

/**
 * @brief      Get element number of an element
 *
 * @param[in]  elname  Element name
 *
 * @return     The atom elnr.
 */
unsigned int AtomSettings::get_atom_elnr(const std::string& elname){
    return this->root["atoms"]["elnr"][QString(elname.c_str())].toString().toUInt();
}

/**
 * @brief      Get the maximum bond distance between two atoms
 *
 * @param[in]  atoma  The atoma
 * @param[in]  atomb  The atomb
 *
 * @return     The bond distance.
 */
double AtomSettings::get_bond_distance(int atoma, int atomb) {
    return this->bond_distances[atoma][atomb];
}

/**
 * @brief      Gets the name from element number.
 *
 * @param[in]  elnr  The elnr
 *
 * @return     The name from elnr.
 */
std::string AtomSettings::get_name_from_elnr(unsigned int elnr) {
    return this->root["atoms"]["nr2element"][QString::number(elnr)].toString().toStdString();
}
