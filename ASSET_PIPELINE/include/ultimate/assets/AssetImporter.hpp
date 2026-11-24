#pragma once
#include "ultimate/assets/AssetRecord.hpp"
#include <filesystem>
#include <string>

namespace ultimate::assets {
struct ImportResult { bool success{false}; std::filesystem::path destination; std::string message; };
class AssetImporter {
public:
    ImportResult importFile(const AssetRecord& record, const std::filesystem::path& sourceFile,
                            const std::filesystem::path& assetRoot) const;
};
}
