#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Shader.h"
#include "Model.h"

// Struktur für eine Pflanzenart (z.B. "Birch_Tree_1")
struct ForestObject {
    Model* model;                // Das geladene 3D-Modell
    std::vector<glm::mat4> transforms; // Liste aller Positionen für dieses Modell
};

class ForestSystem {
public:
    ForestSystem();
    ~ForestSystem();

    // Terrain-Daten übernehmen (kopiert aus GrassSystem)
    void initTerrainData(const std::vector<float>& vertices,
                         const std::vector<unsigned int>& indices,
                         float terrainScale);

    // Fügt ein Modell hinzu und verstreut es
    // path: Dateipfad (.glb)
    // amount: Wie viele davon?
    // scale: Basis-Größe
    // minDist: Mindestabstand zu anderen Objekten (gegen Überlappung)
    void addObject(const std::string& path, int amount, float scale, float minDist = 2.0f);

    // Zeichnet alles
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);

private:
    std::vector<ForestObject> objects;

    // Grid-Logik (Kopie von GrassSystem für Höhenbestimmung)
    struct GridCell {
        std::vector<int> indices;
    };
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
    glm::vec3 getNormalFromGrid(float x, float z);
    bool isForestSurface(float x, float z, float slopeLimitDegrees);

    // Hilfsfunktion: Prüft Abstand zu allen bereits platzierten Objekten
    bool checkDistance(float x, float z, float minDist);
    // Speichert alle Positionen aller Bäume für Kollisionsprüfung
    std::vector<glm::vec2> globalPositions;
};