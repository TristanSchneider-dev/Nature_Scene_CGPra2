cmake_minimum_required(VERSION 3.20)
project(TerrainOpenGL)

set(CMAKE_CXX_STANDARD 17)

# Pakete finden
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
# HINWEIS: Falls später Linker-Fehler kommen, musst du in vcpkg
# evtl. die Features installieren: "vcpkg install imgui[glfw-binding,opengl3-binding]"

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

        # Glad C-Datei (lokal)
        libs/glad/src/glad.c
)

# Include Directories
target_include_directories(TerrainOpenGL PRIVATE
        src
        libs/glad/include
)

# Libraries linken
target_link_libraries(TerrainOpenGL PRIVATE
        glfw
        glm::glm
        assimp::assimp
        OpenGL::GL
        imgui::imgui
)