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

#include "user_action.h"

/**
 * @brief      Constructs a new instance.
 */
UserAction::UserAction(const std::shared_ptr<Scene>& _scene):
    scene(_scene) {
}

/**
 * @brief      Updates the given cursor position.
 *
 * @param[in]  cursor_position  The cursor position
 */
void UserAction::update(QPoint _cursor_position) {
    this->cursor_position_now = _cursor_position;
    // qDebug() << this->cursor_position_now;

    if(this->movement_action != MovementAction::MOVEMENT_NONE ||
       this->rotation_action != RotationAction::ROTATION_NONE) {
        this->calculate_transposition_matrix();
        emit request_update();
    }
}

/**
 * @brief      Handle a translation action
 */
void UserAction::handle_action_movement() {
    if(this->structure->get_nr_atoms_primary_buffer() == 0) {
        return; // do nothing
    }

    if(this->movement_action == MovementAction::MOVEMENT_NONE) {
        this->movement_action = MovementAction::MOVEMENT_FREE;
        this->cursor_position_start = this->cursor_position_now;
        emit transmit_message("<b>Move atoms</b> | Free movement");
        emit request_update();
    } else {
        this->movement_action = MovementAction::MOVEMENT_NONE;
        emit(signal_push_structure());
        this->structure->commit_transposition(this->scene->transposition);
        this->scene->transposition.setToIdentity();
        emit transmit_message("");
        emit request_update();
        emit signal_update_structure_info();
    }
}

/**
 * @brief      Handle a rotation action
 */
void UserAction::handle_action_rotation() {
    if(this->structure->get_nr_atoms_primary_buffer() == 0) {
        return; // do nothing
    }

    if(this->rotation_action == RotationAction::ROTATION_NONE) {
        this->rotation_action = RotationAction::ROTATION_FREE;
        this->cursor_position_start = this->cursor_position_now;
        emit transmit_message("<b>Rotate atoms</b> | Free rotation");
        emit request_update();
    } else {
        this->rotation_action = RotationAction::ROTATION_NONE;
        emit(signal_push_structure());
        this->structure->commit_transposition(this->scene->transposition);
        this->scene->transposition.setToIdentity();
        emit transmit_message("");
        emit request_update();
        emit signal_update_structure_info();
    }
}

/**
 * @brief      Handle left mouse click action
 */
void UserAction::handle_left_mouse_click() {
    if(this->movement_action != MovementAction::MOVEMENT_NONE) {
        this->handle_action_movement();
    }

    if(this->rotation_action != RotationAction::ROTATION_NONE) {
        this->handle_action_rotation();
    }
}

/**
 * @brief      Handle key stroke
 *
 * @param[in]  key        The key
 * @param[in]  modifiers  The modifiers
 */
