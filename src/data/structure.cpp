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

#include "structure.h"

/**
 * @brief      Constructs a new instance.
 */
Structure::Structure(const MatrixUnitcell& _unitcell) :
unitcell(_unitcell) {

}

/**
 * @brief      Constructs a new instance.
 */
Structure::Structure(const Fragment& fragment) {
    this->unitcell = MatrixUnitcell::Identity() * 5.0f;
    this->atoms = fragment.atoms;
    this->center();
    this->construct_bonds();
}

/**
 * @brief      Constructs a new instance.
 */
Structure::Structure(unsigned int elnr) {
    this->unitcell = MatrixUnitcell::Identity() * 2.5f;
    this->atoms.emplace_back(elnr, 0.0, 0.0, 0.0);
    this->center();
}

/**
 * @brief      Gets the root mean square force
 *
 * @return     The root mean square force.
 */
double Structure::get_rms_force() const {
    double sum = 0.0;
    for(const auto& force : this->forces) {
        sum += force.lengthSquared();
    }

    return sum / (float)this->forces.size();
}

/**
 * @brief      Add an atom to the structure
 *
 * @param[in]  atnr  Atom number
 * @param[in]  x     x coordinate
 * @param[in]  y     y coordinate
 * @param[in]  z     z coordinate
 */
void Structure::add_atom(unsigned int atnr, double x, double y, double z) {
    this->atoms.emplace_back(atnr, x, y, z);
    this->transpose_atom(this->atoms.size() - 1, QMatrix4x4());
    this->radii.push_back(AtomSettings::get().get_atom_radius(AtomSettings::get().get_name_from_elnr(atnr)));
}

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
void Structure::add_atom(unsigned int atnr, double x, double y, double z, double fx, double fy, double fz) {
    this->add_atom(atnr, x, y, z);
    this->forces.push_back(QVector3D(fx, fy, fz));
}

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
void Structure::add_atom(unsigned int atnr, double x, double y, double z, bool sx, bool sy, bool sz) {
    this->add_atom(atnr, x, y, z);
    this->atoms.back().selective_dynamics = {sx, sy, sz};
}

/**
 * @brief      Delete atoms in the primary buffer
 */
void Structure::delete_atoms() {
    std::sort(this->primary_buffer.begin(), this->primary_buffer.end(), std::greater<unsigned int>());

    // check if forces are known
    if(this->forces.size() == this->atoms.size()) {
        for(unsigned int idx : this->primary_buffer) {
            if(idx < this->get_nr_atoms()) {
                this->forces.erase(this->forces.begin() + idx);
            }
        }
    }

    for(unsigned int idx : this->primary_buffer) {
        if(idx < this->get_nr_atoms()) {
            this->atoms.erase(this->atoms.begin() + idx);
        }
    }

    // update contents
    this->update();
}

/**
 * @brief      Commits a transposition.
 *
 * @param[in]  transposition  The transposition
 */
void Structure::commit_transposition(const QMatrix4x4& transposition) {
    for(unsigned int idx : this->primary_buffer) {
        if(idx < this->get_nr_atoms()) {
            this->transpose_atom(idx, transposition);
        }
    }

    // update contents
    this->update();
}

/**
 * @brief      Center the structure at the origin
 */
void Structure::center() {
    double sumx = 0.0;
    double sumy = 0.0;
    double sumz = 0.0;

    #pragma omp parallel for reduction(+: sumx)
    for(unsigned int i=0; i<this->atoms.size(); i++) {
        sumx += this->atoms[i].x;
    }

    #pragma omp parallel for reduction(+: sumy)
    for(unsigned int i=0; i<this->atoms.size(); i++) {
        sumy += this->atoms[i].y;
    }

    #pragma omp parallel for reduction(+: sumz)
    for(unsigned int i=0; i<this->atoms.size(); i++) {
        sumz += this->atoms[i].z;
    }

    sumx /= (float)this->atoms.size();
    sumy /= (float)this->atoms.size();
    sumz /= (float)this->atoms.size();

    auto cv = get_center_vector();

    #pragma omp parallel for
    for(unsigned int i=0; i<this->atoms.size(); i++) {
        this->atoms[i].x -= sumx + cv[0];
        this->atoms[i].y -= sumy + cv[1];
        this->atoms[i].z -= sumz + cv[2];
    }
}

