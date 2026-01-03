#include "Terrain.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <iostream>

Terrain::Terrain(const std::string& modelPath) {
    loadModel(modelPath);
    loadMaterials();
}

Terrain::~Terrain() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::loadMaterials() {
    std::string root = "../assets/terrain/";

    std::string p1 = root + "ganges_river_pebbles_2k.gltf/textures/";
    matPebbles.albedo = loadTexture((p1 + "ganges_river_pebbles_diff_2k.jpg").c_str());
    matPebbles.normal = loadTexture((p1 + "ganges_river_pebbles_nor_gl_2k.jpg").c_str());
    matPebbles.arm    = loadTexture((p1 + "ganges_river_pebbles_arm_2k.jpg").c_str());

    std::string p2 = root + "rocky_terrain_02_2k.gltf/textures/";
    matGround.albedo = loadTexture((p2 + "rocky_terrain_02_diff_2k.jpg").c_str());
    matGround.normal = loadTexture((p2 + "rocky_terrain_02_nor_gl_2k.jpg").c_str());
    matGround.arm    = loadTexture((p2 + "rocky_terrain_02_arm_2k.jpg").c_str());

    std::string p3 = root + "rocky_terrain_2k.gltf/textures/";
    matRock.albedo = loadTexture((p3 + "rocky_terrain_diff_2k.jpg").c_str());
    matRock.normal = loadTexture((p3 + "rocky_terrain_nor_gl_2k.jpg").c_str());
    matRock.arm    = loadTexture((p3 + "rocky_terrain_arm_2k.jpg").c_str());
}

void Terrain::draw(Shader& shader) {
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, matPebbles.albedo);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, matPebbles.normal);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, matPebbles.arm);

    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, matGround.albedo);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, matGround.normal);
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, matGround.arm);

    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, matRock.albedo);
    glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, matRock.normal);
    glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, matRock.arm);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

unsigned int Terrain::loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED : (nrComponents == 3 ? GL_RGB : GL_RGBA);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

void Terrain::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if(!scene || !scene->mRootNode) { std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl; return; }

    aiMesh* mesh = scene->mMeshes[0];
    std::vector<float> data;
    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Pos
        data.push_back(mesh->mVertices[i].x);
        data.push_back(mesh->mVertices[i].y);
        data.push_back(mesh->mVertices[i].z);
        // Normal
        if (mesh->HasNormals()) {
            data.push_back(mesh->mNormals[i].x);
            data.push_back(mesh->mNormals[i].y);
            data.push_back(mesh->mNormals[i].z);
        } else data.insert(data.end(), {0,1,0});
        // UV
        if(mesh->HasTextureCoords(0)) {
            data.push_back(mesh->mTextureCoords[0][i].x);
            data.push_back(mesh->mTextureCoords[0][i].y);
        } else data.insert(data.end(), {0,0});
        // Tangent
        if(mesh->HasTangentsAndBitangents()) {
            data.push_back(mesh->mTangents[i].x);
            data.push_back(mesh->mTangents[i].y);
            data.push_back(mesh->mTangents[i].z);
        } else data.insert(data.end(), {0,0,0});
    }
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
    }
    indexCount = indices.size();

    // WICHTIG: Daten persistent speichern für GrassSystem
    m_vertices = data;
    m_indices = indices;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    int stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float))); glEnableVertexAttribArray(3);
}