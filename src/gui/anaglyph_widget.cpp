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

#include <QAction>
#include <QGuiApplication>
#include <QMenu>
#include <QOpenGLContext>
#include <QTimer>
#include <QtMath>

namespace {

// Prefer the palette of the containing widget (what the user actually sees),
// fall back to this widget, then the application palette.
QColor effectiveWindowBg(const QWidget* w) {
    if (w && w->parentWidget()) {
        return w->parentWidget()->palette().color(QPalette::Window);
    }
    if (w) {
        return w->palette().color(QPalette::Window);
    }
    return QGuiApplication::palette().color(QPalette::Window);
}

} // namespace

AnaglyphWidget::AnaglyphWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    shader_manager = std::make_shared<ShaderProgramManager>();

    scene = std::make_shared<Scene>();
    scene->camera_position = QVector3D(0.0f, -10.0f, 0.0f);

    // Default matrix orientation on start-up
    reset_matrices();

    // Create a pointer to user actions
    user_action = std::make_shared<UserAction>(scene);

    // Connect to user actions (new-style connect)
    connect(user_action.get(), &UserAction::request_update,
            this, &AnaglyphWidget::call_update);
    connect(user_action.get(), &UserAction::transmit_message,
            this, &AnaglyphWidget::transmit_message);

    // 60 FPS update timer (was previously started but not connected)
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&AnaglyphWidget::update));
    timer->start(1000 / 60);

    setMouseTracking(true);

    // These can help avoid Qt painting a background behind/composited with the GL buffer.
    // (Safe for typical QOpenGLWidget usage.)
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
}

AnaglyphWidget::~AnaglyphWidget()
{
    cleanup();
}

QSize AnaglyphWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize AnaglyphWidget::sizeHint() const
{
    return QSize(400, 400);
}

void AnaglyphWidget::cleanup()
{
    makeCurrent();
    doneCurrent();
}

void AnaglyphWidget::initializeGL()
{
    qDebug() << "Connecting to OpenGL Context";
    connect(context(), &QOpenGLContext::aboutToBeDestroyed,
            this, &AnaglyphWidget::cleanup);

    qDebug() << "Initialize OpenGL functions";
    initializeOpenGLFunctions();

    // Clear color is set every frame where needed; keep a sane initial value here.
    const QColor bg = effectiveWindowBg(this);
    glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);

    qDebug() << "Load shaders";
    load_shaders();

    qDebug() << "Create structure renderer object";
    structure_renderer = std::make_unique<StructureRenderer>(scene, shader_manager, user_action);

    if (!flag_draw_unitcell) {
        qDebug() << "Draw unitcell disabled";
        structure_renderer->disable_draw_unitcell();
    }

    qDebug() << "Build Framebuffers";
    build_framebuffers();

    qDebug() << "Emit OpenGL ready";
    emit(opengl_ready());
}

