#include "ultimate/assets/AssetImporter.hpp"
#include "ultimate/assets/AssetMetadata.hpp"
#include "ultimate/assets/LicenseChecker.hpp"
#include "ultimate/assets/Sha256.hpp"
#include <filesystem>

namespace ultimate::assets {
ImportResult AssetImporter::importFile(const AssetRecord& record, const std::filesystem::path& sourceFile,
                                       const std::filesystem::path& assetRoot) const {
    const auto check = LicenseChecker{}.check(record);
    if (!check.allowed) return {false, {}, "license/source check failed: " + check.reason};
    const auto meta = inspectFile(sourceFile);
    if (!meta.recognized) return {false, {}, "source file type was not recognized"};
    const auto relative = std::filesystem::path(record.fileName).lexically_normal();
    if (relative.empty() || relative.is_absolute() || *relative.begin() == "..") return {false, {}, "unsafe relative destination path"};
    const auto dest = assetRoot / relative;
    std::error_code ec;
    std::filesystem::create_directories(dest.parent_path(), ec);
    if (ec) return {false, dest, "cannot create asset directory: " + ec.message()};
    std::filesystem::copy_file(sourceFile, dest, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) return {false, dest, "copy failed: " + ec.message()};
    return {true, dest, "imported; SHA-256=" + sha256File(dest)};
}
}
