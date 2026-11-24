# ULTIMATE FPS — custom C++20 / Vulkan engine foundation

This repository is an initial, compilable engine foundation, not a finished AAA game and not a claim of parity with Counter-Strike 2. It contains working engine subsystems, a CMake build, tests, a Vulkan swapchain renderer (when the SDK dependencies are installed), and an offline-capable asset pipeline.

## Current prototype direction

The current playable foundation is a nonviolent first-person exploration prototype with movement, collision and timed survey/capture zones. It deliberately excludes firearms, weapon models, weapon sounds, ballistic simulation and combat replication. The codebase is organized so general-purpose engine systems can grow without introducing real-world weapon simulation.

## Implemented

- C++20 project, CMake and CTest.
- Core logging, steady clock, key/value configuration, event bus, aligned memory arena and thread-pool job system.
- Math vectors, entity registry/component pools, AABB collision and axis-separated character movement.
- Platform-neutral input state, first-person camera/controller, player and capture-zone objective.
- Spatial audio cue gain/pan calculation (not yet a device playback backend).
- Optional Vulkan 1.1 renderer using GLFW, a swapchain, image views, render pass, graphics pipeline, framebuffers, command buffers and per-frame synchronization.
- GLSL shaders compiled to SPIR-V by `glslc` when the Vulkan backend is enabled.
- Asset manifest CSV reader/writer, CC license allowlist, HTTPS downloader (when libcurl is available), SHA-256 integrity hashing, file-signature inspection, importer and validator.
- Tests for core, ECS, physics and asset-pipeline behavior.

## Build on Linux

Requirements for the portable/headless target: CMake 3.24+, a C++20 compiler and Threads.

```bash
cmake -S . -B build -G Ninja -DULTIMATE_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/ultimate_fps --headless
./build/assetctl list ASSET_MANIFEST.csv
./build/assetctl validate ASSET_MANIFEST.csv ASSETS
```

The Vulkan desktop backend additionally requires the Vulkan SDK (headers and loader), GLFW 3 and `glslc` in `PATH` or exposed by the Vulkan CMake package. It is optional so CI or machines without a display SDK can still build/test core modules.

HTTPS asset downloading additionally requires libcurl development files. The project detects it with `find_package(CURL)`; otherwise the importer/catalog/validator still build and the downloader reports the missing optional dependency.

## Build on Windows

Install Visual Studio 2022 with the C++ workload, CMake, Ninja, Vulkan SDK and GLFW 3. Ensure `VULKAN_SDK` is set and that `glslc.exe` is reachable. Then run in a Developer PowerShell:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DULTIMATE_BUILD_VULKAN=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The initial Windows target is a runtime executable. Packaging, signing, installer generation and a production content bundle are not implemented yet.

## Asset workflow

`ASSET_MANIFEST.csv` is the authoritative inventory of files that are actually present and imported. It currently contains only the header because outbound DNS was unavailable from the build container during creation; no third-party binary files were downloaded, and the repository does not pretend otherwise.

`ASSET_CANDIDATES.csv` contains a small set of real environment-texture pages whose authors and CC0 declarations were checked against their official pages. These are candidates, not imported assets. `ASSET_SOURCES.md` records source/license facts and the current acquisition limitation.

The downloader accepts only HTTPS URLs from the project's approved providers and metadata declaring an allowlisted license. It limits the import categories to environment, material, architecture, props, vegetation, sky, ambient/interface audio and map content. A provider's global license statement does not replace checking the individual asset page and included license file. SHA-256 is calculated after a completed download. Files are written to a `.partial` path and then renamed only after a successful transfer.

```bash
./build/assetctl download ASSET_MANIFEST.csv ASSETS
./build/assetctl validate ASSETS/ASSET_MANIFEST.csv ASSETS
```

The manifest `SourceUrl` must be a direct file URL on an approved host, not an HTML asset page. Never enter an asset in `ASSET_MANIFEST.csv` until the actual bytes exist locally and the individual license/author metadata has been verified.

## Repository map

- `ENGINE/CORE` — runtime fundamentals.
- `ENGINE/MATH`, `ENGINE/ECS`, `ENGINE/PHYSICS` — reusable systems.
- `ENGINE/GRAPHICS/Vulkan` and `ENGINE/GRAPHICS/Shaders` — optional Vulkan backend.
- `ENGINE/INPUT`, `ENGINE/SCENE`, `ENGINE/AUDIO` — runtime-facing systems.
- `GAME` — first-person traversal and nonviolent objectives.
- `ASSET_PIPELINE` — download, import, validate, metadata and catalog code.
- `ASSETS`, `MAPS`, `CONFIG` — content roots and initial scene configuration.
- `TESTS` — automated test harness.
- `DOCUMENTATION` — architecture, build and pipeline notes.

## Known limitations

- This is an engine foundation with a fullscreen shader pass; it does not yet render imported meshes or PBR materials.
- No asset bytes could be acquired during this environment run because outbound DNS resolution failed. Candidate rows are clearly separated from the actual manifest.
- No production audio output backend, animation runtime, navmesh, matchmaking, replication, editor UI, asset thumbnailer or Windows packaging pipeline is present yet.
- The current collision model is static AABB with axis-separated movement, not a general rigid-body solver.
- No Windows EXE was produced from this Linux environment.

See `DOCUMENTATION/ROADMAP.md` for the next engineering milestones. Use `cmake --preset linux-ninja` or `cmake --preset windows-vs2022` for the supplied configure presets; PowerShell/bash helper scripts are in `SCRIPTS/`.