void AnaglyphWidget::paintGL()
{
    // Coordinate axes to its own framebuffer
    if (flag_axis_enabled) {
        QOpenGLExtraFunctions* f =
            QOpenGLContext::currentContext()->extraFunctions();

        // ------------------------------------------------------------
        // Render axes into MSAA framebuffer
        // ------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::COORDINATE_AXES]);
        glEnable(GL_DEPTH_TEST);

        // Transparent background (will be alpha-blended later)
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        structure_renderer->draw_coordinate_axes();

        // ------------------------------------------------------------
        // Resolve MSAA → texture framebuffer
        // ------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::COORDINATE_AXES]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::COORDINATE_AXES]);

        f->glBlitFramebuffer(
            0, 0, scene->canvas_width, scene->canvas_height,
            0, 0, scene->canvas_width, scene->canvas_height,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Draw structure (regular or stereographic)
    if (stereographic_type_name == "NONE") {
        paint_regular();
    } else {
        paint_stereographic();
    }

    // Composite axes overlay onto final canvas
    if (flag_axis_enabled) {
        glDisable(GL_DEPTH_TEST);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        ShaderProgram* shader = shader_manager->get_shader_program("simple_canvas_shader");
        shader->bind();
        shader->set_uniform("regular_texture", 0);

        quad_vao.bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_color_buffers[FrameBuffer::COORDINATE_AXES]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        quad_vao.release();

        shader->release();
    }
}

void AnaglyphWidget::draw_structure()
{
    if (!structure) {
        return;
    }
    structure_renderer->draw(structure.get(), flag_show_periodicity_xy, flag_show_periodicity_z);
}

void AnaglyphWidget::set_structure(const std::shared_ptr<Structure>& s)
{
    structure = s;
    user_action->set_structure(structure);

    VectorPosition z = VectorPosition::Ones(3);
    auto p = structure->get_unitcell() * z * 1.5;
    scene->camera_position = QVector3D(0.0f, -p.norm(), 0.0f);

    update();
}

void AnaglyphWidget::set_structure_conservative(const std::shared_ptr<Structure>& s)
{
    structure = s;
    user_action->set_structure(structure);
    update();
}

void AnaglyphWidget::resizeGL(int w, int h)
{
    QOpenGLExtraFunctions* f =
        QOpenGLContext::currentContext()->extraFunctions();

    // Update projection
    scene->projection.setToIdentity();
    scene->projection.perspective(
        45.0f,
        GLfloat(w) / GLfloat(h),
        0.01f,
        1000.0f
    );

    scene->canvas_width  = w;
    scene->canvas_height = h;

    // ------------------------------------------------------------------
    // Resize resolved (texture) framebuffers
    // ------------------------------------------------------------------
    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        // Color texture
        glBindTexture(GL_TEXTURE_2D, texture_color_buffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            w,
            h,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            nullptr
        );

        // Depth/stencil renderbuffer (non-MSAA)
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            w,
            h
        );
    }

    // ------------------------------------------------------------------
    // Resize MSAA framebuffers
    // ------------------------------------------------------------------
    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        // MSAA color buffer
        glBindRenderbuffer(GL_RENDERBUFFER, msaa_color_rbo[i]);
        f->glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            MSAA_SAMPLES,
            GL_RGB8,
            w,
            h
        );

        // MSAA depth/stencil buffer
        glBindRenderbuffer(GL_RENDERBUFFER, msaa_depth_rbo[i]);
        f->glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            MSAA_SAMPLES,
            GL_DEPTH24_STENCIL8,
            w,
            h
        );
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AnaglyphWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        user_action->handle_left_mouse_click();
        arcball_rotation_flag = true;
        m_lastPos = event->pos();
    }

    if (event->buttons() & Qt::RightButton) {
        if (structure) {
            QVector3D ray_origin;
            QVector3D ray_direction;
            scene->calculate_ray(event->pos(), &ray_origin, &ray_direction);

            const int selected_atom = get_atom_raycast(ray_origin, ray_direction);
            if (selected_atom != -1) {
                structure->select_atom(selected_atom);
                update();
            }
            emit signal_selection_message(structure->get_selection_string());
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (event->modifiers() & Qt::ControlModifier) {
            custom_menu_requested(event->globalPosition().toPoint());
        }
#else
        if (event->modifiers() & Qt::ControlModifier) {
            custom_menu_requested(event->globalPos());
        }
#endif
    }
}

void AnaglyphWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (arcball_rotation_flag && !(event->buttons() & Qt::LeftButton)) {
        // make arcball rotation permanent (multiplication order matters)
        scene->rotation_matrix = scene->arcball_rotation * scene->rotation_matrix;

        scene->arcball_rotation.setToIdentity();
        arcball_rotation_flag = false;
    }
}

void AnaglyphWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Qt 5: widget-local logical coordinates
    const QPointF logicalPos = event->localPos();

    // Device pixel ratio of the OpenGL widget
    const qreal dpr = this->devicePixelRatioF();

    // Pass logical coords + DPR
    user_action->update(logicalPos, dpr);

    setFocus(Qt::MouseFocusReason);

    if (!arcball_rotation_flag) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const double ex = event->position().x();
    const double ey = event->position().y();
#else
    const double ex = event->pos().x();
    const double ey = event->pos().y();
