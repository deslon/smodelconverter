#include "Fbx2SModel.h"

#include "util/RawModel.hpp"
#include "util/Fbx2Raw.hpp"
#include "util/String_Utils.hpp"

#include <map>

void collectName(SModelData &modeldata, RawModel &raw){
    if (raw.GetRootNode() < raw.GetNodeCount()){
        modeldata.name = raw.GetNode(raw.GetRootNode()).name;
    }else{
        modeldata.name = raw.GetNode(raw.GetNodeCount() - 1).name;
    }
}

void collectVertices(SModelData &modeldata, RawModel &raw){
    int numVertices = raw.GetVertexCount();
    int vertexAttributes = raw.GetVertexAttributes();

    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
        std::vector<Vector2> uv0vec;
        modeldata.texcoords.push_back(uv0vec);
    }

    for (int i = 0; i < numVertices; i++){
        RawVertex rawvertex = raw.GetVertex(i);

        Vector3 vertex;
        vertex.x = rawvertex.position[0];
        vertex.y = rawvertex.position[1];
        vertex.z = rawvertex.position[2];
        modeldata.vertices.push_back(vertex);

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
            Vector3 normal;
            normal.x = rawvertex.normal[0];
            normal.y = rawvertex.normal[1];
            normal.z = rawvertex.normal[2];
            modeldata.normals.push_back(normal);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
            Vector2 uv;
            uv.x = rawvertex.uv0[0];
            uv.y = 1.0 - rawvertex.uv0[1];
            modeldata.texcoords[0].push_back(uv);
        }

        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
            Vector3 tangent;
            tangent.x = rawvertex.tangent[0];
            tangent.y = rawvertex.tangent[1];
            tangent.z = rawvertex.tangent[2];
            modeldata.tangents.push_back(tangent);
        }
    }
}

void collectSubmeshes(SModelData &modeldata, RawModel &raw){
    int trianglesCount = raw.GetTriangleCount();

    std::map<int, int> submeshmap;

    for (int i = 0; i < trianglesCount; i++){
        RawTriangle rawtriangle = raw.GetTriangle(i);
        
        if (submeshmap.count(rawtriangle.materialIndex) == 0){
            MeshData meshdata;
            modeldata.meshes.push_back(meshdata);
            submeshmap[rawtriangle.materialIndex] = modeldata.meshes.size()-1;
        }

        MeshData* meshp = &modeldata.meshes[submeshmap[rawtriangle.materialIndex]];

        RawMaterial rawmaterial = raw.GetMaterial(rawtriangle.materialIndex);

        meshp->name = raw.GetSurface(rawtriangle.surfaceIndex).name;
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

                    meshp->materials.push_back(texturedata);
                }
            
            }
        }
        
    }
}

bool convertFbx2SModel(SModelData &modeldata, std::string path){

    RawModel raw;

    if (!LoadFBXFile(raw, path.c_str(), "png;jpg;jpeg")){
        return false;
    }

    collectName(modeldata, raw);
    collectVertices(modeldata, raw);
    collectSubmeshes(modeldata, raw);

    return true;
}