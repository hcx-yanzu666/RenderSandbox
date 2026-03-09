cmake_minimum_required(VERSION 3.20)
project(RenderSandbox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RenderSandbox
        src/main.cpp
        src/Shader.cpp
        src/Shader.h
        src/Texture2D.cpp
        src/Texture2D.h
        src/Material.cpp
        src/Material.h
        src/Object.cpp
        src/Object.h
        src/render/Renderer.cpp
        src/render/Renderer.h
        src/render/Light.cpp
        src/render/Light.h
        src/render/Framebuffer.cpp
        src/render/Framebuffer.h
        src/render/PostProcessPass.cpp
        src/render/PostProcessPass.h
        src/Mesh.cpp
        src/Mesh.h
        src/Transform.cpp
        src/Transform.h
        src/Model.cpp
        src/Model.h
)

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)


target_link_libraries(RenderSandbox PRIVATE glfw imgui::imgui OpenGL::GL)
target_link_libraries(RenderSandbox PRIVATE glfw imgui::imgui OpenGL::GL glad::glad)
target_link_libraries(RenderSandbox PRIVATE glm::glm)
target_link_libraries(RenderSandbox PRIVATE assimp::assimp)



if (WIN32)
    target_compile_definitions(RenderSandbox PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# 每次构建后，把 assets 目录同步到可执行文件旁边
add_custom_command(TARGET RenderSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:RenderSandbox>/assets
    COMMENT "Copying assets to build directory"
)
