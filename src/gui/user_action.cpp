#include "user_action.h"
#include <QKeyEvent>

/* ============================================================
 *  Construction / update
 * ============================================================ */

/**
 * @brief Construct a UserAction controller.
 *
 * @param _scene Shared scene object used for camera and transforms
 */
UserAction::UserAction(const std::shared_ptr<Scene>& _scene)
    : scene(_scene) {
}

/**
 * @brief Update cursor position and recompute transform if active.
 *
 * @param cursor_pos_logical Cursor position in logical pixels
 * @param dpr Device pixel ratio
 */
void UserAction::update(const QPointF& cursor_pos_logical, qreal dpr) {
    this->cursor_position_now = QPointF(
        cursor_pos_logical.x() * dpr,
        cursor_pos_logical.y() * dpr
    );

    if(this->movement_action != MovementAction::MOVEMENT_NONE ||
       this->rotation_action != RotationAction::ROTATION_NONE) {
        this->calculate_transposition_matrix();
        emit request_update();
    }
}

/* ============================================================
 *  Internal helpers
 * ============================================================ */

/**
 * @brief Check whether no movement or rotation mode is active.
 *
 * @return true if idle, false otherwise
 */
bool UserAction::idle_only() const {
    return this->movement_action == MovementAction::MOVEMENT_NONE &&
           this->rotation_action == RotationAction::ROTATION_NONE;
}

/**
 * @brief Check whether a primary buffer exists.
 *
 * @return true if primary buffer contains atoms
 */
bool UserAction::has_primary() const {
    return this->structure &&
           this->structure->get_nr_atoms_primary_buffer() != 0;
}

/**
 * @brief Check whether a secondary buffer exists.
 *
 * @return true if secondary buffer contains atoms
 */
bool UserAction::has_secondary() const {
    return this->structure &&
           this->structure->get_nr_atoms_secondary_buffer() != 0;
}

/* ============================================================
 *  Action toggles
 * ============================================================ */

/**
 * @brief Toggle movement mode or commit an active movement.
 */
void UserAction::handle_action_movement() {
    if(!has_primary()) return;

    if(this->movement_action == MovementAction::MOVEMENT_NONE) {
        this->movement_action = MovementAction::MOVEMENT_FREE;
        this->cursor_position_start = this->cursor_position_now;
        emit transmit_message("<b>Move atoms</b> | Free movement");
        emit request_update();
        return;
    }

    this->movement_action = MovementAction::MOVEMENT_NONE;
    emit signal_push_structure();
    this->structure->commit_transposition(this->scene->transposition);
    this->scene->transposition.setToIdentity();

    emit transmit_message("");
    emit request_update();
}

/**
 * @brief Toggle rotation mode or commit an active rotation.
 */
void UserAction::handle_action_rotation() {
    if(!has_primary()) return;

    if(this->rotation_action == RotationAction::ROTATION_NONE) {
        this->rotation_action = RotationAction::ROTATION_FREE;
        this->cursor_position_start = this->cursor_position_now;
        emit transmit_message("<b>Rotate atoms</b> | Free rotation");
        emit request_update();
        return;
    }

    this->rotation_action = RotationAction::ROTATION_NONE;
    emit signal_push_structure();
    this->structure->commit_transposition(this->scene->transposition);
    this->scene->transposition.setToIdentity();

    emit transmit_message("");
    emit request_update();
}

/**
 * @brief Finalize movement or rotation on mouse click.
 */
void UserAction::handle_left_mouse_click() {
    if(this->movement_action != MovementAction::MOVEMENT_NONE)
        handle_action_movement();

    if(this->rotation_action != RotationAction::ROTATION_NONE)
        handle_action_rotation();
}

/* ============================================================
 *  Semantic command hooks
 * ============================================================ */

/**
 * @brief Toggle movement mode.
 */
void UserAction::cmd_toggle_move() {
    if(this->rotation_action == RotationAction::ROTATION_NONE)
        handle_action_movement();
}

/**
 * @brief Toggle rotation mode.
 */
void UserAction::cmd_toggle_rotate() {
    if(this->movement_action == MovementAction::MOVEMENT_NONE)
        handle_action_rotation();
}

/**
 * @brief Set movement alignment mode.
 *
 * @param mode Movement action to activate
 * @param msg Status message to display
 */
void UserAction::cmd_set_move(MovementAction mode, const QString& msg) {
    if(this->movement_action == MovementAction::MOVEMENT_NONE) return;
    if(mode == MovementAction::MOVEMENT_FOCUS && !has_secondary()) return;

    this->movement_action = mode;
    emit transmit_message(msg);
    emit request_update();
}

