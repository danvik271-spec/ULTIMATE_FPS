#include "ultimate/core/Log.hpp"
#include "ultimate/game/CaptureZone.hpp"
#include "ultimate/game/Player.hpp"
#include "ultimate/physics/CollisionWorld.hpp"
#include <exception>
#include <iostream>
#include <string>
#ifdef ULTIMATE_HAS_VULKAN
#include "ultimate/graphics/VulkanRenderer.hpp"
#endif

int main(int argc, char** argv) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--headless") headless = true;
    ultimate::physics::CollisionWorld world;
    world.addStaticCollider({{-8.0F,-0.5F,-8.0F},{8.0F,0.0F,8.0F}});
    world.addStaticCollider({{-4.0F,0.0F,-4.0F},{-3.5F,3.0F,4.0F}});
    ultimate::game::Player player;
    ultimate::game::CaptureZone zone("Atrium Survey", {{1.0F,0.0F,1.0F},{3.0F,2.5F,3.0F}}, 4.0F);
    ultimate::core::log(ultimate::core::LogLevel::Info, "ULTIMATE custom runtime initialized");
    ultimate::core::log(ultimate::core::LogLevel::Info, "Prototype mode: first-person exploration and nonviolent capture objectives");
    (void)player; (void)zone; (void)world;
#ifdef ULTIMATE_HAS_VULKAN
    if (!headless) {
        ultimate::graphics::VulkanRenderer renderer;
        std::string error;
        if (!renderer.initialize({}, &error)) {
            ultimate::core::log(ultimate::core::LogLevel::Error, error);
            return 1;
        }
        try {
            while (!renderer.shouldClose()) { renderer.pollEvents(); renderer.drawFrame(); }
        } catch (const std::exception& e) {
            ultimate::core::log(ultimate::core::LogLevel::Error, e.what());
            return 1;
        }
        renderer.shutdown();
        return 0;
    }
#else
    (void)headless;
    std::cout << "Vulkan backend was not built. Install Vulkan SDK, GLFW and glslc, then reconfigure.\n";
#endif
    std::cout << "Headless runtime ready. Core simulation and objective systems are initialized.\n";
    return 0;
}
