#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
namespace test {
using Test = std::pair<std::string, std::function<void()>>;
std::vector<Test>& registry();
struct Register { Register(std::string name, std::function<void()> fn) { registry().emplace_back(std::move(name), std::move(fn)); } };
inline void require(bool condition, const char* expression, const char* file, int line) {
    if (!condition) throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + " assertion failed: " + expression);
}
}
#define TEST(name) static void name(); static test::Register reg_##name(#name, name); static void name()
#define REQUIRE(expr) test::require(static_cast<bool>(expr), #expr, __FILE__, __LINE__)
