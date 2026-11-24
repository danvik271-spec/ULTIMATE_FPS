#include "ultimate/assets/AssetValidator.hpp"
#include "ultimate/assets/LicenseChecker.hpp"
#include "ultimate/assets/AssetMetadata.hpp"
#include "ultimate/assets/Sha256.hpp"
#include <filesystem>

namespace ultimate::assets {
ValidationReport AssetValidator::validate(const AssetCatalog& catalog, const std::filesystem::path& root) const {
    ValidationReport report;
    LicenseChecker licenseChecker;
    for (const auto& record : catalog.records()) {
        ++report.checked;
        const auto license = licenseChecker.check(record);
        if (!license.allowed) { report.issues.push_back({record.fileName, "license check: " + license.reason}); continue; }
        const auto relative = std::filesystem::path(record.fileName).lexically_normal();
        if (relative.empty() || relative.is_absolute() || *relative.begin() == "..") { report.issues.push_back({record.fileName, "unsafe relative file path"}); continue; }
        const auto full = root / relative;
        std::error_code ec;
        if (!std::filesystem::is_regular_file(full, ec) || ec) { report.issues.push_back({record.fileName, "file is missing"}); continue; }
        const auto meta = inspectFile(full);
        if (!meta.recognized) { report.issues.push_back({record.fileName, "unrecognized or unsupported file type"}); continue; }
        if (!record.format.empty() && record.format != meta.format) { report.issues.push_back({record.fileName, "format mismatch: manifest=" + record.format + ", file=" + meta.format}); continue; }
        if (!record.sha256.empty()) {
            const auto digest = sha256File(full);
            if (digest != record.sha256) { report.issues.push_back({record.fileName, "SHA-256 mismatch"}); continue; }
        }
        ++report.valid;
    }
    return report;
}
}
