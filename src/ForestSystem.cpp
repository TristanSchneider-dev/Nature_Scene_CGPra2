#include "ForestSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <algorithm>

ForestSystem::ForestSystem() {}

ForestSystem::~ForestSystem() {
    for (auto& entry : forestTypes) {
        // GPU Buffer aufräumen
        if (entry.second.instanceVBO != 0) {
            glDeleteBuffers(1, &entry.second.instanceVBO);
        }
        delete entry.second.model;
    }
}

// --- 1. TERRAIN DATEN & GRID ---

void ForestSystem::initTerrainData(const std::vector<float>& terrainVertices, const std::vector<unsigned int>& terrainIndices, float terrainScale) {
    grid.vertices = terrainVertices;
    grid.indices = terrainIndices;
    grid.minX = 100000.0f; grid.maxX = -100000.0f;
    grid.minZ = 100000.0f; grid.maxZ = -100000.0f;

    // Bounds berechnen & Skalieren
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

    // Grid aufbauen
    grid.resolution = 200;
    grid.cellWidth = (grid.maxX - grid.minX) / grid.resolution;
    grid.cellDepth = (grid.maxZ - grid.minZ) / grid.resolution;
    grid.cells.resize(grid.resolution * grid.resolution);

    for (size_t i = 0; i < grid.indices.size(); i += 3) {
        unsigned int idx1 = grid.indices[i];
        unsigned int idx2 = grid.indices[i+1];
        unsigned int idx3 = grid.indices[i+2];

        // Vereinfacht: Nur X/Z prüfen für Zuweisung
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

        if (l1 >= 0.0f && l2 >= 0.0f && l3 >= 0.0f) {
            return l1 * p1.y + l2 * p2.y + l3 * p3.y;
        }
    }
    return -1000.0f;
}

bool ForestSystem::isForestSurface(float x, float z) {
    float y = getYFromGrid(x, z);
    if (y < -500.0f) return false;
    // Höhenbegrenzung: Nicht im Wasser, nicht auf Gipfeln
    if (y < 2.0f || y > 35.0f) return false;
    return true;
}

bool ForestSystem::checkDistance(float x, float z, float minDist) {
    for(const auto& pos : globalPositions) {
        // Performance: Squared distance check wäre schneller, aber distance reicht hier
        if (glm::distance(pos, glm::vec2(x, z)) < minDist) return false;
    }
    return true;
}

Model* ForestSystem::getOrLoadModel(const std::string& path) {
    if (forestTypes.find(path) == forestTypes.end()) {
        std::cout << "Lade Asset: " << path << std::endl;
        ForestType newType;
        newType.model = new Model(path);
        forestTypes[path] = newType;
    }
    return forestTypes[path].model;
}

// --- 2. SPAWNING LOGIK ---

void ForestSystem::spawnObject(const std::string& path, float x, float z, float scale, float rotationVar) {
    if (!isForestSurface(x, z)) return;

    getOrLoadModel(path);
    float y = getYFromGrid(x, z);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));

    // Zufalls-Rotation Y
    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // [WICHTIG] +90 Grad X-Rotation für aufrechte Bäume
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Skalierung + leichte Variation
    float s = scale * (0.8f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.4f);
    model = glm::scale(model, glm::vec3(s));

    // CPU-Liste füllen
    TreeInstance instance;
    instance.transform = model;
    forestTypes[path].instances.push_back(instance);

    // Flag setzen, dass GPU-Daten veraltet sind
    forestTypes[path].isSetup = false;

    globalPositions.push_back(glm::vec2(x, z));
}