#endif

    if (ex == m_lastPos.x() && ey == m_lastPos.y()) {
        return;
    }

    // Arcball rotation (adapted from wikibooks)
    const QVector3D va = get_arcball_vector(m_lastPos.x(), m_lastPos.y());
    const QVector3D vb = get_arcball_vector(ex, ey);

    const float dotprod = QVector3D::dotProduct(va, vb);
    if (qFabs(dotprod) > 0.9999f) {
        return;
    }

    const float angle = qAcos(qMin(1.0f, dotprod));

    const QVector4D axis_cam_space(QVector3D::crossProduct(va, vb).normalized());
    const QMatrix3x3 camera_to_model_trans = scene->view.inverted().toGenericMatrix<3, 3>();
    const QVector4D axis_model_space = QMatrix4x4(camera_to_model_trans) * axis_cam_space;

    set_arcball_rotation(qRadiansToDegrees(angle), axis_model_space);
}

QVector3D AnaglyphWidget::get_arcball_vector(int x, int y)
{
    QVector3D p( 1.0f * float(x) / float(geometry().width())  * 2.0f - 1.0f,
                 1.0f * float(y) / float(geometry().height()) * 2.0f - 1.0f,
                 0.0f);
    p[1] = -p[1];

    const float op2 = p[0] * p[0] + p[1] * p[1];
    if (op2 <= 1.0f) {
        p[2] = std::sqrt(1.0f - op2);
    } else {
        p = p.normalized();
    }
    return p;
}

void AnaglyphWidget::set_arcball_rotation(float arcball_angle, const QVector4D& arcball_vector)
{
    scene->arcball_rotation.setToIdentity();
    scene->arcball_rotation.rotate(arcball_angle, QVector3D(arcball_vector));
    update();
}

void AnaglyphWidget::wheelEvent(QWheelEvent* event)
{
    scene->camera_position += event->angleDelta().ry() * 0.01f * QVector3D(0, 1, 0);
    if (scene->camera_position[1] > -5.0f) {
        scene->camera_position[1] = -5.0f;
    }

    const float w = float(scene->canvas_width);
    const float h = float(scene->canvas_height);
    const float ratio = w / h;
    const float zoom = -scene->camera_position[1];

    if (scene->camera_mode == CameraMode::ORTHOGRAPHIC) {
        scene->projection.setToIdentity();
        scene->projection.ortho(-zoom / 2.0f, zoom / 2.0f,
                               -zoom / ratio / 2.0f, zoom / ratio / 2.0f,
                                0.01f, 1000.0f);
    }

    update();
}

void AnaglyphWidget::window_move_event()
{
    top_left = mapToGlobal(QPoint(0, 0));
    update();
}

void AnaglyphWidget::set_stereo(QString stereo_name)
{
    stereographic_type_name = stereo_name.startsWith("stereo") ? stereo_name : "NONE";
    update();
}

/* PRIVATE */

