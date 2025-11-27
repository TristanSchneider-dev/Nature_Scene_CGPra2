#include "UIManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

UIManager::UIManager(GLFWwindow* window)
    : m_window(window), m_isFullscreen(false),
      m_windowedX(100), m_windowedY(100),
      m_windowedWidth(1280), m_windowedHeight(720)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

UIManager::~UIManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::renderUI(Camera& camera, bool& useNormalMap, bool& useARMMap) {
    ImGui::Begin("Settings");

    ImGui::Text("System Info");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if (ImGui::Button(m_isFullscreen ? "Exit Fullscreen" : "Go Fullscreen")) {
        toggleFullscreen();
    }

    ImGui::Separator();

    ImGui::Text("Visual Effects");
    ImGui::Checkbox("Enable Normal Mapping", &useNormalMap);
    ImGui::Checkbox("Enable PBR Materials (ARM)", &useARMMap);

    ImGui::Separator();

    ImGui::Text("Controls:");
    const char* modeText = (camera.mode == Camera::FREE) ? "FREE" : "ORBIT";
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Current Mode: %s", modeText);

    ImGui::BulletText("'1': Free Camera (WASD + Mouse)");
    ImGui::BulletText("'2': Orbit Camera");
    ImGui::BulletText("'F': Toggle Fullscreen");
    ImGui::BulletText("'ALT': Toggle Mouse");

    ImGui::End();
}

void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UIManager::isMouseCaptured() const {
    return ImGui::GetIO().WantCaptureMouse;
}

void UIManager::toggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;

    if (m_isFullscreen) {
        glfwGetWindowPos(m_window, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_windowedX, m_windowedY, m_windowedWidth, m_windowedHeight, 0);
    }
}

void UIManager::setResolution(int width, int height) {
    if (m_isFullscreen) return;
    glfwSetWindowSize(m_window, width, height);
    m_windowedWidth = width;
    m_windowedHeight = height;
}