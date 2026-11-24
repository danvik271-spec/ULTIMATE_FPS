#include "ultimate/game/CaptureZone.hpp"
#include <algorithm>

namespace ultimate::game {
CaptureZone::CaptureZone(std::string name, physics::Aabb bounds, float requiredSeconds)
    : name_(std::move(name)), bounds_(bounds), requiredSeconds_(std::max(0.01F, requiredSeconds)) {}
bool CaptureZone::update(const math::Vec3& p, const float dt) {
    if (completed_) return true;
    const physics::Aabb point{p, p + math::Vec3{0.001F, 0.001F, 0.001F}};
    if (bounds_.overlaps(point)) elapsed_ = std::min(requiredSeconds_, elapsed_ + std::clamp(dt, 0.0F, 0.1F));
    else elapsed_ = std::max(0.0F, elapsed_ - std::clamp(dt, 0.0F, 0.1F) * 0.5F);
    completed_ = elapsed_ >= requiredSeconds_;
    return completed_;
}
float CaptureZone::progress() const noexcept { return elapsed_ / requiredSeconds_; }
}
