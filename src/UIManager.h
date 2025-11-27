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
    void renderUI(Camera& camera, bool& useNormalMap, bool& useARMMap);
    void endFrame();

    void toggleFullscreen();
    bool isMouseCaptured() const;

private:
    GLFWwindow* m_window;
    bool m_isFullscreen;
    int m_windowedX, m_windowedY;
    int m_windowedWidth, m_windowedHeight;

    void setResolution(int width, int height);
};