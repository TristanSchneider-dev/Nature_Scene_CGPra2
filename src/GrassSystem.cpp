#define GLM_ENABLE_EXPERIMENTAL
#include "GrassSystem.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

GrassSystem::GrassSystem() {
    // Shader laden
    shader = new Shader("../shaders/grass.vs.glsl", "../shaders/grass.fs.glsl");

    // Standard Quad
    float q[] = {
        -0.5f,  1.0f, 0.0f,  0.0f, 0.0f,
        -0.5f,  0.0f, 0.0f,  0.0f, 1.0f,
         0.5f,  0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  1.0f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.0f, 0.0f,  1.0f, 1.0f,
         0.5f,  1.0f, 0.0f,  1.0f, 0.0f
    };
    std::copy(std::begin(q), std::end(q), std::begin(quadVertices));
}

GrassSystem::~GrassSystem() {
    delete shader;
    for (auto& g : grassTypes) {
        glDeleteVertexArrays(1, &g.VAO);
        glDeleteBuffers(1, &g.VBO);
        glDeleteBuffers(1, &g.instanceVBO);
        glDeleteTextures(1, &g.textureID);
    }
}

// --- GITTER AUFBAU ---
void GrassSystem::initTerrainData(const std::vector<float>& terrainVertices,
                                  const std::vector<unsigned int>& terrainIndices,
                                  float terrainScale) {
    std::cout << "Baue Acceleration Grid..." << std::endl;

    grid.vertices = terrainVertices;
    grid.indices = terrainIndices;

    grid.minX = 100000.0f; grid.maxX = -100000.0f;
    grid.minZ = 100000.0f; grid.maxZ = -100000.0f;

    // Skalieren und Grenzen finden
    for (size_t i = 0; i < grid.vertices.size(); i += 11) {
        grid.vertices[i]   *= terrainScale;
        grid.vertices[i+1] *= terrainScale;
        grid.vertices[i+2] *= terrainScale;

        float x = grid.vertices[i];
        float z = grid.vertices[i+2];

        if (x < grid.minX) grid.minX = x;
        if (x > grid.maxX) grid.maxX = x;
        if (z < grid.minZ) grid.minZ = z;
        if (z > grid.maxZ) grid.maxZ = z;
    }

    grid.minX -= 1.0f; grid.maxX += 1.0f;
    grid.minZ -= 1.0f; grid.maxZ += 1.0f;

    grid.resolution = 200;
    grid.cellWidth = (grid.maxX - grid.minX) / grid.resolution;
    grid.cellDepth = (grid.maxZ - grid.minZ) / grid.resolution;

    grid.cells.clear();
    grid.cells.resize(grid.resolution * grid.resolution);

    // Dreiecke einsortieren
    for (size_t i = 0; i < grid.indices.size(); i += 3) {
        unsigned int idx1 = grid.indices[i];
        unsigned int idx2 = grid.indices[i+1];
        unsigned int idx3 = grid.indices[i+2];

        glm::vec3 p1(grid.vertices[idx1*11], grid.vertices[idx1*11+1], grid.vertices[idx1*11+2]);
        glm::vec3 p2(grid.vertices[idx2*11], grid.vertices[idx2*11+1], grid.vertices[idx2*11+2]);
        glm::vec3 p3(grid.vertices[idx3*11], grid.vertices[idx3*11+1], grid.vertices[idx3*11+2]);

        float triMinX = std::min({p1.x, p2.x, p3.x});
        float triMaxX = std::max({p1.x, p2.x, p3.x});
        float triMinZ = std::min({p1.z, p2.z, p3.z});
        float triMaxZ = std::max({p1.z, p2.z, p3.z});

        int startX = (int)((triMinX - grid.minX) / grid.cellWidth);
        int endX   = (int)((triMaxX - grid.minX) / grid.cellWidth);
        int startZ = (int)((triMinZ - grid.minZ) / grid.cellDepth);
        int endZ   = (int)((triMaxZ - grid.minZ) / grid.cellDepth);

        startX = std::max(0, std::min(grid.resolution - 1, startX));
        endX   = std::max(0, std::min(grid.resolution - 1, endX));
        startZ = std::max(0, std::min(grid.resolution - 1, startZ));
        endZ   = std::max(0, std::min(grid.resolution - 1, endZ));

        for (int x = startX; x <= endX; x++) {
            for (int z = startZ; z <= endZ; z++) {
                grid.cells[z * grid.resolution + x].push_back(i);
            }
        }
    }
    grid.isBuilt = true;
    std::cout << "Grid fertig." << std::endl;
}

