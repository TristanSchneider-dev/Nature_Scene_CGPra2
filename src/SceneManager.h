#pragma once
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

// NEU: Container für globale Level-Settings
struct EnvSettings {
    float waterHeight = -3.0f;
    float waterSpeed = 0.15f;
    float waterSteepness = 0.35f;
    float waterWavelength = 1.0f;
};

struct SceneObject {
    std::string name;
    std::string modelKey;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void registerModel(std::string key, std::string path);
    void addInstance(std::string modelKey);
    void drawAll(Shader& shader);

    void saveScene(const std::string& filename);
    void loadScene(const std::string& filename);

    std::vector<SceneObject>& getObjects() { return objects; }
    std::map<std::string, Model*>& getResources() { return loadedModels; }

    // Zugriff auf Settings
    EnvSettings env; // Public für direkten UI Zugriff
    int selectedObjectID = -1;

private:
    std::map<std::string, Model*> loadedModels;
    std::vector<SceneObject> objects;
};