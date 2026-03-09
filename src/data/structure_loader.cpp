/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2026 Ivo Filot <i.a.w.filot@tue.nl>                 *
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

#include <Eigen/Eigenvalues>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>

namespace {
/**
 * @brief leading_spaces.
 *
 * @param value Parameter value.
 */
int leading_spaces(const std::string& value) {
    int count = 0;
    while(count < (int)value.size() && value[(size_t)count] == ' ') {
        count++;
    }
    return count;
}

/**
 * @brief sorted_mae.
 *
 * @param lhs Parameter lhs.
 * @param rhs Parameter rhs.
 */
double sorted_mae(const std::vector<double>& lhs,
                  const std::vector<double>& rhs) {
    const size_t n = std::min(lhs.size(), rhs.size());
    if(n == 0) {
        return std::numeric_limits<double>::infinity();
    }

    std::vector<double> a(lhs.begin(), lhs.begin() + n);
    std::vector<double> b(rhs.begin(), rhs.begin() + n);
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    double mae = 0.0;
    for(size_t i=0; i<n; i++) {
        mae += std::abs(a[i] - b[i]);
    }

    return mae / (double)n;
}

/**
 * @brief contains_token.
 *
 * @param line Parameter line.
 * @param token Parameter token.
 */
inline bool contains_token(const std::string& line, const char* token) {
    return line.find(token) != std::string::npos;
}

/**
 * @brief parse_six_columns.
 *
 * @param line Parameter line.
 * @param c1 Parameter c1.
 * @param c2 Parameter c2.
 * @param c3 Parameter c3.
 * @param c4 Parameter c4.
 * @param c5 Parameter c5.
 * @param c6 Parameter c6.
 */
inline bool parse_six_columns(const std::string& line,
                              double& c1,
                              double& c2,
                              double& c3,
                              double& c4,
                              double& c5,
                              double& c6)
{
    const char* p = line.c_str();
    char* end;

    c1 = std::strtod(p, &end); if (p == end) return false; p = end;
    c2 = std::strtod(p, &end); if (p == end) return false; p = end;
    c3 = std::strtod(p, &end); if (p == end) return false; p = end;
    c4 = std::strtod(p, &end); if (p == end) return false; p = end;
    c5 = std::strtod(p, &end); if (p == end) return false; p = end;
    c6 = std::strtod(p, &end); if (p == end) return false;

    return true;
}

}

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
    qDebug() << "Building structure via StructureLoader";
    QFileInfo qfi(QString(filename.c_str()));

    std::shared_ptr<Structure> structure;

    if(qfi.fileName().startsWith("POSCAR") || 
       qfi.fileName().startsWith("CONTCAR") || 
       qfi.completeSuffix() == "vasp") {
        structure = this->load_poscar(filename);
    } else if(qfi.fileName().startsWith("OUTCAR")) {
        structure = this->load_outcar(filename).back();
    } else if(qfi.completeSuffix() == "geo") {
        structure = this->load_geo(filename);
    }  else if(qfi.completeSuffix() == "xyz") {
        qDebug() << "Opening .xyz file";
        structure = this->load_xyz(filename);
    } else if(qfi.completeSuffix() == "yaml" || qfi.completeSuffix() == "yml") {
        structure = this->load_yaml(filename).back();
    }

    if(!structure) {
        std::string fname = qfi.fileName().toStdString();
        throw std::runtime_error("Unrecognized filename: " + fname);
    } else {
        structure->update();
        return structure;
    }
}

    /**
     * @brief      Load structure + vibrational data from pymkmkit YAML file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structures
     */
