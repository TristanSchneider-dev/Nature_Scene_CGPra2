#include "BirdSystem.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <cmath>

BirdSystem::BirdSystem()
    : globalSpeed(5.0f),
    minHeight(20.0f), maxHeight(60.0f),
    flightCenter(0.0f, 40.0f, 0.0f), flightRadius(80.0f),
    spriteFrames(4), billboardVAO(0), billboardVBO(0), birdTexture(0)
{
    billboardShader = nullptr;
}

BirdSystem::~BirdSystem() {
    if (billboardShader) delete billboardShader;
   
    glDeleteVertexArrays(1, &billboardVAO);
    glDeleteBuffers(1, &billboardVBO);
    glDeleteTextures(1, &birdTexture);
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

    // Setup Billboards
    setupBillboards();

    std::cout << "[BirdSystem] Initialized with " << birdCount << " birds" << std::endl;
}

void BirdSystem::setBirdCount(int count) {
    if (count < 0) count = 0;
    if (count > 200) count = 200; // Maximum für Performance

    int currentCount = birds.size();

    if (count > currentCount) {
        // Vögel hinzufügen
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f);
        std::uniform_real_distribution<float> speedDist(0.8f, 1.2f);
        std::uniform_real_distribution<float> scaleDist(0.8f, 1.5f);

        for (int i = currentCount; i < count; i++) {
            birds.emplace_back(
                getRandomStartPosition(),
                getRandomVelocity(),
                phaseDist(gen),
                speedDist(gen),
                scaleDist(gen)
            );
        }
        std::cout << "[BirdSystem] Added " << (count - currentCount) << " birds (total: " << count << ")" << std::endl;
    }
    else if (count < currentCount) {
        // Vögel entfernen
        birds.resize(count);
        std::cout << "[BirdSystem] Removed " << (currentCount - count) << " birds (total: " << count << ")" << std::endl;
    }
}
// ============================================================================
// BEWEGUNGS-UPDATE
// ============================================================================

void BirdSystem::update(float deltaTime, float currentTime) {
    for (auto& bird : birds) {
        updateBirdMovement(bird, deltaTime, currentTime);
    }
}

void BirdSystem::updateBirdMovement(Bird& bird, float deltaTime, float currentTime) {
    // 1. Basis-Bewegung
    bird.position += bird.velocity * globalSpeed * deltaTime;

    // 2. Sinus-Wellenbewegung (auf und ab)
    bird.position.y += std::sin(currentTime * 2.0f + bird.phase) * 0.05f;

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
    float randomTurn = std::sin(currentTime * 3.0f + bird.phase) * 0.02f;
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


//  BILLBOARD BIRD RENDERING

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

void BirdSystem::draw(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time) {
    // Alpha Blending aktivieren
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Depth Test an, aber kein Depth Write
    glDepthMask(GL_FALSE);

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

    // Zurücksetzen
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}