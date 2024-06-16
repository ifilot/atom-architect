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

#include "fragment.h"

Fragment::Fragment(const std::string& _name, const QString& _label) :
name(_name),
label(_label) {

}

/**
 * @brief      Add an atom to the structure
 *
 * @param[in]  atnr  Atom number
 * @param[in]  x     x coordinate
 * @param[in]  y     y coordinate
 * @param[in]  z     z coordinate
 */
void Fragment::add_atom(unsigned int atnr, double x, double y, double z) {
    this->atoms.emplace_back(atnr, x, y, z);
}
