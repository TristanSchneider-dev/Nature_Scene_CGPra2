#include "BirdSystem.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <cmath>

BirdSystem::BirdSystem()
    : renderMode(BILLBOARD), globalSpeed(5.0f),
    minHeight(20.0f), maxHeight(60.0f),
    flightCenter(0.0f, 40.0f, 0.0f), flightRadius(80.0f),
    spriteFrames(4), billboardVAO(0), billboardVBO(0), birdTexture(0),
    meshVAO(0), meshVBO(0), meshEBO(0), meshVertexCount(0)
{
    billboardShader = nullptr;
    meshShader = nullptr;
}

BirdSystem::~BirdSystem() {
    if (billboardShader) delete billboardShader;
    if (meshShader) delete meshShader;

    glDeleteVertexArrays(1, &billboardVAO);
    glDeleteBuffers(1, &billboardVBO);
    glDeleteTextures(1, &birdTexture);

    glDeleteVertexArrays(1, &meshVAO);
    glDeleteBuffers(1, &meshVBO);
    glDeleteBuffers(1, &meshEBO);
}

void BirdSystem::init(int birdCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f);
    std::uniform_real_distribution<float> speedDist(0.8f, 1.2f);
    std::uniform_real_distribution<float> scaleDist(0.8f, 1.5f);

    // Erstelle Vögel
    birds.clear();
    for (int i = 0; i < birdCount; i++) {
        birds.emplace_back(
            getRandomStartPosition(),
            getRandomVelocity(),
            phaseDist(gen),
            speedDist(gen),
            scaleDist(gen)
        );
    }

    // Setup beide Modi
    setupBillboards();
    setupMesh();

    std::cout << "[BirdSystem] Initialized with " << birdCount << " birds" << std::endl;
}

// ============================================================================
// BEWEGUNGS-UPDATE
// ============================================================================

void BirdSystem::update(float deltaTime) {
    for (auto& bird : birds) {
        updateBirdMovement(bird, deltaTime);
    }
}

void BirdSystem::updateBirdMovement(Bird& bird, float deltaTime) {
    // 1. Basis-Bewegung
    bird.position += bird.velocity * globalSpeed * deltaTime;

    // 2. Sinus-Wellenbewegung (auf und ab)
    bird.position.y += std::sin(glfwGetTime() * 2.0f + bird.phase) * 0.05f;

    // 3. Kreisende Bewegung um Zentrum
    glm::vec3 toCenter = flightCenter - bird.position;
    float dist = glm::length(glm::vec2(toCenter.x, toCenter.z));

    // Wenn zu weit weg -> zurück zum Zentrum
    if (dist > flightRadius) {
        bird.velocity = glm::normalize(toCenter) * glm::length(bird.velocity);
    }

    // 4. Höhenbegrenzung
    if (bird.position.y < minHeight) {
        bird.velocity.y = std::abs(bird.velocity.y);
    }
    if (bird.position.y > maxHeight) {
        bird.velocity.y = -std::abs(bird.velocity.y);
    }

    // 5. Leichte Richtungsänderung (Flatter-Effekt)
    float randomTurn = std::sin(glfwGetTime() * 3.0f + bird.phase) * 0.02f;
    float angle = randomTurn;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);

    glm::vec3 newVel = bird.velocity;
    newVel.x = bird.velocity.x * cosA - bird.velocity.z * sinA;
    newVel.z = bird.velocity.x * sinA + bird.velocity.z * cosA;
    bird.velocity = newVel;
}

glm::vec3 BirdSystem::getRandomStartPosition() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(-flightRadius, flightRadius);
    std::uniform_real_distribution<float> yDist(minHeight, maxHeight);
    std::uniform_real_distribution<float> zDist(-flightRadius, flightRadius);

    return flightCenter + glm::vec3(xDist(gen), yDist(gen), zDist(gen));
}

glm::vec3 BirdSystem::getRandomVelocity() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    glm::vec3 vel(dist(gen), dist(gen) * 0.3f, dist(gen));
    return glm::normalize(vel) * 2.0f;
}

// ============================================================================
// OPTION 1: BILLBOARD RENDERING
// ============================================================================