// --- HÖHE FINDEN (Grid Lookup) ---
float GrassSystem::getYFromGrid(float x, float z) {
    if (!grid.isBuilt) return 0.0f;
    if (x < grid.minX || x > grid.maxX || z < grid.minZ || z > grid.maxZ) return -1000.0f;

    int gridX = (int)((x - grid.minX) / grid.cellWidth);
    int gridZ = (int)((z - grid.minZ) / grid.cellDepth);

    if (gridX < 0 || gridX >= grid.resolution || gridZ < 0 || gridZ >= grid.resolution) return -1000.0f;

    const auto& cellIndices = grid.cells[gridZ * grid.resolution + gridX];

    for (int idxStart : cellIndices) {
        unsigned int idx1 = grid.indices[idxStart];
        unsigned int idx2 = grid.indices[idxStart+1];
        unsigned int idx3 = grid.indices[idxStart+2];

        glm::vec3 p1(grid.vertices[idx1*11], grid.vertices[idx1*11+1], grid.vertices[idx1*11+2]);
        glm::vec3 p2(grid.vertices[idx2*11], grid.vertices[idx2*11+1], grid.vertices[idx2*11+2]);
        glm::vec3 p3(grid.vertices[idx3*11], grid.vertices[idx3*11+1], grid.vertices[idx3*11+2]);

        float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
        if (std::abs(det) < 0.001f) continue;

        float l1 = ((p2.z - p3.z) * (x - p3.x) + (p3.x - p2.x) * (z - p3.z)) / det;
        float l2 = ((p3.z - p1.z) * (x - p3.x) + (p1.x - p3.x) * (z - p3.z)) / det;
        float l3 = 1.0f - l1 - l2;

        if (l1 >= 0.0f && l2 >= 0.0f && l3 >= 0.0f) {
            return l1 * p1.y + l2 * p2.y + l3 * p3.y;
        }
    }
    return -1000.0f;
}

