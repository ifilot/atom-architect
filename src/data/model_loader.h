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

#include <QRegularExpression>

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

#include "model.h"

class ModelLoader {

private:
    double load_time;

public:
    /**
     * @brief      Constructs a new instance.
     */
    ModelLoader();

    /**
     * @brief      Loads a model.
     *
     * @param[in]  path  The path
     *
     * @return     Shared pointer to model object
     */
    std::shared_ptr<Model> load_model(const std::string& path);

    /**
     * @brief      Get the load time for the object.
     */
    inline double get_load_time() const {
        return this->load_time;
    }

private:
    /**
     * @brief      Load object data from obj file
     *
     * @param[in]  path   Path to file
     */
    std::shared_ptr<Model> load_data_obj(const std::string& path);

    /**
     * @brief      Load object data from ply file
     *
     * @param[in]  path   Path to file
     */
    std::shared_ptr<Model> load_data_ply(const std::string& path);

    /**
     * @brief      Loads a ply file from hard drive stored as little endian binary
     *
     * @param[in]  path   Path to file
     */
    std::shared_ptr<Model> load_data_ply_binary(const std::string& path);

    /**
     * @brief      Loads a ply file from hard drive stored in ascii format
     *
     * @param[in]  path   The path
     */
    std::shared_ptr<Model> load_data_ply_ascii(const std::string& path);
};
