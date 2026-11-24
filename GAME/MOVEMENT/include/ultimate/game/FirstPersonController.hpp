#pragma once
#include "ultimate/game/Player.hpp"
#include "ultimate/input/InputState.hpp"

namespace ultimate::game {
class FirstPersonController {
public:
    float walkSpeed{4.2F};
    float sprintSpeed{6.0F};
    float lookSensitivity{0.0022F};
    void update(Player& player, const input::InputState& input, float deltaSeconds, const physics::CollisionWorld& world) const;
};
}
