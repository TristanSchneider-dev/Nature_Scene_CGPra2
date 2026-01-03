#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

class Skybox {
public:
    // Konstruktor akzeptiert nun zwei Sets von Pfaden
    Skybox(const std::vector<std::string>& dayFaces, const std::vector<std::string>& nightFaces);
    ~Skybox();

    void draw(const glm::mat4& view, const glm::mat4& projection);

    // Toggle Funktion
    void setDay(bool isDay);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int dayCubemapTexture;
    unsigned int nightCubemapTexture;

    bool isDay = true; // Aktueller Zustand

    Shader* skyboxShader;

    unsigned int loadCubemap(const std::vector<std::string>& faces);
    void setupMesh();
};