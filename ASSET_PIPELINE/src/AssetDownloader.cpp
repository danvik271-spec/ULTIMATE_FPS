#include "ultimate/assets/AssetDownloader.hpp"
#include "ultimate/assets/LicenseChecker.hpp"
#include "ultimate/assets/AssetMetadata.hpp"
#include "ultimate/assets/Sha256.hpp"
#include <filesystem>
#include <algorithm>
#include <cctype>
#ifdef ULTIMATE_HAS_CURL
#include <curl/curl.h>
#include <cstdio>
#endif

namespace ultimate::assets {
#ifdef ULTIMATE_HAS_CURL
namespace {
size_t writeFile(char* data, const size_t size, const size_t count, void* user) {
    return std::fwrite(data, 1, size * count, static_cast<std::FILE*>(user));
}
}
#endif
DownloadResult AssetDownloader::download(const AssetRecord& record, const std::filesystem::path& destination) const {
    const auto check = LicenseChecker{}.check(record);
    if (!check.allowed) return {false, "license/source check failed: " + check.reason, {}};
#ifndef ULTIMATE_HAS_CURL
    (void)destination;
    return {false, "this build has no libcurl; install libcurl development files and reconfigure", {}};
#else
    std::error_code ec;
    std::filesystem::create_directories(destination.parent_path(), ec);
    if (ec) return {false, "cannot create destination directory: " + ec.message(), {}};
    const auto tempPath = destination.parent_path() / (destination.stem().string() + ".partial" + destination.extension().string());
    const auto temp = tempPath.string();
    std::FILE* file = std::fopen(temp.c_str(), "wb");
    if (!file) return {false, "cannot create temporary destination", {}};
    static const CURLcode globalInit = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (globalInit != CURLE_OK) { std::fclose(file); std::filesystem::remove(temp, ec); return {false, "curl global initialization failed", {}}; }
    CURL* curl = curl_easy_init();
    if (!curl) { std::fclose(file); std::filesystem::remove(temp, ec); return {false, "curl initialization failed", {}}; }
    curl_easy_setopt(curl, CURLOPT_URL, record.sourceUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "UltimateFPSAssetPipeline/0.1 (+local-asset-importer)");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    const CURLcode result = curl_easy_perform(curl);
    long response = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
    char* effectiveUrl = nullptr; curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);
    const std::string finalUrl = effectiveUrl ? effectiveUrl : "";
    curl_easy_cleanup(curl);
    const int closeResult = std::fclose(file);
    if (result != CURLE_OK || closeResult != 0 || response < 200 || response >= 300) {
        std::filesystem::remove(temp, ec);
        return {false, std::string("download failed: ") + curl_easy_strerror(result) + " (HTTP " + std::to_string(response) + ")", {}};
    }
    AssetRecord finalRecord = record;
    finalRecord.sourceUrl = finalUrl;
    const auto finalCheck = LicenseChecker{}.check(finalRecord);
    if (!finalCheck.allowed) { std::filesystem::remove(temp, ec); return {false, "redirect target rejected: " + finalCheck.reason, {}}; }
    const auto metadata = inspectFile(temp);
    if (!metadata.recognized) { std::filesystem::remove(temp, ec); return {false, "downloaded payload does not match a recognized media format", {}}; }
    if (!record.format.empty()) {
        auto expected = record.format; auto actual = metadata.format;
        std::transform(expected.begin(), expected.end(), expected.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(actual.begin(), actual.end(), actual.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (expected != actual) { std::filesystem::remove(temp, ec); return {false, "downloaded file format mismatch: expected " + record.format + ", got " + metadata.format, {}}; }
    }
    const auto digest = sha256File(temp);
    if (digest.empty()) { std::filesystem::remove(temp, ec); return {false, "cannot compute SHA-256 after download", {}}; }
    std::filesystem::rename(temp, destination, ec);
    if (ec) { std::filesystem::remove(temp); return {false, "cannot atomically place downloaded file: " + ec.message(), {}}; }
    return {true, "downloaded; " + check.reason, digest};
#endif
}
}
