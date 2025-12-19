#include "Terrain.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <iostream>
#include <cstdlib>

Terrain::Terrain(const std::string& modelPath) {
    loadModel(modelPath);
    loadTextures();
}

Terrain::~Terrain() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::draw(Shader& shader) {
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tPebbleD);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, tPebbleN);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, tPebbleARM);

    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, tGravelD);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, tGravelN);
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, tGravelARM);

    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, tRockD);
    glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, tRockN);
    glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, tRockARM);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void Terrain::loadTextures() {
    std::string pGravel = "../assets/textures/gravel_ground_01_2k.gltf/textures/";
    std::string pPebbles = "../assets/textures/ganges_river_pebbles_2k.gltf/textures/";
    std::string pRock = "../assets/textures/rocky_terrain_02_2k.gltf/textures/";

    tPebbleD   = loadTexture((pPebbles + "ganges_river_pebbles_diff_2k.jpg").c_str());
    tPebbleN   = loadTexture((pPebbles + "ganges_river_pebbles_nor_gl_2k.jpg").c_str());
    tPebbleARM = loadTexture((pPebbles + "ganges_river_pebbles_arm_2k.jpg").c_str());

    tGravelD   = loadTexture((pGravel + "gravel_ground_01_diff_2k.jpg").c_str());
    tGravelN   = loadTexture((pGravel + "gravel_ground_01_nor_gl_2k.jpg").c_str());
    tGravelARM = createDefaultARM(1.0f, 1.0f, 0.0f);

    tRockD   = loadTexture((pRock + "rocky_terrain_02_diff_2k.jpg").c_str());
    tRockN   = loadTexture((pRock + "rocky_terrain_02_nor_gl_2k.jpg").c_str());
    tRockARM = loadTexture((pRock + "rocky_terrain_02_arm_2k.jpg").c_str());
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
        std::cout << "Texture failed: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int Terrain::createDefaultARM(float ao, float roughness, float metallic) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    unsigned char data[3] = { (unsigned char)(ao * 255.0f), (unsigned char)(roughness * 255.0f), (unsigned char)(metallic * 255.0f) };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return textureID;
}

void Terrain::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    aiMesh* mesh = scene->mMeshes[0];
    std::vector<float> data;
    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        data.push_back(mesh->mVertices[i].x);
        data.push_back(mesh->mVertices[i].y);
        data.push_back(mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            data.push_back(mesh->mNormals[i].x);
            data.push_back(mesh->mNormals[i].y);
            data.push_back(mesh->mNormals[i].z);
        } else { data.insert(data.end(), {0.0f, 1.0f, 0.0f}); }

        if(mesh->HasVertexColors(0)) {
            data.push_back(mesh->mColors[0][i].r);
            data.push_back(mesh->mColors[0][i].g);
            data.push_back(mesh->mColors[0][i].b);
        } else { data.insert(data.end(), {0.0f, 0.0f, 0.0f}); }

        if(mesh->HasTextureCoords(0)) {
            data.push_back(mesh->mTextureCoords[0][i].x);
            data.push_back(mesh->mTextureCoords[0][i].y);
        } else { data.insert(data.end(), {0.0f, 0.0f}); }

        if(mesh->HasTangentsAndBitangents()) {
            data.push_back(mesh->mTangents[i].x);
            data.push_back(mesh->mTangents[i].y);
            data.push_back(mesh->mTangents[i].z);
        } else { data.insert(data.end(), {0.0f, 0.0f, 0.0f}); }
    }
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
    }
    indexCount = indices.size();

    // DATEN SICHERN für Grass-Positionierung
    this->m_vertices = data;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    int stride = 14 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float))); glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float))); glEnableVertexAttribArray(4);
}