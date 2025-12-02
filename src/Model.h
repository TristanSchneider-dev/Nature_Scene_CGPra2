#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <map>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

struct Texture {
    unsigned int id;
    std::string type; // "texture_diffuse", "texture_normal", "texture_arm"
    std::string path; // Speicherpfad zur Vermeidung von Doppelladungen
};

struct SubMesh {
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    SubMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);
};

class Model {
public:
    std::vector<Texture> textures_loaded; // Cache
    std::vector<SubMesh> meshes;
    std::string directory;

    Model(std::string const& path);
    void Draw(Shader& shader, glm::mat4 modelMatrix);

private:
    void loadModel(std::string const& path);
    void processNode(aiNode* node, const aiScene* scene);
    SubMesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);
    unsigned int TextureFromFile(const char* path, const std::string& directory);
    unsigned int TextureFromMemory(const aiTexture* aiTex); // NEU: Für embedded Texturen
};