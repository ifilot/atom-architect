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
#include <QMatrix4x4>
#include <QtMath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include <vector>

#include "../data/model_loader.h"
#include "../data/structure.h"
#include "shader_program_manager.h"
#include "user_action.h"

class StructureRenderer {
private:
    // sphere facets
    std::vector<glm::vec3> sphere_vertices;
    std::vector<glm::vec3> sphere_normals;
    std::vector<unsigned int> sphere_indices;

    // cylinder facets
    std::vector<glm::vec3> cylinder_vertices;
    std::vector<glm::vec3> cylinder_normals;
    std::vector<unsigned int> cylinder_indices;

    // vao and vbo for rendering
    QOpenGLVertexArrayObject vao_sphere;
    QOpenGLBuffer vbo_sphere[3];

    QOpenGLVertexArrayObject vao_cylinder;
    QOpenGLBuffer vbo_cylinder[3];

    QOpenGLVertexArrayObject vao_unitcell;
    QOpenGLBuffer vbo_unitcell[2];

    QOpenGLVertexArrayObject vao_line;
    QOpenGLBuffer vbo_line[2];

    QOpenGLVertexArrayObject vao_plane;
    QOpenGLBuffer vbo_plane[2];

    // couple of pointers to important matrices
    std::shared_ptr<Scene> scene;

    std::shared_ptr<ShaderProgramManager> shader_manager;
    std::shared_ptr<UserAction> user_action;

    // models
    std::shared_ptr<Model> axis_model;

    bool flag_draw_unitcell = true;     // whether to draw the unitcell

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  _scene           The scene
     * @param[in]  _shader_manager  The shader manager
     * @param[in]  _user_action     The user action
     */
    StructureRenderer(const std::shared_ptr<Scene>& _scene,
                      const std::shared_ptr<ShaderProgramManager>& _shader_manager,
                      const std::shared_ptr<UserAction>& _user_action);

    /**
     * @brief      Draw the structure
     *
     * @param[in]  structure     The structure
     * @param      model_shader  The model shader
     */
    void draw(const Structure *structure, bool periodicity_xy = false, bool periodicity_z = false);

    /**
     * @brief      Draw the structure
     *
     * @param[in]  structure     The structure
     */
    void draw_silhouette(const Structure *structure);

    /**
     * @brief      Draws coordinate axes.
     */
    void draw_coordinate_axes();

    /**
     * @brief      Disables the drawing of the unitcell
     */
    inline void disable_draw_unitcell() {
        this->flag_draw_unitcell = false;
    }

private:
    /**
     * @brief      Draws atoms.
     *
     * @param[in]  atoms           The atoms
     * @param[in]  structure       The structure
     * @param[in]  periodicity_xy  The periodicity xy
     * @param[in]  periodicity_z   The periodicity z
     */
    void draw_atoms(const std::vector<Atom>& atoms, const Structure* structure, bool periodicity_xy = false, bool periodicity_z = false);

    /**
     * @brief      Draws silhouette of atoms.
     *
     * @param[in]  atoms           The atoms
     * @param[in]  structure       The structure
     */
    void draw_atoms_silhouette(const std::vector<Atom>& atoms, const Structure* structure);

    /**
     * @brief      Draws atoms in the regular unit cell.
     *
     * @param[in]  structure  The structure
     */
    void draw_atoms_regular(const Structure* structure);

    /**
     * @brief      Draws the atoms in the periodicity expansions.
     *
     * @param[in]  structure       The structure
     * @param[in]  periodicity_xy  The periodicity xy
     * @param[in]  periodicity_z   The periodicity z
     */
    void draw_atoms_expansion(const Structure* structure, bool periodicity_xy, bool periodicity_z);

    /**
     * @brief      Draws bonds.
     *
     * @param[in]  structure  The structure
     */
    void draw_bonds(const Structure* structure);

    /**
     * @brief      Draws the unitcell.
     *
     * @param[in]  structure  The structure
     */
    void draw_unitcell(const Structure* structure);

    /**
     * @brief      Draw movement lines
     *
     * @param[in]  structure  The structure
     */
    void draw_movement_lines(const Structure* structure);

    /**
     * @brief      Draw movement plane
     *
     * @param[in]  structure  The structure
     */
    void draw_movement_plane(const Structure* structure);

    /**
     * @brief      Generate coordinates of a sphere
     *
     * @param[in]  tesselation_level  The tesselation level
     */
    void generate_sphere_coordinates(unsigned int tesselation_level);

    /**
     * @brief      Generate coordinates for a default cylinder (radius 1, height 1)
     *
     * @param[in]  stack_count  The stack count
     * @param[in]  slice_count  The slice count
     */
    void generate_cylinder_coordinates(unsigned int stack_count, unsigned int slice_count);

    /**
     * @brief      Generate the coordinates of the unitcell
     */
    void generate_coordinates_unitcell(const MatrixUnitcell& unitcell);

    /**
     * @brief      Update the unitcell vertices in the unit cell
     *
     * @param[in]  unitcell  The unitcell
     */
    void set_unitcell_vertices(const MatrixUnitcell& unitcell);

    /**
     * @brief      Load all data to a vertex array object
     */
    void load_sphere_to_vao();

    /**
     * @brief      Load all data to a vertex array object
     */
    void load_cylinder_to_vao();

    /**
     * @brief      Load simple line data to vertex array object
     */
    void load_line_to_vao();

    /**
     * @brief      Load simple plane data to vertex array object
     */
    void load_plane_to_vao();

    /**
     * @brief      Loads an arrow model.
     */
    void load_arrow_model();

    /**
     * @brief      Darken color
     *
     * @param[in]  color   The color
     * @param[in]  amount  The amount
     *
     * @return     The 3D vector.
     */
    QVector3D darken(const QVector3D& color, float amount) const;

    /**
     * @brief      Lighten color
     *
     * @param[in]  color   The color
     * @param[in]  amount  The amount
     *
     * @return     The 3D vector.
     */
    QVector3D lighten(const QVector3D& color, float amount) const;

    /**
     * @brief      Mix colors
     *
     * @param[in]  color1  First color
     * @param[in]  color2  Second color
     * @param[in]  amount  The amount
     *
     * @return     The 3D vector.
     */
    QVector3D mix(const QVector3D& color1, const QVector3D& color2, float amount) const;
};
