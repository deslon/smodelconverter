#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <iostream>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vector2 {
    float x;
    float y;
};

struct MeshVertex {
    Vector3 vertex;
    Vector2 texCoords;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};

struct MeshNode {
    std::vector<MeshVertex> meshVertices;
};


std::string directory;
std::vector<MeshNode> meshNodes;

void processNode(aiNode *node, const aiScene *scene);
MeshNode processMesh(aiMesh *mesh, const aiScene *scene);

int main (int argc, char *argv[]){
    if (argc < 2){
        fprintf(stdout,"Usage: %s <input file> <output file> [options]\n",argv[0]);
        return 1;
    }

    std::string path = argv[1];

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        return 1;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    typename std::vector<MeshNode>::size_type size = meshNodes.size();

    std::ofstream ofile;
    ofile.open("test.txt", std::ios::out | std::ios::binary);
    ofile.write((char*)&size, sizeof(size));
    ofile.write((char*)&meshNodes.front(), sizeof(MeshNode) * meshNodes.size());
    ofile.close();


    std::vector<MeshNode> meshNodesTest;
    typename std::vector<MeshNode>::size_type size2 = 0;

    std::ifstream ifile;
    ifile.open("test.txt", std::ios::in | std::ios::binary);
    ifile.read((char*)&size2, sizeof(size2));
    meshNodesTest.resize(size2);
    ifile.read((char*)&meshNodesTest.front(), sizeof(MeshNode) * meshNodes.size());
    ifile.close();


    for (int i = 0; i < meshNodes.size(); i++){
        printf("Vertex: x: %f, y: %f, z: %f\n", meshNodes[i].meshVertices[0].vertex.x, meshNodes[i].meshVertices[0].vertex.y, meshNodes[i].meshVertices[0].vertex.z);
    }
/*
    for (int i = 0; i < meshNodesTest.size(); i++){
        printf("Vertex2: x: %f, y: %f, z: %f\n", meshNodesTest[i].meshVertices[0].vertex.x, meshNodesTest[i].meshVertices[0].vertex.y, meshNodesTest[i].meshVertices[0].vertex.z);
    }
*/
  return 0;
}

void processNode(aiNode *node, const aiScene *scene){

    for(unsigned int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshNodes.push_back(processMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++){
        processNode(node->mChildren[i], scene);
    }

}

MeshNode processMesh(aiMesh *mesh, const aiScene *scene){

    MeshNode meshNode;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){

        MeshVertex meshVertex;

        meshVertex.vertex.x = mesh->mVertices[i].x;
        meshVertex.vertex.y = mesh->mVertices[i].y;
        meshVertex.vertex.z = mesh->mVertices[i].z;

        if(mesh->mTextureCoords[0]) {
            meshVertex.texCoords.x = mesh->mTextureCoords[0][i].x; 
            meshVertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            meshVertex.texCoords.x = 0.0f; 
            meshVertex.texCoords.y = 0.0f;
        }

        meshVertex.normal.x = mesh->mNormals[i].x;
        meshVertex.normal.y = mesh->mNormals[i].y;
        meshVertex.normal.z = mesh->mNormals[i].z;

        meshVertex.tangent.x = mesh->mTangents[i].x;
        meshVertex.tangent.y = mesh->mTangents[i].y;
        meshVertex.tangent.z = mesh->mTangents[i].z;

        meshVertex.bitangent.x = mesh->mBitangents[i].x;
        meshVertex.bitangent.y = mesh->mBitangents[i].y;
        meshVertex.bitangent.z = mesh->mBitangents[i].z;

        meshNode.meshVertices.push_back(meshVertex);
    }

    return meshNode;

}