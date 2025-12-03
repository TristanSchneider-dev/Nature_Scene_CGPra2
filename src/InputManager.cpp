#include "InputManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <algorithm>

InputManager::InputManager(GLFWwindow* window, Camera& camera, UIManager& ui, SceneManager& sceneManager)
    : window(window), camera(camera), ui(ui), sceneManager(sceneManager)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = width / 2.0f;
    lastY = height / 2.0f;

    glfwSetWindowUserPointer(window, this);

    glfwSetCursorPosCallback(window, mouseCallbackStatic);
    glfwSetScrollCallback(window, scrollCallbackStatic);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackStatic);
    glfwSetFramebufferSizeCallback(window, resizeCallbackStatic);
    glfwSetKeyCallback(window, keyCallbackStatic);
    glfwSetCharCallback(window, charCallbackStatic);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::processInput(float deltaTime) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;

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

void InputManager::onMouse(double xpos, double ypos) {
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
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (!menuMode) camera.processScroll((float)yoffset);
}

void InputManager::onResize(int width, int height) {
    glViewport(0, 0, width, height);
    camera.setViewportSize((float)width, (float)height);
}

void InputManager::onMouseClick(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // UI Blockt Raycast (für Gizmos)
        if (ui.isMouseCaptured()) return;

        if (menuMode) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Raycast Berechnung
            float x = (2.0f * (float)xpos) / width - 1.0f;
            float y = 1.0f - (2.0f * (float)ypos) / height;
            glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);

            glm::mat4 proj = glm::perspective(glm::radians(camera.getFov()), (float)width / (float)height, 0.1f, 1000.0f);
            glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

            glm::vec3 ray_wor = glm::vec3(glm::inverse(camera.getViewMatrix()) * ray_eye);
            ray_wor = glm::normalize(ray_wor);

            // Objekt suchen
            int hitIndex = sceneManager.getClosestObjectFromRay(camera.getPosition(), ray_wor);
            sceneManager.selectedObjectID = hitIndex;
        }
    }
}

// --- Statische Wrapper ---

void InputManager::mouseCallbackStatic(GLFWwindow* w, double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(w, x, y);
    if (auto* i = (InputManager*)glfwGetWindowUserPointer(w)) i->onMouse(x, y);
}

void InputManager::scrollCallbackStatic(GLFWwindow* w, double x, double y) {
    ImGui_ImplGlfw_ScrollCallback(w, x, y);
    if (auto* i = (InputManager*)glfwGetWindowUserPointer(w)) i->onScroll(x, y);
}

void InputManager::mouseButtonCallbackStatic(GLFWwindow* w, int b, int a, int m) {
    ImGui_ImplGlfw_MouseButtonCallback(w, b, a, m);
    if (auto* i = (InputManager*)glfwGetWindowUserPointer(w)) i->onMouseClick(b, a, m);
}

void InputManager::resizeCallbackStatic(GLFWwindow* w, int width, int height) {
    if (auto* i = (InputManager*)glfwGetWindowUserPointer(w)) i->onResize(width, height);
}

void InputManager::keyCallbackStatic(GLFWwindow* w, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(w, key, scancode, action, mods);
}

void InputManager::charCallbackStatic(GLFWwindow* w, unsigned int codepoint) {
    ImGui_ImplGlfw_CharCallback(w, codepoint);
}