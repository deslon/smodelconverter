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

struct MeshVertexData {
    Vector3 vertex;
    Vector2 texcoord;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};

struct MeshMaterialData {
    int type;
    std::string texture;
};

struct BoneVertexWeight{
    unsigned int vertexId;
    float weight;
};

struct BoneData {
    std::string name;
    std::vector<BoneVertexWeight> vertexWeights;
};

struct MeshNodeData {
    std::string name;
    std::vector<MeshVertexData> meshVertices;
    std::vector<unsigned int> indices;
    std::vector<MeshMaterialData> materials;
    std::vector<BoneData> bones;
};

struct ModelNodeData {
    std::string name;
    std::vector<MeshNodeData> meshNodes;
    std::vector<ModelNodeData> nodes;
};

void writeMeshVerticesVector(std::ostream& os, const std::vector<MeshVertexData> &vec);
void writeIndicesVector(std::ostream& os, const std::vector<unsigned int> &vec);
void writeString(std::ostream& os, const std::string &str);
void writeMeshMaterialsVector(std::ostream& os, const std::vector<MeshMaterialData> &vec);
void writeMeshNodesVector(std::ostream& os, const std::vector<MeshNodeData> &vec);
void writeModelNodesVector(std::ostream& os, const std::vector<ModelNodeData> &vec);
void writeModelNode(std::ostream& os, const ModelNodeData &modelNode);

void readMeshVerticesVector(std::istream& is, std::vector<MeshVertexData> &vec);
void readIndicesVector(std::istream& is, std::vector<unsigned int> &vec);
void readString(std::istream& is, std::string &str);
void readMeshMaterialsVector(std::istream& is, std::vector<MeshMaterialData> &vec);
void readMeshNodesVector(std::istream& is, std::vector<MeshNodeData> &vec);

ModelNodeData processNode(aiNode *node, const aiScene *scene);
MeshNodeData processMesh(const aiMesh *mesh, const aiMaterial* material);
std::vector<BoneData> processBones(const aiMesh *mesh);
std::vector<MeshMaterialData> processMaterials(const aiMaterial *mat, aiTextureType assimp_type, int type);

void printNode(const ModelNodeData &modelNode, int layerTree = 0);

ModelNodeData modeldata;

int main (int argc, char *argv[]){
    if (argc < 2){
        fprintf(stdout,"Usage: %s <input file> [output file] [options]\n",argv[0]);
        return 1;
    }

    std::string path = argv[1];

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        return 1;
    }

    //directory = path.substr(0, path.find_last_of('/'));

    modeldata = processNode(scene->mRootNode, scene);
/*
    int version = 1;

    std::ofstream os;
    os.open("test.smodel", std::ios::out | std::ios::binary);
    os.write("SMODEL", sizeof(char) * 6);
    os.write((char*)&version, sizeof(int));
    writeModelNode(os, modeldata);
    os.close();
    */
/*
    char* readsig= new char[6];
    int readversion;
    ModelNodeData modelDataTest;

    std::ifstream is;
    is.open("test.smodel", std::ios::in | std::ios::binary);
    is.read(readsig, sizeof(char) * 6);
    is.read((char*)&readversion, sizeof(int));
    readMeshNodesVector(is, modelDataTest.meshNodes);
    is.close();
*/
    printf("\n-----------Model summary---------------\n\n");

    printNode(modeldata);
/*
    printf("Nodes count: %i\n", (int)modeldata.meshNodes.size());
    for (int i = 0; i < modeldata.meshNodes.size(); i++){
        printf("+++++++++\n");
        printf("Node name: %s\n", modeldata.meshNodes[i].name.c_str());
        printf("Vertex count: %i\n", (int)modeldata.meshNodes[i].meshVertices.size());
        if ( modeldata.meshNodes[i].meshVertices.size() > 0)
            printf("First vertex: x: %f, y: %f, z: %f\n", modeldata.meshNodes[i].meshVertices[0].vertex.x, modeldata.meshNodes[i].meshVertices[0].vertex.y, modeldata.meshNodes[i].meshVertices[0].vertex.z);
        printf("Index count: %i\n", (int)modeldata.meshNodes[i].indices.size());
        if ( modeldata.meshNodes[i].materials.size() > 0)
            printf("Texture: %s\n", modeldata.meshNodes[i].materials[0].texture.c_str());
    }
*/    
    printf("+++++++++\n");
