#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "SceneManager.h"
#include "imgui.h"

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    void beginFrame();
    void endFrame();

    // Wasser-Variablen entfernt, da jetzt in SceneManager
    void renderUI(Camera& camera, SceneManager& sceneManager,
                  bool& useNormalMap, bool& useARMMap,
                  bool& limitFps, int& fpsLimit,
                  bool& enableFog, float& fogDensity);

    void toggleFullscreen();
    void setVSync(bool enabled);
    bool isMouseCaptured() const;

private:
    GLFWwindow* m_window;
    bool m_isFullscreen;
    bool m_vsyncEnabled;
    int m_windowedX, m_windowedY;
    int m_windowedWidth, m_windowedHeight;
};