void AnaglyphWidget::load_shaders()
{
    shader_manager->create_shader_program("model_shader", ShaderProgramType::ModelShader,
                                         ":/assets/shaders/phong.vs", ":/assets/shaders/phong.fs");
    shader_manager->create_shader_program("axes_shader", ShaderProgramType::AxesShader,
                                         ":/assets/shaders/axes.vs", ":/assets/shaders/axes.fs");
    shader_manager->create_shader_program("unitcell_shader", ShaderProgramType::UnitcellShader,
                                         ":/assets/shaders/line.vs", ":/assets/shaders/line.fs");
    shader_manager->create_shader_program("plane_shader", ShaderProgramType::PlaneShader,
                                         ":/assets/shaders/plane.vs", ":/assets/shaders/plane.fs");
    shader_manager->create_shader_program("silhouette_shader", ShaderProgramType::SilhouetteShader,
                                         ":/assets/shaders/silhouette.vs", ":/assets/shaders/silhouette.fs");

    shader_manager->create_shader_program("stereo_anaglyph_red_cyan", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_anaglyph_red_cyan.fs");
    shader_manager->create_shader_program("stereo_interlaced_checkerboard_lr", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_checkerboard_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_checkerboard_rl", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_checkerboard_rl.fs");
    shader_manager->create_shader_program("stereo_interlaced_columns_lr", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_columns_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_columns_rl", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_columns_rl.fs");
    shader_manager->create_shader_program("stereo_interlaced_rows_lr", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_rows_lr.fs");
    shader_manager->create_shader_program("stereo_interlaced_rows_rl", ShaderProgramType::StereoscopicShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/stereo_interlaced_rows_rl.fs");

    shader_manager->create_shader_program("canvas_shader", ShaderProgramType::CanvasShader,
                                         ":/assets/shaders/stereo.vs", ":/assets/shaders/canvas.fs");
    shader_manager->create_shader_program("simple_canvas_shader", ShaderProgramType::SimpleCanvasShader,
                                         ":/assets/shaders/simplecanvas.vs", ":/assets/shaders/simplecanvas.fs");
}

void AnaglyphWidget::build_framebuffers()
{
    QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

    const int w = geometry().width();
    const int h = geometry().height();

    // ---------------------------------------------------------------------
    // RESOLVED (texture) framebuffers – what shaders will sample from
    // ---------------------------------------------------------------------
    glGenFramebuffers(FrameBuffer::NR_FRAMEBUFFERS, framebuffers);
    glGenTextures(FrameBuffer::NR_FRAMEBUFFERS, texture_color_buffers);
    glGenRenderbuffers(FrameBuffer::NR_FRAMEBUFFERS, rbo);

    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);

        glBindTexture(GL_TEXTURE_2D, texture_color_buffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture_color_buffers[i], 0);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, rbo[i]);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            qWarning() << "Resolved framebuffer incomplete";
        }
    }

    // ---------------------------------------------------------------------
    // MSAA framebuffers – rendering happens here
    // ---------------------------------------------------------------------
    glGenFramebuffers(FrameBuffer::NR_FRAMEBUFFERS, msaa_fbo);
    glGenRenderbuffers(FrameBuffer::NR_FRAMEBUFFERS, msaa_color_rbo);
    glGenRenderbuffers(FrameBuffer::NR_FRAMEBUFFERS, msaa_depth_rbo);

    for (unsigned int i = 0; i < FrameBuffer::NR_FRAMEBUFFERS; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[i]);

        // Multisampled color buffer
        glBindRenderbuffer(GL_RENDERBUFFER, msaa_color_rbo[i]);
        f->glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            MSAA_SAMPLES,
            GL_RGB8,
            w, h
        );
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, msaa_color_rbo[i]);

        // Multisampled depth+stencil buffer
        glBindRenderbuffer(GL_RENDERBUFFER, msaa_depth_rbo[i]);
        f->glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            MSAA_SAMPLES,
            GL_DEPTH24_STENCIL8,
            w, h
        );
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, msaa_depth_rbo[i]);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            qWarning() << "MSAA framebuffer incomplete";
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------------------------------------------------------------------
    // Screen quad VAO / VBO (unchanged)
    // ---------------------------------------------------------------------
    const float quadvecs[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    quad_vao.create();
    quad_vao.bind();

    quad_vbo.create();
    quad_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    quad_vbo.bind();
    quad_vbo.allocate(quadvecs, sizeof(quadvecs));

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                             4 * sizeof(float), nullptr);

    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                             4 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));

    quad_vao.release();
}

void AnaglyphWidget::reset_matrices()
{
    scene->rotation_matrix.setToIdentity();
    scene->rotation_matrix.rotate(20.0f, QVector3D(1, 0, 0));
    scene->rotation_matrix.rotate(30.0f, QVector3D(0, 0, 1));
    scene->arcball_rotation.setToIdentity();
}

