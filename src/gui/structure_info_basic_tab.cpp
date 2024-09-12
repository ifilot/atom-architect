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

#include "structure_info_basic_tab.h"

StructureInfoBasicTab::StructureInfoBasicTab(QWidget* parent) : StructureInfoTab(parent) {
    // set gridlayout as default layout
    this->layout = new QVBoxLayout();
    this->scrollarea->setLayout(this->layout);

    // build labels
    this->labelmap.emplace("number_of_atoms", new QLabel());
    this->labelmap.emplace("type_of_elements", new QLabel());
    this->labelmap.emplace("unitcell_dimensions", new QLabel());
    this->labelmap.emplace("filler", new QLabel());

    this->layout->addWidget(new QLabel("<b>Number of atoms:</b>"));
    this->layout->addWidget(this->get_label("number_of_atoms"));
    this->layout->addWidget(new QLabel("<b>Types of elements</b>"));
    this->layout->addWidget(this->get_label("type_of_elements"));
    this->layout->addWidget(new QLabel("<b>Unitcell dimensions</b>"));
    this->layout->addWidget(this->get_label("unitcell_dimensions"));

    // atoms
    this->layout->addWidget(new QLabel("<b>Atomic coordinates (Cartesian)</b>"));

    // table with atomic coordinates
    this->table_atomic_data = new QTableWidget(this);
    this->table_atomic_data->setColumnCount(7);
    this->table_atomic_data->setHorizontalHeaderLabels({tr("atom"), tr("x"), tr("y"), tr("z"), tr("sx"), tr("sy"), tr("sz")});
    this->table_atomic_data->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->table_atomic_data->setShowGrid(false);
    this->table_atomic_data->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table_atomic_data->setColumnWidth(0, 50);
    this->table_atomic_data->setCornerButtonEnabled(false);
    this->layout->addWidget(this->table_atomic_data);
}

/**
 * @brief      Update data in tab based on current structure
 */
void StructureInfoBasicTab::update_data() {
    qDebug() << "Updating structure info: " << (size_t)this->structure.get();
    this->get_label("number_of_atoms")->setText(QString::number(this->structure->get_nr_atoms()));
    this->get_label("type_of_elements")->setText(this->structure->get_elements_string().c_str());

    char buf[64];

    MatrixUnitcell unitcell = this->structure->get_unitcell();
    QString unitcell_str = "<pre>";
    for(unsigned int i=0; i<3; i++) {
        sprintf(buf, "%12.6f  %12.6f  %12.6f\n", unitcell(i,0), unitcell(i,1), unitcell(i,2));
        unitcell_str += QString(buf);
    }
    unitcell_str += "</pre>";
    this->get_label("unitcell_dimensions")->setText(unitcell_str);

    this->update_table();
}

/**
 * @brief      Get pointer to label from key
 *
 * @param[in]  _key  The key
 *
 * @return     The label.
 */
QLabel* StructureInfoBasicTab::get_label(const std::string& key) {
    auto got = this->labelmap.find(key);
    if(got != this->labelmap.end()) {
        return got->second;
    } else {
        throw std::logic_error("Invalid key requested from labelmap in StructureInfoBasicTab");
    }
}

void StructureInfoBasicTab::reset() {

}

void StructureInfoBasicTab::update_table() {
    this->table_atomic_data->setRowCount(this->structure->get_nr_atoms());

    for(unsigned int i=0; i<this->structure->get_nr_atoms(); i++) {
        const auto& atom = this->structure->get_atom(i);
        this->table_atomic_data->setItem(i, 0, new QTableWidgetItem(AtomSettings::get().get_name_from_elnr(atom.atnr).c_str()));

        this->table_atomic_data->setItem(i, 1, new QTableWidgetItem(QString::number(atom.x, 'f', 6)));
        this->table_atomic_data->item(i,1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        this->table_atomic_data->setItem(i, 2, new QTableWidgetItem(QString::number(atom.y, 'f', 6)));
        this->table_atomic_data->item(i,2)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        this->table_atomic_data->setItem(i, 3, new QTableWidgetItem(QString::number(atom.z, 'f', 6)));
        this->table_atomic_data->item(i,3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        this->table_atomic_data->setCellWidget(i, 4, new QCheckBox());
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,4))->setCheckState(atom.selective_dynamics[0] ? Qt::Checked : Qt::Unchecked);
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,4))->setEnabled(false);

        this->table_atomic_data->setCellWidget(i, 5, new QCheckBox());
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,5))->setCheckState(atom.selective_dynamics[1] ? Qt::Checked : Qt::Unchecked);
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,5))->setEnabled(false);

        this->table_atomic_data->setCellWidget(i, 6, new QCheckBox());
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,6))->setCheckState(atom.selective_dynamics[2] ? Qt::Checked : Qt::Unchecked);
        reinterpret_cast<QCheckBox*>(this->table_atomic_data->cellWidget(i,6))->setEnabled(false);
    }

    this->table_atomic_data->resizeColumnsToContents();
}
