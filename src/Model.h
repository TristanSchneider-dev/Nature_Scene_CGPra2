#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

class Model {
public:
    Model(const std::string& path);
    ~Model();

    void setTextures(const std::string& albedoPath, const std::string& normalPath, const std::string& armPath);
    void setTexturesFromRoughness(const std::string& albedoPath, const std::string& normalPath, const std::string& roughnessPath);
    void draw(Shader& shader, glm::mat4 modelMatrix);

private:
    unsigned int VAO, VBO, EBO, indexCount;
    unsigned int tAlbedo, tNormal, tARM;

    void loadModel(const std::string& path);
    unsigned int loadTexture(const char* path);
    unsigned int createARMFromRoughness(const char* path);

    unsigned int createWhiteTexture();
    unsigned int createFlatNormalTexture();
    unsigned int createDefaultARMTexture();
};