/**
 * @brief Set rotation alignment mode.
 *
 * @param mode Rotation action to activate
 * @param msg Status message to display
 */
void UserAction::cmd_set_rotate(RotationAction mode, const QString& msg) {
    if(this->rotation_action == RotationAction::ROTATION_NONE) return;
    if((mode == RotationAction::ROTATION_FOCUS ||
        mode == RotationAction::ROTATION_REVOLVE_SECONDARY ||
        mode == RotationAction::ROTATION_REVOLVE_CAMERA) &&
       !has_secondary()) return;

    this->rotation_action = mode;
    emit transmit_message(msg);
    emit request_update();
}

/* ============================================================
 *  Selection / editing
 * ============================================================ */

/**
 * @brief Select all atoms.
 */
void UserAction::cmd_select_all() {
    if(!idle_only()) return;
    this->structure->select_all_atoms();
    emit signal_selection_message(this->structure->get_selection_string());
    emit request_update();
}

/**
 * @brief Clear atom selection.
 */
void UserAction::cmd_deselect_all() {
    if(!idle_only()) return;
    this->structure->clear_selection();
    emit signal_selection_message(this->structure->get_selection_string());
    emit request_update();
}

/**
 * @brief Invert atom selection.
 */
void UserAction::cmd_invert_selection() {
    if(!idle_only()) return;
    this->structure->invert_selection();
    emit signal_selection_message(this->structure->get_selection_string());
    emit request_update();
}

/**
 * @brief Delete selected atoms.
 */
void UserAction::cmd_delete_selection() {
    if(!idle_only() || !has_primary()) return;
    emit signal_push_structure();
    this->structure->delete_atoms();
    this->structure->clear_selection();
    emit request_update();
}

/**
 * @brief Freeze selected atoms.
 */
void UserAction::cmd_set_frozen() {
    if(!idle_only()) return;
    emit signal_push_structure();
    this->structure->set_frozen();
    emit request_update();
}

/**
 * @brief Unfreeze selected atoms.
 */
void UserAction::cmd_set_unfrozen() {
    if(!idle_only()) return;
    emit signal_push_structure();
    this->structure->set_unfrozen();
    emit request_update();
}

/**
 * @brief Add a fragment to the current structure.
 */
void UserAction::cmd_add_fragment() {
    if(!idle_only()) return;
    add_fragment();
}

/* ============================================================
 *  Keyboard input
 * ============================================================ */

/**
 * @brief Handle keyboard input for structure manipulation.
 *
 * @param event Qt key event
 * @return true if event was handled
 */
bool UserAction::handle_key(QKeyEvent* event) {
    const int key = event->key();
    const auto mods = event->modifiers();

    if(key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt)
        return false;

    if(key == Qt::Key_G) { cmd_toggle_move(); return true; }
    if(key == Qt::Key_R) { cmd_toggle_rotate(); return true; }

    if(this->movement_action != MovementAction::MOVEMENT_NONE) {
        switch(key) {
            case Qt::Key_X: cmd_set_move(MovementAction::MOVEMENT_X, "<b>Move atoms</b> | X-alignment"); break;
            case Qt::Key_Y: cmd_set_move(MovementAction::MOVEMENT_Y, "<b>Move atoms</b> | Y-alignment"); break;
            case Qt::Key_Z: cmd_set_move(MovementAction::MOVEMENT_Z, "<b>Move atoms</b> | Z-alignment"); break;
            case Qt::Key_F: cmd_set_move(MovementAction::MOVEMENT_FOCUS, "<b>Move atoms</b> | Focus alignment"); break;
            default: return false;
        }
        return true;
    }

    if(this->rotation_action != RotationAction::ROTATION_NONE) {
        switch(key) {
            case Qt::Key_X: cmd_set_rotate(RotationAction::ROTATION_X, "<b>Rotate atoms</b> | X-alignment"); break;
            case Qt::Key_Y: cmd_set_rotate(RotationAction::ROTATION_Y, "<b>Rotate atoms</b> | Y-alignment"); break;
            case Qt::Key_Z: cmd_set_rotate(RotationAction::ROTATION_Z, "<b>Rotate atoms</b> | Z-alignment"); break;
            case Qt::Key_F: cmd_set_rotate(RotationAction::ROTATION_FOCUS, "<b>Rotate atoms</b> | Focus"); break;
            case Qt::Key_S: cmd_set_rotate(RotationAction::ROTATION_REVOLVE_SECONDARY, "<b>Rotate atoms</b> | Revolve secondary"); break;
            case Qt::Key_C: cmd_set_rotate(RotationAction::ROTATION_REVOLVE_CAMERA, "<b>Rotate atoms</b> | Revolve camera"); break;
            default: return false;
        }
        return true;
    }

    if(mods & Qt::ControlModifier) {
        if((mods & Qt::ShiftModifier) && key == Qt::Key_F) {
            cmd_set_unfrozen();
            return true;
        }

        switch(key) {
            case Qt::Key_A: cmd_select_all(); break;
            case Qt::Key_D: cmd_deselect_all(); break;
            case Qt::Key_I: cmd_invert_selection(); break;
            case Qt::Key_Z: emit signal_decrement_structure_stack_pointer(); break;
            case Qt::Key_Y: emit signal_increment_structure_stack_pointer(); break;
            case Qt::Key_F: cmd_set_frozen(); break;
            default: return false;
        }
        return true;
    }

    if(key == Qt::Key_A && (mods & Qt::ShiftModifier)) {
        cmd_add_fragment();
        return true;
    }

    if(key == Qt::Key_Delete) {
        cmd_delete_selection();
        return true;
    }

    return false;
}

