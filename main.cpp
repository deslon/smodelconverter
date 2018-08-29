#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>

#include <iostream>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "modeldata.h"
#include "readwriteSModel.h"

/*
void writeMeshVerticesVector(std::ostream& os, const std::vector<MeshVertexData> &vec);
void writeIndicesVector(std::ostream& os, const std::vector<unsigned int> &vec);
void writeString(std::ostream& os, const std::string &str);
void writeMeshMaterialsVector(std::ostream& os, const std::vector<MeshMaterialData> &vec);
void writeMeshNodesVector(std::ostream& os, const std::vector<MeshData> &vec);
void writeModelNodesVector(std::ostream& os, const std::vector<ModelData> &vec);
void writeModelNode(std::ostream& os, const ModelData &modelNode);

void readMeshVerticesVector(std::istream& is, std::vector<MeshVertexData> &vec);
void readIndicesVector(std::istream& is, std::vector<unsigned int> &vec);
void readString(std::istream& is, std::string &str);
void readMeshMaterialsVector(std::istream& is, std::vector<MeshMaterialData> &vec);
void readMeshNodesVector(std::istream& is, std::vector<MeshData> &vec);
*/
std::vector<MeshData> collectModelMeshes(const aiScene *scene, aiNode *node);
std::string collectModelName(const aiScene *scene, aiNode *node);
MeshData processMesh(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, const aiMaterial* material);
BoneData* collectBones(const aiScene *scene, const aiNode *node);
std::vector<BoneWeightData> processBonesWeights(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, int vertexOffset);
std::vector<MaterialData> processMaterials(const aiMaterial *mat, aiTextureType assimp_type, int type);
void selectNecessaryNodes(aiNode* node, const aiNode* modelRoot);
bool compareMeshNodeOrParent(const aiNode* node, const aiNode* modelRoot);
float** getOffsetMatrix(const aiScene *scene, const aiString boneName);
aiMatrix4x4 getDerivedTransform(const aiNode* node, const aiNode* sceneRoot);
aiMatrix4x4 getInverseDerivedTransform(const aiNode* node, const aiNode* sceneRoot);
float** convertAssimpMatrix4(const aiMatrix4x4 matrix);

void printModel(const ModelData &modelNode);
void printSkeleton(const BoneData &bone, int layerTree = 0);


ModelData modeldata;

aiNode* sceneRoot = NULL;
aiNode* modelRoot = NULL;
aiNode* boneRoot = NULL;
std::map<aiNode*, bool> necessaryNodes;

int main (int argc, char *argv[]){
    if (argc < 2){
        fprintf(stdout,"Usage: %s <input file> [output file] [options]\n",argv[0]);
        return 1;
    }

    std::string path = argv[1];

    unsigned flags =
        aiProcess_Triangulate |
        //aiProcess_ConvertToLeftHanded |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace | 
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        //aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FixInfacingNormals |
        aiProcess_FindInvalidData |
        aiProcess_GenUVCoords |
        aiProcess_FindInstances |
        aiProcess_OptimizeMeshes;

    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(path, flags);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        return 1;
    }

    //directory = path.substr(0, path.find_last_of('/'));

    sceneRoot = scene->mRootNode;

    modeldata.name = collectModelName(scene, sceneRoot);
    modeldata.meshes = collectModelMeshes(scene, modelRoot);
    modeldata.skeleton = collectBones(scene, boneRoot);

    std::ofstream os;
    os.open("test.smodel", std::ios::out | std::ios::binary);
    writeModel(os, modeldata);
    os.close();
    
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

    printModel(modeldata);
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
void printModel(const ModelData &modelData){

    printf("Model name: %s, vertices: %i, bone weights: %i\n", modelData.name.c_str(), (int)modelData.vertices.size(), (int)modelData.bonesWeights.size());

        for (int i = 0; i < modelData.meshes.size(); i++){
            printf(">Mesh (%s), indices: %i\n", 
                modelData.meshes[i].name.c_str(), 
                (int)modelData.meshes[i].indices.size());
/*
            for (int b = 0; b < modelData.meshes[i].bones.size(); b++){
                printf(">>Bone (%s), weights: %i\n", 
                    modelData.meshes[i].bones[b].name.c_str(), 
                    (int)modelData.meshes[i].bones[b].vertexWeights.size());
            }
*/
        }
    if (modelData.skeleton)
        printSkeleton(*modelData.skeleton);
}