void BirdSystem::setupBillboards() {
    // Quad (wird zur Kamera ausgerichtet)
    float quadVertices[] = {
        // Pos          // UV
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &billboardVAO);
    glGenBuffers(1, &billboardVBO);

    glBindVertexArray(billboardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Shader erstellen
    billboardShader = new Shader(
        "../../../shaders/bird_billboard.vs.glsl",
        "../../../shaders/bird_billboard.fs.glsl"
    );

    // Textur laden (erstelle dummy wenn nicht vorhanden)
    birdTexture = loadBirdTexture("../../../assets/birds/bird_sprite.png");
}

void BirdSystem::drawBillboards(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time) {
    billboardShader->use();
    billboardShader->setMat4("projection", projection);
    billboardShader->setMat4("view", view);
    billboardShader->setVec3("cameraPos", cameraPos);
    billboardShader->setInt("birdTexture", 0);
    billboardShader->setInt("frameCount", spriteFrames);
    billboardShader->setFloat("time", time);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, birdTexture);

    glBindVertexArray(billboardVAO);

    for (const auto& bird : birds) {
        billboardShader->setVec3("birdPos", bird.position);
        billboardShader->setFloat("birdScale", bird.scale);
        billboardShader->setFloat("wingPhase", bird.phase);
        billboardShader->setFloat("wingSpeed", bird.wingSpeed);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
}

unsigned int BirdSystem::loadBirdTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "[BirdSystem] Loaded bird texture: " << path << std::endl;
    }
    else {
        std::cout << "[BirdSystem] Failed to load texture, creating fallback" << std::endl;
        // Erstelle einfache weiße Textur als Fallback
        unsigned char white[] = { 255, 255, 255, 255 };
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    }

    stbi_image_free(data);
    return textureID;
}

// ============================================================================
// OPTION 2: 3D MESH RENDERING
// ============================================================================

void BirdSystem::setupMesh() {
    createSimpleBirdMesh();

    meshShader = new Shader(
        "../../../shaders/bird_mesh.vs.glsl",
        "../../../shaders/bird_mesh.fs.glsl"
    );
}

void BirdSystem::createSimpleBirdMesh() {
    // Einfacher Vogel: Körper + 2 Flügel
    // Format: X, Y, Z, NX, NY, NZ, IsWing
    std::vector<float> vertices = {
        // Körper (Dreieck nach vorne)
         0.0f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  // Kopf
        -0.2f,  0.0f, -0.3f,  0.0f, 1.0f, 0.0f, 0.0f,  // Links
         0.2f,  0.0f, -0.3f,  0.0f, 1.0f, 0.0f, 0.0f,  // Rechts
         0.0f, -0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  // Unten

         // Linker Flügel
         -0.2f,  0.0f, -0.3f,  0.0f, 1.0f, 0.0f, 1.0f,  // Basis
         -0.8f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,  // Spitze
         -0.6f,  0.0f, -0.4f,  0.0f, 1.0f, 0.0f, 1.0f,  // Hinten

         // Rechter Flügel
          0.2f,  0.0f, -0.3f,  0.0f, 1.0f, 0.0f, 1.0f,  // Basis
          0.8f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,  // Spitze
          0.6f,  0.0f, -0.4f,  0.0f, 1.0f, 0.0f, 1.0f   // Hinten
    };

    std::vector<unsigned int> indices = {
        // Körper
        0, 1, 2,  // Top
        1, 3, 2,  // Bottom

        // Linker Flügel
        4, 5, 6,

        // Rechter Flügel
        7, 8, 9
    };

    meshVertexCount = indices.size();

    glGenVertexArrays(1, &meshVAO);
    glGenBuffers(1, &meshVBO);
    glGenBuffers(1, &meshEBO);

    glBindVertexArray(meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

    // IsWing flag
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void BirdSystem::drawMeshes(const glm::mat4& view, const glm::mat4& projection, float time) {
    meshShader->use();
    meshShader->setMat4("projection", projection);
    meshShader->setMat4("view", view);
    meshShader->setFloat("time", time);
    meshShader->setVec3("birdColor", glm::vec3(0.3f, 0.3f, 0.35f)); // Dunkles Grau

    glBindVertexArray(meshVAO);

    for (const auto& bird : birds) {
        // Berechne Rotation basierend auf Flugrichtung
        glm::vec3 forward = glm::normalize(bird.velocity);
        float yaw = std::atan2(forward.x, forward.z);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, bird.position);
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(bird.scale));

        meshShader->setMat4("model", model);
        meshShader->setFloat("wingPhase", bird.phase);
        meshShader->setFloat("wingSpeed", bird.wingSpeed);

        glDrawElements(GL_TRIANGLES, meshVertexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

// ============================================================================
// MAIN DRAW
// ============================================================================

void BirdSystem::draw(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time) {
    // Alpha Blending aktivieren für transparente Vögel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Depth Test an, aber kein Depth Write (damit Vögel sich nicht gegenseitig verdecken)
    glDepthMask(GL_FALSE);

    if (renderMode == BILLBOARD) {
        drawBillboards(view, projection, cameraPos, time);
    }
    else {
        drawMeshes(view, projection, time);
    }

    // Zurücksetzen
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}