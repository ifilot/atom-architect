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

#include "structure_loader.h"

/**
 * @brief      Constructs a new instance.
 */
StructureLoader::StructureLoader(){}

/**
 * @brief      Load a file
 *
 * @param[in]  filename  The filename
 *
 * @return     shared ptr to structure object
 */
std::shared_ptr<Structure> StructureLoader::load_file(const std::string& filename) {
    QFileInfo qfi(QString(filename.c_str()));
    std::shared_ptr<Structure> structure;
    if(qfi.fileName().startsWith("POSCAR") || qfi.fileName().startsWith("CONTCAR") || qfi.fileName().endsWith(".vasp")) {
        structure = this->load_poscar(filename);
    } else if(qfi.fileName().startsWith("OUTCAR")) {
        structure = this->load_outcar(filename).back();
    } else if(qfi.completeSuffix() == "geo") {
        structure = this->load_geo(filename);
    }  else if(qfi.completeSuffix() == ".xyz") {
        structure = this->load_xyz(filename);
    }

    if(!structure) {
        std::string fname = qfi.fileName().toStdString();
        throw std::runtime_error("Unrecognised filename: " + fname);
    } else {
        structure->update();
        return structure;
    }
}

/**
 * @brief      Load structure from .geo file
 *
 * @param[in]  filename  The filename
 *
 * @return     Structure
 */
std::shared_ptr<Structure> StructureLoader::load_geo(const std::string& filename) {
    std::ifstream infile(filename);

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    // regex for a line where the first column is a label
    static QRegularExpression regex_atomline("^\\s*[0-9|\\*]+\\s+([A-Za-z]+)\\s+([0-9e.-]+)\\s+([0-9e.-]+)\\s+([0-9e.-]+).*$");

    // regex for three doubles
    static QRegularExpression regex_double3("^\\s*([0-9e.-]+)\\s+([0-9e.-]+)\\s+([0-9e.-]+)\\s*(.*)$");

    std::string line;      // string to store line in

    // skip first lines
    std::getline(infile, line);

    // build unit cell
    std::getline(infile, line);
    QRegularExpressionMatch match1 = regex_double3.match(QString(line.c_str()));
    double a = match1.captured(1).toDouble();
    double b = match1.captured(2).toDouble();
    double c = match1.captured(3).toDouble();

    std::getline(infile, line);
    QRegularExpressionMatch match2 = regex_double3.match(QString(line.c_str()));
    double alpha = match2.captured(1).toDouble();
    double beta = match2.captured(2).toDouble();
    double gamma = match2.captured(3).toDouble();
    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);

    // see this page : https://en.wikipedia.org/wiki/Fractional_coordinates
    double cosalpha = std::cos(M_PI / 180. * alpha);
    double cosbeta = std::cos(M_PI / 180. * beta);
    double cosgamma = std::cos(M_PI / 180. * gamma);
    double singamma = std::sin(M_PI / 180. * gamma);
    double Omega = a * b * c * std::sqrt(1.0 - (cosalpha * cosalpha) - (cosbeta * cosbeta) - (cosgamma * cosgamma) + 2.0 * cosalpha * cosbeta * cosgamma);
    unitcell(0,0) = a;  // align first lattice parameter with the x-axis
    unitcell(1,0) = b * cosgamma;
    unitcell(1,1) = b * singamma;
    unitcell(2,0) = c * cosbeta;
    unitcell(2,1) = c * (cosalpha - cosbeta * cosgamma) / singamma;
    unitcell(2,2) = Omega / (a * b * singamma);

    auto structure = std::make_shared<Structure>(unitcell);

    while(std::getline(infile, line)) {
        // start reading atoms
        try {
            // try to read the first column as the element label
            QRegularExpressionMatch match3 = regex_atomline.match(QString(line.c_str()));
            if(match3.hasMatch()) {
                structure->add_atom(AtomSettings::get().get_atom_elnr(match3.captured(1).toStdString()),
                                   match3.captured(2).toDouble(),
                                   match3.captured(3).toDouble(),
                                   match3.captured(4).toDouble()
                               );
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Could not interpret line: " + line);
        }
    }

    return structure;
}

