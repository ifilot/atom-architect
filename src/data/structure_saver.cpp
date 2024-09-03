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
#include "structure_saver.h"

StructureSaver::StructureSaver() {

}

/**
 * @brief      Saves structures as a poscar.
 *
 * @param      structure  The structure
 */
void StructureSaver::save_poscar(const Structure* structure, const std::string& filename) {
    const std::string path = filename;
    std::ofstream outfile(path.c_str());

    // Prepare numeric output format; everything is direct via the "numform" variable, which
    // specifies how floating point numbers should be outputted. Numform is used to generate
    // 'sprintf-strings', which in turn are used to produce the atomic coordinate and unit
    // cell representations in the POSCAR file.
    static const size_t BUFSIZE = 64;
    char buf[BUFSIZE]; // buffer used for sprintf
    static const char numform[] = "%12.6f";
    sprintf(buf, "  %s  %s  %s\n", numform, numform, numform);
    std::string output3f(buf);
    sprintf(buf, "  %s  %s  %s  %%c  %%c  %%c\n", numform, numform, numform);
    std::string output3f3b(buf);

    // sort by atom types
    std::vector<unsigned int> elnrs;
    std::vector<unsigned int> nrs;

    for(unsigned int i=0; i<structure->get_nr_atoms(); i++) {
        unsigned int elnr = structure->get_atom(i).atnr;
        bool flag = false;
        for(unsigned int j=0; j<elnrs.size(); j++) {
            if(elnrs[j] == elnr) {
                flag = true;
                nrs[j]++;
                break;
            }
        }
        if(!flag) {
            elnrs.push_back(elnr);
            nrs.push_back(1);
        }
    }

    if(outfile.is_open()) {
        // comment line
        outfile << "VASP POSCAR" << std::endl;

        // scalar
        outfile << "1.0000000" << std::endl;

        // unitcell
        MatrixUnitcell unitcell = structure->get_unitcell();
        for(unsigned int i=0; i<3; i++) {
            size_t nrwritten = sprintf(buf, output3f.c_str(),
                    unitcell(i,0),
                    unitcell(i,1),
                    unitcell(i,2)
            );
            if(nrwritten > BUFSIZE) {
                throw std::runtime_error("Buffer overflow detected.");
            }
            outfile << std::string(buf);
        }

        // elements
        for(unsigned int i=0; i<elnrs.size(); i++) {
            outfile << "  " << AtomSettings::get().get_name_from_elnr(elnrs[i]);
        }
        outfile << std::endl;

        // print number of atoms per type
        for(unsigned int i=0; i<nrs.size(); i++) {
            outfile << "  " << nrs[i];
        }
        outfile << std::endl;

        // check if there are atoms which have selective dynamics, if even a single has restricted dynamics,
        // the tag needs to be enabled to limit the movement of that atom
        bool flag_selective_dynamics = false;
        for(const Atom& atom : structure->get_atoms()) {
            if(atom.selective_dynamics[0] == false || atom.selective_dynamics[1] == false || atom.selective_dynamics[2] == false) {
                flag_selective_dynamics = true;
                break;
            }
        }

        // add selective dynamics tag if there is a single atom that has selective dynamics
        if(flag_selective_dynamics) {
            outfile << "Selective dynamics" << std::endl;
        }

        // write direct coordinates
        outfile << "Direct" << std::endl;

        // print atoms
        // TODO: Use "multiarg"
        for(unsigned int i=0; i<elnrs.size(); i++) {
            for(const Atom& atom : structure->get_atoms()) {
                if(atom.atnr == elnrs[i]) {
                    VectorPosition directpos = unitcell.inverse().transpose() * atom.get_pos_eigen();
                    if(flag_selective_dynamics) {
                        size_t nrwritten = sprintf(buf, output3f3b.c_str(),
                            directpos[0],
                            directpos[1],
                            directpos[2],
                            atom.selective_dynamics[0] ? 'T' : 'F',
                            atom.selective_dynamics[1] ? 'T' : 'F',
                            atom.selective_dynamics[2] ? 'T' : 'F'
                        );
                        if(nrwritten > BUFSIZE) {
                            throw std::runtime_error("Buffer overflow detected.");
                        }
                        outfile << std::string(buf);
                    } else {
                        size_t nrwritten = sprintf(buf, output3f.c_str(),
                                directpos[0],
                                directpos[1],
                                directpos[2]
                        );
                        if(nrwritten > BUFSIZE) {
                            throw std::runtime_error("Buffer overflow detected.");
                        }
                        outfile << std::string(buf);
                    }
                }
            }
        }

    } else {
        throw std::runtime_error("Could not open path for writing.");
        return;
    }
}
