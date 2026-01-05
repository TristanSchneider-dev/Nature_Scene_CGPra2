#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

#include "Shader.h"
#include "Model.h"

// Definition einer einzelnen Instanz (Position/Rotation/Scale als Matrix)
struct TreeInstance {
    glm::mat4 transform;
};

// Definition eines Wald-Typs (z.B. "Birch_Tree_1.glb")
// Enthält das 3D-Modell und eine Liste ALLER Positionen dieses Baums
struct ForestType {
    Model* model = nullptr;

    // CPU-Daten: Liste aller Instanzen
    std::vector<TreeInstance> instances;

    // GPU-Daten für Instancing (Performance!)
    unsigned int instanceVBO = 0;       // Der Puffer auf der Grafikkarte
    std::vector<glm::mat4> matrixCache; // Temporärer Speicher zum Hochladen
    bool isSetup = false;               // Müssen wir die Daten neu hochladen?
};

class ForestSystem {
public:
    ForestSystem();
    ~ForestSystem();

    // Initialisiert das Terrain-Grid für Höhenberechnung
    void initTerrainData(const std::vector<float>& vertices,
                         const std::vector<unsigned int>& indices,
                         float terrainScale);

    // Erstellt thematische Wälder ("Birch", "Pine", "Oak", "Scrub")
    void addBiomeCluster(const std::string& type, int groups, const std::string& assetPath);

    // Zeichnet alle Bäume (nutzt Instancing für Performance)
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);

private:
    // Lädt die Matrizen in den VBO (wird automatisch von draw aufgerufen)
    void updateInstances();

    // Hilfsfunktion: Platziert ein einzelnes Objekt
    void spawnObject(const std::string& path, float x, float z, float scale, float rotationVar);

    // Hilfsfunktion: Lädt Modell nur einmal (Caching)
    Model* getOrLoadModel(const std::string& path);

    // Speichert alle geladenen Modelle und ihre Instanzen
    std::map<std::string, ForestType> forestTypes;

    // Einfache Kollisionsvermeidung
    std::vector<glm::vec2> globalPositions;
    bool checkDistance(float x, float z, float minDist);

    // Grid-System für schnelle Höhenabfrage (wie beim Gras)
    struct Grid {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        float minX, maxX, minZ, maxZ;
        float cellWidth, cellDepth;
        int resolution;
        bool isBuilt = false;
        std::vector<std::vector<int>> cells;
    } grid;

    float getYFromGrid(float x, float z);
    bool isForestSurface(float x, float z);
};