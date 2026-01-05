#include "ForestSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <algorithm>

ForestSystem::ForestSystem() {}

ForestSystem::~ForestSystem() {
    for (auto& obj : objects) {
        delete obj.model;
    }
}

void ForestSystem::initTerrainData(const std::vector<float>& terrainVertices, const std::vector<unsigned int>& terrainIndices, float terrainScale) {
    grid.vertices = terrainVertices;
    grid.indices = terrainIndices;
    grid.minX = 100000.0f; grid.maxX = -100000.0f;
    grid.minZ = 100000.0f; grid.maxZ = -100000.0f;

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
    grid.minX -= 1.0f; grid.maxX += 1.0f; grid.minZ -= 1.0f; grid.maxZ += 1.0f;
    grid.resolution = 200;
    grid.cellWidth = (grid.maxX - grid.minX) / grid.resolution;
    grid.cellDepth = (grid.maxZ - grid.minZ) / grid.resolution;
    grid.cells.resize(grid.resolution * grid.resolution);

    for (size_t i = 0; i < grid.indices.size(); i += 3) {
        unsigned int idx1 = grid.indices[i];
        unsigned int idx2 = grid.indices[i+1];
        unsigned int idx3 = grid.indices[i+2];
        glm::vec3 p1(grid.vertices[idx1*11], 0, grid.vertices[idx1*11+2]);
        glm::vec3 p2(grid.vertices[idx2*11], 0, grid.vertices[idx2*11+2]);
        glm::vec3 p3(grid.vertices[idx3*11], 0, grid.vertices[idx3*11+2]);

        float minX = std::min({p1.x, p2.x, p3.x}); float maxX = std::max({p1.x, p2.x, p3.x});
        float minZ = std::min({p1.z, p2.z, p3.z}); float maxZ = std::max({p1.z, p2.z, p3.z});

        int startX = std::max(0, std::min(grid.resolution - 1, (int)((minX - grid.minX) / grid.cellWidth)));
        int endX   = std::max(0, std::min(grid.resolution - 1, (int)((maxX - grid.minX) / grid.cellWidth)));
        int startZ = std::max(0, std::min(grid.resolution - 1, (int)((minZ - grid.minZ) / grid.cellDepth)));
        int endZ   = std::max(0, std::min(grid.resolution - 1, (int)((maxZ - grid.minZ) / grid.cellDepth)));

        for (int x = startX; x <= endX; x++) {
            for (int z = startZ; z <= endZ; z++) {
                grid.cells[z * grid.resolution + x].push_back(i);
            }
        }
    }
    grid.isBuilt = true;
    std::cout << "[Forest] Terrain Grid initialisiert." << std::endl;
}

float ForestSystem::getYFromGrid(float x, float z) {
    if (!grid.isBuilt) return 0.0f;
    int gridX = (int)((x - grid.minX) / grid.cellWidth);
    int gridZ = (int)((z - grid.minZ) / grid.cellDepth);
    if (gridX < 0 || gridX >= grid.resolution || gridZ < 0 || gridZ >= grid.resolution) return -1000.0f;

    for (int idxStart : grid.cells[gridZ * grid.resolution + gridX]) {
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
        if (l1 >= 0.0f && l2 >= 0.0f && l3 >= 0.0f) return l1 * p1.y + l2 * p2.y + l3 * p3.y;
    }
    return -1000.0f;
}

glm::vec3 ForestSystem::getNormalFromGrid(float x, float z) {
    // Gibt (0,1,0) zurück, damit Bäume gerade wachsen und nicht schief am Hang stehen.
    // Wer schiefe Bäume will, muss hier die Interpolation aus GrassSystem kopieren.
    return glm::vec3(0,1,0);
}

bool ForestSystem::isForestSurface(float x, float z, float slopeLimitDegrees) {
    float y = getYFromGrid(x, z);
    if (y < -500.0f) return false;
    // Höhenbegrenzung für Wald
    if (y < -2.0f || y > 25.0f) return false;
    return true;
}

bool ForestSystem::checkDistance(float x, float z, float minDist) {
    for(const auto& pos : globalPositions) {
        float d = glm::distance(pos, glm::vec2(x, z));
        if (d < minDist) return false;
    }
    return true;
}

void ForestSystem::addObject(const std::string& path, int amount, float scale, float minDist) {
    if (!grid.isBuilt) {
        std::cout << "ERROR: Terrain nicht initialisiert!" << std::endl;
        return;
    }

    ForestObject newObj;
    std::cout << "Lade Modell: " << path << " ... ";
    newObj.model = new Model(path);
    std::cout << "Fertig." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(grid.minX + 10.0f, grid.maxX - 10.0f);
    std::uniform_real_distribution<float> disZ(grid.minZ + 10.0f, grid.maxZ - 10.0f);
    std::uniform_real_distribution<float> rotDis(0.0f, 360.0f);
    std::uniform_real_distribution<float> scaleVar(0.8f, 1.4f);

    int placed = 0;
    int attempts = 0;

    while (placed < amount && attempts < amount * 10) {
        attempts++;
        float x = disX(gen);
        float z = disZ(gen);

        if (!isForestSurface(x, z, 35.0f)) continue;
        if (!checkDistance(x, z, minDist)) continue;

        float y = getYFromGrid(x, z);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));

        // 1. Zufällige Rotation um Y (Varianz)
        model = glm::rotate(model, glm::radians(rotDis(gen)), glm::vec3(0.0f, 1.0f, 0.0f));

        // [FIX] 2. Modell aufrichten: GLB liegt oft flach (Z-Up), OpenGL ist Y-Up.
        // Wir drehen es -90 Grad um die X-Achse.
        model = glm::rotate(model, glm::radians(+90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        float s = scale * scaleVar(gen);
        model = glm::scale(model, glm::vec3(s));

        newObj.transforms.push_back(model);
        globalPositions.push_back(glm::vec2(x, z));
        placed++;
    }

    std::cout << "Platziert: " << placed << "/" << amount << " " << path << std::endl;
    objects.push_back(newObj);
}

void ForestSystem::draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("viewPos", viewPos);

    // [WICHTIG] Wir schalten Normal/ARM Maps aus, weil wir (noch) keine für die Bäume geladen haben.
    // Wenn das auf 'true' steht, aber keine Map da ist, wird alles schwarz!
    shader.setBool("useNormalMap", false);
    shader.setBool("useARMMap", false);

    for (auto& obj : objects) {
        for (const auto& modelMatrix : obj.transforms) {
            obj.model->Draw(shader, modelMatrix);
        }
    }
}