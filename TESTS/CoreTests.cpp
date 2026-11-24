#include "TestHarness.hpp"
#include "ultimate/core/Clock.hpp"
#include "ultimate/core/Config.hpp"
#include "ultimate/core/EventBus.hpp"
#include "ultimate/core/JobSystem.hpp"
#include "ultimate/core/MemoryArena.hpp"
#include <filesystem>
#include <fstream>
#include <string>

TEST(MemoryArenaHonorsAlignmentAndReset) {
    ultimate::core::MemoryArena arena(128);
    auto* value = arena.make<std::uint64_t>(0x1234U);
    REQUIRE(reinterpret_cast<std::uintptr_t>(value) % alignof(std::uint64_t) == 0);
    REQUIRE(*value == 0x1234U);
    REQUIRE(arena.used() >= sizeof(std::uint64_t));
    arena.reset(); REQUIRE(arena.used() == 0);
}
TEST(EventBusSubscribePublishUnsubscribe) {
    struct Ping { int value; };
    ultimate::core::EventBus bus; int observed = 0;
    const auto id = bus.subscribe<Ping>([&](const Ping& e) { observed += e.value; });
    bus.publish(Ping{7}); REQUIRE(observed == 7);
    REQUIRE(bus.unsubscribe<Ping>(id)); bus.publish(Ping{9}); REQUIRE(observed == 7);
}
TEST(JobSystemRunsSubmittedWork) {
    ultimate::core::JobSystem jobs(2);
    auto a = jobs.submit([](int x, int y) { return x + y; }, 10, 32);
    auto b = jobs.submit([] { return std::string("ok"); });
    REQUIRE(a.get() == 42); REQUIRE(b.get() == "ok"); jobs.waitIdle();
}
TEST(ConfigReadsAndWritesKeyValuePairs) {
    const auto file = std::filesystem::temp_directory_path() / "ultimate_config_test.ini";
    ultimate::core::Config config; config.set("width", "1920"); config.set("vsync", "true");
    std::string error; REQUIRE(config.save(file, &error));
    ultimate::core::Config loaded; REQUIRE(loaded.load(file, &error));
    REQUIRE(loaded.getInt("width", 0) == 1920); REQUIRE(loaded.get("vsync") == "true");
    std::filesystem::remove(file);
}