// --- NEU: NORMALE BERECHNEN ---
glm::vec3 GrassSystem::getNormalFromGrid(float x, float z) {
    if (!grid.isBuilt) return glm::vec3(0.0f, 1.0f, 0.0f);
    if (x < grid.minX || x > grid.maxX || z < grid.minZ || z > grid.maxZ)
        return glm::vec3(0.0f, 1.0f, 0.0f);

    int gridX = (int)((x - grid.minX) / grid.cellWidth);
    int gridZ = (int)((z - grid.minZ) / grid.cellDepth);

    if (gridX < 0 || gridX >= grid.resolution || gridZ < 0 || gridZ >= grid.resolution)
        return glm::vec3(0.0f, 1.0f, 0.0f);

    const auto& cellIndices = grid.cells[gridZ * grid.resolution + gridX];

    for (int idxStart : cellIndices) {
        unsigned int idx1 = grid.indices[idxStart];
        unsigned int idx2 = grid.indices[idxStart+1];
        unsigned int idx3 = grid.indices[idxStart+2];

        glm::vec3 p1(grid.vertices[idx1*11], grid.vertices[idx1*11+1], grid.vertices[idx1*11+2]);
        glm::vec3 p2(grid.vertices[idx2*11], grid.vertices[idx2*11+1], grid.vertices[idx2*11+2]);
        glm::vec3 p3(grid.vertices[idx3*11], grid.vertices[idx3*11+1], grid.vertices[idx3*11+2]);

        // Prüfe ob Punkt im Dreieck
        float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
        if (std::abs(det) < 0.001f) continue;

        float l1 = ((p2.z - p3.z) * (x - p3.x) + (p3.x - p2.x) * (z - p3.z)) / det;
        float l2 = ((p3.z - p1.z) * (x - p3.x) + (p1.x - p3.x) * (z - p3.z)) / det;
        float l3 = 1.0f - l1 - l2;

        if (l1 >= 0.0f && l2 >= 0.0f && l3 >= 0.0f) {
            // Hole die Normalen aus den Vertex-Daten (Offset 3,4,5)
            glm::vec3 n1(grid.vertices[idx1*11+3], grid.vertices[idx1*11+4], grid.vertices[idx1*11+5]);
            glm::vec3 n2(grid.vertices[idx2*11+3], grid.vertices[idx2*11+4], grid.vertices[idx2*11+5]);
            glm::vec3 n3(grid.vertices[idx3*11+3], grid.vertices[idx3*11+4], grid.vertices[idx3*11+5]);

            // Interpoliere die Normale
            glm::vec3 normal = l1 * n1 + l2 * n2 + l3 * n3;
            return glm::normalize(normal);
        }
    }
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

// --- NEU: PRÜFE OB GRASFLÄCHE (anhand der Y-Höhe des Terrains) ---
bool GrassSystem::isGrassSurface(float x, float z) {
    float y = getYFromGrid(x, z);
    if (y < -500.0f) return false;

    // Das Terrain hat unterschiedliche Höhen für verschiedene Materialien:
    // - Niedrige Bereiche (Pebbles): y < -5
    // - Mittlere Bereiche (Gras): -5 < y < 15
    // - Hohe Bereiche (Felsen): y > 15

    // Nur auf Gras spawnen (mittlere Höhe)
    if (y < -3.0f || y > 18.0f) return false;

    // Prüfe Steigung - kein Gras auf zu steilen Hängen
    glm::vec3 normal = getNormalFromGrid(x, z);
    float slope = std::acos(glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Maximal 45 Grad Neigung
    if (slope > glm::radians(45.0f)) return false;

    return true;
}

// --- NEU: VERBESSERTES NOISE FÜR NATÜRLICHERE VERTEILUNG ---
float GrassSystem::getDetailedNoise(float x, float z) {
    // Multi-Oktaven Perlin-ähnliches Noise
    float n = 0.0f;

    // Große Features
    n += std::sin(x * 0.03f) * std::cos(z * 0.03f) * 0.5f;

    // Mittlere Features
    n += std::sin(x * 0.1f + z * 0.07f) * 0.3f;
    n += std::cos(x * 0.08f - z * 0.12f) * 0.2f;

    // Kleine Details
    n += std::sin(x * 0.5f) * std::cos(z * 0.5f) * 0.15f;
    n += std::sin(x * 0.7f + z * 0.6f) * 0.1f;

    // Pseudo-Random Variation
    float hash = std::sin(x * 12.9898f + z * 78.233f) * 43758.5453f;
    n += (hash - std::floor(hash)) * 0.1f;

    return n;
}

// --- VERBESSERTE GRAS-PLATZIERUNG ---
void GrassSystem::addGrassType(const std::string& texturePath, int amount, float spreadRadius, float scale, bool isLeaf) {
    if (!grid.isBuilt) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-spreadRadius, spreadRadius);
    std::uniform_real_distribution<float> rotDis(0.0f, 360.0f);
    std::uniform_real_distribution<float> scaleVar(0.7f, 1.3f);
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    std::uniform_real_distribution<float> tilt(-5.0f, 5.0f); // Leichte zusätzliche Variation

    std::vector<glm::mat4> tempMatrices;
    tempMatrices.reserve(amount);

    int placed = 0;
    int attempts = 0;
    int maxAttempts = amount * 5; // Mehr Versuche für bessere Platzierung

    while (placed < amount && attempts < maxAttempts) {
        attempts++;
        float x = dis(gen);
        float z = dis(gen);

        // 1. MATERIAL-CHECK: Nur auf Grasflächen spawnen
        if (!isGrassSurface(x, z)) continue;

        // 2. NOISE-BASED DENSITY: Natürlichere Cluster
        float noise = getDetailedNoise(x, z);
        float density = (noise + 1.0f) * 0.5f; // Normalisiere auf 0-1

        // Verwende Noise für Wahrscheinlichkeit (mehr Gras in "hellen" Bereichen)
        if (prob(gen) > density * 0.8f + 0.2f) continue; // 20-100% Chance je nach Noise

        float y = getYFromGrid(x, z);
        if (y < -500.0f) continue;

        // 3. NORMALE-BASIERTE AUSRICHTUNG
        glm::vec3 normal = getNormalFromGrid(x, z);

        // Erstelle Transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));

        // Rotation um Y-Achse (zufällig)
        float yRotation = rotDis(gen);
        model = glm::rotate(model, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f));

        if (isLeaf) {
            // LEAFS: Flach auf den Boden legen, parallel zur Terrain-Oberfläche

            // Berechne Rotationsachse und Winkel um Leaf flach zu machen
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 rotAxis = glm::cross(up, normal);
            float rotAxisLen = glm::length(rotAxis);

            if (rotAxisLen > 0.001f) {
                rotAxis = glm::normalize(rotAxis);
                float angle = std::acos(glm::clamp(glm::dot(up, normal), -1.0f, 1.0f));

                // Wende die volle Rotation an um parallel zum Terrain zu sein
                model = glm::rotate(model, angle, rotAxis);
            }

            // Rotiere 90° um X-Achse um das Leaf flach hinzulegen
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            // Leichte zufällige Rotation für Variation
            model = glm::rotate(model, glm::radians(tilt(gen) * 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            // Hebe Leafs leicht an damit sie nicht komplett im Boden versinken
            model = glm::translate(model, glm::vec3(0.0f, 0.02f, 0.0f));

        } else {
            // GRAS: Aufrecht stehend, folgt der Terrain-Normale
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            // Berechne Rotationsachse und Winkel
            glm::vec3 rotAxis = glm::cross(up, normal);
            float rotAxisLen = glm::length(rotAxis);

            if (rotAxisLen > 0.001f) {
                rotAxis = glm::normalize(rotAxis);
                float angle = std::acos(glm::clamp(glm::dot(up, normal), -1.0f, 1.0f));

                // Wende die Rotation an (aber nicht zu extrem - 80% der vollen Neigung)
                model = glm::rotate(model, angle * 0.8f, rotAxis);
            }

            // Füge leichte zufällige Neigung für Natürlichkeit hinzu
            model = glm::rotate(model, glm::radians(tilt(gen)), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(tilt(gen)), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // Skalierung mit Variation
        float finalScale = scale * scaleVar(gen);
        model = glm::scale(model, glm::vec3(finalScale));

        tempMatrices.push_back(model);
        placed++;
    }

    if (placed == 0) {
        std::cout << "WARNUNG: Kein Gras platziert für " << texturePath << std::endl;
        return;
    }

    unsigned int texID = loadTexture(texturePath.c_str());
    GrassType newType(texID, placed);
    newType.modelMatrices = tempMatrices;

    setupBuffers(newType);
    grassTypes.push_back(newType);

    float successRate = (float)placed / (float)attempts * 100.0f;
    std::cout << "Gras platziert: " << placed << "/" << amount
              << " (" << successRate << "% Erfolgsrate) - " << texturePath << std::endl;
}

void GrassSystem::setupBuffers(GrassType& grass) {
    if (grass.modelMatrices.empty()) return;

    glGenVertexArrays(1, &grass.VAO);
    glGenBuffers(1, &grass.VBO);
    glGenBuffers(1, &grass.instanceVBO);

    glBindVertexArray(grass.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, grass.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, grass.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, grass.amount * sizeof(glm::mat4), &grass.modelMatrices[0], GL_STATIC_DRAW);

    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(i * vec4Size));
        glVertexAttribDivisor(3 + i, 1);
    }
    glBindVertexArray(0);
}

void GrassSystem::draw(const glm::mat4& view, const glm::mat4& projection, float time,
                       const glm::vec3& camPos, const glm::vec3& lightPos, const glm::vec3& lightColor) {
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setFloat("time", time);

    shader->setVec3("viewPos", camPos);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);

    shader->setInt("texture_diffuse1", 0);

    glDisable(GL_CULL_FACE);

    for (const auto& grass : grassTypes) {
        if(grass.amount == 0) continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass.textureID);

        glBindVertexArray(grass.VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, grass.amount);
        glBindVertexArray(0);
    }

    glEnable(GL_CULL_FACE);
}

unsigned int GrassSystem::loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}