/* ============================================================
 *  Fragment / camera
 * ============================================================ */

/**
 * @brief Set the current fragment used for insertion.
 *
 * @param _fragment Fragment to insert
 */
void UserAction::set_fragment(const Fragment& _fragment) {
    this->fragment = std::make_unique<Fragment>(_fragment);
}

/**
 * @brief Align the camera to a predefined direction.
 *
 * @param direction CameraAlignment enum value
 */
void UserAction::set_camera_alignment(int direction) {
    QVector3D dirvec;

    switch((CameraAlignment)direction) {
        case CameraAlignment::DEFAULT:
            this->scene->rotation_matrix.setToIdentity();
            this->scene->rotation_matrix.rotate(20.0, QVector3D(1,0,0));
            this->scene->rotation_matrix.rotate(30.0, QVector3D(0,0,1));
            emit request_update();
            return;
        case CameraAlignment::TOP:    dirvec = QVector3D(0,0,1); break;
        case CameraAlignment::BOTTOM: dirvec = QVector3D(0,0,-1); break;
        case CameraAlignment::LEFT:   dirvec = QVector3D(-1,0,0); break;
        case CameraAlignment::RIGHT:  dirvec = QVector3D(1,0,0); break;
        case CameraAlignment::FRONT:  dirvec = QVector3D(0,1,0); break;
        case CameraAlignment::BACK:   dirvec = QVector3D(0,-1,0); break;
    }

    QVector3D axis;
    float angle;

    if(fabs(dirvec[1]) > .999) {
        axis = QVector3D(0,0,1);
        angle = dirvec[1] < 0 ? -M_PI : 0.0;
    } else {
        axis = QVector3D::crossProduct(QVector3D(0,1,0), dirvec);
        angle = std::acos(dirvec[1]);
    }

    this->scene->rotation_matrix.setToIdentity();
    this->scene->rotation_matrix.rotate(qRadiansToDegrees(angle), axis);
    emit signal_message_statusbar("Change camera alignment");
    emit request_update();
}

/**
 * @brief Set the camera projection mode.
 *
 * @param mode CameraMode enum value
 */
void UserAction::set_camera_mode(int mode) {
    float w = this->scene->canvas_width;
    float h = this->scene->canvas_height;

    if((CameraMode)mode == CameraMode::PERSPECTIVE) {
        this->scene->camera_mode = CameraMode::PERSPECTIVE;
        this->scene->projection.setToIdentity();
        this->scene->projection.perspective(45.0f, w/h, 0.01f, 1000.0f);
    } else {
        this->scene->camera_mode = CameraMode::ORTHOGRAPHIC;
        float ratio = w/h;
        float zoom = -this->scene->camera_position[1];
        this->scene->projection.setToIdentity();
        this->scene->projection.ortho(-zoom/2, zoom/2,
                                      -zoom/ratio/2, zoom/ratio/2,
                                      0.01f, 1000.0f);
    }

    emit request_update();
}

/* ============================================================
 *  Fragment insertion
 * ============================================================ */

/**
 * @brief Insert a fragment at the current selection.
 */
void UserAction::add_fragment() {
    try {
        this->structure->get_position_primary_buffer();
        this->structure->get_position_secondary_buffer();

        bool ok;
        double distance = QInputDialog::getDouble(
            nullptr, tr("Set fragment distance"),
            tr("Distance in angstrom:"), 1.2, 0.5, 3.5, 2, &ok);

        if(!ok) return;

        emit signal_push_structure();

        if(this->fragment) {
            this->structure_operator.add_fragment(
                this->structure.get(), *this->fragment, distance);
        } else {
            throw std::runtime_error("No fragment is set");
        }

        emit request_update();
    } catch(const std::exception& e) {
        QMessageBox::critical(nullptr, tr("Exception encountered"), tr(e.what()));
    }
}

