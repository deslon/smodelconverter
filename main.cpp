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
    Vector2 texcoord;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};

struct MeshMaterial {
    int type;
    std::string texture;
};

struct MeshNode {
    std::string name;
    std::vector<MeshVertex> meshVertices;
    std::vector<unsigned int> indices;
    std::vector<MeshMaterial> materials;
};

struct MeshData {
    std::vector<MeshNode> meshNodes;
};

void writeMeshVerticesVector(std::ostream& os, const std::vector<MeshVertex> &vec);
void writeIndicesVector(std::ostream& os, const std::vector<unsigned int> &vec);
void writeString(std::ostream& os, const std::string &str);
void writeMeshMaterialsVector(std::ostream& os, const std::vector<MeshMaterial> &vec);
void writeMeshNodesVector(std::ostream& os, const std::vector<MeshNode> &vec);

void readMeshVerticesVector(std::istream& is, std::vector<MeshVertex> &vec);
void readIndicesVector(std::istream& is, std::vector<unsigned int> &vec);
void readString(std::istream& is, std::string &str);
void readMeshMaterialsVector(std::istream& is, std::vector<MeshMaterial> &vec);
void readMeshNodesVector(std::istream& is, std::vector<MeshNode> &vec);

void processNode(aiNode *node, const aiScene *scene);
MeshNode processMesh(aiMesh *mesh, const aiScene *scene, const aiString& name);
std::vector<MeshMaterial> loadMaterialTextures(aiMaterial *mat, aiTextureType assimp_type, int type);

MeshData meshdata;

int main (int argc, char *argv[]){
    if (argc < 2){
        fprintf(stdout,"Usage: %s <input file> [output file] [options]\n",argv[0]);
        return 1;
    }

    std::string path = argv[1];

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        return 1;
    }

    //directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    int version = 1;

    std::ofstream os;
    os.open("test.smodel", std::ios::out | std::ios::binary);
    os.write("SMODEL", sizeof(char) * 6);
    os.write((char*)&version, sizeof(int));
    writeMeshNodesVector(os, meshdata.meshNodes);
    os.close();

    char* readsig= new char[6];
    int readversion;
    MeshData meshDataTest;

    std::ifstream is;
    is.open("test.smodel", std::ios::in | std::ios::binary);
    is.read(readsig, sizeof(char) * 6);
    is.read((char*)&readversion, sizeof(int));
    readMeshNodesVector(is, meshDataTest.meshNodes);
    is.close();

    printf("\n-----------Model summary---------------\n\n");

    printf("Nodes count: %i\n", (int)meshdata.meshNodes.size());
    for (int i = 0; i < meshdata.meshNodes.size(); i++){
        printf("+++++++++\n");
        printf("Node num: %s\n", meshdata.meshNodes[i].name.c_str());
        printf("Vertex count: %i\n", (int)meshdata.meshNodes[i].meshVertices.size());
        if ( meshdata.meshNodes[i].meshVertices.size() > 0)
            printf("First vertex: x: %f, y: %f, z: %f\n", meshdata.meshNodes[i].meshVertices[0].vertex.x, meshdata.meshNodes[i].meshVertices[0].vertex.y, meshdata.meshNodes[i].meshVertices[0].vertex.z);
        printf("Index count: %i\n", (int)meshdata.meshNodes[i].indices.size());
        if ( meshdata.meshNodes[i].materials.size() > 0)
            printf("Texture: %s\n", meshdata.meshNodes[i].materials[0].texture.c_str());
    }
    printf("+++++++++\n");

    printf("\n\n-----------Read test---------------\n\n");
    printf("Signature: %s\n", readsig);
    printf("Version: %i\n", readversion);

    printf("Nodes count: %i\n", (int)meshDataTest.meshNodes.size());
    for (int i = 0; i < meshDataTest.meshNodes.size(); i++){
        printf("+++++++++\n");
        printf("Node num: %s\n", meshDataTest.meshNodes[i].name.c_str());
        printf("Vertex count: %i\n", (int)meshDataTest.meshNodes[i].meshVertices.size());
        if ( meshDataTest.meshNodes[i].meshVertices.size() > 0)
            printf("First vertex: x: %f, y: %f, z: %f\n", meshDataTest.meshNodes[i].meshVertices[0].vertex.x, meshDataTest.meshNodes[i].meshVertices[0].vertex.y, meshDataTest.meshNodes[i].meshVertices[0].vertex.z);
        printf("Index count: %i\n", (int)meshDataTest.meshNodes[i].indices.size());
        if ( meshDataTest.meshNodes[i].materials.size() > 0)
            printf("Texture: %s\n", meshDataTest.meshNodes[i].materials[0].texture.c_str());
    }
    printf("+++++++++\n");

  return 0;
}

void writeMeshVerticesVector(std::ostream& os, const std::vector<MeshVertex> &vec){
    typename std::vector<MeshVertex>::size_type size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * sizeof(MeshVertex));
}

void writeIndicesVector(std::ostream& os, const std::vector<unsigned int> &vec){
    typename std::vector<unsigned int>::size_type size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void writeString(std::ostream& os, const std::string &str){
    typename std::string::size_type size = str.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&str[0], size);
}