void UserAction::handle_key(int key, Qt::KeyboardModifiers modifiers) {
    if(key == Qt::Key_G && this->rotation_action == RotationAction::ROTATION_NONE) {
        this->handle_action_movement();
        return;
    }

    if(key == Qt::Key_R && this->movement_action == MovementAction::MOVEMENT_NONE) {
        this->handle_action_rotation();
        return;
    }

    // align movement
    if(this->movement_action != MovementAction::MOVEMENT_NONE) {
        switch(key) {
            case Qt::Key_X:
                this->movement_action = MovementAction::MOVEMENT_X;
                emit transmit_message("<b>Move atoms</b> | X-alignment");
            break;
            case Qt::Key_Y:
                this->movement_action = MovementAction::MOVEMENT_Y;
                emit transmit_message("<b>Move atoms</b> | Y-alignment");
            break;
            case Qt::Key_Z:
                this->movement_action = MovementAction::MOVEMENT_Z;
                emit transmit_message("<b>Move atoms</b> | Z-alignment");
            break;
            case Qt::Key_F:
                if(this->structure->get_nr_atoms_secondary_buffer() == 0) {
                    return;
                }
                this->movement_action = MovementAction::MOVEMENT_FOCUS;
                emit transmit_message("<b>Move atoms<b> | Focus alignment");
            break;
        }

        emit request_update();
        return;
    }

    // set align rotation
    if(this->rotation_action != RotationAction::ROTATION_NONE) {
        switch(key) {
            case Qt::Key_X:
                this->rotation_action = RotationAction::ROTATION_X;
                emit transmit_message("<b>Rotate atoms</b> | X-alignment");
            break;
            case Qt::Key_Y:
                this->rotation_action = RotationAction::ROTATION_Y;
                emit transmit_message("<b>Rotate atoms</b> | Y-alignment");
            break;
            case Qt::Key_Z:
                this->rotation_action = RotationAction::ROTATION_Z;
                emit transmit_message("<b>Rotate atoms</b> | Z-alignment");
            break;
            case Qt::Key_F:
                if(this->structure->get_nr_atoms_secondary_buffer() == 0) {
                    return;
                }
                this->rotation_action = RotationAction::ROTATION_FOCUS;
            break;
        }

        emit request_update();
        return;
    }

    // the following operations are only valid when the atoms are neither in
    // movement nor in rotation mode
    if(this->movement_action == MovementAction::MOVEMENT_NONE &&
       this->rotation_action == RotationAction::ROTATION_NONE) {

        // CTRL + KEY operations
        if(modifiers == Qt::ControlModifier) {
            switch(key) {
                case Qt::Key_D:
                    this->structure->clear_selection();
                    emit signal_selection_message(this->structure->get_selection_string());
                    emit request_update();
                break;
                case Qt::Key_A:
                    this->structure->select_all_atoms();
                    emit signal_selection_message(this->structure->get_selection_string());
                    emit request_update();
                break;
                case Qt::Key_Z: // undo action
                    emit signal_decrement_structure_stack_pointer();
                    emit request_update();
                    emit signal_update_structure_info();
                break;
                case Qt::Key_Y: // redo action
                    emit signal_increment_structure_stack_pointer();
                    emit request_update();
                    emit signal_update_structure_info();
                break;
                case Qt::Key_I: // invert selection
                    this->structure->invert_selection();
                    emit signal_selection_message(this->structure->get_selection_string());
                    emit request_update();
                break;
                case Qt::Key_F: // set frozen
                    qDebug() << "Freezing atoms";
                    emit(signal_push_structure());
                    this->structure->set_frozen();
                    emit request_update();
                    emit signal_update_structure_info();
                break;
            }
        }

        // CTRL + SHIFT + KEY operations
        if((modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier)) {
            switch(key) {
                case Qt::Key_F: // unset frozen
                    qDebug() << "Unfreezing atoms";
                    emit(signal_push_structure());
                    this->structure->set_unfrozen();
                    emit request_update();
                    emit signal_update_structure_info();
                break;
            }
        }

        // add atom
        if(key == Qt::Key_A && modifiers == Qt::ShiftModifier) {
            this->add_fragment();
            emit request_update();
            emit signal_update_structure_info();
            return;
        }

        // delete selected atoms in primary buffer
        if(key == Qt::Key_Delete && this->structure->get_nr_atoms_primary_buffer() != 0) {
            emit(signal_push_structure());
            this->structure->delete_atoms();
            this->structure->clear_selection();
            emit request_update();
            emit signal_update_structure_info();
            return;
        }
    }

}

/**
 * @brief      Sets the camera alignment.
 *
 * @param[in]  direction  The direction
 */
void UserAction::set_camera_alignment(int direction) {
    QVector3D dirvec;

    switch((CameraAlignment)direction) {
        case CameraAlignment::DEFAULT:
            this->scene->rotation_matrix.setToIdentity();
            this->scene->rotation_matrix.rotate(20.0, QVector3D(1,0,0));
            this->scene->rotation_matrix.rotate(30.0, QVector3D(0,0,1));
            emit(request_update());
        return;
        case CameraAlignment::TOP:
            dirvec = QVector3D(0.0f, 0.0f, 1.0f);
        break;
        case CameraAlignment::BOTTOM:
            dirvec = QVector3D(0.0f, 0.0f, -1.0f);
        break;
        case CameraAlignment::LEFT:
            dirvec = QVector3D(-1.0f, 0.0f, 0.0f);
        break;
        case CameraAlignment::RIGHT:
            dirvec = QVector3D(1.0f, 0.0f, 0.0f);
        break;
        case CameraAlignment::FRONT:
            dirvec = QVector3D(0.0f, 1.0f, 0.0f);
        break;
        case CameraAlignment::BACK:
            dirvec = QVector3D(0.0f, -1.0f, 0.0f);
        break;
    }

    QVector3D axis;
    float angle;

    // avoid gimball locking
    if (fabs(dirvec[1]) > .999) {
        if(dirvec[1] < 0.0) {
            axis = QVector3D(0.0, 0.0, 1.0);
            angle = -M_PI;
        } else {
            axis = QVector3D(0.0, 0.0, 1.0);
            angle = 0.0;
        }
    } else {
        axis = QVector3D::crossProduct(QVector3D(0.0, 1.0, 0.0), dirvec);
        angle = std::acos(dirvec[1]);
    }

    this->scene->rotation_matrix.setToIdentity();
    this->scene->rotation_matrix.rotate(qRadiansToDegrees(angle), axis);
    emit(signal_message_statusbar("Change camera alignment"));
    emit(request_update());
}

