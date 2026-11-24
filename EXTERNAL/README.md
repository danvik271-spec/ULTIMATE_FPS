# External SDK/dependency boundary

Third-party libraries are not vendored into this repository. The build detects system-installed Vulkan SDK, GLFW and libcurl dependencies. This keeps upstream code and licenses separate and avoids silently copying unreviewed dependencies into the engine source tree.