int AnaglyphWidget::get_atom_raycast(const QVector3D& ray_origin, const QVector3D& ray_vector)
{
    if (!structure) {
        return -1;
    }

    int selected_atom = -1;
    float best_y = 1000.0f;

    const auto vec_ctr = structure->get_center_vector();
    QMatrix4x4 model;

    // Base atoms
    for (unsigned int i = 0; i < structure->get_nr_atoms(); ++i) {
        const Atom& atom = structure->get_atom(i);

        model.setToIdentity();
        model *= scene->rotation_matrix;
        model.translate(vec_ctr);

        const QVector3D pos = model.map(atom.get_pos_qtvec());

        const float radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);
        const float b = QVector3D::dotProduct(ray_vector, ray_origin - pos);
        const float c = QVector3D::dotProduct(ray_origin - pos, ray_origin - pos) - (radius * radius);

        if (b * b >= c) { // hit
            if (pos[1] < best_y) {
                selected_atom = int(i);
                best_y = pos[1];
            }
        }
    }

    // Expanded atoms
    const auto& exp = structure->get_atoms_expansion();
    for (unsigned int i = 0; i < exp.size(); ++i) {
        const auto& atom = exp[i];

        const bool is_xy = (atom.atomtype & (1 << ATOM_EXPANSION_XY));
        const bool is_z  = (atom.atomtype & (1 << ATOM_EXPANSION_Z)); // FIXED: was (1 < ATOM_EXPANSION_Z)

        // Keep original intent but make it readable
        const bool show_this =
            (flag_show_periodicity_xy && flag_show_periodicity_z && (is_xy || is_z)) ||
            (flag_show_periodicity_xy && is_xy && !is_z) ||
            (flag_show_periodicity_z  && is_z  && !is_xy);

        if (!show_this) {
            continue;
        }

        model.setToIdentity();
        model *= (scene->arcball_rotation * scene->rotation_matrix);
        model.translate(vec_ctr);

        const QVector3D pos = model.map(atom.get_pos_qtvec());

        const float radius = AtomSettings::get().get_atom_radius_from_elnr(atom.atnr);
        const float b = QVector3D::dotProduct(ray_vector, ray_origin - pos);
        const float c = QVector3D::dotProduct(ray_origin - pos, ray_origin - pos) - (radius * radius);

        if (b * b >= c) {
            if (pos[1] < best_y) {
                selected_atom = int(i) + int(structure->get_nr_atoms());
                best_y = pos[1];
            }
        }
    }

    return selected_atom;
}

void AnaglyphWidget::paint_regular()
{
    QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // View matrix
    const QVector3D lookat(0.0f, 1.0f, 0.0f);
    scene->view.setToIdentity();
    scene->view.lookAt(scene->camera_position, lookat, QVector3D(0.0f, 0.0f, 1.0f));

    // ============================================================
    // SILHOUETTE PASS (MSAA)
    // ============================================================
    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_NORMAL]);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (structure) {
        structure_renderer->draw_silhouette(structure.get());
    }

    // Resolve MSAA → texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_NORMAL]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::SILHOUETTE_NORMAL]);
    f->glBlitFramebuffer(
        0, 0, scene->canvas_width, scene->canvas_height,
        0, 0, scene->canvas_width, scene->canvas_height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    // ============================================================
    // STRUCTURE PASS (MSAA)
    // ============================================================
    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_NORMAL]);
    glEnable(GL_DEPTH_TEST);

    QColor bg = parentWidget()
        ? parentWidget()->palette().color(QPalette::Window)
        : palette().color(QPalette::Window);

    glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_structure();

    // Resolve MSAA → texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_NORMAL]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::STRUCTURE_NORMAL]);
    f->glBlitFramebuffer(
        0, 0, scene->canvas_width, scene->canvas_height,
        0, 0, scene->canvas_width, scene->canvas_height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ============================================================
    // COMPOSITE TO SCREEN
    // ============================================================
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    ShaderProgram* canvas_shader = shader_manager->get_shader_program("canvas_shader");
    canvas_shader->bind();
    canvas_shader->set_uniform("regular_texture", 0);
    canvas_shader->set_uniform("silhouette_texture", 1);

    quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D,
        texture_color_buffers[FrameBuffer::STRUCTURE_NORMAL]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D,
        texture_color_buffers[FrameBuffer::SILHOUETTE_NORMAL]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    quad_vao.release();

    canvas_shader->release();
}

