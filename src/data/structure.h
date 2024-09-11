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

#include <QDebug>
#include <QVector3D>
#include <QMatrix4x4>
#include <QGenericMatrix>
#include <vector>
#include <QString>

#include "atom_settings.h"
#include "matrixmath.h"
#include "atom.h"
#include "bond.h"
#include "fragment.h"

/**
 * @brief      This class describes a chemical structure.
 */
class Structure {

private:
    std::vector<Atom> atoms;            // atoms in the structure
    std::vector<Bond> bonds;            // bonds between the atoms

    double energy = 0.0;                // energy of the structure (if known, zero otherwise)
    std::vector<QVector3D> forces;      // forces on the atoms (if known, empty array otherwise)

    std::vector<Atom> atoms_expansion;  // atoms in the unit cell expansion
    std::vector<Bond> bonds_expansion;  // bonds in the unit cell expansion

    MatrixUnitcell unitcell;            // matrix describing the unit cell
    std::vector<double> radii;          // radii of the atoms

    std::unordered_map<std::string, unsigned int> element_types;    // elements present in the structure

    // atom selection buffers
    std::vector<unsigned int> primary_buffer;   // primary selection buffer
    std::vector<unsigned int> secondary_buffer; // secondary selection buffer

public:
    /**
     * @brief      Constructs a new instance.
     */
    Structure(const MatrixUnitcell& unitcell);

    /**
     * @brief      Constructs a new instance.
     */
    Structure(const Fragment& fragment);

    /**
     * @brief      Constructs a new instance.
     */
    Structure(unsigned int elnr);

    /**
     * @brief      Sets the energy.
     *
     * @param[in]  _energy  The energy
     */
    inline void set_energy(double _energy) {
        this->energy = _energy;
    }

    /**
     * @brief      Gets the energy.
     *
     * @return     The energy.
     */
    double get_energy() const {
        return this->energy;
    }

    /**
     * @brief      Gets the root mean square force
     *
     * @return     The root mean square force.
     */
    double get_rms_force() const;

    /**
     * @brief      Get all atoms from the structure
     *
     * @return     The atoms.
     */
    inline const auto& get_atoms() const {
        return this->atoms;
    }

    /**
     * @brief      Get all bonds from the structure
     *
     * @return     The atoms.
     */
    inline const auto& get_bonds() const {
        return this->bonds;
    }

    /**
     * @brief      Get all atoms from the structure
     *
     * @return     The atoms.
     */
    inline const auto& get_atoms_expansion() const {
        return this->atoms_expansion;
    }

    /**
     * @brief      Get all bonds from the structure
     *
     * @return     The atoms.
     */
    inline const auto& get_bonds_expansion() const {
        return this->bonds_expansion;
    }

    /**
     * @brief      Get specific atom
     *
     * @param[in]  idx   The index
     *
     * @return     The atom.
     */
    inline const Atom& get_atom(unsigned int idx) const {
        return this->atoms[idx];
    }

    /**
     * @brief      Get specific bond
     *
     * @param[in]  idx   The index
     *
     * @return     The bond.
     */
    inline const Bond& get_bond(unsigned int idx) const {
        return this->bonds[idx];
    }

    /**
     * @brief      Gets the unitcell.
     *
     * @return     The unitcell.
     */
    inline const auto& get_unitcell() const {
        return this->unitcell;
    }

    /**
     * @brief      Gets the atomic radius.
     *
     * @param[in]  idx   The index
     *
     * @return     The radius.
     */
    inline double get_radius(unsigned int idx) const {
        return this->radii[idx];
    }

    /**
     * @brief      Add an atom to the structure
     *
     * @param[in]  atnr  Atom number
     * @param[in]  x     x coordinate
     * @param[in]  y     y coordinate
     * @param[in]  z     z coordinate
     */
    void add_atom(unsigned int atnr, double x, double y, double z);

    /**
     * @brief      Add an atom to the structure including forces
     *
     * @param[in]  atnr  Atom number
     * @param[in]  x     x coordinate
     * @param[in]  y     y coordinate
     * @param[in]  z     z coordinate
     * @param[in]  fx    force in x direction
     * @param[in]  fy    force in y direction
     * @param[in]  fz    force in z direction
     */
    void add_atom(unsigned int atnr, double x, double y, double z, double fx, double fy, double fz);

