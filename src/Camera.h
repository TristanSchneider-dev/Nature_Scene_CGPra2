#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum CameraMode { ORBIT, FREE };

    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f));

    void processInput(GLFWwindow* window, float deltaTime);
    void processMouse(float xPos, float yPos);
    void processScroll(float yOffset);
    void updateOrbit(float deltaTime);
    void resetToDefault();
    void resetFirstMouse();
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    void setViewportSize(float width, float height);

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getWorldUp() const { return up; }
    float getFov() const { return fov; }

    CameraMode mode;
    CameraMode lastMode;

private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    float yaw;
    float pitch;
    float fov;
    float lastX;
    float lastY;
    bool firstMouse;
    float cameraSpeed;

    glm::vec3 orbitCenter;
    float orbitRadius;
    float orbitYaw;
    float orbitPitch;
    float orbitSpeed;

    float viewportWidth;
    float viewportHeight;
};