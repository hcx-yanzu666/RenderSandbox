cmake_minimum_required(VERSION 3.20)
project(RenderSandbox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RenderSandbox
        src/main.cpp
)

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(RenderSandbox PRIVATE glfw imgui::imgui OpenGL::GL)

if (WIN32)
    target_compile_definitions(RenderSandbox PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