void writeMeshMaterialsVector(std::ostream& os, const std::vector<MeshMaterial> &vec){
    typename std::vector<MeshMaterial>::size_type size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (typename std::vector<MeshMaterial>::size_type i = 0; i < size; ++i){
        os.write((char*)&vec[i].type, sizeof(int));
        writeString(os, vec[i].texture);
    }
}

void writeMeshNodesVector(std::ostream& os, const std::vector<MeshNode> &vec){
    typename std::vector<MeshNode>::size_type size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (typename std::vector<MeshNode>::size_type i = 0; i < size; ++i){
        writeString(os, vec[i].name);
        writeMeshVerticesVector(os, vec[i].meshVertices);
        writeIndicesVector(os, vec[i].indices);
        writeMeshMaterialsVector(os, vec[i].materials);
    }
}

void readMeshVerticesVector(std::istream& is, std::vector<MeshVertex> &vec){
    typename std::vector<MeshVertex>::size_type size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * sizeof(MeshVertex));
}

void readIndicesVector(std::istream& is, std::vector<unsigned int> &vec){
    typename std::vector<unsigned int>::size_type size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void readString(std::istream& is, std::string &str){
    typename std::string::size_type size = 0;
    is.read((char*)&size, sizeof(size));
    str.resize(size);
    is.read((char*)&str[0], size);
}

void readMeshMaterialsVector(std::istream& is, std::vector<MeshMaterial> &vec){
    typename std::vector<MeshMaterial>::size_type size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (typename std::vector<MeshNode>::size_type i = 0; i < size; ++i){
        is.read((char*)&vec[i].type, sizeof(int));
        readString(is, vec[i].texture);
    }
}

void readMeshNodesVector(std::istream& is, std::vector<MeshNode> &vec){
    typename std::vector<MeshVertex>::size_type size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (typename std::vector<MeshNode>::size_type i = 0; i < size; ++i){
        readString(is, vec[i].name);
        readMeshVerticesVector(is, vec[i].meshVertices);
        readIndicesVector(is, vec[i].indices);
        readMeshMaterialsVector(is, vec[i].materials);
    }
}

void processNode(aiNode *node, const aiScene *scene){

    for(unsigned int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshdata.meshNodes.push_back(processMesh(mesh, scene, node->mName));
    }
    

    for(unsigned int i = 0; i < node->mNumChildren; i++){
        processNode(node->mChildren[i], scene);
    }

}

MeshNode processMesh(aiMesh *mesh, const aiScene *scene, const aiString& name){

    MeshNode meshNode;

    meshNode.name = name.C_Str();

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){

        MeshVertex meshVertex;

        meshVertex.vertex.x = mesh->mVertices[i].x;
        meshVertex.vertex.y = mesh->mVertices[i].y;
        meshVertex.vertex.z = mesh->mVertices[i].z;

        if(mesh->HasTextureCoords(0)) {
            meshVertex.texcoord.x = mesh->mTextureCoords[0][i].x; 
            meshVertex.texcoord.y = mesh->mTextureCoords[0][i].y;
        } else {
            meshVertex.texcoord.x = 0.0f; 
            meshVertex.texcoord.y = 0.0f;
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

    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            meshNode.indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    meshNode.materials = loadMaterialTextures(material, aiTextureType_SHININESS, 1);
/*
    std::cout << "aiTextureType_NONE: "         << material->GetTextureCount(aiTextureType_NONE) << std::endl;
    std::cout << "aiTextureType_DIFFUSE: "      << material->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
    std::cout << "aiTextureType_AMBIENT: "      << material->GetTextureCount(aiTextureType_AMBIENT) << std::endl;
    std::cout << "aiTextureType_EMISSIVE: "     << material->GetTextureCount(aiTextureType_EMISSIVE) << std::endl;
    std::cout << "aiTextureType_HEIGHT: "       << material->GetTextureCount(aiTextureType_HEIGHT) << std::endl;
    std::cout << "aiTextureType_NORMALS: "      << material->GetTextureCount(aiTextureType_NORMALS) << std::endl;
    std::cout << "aiTextureType_SHININESS: "    << material->GetTextureCount(aiTextureType_SHININESS) << std::endl;
    std::cout << "aiTextureType_OPACITY: "      << material->GetTextureCount(aiTextureType_OPACITY) << std::endl;
    std::cout << "aiTextureType_DISPLACEMENT: " << material->GetTextureCount(aiTextureType_DISPLACEMENT) << std::endl;
    std::cout << "aiTextureType_LIGHTMAP: "     << material->GetTextureCount(aiTextureType_LIGHTMAP) << std::endl;
    std::cout << "aiTextureType_REFLECTION: "   << material->GetTextureCount(aiTextureType_REFLECTION) << std::endl;
    std::cout << "aiTextureType_UNKNOWN: "      << material->GetTextureCount(aiTextureType_UNKNOWN) << std::endl;
*/
    return meshNode;

}

std::vector<MeshMaterial> loadMaterialTextures(aiMaterial *mat, aiTextureType assimp_type, int type){
    std::vector<MeshMaterial> material;
    for(unsigned int i = 0; i < mat->GetTextureCount(assimp_type); i++){
        aiString str;
        mat->GetTexture(assimp_type, i, &str);
        material.push_back({type, str.C_Str()});
    }
    return material;
}