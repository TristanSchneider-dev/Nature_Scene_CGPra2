cmake_minimum_required(VERSION 3.20)
project(TerrainOpenGL)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(stb REQUIRED)

add_executable(TerrainOpenGL
        src/main.cpp
        src/Camera.cpp
        src/Camera.h
        src/Shader.h
        src/UIManager.cpp
        src/UIManager.h
        src/Terrain.h
        src/Terrain.cpp
        src/InputManager.h
        src/InputManager.cpp
        libs/glad/src/glad.c
        src/PostProcessor.h
        src/Model.h
        src/Model.cpp
        src/WaterPlane.h
        src/SceneManager.h
        src/SceneManager.cpp
        src/ImGuizmo.cpp
        src/Skybox.h
        src/Skybox.cpp
        src/GrassSystem.h
        src/GrassSystem.cpp
        src/ForestSystem.cpp
 "src/ShadowMapper.h")

target_include_directories(TerrainOpenGL PRIVATE
        src
        libs/glad/include
        ${STB_INCLUDE_DIR}
)

target_link_libraries(TerrainOpenGL PRIVATE
        glfw
        glm::glm
        assimp::assimp
        OpenGL::GL
        imgui::imgui
)