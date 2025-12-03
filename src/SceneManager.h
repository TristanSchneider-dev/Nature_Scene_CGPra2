#pragma once

#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

// Wichtig: Forward Declaration oder Includes
#include "Model.h"
#include "Shader.h"

// Einstellungen für die Umgebung (Wasser)
struct EnvSettings {
    float waterHeight = -3.0f;
    float waterSpeed = 0.15f;
    float waterSteepness = 0.35f;
    float waterWavelength = 1.0f;
};

// Ein platziertes Objekt in der Welt
struct SceneObject {
    std::string name;       // Name in der UI Liste (z.B. "DeadTree_0")
    std::string modelKey;   // Welches 3D-Modell nutzt es? (z.B. "DeadTree")

    glm::vec3 position;
    glm::vec3 rotation;     // In Grad (Euler Angles)
    glm::vec3 scale;
};

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    // Lädt automatisch alle .glb/.gltf/.fbx/.obj aus einem Ordner
    // Ignoriert "landscape.fbx" automatisch.
    void loadAssetsFromFolder(const std::string& folderPath);

    // Manuelles Registrieren eines Modells (wird von loadAssetsFromFolder genutzt)
    void registerModel(std::string key, std::string path);

    // Erstellt eine neue Instanz eines Modells in der Szene
    void addInstance(std::string modelKey);

    // Zeichnet alle Objekte in der Szene
    void drawAll(Shader& shader);

    // Speichern & Laden (inklusive Wasser-Settings)
    void saveScene(const std::string& filename);
    void loadScene(const std::string& filename);

    // Getter für die UI
    std::vector<SceneObject>& getObjects() { return objects; }
    std::map<std::string, Model*>& getResources() { return loadedModels; }

    // Public Variables für UI Zugriff
    EnvSettings env;
    int selectedObjectID = -1; // Index des aktuell ausgewählten Objekts (-1 = keins)
    int getClosestObjectFromRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir);

private:
    // Ressourcen-Cache: Speichert jedes 3D-Modell nur EINMAL
    std::map<std::string, Model*> loadedModels;

    // Liste aller Objekte, die im Level platziert sind
    std::vector<SceneObject> objects;
};