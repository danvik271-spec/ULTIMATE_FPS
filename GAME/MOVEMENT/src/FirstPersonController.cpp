#include "ultimate/game/FirstPersonController.hpp"
#include <algorithm>
#include <cmath>

namespace ultimate::game {
void FirstPersonController::update(Player& player, const input::InputState& state, const float dt, const physics::CollisionWorld& world) const {
    const float safeDt = std::clamp(dt, 0.0F, 0.05F);
    const auto look = state.lookDelta();
    player.camera().rotate(look.x * lookSensitivity, -look.y * lookSensitivity);
    math::Vec3 wish{};
    const auto forward = player.camera().forward();
    const auto right = player.camera().right();
    if (state.down(input::Key::Forward)) wish += {forward.x, 0.0F, forward.z};
    if (state.down(input::Key::Backward)) wish -= {forward.x, 0.0F, forward.z};
    if (state.down(input::Key::Right)) wish += right;
    if (state.down(input::Key::Left)) wish -= right;
    wish.y = 0.0F;
    wish = math::normalized(wish);
    const float speed = state.down(input::Key::Sprint) ? sprintSpeed : walkSpeed;
    player.move(wish * (speed * safeDt), world);
}
}
