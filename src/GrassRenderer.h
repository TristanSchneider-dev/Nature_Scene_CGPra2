#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"
#include "Terrain.h"

// Die drei Typen unserer Vegetation
enum VegetationType { GRASS, SINGLE_BLADE, LEAF };

class GrassRenderer {
public:
    // Konstruktor mit Seed (für Zufallsvariation) und Typ
    GrassRenderer(const Terrain& terrain, int amount, const std::string& texturePath, int seed, VegetationType type);
    ~GrassRenderer();

    void draw(const glm::mat4& view, const glm::mat4& projection);

    // Damit wir die Farben in der Main setzen können
    void setColors(glm::vec3 healthy, glm::vec3 dry);

private:
    unsigned int VAO, VBO, instanceVBO;
    unsigned int textureID;
    unsigned int amount;
    Shader* shader;

    // Farben
    glm::vec3 colHealthy;
    glm::vec3 colDry;

    VegetationType type;
    int seed;

    void initMesh();
    unsigned int loadTexture(const char* path);
    void generatePositions(const Terrain& terrain, int maxCount);
};