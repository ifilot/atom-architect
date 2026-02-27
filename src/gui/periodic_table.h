/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2026 Ivo Filot <i.a.w.filot@tue.nl>                 *
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
#include <QDialog>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

#include <unordered_map>

#include "../data/atom_settings.h"

/**
 * @brief DialogPeriodicTable class.
 */
class DialogPeriodicTable : public QDialog {
    Q_OBJECT

private:
    QGridLayout* layout;
    std::unordered_map<std::string, QPushButton*> buttons;

private:
/**
 * @brief add_element_button.
 *
 * @param elnr Parameter elnr.
 * @param row Parameter row.
 * @param column Parameter column.
 * @param block_color Parameter block_color.
 */
    void add_element_button(unsigned int elnr, int row, int column, const QString& block_color);
/**
 * @brief create_element_tile.
 *
 * @param elnr Parameter elnr.
 * @param symbol Parameter symbol.
 * @param block_color Parameter block_color.
 */
    QIcon create_element_tile(unsigned int elnr, const QString& symbol, const QString& block_color) const;
/**
 * @brief get_full_name.
 *
 * @param elnr Parameter elnr.
 */
    QString get_full_name(unsigned int elnr) const;
/**
 * @brief add_table_labels.
 *
 */
    void add_table_labels();
/**
 * @brief add_block_legend.
 *
 */
    void add_block_legend();

public:
/**
 * @brief DialogPeriodicTable.
 *
 * @param parent Parameter parent.
 */
    DialogPeriodicTable(QWidget *parent = nullptr);

private:
/**
 * @brief add_buttons.
 *
 */
    void add_buttons();

private slots:
/**
 * @brief return_element.
 *
 */
    void return_element();
};
