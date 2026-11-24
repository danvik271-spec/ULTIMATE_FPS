#pragma once
#include "ultimate/assets/AssetRecord.hpp"
#include <string>

namespace ultimate::assets {
struct LicenseCheck { bool allowed{false}; std::string reason; };
class LicenseChecker {
public:
    [[nodiscard]] LicenseCheck check(const AssetRecord& record) const;
    [[nodiscard]] static bool isAllowedLicense(std::string license);
};
}
