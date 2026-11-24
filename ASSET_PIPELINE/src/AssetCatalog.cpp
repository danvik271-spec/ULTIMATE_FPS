#include "ultimate/assets/AssetCatalog.hpp"
#include <fstream>
#include <iterator>
#include <sstream>
#include <unordered_map>

namespace ultimate::assets {
namespace {
std::string quote(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string out = "\"";
    for (const char c : value) { if (c == '"') out += '"'; out += c; }
    return out + '"';
}
bool parseRow(const std::string& line, std::vector<std::string>& fields) {
    fields.clear(); std::string field; bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') { field += '"'; ++i; }
            else if (c == '"') quoted = false;
            else field += c;
        } else if (c == ',' ) { fields.push_back(std::move(field)); field.clear(); }
        else if (c == '"' && field.empty()) quoted = true;
        else field += c;
    }
    if (quoted) return false;
    fields.push_back(std::move(field)); return true;
}
std::string get(const std::vector<std::string>& row, const std::unordered_map<std::string, std::size_t>& columns, const char* key) {
    const auto it = columns.find(key);
    return it != columns.end() && it->second < row.size() ? row[it->second] : std::string{};
}
}
bool AssetCatalog::loadCsv(const std::filesystem::path& path, std::string* error) {
    std::ifstream in(path);
    if (!in) { if (error) *error = "cannot open manifest: " + path.string(); return false; }
    std::string line;
    if (!std::getline(in, line)) { if (error) *error = "manifest has no header"; return false; }
    std::vector<std::string> header;
    if (!parseRow(line, header)) { if (error) *error = "invalid CSV header"; return false; }
    std::unordered_map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i) columns[header[i]] = i;
    for (const char* required : {"Name", "Category", "FileName", "Source", "SourceUrl", "Author", "License", "Resolution", "Format", "IntegrationStatus", "SHA256"}) {
        if (!columns.contains(required)) { if (error) *error = std::string("missing column: ") + required; return false; }
    }
    std::vector<AssetRecord> loaded;
    std::size_t lineNo = 1;
    while (std::getline(in, line)) {
        ++lineNo; if (line.empty()) continue;
        std::vector<std::string> row;
        if (!parseRow(line, row)) { if (error) *error = "malformed CSV at line " + std::to_string(lineNo); return false; }
        AssetRecord r;
        r.name = get(row, columns, "Name"); r.category = get(row, columns, "Category");
        r.fileName = get(row, columns, "FileName"); r.source = get(row, columns, "Source");
        r.sourceUrl = get(row, columns, "SourceUrl"); r.author = get(row, columns, "Author");
        r.license = get(row, columns, "License"); r.resolution = get(row, columns, "Resolution");
        r.format = get(row, columns, "Format"); r.integrationStatus = get(row, columns, "IntegrationStatus");
        r.sha256 = get(row, columns, "SHA256");
        loaded.push_back(std::move(r));
    }
    records_ = std::move(loaded); return true;
}
bool AssetCatalog::saveCsv(const std::filesystem::path& path, std::string* error) const {
    std::error_code ec; if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (!out) { if (error) *error = "cannot write manifest: " + path.string(); return false; }
    out << "Name,Category,FileName,Source,SourceUrl,Author,License,Resolution,Format,IntegrationStatus,SHA256\n";
    for (const auto& r : records_) {
        const std::string fields[] = {r.name,r.category,r.fileName,r.source,r.sourceUrl,r.author,r.license,r.resolution,r.format,r.integrationStatus,r.sha256};
        for (std::size_t i = 0; i < std::size(fields); ++i) { if (i) out << ','; out << quote(fields[i]); }
        out << '\n';
    }
    if (!out) { if (error) *error = "failed writing manifest"; return false; }
    return true;
}
void AssetCatalog::add(AssetRecord record) { records_.push_back(std::move(record)); }
}