/**
 * @brief      Load structure from .xyz file
 *
 * @param[in]  filename  The filename
 *
 * @return     Structure
 */
std::shared_ptr<Structure> StructureLoader::load_xyz(const std::string& filename) {
    static QRegularExpression whitespace("\\s+");

    std::ifstream infile(filename);

    // grab number of atoms
    std::string line;
    std::getline(infile, line);
    unsigned int nr_atoms = QString(line.c_str()).toUInt();

    // skip the comment line
    std::getline(infile, line);

    // read atoms
    std::vector<VectorPosition> positions;
    std::vector<unsigned int> elements;
    for(unsigned int i=0; i<nr_atoms; i++) {
        std::getline(infile, line);
        QStringList pieces = QString(line.c_str()).trimmed().split(whitespace);

        elements.push_back(AtomSettings::get().get_atom_elnr(pieces[0].toStdString()));
        double x = pieces[1].toDouble();
        double y = pieces[2].toDouble();
        double z = pieces[3].toDouble();
        positions.emplace_back(x,y,z);
    }

    // calculate center position
    VectorPosition ctr(0,0,0);
    for(const VectorPosition& p : positions) {
        ctr += p;
    }
    ctr /= (double)nr_atoms;

    // center atoms
    for(unsigned int i=0; i<nr_atoms; i++) {
        positions[i] -= ctr;
    }

    // build unit cell based on smallest and largest value
    VectorPosition minv(0,0,0);
    VectorPosition maxv(0,0,0);
    for(unsigned int i=0; i<nr_atoms; i++) {
        for(unsigned int j=0; j<3; j++) {
            minv[j] = std::min(positions[i][j], minv[j]);
            maxv[j] = std::max(positions[i][j], maxv[j]);
        }
    }
    VectorPosition dv = maxv - minv;

    // build unitcell with cell boundaries at 10 A
    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);
    unitcell(0,0) = dv[0] + 10.0;
    unitcell(1,1) = dv[1] + 10.0;
    unitcell(2,2) = dv[2] + 10.0;

    // add atoms to structure
    auto structure = std::make_shared<Structure>(unitcell);
    for(unsigned int i=0; i<nr_atoms; i++) {
        structure->add_atom(elements[i],
                            positions[i][0] + unitcell(0,0) * 0.5,
                            positions[i][1] + unitcell(1,1) * 0.5,
                            positions[i][2] + unitcell(2,2) * 0.5);
    }

    // center the structure
    structure->center();

    return structure;
}

/**
 * @brief      Load structure from POSCAR file
 *
 * @param[in]  filename  The filename
 *
 * @return     Structure
 */
