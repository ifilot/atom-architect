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

#include <cmath>
#include <QMatrix4x4>
#include <stdexcept>

/**
 * @brief      This class describes a camera alignment.
 */
enum class CameraAlignment {
    DEFAULT,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

/**
 * @brief      This class describes a camera mode.
 */
enum class CameraMode {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

class Scene {
public:
    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 rotation_matrix;         // stores rotation state of object
    QMatrix4x4 arcball_rotation;
    QMatrix4x4 transposition;           // stores transposition rotation / translation
    QVector3D camera_position;

    int canvas_width;
    int canvas_height;

    CameraMode camera_mode = CameraMode::PERSPECTIVE;

    Scene();

    /**
     * @brief       calculate a ray originating based on mouse position and current view
     *
     * @param       mouse position
     * @param       pointer to vector holding ray origin
     * @param       pointer to vector holding ray direction
     * @return      void
     */
    void calculate_ray(const QPoint& mouse_position, QVector3D* ray_origin, QVector3D* ray_direction);

    /**
     * @brief      Calculates the point of intersection of a ray with a plane
     *
     * @param[in]  ray_origin    The ray origin
     * @param[in]  ray_vector    The ray vector
     * @param[in]  plane_origin  The plane origin
     * @param[in]  plane_normal  The plane normal
     *
     * @return     The ray plane intersection.
     */
    QVector3D calculate_ray_plane_intersection(const QVector3D& ray_origin,
                                               const QVector3D& ray_vector,
                                               const QVector3D& plane_origin,
                                               const QVector3D& plane_normal);
};