void AnaglyphWidget::paint_stereographic()
{
    QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    const QVector3D lookat(0.0f, 1.0f, 0.0f);
    const float dist = 1.0f - scene->camera_position[1];
    const float eye_sep = dist / 30.0f;

    // ------------------------------------------------------------
    // LEFT SILHOUETTE (MSAA)
    // ------------------------------------------------------------
    scene->view.setToIdentity();
    scene->view.lookAt(scene->camera_position - QVector3D(eye_sep / 2.0f, 0, 0),
                       lookat, QVector3D(0, 0, 1));

    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_LEFT]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (structure) {
        structure_renderer->draw_silhouette(structure.get());
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_LEFT]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::SILHOUETTE_LEFT]);
    f->glBlitFramebuffer(0, 0, scene->canvas_width, scene->canvas_height,
                      0, 0, scene->canvas_width, scene->canvas_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // ------------------------------------------------------------
    // RIGHT SILHOUETTE (MSAA)
    // ------------------------------------------------------------
    scene->view.setToIdentity();
    scene->view.lookAt(scene->camera_position + QVector3D(eye_sep / 2.0f, 0, 0),
                       lookat, QVector3D(0, 0, 1));

    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_RIGHT]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (structure) {
        structure_renderer->draw_silhouette(structure.get());
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::SILHOUETTE_RIGHT]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::SILHOUETTE_RIGHT]);
    f->glBlitFramebuffer(0, 0, scene->canvas_width, scene->canvas_height,
                      0, 0, scene->canvas_width, scene->canvas_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // ------------------------------------------------------------
    // LEFT STRUCTURE (MSAA)
    // ------------------------------------------------------------
    scene->view.setToIdentity();
    scene->view.lookAt(scene->camera_position - QVector3D(eye_sep / 2.0f, 0, 0),
                       lookat, QVector3D(0, 0, 1));

    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_LEFT]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_structure();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_LEFT]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::STRUCTURE_LEFT]);
    f->glBlitFramebuffer(0, 0, scene->canvas_width, scene->canvas_height,
                      0, 0, scene->canvas_width, scene->canvas_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // ------------------------------------------------------------
    // RIGHT STRUCTURE (MSAA)
    // ------------------------------------------------------------
    scene->view.setToIdentity();
    scene->view.lookAt(scene->camera_position + QVector3D(eye_sep / 2.0f, 0, 0),
                       lookat, QVector3D(0, 0, 1));

    glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_RIGHT]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_structure();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo[FrameBuffer::STRUCTURE_RIGHT]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[FrameBuffer::STRUCTURE_RIGHT]);
    f->glBlitFramebuffer(0, 0, scene->canvas_width, scene->canvas_height,
                      0, 0, scene->canvas_width, scene->canvas_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    // FINAL ANAGLYPH COMPOSITE (unchanged)
    // ------------------------------------------------------------
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    ShaderProgram* stereo_shader =
        shader_manager->get_shader_program(stereographic_type_name.toUtf8().constData());

    stereo_shader->bind();
    stereo_shader->set_uniform("left_eye_texture", 0);
    stereo_shader->set_uniform("right_eye_texture", 1);
    stereo_shader->set_uniform("screen_x", top_left.x());
    stereo_shader->set_uniform("screen_y", top_left.y());

    quad_vao.bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D,
        texture_color_buffers[FrameBuffer::STRUCTURE_LEFT]);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D,
        texture_color_buffers[FrameBuffer::STRUCTURE_RIGHT]);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    quad_vao.release();

    stereo_shader->release();
}

void AnaglyphWidget::custom_menu_requested(QPoint pos)
{
    auto* menu = new QMenu(this);
    menu->addAction(new QAction("Disable frequency", menu));
    menu->addAction(new QAction("Enable frequency", menu));
    menu->popup(pos);
}

void AnaglyphWidget::call_update()
{
    update();
}

void AnaglyphWidget::transmit_message(const QString& text)
{
    emit signal_interaction_message(text);
}
