#include "GrassRenderer.h"
// Falls stb_image noch nicht in main implementiert wurde,
// müsstest du hier #define STB_IMAGE_IMPLEMENTATION nutzen.
// Aber wir gehen davon aus, dass es in main.cpp ist.
#include <stb_image.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

// Organischer Noise für natürliche Verteilung
// Addiert zwei Sinuswellen für "krumme" Muster
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

    // Standardfarben setzen
    this->colHealthy = glm::vec3(0.34f, 0.40f, 0.05f);
    this->colDry     = glm::vec3(0.27f, 0.31f, 0.07f);

    // 1. Mesh bauen (Gras-Kreuz, Einzelhalm oder Blatt)
    initMesh();

    // 2. Positionen berechnen (basierend auf Noise & Terrain)
    generatePositions(terrain, amount);

    // 3. Instanz-Daten (Positionen) verknüpfen
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Attribut 2: vec4 (x, y, z, dryness)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1); // 1x pro Instanz

    glBindVertexArray(0);
}

GrassRenderer::~GrassRenderer() {
    delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

void GrassRenderer::setColors(glm::vec3 healthy, glm::vec3 dry) {
    this->colHealthy = healthy;
    this->colDry = dry;
}

void GrassRenderer::generatePositions(const Terrain& terrain, int maxCount) {
    std::vector<glm::vec4> instanceData;
    const std::vector<float>& vertices = terrain.getVertices();
    int stride = 14;
    size_t numVerts = vertices.size() / stride;

    // Seed setzen für Variation zwischen den Renderer-Instanzen
    srand(seed);

    int step = 1;

    for(size_t i = 0; i < numVerts; i += step) {
        size_t idx = i * stride;

        float vx = vertices[idx + 0];
        float vy = vertices[idx + 1];
        float vz = vertices[idx + 2];
        float rock = vertices[idx + 8]; // Blau-Kanal (Fels)

        // Vegetation wächst auf Fels/Wiese (Blau > 0.4)
        if(rock > 0.4f) {

            // Noise holen (verschoben durch Seed)
            float noiseVal = organicNoise(vx + seed * 13.0f, vz + seed * 13.0f);

            // Normalisieren 0..1
            float n = (noiseVal + 1.5f) / 3.0f;
            if(n < 0) n = 0; if(n > 1) n = 1;

            // --- Seltenheits-Steuerung ---
            float clumpVal = 0.0f;

            if(type == LEAF) {
                // Blätter sehr selten (exponentiell)
                clumpVal = pow(n, 15.0f);
            }
            else if (type == SINGLE_BLADE) {
                // Einzelhalme mittel-selten (Auflockerung)
                clumpVal = pow(n, 6.0f);
            }
            else {
                // Hauptgras (Teppich)
                clumpVal = pow(n, 4.0f);
            }

            // Schwellwert
            if(clumpVal > 0.1f) {

                int chance = (int)(clumpVal * 100.0f);

                // Blätter zusätzlich stark ausdünnen
                if(type == LEAF) chance /= 20;

                if((rand() % 100) < chance) {

                    float range = 0.1f;
                    float jitterX = ((rand() % 100) / 100.0f) * (range*2) - range;
                    float jitterZ = ((rand() % 100) / 100.0f) * (range*2) - range;

                    // Farbe berechnen
                    float dryness = (rock - 0.4f) * 1.5f;
                    float randomVar = ((rand() % 100) / 100.0f) * 0.4f - 0.2f;
                    dryness += randomVar;
                    if(dryness < 0.0f) dryness = 0.0f;
                    if(dryness > 1.0f) dryness = 1.0f;

                    instanceData.push_back(glm::vec4(vx + jitterX, vy, vz + jitterZ, dryness));
                }
            }
        }
        if(instanceData.size() >= maxCount) break;
    }

    this->amount = instanceData.size();

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(glm::vec4), instanceData.empty() ? 0 : instanceData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GrassRenderer::draw(const glm::mat4& view, const glm::mat4& projection) {
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    shader->setVec3("colorHealthy", colHealthy);
    shader->setVec3("colorDry",     colDry);

    // --- TEXTUR BINDING FIX ---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Dem Shader explizit sagen: "grassTexture" ist auf Slot 0!
    shader->setInt("grassTexture", 0);

    glBindVertexArray(VAO);

    // Anzahl Vertices wählen
    int verts = 0;
    if(type == GRASS) verts = 12;       // Kreuz (2 Quads)
    if(type == SINGLE_BLADE) verts = 6; // Einzeln (1 Quad)
    if(type == LEAF) verts = 6;         // Blatt (1 Quad)

    glDrawArraysInstanced(GL_TRIANGLES, 0, verts, amount);
    glBindVertexArray(0);
}

void GrassRenderer::initMesh() {
    // Array groß genug für alle Fälle (max 12 * 5 floats)
    float vertices[60];

    if (type == GRASS) {
        // --- 1. HAUPTGRAS (Kreuz) ---
        float s = 0.06f;
        float h = 0.10f;
        float v[] = {
            -s, 0.0f, 0.0f,  0.0f, 0.0f,  s, 0.0f, 0.0f,  1.0f, 0.0f,  s, h, 0.0f,  1.0f, 1.0f,
             s, h, 0.0f,  1.0f, 1.0f, -s, h, 0.0f,  0.0f, 1.0f, -s, 0.0f, 0.0f,  0.0f, 0.0f,
             0.0f, 0.0f, -s,  0.0f, 0.0f, 0.0f, 0.0f,  s,  1.0f, 0.0f, 0.0f, h,  s,  1.0f, 1.0f,
             0.0f, h,  s,  1.0f, 1.0f, 0.0f, h, -s,  0.0f, 1.0f, 0.0f, 0.0f, -s,  0.0f, 0.0f
        };
        for(int i=0; i<60; i++) vertices[i] = v[i];
    }
    else if (type == SINGLE_BLADE) {
        // --- 2. EINZELHALM (Schmaler) ---
        float s = 0.03f;
        float h = 0.12f;
        float v[] = {
            -s, 0.0f, 0.0f,  0.0f, 0.0f,
             s, 0.0f, 0.0f,  1.0f, 0.0f,
             s,    h, 0.0f,  1.0f, 1.0f,
             s,    h, 0.0f,  1.0f, 1.0f,
            -s,    h, 0.0f,  0.0f, 1.0f,
            -s, 0.0f, 0.0f,  0.0f, 0.0f
        };
        for(int i=0; i<30; i++) vertices[i] = v[i];
    }
    else { // LEAF
        // --- 3. BLATT (Liegend) ---
        // Vergrößert: s von 0.05 auf 0.15
        float s = 0.15f;
        float yOff = 0.02f; // Leicht über Boden gegen Z-Fighting
        float v[] = {
            -s, yOff, -s, 0.0f, 0.0f,  s, yOff, -s, 1.0f, 0.0f,  s, yOff,  s, 1.0f, 1.0f,
             s, yOff,  s, 1.0f, 1.0f, -s, yOff,  s, 0.0f, 1.0f, -s, yOff, -s, 0.0f, 0.0f
        };
        for(int i=0; i<30; i++) vertices[i] = v[i];
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Größe berechnen
    int size = (type == GRASS) ? sizeof(float)*60 : sizeof(float)*30;
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Attribut 0: Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Attribut 1: UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

unsigned int GrassRenderer::loadTexture(const char* path) {
    unsigned int id;
    glGenTextures(1, &id);
    stbi_set_flip_vertically_on_load(true);
    int w, h, nrChannels;
    unsigned char *data = stbi_load(path, &w, &h, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Failed to load grass texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return id;
}