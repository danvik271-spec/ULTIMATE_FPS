#pragma once
#include "ultimate/assets/AssetCatalog.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace ultimate::assets {
struct ValidationIssue { std::string fileName; std::string message; };
struct ValidationReport { std::size_t checked{0}; std::size_t valid{0}; std::vector<ValidationIssue> issues; };
class AssetValidator {
public:
    ValidationReport validate(const AssetCatalog& catalog, const std::filesystem::path& assetRoot) const;
};
}
