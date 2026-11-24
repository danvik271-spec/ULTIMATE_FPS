#pragma once
#include "ultimate/assets/AssetRecord.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace ultimate::assets {
class AssetCatalog {
public:
    bool loadCsv(const std::filesystem::path& path, std::string* error = nullptr);
    bool saveCsv(const std::filesystem::path& path, std::string* error = nullptr) const;
    void add(AssetRecord record);
    [[nodiscard]] const std::vector<AssetRecord>& records() const noexcept { return records_; }
    [[nodiscard]] std::vector<AssetRecord>& records() noexcept { return records_; }
private:
    std::vector<AssetRecord> records_;
};
}