/**
 * @brief      Get the largest distance from the origin
 *
 * @return     The largest distance.
 */
QVector3D Structure::get_largest_distance() const {
    unsigned int idx = 0;
    float dist = -1.0f;

    #pragma omp parallel for
    for(unsigned int i=0; i<this->atoms.size(); i++) {
        float vdist = this->atoms[i].get_pos_qtvec().lengthSquared();

        if(vdist > dist) {
            #pragma omp critical
            {
                dist = vdist;
                idx = i;
            }
        }
    }

    return this->atoms[idx].get_pos_qtvec();
}

/**
 * @brief      Gets the elements in this structure as a string
 *
 * @return     String holding comma seperated list of elements
 */
std::string Structure::get_elements_string() const {
    std::string result;

    for(const auto& item : this->element_types) {
        result += QString("%1 (%2); ").arg(QString(item.first.c_str())).arg(item.second).toStdString();
    }

    // remove last two characters
    if(result.size() > 2) {
        result.pop_back();
        result.pop_back();
    }

    return result;
}

/**
 * @brief      Get the centering vector
 *
 * @return     Vector that puts unitcell at the origin
 */
QVector3D Structure::get_center_vector() const {
    auto ctr = this->unitcell.transpose() * VectorPosition::Ones() * 0.5;
    return QVector3D(-ctr(0), -ctr(1), -ctr(2));
}

/**
 * @brief      Update data based on contents;
 */
void Structure::update() {
    this->count_elements();
    this->construct_bonds();
    this->build_expansion();
}

/**
 * @brief      Select atom by idx
 *
 * @param[in]  idx   The index
 */
void Structure::select_atom(unsigned int idx) {
    unsigned int select = 0;
    if(idx < this->get_nr_atoms()) {
        this->atoms[idx].select_atom();
        select = this->atoms[idx].select;
    } else {
        this->atoms_expansion[idx - this->get_nr_atoms()].select_atom();
        select = this->atoms_expansion[idx - this->get_nr_atoms()].select;
    }

    if(select == 1) {   // add to first buffer
        this->primary_buffer.push_back(idx);
    } else if(select == 2) { // add to first buffer and remove from first
        this->secondary_buffer.push_back(idx);
        this->primary_buffer.erase(std::remove(this->primary_buffer.begin(), this->primary_buffer.end(), idx), this->primary_buffer.end());
    } else { // remove from second buffer
        this->secondary_buffer.erase(std::remove(this->secondary_buffer.begin(), this->secondary_buffer.end(), idx), this->secondary_buffer.end());
    }
}

/**
 * @brief      Gets the position primary buffer.
 *
 * @return     The position primary buffer.
 */
QVector3D Structure::get_position_primary_buffer() const {
    if(this->primary_buffer.size() == 0) {
        throw std::runtime_error("No atoms in primary buffer. This exception should not be thrown. Please file an issue.");
    }

    QVector3D ctr(0.0, 0.0, 0.0);
    for(unsigned int idx : this->primary_buffer) {
        if(idx >= this->get_nr_atoms()) {
            ctr += this->atoms_expansion[idx - this->get_nr_atoms()].get_pos_qtvec();
        } else {
            ctr += this->atoms[idx].get_pos_qtvec();
        }
    }
    ctr /= (float)this->primary_buffer.size();

    return ctr;
}

/**
 * @brief      Gets the position secondary buffer.
 *
 * @return     The position secondary buffer.
 */
