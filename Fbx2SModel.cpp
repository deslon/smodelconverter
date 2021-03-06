#include "Fbx2SModel.h"

#include "util/RawModel.hpp"
#include "util/Fbx2Raw.hpp"
#include "util/String_Utils.hpp"

#include <map>

void collectName(SModelData &modeldata, RawModel &raw){
    modeldata.name = raw.GetNode(raw.GetNodeById(raw.GetRootNode())).name;
}

void collectVertices(SModelData &modeldata, RawModel &raw){
    int numVertices = raw.GetVertexCount();
    int vertexAttributes = raw.GetVertexAttributes();

    modeldata.vertexMask |= VERTEX_ELEMENT_POSITION;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_UV0;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_UV1;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_NORMAL;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_TANGENT;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_BINORMAL) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_BITANGENT;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_BONE_INDICES;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) != 0)
        modeldata.vertexMask |= VERTEX_ELEMENT_BONE_WEIGHTS;


    for (int i = 0; i < numVertices; i++){
        VertexData vertexData;

        RawVertex rawvertex = raw.GetVertex(i);

        Vector3 vertex;
        vertex.x = rawvertex.position[0];
        vertex.y = rawvertex.position[1];
        vertex.z = rawvertex.position[2];
        vertexData.position = vertex;

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
            Vector3 normal;
            normal.x = rawvertex.normal[0];
            normal.y = rawvertex.normal[1];
            normal.z = rawvertex.normal[2];
            vertexData.normal = normal;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
            Vector2 uv;
            uv.x = rawvertex.uv0[0];
            uv.y = 1.0 - rawvertex.uv0[1];
            vertexData.texcoord0 = uv;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
            Vector2 uv;
            uv.x = rawvertex.uv1[0];
            uv.y = 1.0 - rawvertex.uv1[1];
            vertexData.texcoord1 = uv;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
            Vector3 tangent;
            tangent.x = rawvertex.tangent[0];
            tangent.y = rawvertex.tangent[1];
            tangent.z = rawvertex.tangent[2];
            vertexData.tangent = tangent;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_BINORMAL) != 0) {
            Vector3 bitangent;
            bitangent.x = rawvertex.binormal[0];
            bitangent.y = rawvertex.binormal[1];
            bitangent.z = rawvertex.binormal[2];
            vertexData.bitangent = bitangent;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) != 0) {
            Vector4 boneIndices;
            boneIndices.x = rawvertex.jointIndices[0];
            boneIndices.y = rawvertex.jointIndices[1];
            boneIndices.z = rawvertex.jointIndices[2];
            boneIndices.w = rawvertex.jointIndices[3];
            vertexData.boneIndices = boneIndices;
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) != 0) {
            Vector4 boneWeights;
            boneWeights.x = rawvertex.jointWeights[0];
            boneWeights.y = rawvertex.jointWeights[1];
            boneWeights.z = rawvertex.jointWeights[2];
            boneWeights.w = rawvertex.jointWeights[3];
            vertexData.boneWeights = boneWeights;
        }

        modeldata.vertices.push_back(vertexData);

    }
}

void collectSubmeshes(SModelData &modeldata, RawModel &raw, RawNode &skeletonRoot, std::map<long, FbxAMatrix>& inverseBindMatricesMap, std::map<long, long>& boneIndexMap){
    int trianglesCount = raw.GetTriangleCount();

    std::map<int, int> submeshmap;
    skeletonRoot.isJoint = false;
    RawSurface skeletonRawSurface;

    for (int i = 0; i < trianglesCount; i++){
        RawTriangle rawtriangle = raw.GetTriangle(i);
        
        if (submeshmap.count(rawtriangle.materialIndex) == 0){
            MeshData meshdata;
            modeldata.meshes.push_back(meshdata);
            submeshmap[rawtriangle.materialIndex] = modeldata.meshes.size()-1;
        }

        MeshData* meshp = &modeldata.meshes[submeshmap[rawtriangle.materialIndex]];

        RawMaterial rawmaterial = raw.GetMaterial(rawtriangle.materialIndex);
        RawSurface rawsurface = raw.GetSurface(rawtriangle.surfaceIndex);

        if (raw.GetNode(raw.GetNodeById(rawsurface.skeletonRootId)).isJoint){
            skeletonRoot = raw.GetNode(raw.GetNodeById(rawsurface.skeletonRootId));
            skeletonRawSurface = rawsurface;
        }

        //meshp->name = rawsurface.name;
        modeldata.name = rawsurface.name;

        meshp->name = "";
        meshp->indices.push_back(rawtriangle.verts[0]);
        meshp->indices.push_back(rawtriangle.verts[1]);
        meshp->indices.push_back(rawtriangle.verts[2]);

        for (int t = 0; t < RAW_TEXTURE_USAGE_MAX; t++){
            if (rawmaterial.textures[t] >= 0){
                RawTexture rawtexture = raw.GetTexture(rawmaterial.textures[t]);
            
                if (rawtexture.usage == RAW_TEXTURE_USAGE_DIFFUSE){
                    MaterialData texturedata;

                    texturedata.type = 1;
                    //if (!rawtexture.fileLocation.empty()){
                    //    texturedata.texture = rawtexture.fileLocation;
                    //}else{
                    //    texturedata.texture = rawtexture.fileName;
                    //}
                    texturedata.texture = StringUtils::GetFileNameString(StringUtils::GetCleanPathString(rawtexture.fileName));

                    //TODO: Prevent creation multiple of same texture
                    if (meshp->materials.size() == 0)
                        meshp->materials.push_back(texturedata);
                }
            
            }
        }
        
    }

    for (int i = 0; i < skeletonRawSurface.jointIds.size(); i++){
        inverseBindMatricesMap[skeletonRawSurface.jointIds[i]] = skeletonRawSurface.inverseBindMatrices[i];
        boneIndexMap[skeletonRawSurface.jointIds[i]] = i;
    }
}

