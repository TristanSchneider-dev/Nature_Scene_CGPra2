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
#include "Model.h"
#include "WaterPlane.h"

#include <iostream>
#include <thread>
#include <string>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Engine", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
    UIManager ui(window);
    InputManager inputManager(window, camera, ui);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    PostProcessor postEffects(fbW, fbH);

    Shader terrainShader("../shaders/terrain.vs.glsl", "../shaders/terrain.fs.glsl");
    Shader objectShader("../shaders/object.vs.glsl", "../shaders/object.fs.glsl");

    // --- WATER SETUP ---
    Shader waterShader("../shaders/water.vs.glsl", "../shaders/water.fs.glsl");
    WaterPlane waterPlane(200.0f, 200);

    // NEU: Ruhigere Standardwerte
    float waterSpeed = 0.15f;       // Viel langsamer
    float waterSteepness = 0.35f;   // Flacher, weniger chaotisch
    float waterWavelength = 1.0f;

    Terrain terrain("../assets/objects/landscape.fbx");

    GrassRenderer grassMain(terrain, 40000, "../assets/textures/grass_blade01.png", 1, GRASS);
    grassMain.setColors(glm::vec3(0.34f, 0.40f, 0.05f), glm::vec3(0.27f, 0.31f, 0.07f));

    GrassRenderer grassVar(terrain, 80000, "../assets/textures/grass_blade02.png", 2, SINGLE_BLADE);
    grassVar.setColors(glm::vec3(0.40f, 0.50f, 0.10f), glm::vec3(0.35f, 0.35f, 0.15f));

    GrassRenderer leaves(terrain, 50000, "../assets/textures/leaf01.png", 3, LEAF);
    leaves.setColors(glm::vec3(0.42f, 0.20f, 0.10f), glm::vec3(0.55f, 0.35f, 0.15f));

    // --- ROCK SET ---
    std::string rFolder = "../assets/objects/rock_moss_set_01/";
    std::string rDiff   = rFolder + "textures/rock_moss_set_01_diff.jpg";
    std::string rNor    = rFolder + "textures/rock_moss_set_01_nor_gl.png";
    std::string rRough  = rFolder + "textures/rock_moss_set_01_rough.png";

    Model rock1(rFolder + "rock_moss_01.gltf"); rock1.setTexturesFromRoughness(rDiff, rNor, rRough);
    Model rock2(rFolder + "rock_moss_02.gltf"); rock2.setTexturesFromRoughness(rDiff, rNor, rRough);
    Model rock3(rFolder + "rock_moss_03.gltf"); rock3.setTexturesFromRoughness(rDiff, rNor, rRough);
    Model rock4(rFolder + "rock_moss_04.gltf"); rock4.setTexturesFromRoughness(rDiff, rNor, rRough);
    Model rock5(rFolder + "rock_moss_05.gltf"); rock5.setTexturesFromRoughness(rDiff, rNor, rRough);
    Model rock6(rFolder + "rock_moss_06.gltf"); rock6.setTexturesFromRoughness(rDiff, rNor, rRough);

    terrainShader.use();
    terrainShader.setInt("texGravelDiff", 0); terrainShader.setInt("texGravelNor", 1); terrainShader.setInt("texGravelArm", 2);
    terrainShader.setInt("texPebblesDiff", 3); terrainShader.setInt("texPebblesNor", 4); terrainShader.setInt("texPebblesArm", 5);
    terrainShader.setInt("texRockDiff", 6); terrainShader.setInt("texRockNor", 7); terrainShader.setInt("texRockArm", 8);
    terrainShader.setVec3("lightPos", glm::vec3(0.0f, 100.0f, 100.0f));
    terrainShader.setVec3("lightColor", glm::vec3(1.0f));

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.processInput(deltaTime);

        int currentW, currentH;
        glfwGetFramebufferSize(window, &currentW, &currentH);
        postEffects.checkResize(currentW, currentH);

        postEffects.beginRender();
        glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (float)currentW / (float)currentH;
        glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), aspect, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = camera.getViewMatrix();

        // 1. Terrain
        terrainShader.use();
        terrainShader.setBool("useNormalMap", useNormalMap);
        terrainShader.setBool("useARMMap", useARMMap);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", glm::mat4(1.0f));
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrain.draw(terrainShader);

        for(int i = 0; i < 9; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // 2. Rocks
        objectShader.use();
        objectShader.setBool("useNormalMap", useNormalMap);
        objectShader.setBool("useARMMap", useARMMap);
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        objectShader.setVec3("viewPos", camera.getPosition());
        objectShader.setVec3("lightPos", glm::vec3(50.0f, 100.0f, 50.0f));
        objectShader.setVec3("lightColor", glm::vec3(1.5f));

        glm::mat4 model;

        // Rock 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.2f, 8.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        rock1.draw(objectShader, model);

        // Rock 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 0.1f, 12.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(1.2f));
        rock2.draw(objectShader, model);

        // Rock 3
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1,0,0));
        rock3.draw(objectShader, model);

        // Rock 4
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 18.0f));
        rock4.draw(objectShader, model);

        // Rock 5
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(11.0f, 0.3f, 9.0f));
        model = glm::scale(model, glm::vec3(2.0f));
        rock5.draw(objectShader, model);

        // Rock 6
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-12.0f, 0.0f, 14.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,1,0));
        rock6.draw(objectShader, model);

        // Ersatz-Bäume (Rocks)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(12.0f, 0.1f, 8.0f));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(2.5f));
        rock3.draw(objectShader, model);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 15.0f));
        model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(2.2f));
        rock2.draw(objectShader, model);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.5f, 0.0f, 16.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        rock6.draw(objectShader, model);

        // 3. Water Rendering (NEU KONFIGURIERT)
        waterShader.use();
        waterShader.setFloat("time", (float)glfwGetTime()); // Wichtig für Noise-Bewegung
        waterShader.setFloat("speedMult", waterSpeed);
        waterShader.setFloat("steepnessMult", waterSteepness);
        waterShader.setFloat("wavelengthMult", waterWavelength);
        waterShader.setVec3("lightPos", glm::vec3(50.0f, 100.0f, 50.0f));
        waterShader.setVec3("lightColor", glm::vec3(1.5f));

        // NEU: Wasser auf -3.0 gesetzt (tiefer)
        glm::mat4 waterModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -30.0f, 0.0f));
        waterShader.setMat4("model", waterModel);

        waterPlane.draw(waterShader, view, projection, camera.getPosition());

        // 4. Grass
        glDisable(GL_CULL_FACE);
        grassMain.draw(view, projection);
        grassVar.draw(view, projection);
        leaves.draw(view, projection);
        glEnable(GL_CULL_FACE);

        float effectiveDensity = enableFog ? fogDensity : 0.0f;
        postEffects.endRender(NEAR_PLANE, FAR_PLANE, fogColor, effectiveDensity);

        // 5. UI
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