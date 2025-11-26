#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum CameraMode { FIXED, ORBIT, FREE };

    Camera();

    void processInput(GLFWwindow* window, float deltaTime);
    void processMouse(float xPos, float yPos);
    void processScroll(float yOffset);
    void updateOrbit(float deltaTime);
    void resetToDefault();
    void resetFirstMouse(); // Hinzugefügte Methode
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    void setViewportSize(float width, float height);

    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getWorldUp() const { return up; }
    float getFov() const { return fov; }

    // Public members for state-machine logic in main
    CameraMode mode;
    CameraMode lastMode;

private:
    // Position und Orientierung
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    // Freie Kamera Parameter
    float yaw;
    float pitch;
    float fov;
    float lastX;
    float lastY;
    bool firstMouse;
    float cameraSpeed;

    // Orbit Kamera Parameter
    glm::vec3 orbitCenter;
    float orbitRadius;
    float orbitYaw;
    float orbitPitch;
    float orbitSpeed;

    float viewportWidth;
    float viewportHeight;
};