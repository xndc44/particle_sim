cmake_minimum_required(VERSION 3.28)
project(particle_simulation)

set(CMAKE_CXX_STANDARD 17)

# Include FetchContent module
include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest  # You can specify a tag or branch (e.g., 2.5.1 or latest)
)
FetchContent_MakeAvailable(glfw)

# Fetch ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master  # Or a specific commit/tag/branch
)
FetchContent_MakeAvailable(imgui)

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add the executable
add_executable(particle_simulation main.cpp)

# Link GLFW and OpenGL
target_link_libraries(particle_simulation PRIVATE glfw OpenGL::GL)

# Include necessary directories
target_include_directories(particle_simulation PRIVATE
        ${glfw_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}  # Add ImGui headers
        ${imgui_SOURCE_DIR}/backends  # Add the backends directory to the include path
)

# Add ImGui source files to the project (no need to link -limgui)
target_sources(particle_simulation PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)