/*
    printf("\n\n-----------Read test---------------\n\n");
    printf("Signature: %s\n", readsig);
    printf("Version: %i\n", readversion);

    printf("Nodes count: %i\n", (int)modelDataTest.meshNodes.size());
    for (int i = 0; i < modelDataTest.meshNodes.size(); i++){
        printf("+++++++++\n");
        printf("Node name: %s\n", modelDataTest.meshNodes[i].name.c_str());
        printf("Vertex count: %i\n", (int)modelDataTest.meshNodes[i].meshVertices.size());
        if ( modelDataTest.meshNodes[i].meshVertices.size() > 0)
            printf("First vertex: x: %f, y: %f, z: %f\n", modelDataTest.meshNodes[i].meshVertices[0].vertex.x, modelDataTest.meshNodes[i].meshVertices[0].vertex.y, modelDataTest.meshNodes[i].meshVertices[0].vertex.z);
        printf("Index count: %i\n", (int)modelDataTest.meshNodes[i].indices.size());
        if ( modelDataTest.meshNodes[i].materials.size() > 0)
            printf("Texture: %s\n", modelDataTest.meshNodes[i].materials[0].texture.c_str());
    }
    printf("+++++++++\n");
*/
  return 0;
}

void printNode(const ModelNodeData &modelNode, int layerTree){
    std::string strtree;
    for(int i = 0; i < layerTree; i++){
        strtree += "-";
    }
    printf("%sNode name: %s\n", strtree.c_str(), modelNode.name.c_str());

    if (modelNode.meshNodes.size() > 0){
        for (int i = 0; i < modelNode.meshNodes.size(); i++){
            printf("%s>Mesh (%s), vertices: %i, bones: %i\n", strtree.c_str(), 
                modelNode.meshNodes[i].name.c_str(), 
                (int)modelNode.meshNodes[i].meshVertices.size(), 
                (int)modelNode.meshNodes[i].bones.size());

            for (int b = 0; b < modelNode.meshNodes[i].bones.size(); b++){
                printf("%s>>Bone (%s), weights: %i\n", strtree.c_str(), 
                    modelNode.meshNodes[i].bones[b].name.c_str(), 
                    (int)modelNode.meshNodes[i].bones[b].vertexWeights.size());
            }
        }
    }

    for (int i = 0; i < modelNode.nodes.size(); i++){
        printNode(modelNode.nodes[i], layerTree+1);
    }
    /*
    for (int i = 0; i < modeldata.meshNodes.size(); i++){
        printf("+++++++++\n");
        printf("Node name: %s\n", modeldata.meshNodes[i].name.c_str());
        printf("Vertex count: %i\n", (int)modeldata.meshNodes[i].meshVertices.size());
        if ( modeldata.meshNodes[i].meshVertices.size() > 0)
            printf("First vertex: x: %f, y: %f, z: %f\n", modeldata.meshNodes[i].meshVertices[0].vertex.x, modeldata.meshNodes[i].meshVertices[0].vertex.y, modeldata.meshNodes[i].meshVertices[0].vertex.z);
        printf("Index count: %i\n", (int)modeldata.meshNodes[i].indices.size());
        if ( modeldata.meshNodes[i].materials.size() > 0)
            printf("Texture: %s\n", modeldata.meshNodes[i].materials[0].texture.c_str());
    }
    */
}

void writeMeshVerticesVector(std::ostream& os, const std::vector<MeshVertexData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * sizeof(MeshVertexData));
}

void writeIndicesVector(std::ostream& os, const std::vector<unsigned int> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void writeString(std::ostream& os, const std::string &str){
    size_t size = str.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&str[0], size);
}

void writeMeshMaterialsVector(std::ostream& os, const std::vector<MeshMaterialData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os.write((char*)&vec[i].type, sizeof(int));
        writeString(os, vec[i].texture);
    }
}

void writeMeshNodesVector(std::ostream& os, const std::vector<MeshNodeData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeString(os, vec[i].name);
        writeMeshVerticesVector(os, vec[i].meshVertices);
        writeIndicesVector(os, vec[i].indices);
        writeMeshMaterialsVector(os, vec[i].materials);
    }
}

void writeModelNodesVector(std::ostream& os, const std::vector<ModelNodeData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeModelNode(os, vec[i]);
    }
}

void writeModelNode(std::ostream& os, const ModelNodeData &modelNode){
    writeString(os, modelNode.name);
    writeMeshNodesVector(os, modelNode.meshNodes);
    writeModelNodesVector(os, modelNode.nodes);
}

