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

#include "structure_renderer.h"

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  _scene           The scene
 * @param[in]  _shader_manager  The shader manager
 * @param[in]  _user_action     The user action
 */
StructureRenderer::StructureRenderer(const std::shared_ptr<Scene>& _scene,
                                     const std::shared_ptr<ShaderProgramManager>& _shader_manager,
                                     const std::shared_ptr<UserAction>& _user_action) :
    scene(_scene),
    shader_manager(_shader_manager),
    user_action(_user_action)
{
    qDebug() << "Constructing Structure Renderer object";
    this->generate_sphere_coordinates(3);
    this->generate_cylinder_coordinates(2, 18);
    this->generate_coordinates_unitcell(MatrixUnitcell::Ones(3,3));
    this->load_sphere_to_vao();
    this->load_cylinder_to_vao();
    this->load_line_to_vao();
    this->load_plane_to_vao();

    qDebug() << "Loading arrow model";
    this->load_arrow_model();
}

/**
 * @brief      Draw the structure
 *
 * @param[in]  structure       The structure
 * @param[in]  periodicity_xy  whether to draw periodicity in xy direction
 * @param[in]  periodicity_z   whether to draw periodicity in z direction
 * @param      model_shader  The model shader
 */
void StructureRenderer::draw(const Structure *structure, bool periodicity_xy, bool periodicity_z) {
    this->draw_atoms_regular(structure);

    if(periodicity_xy || periodicity_z) {
        this->draw_atoms_expansion(structure, periodicity_xy, periodicity_z);
    }

    if(structure->get_nr_atoms() < 2000) {
        this->draw_bonds(structure);
    }

    if(this->flag_draw_unitcell) {
        this->draw_unitcell(structure);
    }
    this->draw_movement_lines(structure);
    this->draw_movement_plane(structure);
}

/**
 * @brief      Draw the structure
 *
 * @param[in]  structure     The structure
 */
void StructureRenderer::draw_silhouette(const Structure *structure) {
    this->draw_atoms_silhouette(structure->get_atoms(), structure);
}

/**
 * @brief      Draws atoms in the regular unit cell.
 *
 * @param[in]  structure  The structure
 */
void StructureRenderer::draw_atoms_regular(const Structure* structure) {
    this->draw_atoms(structure->get_atoms(), structure);
}

/**
 * @brief      Draws coordinate axes.
 */
void StructureRenderer::draw_coordinate_axes() {
    ShaderProgram *model_shader = this->shader_manager->get_shader_program("model_shader");
    model_shader->bind();

    const QVector3D red(98.8f/100.0f, 20.8f/100.0f, 32.5f/100.0f);
    const QVector3D green(54.9f/100.0f, 86.7f/100.0f, 0.0f);
    const QVector3D blue(15.7f/100.0f, 60.0f/100.0f, 100.0f/100.0f);

    // set view port, projection and view matrices
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0.75f * this->scene->canvas_width, 0.0f, this->scene->canvas_width * 0.25f, this->scene->canvas_height * 0.25f);

    QMatrix4x4 projection_ortho;
    projection_ortho.setToIdentity();
    float ratio = (float)this->scene->canvas_height / (float)this->scene->canvas_width;
    static const float sz = 25.0f;
    projection_ortho.ortho(-sz, sz, -sz * ratio, sz * ratio, 0.1f, 1000.0f);

    QMatrix4x4 model, view, mvp;
    view.lookAt(QVector3D(0.0, -10.0, 0.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 0.0, 1.0));
    QMatrix4x4 axis_rotation;
    model.setToIdentity();

    // set general properties
    model_shader->set_uniform("view", this->scene->view);
    model_shader->set_uniform("lightpos", QVector3D(0,-1000,1));

    // *******************
    // draw the three axes
    // *******************

    // z-axis
    axis_rotation.setToIdentity();
    model = this->scene->arcball_rotation * this->scene->rotation_matrix * axis_rotation;
    mvp = projection_ortho * view * model;
    model_shader->set_uniform("model", model);
    model_shader->set_uniform("mvp", mvp);
    model_shader->set_uniform("color", blue);
    this->axis_model->draw();

    // y-axis
    axis_rotation.setToIdentity();
    axis_rotation.rotate(-90.0f, QVector3D(1.0, 0.0, 0.0));
    model = this->scene->arcball_rotation * this->scene->rotation_matrix * axis_rotation;
    mvp = projection_ortho * view * model;
    model_shader->set_uniform("model", model);
    model_shader->set_uniform("mvp", mvp);
    model_shader->set_uniform("color", green);
    this->axis_model->draw();

    // x-axis
    axis_rotation.setToIdentity();
    axis_rotation.rotate(90.0f, QVector3D(0.0, 1.0, 0.0));
    model = this->scene->arcball_rotation * this->scene->rotation_matrix * axis_rotation;
    mvp = projection_ortho * view * model;
    model_shader->set_uniform("model", model);
    model_shader->set_uniform("mvp", mvp);
    model_shader->set_uniform("color", red);
    this->axis_model->draw();

    model_shader->release();
}

