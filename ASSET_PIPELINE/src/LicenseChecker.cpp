#include "ultimate/assets/LicenseChecker.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <vector>

namespace ultimate::assets {
namespace {
std::string lower(std::string value) { std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); }); return value; }
bool startsWith(const std::string& s, const std::string& prefix) { return s.rfind(prefix, 0) == 0; }
}
bool LicenseChecker::isAllowedLicense(std::string license) {
    license = lower(std::move(license));
    return license == "cc0-1.0" || license == "cc-by-4.0" || license == "cc-by-sa-4.0";
}
LicenseCheck LicenseChecker::check(const AssetRecord& r) const {
    if (r.name.empty() || r.category.empty() || r.fileName.empty() || r.source.empty()) return {false, "name, safe category, file name and source are required"};
    if (r.author.empty()) return {false, "author field is required; use 'Unknown' only when source metadata explicitly says so"};
    if (!isAllowedLicense(r.license)) return {false, "license is not on the project's allowlist (CC0-1.0, CC-BY-4.0, CC-BY-SA-4.0)"};
    const auto category = lower(r.category);
    const std::vector<std::string> safeCategories = {
        "textures/", "materials/", "environment/", "architecture/", "props/", "vegetation/",
        "skyboxes/", "maps/", "audio/ambience/", "audio/environment/", "audio/ui/", "audio/music/"
    };
    const bool safeCategory = std::any_of(safeCategories.begin(), safeCategories.end(), [&](const std::string& prefix) { return startsWith(category, prefix); });
    if (!safeCategory) return {false, "this build only imports environment, surface, ambience and interface assets"};
    const auto relative = std::filesystem::path(r.fileName).lexically_normal();
    if (relative.empty() || relative.is_absolute() || *relative.begin() == "..") return {false, "asset file name must be a safe relative path"};
    if (!startsWith(r.sourceUrl, "https://")) return {false, "asset URL must use HTTPS"};
    const auto hostStart = std::string("https://").size();
    const auto hostEnd = r.sourceUrl.find_first_of("/:?#", hostStart);
    const auto host = lower(r.sourceUrl.substr(hostStart, hostEnd == std::string::npos ? std::string::npos : hostEnd - hostStart));
    const bool approvedHost = host == "polyhaven.com" || host.ends_with(".polyhaven.com") || host == "polyhaven.org" || host.ends_with(".polyhaven.org") || host == "ambientcg.com" || host.ends_with(".ambientcg.com");
    if (!approvedHost) return {false, "source host is not on the approved environment-asset provider list"};
    return {true, "metadata passes the local allowlist; verify the individual asset's source page and license before distribution"};
}
}