QVector3D Structure::get_position_secondary_buffer() const {
    if(this->secondary_buffer.size() == 0) {
        throw std::logic_error("No atoms in secondary buffer. This exception should not be thrown. Please file an issue.");
    }

    QVector3D ctr(0.0, 0.0, 0.0);
    for(unsigned int idx : this->secondary_buffer) {
        if(idx >= this->get_nr_atoms()) {
            ctr += this->atoms_expansion[idx - this->get_nr_atoms()].get_pos_qtvec();
        } else {
            ctr += this->atoms[idx].get_pos_qtvec();
        }
    }
    ctr /= (float)this->secondary_buffer.size();

    return ctr;
}

/**
 * @brief      Clear the selection_buffers
 */
void Structure::clear_selection() {
    for(unsigned int idx : this->primary_buffer) {
        if(idx >= this->get_nr_atoms()) {
            this->atoms_expansion[idx - this->get_nr_atoms()].select = 0;
        } else {
            this->atoms[idx].select = 0;
        }
    }

    for(unsigned int idx : this->secondary_buffer) {
        if(idx >= this->get_nr_atoms()) {
            this->atoms_expansion[idx - this->get_nr_atoms()].select = 0;
        } else {
            this->atoms[idx].select = 0;
        }
    }

    this->primary_buffer.clear();
    this->secondary_buffer.clear();
}

/**
 * @brief      Select all atoms
 */
void Structure::select_all_atoms() {
    // clear buffers
    this->clear_selection();

    // fill primary buffer with all atoms in the unit cell
    for(unsigned int i=0; i<this->get_nr_atoms(); i++) {
        this->atoms[i].select_atom();
        this->primary_buffer.push_back(i);
    }
}

/**
 * @brief      Invert the selection
 */
void Structure::invert_selection() {
    // make copy of primary selection
    auto list = this->primary_buffer;

    this->select_all_atoms();

    for(unsigned int idx : list) {
        this->atoms[idx].select = 0;
        this->primary_buffer.erase(std::remove(this->primary_buffer.begin(), this->primary_buffer.end(), idx), this->primary_buffer.end());
    }
}

/**
 * @brief      Set frozen
 */
void Structure::set_frozen() {
    for(unsigned int idx : this->primary_buffer) {
        for(unsigned int j=0; j<3; j++) {
            this->atoms[idx].selective_dynamics[j] = false;
        }
    }
}

/**
 * @brief      Set frozen
 */
void Structure::set_unfrozen() {
    for(unsigned int idx : this->primary_buffer) {
        for(unsigned int j=0; j<3; j++) {
            this->atoms[idx].selective_dynamics[j] = true;
        }
    }
}

/**
 * @brief      Get a string containing current selection data
 *
 * @return     The selection string.
 */
QString Structure::get_selection_string() const {
    QString str;

    str += "<b><font color=\"#43f7b5\">P: </font></b>";
    QVector3D ppos;
    try {
        ppos = this->get_position_primary_buffer();
        str += QString("(");
        for(int idx : this->primary_buffer) {
            str += QString("#%1,").arg(idx+1);
        }
        str.remove(str.length()-1, str.length());
        str += QString("); ");
        str += QString("%L1 atoms (%2; %3; %4)<br>").arg(this->primary_buffer.size())
            .arg(QString::number(ppos[0], 'f', 2))
            .arg(QString::number(ppos[1], 'f', 2))
            .arg(QString::number(ppos[2], 'f', 2));
    } catch(const std::exception& e) {
        str += QString("0 atoms<br>");
    }

    str += "<b><font color=\"#ec73ff\">S: </font></b>";
    QVector3D spos;
    try {
        spos = this->get_position_secondary_buffer();
        str += QString("(");
        for(int idx : this->secondary_buffer) {
            str += QString("#%1,").arg(idx+1);
        }
        str.remove(str.length()-1, str.length());
        str += QString("); ");
        str += QString("%L1 atoms (%2; %3; %4)").arg(this->secondary_buffer.size())
            .arg(QString::number(spos[0], 'f', 2))
            .arg(QString::number(spos[1], 'f', 2))
            .arg(QString::number(spos[2], 'f', 2));
    } catch(const std::exception& e) {
        str += QString("0 atoms");
    }

    return str;
}

