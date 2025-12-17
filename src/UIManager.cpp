#include "UIManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TreeGenerator.h" 
#include "Terrain.h"

UIManager::UIManager(GLFWwindow* window)
    : m_window(window), m_isFullscreen(false), m_vsyncEnabled(true)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);
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
    ImGuizmo::BeginFrame();
}

void UIManager::renderUI(Camera& camera, SceneManager& sceneManager,
                         const glm::mat4& view, const glm::mat4& projection,
                         bool& useNormalMap, bool& useARMMap,
                         bool& limitFps, int& fpsLimit,
                         bool& enableFog, float& fogDensity,
                        // NEU: Tree Generator Parameter
                        TreeGenerator* treeGen,
                        const Terrain* terrain)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::AllowAxisFlip(false);

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
    static ImGuizmo::MODE currentGizmoMode = ImGuizmo::WORLD;

    // Hotkeys für Tools
    if (!io.WantCaptureKeyboard) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            currentGizmoOperation = ImGuizmo::TRANSLATE;
            currentGizmoMode = ImGuizmo::WORLD;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            currentGizmoOperation = ImGuizmo::ROTATE;
            currentGizmoMode = ImGuizmo::LOCAL;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            currentGizmoOperation = ImGuizmo::SCALE;
            currentGizmoMode = ImGuizmo::LOCAL;
        }
    }

    ImGui::Begin("Engine Controls");

    if (ImGui::BeginTabBar("Tabs")) {
        if (ImGui::BeginTabItem("Level Editor")) {

            ImGui::Text("Tool:");
            if (ImGui::RadioButton("Move (W)", currentGizmoOperation == ImGuizmo::TRANSLATE)) {
                currentGizmoOperation = ImGuizmo::TRANSLATE;
                currentGizmoMode = ImGuizmo::WORLD;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate (E)", currentGizmoOperation == ImGuizmo::ROTATE)) {
                currentGizmoOperation = ImGuizmo::ROTATE;
                currentGizmoMode = ImGuizmo::LOCAL;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale (R)", currentGizmoOperation == ImGuizmo::SCALE)) {
                currentGizmoOperation = ImGuizmo::SCALE;
                currentGizmoMode = ImGuizmo::LOCAL;
            }

            ImGui::Separator();
            ImGui::Text("Space:");
            if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD)) currentGizmoMode = ImGuizmo::WORLD;
            ImGui::SameLine();
            if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL)) currentGizmoMode = ImGuizmo::LOCAL;

            ImGui::Separator();

            auto& objects = sceneManager.getObjects();
            ImGui::Text("Scene Hierarchy:");
            if (ImGui::BeginListBox("##SceneObjects", ImVec2(-FLT_MIN, 200))) {
                for (int i = 0; i < objects.size(); i++) {
                    bool isSelected = sceneManager.selectedObjectID == i;
                    if (ImGui::Selectable(objects[i].name.c_str(), isSelected)) {
                        sceneManager.selectedObjectID = i;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }

            if (sceneManager.selectedObjectID >= 0 &&
                sceneManager.selectedObjectID < objects.size()) {

                SceneObject& sel = objects[sceneManager.selectedObjectID];

                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Selected: %s", sel.name.c_str());

                ImGui::DragFloat3("Position", &sel.position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &sel.rotation.x, 0.5f);
                ImGui::DragFloat3("Scale",    &sel.scale.x,    0.05f);

                ImGui::Spacing();

                if (ImGui::Button("Delete Object")) {
                    objects.erase(objects.begin() + sceneManager.selectedObjectID);
                    sceneManager.selectedObjectID = -1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Duplicate")) {
                    SceneObject clone = sel;
                    clone.name += "_copy";
                    clone.position.x += 1.0f;
                    objects.push_back(clone);
                    sceneManager.selectedObjectID = (int)objects.size() - 1;
                }
            }

            ImGui::Separator();
            ImGui::Text("Assets:");

            static std::string currentModel = "";
            if (ImGui::BeginCombo("##SpawnAsset", currentModel.c_str())) {
                for (auto const& [key, val] : sceneManager.getResources()) {
                    bool isSelected = currentModel == key;
                    if (ImGui::Selectable(key.c_str(), isSelected)) currentModel = key;
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Spawn Asset") && !currentModel.empty()) {
                sceneManager.addInstance(currentModel);
            }

            ImGui::Separator();
            if (ImGui::Button("Save Level")) sceneManager.saveScene("level_01.txt");
            ImGui::SameLine();
            if (ImGui::Button("Load Level")) sceneManager.loadScene("level_01.txt");

            // --- HIER WURDE DIE WASSER-STEUERUNG EINGEFÜGT ---
            ImGui::Separator();
            if(ImGui::TreeNode("Environment Settings")) {
                ImGui::SliderFloat("Water Height", &sceneManager.env.waterHeight, -10.0f, 5.0f);
                ImGui::SliderFloat("Water Speed", &sceneManager.env.waterSpeed, 0.0f, 2.0f);
                ImGui::SliderFloat("Wave Height", &sceneManager.env.waterSteepness, 0.0f, 1.0f);
                ImGui::SliderFloat("Wave Length", &sceneManager.env.waterWavelength, 0.1f, 5.0f);
                ImGui::TreePop();
            }
            // -------------------------------------------------

            ImGui::EndTabItem();
        }

        // --- NEU: VEGETATION TAB ---
        if (ImGui::BeginTabItem("Vegetation")) {

            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.3f, 1.0f), "🌳 Tree Generation");
            ImGui::Separator();

            // Statische Variablen für UI-Steuerung
            static int maxTrees = 150;
            static int treeSeed = 42;
            static int treeStep = 5;
            static float treeDensityPower = 12.0f;
            static float minTreeDistance = 2.5f;

            // Info Box
            ImGui::BeginChild("InfoBox", ImVec2(0, 60), true);
            if (treeGen) {
                ImGui::Text("Current Trees: %zu", treeGen->getTrees().size());
                ImGui::Text("Status: Ready");
            }
            else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "TreeGenerator not initialized!");
            }
            ImGui::EndChild();

            ImGui::Spacing();

            // --- BAUM-ANZAHL ---
            ImGui::PushItemWidth(200);
            ImGui::SliderInt("Max Trees", &maxTrees, 10, 1000);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            if (ImGui::Button("?##trees")) {
                ImGui::OpenPopup("TreeHelp");
            }
            if (ImGui::BeginPopup("TreeHelp")) {
                ImGui::Text("Maximum number of trees to generate");
                ImGui::Text("Higher values = more trees, slower generation");
                ImGui::EndPopup();
            }

            // --- RANDOM SEED ---
            ImGui::PushItemWidth(150);
            ImGui::InputInt("Random Seed", &treeSeed);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            if (ImGui::Button("Randomize##seed")) {
                treeSeed = rand() % 10000;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), "⚙️ Density Settings");
            ImGui::Spacing();

            // --- VERTEX STEP ---
            ImGui::PushItemWidth(250);
            ImGui::SliderInt("Sample Rate", &treeStep, 1, 20);
            ImGui::PopItemWidth();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("How often to check terrain vertices:");
                ImGui::Text("  1  = Check every vertex (SLOW, very dense)");
                ImGui::Text("  5  = Check every 5th vertex (BALANCED)");
                ImGui::Text("  10 = Check every 10th vertex (FAST, sparse)");
                ImGui::EndTooltip();
            }

            // --- DENSITY POWER ---
            ImGui::PushItemWidth(250);
            ImGui::SliderFloat("Clustering", &treeDensityPower, 6.0f, 20.0f, "%.1f");
            ImGui::PopItemWidth();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Controls how trees cluster together:");
                ImGui::Text("  6-8   = Very dense clusters");
                ImGui::Text("  12-14 = Natural distribution (RECOMMENDED)");
                ImGui::Text("  16-20 = Sparse, isolated trees");
                ImGui::EndTooltip();
            }

            // --- MIN DISTANCE ---
            ImGui::PushItemWidth(250);
            ImGui::SliderFloat("Min Distance", &minTreeDistance, 0.5f, 8.0f, "%.1f units");
            ImGui::PopItemWidth();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Minimum distance between trees");
                ImGui::Text("Lower = trees can be closer together");
                ImGui::Text("Higher = more spread out");
                ImGui::EndTooltip();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // --- BUTTONS ---
            if (ImGui::Button("🔄 Regenerate Trees", ImVec2(200, 40))) {
                if (treeGen && terrain) {
                    std::cout << "[UI] Regenerating trees with settings:" << std::endl;
                    std::cout << "  Max Trees: " << maxTrees << std::endl;
                    std::cout << "  Seed: " << treeSeed << std::endl;
                    std::cout << "  Sample Rate: " << treeStep << std::endl;
                    std::cout << "  Clustering: " << treeDensityPower << std::endl;
                    std::cout << "  Min Distance: " << minTreeDistance << std::endl;

                    // Parameter setzen
                    treeGen->setParameters(treeStep, treeDensityPower, minTreeDistance);

                    // Neu generieren
                    treeGen->generateTrees(*terrain, maxTrees);
                }
                else {
                    std::cout << "[UI] ERROR: TreeGenerator or Terrain not available!" << std::endl;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("🗑️ Clear All", ImVec2(120, 40))) {
                if (treeGen) {
                    treeGen->clear();
                    std::cout << "[UI] Cleared all trees" << std::endl;
                }
            }

            ImGui::Spacing();
            ImGui::Separator();

            // --- PRESETS ---
            ImGui::TextColored(ImVec4(0.7f, 0.5f, 1.0f, 1.0f), "📋 Presets");
            ImGui::Spacing();

            if (ImGui::Button("Dense Forest", ImVec2(120, 25))) {
                maxTrees = 300;
                treeStep = 3;
                treeDensityPower = 8.0f;
                minTreeDistance = 1.5f;
            }

            ImGui::SameLine();

            if (ImGui::Button("Sparse Woods", ImVec2(120, 25))) {
                maxTrees = 80;
                treeStep = 8;
                treeDensityPower = 16.0f;
                minTreeDistance = 4.0f;
            }

            ImGui::SameLine();

            if (ImGui::Button("Natural Mix", ImVec2(120, 25))) {
                maxTrees = 150;
                treeStep = 5;
                treeDensityPower = 12.0f;
                minTreeDistance = 2.5f;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "💡 Tip: Settings take effect when you click 'Regenerate'");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings")) {
            ImGui::Checkbox("VSync", &m_vsyncEnabled); setVSync(m_vsyncEnabled);
            ImGui::Checkbox("Limit FPS", &limitFps);
            if (limitFps) {
                ImGui::SameLine();
                ImGui::DragInt("Target", &fpsLimit, 1, 30, 240);
            }
            ImGui::Separator();
            ImGui::Checkbox("Fog", &enableFog);
            if (enableFog) ImGui::SliderFloat("Density", &fogDensity, 0.0f, 0.1f, "%.4f");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // --------------------------------------------------------
    //      GIZMO LOGIK (Deine funktionierende Version)
    // --------------------------------------------------------
    auto& objects = sceneManager.getObjects();

    if (sceneManager.selectedObjectID >= 0 &&
        sceneManager.selectedObjectID < objects.size()) {

        SceneObject& sel = objects[sceneManager.selectedObjectID];

        float pos[3] = { sel.position.x, sel.position.y, sel.position.z };
        float rot[3] = { sel.rotation.x, sel.rotation.y, sel.rotation.z };
        float scl[3] = { sel.scale.x,    sel.scale.y,    sel.scale.z    };

        glm::mat4 model(1.0f);
        ImGuizmo::RecomposeMatrixFromComponents(pos, rot, scl, glm::value_ptr(model));

        ImGuizmo::SetID(sceneManager.selectedObjectID);

        ImGuizmo::Manipulate(glm::value_ptr(view),
                             glm::value_ptr(projection),
                             currentGizmoOperation,
                             currentGizmoMode,
                             glm::value_ptr(model));

        if (ImGuizmo::IsUsing()) {
            float newPos[3], newRot[3], newScale[3];
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
                                                  newPos, newRot, newScale);

            sel.position = glm::vec3(newPos[0], newPos[1], newPos[2]);
            sel.rotation = glm::vec3(newRot[0], newRot[1], newRot[2]);
            sel.scale    = glm::vec3(newScale[0], newScale[1], newScale[2]);
        }
    }
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
    return ImGui::GetIO().WantCaptureMouse || ImGuizmo::IsOver() || ImGuizmo::IsUsing();
}