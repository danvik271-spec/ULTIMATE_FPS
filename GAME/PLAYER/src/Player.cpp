#include "ultimate/game/Player.hpp"

namespace ultimate::game {
Player::Player() { camera_.setPosition({0.0F, 1.62F, 0.0F}); }
void Player::move(const math::Vec3& displacement, const physics::CollisionWorld& world) {
    motor_.move(position_, displacement, world);
    camera_.setPosition(position_ + math::Vec3{0.0F, 1.62F, 0.0F});
}
}
