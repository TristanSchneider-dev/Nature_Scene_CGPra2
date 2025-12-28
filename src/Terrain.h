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

    // --- WIEDER EINGEFÜGT: Getter für Geometrie-Daten ---
    const std::vector<float>& getVertices() const { return m_vertices; }

    // Optional: Speicher freigeben, wenn nicht mehr benötigt
    void freeClientMemory() {
        m_vertices.clear();
        m_vertices.shrink_to_fit();
    }
    // ----------------------------------------------------

private:
    unsigned int VAO, VBO, EBO, indexCount;

    // Unsere 3 Material-Sets
    TerrainMaterial matPebbles;
    TerrainMaterial matGround;
    TerrainMaterial matRock;

    std::vector<float> m_vertices;

    void loadModel(const std::string& path);
    void loadMaterials();
    unsigned int loadTexture(const char* path);
};