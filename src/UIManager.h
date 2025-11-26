#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "imgui.h"

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    // Setup für den Frame
    void beginFrame();
    
    // Zeichnet die eigentliche UI (Buttons, Text etc.)
    void renderUI(Camera& camera);
    
    // Beendet den Frame und rendert ImGui Daten
    void endFrame();

    // Hilfsfunktionen (public, damit main.cpp z.B. bei 'F'-Taste darauf zugreifen kann)
    void toggleFullscreen();
    bool isMouseCaptured() const; // Wenn true, klickt User in UI, nicht ins Spiel

private:
    GLFWwindow* m_window;
    
    // Fenster Status
    bool m_isFullscreen;
    int m_windowedX, m_windowedY;
    int m_windowedWidth, m_windowedHeight;

    void setResolution(int width, int height);
};