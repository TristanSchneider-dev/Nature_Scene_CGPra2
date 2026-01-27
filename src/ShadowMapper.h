#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Shader.h"

class ShadowMapper {
public:
    ShadowMapper(unsigned int resolution = 4096)
        : shadowMapResolution(resolution) {

        // Create depth map framebuffer
        glGenFramebuffers(1, &depthMapFBO);

        // Create depth cubemap texture for shadows
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Border color = white (no shadow outside frustum)
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Attach depth texture to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create shadow shader
        shadowShader = new Shader(
            "../../../shaders/shadow.vs.glsl",
            "../../../shaders/shadow.fs.glsl"
        );
    }

    ~ShadowMapper() {
        glDeleteFramebuffers(1, &depthMapFBO);
        glDeleteTextures(1, &depthMap);
        delete shadowShader;
    }

    // Calculate light space matrix for directional light
    glm::mat4 getLightSpaceMatrix(const glm::vec3& lightDir, const glm::vec3& sceneCenter,
        float sceneRadius = 100.0f) {
        // Position light far away in direction of light
        glm::vec3 lightPos = sceneCenter - glm::normalize(lightDir) * sceneRadius * 2.0f;

        // Look at scene center
        glm::mat4 lightView = glm::lookAt(
            lightPos,
            sceneCenter,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Orthographic projection for directional light
        float orthoSize = sceneRadius * 1.5f;
        glm::mat4 lightProjection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            0.1f, sceneRadius * 4.0f
        );

        return lightProjection * lightView;
    }

    // Begin shadow pass
    void beginShadowPass() {
        glViewport(0, 0, shadowMapResolution, shadowMapResolution);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Enable depth testing, disable color writes
        glEnable(GL_DEPTH_TEST);

        // Reduce shadow acne with polygon offset
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        // Optional: cull front faces to reduce peter-panning
        // glCullFace(GL_FRONT);
    }

    // End shadow pass
    void endShadowPass(int screenWidth, int screenHeight) {
        glDisable(GL_POLYGON_OFFSET_FILL);
        // glCullFace(GL_BACK); // if you enabled front-face culling

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
    }

    // Bind shadow map for sampling in main render pass
    void bindShadowMap(unsigned int textureUnit = 10) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    // Get shadow shader for depth rendering
    Shader* getShadowShader() { return shadowShader; }

    unsigned int getDepthMap() const { return depthMap; }

private:
    unsigned int depthMapFBO;
    unsigned int depthMap;
    unsigned int shadowMapResolution;
    Shader* shadowShader;
};