/**
 * @brief      Sets the camera mode.
 *
 * @param[in]  mode  The mode
 */
void UserAction::set_camera_mode(int mode) {
    float w = (float)this->scene->canvas_width;
    float h = (float)this->scene->canvas_height;

    switch((CameraMode)mode) {
        case CameraMode::PERSPECTIVE:
            this->scene->camera_mode = CameraMode::PERSPECTIVE;
            this->scene->projection.setToIdentity();
            this->scene->projection.perspective(45.0f, w / h, 0.01f, 1000.0f);
            emit(signal_message_statusbar("Set camera to perspective"));
        break;
        case CameraMode::ORTHOGRAPHIC:
            this->scene->camera_mode = CameraMode::ORTHOGRAPHIC;
            float w = (float)this->scene->canvas_width;
            float h = (float)this->scene->canvas_height;
            float ratio = w/h;
            float zoom = -this->scene->camera_position[1];
            this->scene->projection.setToIdentity();
            this->scene->projection.ortho(-zoom/2.0f, zoom/2.0f, -zoom / ratio /2.0f, zoom / ratio / 2.0f, 0.01f, 1000.0f);
            emit(signal_message_statusbar("Set camera to orthographic"));
        break;
    }

    emit(request_update());
}

/**
 * @brief      Sets the fragment.
 *
 * @param[in]  _fragment  The fragment
 */
void UserAction::set_fragment(const Fragment& _fragment) {
    this->fragment = std::make_unique<Fragment>(_fragment);
}

/**
 * @brief      Sets the cursor position.
 */
void UserAction::set_cursor_position() {
    this->cursor_position_start = QCursor::pos();
    qDebug() << this->cursor_position_start;
}

/**
 * @brief      Adds a fragment.
 */
void UserAction::add_fragment() {
    try {
        // these two functions will throw an exception if the buffer is empty
        this->structure->get_position_primary_buffer();
        this->structure->get_position_secondary_buffer();

        // ask user for input
        bool ok;
        double distance = QInputDialog::getDouble(nullptr, tr("Set fragment distance"), tr("Distance in angstrom:"), 1.2, 0.5, 3.5, 2, &ok);
        if(!ok) {
            return;
        }

        // check if a fragment is actually set
        if(this->fragment) {
            emit(signal_push_structure());
            this->structure_operator.add_fragment(this->structure.get(), *this->fragment.get(), distance);
        } else {
            throw std::runtime_error("No fragment is set");
        }

        emit request_update();
    } catch(const std::exception& e) {
        QMessageBox::critical(nullptr, tr("Exception encountered"), tr(e.what()) );
    }
}

/**
 * @brief      Calculate the matrix by which the atoms will be transposed
 */
