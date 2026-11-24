#include "ultimate/assets/AssetMetadata.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>

namespace ultimate::assets {
namespace {
std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); return s; }
}
FileMetadata inspectFile(const std::filesystem::path& path) {
    FileMetadata m;
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) || ec) { m.description = "not a regular file"; return m; }
    m.sizeBytes = std::filesystem::file_size(path, ec);
    const auto ext = lower(path.extension().string());
    std::ifstream in(path, std::ios::binary);
    std::array<unsigned char, 32> h{};
    in.read(reinterpret_cast<char*>(h.data()), static_cast<std::streamsize>(h.size()));
    const auto n = static_cast<std::size_t>(in.gcount());
    if (n >= 24 && h[0] == 0x89 && h[1] == 'P' && h[2] == 'N' && h[3] == 'G') {
        const auto be32 = [&](std::size_t p) { return (static_cast<unsigned>(h[p])<<24U)|(static_cast<unsigned>(h[p+1])<<16U)|(static_cast<unsigned>(h[p+2])<<8U)|h[p+3]; };
        m.format="PNG"; m.resolution=std::to_string(be32(16))+"x"+std::to_string(be32(20)); m.recognized=true; m.description="PNG image header";
    } else if (n >= 12 && h[0]=='R' && h[1]=='I' && h[2]=='F' && h[3]=='F' && h[8]=='W' && h[9]=='A' && h[10]=='V' && h[11]=='E') {
        m.format="WAV"; m.recognized=true; m.description="RIFF/WAVE audio header";
    } else if (n >= 3 && h[0]==0xff && h[1]==0xd8 && h[2]==0xff) {
        m.format="JPEG"; m.recognized=true; m.description="JPEG header (dimensions require full marker scan)";
    } else if (n >= 4 && h[0]=='g' && h[1]=='l' && h[2]=='T' && h[3]=='F') {
        m.format="GLB"; m.recognized=true; m.description="glTF binary model";
    } else if (ext==".gltf" || ext==".obj" || ext==".fbx" || ext==".ply" || ext==".stl") {
        m.format=ext.substr(1); m.recognized=true; m.description="recognized by model file extension";
    } else if (ext==".exr" || ext==".hdr" || ext==".tga" || ext==".bmp" || ext==".tif" || ext==".tiff") {
        m.format=ext.substr(1); m.recognized=true; m.description="recognized by image file extension";
    } else if (ext==".ogg" || ext==".flac" || ext==".mp3") {
        m.format=ext.substr(1); m.recognized=true; m.description="recognized by audio file extension; codec signature not checked";
    } else {
        m.format=ext.empty()?"unknown":ext.substr(1); m.description="unrecognized media format";
    }
    return m;
}
}
