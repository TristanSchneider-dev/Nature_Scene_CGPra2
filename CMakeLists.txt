cmake_minimum_required(VERSION 3.16)
project(OpenGLTemplate VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

add_library(glad "${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/libs/glad/include")

# --- Executable ---

add_executable(${PROJECT_NAME} src/main.cpp src/Shader.h src/Camera.cpp
        src/Camera.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw glm::glm)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/)