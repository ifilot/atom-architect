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

#include "scene.h"

Scene::Scene() {
}

/**
 * @brief       calculate a ray originating based on mouse position and current view
 *
 * @param       mouse position
 * @param       pointer to vector holding ray origin
 * @param       pointer to vector holding ray direction
 * @return      void
 */
void Scene::calculate_ray(const QPoint& mouse_position, QVector3D* ray_origin, QVector3D* ray_direction) {
    const float screen_width = (float)this->canvas_width;
    const float screen_height = (float)this->canvas_height;

    const QVector3D ray_nds = QVector3D((2.0f * (float)mouse_position.x()) / screen_width - 1.0f,
                                         1.0f - (2.0f * (float)mouse_position.y()) / screen_height,
                                         1.0);

    if(this->camera_mode == CameraMode::ORTHOGRAPHIC) {
        const QVector4D ray_clip(ray_nds[0], ray_nds[1], 0.0, 1.0);

        // the position on the 'camera screen' determines the origin of the
        // ray vector in orthographic projection
        QVector4D ray_eye = this->projection.inverted() * ray_clip;
        ray_eye = QVector4D(ray_eye[0], ray_eye[1], 0.0, 0.0);
        *ray_origin = this->camera_position + (this->view.inverted() * ray_eye).toVector3D();

        // if the projection is orthographic, the ray vector is the same
        // as the view direction of the camera (in world space)
        *ray_direction = -this->camera_position.normalized();
    } else if(this->camera_mode == CameraMode::PERSPECTIVE) {
        const QVector4D ray_clip(ray_nds[0], ray_nds[1], -1.0, 1.0);

        QVector4D ray_eye = this->projection.inverted() * ray_clip;
        ray_eye = QVector4D(ray_eye[0], ray_eye[1], -1.0, 0.0);
        *ray_direction = (this->view.inverted() * ray_eye).toVector3D().normalized();

        // the origin of the ray in perspective projection is simply the position
        // of the camera in world space
        *ray_origin = this->camera_position;
    } else {
        throw std::logic_error("Invalid camera mode");
    }
}

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
QVector3D Scene::calculate_ray_plane_intersection(const QVector3D& ray_origin,
                                                  const QVector3D& ray_vector,
                                                  const QVector3D& plane_origin,
                                                  const QVector3D& plane_normal) {

    float dotprod = QVector3D::dotProduct(ray_vector, plane_normal);

    if(std::fabs(dotprod) < 0.001) {
        return QVector3D(-1, -1, -1);
    } else {
        float t = QVector3D::dotProduct(plane_origin - ray_origin, plane_normal) / dotprod;
        return ray_origin + t * ray_vector;
    }
}