// --- NEUE FUNKTION: Simuliertes Perlin-Noise für Biome ---
// Gibt Werte zwischen ca. -1.0 und 1.0 zurück.
// Erzeugt organische, große Formen.
float getBiomeNoise(float x, float z) {
    // Frequenz: Je kleiner die Zahl, desto GRÖSSER die Wälder.
    // 0.01 ist gut für Maps um die 1000 Einheiten.
    float freq = 0.008f;

    // Wir kombinieren zwei Sinus-Wellen für organischere Formen
    float val = std::sin(x * freq) * std::cos(z * freq);
    val += 0.5f * std::sin((x + z) * freq * 2.0f); // Zweite Lage für Details

    return val;
}

void ForestSystem::addBiomeCluster(const std::string& type, int groups, const std::string& assetPath) {
    if (!grid.isBuilt) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    // Randbereich nutzen
    std::uniform_real_distribution<float> disX(grid.minX + 10.0f, grid.maxX - 10.0f);
    std::uniform_real_distribution<float> disZ(grid.minZ + 10.0f, grid.maxZ - 10.0f);

    // Radius groß lassen für fließende Übergänge
    std::normal_distribution<float> clusterDist(0.0f, 16.0f);

    int groupsCreated = 0;
    int maxAttempts = groups * 50; // Sicherheitsabbruch
    int attempts = 0;

    while (groupsCreated < groups && attempts < maxAttempts) {
        attempts++;

        // 1. Wähle zufälligen Mittelpunkt
        float centerX = disX(gen);
        float centerZ = disZ(gen);

        if (!isForestSurface(centerX, centerZ)) continue;

        // 2. [NEU] BIOME CHECK - "Darf dieser Wald hier wachsen?"
        float noise = getBiomeNoise(centerX, centerZ);
        bool validSpot = false;

        // --- REGELN FÜR DIE BIOME ---
        // Noise geht von ca -1.5 bis +1.5

        if (type == "Birch") {
            // Birken mögen hohe Werte (z.B. > 0.3)
            if (noise > 0.35f) validSpot = true;
        }
        else if (type == "Pine") {
            // Kiefern mögen niedrige Werte (z.B. < -0.3)
            if (noise < -0.35f) validSpot = true;
        }
        else if (type == "Oak") {
            // Eichen wachsen im Übergangsbereich (Mitte)
            if (noise >= -0.3f && noise <= 0.3f) validSpot = true;
        }
        else if (type == "Scrub") {
            // Scrub darf ÜBERALL hin, um Lücken zu füllen!
            // Wir geben ihm nur eine kleine Chance zu failen, damit es nicht zu gleichmäßig ist
            validSpot = true;
        }

        // Wenn der Ort nicht zum Biome passt -> Neuer Versuch!
        if (!validSpot) continue;

        groupsCreated++;

        // --- AB HIER IST ALLES WIE VORHER (Spawning Logic) ---

        if (type == "Birch") {
            // Birkenwald (Sehr dicht)
            int treeCount = 25 + rand() % 10;
            for(int t=0; t<treeCount; t++) {
                float ox = centerX + clusterDist(gen);
                float oz = centerZ + clusterDist(gen);
                std::string treeName = (rand()%2 == 0) ? "Birch_Tree_1.glb" : "Birch_Tree_2.glb";
                if(checkDistance(ox, oz, 0.3f)) spawnObject(assetPath + treeName, ox, oz, 0.0025f, 360.0f);
            }
            // Junge Birken
            for(int t=0; t<25; t++) {
                float ox = centerX + clusterDist(gen);
                float oz = centerZ + clusterDist(gen);
                if(checkDistance(ox, oz, 0.2f)) spawnObject(assetPath + "Young_Birch_Tree_1.glb", ox, oz, 0.0015f, 360.0f);
            }
            // Unterholz
            int bushCount = 60 + rand() % 20;
            for(int b=0; b<bushCount; b++) {
                float ox = centerX + clusterDist(gen) * 1.5f;
                float oz = centerZ + clusterDist(gen) * 1.5f;
                if(rand()%2==0) { if(checkDistance(ox, oz, 0.25f)) spawnObject(assetPath + "Blackberry_Bush_1a.glb", ox, oz, 0.0015f, 360.0f); }
                else { if(checkDistance(ox, oz, 0.2f)) spawnObject(assetPath + "Fern_1a.glb", ox, oz, 0.002f, 360.0f); }
            }
        }
        else if (type == "Pine") {
            // Nadelwald (Dicht & Dunkel)
            int treeCount = 35 + rand() % 10;
            for(int t=0; t<treeCount; t++) {
                float ox = centerX + clusterDist(gen);
                float oz = centerZ + clusterDist(gen);
                std::string treeName;
                int roll = rand() % 3;
                if(roll == 0) treeName = "Pine_Tree_1.glb"; else if(roll == 1) treeName = "Pine_Tree_2.glb"; else treeName = "Fir_Tree_1.glb";
                if(checkDistance(ox, oz, 0.25f)) spawnObject(assetPath + treeName, ox, oz, 0.0025f, 360.0f);
            }
            // Junge Tannen
            for(int t=0; t<25; t++) {
                float ox = centerX + clusterDist(gen); float oz = centerZ + clusterDist(gen);
                if(checkDistance(ox, oz, 0.2f)) spawnObject(assetPath + "Young_Fir_Tree_1.glb", ox, oz, 0.0015f, 360.0f);
            }
            // Details
            int detailCount = 50 + rand() % 20;
            for(int d=0; d<detailCount; d++) {
                float ox = centerX + clusterDist(gen); float oz = centerZ + clusterDist(gen);
                if(rand()%2==0) { if(checkDistance(ox, oz, 0.4f)) spawnObject(assetPath + "Rock_1.glb", ox, oz, 0.005f, 360.0f); }
                else { if(checkDistance(ox, oz, 0.1f)) spawnObject(assetPath + "Fly_Agaric_Group_1.glb", ox, oz, 0.002f, 360.0f); }
            }
        }
        else if (type == "Oak") {
             // Eichen (Lockerer)
             int treeCount = 18 + rand() % 5;
             for(int t=0; t<treeCount; t++) {
                float ox = centerX + clusterDist(gen); float oz = centerZ + clusterDist(gen);
                if(checkDistance(ox, oz, 0.6f)) spawnObject(assetPath + "Oak_Tree_1.glb", ox, oz, 0.0028f, 360.0f);
             }
             // Brennnesseln
             int nettleCount = 90 + rand() % 30;
             for(int n=0; n<nettleCount; n++) {
                 float ox = centerX + clusterDist(gen) * 1.3f; float oz = centerZ + clusterDist(gen) * 1.3f;
                 if(rand()%2==0) { if(checkDistance(ox, oz, 0.15f)) spawnObject(assetPath + "Stinging_Nettle_1.glb", ox, oz, 0.002f, 360.0f); }
                 else { if(checkDistance(ox, oz, 0.15f)) spawnObject(assetPath + "Forest_Grass_1.glb", ox, oz, 0.002f, 360.0f); }
             }
        }
        else if (type == "Scrub") {
            // LÜCKENFÜLLER: Wird überall verteilt, wo noch Platz ist
            int fillerCount = 120 + rand() % 50;
            for(int f=0; f<fillerCount; f++) {
                // Wir streuen Scrub sehr weit (x2.5 Radius), damit es die Lücken zwischen Clustern schließt
                float ox = centerX + clusterDist(gen) * 2.5f;
                float oz = centerZ + clusterDist(gen) * 2.5f;

                int roll = rand() % 4;
                if(roll == 0) { if(checkDistance(ox,oz,0.2f)) spawnObject(assetPath + "Fern_1a.glb", ox, oz, 0.002f, 360.0f); }
                else if (roll == 1) { if(checkDistance(ox,oz,0.2f)) spawnObject(assetPath + "Blackberry_Bush_1a.glb", ox, oz, 0.0015f, 360.0f); }
                else if (roll == 2) { if(checkDistance(ox,oz,0.5f)) spawnObject(assetPath + "Rock_2.glb", ox, oz, 0.004f, 360.0f); }
                else { if(checkDistance(ox,oz,0.2f)) spawnObject(assetPath + "Forest_Grass_1.glb", ox, oz, 0.002f, 360.0f); }
            }
        }
    }
    std::cout << "Biome Cluster '" << type << "' erstellt: " << groupsCreated << " Gruppen." << std::endl;
}

