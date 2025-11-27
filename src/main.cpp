#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Shader.h"
#include "Camera.h"
#include "UIManager.h"
#include "Terrain.h"
#include "InputManager.h" // Neu

#include <iostream>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// UI Toggles (Lokal in Main verwaltet und an UI übergeben)
bool useNormalMap = true;
bool useARMMap = true;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain OpenGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    // Initialisierung der Subsysteme
    Camera camera(glm::vec3(0.0f, 10.0f, 30.0f));
    UIManager ui(window);
    InputManager inputManager(window, camera, ui); // Übernimmt ab hier alle Events

    Shader terrainShader("../shaders/terrain.vs.glsl", "../shaders/terrain.fs.glsl");
    Terrain terrain("../assets/objects/landscape.fbx");

    // Shader Setup (Pebbles/Gravel sind im Terrain bereits getauscht)
    terrainShader.use();
    terrainShader.setInt("texGravelDiff", 0);
    terrainShader.setInt("texGravelNor",  1);
    terrainShader.setInt("texGravelArm",  2);
    terrainShader.setInt("texPebblesDiff", 3);
    terrainShader.setInt("texPebblesNor",  4);
    terrainShader.setInt("texPebblesArm",  5);
    terrainShader.setInt("texRockDiff", 6);
    terrainShader.setInt("texRockNor",  7);
    terrainShader.setInt("texRockArm",  8);

    terrainShader.setVec3("lightColor", glm::vec3(1.0f));
    terrainShader.setVec3("lightPos", glm::vec3(0.0f, 100.0f, 100.0f));

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input Handling
        inputManager.processInput(deltaTime);

        ui.beginFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainShader.use();
        terrainShader.setBool("useNormalMap", useNormalMap);
        terrainShader.setBool("useARMMap", useARMMap);

        glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", camera.getViewMatrix());
        terrainShader.setMat4("model", glm::mat4(1.0f));
        terrainShader.setVec3("viewPos", camera.getPosition());

        terrain.draw(terrainShader);

        ui.renderUI(camera, useNormalMap, useARMMap);
        ui.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}