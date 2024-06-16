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

#include "periodic_table.h"

DialogPeriodicTable::DialogPeriodicTable(QWidget *parent) : QDialog(parent) {
    // set layout
    this->layout = new QGridLayout();
    this->setLayout(this->layout);

    // set title
    this->setWindowTitle("Select atom from Periodic Table");

    // add buttons
    this->add_buttons();
}

void DialogPeriodicTable::add_buttons() {
    //  ROW 1
    /////////

    // hydrogen
    auto name = AtomSettings::get().get_name_from_elnr(1);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 1, 1);

    // helium
    name = AtomSettings::get().get_name_from_elnr(2);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 1, 18);

    // ROW 2
    /////////

    // lithium
    name = AtomSettings::get().get_name_from_elnr(3);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 2, 1);

    // berylium
    name = AtomSettings::get().get_name_from_elnr(4);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 2, 2);

    // boron - neon
    for(unsigned int i=5; i<=10; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 2, i+(13-5));
    }

    // ROW 3
    /////////

    // sodium
    name = AtomSettings::get().get_name_from_elnr(11);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 3, 1);

    // magnesium
    name = AtomSettings::get().get_name_from_elnr(12);
    this->buttons.emplace(name, new QPushButton(name.c_str()));
    this->layout->addWidget(this->buttons.find(name)->second, 3, 2);

    // aluminium - argon
    for(unsigned int i=13; i<=18; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 3, i);
    }

    // ROW 4
    /////////

    // potassium - krypton
    for(unsigned int i=19; i<=36; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 4, i-18);
    }

    // ROW 5
    /////////

    // rubidium - xenon
    for(unsigned int i=37; i<=54; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 5, i-36);
    }

    // ROW 6
    /////////

    // cesium - lathanum
    for(unsigned int i=55; i<=57; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 6, i-54);
    }

    // hafnium - radon
    for(unsigned int i=72; i<=86; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 6, i-68);
    }

    // ROW 7
    /////////

    // francium - actinum
    for(unsigned int i=87; i<=89; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 7, i-86);
    }

    // rutherfordium - organon
    for(unsigned int i=104; i<=118; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 7, i-100);
    }

    // Lanthanides
    //////////////
    for(unsigned int i=58; i<=71; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 8, i-54);
    }

    // Actinides
    //////////////
    for(unsigned int i=90; i<=103; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        this->buttons.emplace(name, new QPushButton(name.c_str()));
        this->layout->addWidget(this->buttons.find(name)->second, 9, i-86);
    }

    for(unsigned int i=1; i<=118; i++) {
        name = AtomSettings::get().get_name_from_elnr(i);
        connect(this->buttons.find(name)->second, SIGNAL(clicked()), this, SLOT(return_element()));
    }
}

void DialogPeriodicTable::return_element() {
    QPushButton* push_button = qobject_cast<QPushButton*>(sender());
    if (push_button) {
        QString button_text = push_button->text();
        int elid = AtomSettings::get().get_atom_elnr(button_text.toStdString());
        this->done(elid);
        return;
    }

    this->done(-1);
}
