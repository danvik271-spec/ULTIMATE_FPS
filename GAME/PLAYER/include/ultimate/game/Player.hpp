#pragma once
#include "ultimate/math/Vec.hpp"
#include "ultimate/physics/CharacterMotor.hpp"
#include "ultimate/scene/Camera.hpp"

namespace ultimate::game {
class Player {
public:
    Player();
    void move(const math::Vec3& displacement, const physics::CollisionWorld& world);
    [[nodiscard]] math::Vec3 position() const noexcept { return position_; }
    [[nodiscard]] scene::Camera& camera() noexcept { return camera_; }
    [[nodiscard]] const scene::Camera& camera() const noexcept { return camera_; }
private:
    math::Vec3 position_{0.0F, 0.0F, 0.0F};
    physics::CharacterMotor motor_{};
    scene::Camera camera_{};
};
}