std::vector<std::shared_ptr<Structure>> StructureLoader::load_yaml(const std::string& filename) {
    qDebug() << "Loading PyMKMKit YAML file: " << QString(filename.c_str());

    std::ifstream infile(filename);
    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    enum class ParseMode {
        None,
        Lattice,
        Coordinates,
        Frequencies,
        DofLabels,
        Hessian
    };

    ParseMode mode = ParseMode::None;

    std::vector<std::array<double, 3>> lattice_vectors;
    struct ParsedAtom {
        std::string element;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };
    std::vector<ParsedAtom> coordinates_direct;
    std::vector<double> frequencies_cm1;
    std::vector<std::string> dof_labels;
    std::vector<std::vector<double>> hessian_rows;
    std::vector<double> current_hessian_row;

    bool has_energy = false;
    double electronic_energy = 0.0;
    bool has_pymkmkit_token = false;

    static const QRegularExpression regex_number("[-+]?(?:[0-9]*\\.[0-9]+|[0-9]+)(?:[eE][-+]?[0-9]+)?");
    static const QRegularExpression regex_dof("^([0-9]+)([XYZ])$");

    std::string line;
    while(std::getline(infile, line)) {
        const QString qline = QString::fromStdString(line);
        const QString trimmed = qline.trimmed();
        const int indent = leading_spaces(line);

        if(trimmed.isEmpty()) {
            continue;
        }

        if(trimmed == "pymkmkit:") {
            has_pymkmkit_token = true;
            continue;
        }

        if(trimmed == "lattice_vectors:") {
            mode = ParseMode::Lattice;
            continue;
        }
        if(trimmed == "coordinates_direct:") {
            mode = ParseMode::Coordinates;
            continue;
        }
        if(trimmed == "frequencies_cm-1:") {
            mode = ParseMode::Frequencies;
            continue;
        }
        if(trimmed == "dof_labels:") {
            mode = ParseMode::DofLabels;
            continue;
        }
        if(trimmed == "matrix:") {
            mode = ParseMode::Hessian;
            continue;
        }

        if(trimmed.startsWith("electronic:")) {
            const QString value = trimmed.section(':', 1).trimmed();
            electronic_energy = value.toDouble();
            has_energy = true;
            continue;
        }

        if(trimmed.endsWith(":") &&
           trimmed != "lattice_vectors:" &&
           trimmed != "coordinates_direct:" &&
           trimmed != "frequencies_cm-1:" &&
           trimmed != "dof_labels:" &&
           trimmed != "matrix:") {
            mode = ParseMode::None;
            continue;
        }

        switch(mode) {
            case ParseMode::Lattice: {
                if(!trimmed.startsWith("-")) {
                    break;
                }

                auto it = regex_number.globalMatch(trimmed);
                std::array<double, 3> parsed_values = {0.0, 0.0, 0.0};
                size_t parsed_count = 0;
                while(it.hasNext() && parsed_count < 3) {
                    parsed_values[parsed_count++] = it.next().captured(0).toDouble();
                }

                if(parsed_count != 3) {
                    throw std::runtime_error("Invalid lattice vector encountered in YAML file.");
                }

                lattice_vectors.push_back(parsed_values);
                break;
            }

            case ParseMode::Coordinates: {
                if(!trimmed.startsWith("- ")) {
                    break;
                }

                const QString data = trimmed.mid(2).trimmed();
                const QStringList parts = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if(parts.size() < 4) {
                    throw std::runtime_error("Invalid coordinates_direct line in YAML file.");
                }

                coordinates_direct.push_back({
                    parts[0].toStdString(),
                    parts[1].toDouble(),
                    parts[2].toDouble(),
                    parts[3].toDouble()
                });
                break;
            }

            case ParseMode::Frequencies: {
                if(trimmed.startsWith("- ")) {
                    frequencies_cm1.push_back(trimmed.mid(2).trimmed().toDouble());
                }
                break;
            }

            case ParseMode::DofLabels: {
                if(trimmed.startsWith("- ")) {
                    dof_labels.push_back(trimmed.mid(2).trimmed().toStdString());
                }
                break;
            }

            case ParseMode::Hessian: {
                if(!trimmed.startsWith("-")) {
                    break;
                }

                auto it = regex_number.globalMatch(trimmed);
                if(!it.hasNext()) {
                    break;
                }

                const double value = it.next().captured(0).toDouble();

                // New Hessian row starts with the outer sequence item at 4 spaces indentation.
                if(indent <= 4 && !current_hessian_row.empty()) {
                    hessian_rows.push_back(current_hessian_row);
                    current_hessian_row.clear();
                }

                current_hessian_row.push_back(value);
                break;
            }

            case ParseMode::None:
            default:
                break;
        }
    }

    if(!current_hessian_row.empty()) {
        hessian_rows.push_back(current_hessian_row);
    }

    if(!has_pymkmkit_token) {
        throw std::runtime_error("Invalid PyMKMKit YAML file: missing 'pymkmkit' token.");
    }

    if(lattice_vectors.size() != 3) {
        throw std::runtime_error("YAML file does not contain exactly 3 lattice vectors.");
    }

    if(coordinates_direct.empty()) {
        throw std::runtime_error("YAML file does not contain any coordinates_direct atoms.");
    }

    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);
    for(int row=0; row<3; row++) {
        // Keep the same row-wise lattice convention as OUTCAR/POSCAR loaders.
        // Each YAML lattice_vectors entry is one direct-lattice vector [x, y, z].
        unitcell(row, 0) = lattice_vectors[(size_t)row][0];
        unitcell(row, 1) = lattice_vectors[(size_t)row][1];
        unitcell(row, 2) = lattice_vectors[(size_t)row][2];
    }

    auto structure = std::make_shared<Structure>(unitcell);
    for(const auto& atom : coordinates_direct) {
        const unsigned int atnr = AtomSettings::get().get_atom_elnr(atom.element);
        VectorPosition direct(atom.x, atom.y, atom.z);
        // lattice vectors are stored row-wise (POSCAR convention in this codebase),
        // hence direct->Cartesian uses unitcell.transpose() * fractional_vector.
        VectorPosition cart = unitcell.transpose() * direct;
        structure->add_atom(atnr, cart(0), cart(1), cart(2));
    }

    if(has_energy) {
        structure->set_energy(electronic_energy);
    }

    if(!dof_labels.empty() && !hessian_rows.empty()) {
        const size_t n = dof_labels.size();
        if(hessian_rows.size() != n) {
            throw std::runtime_error("YAML Hessian matrix row count does not match number of DOF labels.");
        }

        std::vector<int> dof_atom_indices;
        std::vector<QChar> dof_axes;
        std::vector<double> dof_masses_amu;
        dof_atom_indices.reserve(n);
        dof_axes.reserve(n);
        dof_masses_amu.reserve(n);

        for(size_t dof_idx=0; dof_idx<n; dof_idx++) {
            QRegularExpressionMatch label_match = regex_dof.match(QString::fromStdString(dof_labels[dof_idx]));
            if(!label_match.hasMatch()) {
                throw std::runtime_error("Could not parse DOF label: " + dof_labels[dof_idx]);
            }

            const int atom_index = label_match.captured(1).toInt() - 1; // YAML labels are 1-based
            if(atom_index < 0 || atom_index >= (int)coordinates_direct.size()) {
                throw std::runtime_error("DOF label atom index out of bounds: " + dof_labels[dof_idx]);
            }

            const std::string& element = coordinates_direct[(size_t)atom_index].element;
            dof_atom_indices.push_back(atom_index);
            dof_axes.push_back(label_match.captured(2)[0]);
            dof_masses_amu.push_back(AtomSettings::get().get_atom_mass(element));
        }

        // YAML Hessian values are expected in eV/Å^2. Convert to a mass-weighted dynamical matrix.
        Eigen::MatrixXd dynmat((int)n, (int)n);
        for(size_t i=0; i<n; i++) {
            if(hessian_rows[i].size() != n) {
                throw std::runtime_error("YAML Hessian matrix is not square.");
            }

            for(size_t j=0; j<n; j++) {
                const double h_ij = hessian_rows[i][j];
                dynmat((int)i, (int)j) = h_ij / std::sqrt(dof_masses_amu[i] * dof_masses_amu[j]);
            }
        }

        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(dynmat);
        if(solver.info() != Eigen::Success) {
            throw std::runtime_error("Eigen decomposition failed for YAML Hessian matrix.");
        }

        // sqrt(eV/Å^2/amu) -> THz
        constexpr double EV_TO_J = 1.602176634e-19;
        constexpr double ANGSTROM_TO_M = 1.0e-10;
        constexpr double AMU_TO_KG = 1.66053906660e-27;
        constexpr double TWO_PI = 2.0 * M_PI;
        constexpr double SQRT_UNIT_TO_HZ =
            std::sqrt(EV_TO_J / (ANGSTROM_TO_M * ANGSTROM_TO_M * AMU_TO_KG)) / TWO_PI;
        constexpr double HZ_TO_THZ = 1.0e-12;

        std::vector<double> computed_freq_cm1;
        std::vector<double> computed_freq_cm1_flipped;
        computed_freq_cm1.reserve(n);
        computed_freq_cm1_flipped.reserve(n);

        size_t negative_count_normal = 0;
        size_t negative_count_flipped = 0;

        for(size_t mode_idx=0; mode_idx<n; mode_idx++) {
            const double eig = solver.eigenvalues()((int)mode_idx);
            const double signed_cm1 = (eig >= 0.0 ? 1.0 : -1.0) * std::sqrt(std::abs(eig)) * SQRT_UNIT_TO_HZ * HZ_TO_THZ * 33.35640951981521;
            const double signed_cm1_flipped = -signed_cm1;

            computed_freq_cm1.push_back(signed_cm1);
            computed_freq_cm1_flipped.push_back(signed_cm1_flipped);

            if(signed_cm1 < -1e-8) {
                negative_count_normal++;
            }
            if(signed_cm1_flipped < -1e-8) {
                negative_count_flipped++;
            }
        }

        bool flip_frequency_sign = false;
        if(!frequencies_cm1.empty()) {
            const double mae_normal = sorted_mae(computed_freq_cm1, frequencies_cm1);
            const double mae_flipped = sorted_mae(computed_freq_cm1_flipped, frequencies_cm1);
            flip_frequency_sign = (mae_flipped + 1e-12 < mae_normal);

            qDebug() << "PyMKMKit YAML reconstructed frequency MAE (cm^-1), normal sign:" << mae_normal
                     << ", flipped sign:" << mae_flipped
                     << ", selected:" << (flip_frequency_sign ? "flipped" : "normal");
        } else {
            flip_frequency_sign = (negative_count_flipped < negative_count_normal);
            qDebug() << "PyMKMKit YAML sign heuristic based on negative-mode count, normal/flipped:"
                     << negative_count_normal << "/" << negative_count_flipped
                     << ", selected:" << (flip_frequency_sign ? "flipped" : "normal");
        }

        structure->clear_eigenmodes();

        for(size_t mode_idx=0; mode_idx<n; mode_idx++) {
            std::vector<QVector3D> mode_vectors(coordinates_direct.size(), QVector3D(0.0, 0.0, 0.0));

            for(size_t dof_idx=0; dof_idx<n; dof_idx++) {
                const int atom_index = dof_atom_indices[dof_idx];
                const QChar axis = dof_axes[dof_idx];

                // Convert from mass-weighted eigenvector component to Cartesian displacement.
                const double amplitude = solver.eigenvectors()((int)dof_idx, (int)mode_idx) /
                                         std::sqrt(dof_masses_amu[dof_idx]);

                if(axis == 'X') {
                    mode_vectors[(size_t)atom_index].setX(amplitude);
                } else if(axis == 'Y') {
                    mode_vectors[(size_t)atom_index].setY(amplitude);
                } else if(axis == 'Z') {
                    mode_vectors[(size_t)atom_index].setZ(amplitude);
                }
            }

            const double freq_cm1 = flip_frequency_sign ? computed_freq_cm1_flipped[mode_idx]
                                                        : computed_freq_cm1[mode_idx];
            const double freq_thz = freq_cm1 / 33.35640951981521;

            structure->add_eigenmode(freq_thz, mode_vectors);
        }
    }

    return {structure};
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

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

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
    if (!infile) {
        throw std::runtime_error("Could not open " + filename);
    }

    static const QRegularExpression whitespace("\\s+");

    std::string line;

    // ─────────────────────────────────────────────
    // 1. Header & scaling
    // ─────────────────────────────────────────────
    std::getline(infile, line); // system name (ignored)

    std::getline(infile, line);
    const double scale = QString::fromStdString(line).toDouble();

    // ─────────────────────────────────────────────
    // 2. Lattice vectors
    // ─────────────────────────────────────────────
    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3, 3);

    for (int row = 0; row < 3; ++row) {
        std::getline(infile, line);
        const QStringList parts =
            QString::fromStdString(line).trimmed().split(whitespace);

        if (parts.size() != 3) {
            throw std::runtime_error("Invalid lattice vector in POSCAR");
        }

        for (int col = 0; col < 3; ++col) {
            unitcell(row, col) = parts[col].toDouble();
        }
    }

    unitcell *= scale;
    auto structure = std::make_shared<Structure>(unitcell);

    // ─────────────────────────────────────────────
    // 3. Element labels (VASP 5 / 6.4+)
    // ─────────────────────────────────────────────
    std::getline(infile, line);
    const QStringList rawLabels =
        QString::fromStdString(line).trimmed().split(whitespace);

    // Detect VASP4-style POSCAR (no element symbols)
    bool looksLikeVasp5 = false;
    for (const QString& tok : rawLabels) {
        bool ok;
        tok.toDouble(&ok);
        if (!ok) {
            looksLikeVasp5 = true;
            break;
        }
    }

    if (!looksLikeVasp5) {
        throw std::runtime_error(
            "VASP4 POSCAR detected. Only VASP5+ POSCAR files are supported.");
    }

    // Strip everything after '/' (VASP 6.4 labels)
    QStringList elements;
    elements.reserve(rawLabels.size());

    for (const QString& label : rawLabels) {
        const int slash = label.indexOf('/');
        elements.push_back(slash >= 0 ? label.left(slash) : label);
    }

    // ─────────────────────────────────────────────
    // 4. Element counts
    // ─────────────────────────────────────────────
    std::getline(infile, line);
    const QStringList countTokens =
        QString::fromStdString(line).trimmed().split(whitespace);

    if (countTokens.size() != elements.size()) {
        throw std::runtime_error(
            "Mismatch between number of elements and atom counts");
    }

    std::vector<unsigned int> counts;
    counts.reserve(countTokens.size());

    for (const QString& tok : countTokens) {
        counts.push_back(tok.toUInt());
    }

    // ─────────────────────────────────────────────
    // 5. Selective dynamics & coordinate mode
    // ─────────────────────────────────────────────
    std::getline(infile, line);

    bool selectiveDynamics = false;
    if (!line.empty() && (line[0] == 'S' || line[0] == 's')) {
        selectiveDynamics = true;
        std::getline(infile, line);
    }

    const bool direct =
        (!line.empty() && (line[0] == 'D' || line[0] == 'd'));

    // ─────────────────────────────────────────────
    // 6. Atom positions
    // ─────────────────────────────────────────────
    static const QRegularExpression xyz(
        "^\\s*([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s*(.*)$");

    static const QRegularExpression xyzTF(
        "^\\s*([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)"
        "\\s+([TF])\\s+([TF])\\s+([TF]).*$");

    for (int i = 0; i < elements.size(); ++i) {
        const unsigned int elid =
            AtomSettings::get().get_atom_elnr(elements[i].toStdString());

        for (unsigned int n = 0; n < counts[i]; ++n) {
            std::getline(infile, line);
            const QString qline = QString::fromStdString(line);

            double x, y, z;
            bool sx = true, sy = true, sz = true;

            if (selectiveDynamics) {
                const auto m = xyzTF.match(qline);
                if (!m.hasMatch()) {
                    throw std::runtime_error("Invalid atomic position line");
                }

                x = m.captured(1).toDouble();
                y = m.captured(2).toDouble();
                z = m.captured(3).toDouble();
                sx = (m.captured(4) == "T");
                sy = (m.captured(5) == "T");
                sz = (m.captured(6) == "T");
            } else {
                const auto m = xyz.match(qline);
                if (!m.hasMatch()) {
                    throw std::runtime_error("Invalid atomic position line");
                }

                x = m.captured(1).toDouble();
                y = m.captured(2).toDouble();
                z = m.captured(3).toDouble();
            }

            VectorPosition pos(x, y, z);
            if (direct) {
                pos = unitcell.transpose() * pos;
            }

            structure->add_atom(elid, pos(0), pos(1), pos(2), sx, sy, sz);
        }
    }

    return structure;
}

    /**
     * @brief      Load structure from OUTCAR file
     *
     * @param[in]  filename  The filename
     *
     * @return     Structures
     */
