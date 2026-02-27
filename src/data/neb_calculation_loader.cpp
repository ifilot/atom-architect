#include "neb_calculation_loader.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QObject>

#include <algorithm>

#include "structure_loader.h"

bool NebCalculationLoader::load(const QString& root_directory, QString* error_message)
{
    structures_.clear();

    const QDir root(root_directory);
    if(!root.exists()) {
        if(error_message) {
            *error_message = QObject::tr("Selected directory does not exist.");
        }
        return false;
    }

    QFileInfoList all_subdirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                   QDir::Name | QDir::IgnoreCase);
    std::vector<QFileInfo> image_directories;
    image_directories.reserve((size_t)all_subdirs.size());

    const QRegularExpression image_directory_regex("^\\d+$");

    for(const QFileInfo& entry : all_subdirs) {
        if(image_directory_regex.match(entry.fileName()).hasMatch()) {
            image_directories.push_back(entry);
        }
    }

    std::sort(image_directories.begin(), image_directories.end(),
              [](const QFileInfo& lhs, const QFileInfo& rhs) {
                  bool ok_lhs = false;
                  bool ok_rhs = false;
                  const int lhs_value = lhs.fileName().toInt(&ok_lhs);
                  const int rhs_value = rhs.fileName().toInt(&ok_rhs);

                  if(ok_lhs && ok_rhs && lhs_value != rhs_value) {
                      return lhs_value < rhs_value;
                  }

                  return lhs.fileName() < rhs.fileName();
              });

    if(image_directories.size() < 3) {
        if(error_message) {
            *error_message = QObject::tr("No valid VASP NEB folder structure was found. "
                                         "Expected image folders (e.g. 00, 01, 02, ...) with "
                                         "at least one intermediate image.");
        }
        return false;
    }

    StructureLoader structure_loader;
    for(size_t i = 1; i + 1 < image_directories.size(); ++i) {
        const QString outcar_path = QDir(image_directories[i].absoluteFilePath()).filePath("OUTCAR");
        const QFileInfo outcar_info(outcar_path);

        if(!outcar_info.exists() || !outcar_info.isFile()) {
            if(error_message) {
                *error_message = QObject::tr("Folder '%1' does not contain an OUTCAR file.")
                                     .arg(image_directories[i].fileName());
            }
            structures_.clear();
            return false;
        }

        try {
            auto final_structure = structure_loader.load_outcar_last(outcar_path.toStdString());
            structures_.push_back(final_structure);
        } catch (const std::exception& e) {
            if(error_message) {
                *error_message = QObject::tr("Failed to read OUTCAR in folder '%1': %2")
                                     .arg(image_directories[i].fileName(), e.what());
            }
            structures_.clear();
            return false;
        }
    }

    if(structures_.empty()) {
        if(error_message) {
            *error_message = QObject::tr("No intermediate NEB images were found.");
        }
        return false;
    }

    return true;
}

