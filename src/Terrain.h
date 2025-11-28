#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include "Shader.h"

class Terrain {
public:
    Terrain(const std::string& modelPath);
    ~Terrain();

    void draw(Shader& shader);

    // NEU: Nur noch ein Getter. Das Terrain rechnet nicht mehr selbst.
    const std::vector<float>& getVertices() const { return m_vertices; }

private:
    unsigned int VAO, VBO, EBO, indexCount;
    unsigned int tPebbleD, tPebbleN, tPebbleARM;
    unsigned int tGravelD, tGravelN, tGravelARM;
    unsigned int tRockD, tRockN, tRockARM;

    std::vector<float> m_vertices; // Die rohen Daten

    void loadModel(const std::string& path);
    void loadTextures();
    unsigned int loadTexture(const char* path);
    unsigned int createDefaultARM(float ao, float roughness, float metallic);
};