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

#include "shader_program.h"

ShaderProgram::ShaderProgram(const std::string& _name, const ShaderProgramType type, const QString& vertex_filename, const QString& fragment_filename) {
    this->name = _name;
    this->type = type;
    this->vertex_filename = vertex_filename;
    this->fragment_filename = fragment_filename;

    this->m_program = new QOpenGLShaderProgram;

    if (!this->m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertex_filename)) {
        throw std::runtime_error("Could not add vertex shader: " + this->m_program->log().toStdString());
    }
    if (!this->m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment_filename)) {
        throw std::runtime_error("Could not add fragment shader: " + this->m_program->log().toStdString());
    }

    this->add_attributes();

    if (!this->m_program->link()) {
        throw std::runtime_error("Could not link shader: " + this->m_program->log().toStdString());
    }

    this->add_uniforms();
}

ShaderProgram::~ShaderProgram() {
    delete this->m_program;
    this->m_program = 0;
}

void ShaderProgram::add_attributes() {
    // add attributes depending on the shader program type
    switch(this->type) {
        case ShaderProgramType::ModelShader:
            this->m_program->bindAttributeLocation("position", 0);
            this->m_program->bindAttributeLocation("normal", 1);
        break;
        case ShaderProgramType::AxesShader:
            this->m_program->bindAttributeLocation("position", 0);
            this->m_program->bindAttributeLocation("normal", 1);
        break;
        default:
            // nothing to do
        break;
    }
}

void ShaderProgram::add_uniforms() {
    // add uniforms depending on the shader program type
    if (this->type == ShaderProgramType::ModelShader) {
        this->uniforms.emplace("mvp", this->m_program->uniformLocation("mvp"));
        this->uniforms.emplace("model", this->m_program->uniformLocation("model"));
        this->uniforms.emplace("view", this->m_program->uniformLocation("view"));
        this->uniforms.emplace("lightpos", this->m_program->uniformLocation("lightpos"));
        this->uniforms.emplace("color", this->m_program->uniformLocation("color"));
    }

    if (this->type == ShaderProgramType::StereoscopicShader) {
        this->uniforms.emplace("left_eye_texture", this->m_program->uniformLocation("left_eye_texture"));
        this->uniforms.emplace("right_eye_texture", this->m_program->uniformLocation("right_eye_texture"));
        this->uniforms.emplace("screen_x", this->m_program->uniformLocation("screen_x"));
        this->uniforms.emplace("screen_y", this->m_program->uniformLocation("screen_y"));
    }

    if (this->type == ShaderProgramType::AxesShader) {
        this->uniforms.emplace("mvp", this->m_program->uniformLocation("mvp"));
        this->uniforms.emplace("model", this->m_program->uniformLocation("model"));
        this->uniforms.emplace("view", this->m_program->uniformLocation("view"));
        this->uniforms.emplace("color", this->m_program->uniformLocation("color"));
    }

    if (this->type == ShaderProgramType::UnitcellShader) {
        this->uniforms.emplace("mvp", this->m_program->uniformLocation("mvp"));
        this->uniforms.emplace("color", this->m_program->uniformLocation("color"));
    }

    if (this->type == ShaderProgramType::PlaneShader) {
        this->uniforms.emplace("mvp", this->m_program->uniformLocation("mvp"));
        this->uniforms.emplace("color", this->m_program->uniformLocation("color"));
    }

    if (this->type == ShaderProgramType::SilhouetteShader) {
        this->uniforms.emplace("mvp", this->m_program->uniformLocation("mvp"));
        this->uniforms.emplace("color", this->m_program->uniformLocation("color"));
    }

    if (this->type == ShaderProgramType::CanvasShader) {
        this->uniforms.emplace("regular_texture", this->m_program->uniformLocation("regular_texture"));
        this->uniforms.emplace("silhouette_texture", this->m_program->uniformLocation("silhouette_texture"));
    }
}
