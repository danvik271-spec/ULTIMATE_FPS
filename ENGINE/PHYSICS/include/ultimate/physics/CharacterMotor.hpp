#pragma once
#include "ultimate/physics/CollisionWorld.hpp"
#include "ultimate/math/Vec.hpp"

namespace ultimate::physics {
struct MoveResult { math::Vec3 applied{}; bool collidedX{false}; bool collidedY{false}; bool collidedZ{false}; };
class CharacterMotor {
public:
    explicit CharacterMotor(Aabb localBounds = {{-0.3F, 0.0F, -0.3F}, {0.3F, 1.7F, 0.3F}}) : localBounds_(localBounds) {}
    MoveResult move(math::Vec3& position, const math::Vec3& displacement, const CollisionWorld& world) const;
private:
    Aabb localBounds_;
};
}