    /**
     * @brief      Add an atom to the structure including forces
     *
     * @param[in]  atnr  Atom number
     * @param[in]  x     x coordinate
     * @param[in]  y     y coordinate
     * @param[in]  z     z coordinate
     * @param[in]  sx    Selective dynamics x direction
     * @param[in]  sy    Selective dynamics y direction
     * @param[in]  sz    Selective dynamics z direction
     */
    void add_atom(unsigned int atnr, double x, double y, double z, bool sx, bool sy, bool sz);

    /**
     * @brief      Delete atoms in the primary buffer
     */
    void delete_atoms();

    /**
     * @brief      Commits a transposition.
     *
     * @param[in]  transposition  The transposition
     */
    void commit_transposition(const QMatrix4x4& transposition);

    /**
     * @brief      Gets the total number of atoms.
     *
     * @return     The number of atoms
     */
    inline size_t get_nr_atoms() const {
        return this->atoms.size();
    }

    /**
     * @brief      Gets the total number of bonds.
     *
     * @return     The number of bonds
     */
    inline size_t get_nr_bonds() const {
        return this->bonds.size();
    }

    ~Structure() {
        qDebug() << "Deleting structure ("
                 << QString("0x%1").arg((size_t)this, 0, 16)
                 << "; " << this->atoms.size() << " atoms ).";
    }

    //********************************************
    // [END BLOCK] DATA GETTERS AND SETTERS
    //********************************************

    /**
     * @brief      Center the structure at the origin
     */
    void center();

    /**
     * @brief      Get the largest distance from the origin
     *
     * @return     The largest distance.
     */
    QVector3D get_largest_distance() const;

    /**
     * @brief      Gets the elements in this structure as a string
     *
     * @return     String holding comma seperated list of elements
     */
    std::string get_elements_string() const;

    /**
     * @brief      Update data based on contents;
     */
    void update();

    /**
     * @brief      Get the centering vector
     *
     * @return     Vector that puts unitcell at the origin
     */
    QVector3D get_center_vector() const;

    /**
     * @brief      Select atom by idx
     *
     * @param[in]  idx   The index
     */
    void select_atom(unsigned int idx);

    /**
     * @brief      Gets the nr atoms in the primary buffer.
     *
     * @return     The nr atoms primary buffer.
     */
    inline size_t get_nr_atoms_primary_buffer() const {
        return this->primary_buffer.size();
    }

    /**
     * @brief      Gets the nr atoms in the secondary buffer.
     *
     * @return     The nr atoms secondary buffer.
     */
    inline size_t get_nr_atoms_secondary_buffer() const {
        return this->secondary_buffer.size();
    }

    /**
     * @brief      Gets the position primary buffer.
     *
     * @return     The position primary buffer.
     */
    QVector3D get_position_primary_buffer() const;

    /**
     * @brief      Gets the position secondary buffer.
     *
     * @return     The position secondary buffer.
     */
    QVector3D get_position_secondary_buffer() const;

    /**
     * @brief      Clear the selection_buffers
     */
    void clear_selection();

    /**
     * @brief      Select all atoms
     */
    void select_all_atoms();

    /**
     * @brief      Select all atoms
     */
    void invert_selection();

    /**
     * @brief      Toggle frozen
     */
    void set_frozen();

    /**
     * @brief      Set unfrozen
     */
    void set_unfrozen();

    /**
     * @brief      Get a string containing current selection data
     *
     * @return     The selection string.
     */
    QString get_selection_string() const;

private:
    /**
     * @brief      Count the number of elements
     */
    void count_elements();

    /**
     * @brief      Construct the bonds
     */
    void construct_bonds();

    /**
     * @brief      Expand unit cell
     */
    void build_expansion();

    /**
     * @brief      Transpose single atom
     *
     * @param[in]  idx            Atom index
     * @param[in]  transposition  The transposition
     */
    void transpose_atom(unsigned int idx, const QMatrix4x4& transposition);

    /**
     * @brief      Gets the unitcell matrix.
     *
     * @param[in]  matrix  The matrix
     *
     * @return     The casted matrix.
     */
    QMatrix3x3 get_matrix3x3(const MatrixUnitcell& matrix) const;
};
