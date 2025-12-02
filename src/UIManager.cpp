#include "UIManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

UIManager::UIManager(GLFWwindow* window)
    : m_window(window), m_isFullscreen(false), m_vsyncEnabled(true)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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

void UIManager::renderUI(Camera& camera, SceneManager& sceneManager,
                         bool& useNormalMap, bool& useARMMap,
                         bool& limitFps, int& fpsLimit,
                         bool& enableFog, float& fogDensity)
{
    ImGui::Begin("Engine Controls");

    if (ImGui::BeginTabBar("Tabs")) {

        // --- TAB 1: LEVEL EDITOR ---
        if (ImGui::BeginTabItem("Level Editor")) {

            // A. ENVIRONMENT SETTINGS (Jetzt hier!)
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Environment");
            ImGui::SliderFloat("Water Height", &sceneManager.env.waterHeight, -10.0f, 5.0f);
            ImGui::SliderFloat("Water Speed",  &sceneManager.env.waterSpeed, 0.0f, 2.0f);
            ImGui::SliderFloat("Wave Height",  &sceneManager.env.waterSteepness, 0.0f, 1.0f);
            ImGui::SliderFloat("Wave Length",  &sceneManager.env.waterWavelength, 0.1f, 5.0f);

            ImGui::Separator();

            // B. OBJEKT SPAWNER
            ImGui::Text("Spawn Object:");
            static std::string currentModel = "";
            if (ImGui::BeginCombo("Asset", currentModel.c_str())) {
                for (auto const& [key, val] : sceneManager.getResources()) {
                    bool isSelected = (currentModel == key);
                    if (ImGui::Selectable(key.c_str(), isSelected)) currentModel = key;
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Spawn") && !currentModel.empty()) {
                sceneManager.addInstance(currentModel);
            }

            ImGui::Separator();

            // C. SCENE LISTE
            ImGui::Text("Scene Objects:");
            auto& objects = sceneManager.getObjects();
            if (ImGui::BeginListBox("##SceneList", ImVec2(-FLT_MIN, 150))) {
                for (int i = 0; i < objects.size(); i++) {
                    const bool isSelected = (sceneManager.selectedObjectID == i);
                    if (ImGui::Selectable(objects[i].name.c_str(), isSelected)) {
                        sceneManager.selectedObjectID = i;
                    }
                }
                ImGui::EndListBox();
            }

            // D. TRANSFORM GIZMOS
            if (sceneManager.selectedObjectID >= 0 && sceneManager.selectedObjectID < objects.size()) {
                SceneObject& sel = objects[sceneManager.selectedObjectID];
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1,1,0,1), "Selected: %s", sel.name.c_str());
                ImGui::DragFloat3("Pos", &sel.position.x, 0.1f);
                ImGui::DragFloat3("Rot", &sel.rotation.x, 1.0f);
                ImGui::DragFloat3("Scale", &sel.scale.x, 0.05f);

                if(ImGui::Button("Delete")) {
                    objects.erase(objects.begin() + sceneManager.selectedObjectID);
                    sceneManager.selectedObjectID = -1;
                }
                ImGui::SameLine();
                if(ImGui::Button("Clone")) {
                    SceneObject clone = sel;
                    clone.name += "_copy";
                    clone.position.x += 1.0f;
                    objects.push_back(clone);
                }
            }

            ImGui::Separator();
            // SPEICHERN BUTTON (Speichert jetzt auch Wasser!)
            if (ImGui::Button("Save Level")) sceneManager.saveScene("level_01.txt");
            ImGui::SameLine();
            if (ImGui::Button("Load Level")) sceneManager.loadScene("level_01.txt");

            ImGui::EndTabItem();
        }

        // --- TAB 2: GLOBAL SETTINGS ---
        if (ImGui::BeginTabItem("Settings")) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            if (ImGui::Checkbox("VSync", &m_vsyncEnabled)) setVSync(m_vsyncEnabled);

            ImGui::Checkbox("Limit FPS", &limitFps);
            if (limitFps) {
                ImGui::SameLine();
                if (ImGui::Button("-") && fpsLimit > 30) fpsLimit -= 30;
                ImGui::SameLine();
                ImGui::Text("%d", fpsLimit);
                ImGui::SameLine();
                if (ImGui::Button("+") && fpsLimit < 240) fpsLimit += 30;
            }

            ImGui::Separator();
            ImGui::Text("Visuals");
            ImGui::Checkbox("Normal Maps", &useNormalMap);
            ImGui::Checkbox("PBR ARM", &useARMMap);
            ImGui::Checkbox("Fog", &enableFog);
            if (enableFog) ImGui::SliderFloat("Density", &fogDensity, 0.0f, 0.1f, "%.3f");

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    setVSync(m_vsyncEnabled);
}

void UIManager::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

bool UIManager::isMouseCaptured() const {
    return ImGui::GetIO().WantCaptureMouse;
}