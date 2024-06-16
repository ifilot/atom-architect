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

#include "shader_program_manager.h"

/**
 * @brief      Default constructor
 */
ShaderProgramManager::ShaderProgramManager() {}

/**
 * @brief      Get pointer to shader program
 *
 * @param[in]  name  The name
 *
 * @return     The shader program.
 */
ShaderProgram* ShaderProgramManager::get_shader_program(const std::string& name) {
    auto got = this->shader_program_map.find(name);

    if (got != this->shader_program_map.end()) {
        // shader program is in map, so return it
        return got->second.get();
    } else {
        throw std::runtime_error("Unknown shader program: " + name);
    }
}

/**
 * @brief      Creates a shader program.
 *
 * @param[in]  name               The name
 * @param[in]  type               The type
 * @param[in]  vertex_filename    The vertex filename
 * @param[in]  fragment_filename  The fragment filename
 *
 * @return     pointer to shader program
 */
ShaderProgram* ShaderProgramManager::create_shader_program(const std::string& name, const ShaderProgramType type, const QString& vertex_filename, const QString& fragment_filename) {
    // create program
    ShaderProgram* m_program = new ShaderProgram(name, type, vertex_filename, fragment_filename);

    // add new shader program to unordered map
    this->shader_program_map.emplace(name, m_program);

    // return pointer to shader program
    return m_program;
}

/**
 * @brief      Bind a shader using name
 *
 * @param[in]  name  The name
 */
void ShaderProgramManager::bind(const std::string& name) {
    this->get_shader_program(name)->bind();
}

/**
 * @brief      Release a shader by name
 *
 * @param[in]  name  The name
 */
void ShaderProgramManager::release(const std::string& name) {
    this->get_shader_program(name)->release();
}
