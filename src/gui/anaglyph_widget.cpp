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

#include "anaglyph_widget.h"

/**
 * @brief      Constructs a new instance.
 *
 * @param      parent  The parent
 */
AnaglyphWidget::AnaglyphWidget(QWidget *parent)
    : QOpenGLWidget(parent) {
    const QString operatingsystem = QSysInfo::productType();

    this->shader_manager = std::make_shared<ShaderProgramManager>();

    this->scene = std::make_shared<Scene>();
    this->scene->camera_position = QVector3D(0.0, -10.0f, 0.0);

    auto pTimer = new QTimer(this);
    pTimer->start(1000 / 60.0);

    // set default matrix orientation on start-up
    this->reset_matrices();

    // create a pointer to user actions
    this->user_action = std::make_shared<UserAction>(this->scene);

    // connect to user actions
    connect(this->user_action.get(), SIGNAL(request_update()), this, SLOT(call_update()));
    connect(this->user_action.get(), SIGNAL(transmit_message(const QString&)), this, SLOT(transmit_message(const QString&)));

    // enable mouse tracking
    this->setMouseTracking(true);
}

/**
 * @brief      Destroys the object.
 */
AnaglyphWidget::~AnaglyphWidget() {
    cleanup();
}

/**
 * @brief      Set the minimum window size
 *
 * @return     Minimimum window size
 */
QSize AnaglyphWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

/**
 * @brief      Provide default window size
 *
 * @return     Default window size
 */
QSize AnaglyphWidget::sizeHint() const {
    return QSize(400, 400);
}

/**
 * @brief      Clean the widget
 */
void AnaglyphWidget::cleanup() {
    makeCurrent();
    doneCurrent();
}

/**
 * @brief      Initialize OpenGL environment
 */
void AnaglyphWidget::initializeGL() {
    qDebug() << "Connecting to OpenGL Context";
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &AnaglyphWidget::cleanup);

    qDebug() << "Initialize OpenGL functions";
    this->initializeOpenGLFunctions();

    glClearColor(this->tint, this->tint, this->tint, 1.0f);

    qDebug() << "Load shaders";
    this->load_shaders();

    qDebug() << "Create structure renderer object";
    this->structure_renderer = std::make_unique<StructureRenderer>(this->scene,
                                                                   this->shader_manager,
                                                                   this->user_action);

    if(!this->flag_draw_unitcell) {
        qDebug() << "Draw unitcell";
        this->structure_renderer->disable_draw_unitcell();
    }

    qDebug() << "Build Framebuffers";
    this->build_framebuffers();

    qDebug() << "Emit openGL ready";
    emit(opengl_ready());
}

/**
 * @brief      Render scene
 */
void AnaglyphWidget::paintGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    if (this->stereographic_type_name == "NONE") {
        this->paint_regular();
    } else {
        this->paint_stereographic();
    }

    // paint coordinate axes on top of screen
    if(this->flag_axis_enabled) {
        this->structure_renderer->draw_coordinate_axes();
    }
}

/**
 * @brief      Paint the models in the models vector to the screen
 */
void AnaglyphWidget::draw_structure() {
    if(this->structure) {
        this->structure_renderer->draw(this->structure.get(), this->flag_show_periodicity_xy, this->flag_show_periodicity_z);
    }
}

/**
 * @brief      Set a new structure
 *
 * @param[in]  _structure  The structure
 */
void AnaglyphWidget::set_structure(const std::shared_ptr<Structure>& _structure) {
    this->structure = _structure;
    this->user_action->set_structure(structure);
    VectorPosition z = VectorPosition::Ones(3);
    auto p = structure->get_unitcell() * z * 1.5;
    this->scene->camera_position = QVector3D(0.0, -p.norm(), 0.0);
    this->update();
}

/**
 * @brief      Set a (new) structure
 *
 * Do not modify camera settings
 *
 * @param[in]  _structure  The structure
 */
void AnaglyphWidget::set_structure_conservative(const std::shared_ptr<Structure>& _structure) {
    this->structure = _structure;
    this->user_action->set_structure(structure);
    this->update();
}

/**
 * @brief      Resize window
 *
 * @param[in]  width   screen width
 * @param[in]  height  screen height
 */
void AnaglyphWidget::resizeGL(int w, int h) {
    this->scene->projection.setToIdentity();
    this->scene->projection.perspective(45.0f, GLfloat(w) / h, 0.01f, 1000.0f);

    // store sizes
    this->scene->canvas_width = w;
    this->scene->canvas_height = h;

    // resize textures and render buffers
    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, this->rbo[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    }
}

