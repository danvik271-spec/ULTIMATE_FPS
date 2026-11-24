#pragma once
#include <string>

namespace ultimate::assets {
struct AssetRecord {
    std::string name;
    std::string category;
    std::string fileName;
    std::string source;
    std::string sourceUrl;
    std::string author;
    std::string license;
    std::string resolution;
    std::string format;
    std::string integrationStatus{"catalogued"};
    std::string sha256;
};
}
