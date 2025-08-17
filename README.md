# Vulkan Renderer Skeleton

This is a minimal Vulkan renderer that opens a window (GLFW) and draws a rotating (not yet) static triangle. It sets up:

- Vulkan instance (with optional validation layers)
- Debug messenger
- Surface (Win32 via GLFW)
- Physical & logical device, graphics + present queues
- Swapchain + image views
- Render pass
- Graphics pipeline (fixed-function + simple shaders)
- Framebuffers
- Command pool & buffers
- Synchronization primitives (semaphores & fences)

## Building (Windows)

### Prerequisites
- Install the official Vulkan SDK: https://vulkan.lunarg.com/ (ensures `glslc` is in PATH)
- CMake >= 3.20
- A C++20 capable compiler (MSVC / clang / mingw)
- Python (optional) if you script assets

### Configure & Build
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

Or with Ninja:
```
cmake -S . -B build -G Ninja
cmake --build build
```

### Run
```
build/Debug/rtvk_demo.exe
```
(or corresponding path for your generator/config.)

## Shader Editing
Edit GLSL in `shaders/triangle.vert` and `shaders/triangle.frag`. They are compiled automatically by CMake using `glslc` (from Vulkan SDK) into SPIR-V under `build/shaders/*.spv`.

## Next Steps / Ideas
- Add depth buffer & recreate pipeline with depth testing
- Add vertex buffers (currently uses `gl_VertexIndex`)
- Descriptor sets & uniform buffers (MVP matrices, etc.)
- Proper resource cleanup on swapchain recreation for command buffers
- FPS counter & frame time measurement
- Ray tracing extensions (VK_KHR_acceleration_structure, VK_KHR_ray_tracing_pipeline)

## Ray Tracing Roadmap (High-Level)
1. Create acceleration structures (BLAS/TLAS)
2. Ray tracing pipeline with raygen/miss/closest hit shaders
3. Shader binding tables
4. Descriptor sets for AS, output image, materials
5. Dispatch rays via `vkCmdTraceRaysKHR`
6. Composite result to swapchain

This scaffold keeps the core minimal so you can iterate quickly.
