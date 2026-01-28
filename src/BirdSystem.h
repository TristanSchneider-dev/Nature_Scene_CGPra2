#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include "Shader.h"

// Einzelner Vogel
struct Bird {
    glm::vec3 position;
    glm::vec3 velocity;
    float phase;        // Für unterschiedliche Flügelschlag-Phasen
    float wingSpeed;    // Individuelle Flügelschlag-Geschwindigkeit
    float scale;        // Größe des Vogels

    Bird(glm::vec3 pos, glm::vec3 vel, float ph, float ws, float sc)
        : position(pos), velocity(vel), phase(ph), wingSpeed(ws), scale(sc) {
    }
};

class BirdSystem {
public:
    enum RenderMode {
        BILLBOARD,  // Option 1: 2D Sprites mit Kamera-Ausrichtung
        MESH_3D     // Option 2: Simple 3D Meshes mit Shader-Animation
    };

    BirdSystem();
    ~BirdSystem();

    // Initialisierung
    void init(int birdCount = 50);

    // Update (bewegt Vögel)
    void update(float deltaTime);

    // Render
    void draw(const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time);

    // Wechsle zwischen Billboard und 3D
    void setRenderMode(RenderMode mode) { renderMode = mode; }
    RenderMode getRenderMode() const { return renderMode; }

    // Einstellungen
    void setBirdSpeed(float speed) { globalSpeed = speed; }
    void setFlightHeight(float min, float max) { minHeight = min; maxHeight = max; }

private:
    // Rendering-Modus
    RenderMode renderMode;

    // Vögel-Daten
    std::vector<Bird> birds;

    // Bewegungs-Parameter
    float globalSpeed;
    float minHeight, maxHeight;
    glm::vec3 flightCenter;  // Zentrum um das die Vögel fliegen
    float flightRadius;      // Radius des Flugbereichs

    // === BILLBOARD MODE (Option 1) ===
    Shader* billboardShader;
    unsigned int billboardVAO, billboardVBO;
    unsigned int birdTexture;
    int spriteFrames;

    void setupBillboards();
    void drawBillboards(const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time);
    unsigned int loadBirdTexture(const char* path);

    // === 3D MESH MODE (Option 2) ===
    Shader* meshShader;
    unsigned int meshVAO, meshVBO, meshEBO;
    unsigned int meshVertexCount;

    void setupMesh();
    void drawMeshes(const glm::mat4& view, const glm::mat4& projection, float time);
    void createSimpleBirdMesh();

    // Hilfsfunktionen
    void updateBirdMovement(Bird& bird, float deltaTime);
    glm::vec3 getRandomStartPosition();
    glm::vec3 getRandomVelocity();
};