std::shared_ptr<Structure> StructureLoader::load_poscar(const std::string& filename) {
    std::ifstream infile(filename);

    static QRegularExpression whitespace("\\s+");

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    std::string line;

    // skip first line (name of system)
    std::getline(infile, line);

    // read scaling factor
    std::getline(infile, line);
    double scalar = QString(line.c_str()).toDouble();

    // read matrix
    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);
    for(unsigned int j=0; j<3; j++) {
        std::getline(infile, line);
        QStringList pieces = QString(line.c_str()).trimmed().split(whitespace);
        for(unsigned int i=0; i<3; i++) {
            unitcell(j,i) = pieces[i].toDouble();
        }
    }
    unitcell *= scalar;
    auto structure = std::make_shared<Structure>(unitcell);

    // assume that POSCARS are VASP5 POSCAR files
    // TODO add functionality to also allow for VASP4
    std::getline(infile, line);
    static QRegularExpression regex_els("^.*[A-Za-z]+.*$"); // if the line contains even a single non-numeric character, it is a line containing elements
    QRegularExpressionMatch match = regex_els.match(QString(line.c_str()));
    if(!match.hasMatch()) {
        throw std::runtime_error("This file is probably a VASP4 POSCAR file. You can currently only load VASP5+ POSCAR files");
    }
    QStringList elements = QString(line.c_str()).trimmed().split(whitespace);

    // assess whether the element lines contain any "/"; if so prune the part
    // after the slash
    for (QString& el : elements) {
        int slashPos = el.indexOf('/');
        if (slashPos != -1) {
            el = el.left(slashPos);  // keep only part before the '/'
        }
    }

    // get the number for each element
    std::getline(infile, line);
    std::vector<unsigned int> nr_elements;
    QStringList pieces = QString(line.c_str()).trimmed().split(whitespace);
    for(unsigned int i=0; i<pieces.size(); i++) {
        nr_elements.push_back(pieces[i].toUInt());
    }
    if(nr_elements.size() != (size_t)elements.size()) {
        throw std::runtime_error("Array size for element types does not match array size for number for each element type.");
    }

    // check if next line is selective dynamics, if so, skip
    bool selective_dynamics = false;
    std::getline(infile, line);
    static QRegularExpression regex_sd("^\\s*[Ss].*$");
    QRegularExpressionMatch match2 = regex_sd.match(QString(line.c_str()));
    if(match2.hasMatch()) {
        selective_dynamics = true;
        std::getline(infile, line);
    }

    // direct or cartesian
    bool direct = (line[0] == 'D' || line[0] == 'd') ? true : false;

    // collect atoms
    static QRegularExpression regex_double3("^\\s*([0-9e.-]+)\\s+([0-9e.-]+)\\s+([0-9e.-]+)\\s*(.*)$");
    static QRegularExpression regex_double3_bool3("^\\s*([0-9e.-]+)\\s+([0-9e.-]+)\\s+([0-9e.-]+)\\s+([TF])\\s+([TF])\\s+([TF])\\s*(.*)$");

    for(unsigned int i=0; i<elements.size(); i++) {
        unsigned int elid = AtomSettings::get().get_atom_elnr(elements[i].toStdString());
        for(unsigned int j=0; j<nr_elements[i]; j++) {
            std::getline(infile, line);

            if(selective_dynamics) {
                QRegularExpressionMatch match3 = regex_double3_bool3.match(QString(line.c_str()));
                if(match3.hasMatch()) {
                    double x = match3.captured(1).toDouble();
                    double y = match3.captured(2).toDouble();
                    double z = match3.captured(3).toDouble();

                    bool sx = (match3.captured(4) == "F" ? false : true);
                    bool sy = (match3.captured(5) == "F" ? false : true);
                    bool sz = (match3.captured(6) == "F" ? false : true);

                    VectorPosition position(x,y,z);

                    // build coordinates
                    if(direct) {
                        VectorPosition cartesian = unitcell.transpose() * position;
                        structure->add_atom(elid, cartesian(0), cartesian(1), cartesian(2), sx, sy, sz);
                    } else {
                        structure->add_atom(elid, position(0), position(1), position(2), sx, sy, sz);
                    }
                }
            } else {
                QRegularExpressionMatch match3 = regex_double3.match(QString(line.c_str()));
                if(match3.hasMatch()) {
                    double x = match3.captured(1).toDouble();
                    double y = match3.captured(2).toDouble();
                    double z = match3.captured(3).toDouble();

                    VectorPosition position(x,y,z);

                    // build coordinates
                    if(direct) {
                        VectorPosition cartesian = unitcell.transpose() * position;
                        structure->add_atom(elid, cartesian(0), cartesian(1), cartesian(2));
                    } else {
                        structure->add_atom(elid, position(0), position(1), position(2));
                    }
                }
            }
        }
    }

    return structure;
}

/**
 * @brief      Load structure from OUTCAR file
 *
 * @param[in]  filename  The filename
 *
 * @return     Structure
 */
