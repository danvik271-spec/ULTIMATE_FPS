#include "TestHarness.hpp"
#include "ultimate/physics/CharacterMotor.hpp"
#include "ultimate/scene/Camera.hpp"
#include "ultimate/game/CaptureZone.hpp"

TEST(AabbDetectsOverlap) {
    const ultimate::physics::Aabb a{{0,0,0},{1,1,1}};
    const ultimate::physics::Aabb b{{0.5F,0.5F,0.5F},{2,2,2}};
    const ultimate::physics::Aabb c{{1,0,0},{2,1,1}};
    REQUIRE(a.overlaps(b)); REQUIRE(!a.overlaps(c));
}
TEST(CharacterMotorStopsAtStaticWall) {
    ultimate::physics::CollisionWorld world;
    world.addStaticCollider({{1.0F,0.0F,-2.0F},{2.0F,3.0F,2.0F}});
    ultimate::physics::CharacterMotor motor({{-0.25F,0.0F,-0.25F},{0.25F,1.7F,0.25F}});
    ultimate::math::Vec3 position{0,0,0};
    const auto result = motor.move(position, {2.0F,0.0F,0.0F}, world);
    REQUIRE(result.collidedX); REQUIRE(position.x == 0.0F);
}
TEST(CameraClampsPitchAndProducesNormalizedForward) {
    ultimate::scene::Camera camera; camera.setRotation(0.5F, 9.0F);
    const auto f = camera.forward();
    REQUIRE(ultimate::math::length(f) > 0.99F); REQUIRE(camera.pitch() < 1.6F);
}
TEST(CaptureZoneCompletesAfterTimeInBounds) {
    ultimate::game::CaptureZone zone("test", {{0,0,0},{2,2,2}}, 0.5F);
    REQUIRE(!zone.update({1,1,1}, 0.1F));
    REQUIRE(!zone.update({1,1,1}, 0.1F));
    REQUIRE(!zone.update({1,1,1}, 0.1F));
    REQUIRE(!zone.update({1,1,1}, 0.1F));
    REQUIRE(zone.update({1,1,1}, 0.1F)); REQUIRE(zone.completed());
}