/**
 * @brief      Draws the atoms in the periodicity expansions.
 *
 * @param[in]  structure       The structure
 * @param[in]  periodicity_xy  The periodicity xy
 * @param[in]  periodicity_z   The periodicity z
 */
void StructureRenderer::draw_atoms_expansion(const Structure* structure, bool periodicity_xy, bool periodicity_z) {
    this->draw_atoms(structure->get_atoms_expansion(), structure, periodicity_xy, periodicity_z);
}

/**
 * @brief      Draws atoms.
 *
 * @param[in]  atoms           The atoms
 * @param[in]  structure       The structure
 * @param[in]  periodicity_xy  The periodicity xy
 * @param[in]  periodicity_z   The periodicity z
 */
void StructureRenderer::draw_atoms(const std::vector<Atom>& atoms, const Structure* structure, bool periodicity_xy, bool periodicity_z) {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_sphere.bind();

    ShaderProgram *model_shader = this->shader_manager->get_shader_program("model_shader");
    model_shader->bind();

    QMatrix4x4 model;
    QMatrix4x4 mvp;

    // set general properties
    model_shader->set_uniform("view", this->scene->view);
    model_shader->set_uniform("lightpos", QVector3D(0,-1000,1));

    // get the vector that positions the unitcell at the origin
    auto ctr_vector = structure->get_center_vector();

    for(const Atom& atom : atoms) {
        bool expansion_atom = false;

        // settings per atom type
        if(atom.atomtype & (1 << ATOM_CENTRAL_UNITCELL)) {
            // regular atom
        } else if((periodicity_xy && periodicity_z) && (atom.atomtype & (1 << ATOM_EXPANSION_XY) || atom.atomtype & (1 < ATOM_EXPANSION_Z))) {
            expansion_atom = true;
        } else if(periodicity_xy && atom.atomtype & (1 << ATOM_EXPANSION_XY) && !(atom.atomtype & (1 << ATOM_EXPANSION_Z))) {
            expansion_atom = true;
        } else if(periodicity_z && atom.atomtype & (1 << ATOM_EXPANSION_Z) && !(atom.atomtype & (1 << ATOM_EXPANSION_XY))) {
            expansion_atom = true;
        } else {
            // ignore this atom
            continue;
        }

        // set the color of the atom
        auto col = AtomSettings::get().get_atom_color_qvector(AtomSettings::get().get_name_from_elnr(atom.atnr));
        if(expansion_atom) { // darken atom if it belongs to a periodicity expansion
            col = this->mix(col, QVector3D(1.0f - col[0], 1.0f - col[1], 1.0f - col[2]), 0.4);
        } else {
            for(unsigned int j=0; j<3; j++) {
                if(!atom.selective_dynamics[j]) {
                    col = this->darken(col, 0.5);
                    break;
                }
            }
        }

        if(atom.select != 0) { // highlight atom if it is selected
            col = this->lighten(col, 0.1);
        }

        double radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);

        // build model matrix
        model.setToIdentity();
        model *= (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(ctr_vector);        // position the center of the unitcell at the origin
        if(atom.select == 1) {
            model *= this->scene->transposition;
        }
        model.translate(atom.get_pos());
        model.scale(radius);

        // build model - view - projection matrix
        QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;

        // set per-atom properties
        model_shader->set_uniform("mvp", mvp);
        model_shader->set_uniform("model", model);
        model_shader->set_uniform("color", col);

        // draw atom
        f->glDrawElements(GL_TRIANGLES, this->sphere_indices.size(), GL_UNSIGNED_INT, 0);
    }

    this->vao_sphere.release();
    model_shader->release();
}

/**
 * @brief      Draws silhouette of atoms.
 *
 * @param[in]  atoms           The atoms
 * @param[in]  structure       The structure
 */