/* ============================================================
 *  Transposition math
 * ============================================================ */

/**
 * @brief Compute the current transposition matrix.
 */
void UserAction::calculate_transposition_matrix() {
    if(this->movement_action != MovementAction::MOVEMENT_NONE) {
        QVector3D ray_origin, ray_direction;
        this->scene->calculate_ray(this->cursor_position_start, &ray_origin, &ray_direction);

        auto pos = this->scene->rotation_matrix.map(
            this->structure->get_position_primary_buffer() +
            this->structure->get_center_vector());

        auto source = this->scene->calculate_ray_plane_intersection(
            ray_origin, ray_direction, pos, -this->scene->camera_position);

        this->scene->calculate_ray(this->cursor_position_now, &ray_origin, &ray_direction);
        auto target = this->scene->calculate_ray_plane_intersection(
            ray_origin, ray_direction, pos, -this->scene->camera_position);

        this->scene->transposition.setToIdentity();
        const QVector3D delta_model =
            this->scene->rotation_matrix.inverted().map(target - source);
        this->scene->transposition.translate(project_movement_vector(delta_model));
        return;
    }

    if(this->rotation_action != RotationAction::ROTATION_NONE) {
        float angle = (this->cursor_position_now.x() -
                       this->cursor_position_start.x()) / 20.0f;
        this->scene->transposition = project_rotation_matrix(angle);
    }
}

/**
 * @brief Project a movement vector according to the active movement mode.
 *
 * @param vin Input vector
 * @return Projected vector
 */
QVector3D UserAction::project_movement_vector(const QVector3D& vin) const {
    switch(this->movement_action) {
        case MovementAction::MOVEMENT_X: return QVector3D(vin.x(), 0, 0);
        case MovementAction::MOVEMENT_Y: return QVector3D(0, vin.y(), 0);
        case MovementAction::MOVEMENT_Z: return QVector3D(0, 0, vin.z());
        case MovementAction::MOVEMENT_FOCUS:
            if(has_secondary()) {
                QVector3D proj = (this->structure->get_position_primary_buffer() -
                                  this->structure->get_position_secondary_buffer()).normalized();
                return proj * QVector3D::dotProduct(vin, proj);
            }
        default:
            return vin;
    }
}

/**
 * @brief Compute a rotation matrix for the active rotation mode.
 *
 * @param angle Rotation angle in radians
 * @return Transformation matrix
 */
QMatrix4x4 UserAction::project_rotation_matrix(float angle) const {
    QMatrix4x4 mat;
    QVector3D pivot = this->structure->get_position_primary_buffer();

    auto rotate = [&](const QVector3D& axis) {
        mat.translate(pivot);
        mat.rotate(qRadiansToDegrees(angle), axis);
        mat.translate(-pivot);
    };

    switch(this->rotation_action) {
        case RotationAction::ROTATION_X: rotate({1,0,0}); break;
        case RotationAction::ROTATION_Y: rotate({0,1,0}); break;
        case RotationAction::ROTATION_Z: rotate({0,0,1}); break;
        case RotationAction::ROTATION_FOCUS:
            if(has_secondary())
                rotate((pivot - this->structure->get_position_secondary_buffer()).normalized());
            break;
        case RotationAction::ROTATION_REVOLVE_SECONDARY:
            if(has_secondary()) {
                QVector3D sec = this->structure->get_position_secondary_buffer();
                QVector3D axis = (pivot - sec).normalized();
                mat.translate(sec);
                mat.rotate(qRadiansToDegrees(angle), axis);
                mat.translate(-sec);
            }
            break;
        case RotationAction::ROTATION_REVOLVE_CAMERA:
            if(has_secondary()) {
                QVector3D sec_model = this->structure->get_position_secondary_buffer();
                QVector3D sec_world = this->scene->rotation_matrix.map(
                    sec_model + this->structure->get_center_vector());
                QVector3D cam_world = this->scene->camera_position;

                QVector3D axis_world = (sec_world - cam_world).normalized();
                QVector3D axis_model = this->scene->rotation_matrix.inverted().map(axis_world).normalized();

                mat.translate(sec_model);
                mat.rotate(qRadiansToDegrees(angle), axis_model);
                mat.translate(-sec_model);
            }
            break;
        default:
            break;
    }

    return mat;
}