/**
 * @brief      Parse mouse press event
 *
 * @param      event  The event
 */
void AnaglyphWidget::mousePressEvent(QMouseEvent *event) {
    // handle dragging
    if (event->buttons() & Qt::LeftButton) {
        this->user_action->handle_left_mouse_click();

        // enable arcball rotation mode
        this->arcball_rotation_flag = true;

        // store positions of mouse
        this->m_lastPos = event->pos();
    }

    // handle selection
    if (event->buttons() & Qt::RightButton) {
        QVector3D ray_origin;
        QVector3D ray_direction;

        this->scene->calculate_ray(event->pos(), &ray_origin, &ray_direction);
        int selected_atom = this->get_atom_raycast(ray_origin, ray_direction);
        if(selected_atom != -1) {
            this->structure->select_atom(selected_atom);
            this->update();
        }

        emit(signal_selection_message(this->structure->get_selection_string()));
    }

    // custom menu
    if (event->buttons() & Qt::RightButton && event->modifiers() & Qt::ControlModifier) {
        this->custom_menu_requested(event->globalPosition().toPoint());
    }
}

/**
 * @brief      Parse mouse release event
 *
 * @param      event  The event
 */
void AnaglyphWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (this->arcball_rotation_flag && !(event->buttons() & Qt::LeftButton) ) {
        // make arcball rotation permanent (note that the multiplication order for matrices matters)
        this->scene->rotation_matrix = this->scene->arcball_rotation * this->scene->rotation_matrix;

        // reset the arcball rotation matrix
        this->scene->arcball_rotation.setToIdentity();

        // unset arcball rotation mode
        this->arcball_rotation_flag = false;
    }
}

/**
 * @brief      Parse mouse move event
 *
 * @param      event  The event
 */
void AnaglyphWidget::mouseMoveEvent(QMouseEvent *event) {
    this->user_action->update(this->mapFromGlobal(QCursor::pos()));
    this->setFocus(Qt::MouseFocusReason);

    if(this->arcball_rotation_flag) { // drag event
        // implementation adapted from
        // https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
        double ex = event->position().x();
        double ey = event->position().y();
        if(ex != this->m_lastPos.x() || ey != this->m_lastPos.y()) {

            // calculate arcball vectors
            QVector3D va = this->get_arcball_vector(this->m_lastPos.x(), this->m_lastPos.y());
            QVector3D vb = this->get_arcball_vector(ex, ey);

            // calculate angle between vectors
            float dotprod = QVector3D::dotProduct(va, vb);
            if (qFabs(dotprod) > 0.9999f) {
                return;
            }
            float angle = qAcos(qMin(1.0f, dotprod));

            // determine rotation vector in camera space
            QVector4D axis_cam_space = QVector4D(QVector3D::crossProduct(va, vb).normalized());

            // calculate matrix to change basis from camera to model space
            QMatrix3x3 camera_to_model_trans = this->scene->view.inverted().toGenericMatrix<3,3>();

            // determine rotation vector in model space
            QVector4D axis_model_space = QMatrix4x4(camera_to_model_trans) * axis_cam_space;

            // set the rotation
            this->set_arcball_rotation(qRadiansToDegrees(angle), axis_model_space);
        }
    } else if(event->buttons() & Qt::RightButton) {
        // Nothing for the time being.
    }
}

/**
 * @brief      Calculate the arcball vector for mouse rotation
 *
 * @param      int x    x-position of the mouse cursor on the screen
 * @param      int y    y-position of the mouse cursor on the screen
 */
QVector3D AnaglyphWidget::get_arcball_vector(int x, int y) {
    QVector3D P = QVector3D(1.0f * (float)x / (float)this->geometry().width() * 2.0f - 1.0f,
                            1.0f * (float)y / (float)this->geometry().height() * 2.0f - 1.0f,
                            0.0f);
    P[1] = -P[1];

    float OP_squared = P[0] * P[0] + P[1] * P[1];

    if(OP_squared <= 1.0f)
        P[2] = sqrt(1.0f - OP_squared);
    else
        P = P.normalized();
    return P;
}

/**
 * @brief      Sets the arcball rotation.
 *
 * @param[in]  arcball_angle   The arcball angle
 * @param[in]  arcball_vector  The arcball vector
 */
