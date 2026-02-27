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

#include "periodic_table.h"

#include <array>

#include <QFont>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

namespace {
constexpr const char* S_BLOCK_COLOR = "#cde8ff";
constexpr const char* P_BLOCK_COLOR = "#d6f5d6";
constexpr const char* D_BLOCK_COLOR = "#ffe6b3";
constexpr const char* F_BLOCK_COLOR = "#f2d9ff";

constexpr int TILE_WIDTH = 64;
constexpr int TILE_HEIGHT = 52;

const std::array<const char*, 119> ELEMENT_FULL_NAMES = {
    "",
    "Hydrogen", "Helium", "Lithium", "Beryllium", "Boron", "Carbon", "Nitrogen", "Oxygen", "Fluorine", "Neon",
    "Sodium", "Magnesium", "Aluminium", "Silicon", "Phosphorus", "Sulfur", "Chlorine", "Argon",
    "Potassium", "Calcium", "Scandium", "Titanium", "Vanadium", "Chromium", "Manganese", "Iron", "Cobalt", "Nickel", "Copper", "Zinc",
    "Gallium", "Germanium", "Arsenic", "Selenium", "Bromine", "Krypton",
    "Rubidium", "Strontium", "Yttrium", "Zirconium", "Niobium", "Molybdenum", "Technetium", "Ruthenium", "Rhodium", "Palladium", "Silver", "Cadmium",
    "Indium", "Tin", "Antimony", "Tellurium", "Iodine", "Xenon",
    "Cesium", "Barium", "Lanthanum", "Cerium", "Praseodymium", "Neodymium", "Promethium", "Samarium", "Europium", "Gadolinium", "Terbium", "Dysprosium", "Holmium", "Erbium", "Thulium", "Ytterbium", "Lutetium",
    "Hafnium", "Tantalum", "Tungsten", "Rhenium", "Osmium", "Iridium", "Platinum", "Gold", "Mercury", "Thallium", "Lead", "Bismuth", "Polonium", "Astatine", "Radon",
    "Francium", "Radium", "Actinium", "Thorium", "Protactinium", "Uranium", "Neptunium", "Plutonium", "Americium", "Curium", "Berkelium", "Californium", "Einsteinium", "Fermium", "Mendelevium", "Nobelium", "Lawrencium",
    "Rutherfordium", "Dubnium", "Seaborgium", "Bohrium", "Hassium", "Meitnerium", "Darmstadtium", "Roentgenium", "Copernicium", "Nihonium", "Flerovium", "Moscovium", "Livermorium", "Tennessine", "Oganesson"
};
}

/**
 * @brief DialogPeriodicTable.
 *
 * @param param Parameter param.
 */
DialogPeriodicTable::DialogPeriodicTable(QWidget *parent) : QDialog(parent) {
    this->layout = new QGridLayout();
    this->layout->setSpacing(2);
    this->layout->setContentsMargins(8, 8, 8, 8);
    this->setLayout(this->layout);
    this->setWindowTitle("Select atom from Periodic Table");
    this->add_buttons();
}

/**
 * @brief add_buttons.
 *
 */
void DialogPeriodicTable::add_buttons() {
    this->add_table_labels();

    this->add_element_button(1, 1, 1, S_BLOCK_COLOR);
    this->add_element_button(2, 1, 18, S_BLOCK_COLOR);

    this->add_element_button(3, 2, 1, S_BLOCK_COLOR);
    this->add_element_button(4, 2, 2, S_BLOCK_COLOR);
    for(unsigned int i=5; i<=10; i++) {
        this->add_element_button(i, 2, i+(13-5), P_BLOCK_COLOR);
    }

    this->add_element_button(11, 3, 1, S_BLOCK_COLOR);
    this->add_element_button(12, 3, 2, S_BLOCK_COLOR);
    for(unsigned int i=13; i<=18; i++) {
        this->add_element_button(i, 3, i, P_BLOCK_COLOR);
    }

    for(unsigned int i=19; i<=36; i++) {
        if(i <= 20) {
            this->add_element_button(i, 4, i-18, S_BLOCK_COLOR);
        } else if(i <= 30) {
            this->add_element_button(i, 4, i-18, D_BLOCK_COLOR);
        } else {
            this->add_element_button(i, 4, i-18, P_BLOCK_COLOR);
        }
    }

    for(unsigned int i=37; i<=54; i++) {
        if(i <= 38) {
            this->add_element_button(i, 5, i-36, S_BLOCK_COLOR);
        } else if(i <= 48) {
            this->add_element_button(i, 5, i-36, D_BLOCK_COLOR);
        } else {
            this->add_element_button(i, 5, i-36, P_BLOCK_COLOR);
        }
    }

    for(unsigned int i=55; i<=57; i++) {
        this->add_element_button(i, 6, i-54, i <= 56 ? S_BLOCK_COLOR : D_BLOCK_COLOR);
    }
    for(unsigned int i=72; i<=86; i++) {
        this->add_element_button(i, 6, i-68, i <= 80 ? D_BLOCK_COLOR : P_BLOCK_COLOR);
    }

    for(unsigned int i=87; i<=89; i++) {
        this->add_element_button(i, 7, i-86, i <= 88 ? S_BLOCK_COLOR : D_BLOCK_COLOR);
    }
    for(unsigned int i=104; i<=118; i++) {
        this->add_element_button(i, 7, i-100, i <= 112 ? D_BLOCK_COLOR : P_BLOCK_COLOR);
    }

    for(unsigned int i=58; i<=71; i++) {
        this->add_element_button(i, 8, i-54, F_BLOCK_COLOR);
    }

    for(unsigned int i=90; i<=103; i++) {
        this->add_element_button(i, 9, i-86, F_BLOCK_COLOR);
    }

    this->add_block_legend();

    for(unsigned int i=1; i<=118; i++) {
        auto name = AtomSettings::get().get_name_from_elnr(i);
        connect(this->buttons.find(name)->second, SIGNAL(clicked()), this, SLOT(return_element()));
    }
}

