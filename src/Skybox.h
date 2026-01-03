#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

// Skybox.h
class Skybox {
public:
    Skybox(const std::vector<std::string>& dayFaces, const std::vector<std::string>& nightFaces);
    ~Skybox();

    void draw(const glm::mat4& view, const glm::mat4& projection);
    void setDay(bool day);

    // --- NEU HINZUFÜGEN ---
    void setNightFactor(float factor);
    // ----------------------

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int dayCubemapTexture;
    unsigned int nightCubemapTexture;
    Shader* skyboxShader; // Vermutlich heißt deine Variable hier so
    bool isDay = true;

    void setupMesh();
    unsigned int loadCubemap(const std::vector<std::string>& faces);
};