void readMeshVerticesVector(std::istream& is, std::vector<MeshVertexData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * sizeof(MeshVertexData));
}

void readIndicesVector(std::istream& is, std::vector<unsigned int> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void readString(std::istream& is, std::string &str){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    str.resize(size);
    is.read((char*)&str[0], size);
}

void readMeshMaterialsVector(std::istream& is, std::vector<MeshMaterialData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        is.read((char*)&vec[i].type, sizeof(int));
        readString(is, vec[i].texture);
    }
}

void readMeshNodesVector(std::istream& is, std::vector<MeshNodeData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readString(is, vec[i].name);
        readMeshVerticesVector(is, vec[i].meshVertices);
        readIndicesVector(is, vec[i].indices);
        readMeshMaterialsVector(is, vec[i].materials);
    }
}

ModelNodeData processNode(aiNode *node, const aiScene *scene){
    ModelNodeData modelNode;

    modelNode.name = node->mName.C_Str();

    for(unsigned int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        modelNode.meshNodes.push_back(processMesh(mesh, material));
    }
    
    for(unsigned int i = 0; i < node->mNumChildren; i++){
        modelNode.nodes.push_back(processNode(node->mChildren[i], scene));
    }

    return modelNode;
}

MeshNodeData processMesh(const aiMesh *mesh, const aiMaterial* material){

    MeshNodeData meshNode;

    meshNode.name = mesh->mName.C_Str();

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){

        MeshVertexData meshVertex;

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

        if (mesh->HasNormals()){
            meshVertex.normal.x = mesh->mNormals[i].x;
            meshVertex.normal.y = mesh->mNormals[i].y;
            meshVertex.normal.z = mesh->mNormals[i].z;
        }else{
            meshVertex.normal.x = 0.0f;
            meshVertex.normal.y = 0.0f;
            meshVertex.normal.z = 0.0f;
        }

        if (mesh->HasTangentsAndBitangents()){
            meshVertex.tangent.x = mesh->mTangents[i].x;
            meshVertex.tangent.y = mesh->mTangents[i].y;
            meshVertex.tangent.z = mesh->mTangents[i].z;

            meshVertex.bitangent.x = mesh->mBitangents[i].x;
            meshVertex.bitangent.y = mesh->mBitangents[i].y;
            meshVertex.bitangent.z = mesh->mBitangents[i].z;
        }else{
            meshVertex.tangent.x = 0.0f;
            meshVertex.tangent.y = 0.0f;
            meshVertex.tangent.z = 0.0f;

            meshVertex.bitangent.x = 0.0f;
            meshVertex.bitangent.y = 0.0f;
            meshVertex.bitangent.z = 0.0f;
        }

        meshNode.meshVertices.push_back(meshVertex);
        
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            meshNode.indices.push_back(face.mIndices[j]);
    }

    meshNode.materials = processMaterials(material, aiTextureType_DIFFUSE, 1);
    meshNode.bones = processBones(mesh);
    
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

std::vector<BoneData> processBones(const aiMesh *mesh){

    std::vector<BoneData> bonesData;

    for(unsigned int i = 0; i < mesh->mNumBones; i++){
        //printf("Bone name %s\n", mesh->mBones[i]->mName.C_Str());
        //printf("Matrix %f\n", mesh->mBones[i]->mOffsetMatrix[0][0]);

        BoneData boneData;

        boneData.name = mesh->mBones[i]->mName.C_Str();

        for (uint j = 0 ; j < mesh->mBones[i]->mNumWeights ; j++) {
            BoneVertexWeight vertexWeight;

            vertexWeight.vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
            vertexWeight.weight = mesh->mBones[i]->mWeights[j].mWeight;

            boneData.vertexWeights.push_back(vertexWeight);
        }

        bonesData.push_back(boneData);
    }

    return bonesData;
}

std::vector<MeshMaterialData> processMaterials(const aiMaterial *mat, aiTextureType assimp_type, int type){
    std::vector<MeshMaterialData> material;

    for(unsigned int i = 0; i < mat->GetTextureCount(assimp_type); i++){
        aiString str;
        mat->GetTexture(assimp_type, i, &str);

        MeshMaterialData meshMaterial;
        meshMaterial.type = type;
        meshMaterial.texture = str.C_Str();

        material.push_back(meshMaterial);
    }

    return material;
}