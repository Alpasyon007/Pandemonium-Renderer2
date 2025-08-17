cmake_minimum_required(VERSION 3.20)
project(RayTracerVulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(RTVK_ENABLE_VALIDATION "Enable Vulkan validation layers" ON)
option(RTVK_ENABLE_LOG "Enable logging integration" ON)

# Logging library integration (default: sibling folder 'Pandemonium-Log').
# Override with -DRTVK_LOGGER_PATH=abs/or/relative/path if layout differs.
if (NOT RTVK_ENABLE_LOG)
    message(FATAL_ERROR "Logging must remain enabled (fallback removed). Set RTVK_ENABLE_LOG=ON.")
endif()

if (NOT DEFINED RTVK_LOGGER_PATH)
    # Pandemonium-Log now lives inside this repository under common/Pandemonium-log
    set(RTVK_LOGGER_PATH "${CMAKE_CURRENT_LIST_DIR}/common/Pandemonium-log")
endif()

if (NOT EXISTS "${RTVK_LOGGER_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Pandemonium-Log not found at ${RTVK_LOGGER_PATH}. Set RTVK_LOGGER_PATH to correct path.")
endif()

message(STATUS "Including Pandemonium-Log from ${RTVK_LOGGER_PATH}")
add_subdirectory(${RTVK_LOGGER_PATH} extern/pandemonium_log)

# Detect logger target or require manual specification
if (NOT RTVK_LOGGER_TARGET)
    if (TARGET pandemonium_log)
        set(RTVK_LOGGER_TARGET pandemonium_log)
    elseif (TARGET pandemonium-log)
        set(RTVK_LOGGER_TARGET pandemonium-log)
    elseif (TARGET PandemoniumLog)
        set(RTVK_LOGGER_TARGET PandemoniumLog)
    elseif (TARGET logger)
        set(RTVK_LOGGER_TARGET logger)
    endif()
endif()

if (NOT RTVK_LOGGER_TARGET)
    message(FATAL_ERROR "Unable to detect Pandemonium-Log target. Provide -DRTVK_LOGGER_TARGET=<target-name> when configuring.")
endif()

find_package(Vulkan QUIET)

# Manual fallback: use VULKAN_SDK env var if FindVulkan didn't succeed
if (NOT Vulkan_FOUND)
    message(STATUS "FindVulkan failed; attempting manual discovery via VULKAN_SDK env var")
    if (DEFINED ENV{VULKAN_SDK})
        set(_VSDK $ENV{VULKAN_SDK})
        # Normalize backslashes to forward slashes for CMake
        file(TO_CMAKE_PATH "${_VSDK}" _VSDK_NORM)
        set(Vulkan_INCLUDE_DIR "${_VSDK_NORM}/Include")
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(Vulkan_LIBRARY "${_VSDK_NORM}/Lib/vulkan-1.lib")
        else()
            set(Vulkan_LIBRARY "${_VSDK_NORM}/Lib32/vulkan-1.lib")
        endif()
        if (EXISTS "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.h" AND EXISTS "${Vulkan_LIBRARY}")
            add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
            set_target_properties(Vulkan::Vulkan PROPERTIES IMPORTED_LOCATION "${Vulkan_LIBRARY}")
            set(Vulkan_FOUND TRUE)
            message(STATUS "Vulkan SDK located at ${_VSDK_NORM}")
            # Set GLSLC executable if available
            if (EXISTS "${_VSDK_NORM}/Bin/glslc.exe")
                set(Vulkan_GLSLC_EXECUTABLE "${_VSDK_NORM}/Bin/glslc.exe")
            elseif(EXISTS "${_VSDK_NORM}/Bin/glslc")
                set(Vulkan_GLSLC_EXECUTABLE "${_VSDK_NORM}/Bin/glslc")
            endif()
        endif()
    endif()
endif()

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan SDK. Install from https://vulkan.lunarg.com/ and ensure VULKAN_SDK env var is set (new terminal) or specify -DVulkan_INCLUDE_DIR and -DVulkan_LIBRARY.")
endif()

# GLFW for window + surface
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(GLFW3 glfw3)
endif()

if (NOT GLFW3_FOUND)
    # Fallback: fetch GLFW
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
    set(GLFW_LIB glfw)
else()
    set(GLFW_LIB ${GLFW3_LINK_LIBRARIES})
endif()

add_library(rtvk STATIC
    src/Renderer.cpp
)

target_include_directories(rtvk PUBLIC include)

target_link_libraries(rtvk PUBLIC Vulkan::Vulkan ${GLFW_LIB})

target_link_libraries(rtvk PUBLIC ${RTVK_LOGGER_TARGET})
target_compile_definitions(rtvk PUBLIC RTVK_HAVE_LOGGER=1)

# Common include directory guesses for Pandemonium-Log (only added if they exist)
foreach(dir IN ITEMS Include headers)
    if (EXISTS "${RTVK_LOGGER_PATH}/${dir}")
        # Prepend to include path list to prioritize logger headers
        target_include_directories(rtvk PUBLIC "${RTVK_LOGGER_PATH}/${dir}")
    endif()
endforeach()

if (MSVC)
    target_compile_options(rtvk PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(rtvk PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(rtvk_demo src/main.cpp)

target_link_libraries(rtvk_demo PRIVATE rtvk)

# Shader compilation (GLSL -> SPIR-V)
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS shaders/*.vert shaders/*.frag)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
foreach(shader ${SHADER_SOURCES})
    get_filename_component(fname ${shader} NAME)
    set(spv ${CMAKE_CURRENT_BINARY_DIR}/shaders/${fname}.spv)
    if (Vulkan_GLSLC_EXECUTABLE)
        add_custom_command(
            OUTPUT ${spv}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${spv} ${shader}
            DEPENDS ${shader}
            COMMENT "Compiling shader ${fname}"
            VERBATIM
        )
    else()
        # Fallback: emit a file generation step that fails with a helpful message
        add_custom_command(
            OUTPUT ${spv}
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: glslc not found. Ensure Vulkan SDK Bin directory is in PATH or VULKAN_SDK set." && ${CMAKE_COMMAND} -E false
            DEPENDS ${shader}
            COMMENT "glslc missing for shader ${fname}"
            VERBATIM
        )
    endif()
    list(APPEND SPV_SHADERS ${spv})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(rtvk_demo Shaders)
add_dependencies(rtvk Shaders)

if (RTVK_ENABLE_VALIDATION)
    target_compile_definitions(rtvk PUBLIC RTVK_ENABLE_VALIDATION=1)
endif()

# Provide absolute shader directory for runtime loading
file(TO_CMAKE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shaders" RTVK_SHADER_DIR_CANON)
target_compile_definitions(rtvk PUBLIC RTVK_SHADER_DIR="${RTVK_SHADER_DIR_CANON}")

source_group("Shaders" FILES ${SHADER_SOURCES})
