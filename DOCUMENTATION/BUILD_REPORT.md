# Build verification report

Environment used for this repository creation:
- Linux x86_64
- GCC 14.2.0
- CMake 3.31.6
- Ninja
- libcurl 8.10.1 (pkg-config target)

Verified:
- Debug configure/build: passed.
- Debug CTest: passed.
- Release configure/build: passed.
- Release CTest: passed.
- `ultimate_fps --headless`: launched and initialized core runtime.
- `assetctl list ASSET_MANIFEST.csv`: parsed the manifest (0 imported assets).
- `assetctl validate ASSET_MANIFEST.csv ASSETS`: 0 checked, 0 invalid, because no binary assets were downloadable in the current environment.

Not verified in this environment:
- Vulkan backend compilation/execution: the machine did not have Vulkan SDK headers or GLFW installed; CMake correctly disabled the optional target.
- Windows build/EXE: this environment is Linux.
- External asset downloads: outbound DNS resolution failed for `api.polyhaven.com`; no false downloaded-file entries were created.

The renderer source, GLSL shaders, downloader, importer and validators are present, but their unverified paths must be built/run on a machine with the stated dependencies before being treated as production-ready.
