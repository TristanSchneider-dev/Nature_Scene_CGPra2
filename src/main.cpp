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
#include "GrassRenderer.h"
#include "PostProcessor.h"
#include "WaterPlane.h"
#include "SceneManager.h"
#include "Skybox.h"

#include <iostream>
#include <thread>
#include <vector>

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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Engine", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
    UIManager ui(window);
    SceneManager sceneManager;
    sceneManager.loadAssetsFromFolder("../assets/objects");
    sceneManager.loadScene("level_01.txt");

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
        "../assets/skybox/right.png",
        "../assets/skybox/left.png",
        "../assets/skybox/top.png",
        "../assets/skybox/bottom.png",
        "../assets/skybox/front.png",
        "../assets/skybox/back.png"
    };
    Skybox skybox(faces);

    GrassRenderer grassMain(terrain, 150000, "../assets/grass/grass_blade01.png", 1, GRASS);
    grassMain.setColors(glm::vec3(0.34f, 0.40f, 0.05f), glm::vec3(0.27f, 0.31f, 0.07f));

    GrassRenderer grassVar(terrain, 80000, "../assets/grass/grass_blade02.png", 2, SINGLE_BLADE);
    grassVar.setColors(glm::vec3(0.40f, 0.50f, 0.10f), glm::vec3(0.35f, 0.35f, 0.15f));

    GrassRenderer leaves(terrain, 40000, "../assets/grass/leaf01.png", 3, LEAF);
    leaves.setColors(glm::vec3(0.42f, 0.20f, 0.10f), glm::vec3(0.55f, 0.35f, 0.15f));

    terrainShader.use();
    terrainShader.setInt("pebblesAlbedo", 0);
    terrainShader.setInt("pebblesNormal", 1);
    terrainShader.setInt("pebblesARM", 2);
    terrainShader.setInt("groundAlbedo", 3);
    terrainShader.setInt("groundNormal", 4);
    terrainShader.setInt("groundARM", 5);
    terrainShader.setInt("rockAlbedo", 6);
    terrainShader.setInt("rockNormal", 7);
    terrainShader.setInt("rockARM", 8);
    terrainShader.setFloat("tiling", 60.0f);

    glm::vec3 sunPos = glm::vec3(50.0f, 100.0f, 50.0f);
    glm::vec3 sunColor = glm::vec3(1.0f);
    terrainShader.setVec3("lightPos", sunPos);
    terrainShader.setVec3("lightColor", sunColor);

    double lastFrame = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        inputManager.processInput(deltaTime);

        int currentW, currentH;
        glfwGetFramebufferSize(window, &currentW, &currentH);
        if (currentW == 0 || currentH == 0) {
            glfwWaitEvents();
            continue;
        }
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
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(60.0f));
        terrainShader.setMat4("model", model);
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrain.draw(terrainShader);

        objectShader.use();
        objectShader.setBool("useNormalMap", useNormalMap);
        objectShader.setBool("useARMMap", useARMMap);
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        objectShader.setVec3("viewPos", camera.getPosition());
        objectShader.setVec3("lightPos", sunPos);
        objectShader.setVec3("lightColor", sunColor);
        sceneManager.drawAll(objectShader);

        skybox.draw(view, projection);

        waterShader.use();
        waterShader.setFloat("time", (float)glfwGetTime());
        waterShader.setFloat("speedMult", sceneManager.env.waterSpeed);
        waterShader.setFloat("steepnessMult", sceneManager.env.waterSteepness);
        waterShader.setFloat("wavelengthMult", sceneManager.env.waterWavelength);
        waterShader.setVec3("lightPos", sunPos);
        waterShader.setVec3("lightColor", sunColor);
        waterShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sceneManager.env.waterHeight, 0.0f)));
        waterPlane.draw(waterShader, view, projection, camera.getPosition());

        glDisable(GL_CULL_FACE);
        grassMain.draw(view, projection);
        grassVar.draw(view, projection);
        leaves.draw(view, projection);
        glEnable(GL_CULL_FACE);

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
