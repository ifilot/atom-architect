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

#include "fragment_selector.h"
#include "qjsonarray.h"

FragmentSelector::FragmentSelector(QWidget *parent) : StructureInfoTab(parent) {
    // set gridlayout as default layout
    this->layout = new QVBoxLayout();
    this->scrollarea->setLayout(this->layout);

    // add descriptive label
    this->layout->addWidget(new QLabel(tr("<b>Select widget from list</b>")));

    // add search box
    this->searchbox = new QLineEdit(this);
    this->layout->addWidget(this->searchbox);

    // add fragment list
    this->fragment_list = new QListWidget();
    this->layout->addWidget(this->fragment_list);

    // add secondary anaglyph widget
    this->anaglyph_widget = new AnaglyphWidget();
    this->layout->addWidget(this->anaglyph_widget);
    this->anaglyph_widget->disable_draw_unitcell();

    // add current selection label
    this->label_current_selection = new QLabel("");
    this->layout->addWidget(this->label_current_selection);

    // add periodic table pop-up button
    this->button_periodic_table = new QPushButton("Periodic table");
    this->button_periodic_table->setIcon(QIcon(":/assets/icon/periodic_table_32.png"));
    this->layout->addWidget(this->button_periodic_table);
    connect(this->button_periodic_table, SIGNAL(clicked()), this, SLOT(select_atom_periodic_table()));

    // add fragments
    this->add_fragments_from_file("hydrocarbons.json");
    this->add_fragments_from_file("adsorbates.json");

    // perform initial fuzzy search
    this->perform_fuzzy_search("CO");
    this->fragment_list->setCurrentRow(0);
    this->update_display(this->fragment_list->item(0), this->fragment_list->item(0));

    connect(this->searchbox, SIGNAL(textChanged(const QString&)), this, SLOT(perform_fuzzy_search(const QString&)));
    connect(this->fragment_list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(update_display(QListWidgetItem*, QListWidgetItem*)));
}

/**
 * @brief      Gets the current fragment.
 *
 * @return     The current fragment.
 */
const Fragment& FragmentSelector::get_current_fragment() const {
    auto got = this->fragments.find(this->fragment_list->currentItem()->text().toStdString());
    return got->second;

}

/**
 * @brief      Update data in tab based on current structure
 */
void FragmentSelector::update_data() {

}

/**
 * @brief      Resets the object.
 */
void FragmentSelector::reset() {

}

/**
 * @brief      Add a fragment
 *
 * @param[in]  fragment  The fragment
 */
void FragmentSelector::add_fragment(const Fragment& fragment) {
    this->fragments.emplace(fragment.label.toStdString(), fragment);
}

/**
 * @brief      Add series of fragments from file
 *
 * @param[in]  filename  The filename
 */
void FragmentSelector::add_fragments_from_file(const QString& filename) {
    qDebug() << "Loading fragments: " << filename;

    // try to locate atoms.json
    QFile f(":/assets/fragments/" + filename);
    if (!f.exists()) {
        qWarning() << "Cannot open atoms.json";
        exit(-1);
    }

    // open the file and read the data
    f.open(QIODevice::ReadOnly);
    QString jsondata = f.readAll();
    f.close();

    // try to parse the file
    QJsonParseError parseError;
    QJsonDocument root = QJsonDocument::fromJson(jsondata.toUtf8(), &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
        exit(-1);
    }

    // loop over all fragments
    QJsonObject fragments = root["fragments"].toObject();
    foreach(const QString& key, fragments.keys()) {
        QJsonValue value = fragments.value(key);
        Fragment fragment(key.toStdString(), root["fragments"][key]["label"].toString());

        qDebug() << key;

        QJsonArray atoms = root["fragments"][key]["atoms"].toArray();
        foreach(const auto& v, atoms) {
            qDebug() << v;

            unsigned int elnr = AtomSettings::get().get_atom_elnr(v[0].toString().toStdString());
            double x = v[1].toDouble();
            double y = v[2].toDouble();
            double z = v[3].toDouble();
            fragment.add_atom(elnr, x, y, z);
        }

        this->add_fragment(fragment);

        if(root["fragments"][key]["synonyms"].toArray().size() != 0) {
            QJsonArray synonyms = root["fragments"][key]["synonyms"].toArray();
            foreach(const auto& v, synonyms) {
                qDebug() << v.toString();
                fragment.label = v.toString();
                this->add_fragment(fragment);
            }
        }
    }
}

/**
 * @brief      Perform fuzzy search
 */
void FragmentSelector::perform_fuzzy_search(const QString& _source) {
    std::vector<std::pair<size_t, std::string>> result;

    std::string source = _source.toStdString();
    for(const auto& it : this->fragments) {
        std::string target = it.second.label.toStdString();
        result.emplace_back(this->string_levenshtein_distance(source, target), target);
    }
    std::sort(result.begin(), result.end());

    this->fragment_list->clear();
    for(const auto& res : result) {
        this->fragment_list->addItem(res.second.c_str());
    }
}

/**
 * @brief      Update the anaglyph widget with the selected molecule
 *
 * @param      current   Currently selected fragment
 * @param      previous  Previously selected fragments
 */
void FragmentSelector::update_display(QListWidgetItem *current, QListWidgetItem *previous) {
    if(current != nullptr) {
        auto got = this->fragments.find(current->text().toStdString());
        const auto& fragment = got->second;
        this->anaglyph_widget->set_structure(std::make_shared<Structure>(fragment));
        this->label_current_selection->setText(tr("<b>Selected fragment: </b> ") + current->text());
        emit(signal_new_fragment(fragment));
    }
}

/**
 * @brief      Select an atom from the periodic table
 */
void FragmentSelector::select_atom_periodic_table() {
    DialogPeriodicTable pd;
    int ret = pd.exec();
    if(ret >= 1 && ret <= 118) {
        auto elname = AtomSettings::get().get_name_from_elnr(ret);
        auto fragment = Fragment(elname, elname.c_str());
        fragment.add_atom((unsigned int)ret, 0.0, 0.0, 0.0);
        this->anaglyph_widget->set_structure(std::make_shared<Structure>(fragment));
        this->label_current_selection->setText(tr("<b>Selected atom: </b> ") + elname.c_str());
        emit(signal_new_fragment(fragment));
    }
}

size_t FragmentSelector::string_levenshtein_distance(const std::string& s1, const std::string& s2) {
    const size_t m(s1.size());
    const size_t n(s2.size());

    if( m==0 ) return n;
    if( n==0 ) return m;

    size_t *costs = new size_t[n + 1];

    for( size_t k=0; k<=n; k++ ) {
        costs[k] = k;
    }

    size_t i = 0;
    for(std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i) {
        costs[0] = i+1;
        size_t corner = i;

        size_t j = 0;

        for(std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j) {
            size_t upper = costs[j+1];
            if( *it1 == *it2 ) {
                costs[j+1] = corner;
            }
            else {
                size_t t(upper<corner?upper:corner);
                costs[j+1] = (costs[j]<t?costs[j]:t)+1;
            }
            corner = upper;
        }
    }

    size_t result = costs[n];
    delete [] costs;

    return result;
}
