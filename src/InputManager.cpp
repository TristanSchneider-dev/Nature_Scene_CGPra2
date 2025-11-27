#include "InputManager.h"

// WICHTIG: Diese Includes sind nötig, um Events an ImGui weiterzuleiten.
// Falls deine Pfade anders sind, bitte anpassen (z.B. "vendor/imgui/...")
#include <imgui.h>
#include <imgui_impl_glfw.h>

InputManager::InputManager(GLFWwindow* window, Camera& camera, UIManager& ui)
    : window(window), camera(camera), ui(ui)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = width / 2.0f;
    lastY = height / 2.0f;

    glfwSetWindowUserPointer(window, this);

    // Callbacks installieren (Überschreibt ImGui's automatische Installation)
    glfwSetCursorPosCallback(window, mouseCallbackStatic);
    glfwSetScrollCallback(window, scrollCallbackStatic);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackStatic); // Neu: Klicks
    glfwSetFramebufferSizeCallback(window, resizeCallbackStatic);

    // Initial Input Mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::processInput(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool isFPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!isFPressed) {
            ui.toggleFullscreen();
            isFPressed = true;
        }
    } else {
        isFPressed = false;
    }

    bool altPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    if (altPressed && !lastAltState) {
        menuMode = !menuMode;
        if (menuMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera.resetFirstMouse();
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    lastAltState = altPressed;

    if (!menuMode) {
        camera.processInput(window, deltaTime);
    }
}

// --- Member Handler ---

void InputManager::onMouse(double xpos, double ypos) {
    // Wenn ImGui die Maus will (z.B. wir sind über einem Fenster), ignorieren wir die Kamera
    // (Optional, aber nützlich)
    // if (ImGui::GetIO().WantCaptureMouse && menuMode) return;

    if (menuMode) return;

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    camera.processMouse((float)xpos, (float)ypos);
    lastX = (float)xpos;
    lastY = (float)ypos;
}

void InputManager::onScroll(double xoffset, double yoffset) {
    // Wenn ImGui die Maus fängt, nicht zoomen
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (!menuMode) {
        camera.processScroll((float)yoffset);
    }
}

void InputManager::onMouseClick(int button, int action, int mods) {
    // Hier könnte Logik für Schießen/Interagieren im Spiel rein
}

void InputManager::onResize(int width, int height) {
    glViewport(0, 0, width, height);
    camera.setViewportSize((float)width, (float)height);
}

// --- Statische Wrapper & Weiterleitung an ImGui ---

void InputManager::mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    // 1. Zuerst an ImGui weiterleiten
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    // 2. Dann eigene Logik
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) input->onMouse(xpos, ypos);
}

void InputManager::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
    // 1. Zuerst an ImGui weiterleiten
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    // 2. Dann eigene Logik
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) input->onScroll(xoffset, yoffset);
}

void InputManager::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
    // 1. Zuerst an ImGui weiterleiten (WICHTIG für Klicks!)
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // 2. Dann eigene Logik
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) input->onMouseClick(button, action, mods);
}

void InputManager::resizeCallbackStatic(GLFWwindow* window, int width, int height) {
    // Resize muss meist nicht an ImGui weitergeleitet werden, da ImGui das über das Window-Handle abfragt,
    // aber schaden tut es auch nicht.

    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) input->onResize(width, height);
}