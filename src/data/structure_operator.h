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

#include <QMatrix4x4>

#include "structure.h"

class StructureOperator {
private:

public:
    /**
     * @brief      Constructs a new instance.
     */
    StructureOperator();

    /**
     * @brief      Adds a fragment.
     *
     * @param      structure  The structure
     * @param[in]  fragment   The fragment
     * @param[in]  distance   The distance
     */
    void add_fragment(Structure* structure, const Fragment& fragment, double distance);

private:
    QMatrix4x4 build_z_align_matrix(const QVector3D& target_direction) const;
};
