#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Camera::Camera()
    : mode(FREE), lastMode(FREE), // Default ist jetzt FREE
      position(0.0f, 0.0f, 5.0f), front(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f),
      yaw(-90.0f), pitch(0.0f), fov(70.0f),
      lastX(600.0f), lastY(600.0f), firstMouse(true), cameraSpeed(5.0f),
      orbitCenter(0.0f, 0.0f, 0.0f), orbitRadius(5.0f), orbitYaw(0.0f), orbitPitch(5.0f), orbitSpeed(10.0f),
      viewportWidth(1200.0f), viewportHeight(1200.0f) {}

void Camera::resetFirstMouse() {
    firstMouse = true;
}

void Camera::resetToDefault() {
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    firstMouse = true;
}

void Camera::processInput(GLFWwindow* window, float deltaTime) {
    // TASTE 1: Jetzt FREE Mode
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && mode != FREE) {
        mode = FREE;
        bool isNormalView = (up.x == 0.0f && up.y == 1.0f && up.z == 0.0f);
        if (isNormalView) {
            yaw = glm::degrees(atan2(front.z, front.x));
            pitch = glm::degrees(asin(front.y));
        } else {
            resetToDefault();
        }
        firstMouse = true;
    }

    // TASTE 2: ORBIT (Unverändert)
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && mode != ORBIT) {
        mode = ORBIT;
        orbitRadius = 5.0f;
        orbitYaw = 0.0f;
        orbitPitch = 5.0f;
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // TASTE 3: Jetzt FIXED Mode
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && mode != FIXED) {
        mode = FIXED;
        resetToDefault();
    }

    lastMode = mode;

    if (mode == FREE) {
        float velocity = cameraSpeed * deltaTime;
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 right = glm::normalize(glm::cross(horizontalFront, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += horizontalFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= horizontalFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= right * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += right * velocity;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position.y += velocity;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            position.y -= velocity;
    }
    else if (mode == FIXED) {
        float distance = 5.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position = glm::vec3(0.0f, distance, 0.0f); front = glm::vec3(0.0f, -1.0f, 0.0f); up = glm::vec3(0.0f, 0.0f, -1.0f); yaw = -90.0f; pitch = -90.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position = glm::vec3(0.0f, -distance, 0.0f); front = glm::vec3(0.0f, 1.0f, 0.0f); up = glm::vec3(0.0f, 0.0f, 1.0f); yaw = -90.0f; pitch = 90.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position = glm::vec3(-distance, 0.0f, 0.0f); front = glm::vec3(1.0f, 0.0f, 0.0f); up = glm::vec3(0.0f, 1.0f, 0.0f); yaw = 0.0f; pitch = 0.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position = glm::vec3(distance, 0.0f, 0.0f); front = glm::vec3(-1.0f, 0.0f, 0.0f); up = glm::vec3(0.0f, 1.0f, 0.0f); yaw = 180.0f; pitch = 0.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            position = glm::vec3(0.0f, 0.0f, distance); front = glm::vec3(0.0f, 0.0f, -1.0f); up = glm::vec3(0.0f, 1.0f, 0.0f); yaw = -90.0f; pitch = 0.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            position = glm::vec3(0.0f, 0.0f, -distance); front = glm::vec3(0.0f, 0.0f, 1.0f); up = glm::vec3(0.0f, 1.0f, 0.0f); yaw = 90.0f; pitch = 0.0f;
        }
    }
    else if (mode == ORBIT) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            orbitSpeed = -abs(orbitSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            orbitSpeed = abs(orbitSpeed);
        }
    }
}

void Camera::processMouse(float xPos, float yPos) {
    if (mode != FREE) return;

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::processScroll(float yOffset) {
    if (mode == ORBIT) {
        orbitRadius -= yOffset;
        if (orbitRadius < 1.0f) orbitRadius = 1.0f;
    }
}

void Camera::updateOrbit(float deltaTime) {
    orbitYaw += orbitSpeed * deltaTime;

    float radYaw = glm::radians(orbitYaw);
    float radPitch = glm::radians(orbitPitch);

    position.x = orbitCenter.x + orbitRadius * cos(radPitch) * cos(radYaw);
    position.y = orbitCenter.y + orbitRadius * sin(radPitch);
    position.z = orbitCenter.z + orbitRadius * cos(radPitch) * sin(radYaw);

    front = glm::normalize(orbitCenter - position);
}

glm::mat4 Camera::getViewMatrix() {
    if (mode == FIXED) {
        return glm::lookAt(position, orbitCenter, up);
    } else {
        return glm::lookAt(position, position + front, up);
    }
}

glm::mat4 Camera::getProjectionMatrix() {
    return glm::perspective(glm::radians(fov), viewportWidth / viewportHeight, 0.1f, 100.0f);
}

void Camera::setViewportSize(float width, float height) {
    viewportWidth = width;
    viewportHeight = height;
    lastX = width / 2.0f;
    lastY = height / 2.0f;
}