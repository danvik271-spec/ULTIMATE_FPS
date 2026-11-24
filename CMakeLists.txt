cmake_minimum_required(VERSION 3.24)
project(ULTIMATE_FPS VERSION 0.1.0 LANGUAGES CXX)

option(ULTIMATE_BUILD_TESTS "Build unit and integration tests" ON)
option(ULTIMATE_BUILD_VULKAN "Build the optional Vulkan desktop renderer when dependencies are available" ON)
option(ULTIMATE_ENABLE_CURL "Enable HTTPS downloads in the asset pipeline when libcurl is available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
find_package(Threads REQUIRED)

add_library(ultimate_core
    ENGINE/CORE/src/Log.cpp
    ENGINE/CORE/src/Clock.cpp
    ENGINE/CORE/src/Config.cpp
    ENGINE/CORE/src/JobSystem.cpp
    ENGINE/PHYSICS/src/CollisionWorld.cpp
    ENGINE/PHYSICS/src/CharacterMotor.cpp
    ENGINE/INPUT/src/InputState.cpp
    ENGINE/SCENE/src/Camera.cpp
    ENGINE/AUDIO/src/AudioMixer.cpp
    GAME/PLAYER/src/Player.cpp
    GAME/MOVEMENT/src/FirstPersonController.cpp
    GAME/OBJECTIVES/src/CaptureZone.cpp
)
target_include_directories(ultimate_core PUBLIC
    ENGINE/CORE/include ENGINE/MATH/include ENGINE/ECS/include
    ENGINE/PHYSICS/include ENGINE/INPUT/include ENGINE/SCENE/include
    ENGINE/AUDIO/include GAME/PLAYER/include GAME/MOVEMENT/include
    GAME/OBJECTIVES/include
)
target_link_libraries(ultimate_core PUBLIC Threads::Threads)
target_compile_features(ultimate_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(ultimate_core PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(ultimate_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

# Asset pipeline: networking is optional; offline import, catalog, license and
# integrity validation remain available without libcurl.
add_library(ultimate_assets
    ASSET_PIPELINE/src/AssetCatalog.cpp
    ASSET_PIPELINE/src/AssetDownloader.cpp
    ASSET_PIPELINE/src/AssetImporter.cpp
    ASSET_PIPELINE/src/AssetMetadata.cpp
    ASSET_PIPELINE/src/AssetValidator.cpp
    ASSET_PIPELINE/src/LicenseChecker.cpp
    ASSET_PIPELINE/src/Sha256.cpp
    ASSET_PIPELINE/src/TextureProcessor.cpp
    ASSET_PIPELINE/src/AudioProcessor.cpp
    ASSET_PIPELINE/src/ModelProcessor.cpp
)
target_include_directories(ultimate_assets PUBLIC ASSET_PIPELINE/include ENGINE/CORE/include)
target_link_libraries(ultimate_assets PUBLIC ultimate_core)

if(ULTIMATE_ENABLE_CURL)
    # Prefer pkg-config: some environments expose a static libcurl CMake target
    # without its transitive TLS/compression dependencies.
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBCURL QUIET IMPORTED_TARGET libcurl)
    endif()
    if(TARGET PkgConfig::LIBCURL)
        target_link_libraries(ultimate_assets PRIVATE PkgConfig::LIBCURL)
        target_compile_definitions(ultimate_assets PRIVATE ULTIMATE_HAS_CURL=1)
        message(STATUS "Asset HTTPS downloads: enabled (pkg-config libcurl ${LIBCURL_VERSION})")
    else()
        find_package(CURL QUIET)
        if(CURL_FOUND)
            target_link_libraries(ultimate_assets PRIVATE CURL::libcurl)
            target_compile_definitions(ultimate_assets PRIVATE ULTIMATE_HAS_CURL=1)
            message(STATUS "Asset HTTPS downloads: enabled (CMake CURL package)")
        else()
            message(STATUS "Asset HTTPS downloads: disabled (libcurl not found; validation/import still work)")
        endif()
    endif()
endif()

add_executable(ultimate_fps CLIENT/src/main.cpp)
target_link_libraries(ultimate_fps PRIVATE ultimate_core)

add_executable(assetctl ASSET_PIPELINE/tools/assetctl.cpp)
target_link_libraries(assetctl PRIVATE ultimate_assets)

# Vulkan is an optional platform backend so headless CI builds can compile and
# test core/asset systems even when the Vulkan SDK and GLFW are not installed.
if(ULTIMATE_BUILD_VULKAN)
    find_package(Vulkan QUIET)
    find_package(glfw3 CONFIG QUIET)
    if(Vulkan_FOUND AND (TARGET glfw OR TARGET glfw3::glfw))
        if(TARGET glfw)
            set(ULTIMATE_GLFW_TARGET glfw)
        else()
            set(ULTIMATE_GLFW_TARGET glfw3::glfw)
        endif()
        if(NOT Vulkan_GLSLC_EXECUTABLE)
            find_program(Vulkan_GLSLC_EXECUTABLE NAMES glslc)
        endif()
        if(Vulkan_GLSLC_EXECUTABLE)
            set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
            file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")
            foreach(SHADER scene.vert scene.frag)
                add_custom_command(
                    OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER}.spv"
                    COMMAND "${Vulkan_GLSLC_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/ENGINE/GRAPHICS/Shaders/${SHADER}" -o "${SHADER_OUTPUT_DIR}/${SHADER}.spv"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ENGINE/GRAPHICS/Shaders/${SHADER}"
                    VERBATIM
                )
                list(APPEND ULTIMATE_SPIRV_FILES "${SHADER_OUTPUT_DIR}/${SHADER}.spv")
            endforeach()
            add_custom_target(ultimate_shaders DEPENDS ${ULTIMATE_SPIRV_FILES})
            add_library(ultimate_vulkan ENGINE/GRAPHICS/Vulkan/src/VulkanRenderer.cpp)
            target_include_directories(ultimate_vulkan PUBLIC ENGINE/GRAPHICS/Vulkan/include)
            target_link_libraries(ultimate_vulkan PUBLIC Vulkan::Vulkan ${ULTIMATE_GLFW_TARGET})
            target_compile_definitions(ultimate_vulkan PUBLIC ULTIMATE_HAS_VULKAN=1 PRIVATE ULTIMATE_SHADER_DIR="${SHADER_OUTPUT_DIR}")
            add_dependencies(ultimate_vulkan ultimate_shaders)
            target_link_libraries(ultimate_fps PRIVATE ultimate_vulkan)
            target_compile_definitions(ultimate_fps PRIVATE ULTIMATE_HAS_VULKAN=1)
            message(STATUS "Vulkan renderer: enabled")
        else()
            message(WARNING "Vulkan/GLFW found but glslc missing: Vulkan renderer disabled")
        endif()
    else()
        message(STATUS "Vulkan renderer: disabled (requires Vulkan SDK and GLFW)")
    endif()
endif()

if(ULTIMATE_BUILD_TESTS)
    add_executable(ultimate_tests
        TESTS/TestMain.cpp
        TESTS/CoreTests.cpp
        TESTS/PhysicsTests.cpp
        TESTS/AssetTests.cpp
        TESTS/EcsTests.cpp
    )
    target_link_libraries(ultimate_tests PRIVATE ultimate_core ultimate_assets)
    add_test(NAME ultimate_tests COMMAND ultimate_tests)
endif()

install(TARGETS ultimate_fps assetctl RUNTIME DESTINATION bin)
