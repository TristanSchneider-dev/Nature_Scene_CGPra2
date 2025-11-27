#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

class Terrain {
public:
    Terrain(const std::string& modelPath);
    ~Terrain();

    void draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    // Textur-IDs
    unsigned int tPebbleD, tPebbleN, tPebbleARM; // Jetzt auf Slot 0-2 (ehemals Gravel)
    unsigned int tGravelD, tGravelN, tGravelARM; // Jetzt auf Slot 3-5 (ehemals Pebble)
    unsigned int tRockD, tRockN, tRockARM;

    void loadModel(const std::string& path);
    void loadTextures();
    unsigned int loadTexture(const char* path);
    unsigned int createDefaultARM(float ao, float roughness, float metallic);
};