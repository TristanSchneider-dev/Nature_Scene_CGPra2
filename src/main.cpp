#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "UIManager.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera;
UIManager* ui = nullptr;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Terrain Variablen
unsigned int terrainVAO, terrainVBO, terrainEBO;
int terrainResolution = 200; // HÃ¶here AuflÃ¶sung fÃ¼r schÃ¶nere Wellen
float terrainSize = 20.0f;
std::vector<unsigned int> terrainIndices;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.setViewportSize((float)width, (float)height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (ui && ui->isMouseCaptured()) return;
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) return;
    camera.processMouse(static_cast<float>(xposIn), static_cast<float>(yposIn));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ui && ui->isMouseCaptured()) return;
    camera.processScroll(static_cast<float>(yoffset));
}

void createPlane() {
    std::vector<float> vertices;
    terrainIndices.clear();

    float step = terrainSize / (float)terrainResolution;
    float offset = terrainSize / 2.0f;

    for (int z = 0; z <= terrainResolution; ++z) {
        for (int x = 0; x <= terrainResolution; ++x) {
            vertices.push_back(x * step - offset);
            vertices.push_back(0.0f);
            vertices.push_back(z * step - offset);

            // Standard-Normale (wird im Terrain-Shader eh Ã¼berschrieben)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }

    for (int z = 0; z < terrainResolution; ++z) {
        for (int x = 0; x < terrainResolution; ++x) {
            int start = z * (terrainResolution + 1) + x;
            terrainIndices.push_back(start);
            terrainIndices.push_back(start + terrainResolution + 1);
            terrainIndices.push_back(start + 1);

            terrainIndices.push_back(start + 1);
            terrainIndices.push_back(start + terrainResolution + 1);
            terrainIndices.push_back(start + terrainResolution + 2);
        }
    }

    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIndices.size() * sizeof(unsigned int), terrainIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Shader", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    // --- Shader laden ---
    // Standard Shader (fÃ¼r WÃ¼rfel etc.)
    Shader phongShader("shaders/phong.vs.glsl", "shaders/phong.fs.glsl");
    // Spezifischer Terrain Shader
    Shader terrainShader("shaders/terrain.vs.glsl", "shaders/terrain.fs.glsl");

    camera.setViewportSize(SCR_WIDTH, SCR_HEIGHT);
    ui = new UIManager(window);

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);
    createPlane();

    bool fPressed = false;
    bool showMouse = false;
    bool altPressed = false;

    // --- Loop ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        // Input Handling UI/Mouse
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fPressed) {
            ui->toggleFullscreen(); fPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fPressed = false;

        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && !altPressed) {
            showMouse = !showMouse;
            altPressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, showMouse ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            camera.resetFirstMouse();
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) altPressed = false;

        if (!showMouse && !ui->isMouseCaptured()) camera.processInput(window, deltaTime);
        if (camera.mode == Camera::ORBIT) camera.updateOrbit(deltaTime);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ui->beginFrame();

        // --- Terrain Rendern ---
        terrainShader.use();

        // Uniforms setzen
        terrainShader.setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));
        terrainShader.setVec3("lightPos", glm::vec3(5.0f, 20.0f, 10.0f));
        terrainShader.setVec3("viewPos", camera.getPosition());
        terrainShader.setMat4("projection", camera.getProjectionMatrix());
        terrainShader.setMat4("view", camera.getViewMatrix());
        terrainShader.setMat4("model", glm::mat4(1.0f));
        terrainShader.setVec3("objectColor", glm::vec3(0.25f, 0.65f, 0.25f)); // GrasgrÃ¼n

        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(terrainIndices.size()), GL_UNSIGNED_INT, 0);

        // UI Logic
        bool meshChanged = false;
        ui->renderUI(camera, terrainResolution, terrainSize, meshChanged);
        if (meshChanged) createPlane();

        ui->endFrame();
        glfwSwapBuffers(window);
    }

    delete ui;
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainEBO);
    glfwTerminate();
    return 0;
}