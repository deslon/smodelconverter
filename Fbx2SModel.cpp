#include "Fbx2SModel.h"

#include "util/RawModel.hpp"
#include "util/Fbx2Raw.hpp"
#include "util/String_Utils.hpp"

#include <map>

void collectName(SModelData &modeldata, RawModel &raw){
    modeldata.name = raw.GetNode(raw.GetNodeById(raw.GetRootNode())).name;
}

void collectVertices(SModelData &modeldata, RawModel &raw, std::map<int, std::vector<BoneVertexWeightData>>& boneWeightDataMap){
    int numVertices = raw.GetVertexCount();
    int vertexAttributes = raw.GetVertexAttributes();

    modeldata.vertexMask |= VERTEX_ELEMENT_POSITION;
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
        modeldata.vertexMask |= VERTEX_ELEMENT_UV0;
    }
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
        modeldata.vertexMask |= VERTEX_ELEMENT_UV1;
    }
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
        modeldata.vertexMask |= VERTEX_ELEMENT_NORMAL;
    }
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
        modeldata.vertexMask |= VERTEX_ELEMENT_TANGENT;
    }
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_BINORMAL) != 0) {
        modeldata.vertexMask |= VERTEX_ELEMENT_BITANGENT;
    }


    for (int i = 0; i < numVertices; i++){
        VertexData vertexData;

        RawVertex rawvertex = raw.GetVertex(i);

        Vector3 vertex;
        vertex.x = rawvertex.position[0];
        vertex.y = rawvertex.position[1];
        vertex.z = rawvertex.position[2];
        vertexData.positions.push_back(vertex);

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
            Vector3 normal;
            normal.x = rawvertex.normal[0];
            normal.y = rawvertex.normal[1];
            normal.z = rawvertex.normal[2];
            vertexData.normals.push_back(normal);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
            Vector2 uv;
            uv.x = rawvertex.uv0[0];
            uv.y = 1.0 - rawvertex.uv0[1];
            vertexData.texcoords0.push_back(uv);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
            Vector2 uv;
            uv.x = rawvertex.uv1[0];
            uv.y = 1.0 - rawvertex.uv1[1];
            vertexData.texcoords1.push_back(uv);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
            Vector3 tangent;
            tangent.x = rawvertex.tangent[0];
            tangent.y = rawvertex.tangent[1];
            tangent.z = rawvertex.tangent[2];
            vertexData.tangents.push_back(tangent);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_BINORMAL) != 0) {
            Vector3 bitangent;
            bitangent.x = rawvertex.binormal[0];
            bitangent.y = rawvertex.binormal[1];
            bitangent.z = rawvertex.binormal[2];
            vertexData.bitangents.push_back(bitangent);
        }

        modeldata.vertices.push_back(vertexData);

        for (int j = 0; j < 4; j++){
            if (rawvertex.jointWeights[j] > 0){

                BoneVertexWeightData boneVertexWeightData;
                boneVertexWeightData.vertexId = i;
                boneVertexWeightData.weight = rawvertex.jointWeights[j];

                boneWeightDataMap[rawvertex.jointIndices[j]].push_back(boneVertexWeightData);
            }
        }

    }
}

void collectSubmeshes(SModelData &modeldata, RawModel &raw, RawNode &skeletonRoot, std::map<long, FbxAMatrix>& inverseBindMatricesMap, std::map<int, std::vector<BoneVertexWeightData>>& boneWeightDataMap){
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
    }

    for ( const auto &p : boneWeightDataMap ){
        BoneWeightData boneWeightData;
        boneWeightData.boneId = skeletonRawSurface.jointIds[p.first];
        boneWeightData.vertexWeights = p.second;

        modeldata.boneWeights.push_back(boneWeightData);
    }
}

void collectNodesHierarchy(SModelData &modeldata, RawModel &raw, RawNode &rawnode, std::map<long, FbxAMatrix>& inverseBindMatricesMap, BoneData& boneData){
    boneData.name = rawnode.name;
    boneData.boneId = rawnode.id;

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
        FbxAMatrix invBindMatrix = inverseBindMatricesMap[rawnode.id];
        for (int i = 0; i < 4; i++){
            for (int j = 0; j < 4; j++){
                boneData.offsetMatrix[i][j] = invBindMatrix.Get(i, j);
            }
        }
    }else{
        printf("Warning: node %s has a joint parent but is not a joint. Cannot get offsetMatrix (inverseBindMatrix) data.\n", rawnode.name.c_str());
    }

    for (int i = 0; i < rawnode.childIds.size(); i++){
        BoneData boneChild;
        collectNodesHierarchy(modeldata, raw, raw.GetNode(raw.GetNodeById(rawnode.childIds[i])), inverseBindMatricesMap, boneChild);
        boneData.children.push_back(boneChild);
    }
}

bool convertFbx2SModel(SModelData &modeldata, std::string path){

    RawModel raw;
    RawNode skeletonRoot;
    std::map<long, FbxAMatrix> inverseBindMatricesMap;
    std::map<int, std::vector<BoneVertexWeightData>> boneWeightDataMap;

    if (!LoadFBXFile(raw, path.c_str(), "png;jpg;jpeg")){
        return false;
    }

    //collectName(modeldata, raw); //Getting submesh name for model
    collectVertices(modeldata, raw, boneWeightDataMap);
    collectSubmeshes(modeldata, raw, skeletonRoot, inverseBindMatricesMap, boneWeightDataMap);
    if (skeletonRoot.isJoint){
        modeldata.skeleton = new BoneData;
        collectNodesHierarchy(modeldata, raw, skeletonRoot, inverseBindMatricesMap, *modeldata.skeleton);
    }

    return true;
}