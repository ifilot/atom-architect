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

#include <QOpenGLShaderProgram>
#include <QString>

#include "shader_program_types.h"

class ShaderProgram {
private:
    QOpenGLShaderProgram *m_program;
    ShaderProgramType type;

    std::string name;
    QString vertex_filename;
    QString fragment_filename;

    std::unordered_map<std::string, int> uniforms;

    void add_attributes();
    void add_uniforms();

public:
    ShaderProgram(const std::string& _name, const ShaderProgramType type, const QString& vertex_filename, const QString& fragment_filename);

    template <typename T>
    void set_uniform(const std::string &name, T const &value) {
        auto got = this->uniforms.find(name);

        if (got == this->uniforms.end()) {
            throw std::logic_error("Invalid uniform name: " + name);
        }

        this->m_program->setUniformValue(got->second, value);
    }

    inline bool bind() {
        return this->m_program->bind();
    }

    inline void release() {
        this->m_program->release();
    }

    inline ShaderProgramType get_type() {
        return this->type;
    }

    ~ShaderProgram();
};
