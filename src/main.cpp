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
#include "InputManager.h"
#include "GrassRenderer.h"
#include "PostProcessor.h"
#include "Model.h" // Lassen wir drin, damit es bereit ist, wenn du es brauchst
#include "WaterPlane.h"

#include <iostream>
#include <thread>
#include <string>
#include <vector>

// --- KONFIGURATION ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

// Globale Settings
bool useNormalMap = true;
bool useARMMap = true;
bool limitFps = true;
int fpsLimit = 60;
bool enableFog = true;
float fogDensity = 0.025f;
glm::vec3 fogColor = glm::vec3(0.5f, 0.6f, 0.7f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main()
{
    // 1. INITIALISIERUNG
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Engine - Base", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // 2. SYSTEME SETUP
    Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
    UIManager ui(window);
    InputManager inputManager(window, camera, ui);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    PostProcessor postEffects(fbW, fbH);

    // 3. SHADER LADEN
    Shader terrainShader("../shaders/terrain.vs.glsl", "../shaders/terrain.fs.glsl");
    Shader waterShader("../shaders/water.vs.glsl", "../shaders/water.fs.glsl");
    // ObjectShader behalten wir geladen, falls du später wieder was einfügen willst
    Shader objectShader("../shaders/object.vs.glsl", "../shaders/object.fs.glsl");

    // 4. LANDSCHAFT & WASSER
    Terrain terrain("../assets/objects/landscape.fbx");

    // Wasser Setup
    WaterPlane waterPlane(200.0f, 200);
    float waterSpeed = 0.15f;
    float waterSteepness = 0.35f;
    float waterWavelength = 1.0f;

    // 5. VEGETATION (GRAS)
    GrassRenderer grassMain(terrain, 40000, "../assets/textures/grass_blade01.png", 1, GRASS);
    grassMain.setColors(glm::vec3(0.34f, 0.40f, 0.05f), glm::vec3(0.27f, 0.31f, 0.07f));

    GrassRenderer grassVar(terrain, 80000, "../assets/textures/grass_blade02.png", 2, SINGLE_BLADE);
    grassVar.setColors(glm::vec3(0.40f, 0.50f, 0.10f), glm::vec3(0.35f, 0.35f, 0.15f));

    GrassRenderer leaves(terrain, 50000, "../assets/textures/leaf01.png", 3, LEAF);
    leaves.setColors(glm::vec3(0.42f, 0.20f, 0.10f), glm::vec3(0.55f, 0.35f, 0.15f));


    // Terrain Shader Config
    terrainShader.use();
    terrainShader.setInt("texGravelDiff", 0); terrainShader.setInt("texGravelNor", 1); terrainShader.setInt("texGravelArm", 2);
    terrainShader.setInt("texPebblesDiff", 3); terrainShader.setInt("texPebblesNor", 4); terrainShader.setInt("texPebblesArm", 5);
    terrainShader.setInt("texRockDiff", 6); terrainShader.setInt("texRockNor", 7); terrainShader.setInt("texRockArm", 8);
    terrainShader.setVec3("lightPos", glm::vec3(50.0f, 100.0f, 50.0f));
    terrainShader.setVec3("lightColor", glm::vec3(1.0f));

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // 6. RENDER LOOP
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.processInput(deltaTime);

        // Framebuffer Resize Check
        int currentW, currentH;
        glfwGetFramebufferSize(window, &currentW, &currentH);
        postEffects.checkResize(currentW, currentH);

        // --- RENDER START ---
        postEffects.beginRender();
        glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (float)currentW / (float)currentH;
        glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), aspect, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = camera.getViewMatrix();
        glm::vec3 sunPos = glm::vec3(50.0f, 100.0f, 50.0f);

        // A) TERRAIN
        terrainShader.use();
        terrainShader.setBool("useNormalMap", useNormalMap);
        terrainShader.setBool("useARMMap", useARMMap);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", glm::mat4(1.0f));
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrain.draw(terrainShader);

        // Texturen resetten
        for(int i = 0; i < 9; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // B) WASSER
        waterShader.use();
        waterShader.setFloat("time", (float)glfwGetTime());
        waterShader.setFloat("speedMult", waterSpeed);
        waterShader.setFloat("steepnessMult", waterSteepness);
        waterShader.setFloat("wavelengthMult", waterWavelength);
        waterShader.setVec3("lightPos", sunPos);
        waterShader.setVec3("lightColor", glm::vec3(1.5f));

        // Wasser positionieren (leicht unter 0, damit Ufer entstehen)
        glm::mat4 waterModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f));
        waterShader.setMat4("model", waterModel);

        waterPlane.draw(waterShader, view, projection, camera.getPosition());

        // C) GRAS (Vegetation)
        glDisable(GL_CULL_FACE);
        grassMain.draw(view, projection);
        grassVar.draw(view, projection);
        leaves.draw(view, projection);
        glEnable(GL_CULL_FACE);

        // --- POST PROCESSING & UI ---
        float effectiveDensity = enableFog ? fogDensity : 0.0f;
        postEffects.endRender(NEAR_PLANE, FAR_PLANE, fogColor, effectiveDensity);

        ui.beginFrame();
        ui.renderUI(camera, useNormalMap, useARMMap, limitFps, fpsLimit, enableFog, fogDensity,
                    waterSpeed, waterSteepness, waterWavelength);
        ui.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (limitFps && fpsLimit > 0) {
            double targetFrameTime = 1.0 / fpsLimit;
            while (glfwGetTime() < currentFrame + targetFrameTime) {
                std::this_thread::yield();
            }
        }
    }

    glfwTerminate();
    return 0;
}