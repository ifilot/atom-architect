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

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QDebug>

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <exception>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

class Model {
private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    bool flag_loaded_vao = false;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo[4];

public:

    Model(std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);

    /**
     * @brief      Draw the model
     */
    void draw();

    /**
     * @brief      Destroys the object.
     */
    ~Model();

    /**
     * @brief      Get maximum vector
     *
     * @return     The maximum vector distance
     */
    glm::vec3 get_max_dim() const;

    /**
     * @brief      Load all data to a vertex array object
     */
    void load_to_vao();

    inline size_t get_num_vertices() const {
        return this->positions.size();
    }

    inline size_t get_num_normals() const {
        return this->normals.size();
    }

    inline size_t get_num_indices() const {
        return this->indices.size();
    }

    inline bool is_loaded() const {
        return this->flag_loaded_vao;
    }
};
