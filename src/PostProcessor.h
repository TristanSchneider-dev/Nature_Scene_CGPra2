#pragma once
#include <glad/glad.h>
#include <iostream>
#include "Shader.h"

class PostProcessor {
public:
    unsigned int FBO;
    unsigned int textureColor;
    unsigned int textureDepth;

    // NEU: Wir merken uns die aktuelle Größe
    int width, height;

    PostProcessor(int w, int h) : width(w), height(h) {
        shader = new Shader("../../../shaders/post_fog.vs.glsl", "../../../shaders/post_fog.fs.glsl");
        initQuad();
        initFramebuffer(w, h);
    }

    ~PostProcessor() {
        delete shader;
        glDeleteFramebuffers(1, &FBO);
        glDeleteTextures(1, &textureColor);
        glDeleteTextures(1, &textureDepth);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }

    // NEU: Diese Funktion ruft initFramebuffer nur auf, wenn sich wirklich was ändert
    void checkResize(int newW, int newH) {
        if (newW != width || newH != height) {
            this->width = newW;
            this->height = newH;

            // Alte Texturen löschen
            glDeleteFramebuffers(1, &FBO);
            glDeleteTextures(1, &textureColor);
            glDeleteTextures(1, &textureDepth);

            // Neu erstellen
            initFramebuffer(newW, newH);
        }
    }

    void beginRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glEnable(GL_DEPTH_TEST);
    }

    void endRender(float zNear, float zFar, glm::vec3 fogCol, float fogDensity) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->use();
        shader->setInt("screenTexture", 0);
        shader->setInt("depthTexture", 1);
        shader->setFloat("zNear", zNear);
        shader->setFloat("zFar", zFar);
        shader->setVec3("fogColor", fogCol);
        shader->setFloat("density", fogDensity);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureDepth);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

private:
    Shader* shader;
    unsigned int quadVAO, quadVBO;

    void initFramebuffer(int w, int h) {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // Color
        glGenTextures(1, &textureColor);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor, 0);

        // Depth
        glGenTextures(1, &textureDepth);
        glBindTexture(GL_TEXTURE_2D, textureDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void initQuad() {
        // ... (Der Quad Code bleibt gleich wie vorher)
        float quadVertices[] = { -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
};