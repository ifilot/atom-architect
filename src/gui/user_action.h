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

#include <QObject>
#include <QDebug>
#include <QPoint>
#include <QCursor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QtMath>
#include <QMessageBox>
#include <QInputDialog>

#include "../data/fragment.h"
#include "../data/structure.h"
#include "../data/structure_operator.h"
#include "scene.h"

// movement actions
enum class MovementAction {
    MOVEMENT_NONE,
    MOVEMENT_FREE,
    MOVEMENT_X,
    MOVEMENT_Y,
    MOVEMENT_Z,
    MOVEMENT_FOCUS,
};

// rotation actions
enum class RotationAction {
    ROTATION_NONE,
    ROTATION_FREE,
    ROTATION_X,
    ROTATION_Y,
    ROTATION_Z,
    ROTATION_FOCUS,
};

/**
 * @brief      Stores the action of a user on a structure
 */
class UserAction : public QObject {
    Q_OBJECT

private:
    QPointF cursor_position_start;
    QPointF cursor_position_now;
    MovementAction movement_action = MovementAction::MOVEMENT_NONE;
    RotationAction rotation_action = RotationAction::ROTATION_NONE;
    std::shared_ptr<Structure> structure;
    QVector3D translation_vector;

    std::shared_ptr<Scene> scene;
    StructureOperator structure_operator;

    std::unique_ptr<Fragment> fragment;

public:
    /**
     * @brief      Constructs a new instance.
     */
    UserAction(const std::shared_ptr<Scene>& _scene);

    /**
     * @brief      Updates the given cursor position.
     *
     * @param[in]  cursor_position  The cursor position
     */
    void update(const QPointF& cursor_pos_logical, qreal dpr);

    /**
     * @brief      Handle a translation action
     */
    void handle_action_movement();

    /**
     * @brief      Handle a rotation action
     */
    void handle_action_rotation();

    /**
     * @brief      Handle left mouse click action
     */
    void handle_left_mouse_click();

    /**
     * @brief Deselect all atoms
     */
    void deselect();

    /**
     * @brief      Sets the structure.
     *
     * @param[in]  _structure  The structure
     */
    inline void set_structure(const std::shared_ptr<Structure> _structure) {
        this->structure = _structure;
    }

    inline auto get_movement_action() const {
        return this->movement_action;
    }

    inline auto get_rotation_action() const {
        return this->rotation_action;
    }

    /**
     * @brief      Handle key stroke
     *
     * @param[in]  key        The key
     * @param[in]  modifiers  The modifiers
     */
    void handle_key(int key, Qt::KeyboardModifiers modifiers);

    /**
     * @brief      Sets the camera alignment.
     *
     * @param[in]  direction  The direction
     */
    void set_camera_alignment(int direction);

    /**
     * @brief      Sets the camera mode.
     *
     * @param[in]  mode  The mode
     */
    void set_camera_mode(int mode);

public slots:
    /**
     * @brief      Sets the fragment.
     *
     * @param[in]  _fragment  The fragment
     */
    void set_fragment(const Fragment& _fragment);

signals:
    /**
     * @brief      Request new OpenGL update
     */
    void request_update();

    /**
     * @brief      Transmit a message of the selection
     *
     * @param[in]  text  The text
     */
    void transmit_message(const QString& text);

    /**
     * @brief      Transmit a message to the statusbar
     *
     * @param[in]  text  The text
     */
    void signal_message_statusbar(const QString& text);

    /**
     * @brief send atomic selection message
     * @param text to send
     */
    void signal_selection_message(const QString& text);

    /**
     * @brief Provides a signal that the old structure should be
     *        stored and a new structure should be pushed to the
     *        structure stack. The structure stack pointer is
     *        decremented accordingly.
     */
    void signal_push_structure();

    /**
     * @brief Increment the structure stack pointer
     */
    void signal_increment_structure_stack_pointer();

    /**
     * @brief Decrement the structure stack pointer
     */
    void signal_decrement_structure_stack_pointer();

private:
    /**
     * @brief      Sets the cursor position.
     */
    void set_cursor_position();

    /**
     * @brief      Calculate the matrix by which the atoms will be transposed
     */
    void calculate_transposition_matrix();

    /**
     * @brief      Adds a fragment.
     */
    void add_fragment();

    /**
     * @brief      Calculate a movement projection vector for aligned movement
     *
     * @param[in]  vin   Input vector
     *
     * @return     Output vector
     */
    QVector3D project_movement_vector(const QVector3D& vin) const;

    /**
     * @brief      Calculate a rotation matrix
     *
     * @param[in]  angle  The angle
     *
     * @return     The transformation matrix in 3D space.
     */
    QMatrix4x4 project_rotation_matrix(float angle) const;
};
