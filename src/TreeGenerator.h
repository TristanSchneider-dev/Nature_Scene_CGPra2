#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"
#include "Terrain.h"

// Verschiedene Baum-Typen
enum TreeType {
    PINE_TREE,      // Nadelbaum
    OAK_TREE,       // Laubbaum
    DEAD_TREE,      // Toter Baum
};

// Ein einzelner generierter Baum
struct TreeInstance {
    glm::vec3 position;
    glm::vec3 normal;      // Für Ausrichtung am Hang
    float scale;           // Größenvariation
    float rotation;        // Y-Rotation in Grad
    TreeType type;
};

class TreeGenerator {
public:
    TreeGenerator(const Terrain& terrain, int maxTrees, int seed);
    ~TreeGenerator();

    // Hauptfunktion: Generiert Bäume auf dem Terrain
    void generateTrees(const Terrain& terrain, int maxCount);

    // Zeichnet alle Baum-Instanzen (ruft Model-System auf)
    void drawTrees(Shader& shader,
        class Model* pineModel,
        class Model* oakModel,
        class Model* deadModel);

    // Getter für die generierten Bäume (z.B. für SceneManager Export)
    const std::vector<TreeInstance>& getTrees() const { return trees; }

    // Manuell einen Baum hinzufügen
    void addTree(glm::vec3 pos, TreeType type, float scale = 1.0f);

    // Alle Bäume löschen
    void clear() { trees.clear(); }

    // NEU: Setter für UI-Parameter
    void setParameters(int step, float densityPower, float minDist) {
        m_step = step;
        m_densityPower = densityPower;
        m_minDistance = minDist;
    }

private:
    std::vector<TreeInstance> trees;
    int seed;

    // Generierungs-Parameter (können von außen gesetzt werden)
    int m_step = 5;
    float m_densityPower = 12.0f;
    float m_minDistance = 2.5f;

    // Hilfsfunktionen
    float getTerrainHeight(const std::vector<float>& vertices, float x, float z);
    glm::vec3 getTerrainNormal(const std::vector<float>& vertices, float x, float z);
    TreeType selectTreeType(float rockValue, float height, float noiseValue);
    bool isValidTreePosition(const glm::vec3& pos, const std::vector<TreeInstance>& existing);
};