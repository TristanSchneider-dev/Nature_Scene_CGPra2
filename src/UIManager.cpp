#include "UIManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

UIManager::UIManager(GLFWwindow* window)
    : m_window(window), m_isFullscreen(false), m_vsyncEnabled(false),
      m_windowedX(100), m_windowedY(100),
      m_windowedWidth(1280), m_windowedHeight(720)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    setVSync(true);
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

// NEU: Parameter enableFog übernehmen
void UIManager::renderUI(Camera& camera, bool& useNormalMap, bool& useARMMap, bool& limitFps, int& fpsLimit, bool& enableFog, float& fogDensity) {
    ImGui::Begin("Settings");

    ImGui::Text("System Info");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if (ImGui::Checkbox("VSync", &m_vsyncEnabled)) {
        setVSync(m_vsyncEnabled);
    }

    ImGui::Checkbox("Limit FPS", &limitFps);
    if (limitFps) {
        ImGui::Indent();
        ImGui::SliderInt("Max FPS", &fpsLimit, 30, 240);
        ImGui::Unindent();
    }

    if (ImGui::Button(m_isFullscreen ? "Exit Fullscreen" : "Go Fullscreen")) {
        toggleFullscreen();
    }

    ImGui::Separator();

    ImGui::Text("Visual Effects");
    ImGui::Checkbox("Enable Normal Mapping", &useNormalMap);
    ImGui::Checkbox("Enable PBR Materials (ARM)", &useARMMap);

    // --- NEBEL CONFIG ---
    ImGui::Checkbox("Enable Fog", &enableFog);
    if (enableFog) {
        ImGui::Indent();
        // Slider von 0.001 (sehr leicht) bis 0.1 (extrem dicht)
        // "%.3f" zeigt 3 Nachkommastellen an, damit man fein tunen kann
        ImGui::SliderFloat("Intensity", &fogDensity, 0.0f, 0.1f, "%.3f");
        ImGui::Unindent();
    }
    // --------------------

    ImGui::Separator();

    ImGui::Text("Controls:");
    const char* modeText = (camera.mode == Camera::FREE) ? "FREE" : "ORBIT";
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Current Mode: %s", modeText);
    ImGui::BulletText("'1': Free Camera");
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
        setVSync(m_vsyncEnabled);
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_windowedX, m_windowedY, m_windowedWidth, m_windowedHeight, 0);
        setVSync(m_vsyncEnabled);
    }
}

void UIManager::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void UIManager::setResolution(int width, int height) {
    if (m_isFullscreen) return;
    glfwSetWindowSize(m_window, width, height);
    m_windowedWidth = width;
    m_windowedHeight = height;
}