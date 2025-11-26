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

    // UPDATE: Neue Parameter für das Terrain hinzugefügt
    void renderUI(Camera& camera, int& terrainRes, float& terrainSize, bool& meshChanged);

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