std::vector<std::shared_ptr<Structure>> StructureLoader::load_outcar(const std::string& filename) {
    qDebug() << "Loading OUTCAR: " << QString(filename.c_str());
    std::ifstream infile(filename);

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    // vasp version
    unsigned int vasp_version = 0;

    // current reading state
    unsigned int readstate = 0;
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS);
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT);
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_OPEN);

    // number of atoms and number of states
    unsigned int nr_atoms = 0;
    unsigned int nr_states = 0;

    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);
    std::vector<double> energies;
    std::vector<std::string> elements;
    std::vector<unsigned int> nr_atoms_per_elm;

    /*
    * Define all the regex patterns
    */
    static QRegularExpression whitespace("\\s+");
    static QRegularExpression regex_vasp_version("^\\s*vasp.([0-9]).([0-9]+).([0-9]+).*$");
    static QRegularExpression regex_element("^\\s*(VRHFIN\\s+=)([A-Za-z]+)\\s*:.*$");
    static QRegularExpression regex_ions_per_element("^\\s*(ions per type =\\s+)([0-9 ]+)\\s*$");
    static QRegularExpression regex_lattice_vectors("^\\s*direct lattice vectors.*$");
    static QRegularExpression regex_atoms("^\\s*POSITION.*$");
    static QRegularExpression regex_grab_numbers("^\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+).*$");
    static QRegularExpression regex_grab_energy("^\\s+energy  without entropy=\\s+([0-9.-]+)\\s+energy\\(sigma->0\\) =\\s+([0-9.-]+).*$");

    std::string line;

    std::vector<std::shared_ptr<Structure>> structures;

    while(std::getline(infile, line)) { // loop over all the lines in the file
        /*
         * Collect the vasp version (4 or 5)
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS) ) {
            // get the elements and put these in an array
            QRegularExpressionMatch match = regex_vasp_version.match(QString(line.c_str()));
            if (match.hasMatch()) {

                vasp_version = match.captured(1).toUInt();
                unsigned int version_major = match.captured(2).toUInt();
                unsigned int version_minor = match.captured(3).toUInt();

                qDebug() << "Detected VASP: " << vasp_version << "." << version_major << "." << version_minor;

                continue;
            }
        }

        /*
         * Collect the elements
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS) ) {
            QRegularExpressionMatch match = regex_element.match(QString(line.c_str()));
            if (match.hasMatch()) {
                elements.push_back(match.captured(2).toStdString());

                qDebug() << "Captured element:" << match.captured(2);

                continue;
            }
        }

        /*
         * Collect the number of ions of each element type
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT) ) {

            QRegularExpressionMatch match = regex_ions_per_element.match(QString(line.c_str()));
            if (match.hasMatch()) {
                QString subline = match.captured(2);
                QStringList pieces = subline.split(whitespace);
                for(unsigned int i=0; i<pieces.size(); i++) {
                    nr_atoms_per_elm.push_back(pieces[i].toUInt());
                    nr_atoms += pieces[i].toUInt();
                }

                // remove ions state and elements state
                readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS);
                readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT);
                readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS);

                // check if a vasp version has been identified, if not, terminate
                if(!(vasp_version == 4 || vasp_version == 5 || vasp_version == 6)) {
                    throw std::runtime_error("Invalid VASP version encountered: " + QString::number(vasp_version).toStdString());
                }

                continue;
            }
        }

        /*
         * Collect the dimensions of the unit cell. Note that if an IBRION=3 calculation is
         * being run, this is not gathered by this class. It is assumed that each state
         * has the same unit cell. (that means, IBRION != 3 calculations)
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS) ) {
            // get the dimensionality of the unit cell
            QRegularExpressionMatch match = regex_lattice_vectors.match(QString(line.c_str()));
            if (match.hasMatch()) {

                // grab next tree lines
                for(unsigned int i=0; i<3; i++) {
                    std::getline(infile, line);
                    QRegularExpressionMatch match2 = regex_grab_numbers.match(QString(line.c_str()));
                    if (match2.hasMatch()) {
                        unitcell(i,0) = match2.captured(1).toDouble();
                        unitcell(i,1) = match2.captured(2).toDouble();
                        unitcell(i,2) = match2.captured(3).toDouble();
                    }
                }

                readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS);
                readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS);

                continue;
            }
        }

        /*
         * Collect the energy of the state
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS) ) {
            QRegularExpressionMatch match = regex_grab_energy.match(QString(line.c_str()));
            if (match.hasMatch()) {

                energies.push_back(match.captured(2).toDouble());

                if(vasp_version == 5) {
                    nr_states++;
                }

                continue;
            }
        }

        /*
         * Collect the atomic positions and forces for this state
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS) ) {

            QRegularExpressionMatch match = regex_atoms.match(QString(line.c_str()));
            if (match.hasMatch()) {
                std::getline(infile, line); // skip dashed line

                // create a new structure
                structures.push_back(std::make_shared<Structure>(unitcell));

                for(unsigned i=0; i<nr_atoms_per_elm.size(); i++) {
                    for(unsigned int j=0; j<nr_atoms_per_elm[i]; j++) {
                        std::getline(infile, line);
                        QRegularExpressionMatch match2 = regex_grab_numbers.match(QString(line.c_str()));
                        if (match2.hasMatch()) {
                            double x = match2.captured(1).toDouble();
                            double y = match2.captured(2).toDouble();
                            double z = match2.captured(3).toDouble();
                            double fx = match2.captured(4).toDouble();
                            double fy = match2.captured(5).toDouble();
                            double fz = match2.captured(6).toDouble();

                            unsigned int atnr = AtomSettings::get().get_atom_elnr(elements[i]);
                            structures.back()->add_atom(atnr, x, y, z, fx, fy, fz);
                        }
                    }
                }

                if(vasp_version == 4) {
                    nr_states++;
                }

                continue;
            }
        }
    }

    // energies are sometimes given either before or after the coordinates, hence, only
    // set the energies after everything has been parsed
    if(energies.size() != structures.size()) {
        throw std::runtime_error("Number of energies does not match number of structures.");
    }

    for(unsigned int i=0; i<energies.size(); i++) {
        structures[i]->set_energy(energies[i]);
    }

    return structures;
}

/**
 * @brief      Load NEB binary
 *
 * @param[in]  filename  The filename
 *
 * @return     Bundled set of structures
 */
std::vector<std::vector<std::shared_ptr<Structure>>> StructureLoader::load_neb_bin(const std::string& filename) {
    std::ifstream infile(filename, std::ios::in | std::ios::binary);

    uint32_t datatype = 0;
    uint32_t nr_images = 0;
    uint32_t nr_structures = 0;
    uint32_t nr_atoms = 0;

    infile.read((char*)&datatype, sizeof(uint32_t));
    infile.read((char*)&nr_images, sizeof(uint32_t));
    infile.read((char*)&nr_structures, sizeof(uint32_t));
    infile.read((char*)&nr_atoms, sizeof(uint32_t));

    if(datatype != 1) {
        throw std::runtime_error("Invalid datatype of binary package.");
    }

    std::vector<std::vector<std::shared_ptr<Structure>>> images(nr_images);
    for(unsigned int i=0; i<nr_images; i++) {
        for(unsigned int j=0; j<nr_structures; j++) {

            // read unitcell
            MatrixUnitcell mat = MatrixUnitcell::Zero(3,3);
            for(unsigned int k=0; k<3; k++) {
                for(unsigned int l=0; l<3; l++) {
                    double val = 0.0;
                    infile.read((char*)&val, sizeof(double));
                    mat(k,l) = val;
                }
            }

            // read energy
            double energy = 0.0;
            infile.read((char*)&energy, sizeof(double));

            images[i].push_back(std::make_shared<Structure>(mat));
            images[i].back()->set_energy(energy);

            // read atoms
            for(unsigned int a=0; a<nr_atoms; a++) {
                uint8_t atid;
                double x, y, z, fx, fy, fz;

                infile.read((char*)&atid, sizeof(uint8_t));
                infile.read((char*)&x, sizeof(double));
                infile.read((char*)&y, sizeof(double));
                infile.read((char*)&z, sizeof(double));
                infile.read((char*)&fx, sizeof(double));
                infile.read((char*)&fy, sizeof(double));
                infile.read((char*)&fz, sizeof(double));

                images[i].back()->add_atom(atid, x, y, z, fx, fy, fz);
            }
        }
    }

    return images;
}
