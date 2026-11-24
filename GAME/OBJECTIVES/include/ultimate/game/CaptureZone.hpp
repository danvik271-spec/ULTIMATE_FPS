#pragma once
#include "ultimate/math/Vec.hpp"
#include "ultimate/physics/Aabb.hpp"
#include <string>

namespace ultimate::game {
class CaptureZone {
public:
    CaptureZone(std::string name, physics::Aabb bounds, float requiredSeconds);
    bool update(const math::Vec3& playerPosition, float deltaSeconds);
    [[nodiscard]] bool completed() const noexcept { return completed_; }
    [[nodiscard]] float progress() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
private:
    std::string name_;
    physics::Aabb bounds_;
    float requiredSeconds_{1.0F};
    float elapsed_{0.0F};
    bool completed_{false};
};
}
