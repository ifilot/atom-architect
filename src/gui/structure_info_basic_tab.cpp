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

#include "structure_info_basic_tab.h"

/**
 * @brief      Constructs a new instance.
 *
 * @param      parent  The parent
 */
StructureInfoBasicTab::StructureInfoBasicTab(QWidget* parent)
    : StructureInfoTab(parent)
{
    // vertical layout inside scroll area
    this->layout = new QVBoxLayout();
    this->scrollarea->setLayout(this->layout);

    // labels
    this->labelmap.emplace("number_of_atoms", new QLabel());
    this->labelmap.emplace("type_of_elements", new QLabel());
    this->labelmap.emplace("unitcell_dimensions", new QLabel());

    // basic info
    this->layout->addWidget(new QLabel("<b>Number of atoms:</b>"));
    this->layout->addWidget(this->get_label("number_of_atoms"));

    this->layout->addWidget(new QLabel("<b>Types of elements:</b>"));
    this->layout->addWidget(this->get_label("type_of_elements"));

    this->layout->addWidget(new QLabel("<b>Unit cell dimensions:</b>"));
    this->layout->addWidget(this->get_label("unitcell_dimensions"));

    this->layout->addStretch(1);
}

/**
 * @brief update_data.
 *
 */
void StructureInfoBasicTab::update_data()
{
    if (!this->structure) {
        return;
    }

    this->get_label("number_of_atoms")
        ->setText(QString::number(this->structure->get_nr_atoms()));

    this->get_label("type_of_elements")
        ->setText(this->structure->get_elements_string().c_str());

    // --- Unit cell ---
    char buf[64];
    MatrixUnitcell unitcell = this->structure->get_unitcell();

    QString unitcell_str = "<pre>";
    for (unsigned int i = 0; i < 3; i++) {
        sprintf(buf, "%12.6f  %12.6f  %12.6f\n",
                unitcell(i,0), unitcell(i,1), unitcell(i,2));
        unitcell_str += QString(buf);
    }
    unitcell_str += "</pre>";

    this->get_label("unitcell_dimensions")->setText(unitcell_str);
}

    /**
     * @brief      Get pointer to label from key
     *
     * @param[in]  key  The key
     *
     * @return     The label.
     */
QLabel* StructureInfoBasicTab::get_label(const std::string& key)
{
    auto it = this->labelmap.find(key);
    if (it == this->labelmap.end()) {
        throw std::logic_error(
            "Invalid key requested from labelmap in StructureInfoBasicTab");
    }
    return it->second;
}

    /**
     * @brief      Resets the object.
     */
void StructureInfoBasicTab::reset()
{
}
