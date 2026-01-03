#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

struct GrassType {
    unsigned int textureID;
    unsigned int VAO, VBO, instanceVBO;
    std::vector<glm::mat4> modelMatrices;
    int amount;

    GrassType(unsigned int texID, int count)
        : textureID(texID), amount(count), VAO(0), VBO(0), instanceVBO(0) {}
};

// Beschleunigungs-Gitter Struktur
struct AccelerationGrid {
    float minX, maxX, minZ, maxZ;
    float cellWidth, cellDepth;
    int resolution = 100;
    std::vector<std::vector<int>> cells;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    bool isBuilt = false;
};

class GrassSystem {
public:
    GrassSystem();
    ~GrassSystem();

    // 1. Terrain Daten laden (baut das Grid)
    void initTerrainData(const std::vector<float>& terrainVertices,
                         const std::vector<unsigned int>& terrainIndices,
                         float terrainScale);

    // 2. Gras hinzufügen (nutzt das Grid)
    void addGrassType(const std::string& texturePath, int amount, float spreadRadius, float scale, bool isLeaf = false);

    // 3. Zeichnen (Update: Jetzt mit Licht-Infos!)
    void draw(const glm::mat4& view, const glm::mat4& projection, float time,
              const glm::vec3& camPos, const glm::vec3& lightPos, const glm::vec3& lightColor);

private:
    Shader* shader;
    std::vector<GrassType> grassTypes;
    AccelerationGrid grid;

    float quadVertices[30];

    unsigned int loadTexture(const char* path);
    void setupBuffers(GrassType& grass);
    float getYFromGrid(float x, float z);

    // Neue Methoden für bessere Platzierung
    glm::vec3 getNormalFromGrid(float x, float z);
    bool isGrassSurface(float x, float z);
    float getDetailedNoise(float x, float z);
};