void AnaglyphWidget::set_arcball_rotation(float arcball_angle, const QVector4D& arcball_vector) {
    this->scene->arcball_rotation.setToIdentity();
    this->scene->arcball_rotation.rotate(arcball_angle, QVector3D(arcball_vector));
    this->update();
}


/**
 * @brief      Parse mouse wheel event
 *
 * @param      event  The event
 */
void AnaglyphWidget::wheelEvent(QWheelEvent *event) {

    this->scene->camera_position += event->angleDelta().ry() * 0.01f * QVector3D(0, 1, 0);
    if(this->scene->camera_position[1] > -5.0) {
        this->scene->camera_position[1] = -5.0;
    }

    float w = (float)this->scene->canvas_width;
    float h = (float)this->scene->canvas_height;
    float ratio = w/h;
    float zoom = -this->scene->camera_position[1];

    if(this->scene->camera_mode == CameraMode::ORTHOGRAPHIC) {
        this->scene->projection.setToIdentity();
        this->scene->projection.ortho(-zoom/2.0f, zoom/2.0f, -zoom / ratio /2.0f, zoom / ratio / 2.0f, 0.01f, 1000.0f);
    }

    this->update();
}

/**
 * @brief Update when moving window
 */
void AnaglyphWidget::window_move_event() {
    this->top_left = mapToGlobal(QPoint(0, 0));
    this->update();
}

/**
 * @brief Set stereographic projection type
 * @param stereographic projection name
 */
void AnaglyphWidget::set_stereo(QString stereo_name) {
    if (stereo_name.startsWith("stereo")) {
        this->stereographic_type_name = stereo_name;
    } else {
        this->stereographic_type_name = "NONE";
    }

    this->update();
}

/**
 * PRIVATE FUNCTIONS
 */

/**
 * @brief      Load OpenGL shaders
 */
void AnaglyphWidget::load_shaders() {
    // create regular shaders
    shader_manager->create_shader_program("model_shader", ShaderProgramType::ModelShader, ":/assets/shaders/phong.vs", ":/assets/shaders/phong.fs");
    shader_manager->create_shader_program("axes_shader", ShaderProgramType::AxesShader, ":/assets/shaders/axes.vs", ":/assets/shaders/axes.fs");
    shader_manager->create_shader_program("unitcell_shader", ShaderProgramType::UnitcellShader, ":/assets/shaders/line.vs", ":/assets/shaders/line.fs");
    shader_manager->create_shader_program("plane_shader", ShaderProgramType::PlaneShader, ":/assets/shaders/plane.vs", ":/assets/shaders/plane.fs");
    shader_manager->create_shader_program("silhouette_shader", ShaderProgramType::SilhouetteShader, ":/assets/shaders/silhouette.vs", ":/assets/shaders/silhouette.fs");

    // create shaders for the stereographic projections
    shader_manager->create_shader_program("stereo_anaglyph_red_cyan", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_anaglyph_red_cyan.fs");
    shader_manager->create_shader_program("stereo_interlaced_checkerboard_lr", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_checkerboard_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_checkerboard_rl", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_checkerboard_rl.fs");
    shader_manager->create_shader_program("stereo_interlaced_columns_lr", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_columns_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_columns_rl", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_columns_rl.fs");
    shader_manager->create_shader_program("stereo_interlaced_rows_lr", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_rows_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_rows_rl", ShaderProgramType::StereoscopicShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_rows_rl.fs");

    // load shader for the Canvas
    shader_manager->create_shader_program("canvas_shader", ShaderProgramType::CanvasShader, ":/assets/shaders/stereo.vs", ":/assets/shaders/canvas.fs");
}

/**
 * @brief      Build a canvas used for stereographic rendering
 */
void AnaglyphWidget::build_framebuffers() {
    // initialize frame buffers
    glGenFramebuffers(FrameBuffer::NR_FRAMEBUFFERS, this->framebuffers);
    glGenTextures(FrameBuffer::NR_FRAMEBUFFERS, this->texture_color_buffers);
    glGenRenderbuffers(FrameBuffer::NR_FRAMEBUFFERS, this->rbo);

    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[i]);
        glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->geometry().width(), this->geometry().height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture_color_buffers[i], 0);

        glBindRenderbuffer(GL_RENDERBUFFER, this->rbo[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->geometry().width(), this->geometry().height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo[i]);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            qWarning() << "Framebuffer is not complete.";
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create screen quad vao
    // TODO maybe move this away from here?
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    this->quad_vao.create();
    this->quad_vao.bind();

    this->quad_vbo.create();
    this->quad_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->quad_vbo.bind();
    this->quad_vbo.allocate(quadVertices, sizeof(quadVertices));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    this->quad_vao.release();
}


