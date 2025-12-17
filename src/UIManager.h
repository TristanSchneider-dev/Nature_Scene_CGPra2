#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "SceneManager.h"
#include "imgui.h"
#include "ImGuizmo.h" // Stelle sicher, dass ImGuizmo.h im src Ordner liegt

class TreeGenerator;
class Terrain;

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    void beginFrame();
    void endFrame();

    // RenderUI braucht jetzt View & Projection Matrix für das 3D-Gizmo
    void renderUI(Camera& camera, SceneManager& sceneManager,
                  const glm::mat4& view, const glm::mat4& projection,
                  bool& useNormalMap, bool& useARMMap,
                  bool& limitFps, int& fpsLimit,
                  bool& enableFog, float& fogDensity,
                  TreeGenerator* treeGen = nullptr,      // NEU
                  const Terrain* terrain = nullptr);     // NEU

    void toggleFullscreen();
    void setVSync(bool enabled);

    // Gibt true zurück, wenn Maus über UI oder Gizmo ist (Kamera stoppen)
    bool isMouseCaptured() const;

private:
    GLFWwindow* m_window;
    bool m_isFullscreen;
    bool m_vsyncEnabled;
    int m_windowedX, m_windowedY;
    int m_windowedWidth, m_windowedHeight;
};