void StructureRenderer::draw_atoms_silhouette(const std::vector<Atom>& atoms, const Structure* structure) {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_sphere.bind();

    ShaderProgram *model_shader = this->shader_manager->get_shader_program("silhouette_shader");
    model_shader->bind();

    QMatrix4x4 model;
    QMatrix4x4 mvp;

    // get the vector that positions the unitcell at the origin
    auto ctr_vector = structure->get_center_vector();

    float counter = 10.0f;
    for(const Atom& atom : atoms) {
        if(!(atom.atomtype & (1 << ATOM_CENTRAL_UNITCELL))) {
            continue;
        }

        auto col = QVector3D(0.0f, 0.0f, 0.0f);
        switch(atom.select) {
            case 0:
                col = QVector3D(0.0f, 0.0f, 0.0f);
            break;
            case 1:
                counter += 1.0f;
                col = QVector3D(counter/255.f, 0.0f, 0.25f);
            break;
            case 2:
                counter += 1.0f;
                col = QVector3D(counter/255.f, 0.f, 0.50f);
            break;
            default:
                col = QVector3D(0.0f, 0.0f, 0.0f);
            break;
        }

        double radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);

        // build model matrix
        model.setToIdentity();
        model *= (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(ctr_vector);        // position the center of the unitcell at the origin
        if(atom.select == 1) {
            model *= this->scene->transposition;
        }
        model.translate(atom.get_pos());
        model.scale(radius);

        // build model - view - projection matrix
        QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;

        // set per-atom properties
        model_shader->set_uniform("mvp", mvp);
        model_shader->set_uniform("color", col);

        // draw atom
        f->glDrawElements(GL_TRIANGLES, this->sphere_indices.size(), GL_UNSIGNED_INT, 0);
    }

    this->vao_sphere.release();
    model_shader->release();
}

/**
 * @brief      Draws bonds.
 *
 * @param[in]  structure  The structure
 */
