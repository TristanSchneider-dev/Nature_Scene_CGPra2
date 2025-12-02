#include "SceneManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
    for (auto const& [key, val] : loadedModels) {
        delete val;
    }
}

void SceneManager::registerModel(std::string key, std::string path) {
    if (loadedModels.find(key) == loadedModels.end()) {
        std::cout << "Loading Asset: " << key << " from " << path << "..." << std::endl;
        loadedModels[key] = new Model(path);
    }
}

void SceneManager::addInstance(std::string modelKey) {
    if (loadedModels.find(modelKey) != loadedModels.end()) {
        SceneObject obj;
        obj.modelKey = modelKey;
        obj.name = modelKey + "_" + std::to_string(objects.size());
        obj.position = glm::vec3(0, 0, 0);
        obj.rotation = glm::vec3(0, 0, 0);
        obj.scale = glm::vec3(1.0f);
        objects.push_back(obj);
        selectedObjectID = objects.size() - 1;
    }
}

void SceneManager::drawAll(Shader& shader) {
    for (auto& obj : objects) {
        if (loadedModels.find(obj.modelKey) != loadedModels.end()) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, obj.scale);
            loadedModels[obj.modelKey]->Draw(shader, model);
        }
    }
}

void SceneManager::saveScene(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    // 1. Environment speichern
    file << env.waterHeight << " "
         << env.waterSpeed << " "
         << env.waterSteepness << " "
         << env.waterWavelength << "\n";

    // 2. Objekte speichern
    file << objects.size() << "\n";
    for (const auto& obj : objects) {
        file << obj.modelKey << " " << obj.name << "\n";
        file << obj.position.x << " " << obj.position.y << " " << obj.position.z << "\n";
        file << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << "\n";
        file << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << "\n";
    }
    std::cout << "Scene (incl Water) saved to " << filename << std::endl;
}

void SceneManager::loadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No scene file found." << std::endl;
        return;
    }

    // 1. Environment laden
    // (Einfacher Check: Wenn Datei leer ist, stürzt nichts ab)
    if(file >> env.waterHeight >> env.waterSpeed >> env.waterSteepness >> env.waterWavelength) {
        std::cout << "Environment loaded." << std::endl;
    } else {
        // Fallback falls Datei kaputt/alt
        env = EnvSettings();
    }

    // 2. Objekte laden
    objects.clear();
    size_t count;
    file >> count;

    for (size_t i = 0; i < count; i++) {
        SceneObject obj;
        file >> obj.modelKey >> obj.name;
        file >> obj.position.x >> obj.position.y >> obj.position.z;
        file >> obj.rotation.x >> obj.rotation.y >> obj.rotation.z;
        file >> obj.scale.x >> obj.scale.y >> obj.scale.z;

        if (loadedModels.find(obj.modelKey) != loadedModels.end()) {
            objects.push_back(obj);
        }
    }
    std::cout << "Scene loaded: " << objects.size() << " objects." << std::endl;
}