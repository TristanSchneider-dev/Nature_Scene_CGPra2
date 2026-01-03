#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include "Shader.h"

struct TerrainMaterial {
    unsigned int albedo;
    unsigned int normal;
    unsigned int arm;
};

class Terrain {
public:
    Terrain(const std::string& modelPath);
    ~Terrain();

    void draw(Shader& shader);

    // Diese Getter braucht dein GrassSystem in der main.cpp!
    const std::vector<float>& getVertices() const { return m_vertices; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0, indexCount = 0;

    // Materialien
    TerrainMaterial matPebbles;
    TerrainMaterial matGround;
    TerrainMaterial matRock;

    // CPU-Speicher der Geometrie (Wichtig für Physics/Gras)
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;

    // Interne Helper
    void loadModel(const std::string& path);
    void loadMaterials();
    unsigned int loadTexture(const char* path);
};