// --- 3. INSTANCED RENDERING SETUP ---

void ForestSystem::updateInstances() {
    for (auto& entry : forestTypes) {
        ForestType& fType = entry.second;

        // Skip wenn schon aktuell
        if (fType.isSetup) continue;
        if (fType.instances.empty()) continue;

        // 1. Matrix Cache vorbereiten
        fType.matrixCache.clear();
        fType.matrixCache.reserve(fType.instances.size());
        for (const auto& inst : fType.instances) {
            fType.matrixCache.push_back(inst.transform);
        }

        // 2. VBO erstellen falls nötig
        if (fType.instanceVBO == 0) {
            glGenBuffers(1, &fType.instanceVBO);
        }

        // 3. Daten hochladen
        glBindBuffer(GL_ARRAY_BUFFER, fType.instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, fType.matrixCache.size() * sizeof(glm::mat4), &fType.matrixCache[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 4. VAO Konfiguration für Instancing Attribute (Loc 4-7)
        // Wir müssen durch ALLE SubMeshes des Modells iterieren
        for (unsigned int i = 0; i < fType.model->meshes.size(); i++) {
            unsigned int VAO = fType.model->meshes[i].VAO;

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, fType.instanceVBO);

            std::size_t vec4Size = sizeof(glm::vec4);
            // Mat4 belegt 4 Locations (4,5,6,7)
            for (int k = 0; k < 4; k++) {
                glEnableVertexAttribArray(4 + k);
                glVertexAttribPointer(4 + k, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(k * vec4Size));
                // Instancing Divisor: 1 = Update pro Instanz
                glVertexAttribDivisor(4 + k, 1);
            }

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        fType.isSetup = true;
    }
}

void ForestSystem::draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos) {
    // GPU Puffer updaten
    updateInstances();

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("viewPos", viewPos);

    // Keine Maps für generierte Assets (verhindert schwarze Bäume)
    shader.setBool("useNormalMap", false);
    shader.setBool("useARMMap", false);

    // INSTANCING AKTIVIEREN
    shader.setBool("useInstancing", true);

    for (auto& entry : forestTypes) {
        ForestType& fType = entry.second;
        if (fType.matrixCache.empty()) continue;

        Model* model = fType.model;

        // Durch alle Meshes des Models loopen und Instanced zeichnen
        for (unsigned int i = 0; i < model->meshes.size(); i++) {
            // Texturen binden (Kopie der Model::Draw Logik)
            unsigned int diffuseNr = 1;
            unsigned int normalNr = 1;
            unsigned int armNr = 1;

            for(unsigned int j = 0; j < model->meshes[i].textures.size(); j++) {
                glActiveTexture(GL_TEXTURE0 + j);
                std::string name = model->meshes[i].textures[j].type;
                // Da wir keine Map Uniforms setzen, reicht das Binden oft schon für den Fallback
                glBindTexture(GL_TEXTURE_2D, model->meshes[i].textures[j].id);
            }

            // Zeichnen
            glBindVertexArray(model->meshes[i].VAO);

            // MAGIC: Zeichne X Instanzen auf einmal
            glDrawElementsInstanced(GL_TRIANGLES,
                                  static_cast<unsigned int>(model->meshes[i].indices.size()),
                                  GL_UNSIGNED_INT, 0,
                                  static_cast<unsigned int>(fType.matrixCache.size()));

            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0);
        }
    }

    // Instancing für den Rest der Pipeline ausschalten
    shader.setBool("useInstancing", false);
}