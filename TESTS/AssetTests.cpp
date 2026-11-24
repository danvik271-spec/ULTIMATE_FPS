#include "TestHarness.hpp"
#include "ultimate/assets/AssetCatalog.hpp"
#include "ultimate/assets/AssetDownloader.hpp"
#include "ultimate/assets/AssetValidator.hpp"
#include "ultimate/assets/LicenseChecker.hpp"
#include "ultimate/assets/Sha256.hpp"
#include <filesystem>
#include <fstream>

TEST(LicenseCheckerAllowsOnlyDeclaredCreativeCommonsLicenses) {
    ultimate::assets::LicenseChecker check;
    ultimate::assets::AssetRecord r{"Concrete Surface", "TEXTURES/CONCRETE", "concrete.png", "Poly Haven", "https://dl.polyhaven.org/file/sample.png", "Poly Haven", "CC0-1.0", "2K", "PNG", "catalogued", ""};
    REQUIRE(check.check(r).allowed);
    r.license = "All Rights Reserved"; REQUIRE(!check.check(r).allowed);
    r.license = "CC0-1.0"; r.category = "MISC"; REQUIRE(!check.check(r).allowed);
    r.category = "TEXTURES/CONCRETE"; r.sourceUrl = "http://dl.polyhaven.org/file/sample.png"; REQUIRE(!check.check(r).allowed);
    r.sourceUrl = "https://unknown.example/file.png"; REQUIRE(!check.check(r).allowed);
}
TEST(Sha256MatchesKnownDigest) {
    const auto file = std::filesystem::temp_directory_path() / "ultimate_sha_test.txt";
    { std::ofstream out(file, std::ios::binary); out << "abc"; }
    REQUIRE(ultimate::assets::sha256File(file) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    std::filesystem::remove(file);
}
TEST(AssetCatalogRoundTripsCsvWithQuotedFields) {
    const auto file = std::filesystem::temp_directory_path() / "ultimate_manifest_test.csv";
    ultimate::assets::AssetCatalog c;
    c.add({"Concrete, aged", "TEXTURES/CONCRETE", "TEXTURES/CONCRETE/a.png", "Poly Haven", "https://dl.polyhaven.org/file/a.png", "Author", "CC0-1.0", "2K", "PNG", "downloaded", "abc"});
    std::string error; REQUIRE(c.saveCsv(file, &error));
    ultimate::assets::AssetCatalog loaded; REQUIRE(loaded.loadCsv(file, &error));
    REQUIRE(loaded.records().size() == 1); REQUIRE(loaded.records()[0].name == "Concrete, aged");
    std::filesystem::remove(file);
}