std::vector<std::shared_ptr<Structure>> StructureLoader::load_outcar(const std::string& filename) {
    qDebug() << "Loading OUTCAR: " << QString(filename.c_str());
    std::ifstream infile(filename);

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    struct ParsedEigenmode {
        double eigenvalue = 0.0;
        std::vector<QVector3D> eigenvectors;
    };

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
    std::vector<ParsedEigenmode> parsed_eigenmodes;

    bool reading_mode_eigenvectors = false;

    /*
    * Define all the regex patterns
    */
    static QRegularExpression whitespace("\\s+");
    static QRegularExpression regex_vasp_version("^\\s*vasp.([0-9]).([0-9]+).([0-9]+).*$");
    static QRegularExpression regex_element("^\\s*(VRHFIN\\s+=)([A-Za-z]+)\\s*:.*$");
    static QRegularExpression regex_ions_per_element("^\\s*(ions per type =\\s+)([0-9 ]+)\\s*$");
    static QRegularExpression regex_lattice_vectors("^\\s*direct lattice vectors.*$");
    static QRegularExpression regex_atoms("^\\s*POSITION.*$");
    static QRegularExpression regex_grab_energy("^\\s+energy  without entropy=\\s+([0-9.-]+)\\s+energy\\(sigma->0\\) =\\s+([0-9.-]+).*$");
    static QRegularExpression regex_frequency_mode("^\\s*[0-9]+\\s+f(/i)?\\s*=\\s*([0-9eE.+-]+)\\s+THz.*$");
    static QRegularExpression regex_frequency_eigenvector(
        "^\\s*(?:[0-9]+\\s+)?([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s+([0-9eE.+-]+)\\s*$");

    std::string line;

    std::vector<std::shared_ptr<Structure>> structures;

    while(std::getline(infile, line)) { // loop over all the lines in the file
        const QString qline(line.c_str());

        if(reading_mode_eigenvectors) {
            QRegularExpressionMatch mode_vector_match = regex_frequency_eigenvector.match(qline);
            if(mode_vector_match.hasMatch()) {
                parsed_eigenmodes.back().eigenvectors.emplace_back(
                    mode_vector_match.captured(4).toDouble(),
                    mode_vector_match.captured(5).toDouble(),
                    mode_vector_match.captured(6).toDouble()
                );

                if(parsed_eigenmodes.back().eigenvectors.size() == nr_atoms) {
                    reading_mode_eigenvectors = false;
                    qDebug() << "Completed eigenmode" << parsed_eigenmodes.size()
                             << "with" << nr_atoms << "eigenvectors.";
                }

                continue;
            }

            if(!parsed_eigenmodes.back().eigenvectors.empty()) {
                reading_mode_eigenvectors = false;
            }
        }

        if(contains_token(line, "THz")) {
            QRegularExpressionMatch frequency_mode_match = regex_frequency_mode.match(qline);
            if(frequency_mode_match.hasMatch()) {
            double eigenvalue = frequency_mode_match.captured(2).toDouble();
            if(frequency_mode_match.captured(1) == "/i") {
                eigenvalue *= -1.0;
            }

            parsed_eigenmodes.push_back({eigenvalue, {}});
            reading_mode_eigenvectors = true;

            qDebug() << "Detected frequency mode" << parsed_eigenmodes.size()
                     << "(THz):" << eigenvalue;
            continue;
            }
        }

        /*
         * Collect the vasp version (4 or 5)
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS) ) {
            // get the elements and put these in an array
            if(contains_token(line, "vasp.")) {
                QRegularExpressionMatch match = regex_vasp_version.match(qline);
                if (match.hasMatch()) {

                    vasp_version = match.captured(1).toUInt();
                    unsigned int version_major = match.captured(2).toUInt();
                    unsigned int version_minor = match.captured(3).toUInt();

                    qDebug() << "Detected VASP: " << vasp_version << "." << version_major << "." << version_minor;

                    continue;
                }
            }
        }

        /*
         * Collect the elements
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS) ) {
            if(contains_token(line, "VRHFIN")) {
                QRegularExpressionMatch match = regex_element.match(qline);
                if (match.hasMatch()) {
                    elements.push_back(match.captured(2).toStdString());

                    qDebug() << "Captured element:" << match.captured(2);

                    continue;
                }
            }
        }

        /*
         * Collect the number of ions of each element type
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT) ) {
            if(contains_token(line, "ions per type")) {
                QRegularExpressionMatch match = regex_ions_per_element.match(qline);
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
        }

        /*
         * Collect the dimensions of the unit cell. Note that if an IBRION=3 calculation is
         * being run, this is not gathered by this class. It is assumed that each state
         * has the same unit cell. (that means, IBRION != 3 calculations)
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS) ) {
            // get the dimensionality of the unit cell
            if(contains_token(line, "direct lattice vectors")) {
                QRegularExpressionMatch match = regex_lattice_vectors.match(qline);
                if (match.hasMatch()) {

                    // grab next tree lines
                    for(unsigned int i=0; i<3; i++) {
                        std::getline(infile, line);
                        double x, y, z, d4, d5, d6;
                        if(parse_six_columns(line, x, y, z, d4, d5, d6)) {
                            unitcell(i,0) = x;
                            unitcell(i,1) = y;
                            unitcell(i,2) = z;
                        }
                    }

                    readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS);
                    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS);

                    continue;
                }
            }
        }

        /*
         * Collect the energy of the state
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS) ) {
            if(contains_token(line, "energy  without entropy=")) {
                QRegularExpressionMatch match = regex_grab_energy.match(qline);
                if (match.hasMatch()) {

                    energies.push_back(match.captured(2).toDouble());

                    if(vasp_version == 5) {
                        nr_states++;
                    }

                    continue;
                }
            }
        }

        /*
         * Collect the atomic positions and forces for this state
         */
        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS) ) {

            if(!contains_token(line, "POSITION")) {
                continue;
            }

            QRegularExpressionMatch match = regex_atoms.match(qline);
            if (match.hasMatch()) {
                std::getline(infile, line); // skip dashed line

                // create a new structure
                structures.push_back(std::make_shared<Structure>(unitcell));
                qDebug() << "Parsed ionic structure" << structures.size() << "from OUTCAR.";

                for(unsigned i=0; i<nr_atoms_per_elm.size(); i++) {
                    for(unsigned int j=0; j<nr_atoms_per_elm[i]; j++) {
                        std::getline(infile, line);
                        double x, y, z, fx, fy, fz;
                        if(parse_six_columns(line, x, y, z, fx, fy, fz)) {

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

    if(!parsed_eigenmodes.empty()) {
        if(structures.empty()) {
            throw std::runtime_error("Encountered eigenmodes in OUTCAR without atomic structures.");
        }

        qDebug() << "Frequency calculation detected. Ionic structures parsed:" << structures.size()
                 << "; eigenmodes parsed:" << parsed_eigenmodes.size();

        // For frequency calculations, keep only the first ionic structure as reference geometry.
        auto reference_structure = structures.front();
        reference_structure->clear_eigenmodes();

        unsigned int nr_added_modes = 0;
        for(const auto& mode : parsed_eigenmodes) {
            if(mode.eigenvectors.size() != nr_atoms) {
                qDebug() << "Skipping incomplete eigenmode, expected" << nr_atoms
                         << "vectors but found" << mode.eigenvectors.size();
                continue;
            }

            reference_structure->add_eigenmode(mode.eigenvalue, mode.eigenvectors);
            nr_added_modes++;
        }

        qDebug() << "Stored" << nr_added_modes << "eigenmodes on first ionic structure.";

        structures = {reference_structure};

        if(!energies.empty()) {
            structures.front()->set_energy(energies.front());
        }
    }

    return structures;
}

    /**
     * @brief      Load only the final ionic structure from an OUTCAR file
     *
     * @param[in]  filename  The filename
     *
     * @return     Last ionic structure
     */
std::shared_ptr<Structure> StructureLoader::load_outcar_last(const std::string& filename) {
    qDebug() << "Loading final ionic step from OUTCAR: " << QString(filename.c_str());
    std::ifstream infile(filename);

    if(!infile.is_open()) {
        throw std::runtime_error("Could not open " + filename);
    }

    unsigned int vasp_version = 0;

    unsigned int readstate = 0;
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS);
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT);
    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_OPEN);

    unsigned int nr_atoms = 0;

    MatrixUnitcell unitcell = MatrixUnitcell::Zero(3,3);
    std::vector<std::string> elements;
    std::vector<unsigned int> nr_atoms_per_elm;

    bool has_energy = false;
    double last_energy = 0.0;

    std::streampos last_atoms_block_start = std::streampos(-1);

    static QRegularExpression regex_vasp_version("^\\s*vasp.([0-9]).([0-9]+).([0-9]+).*$");
    static QRegularExpression regex_element("^\\s*(VRHFIN\\s+=)([A-Za-z]+)\\s*:.*$");
    static QRegularExpression regex_ions_per_element("^\\s*(ions per type =\\s+)([0-9 ]+)\\s*$");
    static QRegularExpression regex_lattice_vectors("^\\s*direct lattice vectors.*$");
    static QRegularExpression regex_atoms("^\\s*POSITION.*$");
    static QRegularExpression regex_grab_energy("^\\s+energy  without entropy=\\s+([0-9.-]+)\\s+energy\\(sigma->0\\) =\\s+([0-9.-]+).*$");

    std::string line;
    while(true) {
        const std::streampos line_start = infile.tellg();
        if(!std::getline(infile, line)) {
            break;
        }

        const QString qline(line.c_str());

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS)) {
            if(contains_token(line, "vasp.")) {
                QRegularExpressionMatch match = regex_vasp_version.match(qline);
                if(match.hasMatch()) {
                    vasp_version = match.captured(1).toUInt();
                    continue;
                }
            }
        }

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS)) {
            if(contains_token(line, "VRHFIN")) {
                QRegularExpressionMatch match = regex_element.match(qline);
                if(match.hasMatch()) {
                    elements.push_back(match.captured(2).toStdString());
                    continue;
                }
            }
        }

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT)) {
            if(contains_token(line, "ions per type")) {
                QRegularExpressionMatch match = regex_ions_per_element.match(qline);
                if(match.hasMatch()) {
                    QStringList pieces = match.captured(2).split(" ", Qt::SkipEmptyParts);
                    nr_atoms_per_elm.clear();
                    nr_atoms = 0;

                    for(unsigned int i=0; i<pieces.size(); i++) {
                        nr_atoms_per_elm.push_back(pieces[i].toUInt());
                        nr_atoms += pieces[i].toUInt();
                    }

                    readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ELEMENTS);
                    readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_IONS_PER_ELEMENT);
                    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS);

                    if(!(vasp_version == 4 || vasp_version == 5 || vasp_version == 6)) {
                        throw std::runtime_error("Invalid VASP version encountered: " + QString::number(vasp_version).toStdString());
                    }

                    continue;
                }
            }
        }

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS)) {
            if(contains_token(line, "direct lattice vectors")) {
                QRegularExpressionMatch match = regex_lattice_vectors.match(qline);
                if(match.hasMatch()) {
                    for(unsigned int i=0; i<3; i++) {
                        if(!std::getline(infile, line)) {
                            throw std::runtime_error("Unexpected end-of-file while reading OUTCAR lattice vectors.");
                        }

                        double x, y, z, d4, d5, d6;
                        if(parse_six_columns(line, x, y, z, d4, d5, d6)) {
                            unitcell(i,0) = x;
                            unitcell(i,1) = y;
                            unitcell(i,2) = z;
                        }
                    }

                    readstate &= ~(1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_LATTICE_VECTORS);
                    readstate |= (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS);

                    continue;
                }
            }
        }

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS)) {
            if(contains_token(line, "energy  without entropy=")) {
                QRegularExpressionMatch match = regex_grab_energy.match(qline);
                if(match.hasMatch()) {
                    last_energy = match.captured(2).toDouble();
                    has_energy = true;
                    continue;
                }
            }
        }

        if(readstate & (1 << OutcarReadStatus::VASP_OUTCAR_READ_STATE_ATOMS)) {
            if(contains_token(line, "POSITION")) {
                QRegularExpressionMatch match = regex_atoms.match(qline);
                if(match.hasMatch()) {
                    last_atoms_block_start = line_start;
                }
            }
        }
    }

    if(last_atoms_block_start == std::streampos(-1)) {
        throw std::runtime_error("OUTCAR does not contain ionic images.");
    }

    infile.clear();
    infile.seekg(last_atoms_block_start);
    if(!infile.good()) {
        throw std::runtime_error("Failed to seek to final ionic structure in OUTCAR.");
    }

    if(!std::getline(infile, line) || !regex_atoms.match(QString(line.c_str())).hasMatch()) {
        throw std::runtime_error("Failed to read final ionic structure header from OUTCAR.");
    }

    if(!std::getline(infile, line)) {
        throw std::runtime_error("Unexpected end-of-file before final ionic coordinates.");
    }

    auto final_structure = std::make_shared<Structure>(unitcell);
    for(unsigned int i=0; i<nr_atoms_per_elm.size(); i++) {
        for(unsigned int j=0; j<nr_atoms_per_elm[i]; j++) {
            if(!std::getline(infile, line)) {
                throw std::runtime_error("Unexpected end-of-file while reading final ionic coordinates.");
            }

            double x, y, z, fx, fy, fz;
            if(!parse_six_columns(line, x, y, z, fx, fy, fz)) {
                throw std::runtime_error("Invalid atomic position line in final ionic structure.");
            }

            unsigned int atnr = AtomSettings::get().get_atom_elnr(elements[i]);
            final_structure->add_atom(atnr, x, y, z, fx, fy, fz);
        }
    }

    if(has_energy) {
        final_structure->set_energy(last_energy);
    }

    return final_structure;
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
