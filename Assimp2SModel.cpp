#include "Assimp2SModel.h"

#include <vector>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::vector<MeshData> collectModelMeshes(SModelData &modeldata, const aiScene *scene, aiNode *node);
std::string collectModelName(const aiScene *scene, aiNode *node);
MeshData processMesh(SModelData &modeldata, const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, const aiMaterial* material);
BoneData* collectBones(const aiScene *scene, const aiNode *node);
std::vector<BoneWeightData> processBoneWeights(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, int vertexOffset);
std::vector<MaterialData> processMaterials(const aiMaterial *mat, aiTextureType assimp_type, int type);
void selectNecessaryNodes(aiNode* node, const aiNode* modelRoot);
bool compareMeshNodeOrParent(const aiNode* node, const aiNode* modelRoot);
float** getOffsetMatrix(const aiScene *scene, const aiString boneName);
aiMatrix4x4 getDerivedTransform(const aiNode* node, const aiNode* sceneRoot);
aiMatrix4x4 getInverseDerivedTransform(const aiNode* node, const aiNode* sceneRoot);
float** convertAssimpMatrix4(const aiMatrix4x4 matrix);

aiNode* sceneRoot = NULL;
aiNode* modelRoot = NULL;
aiNode* boneRoot = NULL;
std::map<aiNode*, bool> necessaryNodes;

unsigned int boneIdSeq;
std::map<std::string, unsigned int> boneIdMap;

bool convertAssimp2SModel(SModelData &modeldata, std::string path){
    unsigned flags =
        aiProcess_Triangulate |
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

    //if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    if(!scene || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        return false;
    }

    //directory = path.substr(0, path.find_last_of('/'));

    boneIdSeq = 0;
    boneIdMap.clear();
    sceneRoot = scene->mRootNode;

    modeldata.name = collectModelName(scene, sceneRoot);
    modeldata.meshes = collectModelMeshes(modeldata, scene, modelRoot);
    modeldata.skeleton = collectBones(scene, boneRoot);

    return true;
}


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

std::vector<MeshData> collectModelMeshes(SModelData &modeldata, const aiScene *scene, aiNode *node){

    std::vector<MeshData> meshes;

    for(unsigned int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        meshes.push_back(processMesh(modeldata, scene, node, mesh, material));
    }

    return meshes;

}

float** convertAssimpMatrix4(const aiMatrix4x4 matrix){
    float** convMatrix = new float*[4];
    for (int i = 0; i < 4; i++){
        convMatrix[i] = new float[4];
        for (int j = 0; j < 4; j++){
            //Convert row-major to column-major
            convMatrix[i][j] = matrix[j][i];
        }
    }

    return convMatrix;
}

aiMatrix4x4 getInverseDerivedTransform(const aiNode* node, const aiNode* sceneRoot){
    aiMatrix4x4 nodeDerivedInverse;

    if (modelRoot != sceneRoot)
        nodeDerivedInverse = getDerivedTransform(node, sceneRoot);
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

        boneData->bindRotation.w = aiRot.w;
        boneData->bindRotation.x = aiRot.x;
        boneData->bindRotation.y = aiRot.y;
        boneData->bindRotation.z = aiRot.z;

        boneData->bindScale.x = aiScale.x;
        boneData->bindScale.y = aiScale.y;
        boneData->bindScale.z = aiScale.z;

        boneData->name = node->mName.C_Str();

        if (boneIdMap.count(boneData->name) > 0){
            boneData->boneId = boneIdMap[boneData->name];
        }else{
            boneData->boneId = ++boneIdSeq;
        }

        float** resultOffsetMatrix = getOffsetMatrix(scene, node->mName);
        for (int i = 0; i < 4; i++){
            for (int j = 0; j < 4; j++){
                boneData->offsetMatrix[i][j] = resultOffsetMatrix[i][j];
            }
        }

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

MeshData processMesh(SModelData &modeldata, const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, const aiMaterial* material){

    int vertexOffset = (int)modeldata.vertices.size();

    modeldata.vertexMask |= VERTEX_ELEMENT_POSITION;
    if (mesh->HasTextureCoords(0)){
        modeldata.vertexMask |= VERTEX_ELEMENT_UV0;
    }
    if (mesh->HasTextureCoords(1)){
        modeldata.vertexMask |= VERTEX_ELEMENT_UV1;
    }
    if (mesh->HasNormals()){
        modeldata.vertexMask |= VERTEX_ELEMENT_NORMAL;
    }
    if (mesh->HasTangentsAndBitangents()){
        modeldata.vertexMask |= VERTEX_ELEMENT_TANGENT;
        modeldata.vertexMask |= VERTEX_ELEMENT_BITANGENT;
    }

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){

        VertexData vertexData;

        Vector3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        vertexData.positions.push_back(vertex);
        
        for (unsigned uv = 0; uv < mesh->GetNumUVChannels(); uv++){
            Vector2 texcoord;
            if(mesh->HasTextureCoords(uv)) {
                texcoord.x = mesh->mTextureCoords[uv][i].x; 
                texcoord.y = mesh->mTextureCoords[uv][i].y;
                if (uv == 0){
                    vertexData.texcoords0.push_back(texcoord);
                } else if (uv == 1){
                    vertexData.texcoords1.push_back(texcoord);
                }
            }
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
        vertexData.normals.push_back(normal);

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
        vertexData.tangents.push_back(tangent);
        vertexData.bitangents.push_back(bitangent);

        modeldata.vertices.push_back(vertexData);
        
    }

    modeldata.boneWeights = processBoneWeights(scene, modelRoot, mesh, vertexOffset);

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

std::vector<BoneWeightData> processBoneWeights(const aiScene *scene, const aiNode* modelRoot, const aiMesh *mesh, int vertexOffset){

    std::vector<BoneWeightData> bonesData;

    for(unsigned int i = 0; i < mesh->mNumBones; i++){
        //printf("Bone name %s\n", mesh->mBones[i]->mName.C_Str());
        //printf("Matrix %f\n", mesh->mBones[i]->mOffsetMatrix[0][0]);

        BoneWeightData boneData;

        boneData.boneId = ++boneIdSeq;

        boneIdMap[mesh->mBones[i]->mName.C_Str()] = boneData.boneId;

        for (uint j = 0 ; j < mesh->mBones[i]->mNumWeights ; j++) {
            BoneVertexWeightData vertexWeight;

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