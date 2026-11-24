#include "TestHarness.hpp"
#include <exception>
#include <stdexcept>
namespace test { std::vector<Test>& registry() { static std::vector<Test> tests; return tests; } }
int main() {
    std::size_t passed = 0, failed = 0;
    for (const auto& [name, fn] : test::registry()) {
        try { fn(); ++passed; std::cout << "[PASS] " << name << '\n'; }
        catch (const std::exception& e) { ++failed; std::cerr << "[FAIL] " << name << ": " << e.what() << '\n'; }
        catch (...) { ++failed; std::cerr << "[FAIL] " << name << ": unknown exception\n"; }
    }
    std::cout << "Tests: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
