#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"

#include "Shader.h"
#include "Camera.h"
#include "UIManager.h"
#include "Terrain.h"
#include "InputManager.h"
#include "PostProcessor.h"
#include "WaterPlane.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "GrassSystem.h"

#include <iostream>
#include <vector>
#include <string>
#include <thread>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

bool useNormalMap = true;
bool useARMMap = true;
bool limitFps = true;
int fpsLimit = 120;
bool enableFog = true;
float fogDensity = 0.025f;
glm::vec3 fogColor = glm::vec3(0.5f, 0.6f, 0.7f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // WICHTIG FÜR WEICHE KANTEN (COMIC-FIX TEIL 1)
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Forest Kit - Final", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // WICHTIG FÜR WEICHE KANTEN (COMIC-FIX TEIL 2)
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
    UIManager ui(window);
    SceneManager sceneManager;
    //DEBUG NICHT VEGESSEN
    //sceneManager.loadAssetsFromFolder("../assets/objects");

    InputManager inputManager(window, camera, ui, sceneManager);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    PostProcessor postEffects(fbW, fbH);

    Shader terrainShader("../shaders/terrain.vs.glsl", "../shaders/terrain.fs.glsl");
    Shader objectShader("../shaders/object.vs.glsl", "../shaders/object.fs.glsl");
    Shader waterShader("../shaders/water.vs.glsl", "../shaders/water.fs.glsl");

    Terrain terrain("../assets/terrain/landscape.glb");
    WaterPlane waterPlane(200.0f, 200);

    std::vector<std::string> faces = {
        "../assets/skybox/right.png", "../assets/skybox/left.png",
        "../assets/skybox/top.png",   "../assets/skybox/bottom.png",
        "../assets/skybox/front.png", "../assets/skybox/back.png"
    };
    Skybox skybox(faces);

    // --- GRAS SYSTEM (GRID + HIGH DENSITY) ---
    GrassSystem grassSystem;

    // Grid bauen
    grassSystem.initTerrainData(terrain.getVertices(), terrain.getIndices(), 60.0f);

    std::string grassPath = "../assets/grass/";
    float spread = 190.0f;

    // === GRAS VARIANTEN ===

    // A) KLEIN GRAS 1-6: Basis-Dichte - AUFRECHT
    for (int i = 1; i <= 6; i++) {
        std::string num = (i < 10 ? "0" : "") + std::to_string(i);
        grassSystem.addGrassType(grassPath + "grass_" + num + ".png", 800000, spread, 0.15f, false);
    }

    // B) GROSS GRAS 7-10: Höher und dichter - AUFRECHT
    for (int i = 7; i <= 10; i++) {
        std::string num = (i < 10 ? "0" : "") + std::to_string(i);
        grassSystem.addGrassType(grassPath + "grass_" + num + ".png", 8000, spread, 0.2f, false);
    }

    // C) KLEIN GRAS 11-12: Zusätzliche Basis-Dichte - AUFRECHT
    for (int i = 11; i <= 12; i++) {
        std::string num = std::to_string(i);
        grassSystem.addGrassType(grassPath + "grass_" + num + ".png", 1000, spread, 0.2f, false);
    }

    // D) GROSS GRAS 13: Akzent - AUFRECHT
    grassSystem.addGrassType(grassPath + "grass_13.png", 8000, spread, 0.2f, false);

    // === LEAF VARIANTEN ===

    // E) LEAF 1: Stehend (z.B. Farn/Pflanze)
    grassSystem.addGrassType(grassPath + "leaf_01.png", 1000, spread, 0.5f, false);

    // F) LEAFS 2-10: Liegende Blätter (Bodendecker)
    for (int i = 2; i <= 10; i++) {
        std::string num = (i < 10 ? "0" : "") + std::to_string(i);
        int amount = (i <= 5) ? 15000 : 10000; // Mehr von den ersten
        float scale = (i <= 5) ? 0.15f : 0.15f;
        grassSystem.addGrassType(grassPath + "leaf_" + num + ".png", amount, spread, scale, true);
    }

    // ----------------------------------------

    terrainShader.use();
    terrainShader.setInt("pebblesAlbedo", 0); terrainShader.setInt("pebblesNormal", 1); terrainShader.setInt("pebblesARM", 2);
    terrainShader.setInt("groundAlbedo", 3);  terrainShader.setInt("groundNormal", 4);  terrainShader.setInt("groundARM", 5);
    terrainShader.setInt("rockAlbedo", 6);    terrainShader.setInt("rockNormal", 7);    terrainShader.setInt("rockARM", 8);
    terrainShader.setFloat("tiling", 60.0f);

    glm::vec3 sunPos = glm::vec3(50.0f, 100.0f, 50.0f);
    glm::vec3 sunColor = glm::vec3(1.0f);

    auto setLight = [&](Shader& s) {
        s.use();
        s.setVec3("lightPos", sunPos);
        s.setVec3("lightColor", sunColor);
    };
    setLight(terrainShader);
    setLight(objectShader);
    setLight(waterShader);

    double lastFrame = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        inputManager.processInput(deltaTime);

        int currentW, currentH;
        glfwGetFramebufferSize(window, &currentW, &currentH);
        if (currentW == 0 || currentH == 0) { glfwWaitEvents(); continue; }
        postEffects.checkResize(currentW, currentH);

        postEffects.beginRender();
        glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (float)currentW / (float)currentH;
        glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), aspect, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = camera.getViewMatrix();

        terrainShader.use();
        terrainShader.setBool("useNormalMap", useNormalMap);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(60.0f)));
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrain.draw(terrainShader);

        objectShader.use();
        objectShader.setBool("useNormalMap", useNormalMap);
        objectShader.setBool("useARMMap", useARMMap);
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        objectShader.setVec3("viewPos", camera.getPosition());
        sceneManager.drawAll(objectShader);

        // Gras mit Lichtinfos rendern
        grassSystem.draw(view, projection, (float)glfwGetTime(), camera.getPosition(), sunPos, sunColor);

        skybox.draw(view, projection);

        waterShader.use();
        waterShader.setFloat("time", (float)glfwGetTime());
        waterShader.setFloat("speedMult", sceneManager.env.waterSpeed);
        waterShader.setFloat("steepnessMult", sceneManager.env.waterSteepness);
        waterShader.setFloat("wavelengthMult", sceneManager.env.waterWavelength);
        waterShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sceneManager.env.waterHeight, 0.0f)));
        waterPlane.draw(waterShader, view, projection, camera.getPosition());

        postEffects.endRender(NEAR_PLANE, FAR_PLANE, fogColor, enableFog ? fogDensity : 0.0f);

        ui.beginFrame();
        ui.renderUI(camera, sceneManager, view, projection,
                    useNormalMap, useARMMap, limitFps, fpsLimit, enableFog, fogDensity);
        ui.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (limitFps && fpsLimit > 0) {
            double targetFrameTime = 1.0 / static_cast<double>(fpsLimit);
            while (glfwGetTime() < currentFrame + targetFrameTime) {
                std::this_thread::yield();
            }
        }
    }

    glfwTerminate();
    return 0;
}