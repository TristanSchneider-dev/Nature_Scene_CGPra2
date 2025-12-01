#ifndef WATERPLANE_H
#define WATERPLANE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class WaterPlane {
public:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    WaterPlane(float size, int resolution) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float step = size / (float)resolution;
        float offset = size / 2.0f;

        for (int z = 0; z <= resolution; ++z) {
            for (int x = 0; x <= resolution; ++x) {
                vertices.push_back((x * step) - offset);
                vertices.push_back(0.0f);
                vertices.push_back((z * step) - offset);

                vertices.push_back((float)x / resolution);
                vertices.push_back((float)z / resolution);
            }
        }

        for (int z = 0; z < resolution; ++z) {
            for (int x = 0; x < resolution; ++x) {
                unsigned int topLeft     = z * (resolution + 1) + x;
                unsigned int topRight    = topLeft + 1;
                unsigned int bottomLeft  = (z + 1) * (resolution + 1) + x;
                unsigned int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        indexCount = static_cast<int>(indices.size());

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
    }

    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& camPos) {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // HIER WAR DER FEHLER: Die Zeile 'setMat4("model"...)' wurde entfernt.
        // Jetzt nutzt er das Model, das du in der main.cpp gesetzt hast!

        shader.setVec3("viewPos", camPos);

        // Time wird jetzt auch in main.cpp gesetzt, aber sicherheitshalber hier auch noch,
        // falls man es mal vergisst.
        shader.setFloat("time", (float)glfwGetTime()); 

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~WaterPlane() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

#endif