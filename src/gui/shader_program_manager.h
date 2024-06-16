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

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <memory>

#include <QString>

#include "shader_program.h"
#include "shader_program_types.h"

class ShaderProgramManager {
private:
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram> > shader_program_map;

public:
    /**
     * @brief      Default constructor
     */
    ShaderProgramManager();

    /**
     * @brief      Get pointer to shader program
     *
     * @param[in]  name  The name
     *
     * @return     The shader program.
     */
    ShaderProgram* get_shader_program(const std::string& name);

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
    ShaderProgram* create_shader_program(const std::string& name, const ShaderProgramType type, const QString& vertex_filename, const QString& fragment_filename);

    /**
     * @brief      Bind a shader using name
     *
     * @param[in]  name  The name
     */
    void bind(const std::string& name);

    /**
     * @brief      Release a shader by name
     *
     * @param[in]  name  The name
     */
    void release(const std::string& name);

private:

};
