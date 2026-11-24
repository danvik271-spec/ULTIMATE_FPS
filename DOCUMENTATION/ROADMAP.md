# Engineering roadmap

## Milestone 0 — foundation (current)

- CMake/C++20 layout and test runner.
- Runtime core, ECS, math, static AABB collision and first-person traversal.
- Vulkan swapchain and fullscreen pipeline bring-up.
- Manifest, license/category checks, HTTPS downloader, importer and SHA-256 validation.

## Milestone 1 — scene rendering

- Vertex/index buffers, GPU memory allocation, descriptor allocator and staging uploads.
- Depth attachment, camera uniform buffers, mesh submission and material descriptors.
- glTF scene import with validated external dependencies.
- PBR BRDF implementation, environment lighting and image-based lighting.
- Frustum culling and basic batching.

## Milestone 2 — content runtime

- Texture format conversion and mip generation.
- Texture/mesh resource cache, asynchronous streaming and reference tracking.
- Scene serialization versioning, prefabs and reloadable configuration.
- Audio device output backend and streaming decoder.
- Animation clips/skeletons for non-combat characters and environment interactions.

## Milestone 3 — tooling and world systems

- Native scene editor with hierarchy, inspector and viewport.
- Asset browser, material preview and content validation UI.
- Navigation mesh generation and simple non-hostile agents for exploration/tutorial duties.
- Profile markers, GPU timestamps, crash dumps and automated performance captures.

## Milestone 4 — production hardening

- Deterministic simulation tests, fuzzing for asset parsers and malformed-file tests.
- Multiplayer only for safe cooperative objectives, with authentication, snapshot interpolation and server authority.
- Windows packaging, logs/config deployment, crash handling and clean-machine smoke tests.
- Per-platform performance budgets and regression benchmarks.

Each milestone must be implemented and tested before adding large amounts of content. Asset count is not a quality metric; verified provenance, runtime integration and repeatable builds are.