/**
 * @brief      Count the number of elements
 */
void Structure::count_elements() {
    this->element_types.clear();

    for(const auto& atom : this->atoms) {
        std::string atomname = AtomSettings::get().get_name_from_elnr(atom.atnr);
        auto got = this->element_types.find(atomname);
        if(got != this->element_types.end()) {
            got->second++;
        } else {
            this->element_types.emplace(atomname, 1);
        }
    }
}

/**
 * @brief      Construct the bonds
 */
void Structure::construct_bonds() {
    this->bonds.clear();

    for(unsigned int i=0; i<this->atoms.size(); i++) {
        const auto& atom1 = this->atoms[i];
        for(unsigned int j=i+1; j<this->atoms.size(); j++) {
            const auto& atom2 = this->atoms[j];
            double maxdist2 = AtomSettings::get().get_bond_distance(atom1.atnr, atom2.atnr);

            double dist2 = atom1.dist(atom2);

            // check if atoms are bonded
            if(dist2 < maxdist2) {
                this->bonds.emplace_back(atom1, atom2);
            }
        }
    }
}

/**
 * @brief      Expand unit cell
 */
void Structure::build_expansion() {
    this->atoms_expansion.clear();

    VectorPosition p;
    for(int z=-1; z<=1; z++) {
        p[2] = z;
        for(int y=-1; y<=1; y++) {
            p[1] = y;
            for(int x=-1; x<=1; x++) {
                p[0] = x;
                if(!(x == 0 && y == 0 && z == 0)) {
                    VectorPosition dp = this->unitcell.transpose() * p;
                    for(const auto& atom : this->atoms) {
                        unsigned int atomtype = 0;
                        if(z != 0) {
                            atomtype |= (1 << ATOM_EXPANSION_Z);
                        }

                        if(x != 0 || y != 0) {
                            atomtype |= (1 << ATOM_EXPANSION_XY);
                        }

                        this->atoms_expansion.emplace_back(atom.atnr, atom.x, atom.y, atom.z, atomtype);
                        this->atoms_expansion.back().translate(dp[0], dp[1], dp[2]);
                    }
                }
            }
        }
    }
}

/**
 * @brief      Transpose single atom
 *
 * @param[in]  idx            Atom index
 * @param[in]  transposition  The transposition
 */
void Structure::transpose_atom(unsigned int idx, const QMatrix4x4& transposition) {
    QMatrix4x4 unitcellmatrix(this->get_matrix3x3(this->unitcell));
    auto pos = this->atoms[idx].get_pos_qtvec();
    QVector3D newpos = transposition.map(pos);

    // convert to direct coordinates and replace atom within the unitcell
    QVector3D direct = unitcellmatrix.transposed().inverted().map(newpos);
    for(unsigned int i=0; i<3; i++) {
        direct[i] = std::fmod(direct[i], 1.0f);
        if(direct[i] < 0.0f) {
            direct[i] += 1.0f;
        }
    }

    // calculate back to cartesian coordinates
    newpos = unitcellmatrix.transposed().map(direct);

    // update atom coordinates
    this->atoms[idx].x = newpos[0];
    this->atoms[idx].y = newpos[1];
    this->atoms[idx].z = newpos[2];
}

/**
 * @brief      Gets the unitcell matrix.
 *
 * @param[in]  matrix  The matrix
 *
 * @return     The casted matrix.
 */
QMatrix3x3 Structure::get_matrix3x3(const MatrixUnitcell& matrix) const {
    std::vector<float> values(9, 0.0);
    for(unsigned int i=0; i<3; i++) {
        for(unsigned int j=0; j<3; j++) {
            values[i*3+j] = (float)matrix(i,j);
        }
    }
    return QMatrix3x3(&values[0]);
}
