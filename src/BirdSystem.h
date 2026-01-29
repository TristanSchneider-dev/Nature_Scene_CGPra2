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

    // Default-Konstruktor
    Bird()
        : position(0.0f), velocity(0.0f), phase(0.0f), wingSpeed(1.0f), scale(1.0f) {
    }

    Bird(glm::vec3 pos, glm::vec3 vel, float ph, float ws, float sc)
        : position(pos), velocity(vel), phase(ph), wingSpeed(ws), scale(sc) {
    }
};

class BirdSystem {
public:
    BirdSystem();
    ~BirdSystem();

    // Initialisierung
    void init(int birdCount = 50);

    // Update (bewegt Vögel)
    void update(float deltaTime, float currentTime);

    // Render
    void draw(const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time);

    // Einstellungen
    void setBirdSpeed(float speed) { globalSpeed = speed; }
    void setFlightHeight(float min, float max) { minHeight = min; maxHeight = max; }
    void setBirdCount(int count); // Dynamisch Vögel hinzufügen/entfernen
    int getBirdCount() const { return birds.size(); } 

private:
    // Vögel-Daten
    std::vector<Bird> birds;

    // Bewegungs-Parameter
    float globalSpeed;
    float minHeight, maxHeight;
    glm::vec3 flightCenter;  // Zentrum um das die Vögel fliegen
    float flightRadius;      // Radius des Flugbereichs

    // Billboard Rendering
    Shader* billboardShader;
    unsigned int billboardVAO, billboardVBO;
    unsigned int birdTexture;
    int spriteFrames;

    void setupBillboards();
    unsigned int loadBirdTexture(const char* path);

    // Hilfsfunktionen
    void updateBirdMovement(Bird& bird, float deltaTime, float currentTime);
    glm::vec3 getRandomStartPosition();
    glm::vec3 getRandomVelocity();
};