/**
 * @brief      Reset rotation matrices
 */
void AnaglyphWidget::reset_matrices() {
    this->scene->rotation_matrix.setToIdentity();
    this->scene->rotation_matrix.rotate(20.0, QVector3D(1,0,0));
    this->scene->rotation_matrix.rotate(30.0, QVector3D(0,0,1));
    this->scene->arcball_rotation.setToIdentity();
}

/**
 * @brief      get the closest atom from a raycast
 *
 * @param[in]  ray_origin  The ray origin
 * @param[in]  ray_vector  The ray vector
 * @param[in]  rot         rotation matrix
 *
 * Important: the coordinate system is set-up in such a way that the camera is always fixed on the negative y-axis.
 *            the structure is rotated and the camera is always locked to the center of the coordinate system
 *            atoms which are closest, thus have - from the perspective of the camera - the smallest y values
 *
 * @return     the atom, -1 if no atom is hit
 */
int AnaglyphWidget::get_atom_raycast(const QVector3D& ray_origin, const QVector3D& ray_vector) {
    int selected_atom = -1;
    float y = 1000;

    // unsigned int pickbit = this->calculate_bitset(bit);

    auto vec_ctr = this->structure->get_center_vector();

    QMatrix4x4 model;

    for(unsigned int i=0; i<this->structure->get_nr_atoms(); i++) {
        const Atom& atom = this->structure->get_atom(i);

        model.setToIdentity();
        model *= this->scene->rotation_matrix;
        model.translate(vec_ctr);
        QVector3D pos = model.map(atom.get_pos());

        float radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);
        float b = QVector3D::dotProduct(ray_vector, ray_origin - pos);
        float c = QVector3D::dotProduct(ray_origin - pos, ray_origin - pos) - (radius * radius);

        if(b*b >= c ) { // hit
            if(pos[1] < y) {
                selected_atom = i;
                y = pos[1];
            }
        }
    }

    for(unsigned int i=0; i<this->structure->get_atoms_expansion().size(); i++) {
        // perform translation call
        const auto& atom = structure->get_atoms_expansion()[i];

        if((this->flag_show_periodicity_xy && this->flag_show_periodicity_z) && (atom.atomtype & (1 << ATOM_EXPANSION_XY) || atom.atomtype & (1 < ATOM_EXPANSION_Z))) {
            // do nothing
        } else if(this->flag_show_periodicity_xy && atom.atomtype & (1 << ATOM_EXPANSION_XY) && !(atom.atomtype & (1 << ATOM_EXPANSION_Z))) {
            // do nothing
        } else if(this->flag_show_periodicity_z && atom.atomtype & (1 << ATOM_EXPANSION_Z) && !(atom.atomtype & (1 << ATOM_EXPANSION_XY))) {
            // do nothing
        } else {
            continue;
        }

        model.setToIdentity();
        model *= this->scene->arcball_rotation * this->scene->rotation_matrix;
        model.translate(vec_ctr);
        QVector3D pos = model.map(atom.get_pos());

        float radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);
        float b = QVector3D::dotProduct(ray_vector, ray_origin - pos);
        float c = QVector3D::dotProduct(ray_origin - pos, ray_origin - pos) - (radius * radius);

        if(b*b >= c ) { // hit
            if(pos[1] < y) {
                selected_atom = i + this->structure->get_nr_atoms();
                y = pos[1];
            }
        }
    }

    return selected_atom;
}

/**
 * @brief      Regular draw call
 */
void AnaglyphWidget::paint_regular() {
    // set draw settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // set view matrix
    QVector3D lookat = QVector3D(0.0f, 1.0f, 0.0f);
    this->scene->view.setToIdentity();
    this->scene->view.lookAt(this->scene->camera_position, lookat, QVector3D(0.0, 0.0, 1.0));

    // first perform a simple draw call for edge detection of any selected atoms by
    // drawing to the SILHOUETTE_NORMAL framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::SILHOUETTE_NORMAL]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(this->structure) {
        this->structure_renderer->draw_silhouette(this->structure.get());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // regular draw call to the STRUCTURE_NORMAL framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::STRUCTURE_NORMAL]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(this->tint, this->tint, this->tint, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->draw_structure();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // bundle both framebuffers and draw on the canvas
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    ShaderProgram *canvas_shader = this->shader_manager->get_shader_program("canvas_shader");
    canvas_shader->bind();

    // update screen coordinates
    canvas_shader->set_uniform("regular_texture", 0);
    canvas_shader->set_uniform("silhouette_texture", 1);

    // draw quad on screen
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::STRUCTURE_NORMAL]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::SILHOUETTE_NORMAL]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    this->quad_vao.release();

    // release shader
    canvas_shader->release();
}

