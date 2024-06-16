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

#include "model_loader.h"

ModelLoader::ModelLoader() {}

std::shared_ptr<Model> ModelLoader::load_model(const std::string& path) {

    std::shared_ptr<Model> model;

    auto start = std::chrono::system_clock::now();

    if (path.substr(path.size()-4, 4) == ".obj") {
        model = this->load_data_obj(path);
    }

    if (path.substr(path.size()-4, 4) == ".ply") {
        model = this->load_data_ply(path);
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    this->load_time = elapsed_seconds.count();

    return model;
}

/**
 * @brief      Load object data from obj file
 *
 * @param[in]  path   Path to file
 */
std::shared_ptr<Model> ModelLoader::load_data_obj(const std::string& path) {
    qDebug() << "Loading obj: " << QString(path.c_str());
    std::ifstream file(path);

    if(file.is_open()) {
        // set regex patterns
        static QRegularExpression v_line("v\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+).*");
        static QRegularExpression vn_line("vn\\s+([0-9.-]+)\\s+([0-9.-]+)\\s+([0-9.-]+).*");
        //static QRegularExpression vt_line("vt\\s+([0-9.-]+)\\s+([0-9.-]+).*");
        static QRegularExpression f_line("f\\s+([0-9]+)\\/([0-9]+)\\/([0-9]+)\\s+([0-9]+)\\/([0-9]+)\\/([0-9]+)\\s+([0-9]+)\\/([0-9]+)\\/([0-9]+).*");
        static QRegularExpression f2_line("f\\s+([0-9]+)\\/\\/([0-9]+)\\s+([0-9]+)\\/\\/([0-9]+)\\s+([0-9]+)\\/\\/([0-9]+).*");

        // construct holders
        std::vector<glm::vec3> _positions;
        std::vector<glm::vec3> _normals;
        std::vector<unsigned int> position_indices;
        std::vector<unsigned int> normal_indices;

        // construct vectors for final Model
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        std::vector<glm::vec3> colors;

        std::string line;

        // start reading
        while(std::getline(file, line)) {
            QString qline(line.c_str());

             // positions
            QRegularExpressionMatch match1 = v_line.match(qline);
            if (match1.hasMatch()) {
                glm::vec3 pos(match1.captured(1).toFloat(),
                              match1.captured(2).toFloat(),
                              match1.captured(3).toFloat());
                _positions.push_back(pos);
                continue;
            }

            // normals
            QRegularExpressionMatch match2 = vn_line.match(qline);
            if (match2.hasMatch()) {
                glm::vec3 normal(match2.captured(1).toFloat(),
                                 match2.captured(2).toFloat(),
                                 match2.captured(3).toFloat());
                _normals.push_back(normal);
                continue;
            }

            QRegularExpressionMatch match3 = f_line.match(qline);
            if (match3.hasMatch()) {
                position_indices.push_back(match3.captured(1).toUInt() - 1);
                position_indices.push_back(match3.captured(4).toUInt() - 1);
                position_indices.push_back(match3.captured(7).toUInt() - 1);

                normal_indices.push_back(match3.captured(3).toUInt() - 1);
                normal_indices.push_back(match3.captured(6).toUInt() - 1);
                normal_indices.push_back(match3.captured(9).toUInt() - 1);
                continue;
            }

            QRegularExpressionMatch match4 = f2_line.match(qline);
            if (match4.hasMatch()) {
                position_indices.push_back(match4.captured(1).toUInt() - 1);
                position_indices.push_back(match4.captured(3).toUInt() - 1);
                position_indices.push_back(match4.captured(5).toUInt() - 1);

                normal_indices.push_back(match4.captured(2).toUInt() - 1);
                normal_indices.push_back(match4.captured(4).toUInt() - 1);
                normal_indices.push_back(match4.captured(6).toUInt() - 1);
                continue;
            }
        }

        // restructure data
        for(unsigned int i=0; i<position_indices.size(); i++) {
            positions.push_back(_positions[position_indices[i]]);
            normals.push_back(_normals[normal_indices[i]]);
            indices.push_back(i);
        }

        return std::make_shared<Model>(positions, normals, indices);

    } else {
        throw std::runtime_error("Could not open file: " + path);
    }
}

/**
 * @brief      Load object data from ply file
 *
 * @param[in]  path   Path to file
 */
std::shared_ptr<Model> ModelLoader::load_data_ply(const std::string& path) {
    std::ifstream file(path);

    if(file.is_open()) {
        std::string line;
        std::getline(file, line);
        QString qline = QString(line.c_str()).trimmed();
        if(!qline.endsWith(".ply")) {
            throw std::runtime_error("File with .ply extension does not start with \"ply\" header.");
        }

        std::getline(file, line);
        if(line == "format ascii 1.0") {
            file.close();
            return this->load_data_ply_ascii(path);
        }

        if(line == "format binary_little_endian 1.0") {
            file.close();
            return this->load_data_ply_binary(path);
        }

        throw std::runtime_error("Unsupported formatting encountered: " + line);
    } else {
        throw std::runtime_error("Could not open file: " + path);
    }
}

/**
 * @brief      Loads a ply file from hard drive stored as little endian binary
 *
 * @param[in]  path   Path to file
 */
std::shared_ptr<Model> ModelLoader::load_data_ply_binary(const std::string& path) {
    std::shared_ptr<Model> models;

    std::ifstream file(path);

    if(file.is_open()) {
        std::string line;

        // set regex patterns
        static QRegularExpression comment_line("$comment.*");
        static QRegularExpression element_vertex("element vertex ([0-9]+)");
        static QRegularExpression element_face("element face ([0-9]+)");
        static QRegularExpression property_float("property float [A-Za-z]+");
        static QRegularExpression property_uchar("property uchar ([A-Za-z]+)");

        // construct vectors for final Model
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        std::vector<glm::vec3> colors;

        // start reading
        std::getline(file, line);
        QString qline = QString(line.c_str()).trimmed();
        unsigned int headersz = line.size() + 1;
        unsigned int nrvertices = 0;
        unsigned int nrfaces = 0;
        bool has_colors = false;

        while(qline != "end_header") {
            QRegularExpressionMatch what;

            // discard comments
            what = comment_line.match(qline);
            if (what.hasMatch()) {
                continue;
            }

            what = element_vertex.match(qline);
            if (what.hasMatch()) {
                nrvertices = QString(what.captured(1)).toUInt();
                continue;
            }

            what = element_face.match(qline);
            if (what.hasMatch()) {
                nrfaces = QString(what.captured(1)).toUInt();
                continue;
            }

            what = property_uchar.match(qline);
            if (what.hasMatch()) {
                if(what.captured(1) == "red") {
                    has_colors = true;
                }
            }

            std::getline(file, line);
            headersz += line.size() + 1;
            qline = QString(line.c_str()).trimmed();
        }

        while(std::getline(file, line)) {
            if(line == "end_header") {
                break;
            }
        }
        file.close();
        file.open(path, std::ios::binary);
        file.seekg(headersz);

        // resize arrays
        positions.resize(nrvertices);
        normals.resize(nrvertices);
        colors.resize(nrvertices, glm::vec3(1.0f, 1.0f, 1.0f));
        indices.resize(nrfaces * 3);

        // read positions and normals
        for(unsigned int i=0; i<nrvertices; i++) {
            file.read((char*)&positions[i][0], sizeof(float) * 3);
            file.read((char*)&normals[i][0], sizeof(float) * 3);
            if(has_colors) {
                std::array<uint8_t, 3> col;
                file.read((char*)&col[0], sizeof(uint8_t) * 3); // read the color data, but ignore it in the visualization
            }
        }

        // read faces
        uint8_t facesz = 0;
        for(unsigned int i=0; i<nrfaces; i++) {
            file.read((char*)&facesz, sizeof(uint8_t));
            if(facesz != 3) {
                throw std::runtime_error("Unsupported face size encountered: " + std::to_string(facesz));
            }
            file.read((char*)&indices[i * 3], sizeof(unsigned int) * 3);
        }

        file.close();

        return std::make_shared<Model>(positions, normals, indices);
    } else {
        throw std::runtime_error("Could not open file: " + path);
    }
}

/**
 * @brief      Loads a ply file from hard drive stored in ascii format
 *
 * @param[in]  path   The path
 */
std::shared_ptr<Model> ModelLoader::load_data_ply_ascii(const std::string& path) {
    std::ifstream file(path);
    static QRegularExpression whitespace("\\s+");

    if(file.is_open()) {
        std::string line;

        while(std::getline(file, line)) {
            if(line == "end_header") {
                break;
            }
        }

        // construct vectors for final Model
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        std::vector<glm::vec3> colors;

        while(std::getline(file, line)) {
            QStringList pieces = QString(line.c_str()).split(whitespace);
            if(pieces.size() == 9) {
                positions.emplace_back(pieces[0].toFloat(), pieces[1].toFloat(), pieces[2].toFloat());
                normals.emplace_back(pieces[3].toFloat(), pieces[4].toFloat(), pieces[5].toFloat());
                colors.emplace_back(pieces[6].toFloat(), pieces[7].toFloat(), pieces[8].toFloat());
                colors.back() /= 255.0f; // colors are stored, but not used further on in the program
            }
            if(pieces.size() == 5) {
                indices.push_back(pieces[1].toUInt());
                indices.push_back(pieces[2].toUInt());
                indices.push_back(pieces[3].toUInt());
                indices.push_back(pieces[1].toUInt());
                indices.push_back(pieces[3].toUInt());
                indices.push_back(pieces[4].toUInt());
            }
            if(pieces.size() == 4) {
                indices.push_back(pieces[1].toUInt());
                indices.push_back(pieces[2].toUInt());
                indices.push_back(pieces[3].toUInt());
            }
        }

        file.close();

        return std::make_shared<Model>(positions, normals, indices);
    } else {
        throw std::runtime_error("Could not open file: " + path);
    }
}