void collectNodesHierarchy(SModelData &modeldata, RawModel &raw, RawNode &rawnode, std::map<long, FbxAMatrix>& inverseBindMatricesMap, std::map<long, long>& boneIndexMap, BoneData& boneData){
    boneData.name = rawnode.name;

    boneData.bindPosition.x = rawnode.translation[0];
    boneData.bindPosition.y = rawnode.translation[1];
    boneData.bindPosition.z = rawnode.translation[2];

    boneData.bindRotation.w = rawnode.rotation[3];
    boneData.bindRotation.x = rawnode.rotation[0];
    boneData.bindRotation.y = rawnode.rotation[1];
    boneData.bindRotation.z = rawnode.rotation[2];

    boneData.bindScale.x = rawnode.scale[0];
    boneData.bindScale.y = rawnode.scale[1];
    boneData.bindScale.z = rawnode.scale[2];

    if (rawnode.isJoint){
        boneData.boneIndex = boneIndexMap[rawnode.id];

        FbxAMatrix invBindMatrix = inverseBindMatricesMap[rawnode.id];
        for (int i = 0; i < 4; i++){
            for (int j = 0; j < 4; j++){
                boneData.offsetMatrix[i][j] = invBindMatrix.Get(i, j);
            }
        }
    }else{
        boneData.boneIndex = -1;

        for (int i = 0; i < 4; i++){
            boneData.offsetMatrix[i][i] = 1.0;
        }
        printf("Warning: node %s has a joint parent but is not a joint. Cannot get offsetMatrix (inverseBindMatrix) data.\n", rawnode.name.c_str());
    }

    for (int i = 0; i < rawnode.childIds.size(); i++){
        BoneData boneChild;
        collectNodesHierarchy(modeldata, raw, raw.GetNode(raw.GetNodeById(rawnode.childIds[i])), inverseBindMatricesMap, boneIndexMap, boneChild);
        boneData.children.push_back(boneChild);
    }
}

void collectAnimation(SModelData &modeldata, RawModel &raw){
    /*
    printf("Animation count %i\n", raw.GetAnimationCount()); 
    
    for (int i = 0; i < raw.GetAnimationCount(); i++){
        RawAnimation anim = raw.GetAnimation(i);

        printf("Animation %s %i %i %i\n", anim.name.c_str(), (int)anim.times.size(), (int)anim.channels[0].weights.size(), (int)anim.channels[0].translations.size());    
    }
    */  
}

bool convertFbx2SModel(SModelData &modeldata, std::string path){

    RawModel raw;
    RawNode skeletonRoot;
    std::map<long, FbxAMatrix> inverseBindMatricesMap;
    std::map<long, long> boneIndexMap;

    if (!LoadFBXFile(raw, path.c_str(), "png;jpg;jpeg")){
        return false;
    }

    //collectName(modeldata, raw); //Getting submesh name for model
    collectVertices(modeldata, raw);
    collectSubmeshes(modeldata, raw, skeletonRoot, inverseBindMatricesMap, boneIndexMap);
    if (skeletonRoot.isJoint){
        modeldata.skeleton = new BoneData;
        collectNodesHierarchy(modeldata, raw, skeletonRoot, inverseBindMatricesMap, boneIndexMap, *modeldata.skeleton);
    }
    collectAnimation(modeldata, raw);

    return true;
}