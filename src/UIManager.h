#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "imgui.h"

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    void beginFrame();
    // Signatur erweitert: limitFps und fpsLimit
    void renderUI(Camera& camera, bool& useNormalMap, bool& useARMMap, bool& limitFps, int& fpsLimit);
    void endFrame();

    void toggleFullscreen();
    void setVSync(bool enabled);
    bool isMouseCaptured() const;

private:
    GLFWwindow* m_window;
    bool m_isFullscreen;
    bool m_vsyncEnabled;
    int m_windowedX, m_windowedY;
    int m_windowedWidth, m_windowedHeight;

    void setResolution(int width, int height);
};