/**
 * @brief add_element_button.
 *
 * @param elnr Parameter elnr.
 * @param row Parameter row.
 * @param column Parameter column.
 * @param block_color Parameter block_color.
 */
void DialogPeriodicTable::add_element_button(unsigned int elnr, int row, int column, const QString& block_color) {
    const QString symbol = QString::fromStdString(AtomSettings::get().get_name_from_elnr(elnr));

    auto* button = new QPushButton();
    button->setIcon(this->create_element_tile(elnr, symbol, block_color));
    button->setIconSize(QSize(TILE_WIDTH, TILE_HEIGHT));
    button->setFixedSize(TILE_WIDTH + 4, TILE_HEIGHT + 4);
    button->setProperty("elementSymbol", symbol);
    button->setStyleSheet("QPushButton { border: 1px solid #8c8c8c; padding: 0px; } QPushButton:hover { border: 1px solid #2c7be5; }");

    this->buttons.emplace(symbol.toStdString(), button);
    this->layout->addWidget(button, row, column);
}

/**
 * @brief create_element_tile.
 *
 * @param elnr Parameter elnr.
 * @param symbol Parameter symbol.
 * @param block_color Parameter block_color.
 */
QIcon DialogPeriodicTable::create_element_tile(unsigned int elnr, const QString& symbol, const QString& block_color) const {
    QPixmap pixmap(TILE_WIDTH, TILE_HEIGHT);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QColor("#666666"));
    painter.setBrush(QColor(block_color));
    painter.drawRoundedRect(0, 0, TILE_WIDTH - 1, TILE_HEIGHT - 1, 3, 3);

    QFont number_font = painter.font();
    number_font.setPointSize(7);
    painter.setFont(number_font);
    painter.setPen(QColor("#333333"));
    painter.drawText(QRect(4, 2, TILE_WIDTH - 8, 12), Qt::AlignLeft | Qt::AlignTop, QString::number(elnr));

    QFont symbol_font = painter.font();
    symbol_font.setBold(true);
    symbol_font.setPointSize(16);
    painter.setFont(symbol_font);
    painter.setPen(QColor("#1f1f1f"));
    painter.drawText(QRect(2, 11, TILE_WIDTH - 4, 22), Qt::AlignCenter, symbol);

    QFont name_font = painter.font();
    name_font.setBold(false);
    name_font.setPointSize(7);
    painter.setFont(name_font);
    painter.setPen(QColor("#2c2c2c"));
    painter.drawText(QRect(3, 33, TILE_WIDTH - 6, 16), Qt::AlignHCenter | Qt::AlignTop, this->get_full_name(elnr));

    return QIcon(pixmap);
}

/**
 * @brief get_full_name.
 *
 * @param elnr Parameter elnr.
 */
QString DialogPeriodicTable::get_full_name(unsigned int elnr) const {
    if(elnr < ELEMENT_FULL_NAMES.size()) {
        return QString(ELEMENT_FULL_NAMES[elnr]);
    }

    return QString();
}

/**
 * @brief add_table_labels.
 *
 */
void DialogPeriodicTable::add_table_labels() {
    for(int group = 1; group <= 18; group++) {
        auto* group_label = new QLabel(QString::number(group));
        group_label->setAlignment(Qt::AlignCenter);
        this->layout->addWidget(group_label, 0, group);
    }

    for(int period = 1; period <= 7; period++) {
        auto* period_label = new QLabel(QString::number(period));
        period_label->setAlignment(Qt::AlignCenter);
        this->layout->addWidget(period_label, period, 0);
    }

    auto* lanthanide_label = new QLabel("6*");
    lanthanide_label->setAlignment(Qt::AlignCenter);
    this->layout->addWidget(lanthanide_label, 8, 0);

    auto* actinide_label = new QLabel("7*");
    actinide_label->setAlignment(Qt::AlignCenter);
    this->layout->addWidget(actinide_label, 9, 0);
}

/**
 * @brief add_block_legend.
 *
 */
void DialogPeriodicTable::add_block_legend() {
    const std::array<std::pair<QString, QString>, 4> block_labels = {{
        {"s-block", S_BLOCK_COLOR},
        {"p-block", P_BLOCK_COLOR},
        {"d-block", D_BLOCK_COLOR},
        {"f-block", F_BLOCK_COLOR}
    }};

    int column = 1;
    for(const auto& [label, color] : block_labels) {
        auto* block_label = new QLabel(label);
        block_label->setAlignment(Qt::AlignCenter);
        block_label->setStyleSheet(QString("QLabel { background-color: %1; padding: 4px; border: 1px solid #bbbbbb; }").arg(color));
        this->layout->addWidget(block_label, 10, column, 1, 4);
        column += 4;
    }
}

/**
 * @brief return_element.
 *
 */
void DialogPeriodicTable::return_element() {
    QPushButton* push_button = qobject_cast<QPushButton*>(sender());
    if (push_button) {
        const QString symbol = push_button->property("elementSymbol").toString();
        int elid = AtomSettings::get().get_atom_elnr(symbol.toStdString());
        this->done(elid);
        return;
    }

    this->done(-1);
}
