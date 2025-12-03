#include "SceneManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <filesystem> // C++17 Feature für Datei-Scanning

namespace fs = std::filesystem;

SceneManager::SceneManager() {
    // Standard-Werte
    env.waterHeight = -3.0f;
    env.waterSpeed = 0.15f;
    env.waterSteepness = 0.35f;
    env.waterWavelength = 1.0f;
}

SceneManager::~SceneManager() {
    // Speicher aufräumen: Alle geladenen Modelle löschen
    for (auto const& [key, modelPtr] : loadedModels) {
        delete modelPtr;
    }
    loadedModels.clear();
}

void SceneManager::registerModel(std::string key, std::string path) {
    // Nur laden, wenn noch nicht vorhanden
    if (loadedModels.find(key) == loadedModels.end()) {
        std::cout << "[SceneManager] Loading Asset: " << key << " from " << path << "..." << std::endl;
        loadedModels[key] = new Model(path);
    }
}

void SceneManager::loadAssetsFromFolder(const std::string& folderPath) {
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cout << "[SceneManager] ERROR: Asset directory not found: " << folderPath << std::endl;
        return;
    }

    std::cout << "[SceneManager] Scanning folder: " << folderPath << std::endl;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();
            std::string path = entry.path().string();

            // Konvertiere Extension zu Kleinbuchstaben für Vergleich (optional, aber sicherer)
            // Hier einfacher Check:

            // 1. Filter: Landschaft ignorieren (da sie separat in main.cpp behandelt wird)
            if (filename == "landscape.fbx") {
                continue;
            }

            // 2. Filter: Nur 3D Formate
            bool isModel = (extension == ".glb" || extension == ".gltf" || extension == ".fbx" || extension == ".obj");

            if (isModel) {
                // Key ist der Dateiname OHNE Endung (z.B. "tree.glb" -> "tree")
                std::string key = entry.path().stem().string();
                registerModel(key, path);
            }
        }
    }
}

void SceneManager::addInstance(std::string modelKey) {
    if (loadedModels.find(modelKey) != loadedModels.end()) {
        SceneObject obj;
        obj.modelKey = modelKey;
        // Eindeutiger Name: "Key_Index", z.B. "Rock_5"
        obj.name = modelKey + "_" + std::to_string(objects.size());

        // Standard Transform (Spawn bei 0,0,0)
        obj.position = glm::vec3(0, 0, 0);
        obj.rotation = glm::vec3(0, 0, 0);
        obj.scale = glm::vec3(1.0f);

        objects.push_back(obj);

        // Das neue Objekt direkt selektieren
        selectedObjectID = static_cast<int>(objects.size()) - 1;
        std::cout << "Spawned object: " << obj.name << std::endl;
    } else {
        std::cout << "Error: Model key '" << modelKey << "' not found!" << std::endl;
    }
}

void SceneManager::drawAll(Shader& shader) {
    for (auto& obj : objects) {
        // Prüfen ob das Model geladen ist
        if (loadedModels.find(obj.modelKey) != loadedModels.end()) {
            glm::mat4 model = glm::mat4(1.0f);

            // Reihenfolge: Translate -> Rotate -> Scale
            model = glm::translate(model, obj.position);

            // Rotation (Euler Angles)
            model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0, 0, 1));

            model = glm::scale(model, obj.scale);

            // Zeichnen
            loadedModels[obj.modelKey]->Draw(shader, model);
        }
    }
}

void SceneManager::saveScene(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not save to " << filename << std::endl;
        return;
    }

    // 1. Environment Settings speichern
    file << env.waterHeight << " "
         << env.waterSpeed << " "
         << env.waterSteepness << " "
         << env.waterWavelength << "\n";

    // 2. Anzahl der Objekte speichern
    file << objects.size() << "\n";

    // 3. Alle Objekte speichern
    for (const auto& obj : objects) {
        file << obj.modelKey << " " << obj.name << "\n";
        file << obj.position.x << " " << obj.position.y << " " << obj.position.z << "\n";
        file << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << "\n";
        file << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << "\n";
    }

    std::cout << "[SceneManager] Level saved to " << filename << std::endl;
    file.close();
}

void SceneManager::loadScene(const std::string& filename) {
    std::ifstream file(filename);

    // Check 1: Existiert die Datei?
    if (!file.is_open()) {
        std::cout << "[SceneManager] No savefile found (" << filename << "). Starting new scene." << std::endl;
        return;
    }

    std::cout << "[SceneManager] Loading level from " << filename << "..." << std::endl;

    // Buffer Variablen für Environment
    float wh, wsp, wst, wwl;

    // Versuche Environment zu lesen
    if (file >> wh >> wsp >> wst >> wwl) {
        env.waterHeight = wh;
        env.waterSpeed = wsp;
        env.waterSteepness = wst;
        env.waterWavelength = wwl;
    } else {
        std::cout << "WARNING: Save file format invalid (Env settings missing). Using defaults." << std::endl;
        // Datei schließen und abbrechen, oder weiterversuchen? Wir brechen ab um Absturz zu vermeiden.
        file.close();
        return;
    }

    // Objekte laden
    objects.clear();
    size_t count = 0;

    if (!(file >> count)) {
        std::cout << "WARNING: Could not read object count." << std::endl;
        return;
    }

    // Sicherheits-Check
    if (count > 10000) {
        std::cout << "ERROR: Object count is suspiciously high (" << count << "). File corrupted?" << std::endl;
        return;
    }

    for (size_t i = 0; i < count; i++) {
        SceneObject obj;

        // Lesen
        if (!(file >> obj.modelKey >> obj.name
                   >> obj.position.x >> obj.position.y >> obj.position.z
                   >> obj.rotation.x >> obj.rotation.y >> obj.rotation.z
                   >> obj.scale.x >> obj.scale.y >> obj.scale.z)) {
            std::cout << "Error reading object #" << i << ". Stopping load." << std::endl;
            break;
        }

        // Prüfen ob das Model geladen wurde
        if (loadedModels.find(obj.modelKey) != loadedModels.end()) {
            objects.push_back(obj);
        } else {
            // Falls ein Model gelöscht wurde oder der Name anders ist
            std::cout << "WARNING: Scene contains unknown model '" << obj.modelKey << "'. Skipping object." << std::endl;
        }
    }

    std::cout << "[SceneManager] Loaded " << objects.size() << " objects." << std::endl;
    file.close();
}

int SceneManager::getClosestObjectFromRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir) {
    int closestIndex = -1;
    float closestDist = 999999.0f;

    for (size_t i = 0; i < objects.size(); i++) {
        SceneObject& obj = objects[i];

        // VEREINFACHUNG: Wir nehmen an, jedes Objekt ist eine Kugel mit Radius 1.5 * Scale
        // Für präziseres Picking bräuchte man AABBs (Axis Aligned Bounding Boxes) aus dem Model
        float radius = std::max(obj.scale.x, std::max(obj.scale.y, obj.scale.z)) * 1.5f;

        glm::vec3 oc = rayOrigin - obj.position;
        float b = glm::dot(oc, rayDir);
        float c = glm::dot(oc, oc) - radius * radius;
        float h = b * b - c;

        if (h < 0.0f) continue; // Kein Treffer

        float hSqrt = sqrt(h);
        float t = -b - hSqrt; // Distanz zum Eintrittspunkt

        if (t > 0.0f && t < closestDist) {
            closestDist = t;
            closestIndex = (int)i;
        }
    }
    return closestIndex;
}