$ErrorActionPreference = 'Stop'

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'CMake was not found. Install CMake and Visual Studio 2022 C++ tools.'
}
if (-not $env:VULKAN_SDK) {
    Write-Warning 'VULKAN_SDK is not set. The headless/core targets may build, but the Vulkan renderer will be disabled.'
}

cmake --preset windows-vs2022
cmake --build --preset windows-release
ctest --preset windows-release-tests
