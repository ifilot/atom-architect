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

#include "structure_operator.h"

/**
 * @brief      Constructs a new instance.
 */
StructureOperator::StructureOperator() {

}

/**
 * @brief      Adds a fragment.
 *
 * @param      structure  The structure
 * @param[in]  fragment   The fragment
 * @param[in]  distance   The distance
 */
void StructureOperator::add_fragment(Structure* structure, const Fragment& fragment, double distance) {
    auto v1 = structure->get_position_primary_buffer();
    auto v2 = structure->get_position_secondary_buffer();

    auto direction = (v1 - v2).normalized();
    auto rotmat = this->build_z_align_matrix(direction);

    for(const Atom& at : fragment.atoms) {
        auto apos = at.get_pos();
        QVector4D apos4(apos, 1.0);
        auto newpos = (rotmat * apos4).toVector3D();
        newpos += v1 + direction * distance;

        structure->add_atom(at.atnr, newpos[0], newpos[1], newpos[2]);
    }

    structure->clear_selection();
    structure->update();
}

QMatrix4x4 StructureOperator::build_z_align_matrix(const QVector3D& target_direction) const {
    QMatrix4x4 res;
    res.setToIdentity();

    QVector3D axis;
    float angle;

    // avoid gimball locking
    if (fabs(target_direction[2]) > .999) {
        if(target_direction[2] < 0.0) {
            axis = QVector3D(0.0, 1.0, 0.0);
            angle = -180.0;
        } else {
            axis = QVector3D(0.0, 0.0, 1.0);
            angle = 0.0;
        }
    } else {
        axis = QVector3D::crossProduct(QVector3D(0.0, 0.0, 1.0), target_direction);
        angle = std::acos(target_direction[2]) * 180.0 / M_PI;
    }

    res.rotate(angle, axis);

    return res;
}
