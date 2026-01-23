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
 ****************************************************************************/

#pragma once

#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QScrollArea>

#include <unordered_map>

#include "structure_info_tab.h"

class StructureInfoBasicTab : public StructureInfoTab {
private:
    QVBoxLayout* layout;

    std::unordered_map<std::string, QLabel*> labelmap;

    // Preformatted atomic coordinates block
    QLabel* atomic_coordinates_label;

public:
    explicit StructureInfoBasicTab(QWidget* parent = nullptr);

private:
    /**
     * @brief      Get pointer to label from key
     *
     * @param[in]  key  The key
     *
     * @return     The label.
     */
    QLabel* get_label(const std::string& key);

public slots:
    /**
     * @brief      Update data in tab based on current structure
     */
    void update_data();

    /**
     * @brief      Resets the object.
     */
    void reset();
};
