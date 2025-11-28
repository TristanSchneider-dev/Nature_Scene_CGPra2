#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "UIManager.h"

class InputManager {
public:
    InputManager(GLFWwindow* window, Camera& camera, UIManager& ui);

    void processInput(float deltaTime);
    bool isMenuMode() const { return menuMode; }

private:
    GLFWwindow* window;
    Camera& camera;
    UIManager& ui;

    // State
    bool menuMode = false;
    bool lastAltState = false;
    bool firstMouse = true;
    float lastX = 0.0f;
    float lastY = 0.0f;

    // Interne Handler (Logik der Instanz)
    void onMouse(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);
    void onMouseClick(int button, int action, int mods);
    void onResize(int width, int height);

    // Statische Wrapper für GLFW (Weiterleitung an ImGui + Instanz)
    static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void resizeCallbackStatic(GLFWwindow* window, int width, int height);

    // NEU: Nötig für ImGui Text-Input und Tasten-Events
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void charCallbackStatic(GLFWwindow* window, unsigned int codepoint);
};