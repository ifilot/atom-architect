#pragma once

#include <QString>

#include <memory>
#include <vector>

#include "structure.h"

class NebCalculationLoader {
public:
    bool load(const QString& root_directory, QString* error_message = nullptr);

    const std::vector<std::shared_ptr<Structure>>& structures() const {
        return structures_;
    }

private:
    std::vector<std::shared_ptr<Structure>> structures_;
};

