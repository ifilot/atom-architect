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

#include <QFileInfo>
#include <QRegularExpression>
#include <fstream>

#include "atom_settings.h"
#include "structure.h"

enum OutcarReadStatus {
    VASP_OUTCAR_READ_STATE_UNDEFINED,
    VASP_OUTCAR_READ_STATE_ELEMENTS,
    VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT,
    VASP_OUTCAR_READ_STATE_LATTICE_VECTORS,
    VASP_OUTCAR_READ_STATE_ATOMS,
    VASP_OUTCAR_READ_STATE_OPEN,
    VASP_OUTCAR_READ_STATE_FINISHED
};

class StructureLoader {
private:

public:
    /**
     * @brief      Constructs a new instance.
     */
    StructureLoader();

    /**
     * @brief      Load a file
     *
     * @param[in]  filename  The filename
     *
     * @return     shared ptr to structure object
     */
    std::shared_ptr<Structure> load_file(const std::string& filename);

    /**
     * @brief      Load structure from OUTCAR file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structures
     */
    std::vector<std::shared_ptr<Structure>> load_outcar(const std::string& filename);

    /**
     * @brief      Load NEB binary
     *
     * @param[in]  filename  The filename
     *
     * @return     Bundled set of structures
     */
    std::vector<std::vector<std::shared_ptr<Structure>>> load_neb_bin(const std::string& filename);

private:
    /**
     * @brief      Load structure from .geo file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structure
     */
    std::shared_ptr<Structure> load_geo(const std::string& filename);

    /**
     * @brief      Load structure from .xyz file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structure
     */
    std::shared_ptr<Structure> load_xyz(const std::string& filename);

    /**
     * @brief      Load structure from POSCAR file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structure
     */
    std::shared_ptr<Structure> load_poscar(const std::string& filename);

};
