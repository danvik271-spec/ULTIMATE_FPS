#include "ultimate/assets/AssetCatalog.hpp"
#include "ultimate/assets/AssetDownloader.hpp"
#include "ultimate/assets/AssetValidator.hpp"
#include <filesystem>
#include <iostream>
#include <string>

namespace {
void usage() {
    std::cout << "assetctl validate <manifest.csv> <asset-root>\n"
                 "assetctl download <manifest.csv> <asset-root>\n"
                 "assetctl list <manifest.csv>\n\n"
                 "The downloader accepts only HTTPS records with an approved provider host and an allowlisted license.\n";
}
}
int main(int argc, char** argv) {
    if (argc < 3) { usage(); return 2; }
    const std::string command = argv[1];
    ultimate::assets::AssetCatalog catalog;
    std::string error;
    if (!catalog.loadCsv(argv[2], &error)) { std::cerr << error << '\n'; return 2; }
    if (command == "list") {
        for (const auto& r : catalog.records()) std::cout << r.name << " | " << r.category << " | " << r.fileName << " | " << r.license << " | " << r.integrationStatus << '\n';
        std::cout << catalog.records().size() << " catalog entr(y/ies)\n"; return 0;
    }
    if (argc < 4) { usage(); return 2; }
    const std::filesystem::path root = argv[3];
    if (command == "validate") {
        const auto report = ultimate::assets::AssetValidator{}.validate(catalog, root);
        std::cout << "checked=" << report.checked << " valid=" << report.valid << " issues=" << report.issues.size() << '\n';
        for (const auto& issue : report.issues) std::cout << "[FAIL] " << issue.fileName << ": " << issue.message << '\n';
        return report.issues.empty() ? 0 : 1;
    }
    if (command == "download") {
        ultimate::assets::AssetDownloader downloader;
        bool failed = false;
        for (auto& r : catalog.records()) {
            const auto dest = root / r.fileName;
            if (std::filesystem::exists(dest)) { std::cout << "[SKIP] exists: " << r.fileName << '\n'; continue; }
            const auto result = downloader.download(r, dest);
            if (result.success) { r.sha256 = result.sha256; r.integrationStatus = "downloaded"; std::cout << "[OK] " << r.fileName << " " << result.message << '\n'; }
            else { failed = true; std::cerr << "[FAIL] " << r.fileName << ": " << result.message << '\n'; }
        }
        const std::filesystem::path manifestOut = root / "ASSET_MANIFEST.csv";
        if (!catalog.saveCsv(manifestOut, &error)) { std::cerr << error << '\n'; return 2; }
        return failed ? 1 : 0;
    }
    usage(); return 2;
}