void StructureRenderer::draw_bonds(const Structure* structure) {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_cylinder.bind();

    ShaderProgram *model_shader = this->shader_manager->get_shader_program("model_shader");
    model_shader->bind();

    QMatrix4x4 model;
    QMatrix4x4 mvp;

    // set general properties
    model_shader->set_uniform("view", this->scene->view);
    model_shader->set_uniform("lightpos", QVector3D(0,-1000,1));

    // get the vector that positions the unitcell at the origin
    auto ctr_vector = structure->get_center_vector();

    for(unsigned int i=0; i<structure->get_nr_bonds(); i++) {
        const Bond& bond = structure->get_bond(i);

        QVector3D col;

        model.setToIdentity();
        model *= (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(ctr_vector);        // position the center of the unitcell at the origin
        model.translate(bond.atom1.get_pos());
        model.rotate(qRadiansToDegrees(bond.angle), bond.axis);
        model.scale(QVector3D(0.15, 0.15, bond.length * 0.5));

        QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;

        model_shader->set_uniform("mvp", mvp);
        model_shader->set_uniform("model", model);
        col = AtomSettings::get().get_atom_color_qvector(AtomSettings::get().get_name_from_elnr(bond.atom1.atnr));
        for(unsigned int j=0; j<3; j++) {
            if(!bond.atom1.selective_dynamics[j]) {
                col = this->darken(col, 0.5);
                break;
            }
        }
        model_shader->set_uniform("color", col);

        // draw bond
        f->glDrawElements(GL_TRIANGLES, this->cylinder_indices.size(), GL_UNSIGNED_INT, 0);

        model.setToIdentity();
        model *= (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(ctr_vector);        // position the center of the unitcell at the origin
        model.translate(bond.atom1.get_pos() + (bond.direction * bond.length * 0.5));
        model.rotate(qRadiansToDegrees(bond.angle), bond.axis);
        model.scale(QVector3D(0.15, 0.15, bond.length * 0.5));

        mvp = (this->scene->projection) * (this->scene->view) * model;

        model_shader->set_uniform("mvp", mvp);
        model_shader->set_uniform("model", model);
        col = AtomSettings::get().get_atom_color_qvector(AtomSettings::get().get_name_from_elnr(bond.atom2.atnr));
        for(unsigned int j=0; j<3; j++) {
            if(!bond.atom2.selective_dynamics[j]) {
                col = this->darken(col, 0.5);
                break;
            }
        }
        model_shader->set_uniform("color", col);

        // draw bond
        f->glDrawElements(GL_TRIANGLES, this->cylinder_indices.size(), GL_UNSIGNED_INT, 0);
    }

    this->vao_cylinder.release();
    model_shader->release();
}

/**
 * @brief      Draws the unitcell.
 *
 * @param[in]  structure  The structure
 */
void StructureRenderer::draw_unitcell(const Structure* structure) {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->set_unitcell_vertices(structure->get_unitcell());

    ShaderProgram *unitcell_shader = this->shader_manager->get_shader_program("unitcell_shader");
    unitcell_shader->bind();

    QMatrix4x4 model = (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
    model.translate(structure->get_center_vector()); // position the center of the unitcell at the origin
    QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;
    unitcell_shader->set_uniform("mvp", mvp);
    unitcell_shader->set_uniform("color", QVector3D(0.5f, 0.5f, 0.5f));

    this->vao_unitcell.bind();
    f->glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    this->vao_unitcell.release();
    unitcell_shader->release();
}

/**
 * @brief      Draw movement lines
 *
 * @param[in]  structure  The structure
 */
void StructureRenderer::draw_movement_lines(const Structure* structure) {
    if(!(this->user_action->get_movement_action() == MovementAction::MOVEMENT_NONE ||
         this->user_action->get_movement_action() == MovementAction::MOVEMENT_FREE)) {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        ShaderProgram *unitcell_shader = this->shader_manager->get_shader_program("unitcell_shader");
        unitcell_shader->bind();

        QMatrix4x4 model = (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(structure->get_center_vector()); // position the center of the unitcell at the origin
        QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;
        unitcell_shader->set_uniform("mvp", mvp);

        this->vao_line.bind();

        const QVector3D red(98.8f/100.0f, 20.8f/100.0f, 32.5f/100.0f);
        const QVector3D green(54.9f/100.0f, 86.7f/100.0f, 0.0f);
        const QVector3D blue(15.7f/100.0f, 60.0f/100.0f, 100.0f/100.0f);

        // update vector elements
        std::vector<QVector3D> data(3);
        switch(this->user_action->get_movement_action()) {
            case MovementAction::MOVEMENT_NONE:
                // do nothing
            break;
            case MovementAction::MOVEMENT_FREE:
                // do nothing
            break;
            case MovementAction::MOVEMENT_X:
                data[0] = structure->get_position_primary_buffer() + QVector3D(-1000.f, 0.f, 0.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(1000.f, 0.f, 0.f);
                data[2] = red;
            break;
            case MovementAction::MOVEMENT_Y:
                data[0] = structure->get_position_primary_buffer() + QVector3D(0.f, -1000.f, 0.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(0.f, 1000.f, 0.f);
                data[2] = green;
            break;
            case MovementAction::MOVEMENT_Z:
                data[0] = structure->get_position_primary_buffer() + QVector3D(0.f, 0.f, -1000.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(0.f, 0.f, 1000.f);
                data[2] = blue;
            break;
            case MovementAction::MOVEMENT_FOCUS:
                auto v1 = structure->get_position_primary_buffer();
                auto v2 = structure->get_position_secondary_buffer();
                auto v = (v2 - v1).normalized();
                data[0] = v1 - 1000.f * v;
                data[1] = v1 + 1000.f * v;
                data[2] = QVector3D(1.0f, 1.0f, 1.0f);
            break;
        }

        this->vbo_line[0].bind();
        this->vbo_line[0].allocate(&data[0][0], 2 * 3 * sizeof(float));
        unitcell_shader->set_uniform("color", data[2]);

        f->glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
        this->vao_line.release();
        unitcell_shader->release();

        return;
    }

    if(!(this->user_action->get_rotation_action() == RotationAction::ROTATION_NONE ||
         this->user_action->get_rotation_action() == RotationAction::ROTATION_FREE)) {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        ShaderProgram *unitcell_shader = this->shader_manager->get_shader_program("unitcell_shader");
        unitcell_shader->bind();

        QMatrix4x4 model = (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
        model.translate(structure->get_center_vector()); // position the center of the unitcell at the origin
        QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;
        unitcell_shader->set_uniform("mvp", mvp);

        this->vao_line.bind();

        const QVector3D red(98.8f/100.0f, 20.8f/100.0f, 32.5f/100.0f);
        const QVector3D green(54.9f/100.0f, 86.7f/100.0f, 0.0f);
        const QVector3D blue(15.7f/100.0f, 60.0f/100.0f, 100.0f/100.0f);

        // update vector elements
        std::vector<QVector3D> data(3);
        switch(this->user_action->get_rotation_action()) {
            case RotationAction::ROTATION_NONE:
                // do nothing
            break;
            case RotationAction::ROTATION_FREE:
                // do nothing
            break;
            case RotationAction::ROTATION_X:
                data[0] = structure->get_position_primary_buffer() + QVector3D(-1000.f, 0.f, 0.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(1000.f, 0.f, 0.f);
                data[2] = red;
            break;
            case RotationAction::ROTATION_Y:
                data[0] = structure->get_position_primary_buffer() + QVector3D(0.f, -1000.f, 0.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(0.f, 1000.f, 0.f);
                data[2] = green;
            break;
            case RotationAction::ROTATION_Z:
                data[0] = structure->get_position_primary_buffer() + QVector3D(0.f, 0.f, -1000.f);
                data[1] = structure->get_position_primary_buffer() + QVector3D(0.f, 0.f, 1000.f);
                data[2] = blue;
            break;
            case RotationAction::ROTATION_FOCUS:
                auto v1 = structure->get_position_primary_buffer();
                auto v2 = structure->get_position_secondary_buffer();
                auto v = (v2 - v1).normalized();
                data[0] = v1 - 1000.f * v;
                data[1] = v1 + 1000.f * v;
                data[2] = QVector3D(1.0f, 1.0f, 1.0f);
            break;
        }

        this->vbo_line[0].bind();
        this->vbo_line[0].allocate(&data[0][0], 2 * 3 * sizeof(float));
        unitcell_shader->set_uniform("color", data[2]);

        f->glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
        this->vao_line.release();
        unitcell_shader->release();

        return;
    }
}

/**
 * @brief      Draw movement plane
 *
 * @param[in]  structure  The structure
 */
void StructureRenderer::draw_movement_plane(const Structure* structure) {
    if(!(this->user_action->get_movement_action() == MovementAction::MOVEMENT_FREE)) {
        return;
    }

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    ShaderProgram *plane_shader = this->shader_manager->get_shader_program("plane_shader");
    plane_shader->bind();

    QMatrix4x4 model = (this->scene->arcball_rotation) * (this->scene->rotation_matrix);
    model.translate(structure->get_center_vector()); // position the center of the unitcell at the origin
    QMatrix4x4 mvp = (this->scene->projection) * (this->scene->view) * model;
    plane_shader->set_uniform("mvp", mvp);

    this->vao_plane.bind();

    // update vector elements
    const float sz = 3.0;
    std::vector<QVector3D> data(5);
    QVector3D v1 = this->scene->rotation_matrix.inverted().map(QVector3D(1.0f, 0.0f, 0.0f));
    QVector3D v2 = this->scene->rotation_matrix.inverted().map(QVector3D(0.0f, 0.0f, 1.0f));
    data[0] = structure->get_position_primary_buffer() + sz * (-v1 - v2);
    data[1] = structure->get_position_primary_buffer() + sz * (v1 - v2);
    data[2] = structure->get_position_primary_buffer() + sz * (v1 + v2);
    data[3] = structure->get_position_primary_buffer() + sz * (-v1 + v2);
    data[4] = QVector3D(1.0f, 1.0f, 1.0f);

    this->vbo_plane[0].bind();
    this->vbo_plane[0].allocate(&data[0][0], 4 * 3 * sizeof(float));
    plane_shader->set_uniform("color", data[4]);

    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    this->vao_line.release();
    plane_shader->release();
}

/**
 * @brief      Generate coordinates of a sphere
 *
 * @param[in]  tesselation_level  The tesselation level
 */
void StructureRenderer::generate_sphere_coordinates(unsigned int tesselation_level) {
    std::vector<glm::vec3> vertices;

    vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    vertices.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
    vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    vertices.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    vertices.push_back(glm::vec3(0.0f, 0.0f, -1.0f));

    std::vector<unsigned int> triangles;
    triangles.resize(24);

    triangles[0] = 0;
    triangles[1] = 3;
    triangles[2] = 5;

    triangles[3] = 3;
    triangles[4] = 1;
    triangles[5] = 5;

    triangles[6] = 3;
    triangles[7] = 4;
    triangles[8] = 1;

    triangles[9] = 0;
    triangles[10] = 4;
    triangles[11] = 3;

    triangles[12] = 2;
    triangles[13] = 0;
    triangles[14] = 5;

    triangles[15] = 2;
    triangles[16] = 5;
    triangles[17] = 1;

    triangles[18] = 4;
    triangles[19] = 0;
    triangles[20] = 2;

    triangles[21] = 4;
    triangles[22] = 2;
    triangles[23] = 1;

    std::vector<unsigned int> new_triangles;

    for (unsigned int j = 0; j < tesselation_level; j++) {
        new_triangles.resize(0);
        unsigned int size = triangles.size();

        for (unsigned int i = 0; i < size; i += 3) {
            glm::vec3 center1 = glm::normalize((vertices[triangles[i]] + vertices[triangles[i+1]]) / 2.0f);
            glm::vec3 center2 = glm::normalize((vertices[triangles[i]] + vertices[triangles[i+2]]) / 2.0f);
            glm::vec3 center3 = glm::normalize((vertices[triangles[i+1]] + vertices[triangles[i+2]]) / 2.0f);

            vertices.push_back(center1);
            unsigned int a = vertices.size() - 1;
            vertices.push_back(center2);
            unsigned int b = vertices.size() - 1;
            vertices.push_back(center3);
            unsigned int c = vertices.size() - 1;

            new_triangles.push_back(triangles[i]);
            new_triangles.push_back(a);
            new_triangles.push_back(b);

            new_triangles.push_back(triangles[i+1]);
            new_triangles.push_back(c);
            new_triangles.push_back(a);

            new_triangles.push_back(triangles[i+2]);
            new_triangles.push_back(b);
            new_triangles.push_back(c);

            new_triangles.push_back(a);
            new_triangles.push_back(c);
            new_triangles.push_back(b);
        }
        triangles = new_triangles;
    }

    this->sphere_vertices = vertices;
    this->sphere_normals = vertices;  // for a sphere, vertices and normals are equal
    this->sphere_indices = triangles;
}

/**
 * @brief      Generate coordinates for a default cylinder (radius 1, height 1)
 *
 * @param[in]  stack_count  The stack count
 * @param[in]  slice_count  The slice count
 */
void StructureRenderer::generate_cylinder_coordinates(unsigned int stack_count, unsigned int slice_count) {
    // construct vertices and normals
    for (float stack = 0; stack < stack_count; ++stack) {
        for (float slice = 0; slice < slice_count; ++slice) {
            float x = std::sin(2.0f * (float) M_PI * slice / slice_count);
            float y = std::cos(2.0f * (float) M_PI * slice / slice_count);
            float z = stack / (stack_count - 1.0f);

            this->cylinder_vertices.push_back(glm::vec3(x, y, z));

            glm::vec3 normal = glm::normalize(glm::vec3(x, y, 0));

            this->cylinder_normals.push_back(normal);
        }
    }

    // construct indices
    for (unsigned int stack = 0; stack < stack_count - 1; ++stack) {
        for (unsigned int slice = 0; slice < slice_count; ++slice) {
            // point 1
            this->cylinder_indices.push_back(stack * slice_count + slice);

            // point 4
            this->cylinder_indices.push_back((stack + 1) * slice_count + slice);

            // point 3
            if (slice + 1 == slice_count) {
                this->cylinder_indices.push_back((stack + 1) * slice_count);
            } else {
                this->cylinder_indices.push_back((stack + 1) * slice_count + slice + 1);
            }

            // point 1
            this->cylinder_indices.push_back(stack * slice_count + slice);

            // point 3
            if (slice + 1 == slice_count) {
                this->cylinder_indices.push_back((stack + 1) * slice_count);
            } else {
                this->cylinder_indices.push_back((stack + 1) * slice_count + slice + 1);
            }

            // point 2
            if (slice + 1 == slice_count) {
                this->cylinder_indices.push_back(stack * slice_count);
            } else {
                this->cylinder_indices.push_back(stack * slice_count + slice + 1);
            }
        }
    }
}

/**
 * @brief      Generate the coordinates of the unitcell
 */
void StructureRenderer::generate_coordinates_unitcell(const MatrixUnitcell& unitcell) {
    // set vertices of the unitcell
    std::vector<glm::vec3> unitcell_vertices;
    unitcell_vertices.push_back(glm::vec3(0,0,0));
    unitcell_vertices.push_back(glm::vec3(unitcell(0,0), unitcell(0,1), unitcell(0,2)));
    unitcell_vertices.push_back(glm::vec3(unitcell(1,0), unitcell(1,1), unitcell(1,2)));
    unitcell_vertices.push_back(glm::vec3(unitcell(2,0), unitcell(2,1), unitcell(2,2)));
    unitcell_vertices.push_back(unitcell_vertices[1] + unitcell_vertices[2]);
    unitcell_vertices.push_back(unitcell_vertices[1] + unitcell_vertices[3]);
    unitcell_vertices.push_back(unitcell_vertices[2] + unitcell_vertices[3]);
    unitcell_vertices.push_back(unitcell_vertices[4] + unitcell_vertices[3]);

    // these are for the axes
    unitcell_vertices.push_back(glm::vec3(-1000, 0, 0));
    unitcell_vertices.push_back(glm::vec3(1000, 0, 0));
    unitcell_vertices.push_back(glm::vec3(0, -1000, 0));
    unitcell_vertices.push_back(glm::vec3(0, 1000, 0));
    unitcell_vertices.push_back(glm::vec3(0, 0, -1000));
    unitcell_vertices.push_back(glm::vec3(0, 0, 1000));

    // set indices of the unit cell
    std::vector<unsigned int> unitcell_indices;
    unitcell_indices.push_back(0);
    unitcell_indices.push_back(1);
    unitcell_indices.push_back(0);
    unitcell_indices.push_back(2);
    unitcell_indices.push_back(0);
    unitcell_indices.push_back(3);
    unitcell_indices.push_back(1);
    unitcell_indices.push_back(4);
    unitcell_indices.push_back(2);
    unitcell_indices.push_back(4);
    unitcell_indices.push_back(1);
    unitcell_indices.push_back(5);
    unitcell_indices.push_back(4);
    unitcell_indices.push_back(7);
    unitcell_indices.push_back(2);
    unitcell_indices.push_back(6);
    unitcell_indices.push_back(6);
    unitcell_indices.push_back(7);
    unitcell_indices.push_back(7);
    unitcell_indices.push_back(5);
    unitcell_indices.push_back(3);
    unitcell_indices.push_back(5);
    unitcell_indices.push_back(6);
    unitcell_indices.push_back(3);
    unitcell_indices.push_back(8);
    unitcell_indices.push_back(9);
    unitcell_indices.push_back(10);
    unitcell_indices.push_back(11);
    unitcell_indices.push_back(12);
    unitcell_indices.push_back(13);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_unitcell.create();
    this->vao_unitcell.bind();

    this->vbo_unitcell[0].create();
    this->vbo_unitcell[0].setUsagePattern(QOpenGLBuffer::DynamicDraw);
    this->vbo_unitcell[0].bind();
    this->vbo_unitcell[0].allocate(&unitcell_vertices[0][0], unitcell_vertices.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->vbo_unitcell[1] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    this->vbo_unitcell[1].create();
    this->vbo_unitcell[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_unitcell[1].bind();
    this->vbo_unitcell[1].allocate(&unitcell_indices[0], unitcell_indices.size() * sizeof(unsigned int));

    this->vao_sphere.release();
}

/**
 * @brief      Update the unitcell vertices in the unit cell
 *
 * @param[in]  unitcell  The unitcell
 */
void StructureRenderer::set_unitcell_vertices(const MatrixUnitcell& unitcell) {
    // set vertices of the unitcell
    std::vector<glm::vec3> unitcell_vertices;
    unitcell_vertices.push_back(glm::vec3(0,0,0));
    unitcell_vertices.push_back(glm::vec3(unitcell(0,0), unitcell(0,1), unitcell(0,2)));
    unitcell_vertices.push_back(glm::vec3(unitcell(1,0), unitcell(1,1), unitcell(1,2)));
    unitcell_vertices.push_back(glm::vec3(unitcell(2,0), unitcell(2,1), unitcell(2,2)));
    unitcell_vertices.push_back(unitcell_vertices[1] + unitcell_vertices[2]);
    unitcell_vertices.push_back(unitcell_vertices[1] + unitcell_vertices[3]);
    unitcell_vertices.push_back(unitcell_vertices[2] + unitcell_vertices[3]);
    unitcell_vertices.push_back(unitcell_vertices[4] + unitcell_vertices[3]);

    this->vao_unitcell.bind();
    this->vbo_unitcell[0].bind();
    this->vbo_unitcell[0].allocate(&unitcell_vertices[0][0], unitcell_vertices.size() * 3 * sizeof(float));
    this->vao_sphere.release();
}

/**
 * @brief      Load all data to a vertex array object
 */
void StructureRenderer::load_sphere_to_vao() {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_sphere.create();
    this->vao_sphere.bind();

    this->vbo_sphere[0].create();
    this->vbo_sphere[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_sphere[0].bind();
    this->vbo_sphere[0].allocate(&this->sphere_vertices[0][0], this->sphere_vertices.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->vbo_sphere[1].create();
    this->vbo_sphere[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_sphere[1].bind();
    this->vbo_sphere[1].allocate(&this->sphere_normals[0][0], this->sphere_normals.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->vbo_sphere[2] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    this->vbo_sphere[2].create();
    this->vbo_sphere[2].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_sphere[2].bind();
    this->vbo_sphere[2].allocate(&this->sphere_indices[0], this->sphere_indices.size() * sizeof(unsigned int));

    this->vao_sphere.release();
}

/**
 * @brief      Load all data to a vertex array object
 */
void StructureRenderer::load_cylinder_to_vao() {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_cylinder.create();
    this->vao_cylinder.bind();

    this->vbo_cylinder[0].create();
    this->vbo_cylinder[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_cylinder[0].bind();
    this->vbo_cylinder[0].allocate(&this->cylinder_vertices[0][0], this->cylinder_vertices.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->vbo_cylinder[1].create();
    this->vbo_cylinder[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_cylinder[1].bind();
    this->vbo_cylinder[1].allocate(&this->cylinder_normals[0][0], this->cylinder_normals.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->vbo_cylinder[2] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    this->vbo_cylinder[2].create();
    this->vbo_cylinder[2].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_cylinder[2].bind();
    this->vbo_cylinder[2].allocate(&this->cylinder_indices[0], this->cylinder_indices.size() * sizeof(unsigned int));

    this->vao_sphere.release();
}

/**
 * @brief      Load simple line data to vertex array object
 */
void StructureRenderer::load_line_to_vao() {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_line.create();
    this->vao_line.bind();

    std::vector<glm::vec3> vertices(2);
    this->vbo_line[0].create();
    this->vbo_line[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_line[0].bind();
    this->vbo_line[0].allocate(&vertices[0][0], vertices.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    std::vector<unsigned int> indices = {0,1};
    this->vbo_line[1] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    this->vbo_line[1].create();
    this->vbo_line[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_line[1].bind();
    this->vbo_line[1].allocate(&indices[0], 2 * sizeof(unsigned int));

    this->vao_line.release();
}

/**
 * @brief      Load simple line data to vertex array object
 */
void StructureRenderer::load_plane_to_vao() {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->vao_plane.create();
    this->vao_plane.bind();

    std::vector<glm::vec3> vertices(4);
    this->vbo_plane[0].create();
    this->vbo_plane[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_plane[0].bind();
    this->vbo_plane[0].allocate(&vertices[0][0], vertices.size() * 3 * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    std::vector<unsigned int> indices = {0,1,3,1,2,3};
    this->vbo_plane[1] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    this->vbo_plane[1].create();
    this->vbo_plane[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vbo_plane[1].bind();
    this->vbo_plane[1].allocate(&indices[0], 6 * sizeof(unsigned int));

    this->vao_plane.release();
}

/**
 * @brief      Loads an arrow model.
 */
void StructureRenderer::load_arrow_model() {
    // load axis model
    QTemporaryDir tmp_dir;
    QString filename = "arrow.obj";
    QString path = tmp_dir.path() + "/" + filename;
    QFile::copy(":/assets/models/" + filename, path);
    ModelLoader modelloader;
    this->axis_model = modelloader.load_model(path.toStdString());
    this->axis_model->load_to_vao();
}

/**
 * @brief      Darken color
 *
 * @param[in]  color   The color
 * @param[in]  amount  The amount
 *
 * @return     The 3D vector.
 */
QVector3D StructureRenderer::darken(const QVector3D& color, float amount) const {
    return amount * QVector3D(0.0, 0.0, 0.0) + (1.0 - amount) * color;
}

/**
 * @brief      Lighten color
 *
 * @param[in]  color   The color
 * @param[in]  amount  The amount
 *
 * @return     The 3D vector.
 */
QVector3D StructureRenderer::lighten(const QVector3D& color, float amount) const {
    return amount * QVector3D(1.0, 1.0, 1.0) + (1.0 - amount) * color;
}

/**
 * @brief      Mix colors
 *
 * @param[in]  color1  First color
 * @param[in]  color2  Second color
 * @param[in]  amount  The amount
 *
 * @return     The 3D vector.
 */
QVector3D StructureRenderer::mix(const QVector3D& color1, const QVector3D& color2, float amount) const {
    return (1.0 - amount) * color1 + amount * color2;
}
