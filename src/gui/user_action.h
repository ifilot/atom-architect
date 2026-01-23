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
#include <QKeyEvent>

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
    ROTATION_REVOLVE_SECONDARY,
    ROTATION_REVOLVE_CAMERA
};

/**
 * @brief Stores the action of a user on a structure.
 *
 * This class translates keyboard and mouse input into semantic
 * structure-editing operations (move, rotate, select, undo, etc.)
 * while maintaining undo/redo correctness.
 */
class UserAction : public QObject {
    Q_OBJECT

private:
    QPointF cursor_position_start;
    QPointF cursor_position_now;

    MovementAction movement_action = MovementAction::MOVEMENT_NONE;
    RotationAction rotation_action = RotationAction::ROTATION_NONE;

    std::shared_ptr<Structure> structure;
    std::shared_ptr<Scene> scene;

    StructureOperator structure_operator;
    std::unique_ptr<Fragment> fragment;

    /* ============================================================
     *  Internal helpers
     * ============================================================ */

    /**
     * @brief Check whether no movement or rotation is active.
     *
     * @return true if idle, false otherwise
     */
    bool idle_only() const;

    /**
     * @brief Check whether a primary atom buffer exists.
     *
     * @return true if primary buffer contains atoms
     */
    bool has_primary() const;

    /**
     * @brief Check whether a secondary atom buffer exists.
     *
     * @return true if secondary buffer contains atoms
     */
    bool has_secondary() const;

    /**
     * @brief Set movement alignment mode while moving.
     *
     * @param mode Movement alignment mode
     * @param msg Status message
     */
    void cmd_set_move(MovementAction mode, const QString& msg);

    /**
     * @brief Set rotation alignment mode while rotating.
     *
     * @param mode Rotation alignment mode
     * @param msg Status message
     */
    void cmd_set_rotate(RotationAction mode, const QString& msg);

public:
    /**
     * @brief Construct a UserAction controller.
     *
     * @param _scene Shared scene object used for camera and transforms
     */
    explicit UserAction(const std::shared_ptr<Scene>& _scene);

    /**
     * @brief Update cursor position and recompute transform if active.
     *
     * @param cursor_pos_logical Cursor position in logical pixels
     * @param dpr Device pixel ratio
     */
    void update(const QPointF& cursor_pos_logical, qreal dpr);

    /**
     * @brief Toggle movement mode or commit an active movement.
     */
    void handle_action_movement();

    /**
     * @brief Toggle rotation mode or commit an active rotation.
     */
    void handle_action_rotation();

    /**
     * @brief Finalize movement or rotation on mouse click.
     */
    void handle_left_mouse_click();

    /**
     * @brief Set the active structure.
     *
     * @param _structure Shared structure pointer
     */
    inline void set_structure(const std::shared_ptr<Structure>& _structure) {
        this->structure = _structure;
    }

    /**
     * @brief Get current movement action.
     */
    inline MovementAction get_movement_action() const {
        return this->movement_action;
    }

    /**
     * @brief Get current rotation action.
     */
    inline RotationAction get_rotation_action() const {
        return this->rotation_action;
    }

    /**
     * @brief Handle keyboard input (viewport input only).
     *
     * @param event Qt key event
     * @return true if handled
     */
    bool handle_key(QKeyEvent* event);

    /**
     * @brief Align the camera to a predefined direction.
     *
     * @param direction CameraAlignment enum value
     */
    void set_camera_alignment(int direction);

    /**
     * @brief Set the camera projection mode.
     *
     * @param mode CameraMode enum value
     */
    void set_camera_mode(int mode);

public slots:
    /**
     * @brief Set the current fragment used for insertion.
     *
     * @param _fragment Fragment to insert
     */
    void set_fragment(const Fragment& _fragment);

    /* ============================================================
     *  Semantic command hooks (UI + keyboard)
     * ============================================================ */

    /**
     * @brief Toggle movement mode.
     */
    void cmd_toggle_move();

    /**
     * @brief Toggle rotation mode.
     */
    void cmd_toggle_rotate();

    /**
     * @brief Select all atoms.
     */
    void cmd_select_all();

    /**
     * @brief Clear atom selection.
     */
    void cmd_deselect_all();

    /**
     * @brief Invert atom selection.
     */
    void cmd_invert_selection();

    /**
     * @brief Insert a fragment.
     */
    void cmd_add_fragment();

    /**
     * @brief Delete selected atoms.
     */
    void cmd_delete_selection();

    /**
     * @brief Freeze selected atoms.
     */
    void cmd_set_frozen();

    /**
     * @brief Unfreeze selected atoms.
     */
    void cmd_set_unfrozen();

signals:
    /**
     * @brief Request new OpenGL update.
     */
    void request_update();

    /**
     * @brief Transmit interaction/status message.
     */
    void transmit_message(const QString& text);

    /**
     * @brief Transmit a message to the statusbar.
     */
    void signal_message_statusbar(const QString& text);

    /**
     * @brief Send atomic selection message.
     */
    void signal_selection_message(const QString& text);

    /**
     * @brief Push structure onto undo stack.
     */
    void signal_push_structure();

    /**
     * @brief Increment undo stack pointer.
     */
    void signal_increment_structure_stack_pointer();

    /**
     * @brief Decrement undo stack pointer.
     */
    void signal_decrement_structure_stack_pointer();

private:
    /**
     * @brief Store current cursor position as start reference.
     */
    void set_cursor_position();

    /**
     * @brief Compute the current transposition matrix.
     */
    void calculate_transposition_matrix();

    /**
     * @brief Insert a fragment at the current selection.
     */
    void add_fragment();

    /**
     * @brief Project a movement vector according to alignment mode.
     *
     * @param vin Input vector
     * @return Projected vector
     */
    QVector3D project_movement_vector(const QVector3D& vin) const;

    /**
     * @brief Compute a rotation matrix for the active rotation mode.
     *
     * @param angle Rotation angle in radians
     * @return Transformation matrix
     */
    QMatrix4x4 project_rotation_matrix(float angle) const;
};
