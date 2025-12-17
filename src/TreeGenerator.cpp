#include "TreeGenerator.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

// Noise-Funktion für natürliche Verteilung
float treeNoise(float x, float z, int seed) {
    float total = 0.0f;
    total += sin((x + seed * 7.3f) * 0.3f) * cos((z + seed * 3.1f) * 0.3f);
    total += sin(x * 0.8f - z * 0.5f) * 0.6f;
    total += cos(x * 1.2f + z * 0.9f) * 0.4f;
    return total;
}

TreeGenerator::TreeGenerator(const Terrain& terrain, int maxTrees, int seed)
    : seed(seed)
{
    srand(seed);
    generateTrees(terrain, maxTrees);
}

TreeGenerator::~TreeGenerator() {
    trees.clear();
}

void TreeGenerator::generateTrees(const Terrain& terrain, int maxCount) {
    trees.clear();

    const std::vector<float>& vertices = terrain.getVertices();
    int stride = 14; // Siehe Terrain.cpp: 14 floats pro Vertex
    size_t numVerts = vertices.size() / stride;

    std::cout << "[TreeGenerator] Generating trees from " << numVerts << " vertices..." << std::endl;

    // ⚙️ ANPASSUNG 1: Vertex-Abstand (höher = schneller, weniger dicht)
    // Nutze Member-Variable (kann über UI geändert werden)
    int step = m_step;  // Standard: 5 (1=sehr dicht, 10=sehr locker)

    for (size_t i = 0; i < numVerts; i += step) {
        size_t idx = i * stride;

        // Position auslesen
        float vx = vertices[idx + 0];
        float vy = vertices[idx + 1];
        float vz = vertices[idx + 2];

        // Normal auslesen (für Hangausrichtung)
        float nx = vertices[idx + 3];
        float ny = vertices[idx + 4];
        float nz = vertices[idx + 5];

        // Vertex Color (Blender Export: RGB)
        // R = Sand, G = Gras, B = Rock
        float rock = vertices[idx + 8]; // Blue Channel

        // --- BAUM-PLATZIERUNGS-LOGIK ---

        // 1. Nur auf geeignetem Terrain (rock > 0.3 = genug Vegetation)
        if (rock < 0.3f) continue;

        // 2. Nicht zu steil (Normale zu vertikal)
        if (ny < 0.7f) continue; // Hang zu steil

        // 3. Nicht im Wasser (Höhencheck)
        if (vy < -2.5f) continue;

        // 4. Noise-basierte Verteilung
        float noiseVal = treeNoise(vx, vz, seed);
        float n = (noiseVal + 1.5f) / 3.0f; // Normalisieren auf [0, 1]
        if (n < 0) n = 0; if (n > 1) n = 1;

        // ⚙️ ANPASSUNG 2: Baum-Dichte (höhere Potenz = seltener)
        // Nutze Member-Variable (kann über UI geändert werden)
        float treeDensity = pow(n, m_densityPower);  // Standard: 12.0 (8-15)

        if (treeDensity < 0.15f) continue; // Zu niedrige Dichte

        // 6. Zufalls-Check (1% Chance pro geeignetem Vertex)
        int chance = (int)(treeDensity * 100.0f);
        if ((rand() % 100) > chance) continue;

        // 7. Mindestabstand zu anderen Bäumen prüfen
        glm::vec3 pos(vx, vy, vz);
        if (!isValidTreePosition(pos, trees)) continue;

        // --- BAUM ERSTELLEN ---
        TreeInstance tree;
        tree.position = pos;
        tree.normal = glm::vec3(nx, ny, nz);

        // Größenvariation: 80% - 120%
        tree.scale = 0.8f + ((rand() % 40) / 100.0f);

        // Zufällige Y-Rotation
        tree.rotation = (rand() % 360);

        // Baum-Typ basierend auf Höhe und Rock-Wert
        tree.type = selectTreeType(rock, vy, noiseVal);

        trees.push_back(tree);

        // Limit erreicht?
        if (trees.size() >= maxCount) break;
    }

    std::cout << "[TreeGenerator] Generated " << trees.size() << " trees." << std::endl;
}

TreeType TreeGenerator::selectTreeType(float rockValue, float height, float noiseValue) {
    // Logik für Baum-Typ Auswahl:

    // Hohe Lagen (> 2.0) → Nadelbäume
    if (height > 2.0f) {
        return PINE_TREE;
    }

    // Trockene Gebiete (rock < 0.5) → Tote Bäume
    if (rockValue < 0.5f) {
        return DEAD_TREE;
    }

    // Mittlere Lagen → Mix aus Eiche und Kiefer
    float r = (rand() % 100) / 100.0f;

    if (r < 0.6f) {
        return OAK_TREE;
    }
    else {
        return PINE_TREE;
    }
}

bool TreeGenerator::isValidTreePosition(const glm::vec3& pos, const std::vector<TreeInstance>& existing) {
    // ⚙️ ANPASSUNG 3: Mindestabstand zwischen Bäumen
    // Nutze Member-Variable (kann über UI geändert werden)
    float minDistance = m_minDistance;  // Standard: 2.5f (1.5-5.0)

    for (const auto& tree : existing) {
        float dist = glm::distance(pos, tree.position);
        if (dist < minDistance) {
            return false;
        }
    }
    return true;
}

void TreeGenerator::addTree(glm::vec3 pos, TreeType type, float scale) {
    TreeInstance tree;
    tree.position = pos;
    tree.normal = glm::vec3(0, 1, 0);
    tree.scale = scale;
    tree.rotation = (rand() % 360);
    tree.type = type;
    trees.push_back(tree);
}

void TreeGenerator::drawTrees(Shader& shader,
    Model* pineModel,
    Model* oakModel,
    Model* deadModel)
{
    shader.use();

    for (const auto& tree : trees) {
        // Model Matrix berechnen
        glm::mat4 model = glm::mat4(1.0f);

        // 1. Position
        model = glm::translate(model, tree.position);

        // 2. Ausrichtung an Terrain-Normal (optional, für Hänge)
        // Hier vereinfacht: nur Y-Rotation
        model = glm::rotate(model, glm::radians(tree.rotation), glm::vec3(0, 1, 0));

        // 3. Skalierung
        model = glm::scale(model, glm::vec3(tree.scale));

        // Richtiges Modell auswählen
        Model* currentModel = nullptr;

        switch (tree.type) {
        case PINE_TREE:
            currentModel = pineModel;
            break;
        case OAK_TREE:
            currentModel = oakModel;
            break;
        case DEAD_TREE:
            currentModel = deadModel;
            break;
        }

        // Zeichnen (falls Modell geladen)
        if (currentModel) {
            currentModel->Draw(shader, model);
        }
    }
}

// Hilfsfunktionen (optional, für erweiterte Features)
float TreeGenerator::getTerrainHeight(const std::vector<float>& vertices, float x, float z) {
    // Vereinfachte Höhenabfrage (für genaue Positionierung)
    // TODO: Implementiere Barycentric Interpolation für präzise Höhe
    return 0.0f;
}

glm::vec3 TreeGenerator::getTerrainNormal(const std::vector<float>& vertices, float x, float z) {
    // Vereinfachte Normal-Abfrage
    // TODO: Interpoliere Normalen von umliegenden Vertices
    return glm::vec3(0, 1, 0);
}