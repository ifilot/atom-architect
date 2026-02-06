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

#include "bond.h"

Bond::Bond(const Atom& _atom1, const Atom& _atom2, unsigned int _atom1_idx, unsigned int _atom2_idx) :
atom1(_atom1),
atom2(_atom2),
atom1_idx(_atom1_idx),
atom2_idx(_atom2_idx) {
    auto v = this->atom2.get_pos_qtvec() - this->atom1.get_pos_qtvec();

    this->direction = v.normalized();
    this->length = v.length();

    // avoid gimball locking
    if (fabs(this->direction[2]) > .999) {
        if(this->direction[2] < 0.0) {
            this->axis = QVector3D(0.0, 1.0, 0.0);
            this->angle = -M_PI;
        } else {
            this->axis = QVector3D(0.0, 0.0, 1.0);
            this->angle = 0.0;
        }
    } else {
        this->axis = QVector3D::crossProduct(QVector3D(0.0, 0.0, 1.0), this->direction);
        this->angle = std::acos(this->direction[2]);
    }
}