void printSkeleton(const BoneData &bone, int layerTree){
    std::string strtree;
    for(int i = 0; i < layerTree; i++){
        strtree += "-";
    }
    printf("%sBone name: %s\n", strtree.c_str(), bone.name.c_str());
    printf("%s  Position: %f %f %f\n", strtree.c_str(), bone.bindPosition.x, bone.bindPosition.y, bone.bindPosition.z);
    printf("%s  Rotation: %f %f %f %f\n", strtree.c_str(), bone.bindRotation.x, bone.bindRotation.y, bone.bindRotation.z, bone.bindRotation.w);
    printf("%s  Scale: %f %f %f\n", strtree.c_str(), bone.bindScale.x, bone.bindScale.y, bone.bindScale.z);
    printf("%s  Offset:\n", strtree.c_str());
    for (unsigned int i = 0; i < 4; i++)
        printf("%s    %f %f %f %f\n", strtree.c_str(), bone.offsetMatrix[i][0], bone.offsetMatrix[i][1], bone.offsetMatrix[i][2], bone.offsetMatrix[i][3]);

    for (int i = 0; i < bone.children.size(); i++){
        printSkeleton(bone.children[i], layerTree+1);
    }

}

/*
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

void writeMeshNodesVector(std::ostream& os, const std::vector<MeshData> &vec){
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
    writeMeshNodesVector(os, modelNode.meshes);
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

void readMeshNodesVector(std::istream& is, std::vector<MeshData> &vec){
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
*/
std::string collectModelName(const aiScene *scene, aiNode *node){

    if (node->mNumMeshes > 0){

        modelRoot = node;

        return node->mName.C_Str();
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++){
        std::string childReturn = collectModelName(scene, node->mChildren[i]);
        if (!childReturn.empty())
            return childReturn;
    }

    return "";
}

std::vector<MeshData> collectModelMeshes(const aiScene *scene, aiNode *node){

    std::vector<MeshData> meshes;

    for(unsigned int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        meshes.push_back(processMesh(scene, node, mesh, material));
    }

    return meshes;

}

float** convertAssimpMatrix4(const aiMatrix4x4 matrix){
    float** convMatrix = new float*[4];
    for (int i = 0; i < 4; i++){
        convMatrix[i] = new float[4];
        for (int j = 0; j < 4; j++){
            convMatrix[i][j] = matrix[i][j];
        }
    }

    return convMatrix;
}

aiMatrix4x4 getInverseDerivedTransform(const aiNode* node, const aiNode* sceneRoot){
    aiMatrix4x4 nodeDerivedInverse;

    if (modelRoot != sceneRoot)
        nodeDerivedInverse = getDerivedTransform(modelRoot, sceneRoot);
    nodeDerivedInverse.Inverse();

    return nodeDerivedInverse;
}

float** getOffsetMatrix(const aiScene *scene, const aiString boneName){

    aiMatrix4x4 offset;

    for (unsigned i = 0; i < modelRoot->mNumMeshes; i++){

        aiMesh* mesh = scene->mMeshes[modelRoot->mMeshes[i]];
        for (unsigned j = 0; j < mesh->mNumBones; j++){

            aiBone* bone = mesh->mBones[j];
            if (boneName == bone->mName){
                offset = bone->mOffsetMatrix;
                offset *= getInverseDerivedTransform(modelRoot, sceneRoot);
                return convertAssimpMatrix4(offset);
            }
        }
    }

    if (boneName == modelRoot->mName){
        for (unsigned i = 0; i < modelRoot->mNumMeshes; i++){

            aiMesh* mesh = scene->mMeshes[modelRoot->mMeshes[i]];
            if (!mesh->HasBones()){
                offset *= getInverseDerivedTransform(modelRoot, sceneRoot);
                return convertAssimpMatrix4(offset);
            }

        }
    }

    return convertAssimpMatrix4(offset);
}

BoneData* collectBones(const aiScene *scene, const aiNode *node){

    if (necessaryNodes[(aiNode*)node]){

        BoneData* boneData = new BoneData;

        aiMatrix4x4 transform = node->mTransformation;

        if (node == boneRoot){
            transform = getDerivedTransform(node, sceneRoot);
        }

        aiVector3D aiPos;
        aiQuaternion aiRot;
        aiVector3D aiScale;
        transform.Decompose(aiScale, aiRot, aiPos);
    
        boneData->bindPosition.x = aiPos.x;
        boneData->bindPosition.y = aiPos.y;
        boneData->bindPosition.z = aiPos.z;

        boneData->bindRotation.x = aiRot.x;
        boneData->bindRotation.y = aiRot.y;
        boneData->bindRotation.z = aiRot.z;
        boneData->bindRotation.w = aiRot.w;

        boneData->bindScale.x = aiScale.x;
        boneData->bindScale.y = aiScale.y;
        boneData->bindScale.z = aiScale.z;

        boneData->name = node->mName.C_Str();

        boneData->offsetMatrix = getOffsetMatrix(scene, node->mName);

        for(unsigned int i = 0; i < node->mNumChildren; i++){
            if (node->mChildren[i]->mNumMeshes == 0)
                if (necessaryNodes[node->mChildren[i]])
                    boneData->children.push_back(*collectBones(scene, node->mChildren[i]));
        }


        return boneData;
    }

    return NULL;
    
}


