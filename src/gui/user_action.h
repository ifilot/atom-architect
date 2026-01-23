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
    std::shared_ptr<Scene> scene;

    StructureOperator structure_operator;
    std::unique_ptr<Fragment> fragment;

    /* ============================================================
     *  Internal helpers
     * ============================================================ */
    bool idle_only() const;
    bool has_primary() const;
    bool has_secondary() const;

    void cmd_set_move(MovementAction mode, const QString& msg);
    void cmd_set_rotate(RotationAction mode, const QString& msg);

public:
    /**
     * @brief      Constructs a new instance.
     */
    explicit UserAction(const std::shared_ptr<Scene>& _scene);

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
     * @brief      Sets the structure.
     *
     * @param[in]  _structure  The structure
     */
    inline void set_structure(const std::shared_ptr<Structure>& _structure) {
        this->structure = _structure;
    }

    inline MovementAction get_movement_action() const {
        return this->movement_action;
    }

    inline RotationAction get_rotation_action() const {
        return this->rotation_action;
    }

    /**
     * @brief      Handle key stroke (viewport input only)
     */
    bool handle_key(QKeyEvent* event);

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

    /* ============================================================
     *  Semantic command hooks (UI + keyboard)
     * ============================================================ */

    // Transform modes (toggle)
    void cmd_toggle_move();
    void cmd_toggle_rotate();

    // Selection / editing
    void cmd_select_all();
    void cmd_deselect_all();
    void cmd_invert_selection();
    void cmd_add_fragment();
    void cmd_delete_selection();

    // Freeze
    void cmd_set_frozen();
    void cmd_set_unfrozen();

signals:
    /**
     * @brief      Request new OpenGL update
     */
    void request_update();

    /**
     * @brief      Transmit a message of the selection
     */
    void transmit_message(const QString& text);

    /**
     * @brief      Transmit a message to the statusbar
     */
    void signal_message_statusbar(const QString& text);

    /**
     * @brief      Send atomic selection message
     */
    void signal_selection_message(const QString& text);

    /**
     * @brief      Push structure onto undo stack
     */
    void signal_push_structure();

    /**
     * @brief      Increment the structure stack pointer
     */
    void signal_increment_structure_stack_pointer();

    /**
     * @brief      Decrement the structure stack pointer
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
     */
    QVector3D project_movement_vector(const QVector3D& vin) const;

    /**
     * @brief      Calculate a rotation matrix
     */
    QMatrix4x4 project_rotation_matrix(float angle) const;
};
