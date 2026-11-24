# Architecture overview

## Dependency direction

`ultimate_core` contains reusable runtime code and has no dependency on Vulkan or the asset downloader's network backend. `ultimate_assets` builds on core and uses `std::filesystem` plus optional libcurl. `ultimate_vulkan` is a separate optional target and depends on the Vulkan SDK, GLFW and compiled SPIR-V shaders. The client executable links the core target and links the Vulkan target only when all optional dependencies are available.

## Frame responsibilities

The current Vulkan frame path is:

1. Wait for the current frame fence.
2. Acquire a swapchain image.
3. Reset the fence and command buffer.
4. Record the render pass, fullscreen triangle pipeline and draw.
5. Submit, waiting on the acquire semaphore and signaling the render-finished semaphore.
6. Present the acquired image and recreate the swapchain on resize/out-of-date results.

The fullscreen triangle is a renderer bring-up pass, not the eventual world renderer. Mesh vertex/index buffers, descriptor sets, material binding, depth buffering, GPU scene data, lighting, shadow maps and post-processing are future milestones.

## Core modules

- `Clock`: monotonic frame timing.
- `EventBus`: type-indexed synchronous event subscriptions.
- `JobSystem`: worker queue and `std::future` task submission.
- `MemoryArena`: caller-owned fixed-capacity aligned bump allocator.
- `Registry`: runtime component pools keyed by C++ type.
- `CollisionWorld`: static AABB set.
- `CharacterMotor`: axis-separated movement against static AABBs.
- `Camera` and `FirstPersonController`: yaw/pitch camera and normalized horizontal movement.
- `CaptureZone`: timed nonviolent objective state.
- `AudioMixer`: spatial attenuation/panning calculation; output device integration is not implemented.

## Asset data flow

The intended flow is:

`verified source metadata -> license/category gate -> HTTPS transfer -> SHA-256 -> local file inspection -> manifest update -> validation -> runtime resource loader`

The current asset pipeline implements the first five steps with manifest-driven inputs. It intentionally does not scrape sites or infer licenses. Asset page review remains a required human check because license declarations, creator information and redistribution terms may vary by asset.