aiMatrix4x4 getDerivedTransform(const aiNode* node, const aiNode* sceneRoot){

    aiMatrix4x4 transform = node->mTransformation;

    while (node && node != sceneRoot)
    {
        node = node->mParent;
        if (node)
            transform = node->mTransformation * transform;
    }
    return transform;
}

MeshData processMesh(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, const aiMaterial* material){

    int vertexOffset = (int)modeldata.vertices.size();

    modeldata.texcoords.resize(mesh->GetNumUVChannels());

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){

        Vector3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        modeldata.vertices.push_back(vertex);
        
        for (unsigned uv = 0; uv < mesh->GetNumUVChannels(); uv++){

            Vector2 texcoord;
            if(mesh->HasTextureCoords(uv)) {
                texcoord.x = mesh->mTextureCoords[uv][i].x; 
                texcoord.y = mesh->mTextureCoords[uv][i].y;
            } else {
                texcoord.x = 0.0f; 
                texcoord.y = 0.0f;
            }

            modeldata.texcoords[uv].push_back(texcoord);
        }

        Vector3 normal;
        if (mesh->HasNormals()){
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
        }else{
            normal.x = 0.0f;
            normal.y = 0.0f;
            normal.z = 0.0f;
        }
        modeldata.normals.push_back(normal);

        Vector3 tangent;
        Vector3 bitangent;
        if (mesh->HasTangentsAndBitangents()){
            tangent.x = mesh->mTangents[i].x;
            tangent.y = mesh->mTangents[i].y;
            tangent.z = mesh->mTangents[i].z;

            bitangent.x = mesh->mBitangents[i].x;
            bitangent.y = mesh->mBitangents[i].y;
            bitangent.z = mesh->mBitangents[i].z;
        }else{
            tangent.x = 0.0f;
            tangent.y = 0.0f;
            tangent.z = 0.0f;

            bitangent.x = 0.0f;
            bitangent.y = 0.0f;
            bitangent.z = 0.0f;
        }
        modeldata.tangents.push_back(tangent);
        modeldata.bitangents.push_back(bitangent);
        
    }

    modeldata.bonesWeights = processBonesWeights(scene, modelRoot, mesh, vertexOffset);

    //-----------SubsMesh----------------
    MeshData meshData;

    meshData.name = mesh->mName.C_Str();

    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            meshData.indices.push_back(vertexOffset + face.mIndices[j]);
    }

    meshData.materials = processMaterials(material, aiTextureType_DIFFUSE, 1);
    
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

    return meshData;
}

std::vector<BoneWeightData> processBonesWeights(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, int vertexOffset){

    std::vector<BoneWeightData> bonesData;

    for(unsigned int i = 0; i < mesh->mNumBones; i++){
        //printf("Bone name %s\n", mesh->mBones[i]->mName.C_Str());
        //printf("Matrix %f\n", mesh->mBones[i]->mOffsetMatrix[0][0]);

        BoneWeightData boneData;

        boneData.name = mesh->mBones[i]->mName.C_Str();

        for (uint j = 0 ; j < mesh->mBones[i]->mNumWeights ; j++) {
            BoneVertexWeight vertexWeight;

            vertexWeight.vertexId = vertexOffset + mesh->mBones[i]->mWeights[j].mVertexId;
            vertexWeight.weight = mesh->mBones[i]->mWeights[j].mWeight;

            boneData.vertexWeights.push_back(vertexWeight);
        }

        bonesData.push_back(boneData);

        selectNecessaryNodes(sceneRoot->FindNode(mesh->mBones[i]->mName), modelRoot);
    }

    return bonesData;
}

void selectNecessaryNodes(aiNode* node, const aiNode* modelRoot){
    while (node){
        if (!compareMeshNodeOrParent(node, modelRoot)){
            necessaryNodes[node] = true;
            boneRoot = node;
        }
        node = node->mParent;
    }
}

bool compareMeshNodeOrParent(const aiNode* node, const aiNode* modelRoot){
    aiNode* currentMeshNode = (aiNode*)modelRoot;

    while (currentMeshNode){
        if (currentMeshNode == node)
            return true;

        currentMeshNode = currentMeshNode->mParent;
    }

    return false;
}


std::vector<MaterialData> processMaterials(const aiMaterial *mat, aiTextureType assimp_type, int type){
    std::vector<MaterialData> material;

    for(unsigned int i = 0; i < mat->GetTextureCount(assimp_type); i++){
        aiString str;
        mat->GetTexture(assimp_type, i, &str);

        MaterialData meshMaterial;
        meshMaterial.type = type;
        meshMaterial.texture = str.C_Str();

        material.push_back(meshMaterial);
    }

    return material;
}