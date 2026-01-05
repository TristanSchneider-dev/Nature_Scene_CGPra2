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
#include "ForestSystem.h" // Neu

#include <iostream>
#include <vector>
#include <string>
#include <thread>

const unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
const float NEAR_PLANE = 0.1f, FAR_PLANE = 1000.0f;

bool useNormalMap = true, useARMMap = true, limitFps = true, enableFog = true, isDay = true;
int fpsLimit = 120;
float fogDensity = 0.025f;

glm::vec3 fogColorDay(0.5f, 0.6f, 0.7f), fogColorNight(0.05f, 0.05f, 0.1f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Forest Kit - Final", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST); glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE); glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
    UIManager ui(window);
    SceneManager sceneManager;
    InputManager inputManager(window, camera, ui, sceneManager);

    int fbW, fbH; glfwGetFramebufferSize(window, &fbW, &fbH);
    PostProcessor postEffects(fbW, fbH);

    Shader terrainShader("../shaders/terrain.vs.glsl", "../shaders/terrain.fs.glsl");
    Shader objectShader("../shaders/object.vs.glsl", "../shaders/object.fs.glsl");
    Shader waterShader("../shaders/water.vs.glsl", "../shaders/water.fs.glsl");

    Terrain terrain("../assets/terrain/landscape.glb");
    WaterPlane waterPlane(800.0f, 800);

    std::vector<std::string> dayFaces = {
        "../assets/skybox/day_right.png", "../assets/skybox/day_left.png", "../assets/skybox/day_top.png",
        "../assets/skybox/day_bottom.png", "../assets/skybox/day_front.png", "../assets/skybox/day_back.png"
    };
    std::vector<std::string> nightFaces = {
        "../assets/skybox/night_right.png", "../assets/skybox/night_left.png", "../assets/skybox/night_top.png",
        "../assets/skybox/night_bottom.png", "../assets/skybox/night_front.png", "../assets/skybox/night_back.png"
    };
    Skybox skybox(dayFaces, nightFaces);

    // --- GRASS SETUP ---
    GrassSystem grassSystem;
    grassSystem.initTerrainData(terrain.getVertices(), terrain.getIndices(), 60.0f);
    std::string gp = "../assets/grass/"; float sp = 190.0f;

    for (int i = 1; i <= 6; i++) grassSystem.addGrassType(gp + "grass_" + (i<10?"0":"") + std::to_string(i) + ".png", 800000, sp, 0.15f, false);
    for (int i = 7; i <= 10; i++) grassSystem.addGrassType(gp + "grass_" + (i<10?"0":"") + std::to_string(i) + ".png", 8000, sp, 0.2f, false);
    for (int i = 11; i <= 12; i++) grassSystem.addGrassType(gp + "grass_" + std::to_string(i) + ".png", 1000, sp, 0.2f, false);
    grassSystem.addGrassType(gp + "grass_13.png", 8000, sp, 0.2f, false);
    grassSystem.addGrassType(gp + "leaf_01.png", 1000, sp, 0.5f, false);
    for (int i = 2; i <= 10; i++) grassSystem.addGrassType(gp + "leaf_" + (i<10?"0":"") + std::to_string(i) + ".png", (i<=5?15000:10000), sp, 0.15f, true);

    // --- FOREST SETUP ---
    ForestSystem forest;
    forest.initTerrainData(terrain.getVertices(), terrain.getIndices(), 60.0f);

    std::string fp = "../assets/forrest/";

    // Wir fluten die Map jetzt mit kleineren Objekten für mehr Dichte und Vielfalt:

    // 80 Gruppen Birken (Birkenwald)
    forest.addBiomeCluster("Birch", 80, fp);

    // 80 Gruppen Nadelwald (Kiefer + Tanne)
    forest.addBiomeCluster("Pine", 80, fp);

    // 60 Gruppen Eichenwald
    forest.addBiomeCluster("Oak", 60, fp);

    // 100 Gruppen Gestrüpp (verbindet die Wälder)
    forest.addBiomeCluster("Scrub", 100, fp);

    // Shader config
    terrainShader.use();
    terrainShader.setInt("pebblesAlbedo", 0); terrainShader.setInt("pebblesNormal", 1); terrainShader.setInt("pebblesARM", 2);
    terrainShader.setInt("groundAlbedo", 3); terrainShader.setInt("groundNormal", 4); terrainShader.setInt("groundARM", 5);
    terrainShader.setInt("rockAlbedo", 6); terrainShader.setInt("rockNormal", 7); terrainShader.setInt("rockARM", 8);
    terrainShader.setFloat("tiling", 60.0f);

    glm::vec3 sunPosDay(50.0f, 100.0f, 50.0f), sunColorDay(1.0f);
    glm::vec3 sunPosNight(50.0f, 100.0f, -50.0f), sunColorNight(0.1f, 0.1f, 0.3f);
    double lastFrame = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        glm::vec3 curSunPos = isDay ? sunPosDay : sunPosNight;
        glm::vec3 curSunCol = isDay ? sunColorDay : sunColorNight;
        glm::vec3 curFogCol = isDay ? fogColorDay : fogColorNight;
        skybox.setDay(isDay);
        skybox.setNightFactor(isDay ? 0.0f : 1.0f);

        auto setLight = [&](Shader& s) { s.use(); s.setVec3("lightPos", curSunPos); s.setVec3("lightColor", curSunCol); };
        setLight(terrainShader); setLight(objectShader); setLight(waterShader);

        inputManager.processInput(deltaTime);
        int cw, ch; glfwGetFramebufferSize(window, &cw, &ch);
        if (cw == 0 || ch == 0) { glfwWaitEvents(); continue; }
        postEffects.checkResize(cw, ch);

        postEffects.beginRender();
        glClearColor(curFogCol.r, curFogCol.g, curFogCol.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 proj = glm::perspective(glm::radians(camera.getFov()), (float)cw/(float)ch, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = camera.getViewMatrix();

        // Terrain
        terrainShader.use();
        terrainShader.setBool("useNormalMap", useNormalMap);
        terrainShader.setMat4("projection", proj); terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(60.0f)));
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrain.draw(terrainShader);

        // [FIX] Textur-Slots säubern, damit Bäume nicht Terrain-Texturen erben
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

        // Objects & Forest
        objectShader.use();
        objectShader.setBool("useNormalMap", useNormalMap);
        objectShader.setBool("useARMMap", useARMMap);
        objectShader.setMat4("projection", proj); objectShader.setMat4("view", view);
        objectShader.setVec3("viewPos", camera.getPosition());

        sceneManager.drawAll(objectShader); // Manuell platzierte Objekte
        forest.draw(objectShader, view, proj, camera.getPosition()); // Automatisch generierter Wald

        // Grass & Skybox
        grassSystem.draw(view, proj, (float)glfwGetTime(), camera.getPosition(), curSunPos, curSunCol);
        skybox.draw(view, proj);

        // Water
        waterShader.use();
        waterShader.setFloat("time", (float)glfwGetTime());
        waterShader.setFloat("speedMult", sceneManager.env.waterSpeed);
        waterShader.setFloat("steepnessMult", sceneManager.env.waterSteepness);
        waterShader.setFloat("wavelengthMult", sceneManager.env.waterWavelength);
        waterShader.setVec3("fogColor", curFogCol);
        waterShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sceneManager.env.waterHeight, 0.0f)));
        waterPlane.draw(waterShader, view, proj, camera.getPosition());

        postEffects.endRender(NEAR_PLANE, FAR_PLANE, curFogCol, enableFog ? fogDensity : 0.0f);

        ui.beginFrame();
        ui.renderUI(camera, sceneManager, view, proj, useNormalMap, useARMMap, limitFps, fpsLimit, enableFog, fogDensity, isDay);
        ui.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (limitFps && fpsLimit > 0) {
            double target = 1.0 / fpsLimit;
            while (glfwGetTime() < currentFrame + target) std::this_thread::yield();
        }
    }
    glfwTerminate();
    return 0;
}