/**
 * @brief      Stereographic draw call
 */
void AnaglyphWidget::paint_stereographic() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // set convergence point and intra-ocular separation
    QVector3D lookat = QVector3D(0.0f, 1.0f, 0.0f);
    float dist = 1.0f - this->scene->camera_position[1];
    float eye_sep = dist / 30.0f;

    // draw silhouette for left eye
    this->scene->view.setToIdentity();
    this->scene->view.lookAt(this->scene->camera_position - QVector3D(eye_sep / 2.0, 0.0, 0.0), lookat, QVector3D(0.0, 0.0, 1.0));
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::SILHOUETTE_LEFT]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->structure_renderer->draw_silhouette(this->structure.get());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw silhouette for right eye
    this->scene->view.setToIdentity();
    this->scene->view.lookAt(this->scene->camera_position + QVector3D(eye_sep / 2.0, 0.0, 0.0), lookat, QVector3D(0.0, 0.0, 1.0));
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::SILHOUETTE_RIGHT]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->structure_renderer->draw_silhouette(this->structure.get());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw structure for left eye
    this->scene->view.setToIdentity();
    this->scene->view.lookAt(this->scene->camera_position - QVector3D(eye_sep / 2.0, 0.0, 0.0), lookat, QVector3D(0.0, 0.0, 1.0));
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::STRUCTURE_LEFT]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(this->tint, this->tint, this->tint, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->draw_structure();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw structure for right eye
    this->scene->view.setToIdentity();
    this->scene->view.lookAt(this->scene->camera_position + QVector3D(eye_sep / 2.0, 0.0, 0.0), lookat, QVector3D(0.0, 0.0, 1.0));
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::STRUCTURE_RIGHT]);
    glEnable(GL_DEPTH_TEST);
    glClearColor(this->tint, this->tint, this->tint, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->draw_structure();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // combine L+L
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::ANAGLYPH_LEFT]);
    ShaderProgram *canvas_shader = this->shader_manager->get_shader_program("canvas_shader");
    canvas_shader->bind();

    // update screen coordinates
    canvas_shader->set_uniform("regular_texture", 0);
    canvas_shader->set_uniform("silhouette_texture", 1);

    // draw quad on screen
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    this->quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::STRUCTURE_LEFT]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::SILHOUETTE_LEFT]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    this->quad_vao.release();
    canvas_shader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // combine R+R
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffers[FrameBuffer::ANAGLYPH_RIGHT]);
    canvas_shader->bind();

    // update screen coordinates
    canvas_shader->set_uniform("regular_texture", 0);
    canvas_shader->set_uniform("silhouette_texture", 1);

    // draw quad on screen
    this->quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::STRUCTURE_RIGHT]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::SILHOUETTE_RIGHT]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    this->quad_vao.release();
    canvas_shader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // combine left and right for anaglyph
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    ShaderProgram *stereographic_shader = this->shader_manager->get_shader_program(this->stereographic_type_name.toUtf8().constData());
    stereographic_shader->bind();

    // update screen coordinates
    stereographic_shader->set_uniform("left_eye_texture", 0);
    stereographic_shader->set_uniform("right_eye_texture", 1);
    stereographic_shader->set_uniform("screen_x", this->top_left.x());
    stereographic_shader->set_uniform("screen_y", this->top_left.y());

    // draw quad on screen
    this->quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::ANAGLYPH_LEFT]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, this->texture_color_buffers[FrameBuffer::ANAGLYPH_RIGHT]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    this->quad_vao.release();

    stereographic_shader->release();
}

/**
 * @brief Create menu on screen when right clicking
 * @param pos
 */
void AnaglyphWidget::custom_menu_requested(QPoint pos) {
    QMenu *menu = new QMenu(this);
    menu->addAction(new QAction("Disable frequency", this));
    menu->addAction(new QAction("Enable frequency", this));
    menu->popup(pos);
}

void AnaglyphWidget::call_update() {
    this->update();
}

/**
 * @brief      Transmit a message to the user
 */
void AnaglyphWidget::transmit_message(const QString& text) {
    emit(signal_interaction_message(text));
}