void UserAction::calculate_transposition_matrix() {
    if(this->movement_action != MovementAction::MOVEMENT_NONE) {
        // calculate source vector in world space
        QVector3D ray_origin, ray_direction;
        this->scene->calculate_ray(this->cursor_position_start, &ray_origin, &ray_direction);
        auto pos = this->scene->rotation_matrix.map(this->structure->get_position_primary_buffer() + this->structure->get_center_vector());
        auto source = this->scene->calculate_ray_plane_intersection(ray_origin, ray_direction, pos, -this->scene->camera_position);

        // calculate target vector in world space
        this->scene->calculate_ray(this->cursor_position_now, &ray_origin, &ray_direction);
        auto target = this->scene->calculate_ray_plane_intersection(ray_origin, ray_direction, pos, -this->scene->camera_position);

        // construct transposition matrix in model space
        this->scene->transposition.setToIdentity();
        this->scene->transposition.translate(this->scene->rotation_matrix.inverted().map(this->project_movement_vector(target - source)));
        return;
    }

    if(this->rotation_action != RotationAction::ROTATION_NONE) {
        float angle = (this->cursor_position_now.x() - this->cursor_position_start.x()) / 20.0f;
        this->scene->transposition = this->project_rotation_matrix(angle);
        return;
    }
}

/**
 * @brief      Calculate a movement projection vector for aligned movement
 *
 * @param[in]  vin Input vector
 *
 * @return     Output vector
 */
QVector3D UserAction::project_movement_vector(const QVector3D& vin) const {
    switch(this->movement_action) {
        case MovementAction::MOVEMENT_FREE:
            return vin;
        case MovementAction::MOVEMENT_X:
            return this->scene->rotation_matrix.map(QVector3D::dotProduct(vin, this->scene->rotation_matrix.map(QVector3D(1.0f, 0.0f, 0.0f))) * QVector3D(1.0f, 0.0f, 0.0f));
        case MovementAction::MOVEMENT_Y:
            return this->scene->rotation_matrix.map(QVector3D::dotProduct(vin, this->scene->rotation_matrix.map(QVector3D(0.0f, 1.0f, 0.0f))) * QVector3D(0.0f, 1.0f, 0.0f));
        case MovementAction::MOVEMENT_Z:
            return this->scene->rotation_matrix.map(QVector3D::dotProduct(vin, this->scene->rotation_matrix.map(QVector3D(0.0f, 0.0f, 1.0f))) * QVector3D(0.0f, 0.0f, 1.0f));
        case MovementAction::MOVEMENT_FOCUS:
            if(this->structure->get_position_secondary_buffer() != QVector3D(0.0f, 0.0f, 0.0f)) {
                QVector3D proj = (this->structure->get_position_primary_buffer() - this->structure->get_position_secondary_buffer()).normalized();
                auto dir = this->scene->rotation_matrix.map(proj * QVector3D::dotProduct(vin, proj));
                return dir;
            } else {
                return vin;
            }
        default:
            return vin;
    }
}

/**
 * @brief      Calculate a rotation matrix
 *
 * @param[in]  angle  The angle
 *
 * @return     The transformation matrix in 3D space.
 */
QMatrix4x4 UserAction::project_rotation_matrix(float angle) const {
    QMatrix4x4 mat;

    switch(this->rotation_action) {
        case RotationAction::ROTATION_X:
            mat.translate(this->structure->get_position_primary_buffer());
            mat.rotate(qRadiansToDegrees(angle), QVector3D(1.0f, 0.0f, 0.0f));
            mat.translate(-this->structure->get_position_primary_buffer());
        break;
        case RotationAction::ROTATION_Y:
            mat.translate(this->structure->get_position_primary_buffer());
            mat.rotate(qRadiansToDegrees(angle), QVector3D(0.0f, 1.0f, 0.0f));
            mat.translate(-this->structure->get_position_primary_buffer());
        break;
        case RotationAction::ROTATION_Z:
            mat.translate(this->structure->get_position_primary_buffer());
            mat.rotate(qRadiansToDegrees(angle), QVector3D(0.0f, 0.0f, 1.0f));
            mat.translate(-this->structure->get_position_primary_buffer());
        break;
        case RotationAction::ROTATION_FOCUS:
            if(this->structure->get_position_secondary_buffer() != QVector3D(0,0,0)) {
                QVector3D dir = (this->structure->get_position_primary_buffer() - this->structure->get_position_secondary_buffer()).normalized();
                mat.translate(this->structure->get_position_primary_buffer());
                mat.rotate(qRadiansToDegrees(angle), dir);
                mat.translate(-this->structure->get_position_primary_buffer());
            }
        break;
        default:
            // do nothing
        break;
    }

    return mat;
}
