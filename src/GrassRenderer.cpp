#include "GrassRenderer.h"
#include <stb_image.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

// Noise Funktion (bleibt gleich)
float organicNoise(float x, float z) {
    float total = 0.0f;
    total += sin(x * 0.5f + z * 0.2f) * cos(z * 0.5f - x * 0.2f);
    total += sin(x * 1.5f) * cos(z * 1.5f) * 0.5f;
    return total;
}

GrassRenderer::GrassRenderer(const Terrain& terrain, int amount, const std::string& texturePath, int seed, VegetationType type)
    : seed(seed), type(type)
{
    this->shader = new Shader("../shaders/grass.vs.glsl", "../shaders/grass.fs.glsl");
    this->textureID = loadTexture(texturePath.c_str());

    this->colHealthy = glm::vec3(0.34f, 0.40f, 0.05f);
    this->colDry     = glm::vec3(0.27f, 0.31f, 0.07f);

    initMesh();
    generatePositions(terrain, amount);

    // --- NEUES BUFFER LAYOUT ---
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Wir haben jetzt 7 Floats pro Instanz:
    // 3x Pos, 3x Normal, 1x Dryness
    GLsizei stride = 7 * sizeof(float);

    // Attribut 2: Position (vec3) -> Offset 0
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribDivisor(2, 1);

    // NEU: Attribut 3: Normal (vec3) -> Offset 3 floats
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // Attribut 4: Dryness (float) -> Offset 6 floats (war vorher Attribut 2.w)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
}

GrassRenderer::~GrassRenderer() {
    delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

// ... setColors bleibt gleich ...
void GrassRenderer::setColors(glm::vec3 healthy, glm::vec3 dry) {
    this->colHealthy = healthy;
    this->colDry = dry;
}

void GrassRenderer::generatePositions(const Terrain& terrain, int maxCount) {
    // Wir nutzen jetzt einen flachen float vector statt vec4
    std::vector<float> instanceData;

    const std::vector<float>& vertices = terrain.getVertices();
    int stride = 14;
    size_t numVerts = vertices.size() / stride;

    srand(seed);
    int step = 1;

    for(size_t i = 0; i < numVerts; i += step) {
        size_t idx = i * stride;

        // Position
        float vx = vertices[idx + 0];
        float vy = vertices[idx + 1];
        float vz = vertices[idx + 2];

        // NEU: Normale auslesen (Offset 3, 4, 5)
        float nx = vertices[idx + 3];
        float ny = vertices[idx + 4];
        float nz = vertices[idx + 5];

        float rock = vertices[idx + 8];

        if(rock > 0.4f) {
            float noiseVal = organicNoise(vx + seed * 13.0f, vz + seed * 13.0f);
            float n = (noiseVal + 1.5f) / 3.0f;
            if(n < 0) n = 0; if(n > 1) n = 1;

            float clumpVal = 0.0f;
            if(type == LEAF) clumpVal = pow(n, 15.0f);
            else if (type == SINGLE_BLADE) clumpVal = pow(n, 6.0f);
            else clumpVal = pow(n, 4.0f);

            if(clumpVal > 0.1f) {
                int chance = (int)(clumpVal * 100.0f);
                if(type == LEAF) chance /= 20;

                if((rand() % 100) < chance) {
                    float range = 0.1f;
                    float jitterX = ((rand() % 100) / 100.0f) * (range*2) - range;
                    float jitterZ = ((rand() % 100) / 100.0f) * (range*2) - range;

                    float dryness = (rock - 0.4f) * 1.5f;
                    float randomVar = ((rand() % 100) / 100.0f) * 0.4f - 0.2f;
                    dryness += randomVar;
                    if(dryness < 0.0f) dryness = 0.0f; if(dryness > 1.0f) dryness = 1.0f;

                    // --- DATEN SCHREIBEN (7 Floats) ---
                    // 1. Position
                    instanceData.push_back(vx + jitterX);
                    instanceData.push_back(vy);
                    instanceData.push_back(vz + jitterZ);

                    // 2. Normale (für Ausrichtung)
                    instanceData.push_back(nx);
                    instanceData.push_back(ny);
                    instanceData.push_back(nz);

                    // 3. Dryness
                    instanceData.push_back(dryness);
                }
            }
        }
        // Check: maxCount * 7, da wir 7 floats pro Instanz haben
        if(instanceData.size() >= maxCount * 7) break;
    }

    // Wichtig: Amount ist Anzahl der Instanzen, also Vektorgröße / 7
    this->amount = instanceData.size() / 7;

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float), instanceData.empty() ? 0 : instanceData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ... draw, initMesh, loadTexture bleiben GLEICH wie in der letzten Version ...
// (Kopiere sie einfach aus dem vorherigen Schritt, die ändern sich nicht, außer draw->TextureID fix)
void GrassRenderer::draw(const glm::mat4& view, const glm::mat4& projection) {
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("colorHealthy", colHealthy);
    shader->setVec3("colorDry",     colDry);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader->setInt("grassTexture", 0);

    glBindVertexArray(VAO);
    int verts = 0;
    if(type == GRASS) verts = 12;
    if(type == SINGLE_BLADE) verts = 6;
    if(type == LEAF) verts = 6;
    glDrawArraysInstanced(GL_TRIANGLES, 0, verts, amount);
    glBindVertexArray(0);
}

void GrassRenderer::initMesh() {
    float vertices[60];
    if (type == GRASS) {
        float s = 0.06f; float h = 0.10f;
        float v[] = { -s,0,0,0,0, s,0,0,1,0, s,h,0,1,1, s,h,0,1,1, -s,h,0,0,1, -s,0,0,0,0,
                       0,0,-s,0,0, 0,0,s,1,0, 0,h,s,1,1, 0,h,s,1,1, 0,h,-s,0,1, 0,0,-s,0,0 };
        for(int i=0; i<60; i++) vertices[i] = v[i];
    } else if (type == SINGLE_BLADE) {
        float s = 0.03f; float h = 0.12f;
        float v[] = { -s,0,0,0,0, s,0,0,1,0, s,h,0,1,1, s,h,0,1,1, -s,h,0,0,1, -s,0,0,0,0 };
        for(int i=0; i<30; i++) vertices[i] = v[i];
    } else { // LEAF
        float s = 0.15f; float yOff = 0.02f;
        float v[] = { -s,yOff,-s,0,0, s,yOff,-s,1,0, s,yOff,s,1,1, s,yOff,s,1,1, -s,yOff,s,0,1, -s,yOff,-s,0,0 };
        for(int i=0; i<30; i++) vertices[i] = v[i];
    }
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    int size = (type == GRASS) ? sizeof(float)*60 : sizeof(float)*30;
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

unsigned int GrassRenderer::loadTexture(const char* path) {
    unsigned int id; glGenTextures(1, &id); stbi_set_flip_vertically_on_load(true);
    int w, h, nrChannels; unsigned char *data = stbi_load(path, &w, &h, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    stbi_image_free(data);
    return id;
}