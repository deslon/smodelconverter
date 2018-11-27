#ifndef SMODELDATA_H
#define SMODELDATA_H

#include <vector>
#include <string>

enum VertexElement
{
    VERTEX_ELEMENT_POSITION      = 1 << 0,
    VERTEX_ELEMENT_NORMAL        = 1 << 1,
    VERTEX_ELEMENT_TANGENT       = 1 << 2,
    VERTEX_ELEMENT_BITANGENT     = 1 << 3,
    VERTEX_ELEMENT_COLOR         = 1 << 4,
    VERTEX_ELEMENT_UV0           = 1 << 5,
    VERTEX_ELEMENT_UV1           = 1 << 6,
    VERTEX_ELEMENT_BONE_INDICES  = 1 << 7,
    VERTEX_ELEMENT_BONE_WEIGHTS  = 1 << 8
};

struct Quaternion {
    float w;
    float x;
    float y;
    float z;
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vector2 {
    float x;
    float y;
};

struct MaterialData {
    int type;
    std::string texture;
};

struct MeshData {
    std::string name;
    std::vector<unsigned int> indices;
    std::vector<MaterialData> materials;
};

struct BoneData {
    std::string name;
    int boneIndex;
    Vector3 bindPosition;
    Quaternion bindRotation;
    Vector3 bindScale;
    float offsetMatrix[4][4];
    std::vector<BoneData> children;
};

struct VertexData {
    Vector3 position;
    Vector2 texcoord0;
    Vector2 texcoord1;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    Vector4 boneIndices;
    Vector4 boneWeights;
};

struct SModelData {
    std::string name;
    int vertexMask;
    std::vector<VertexData> vertices;
    std::vector<MeshData> meshes;
    BoneData* skeleton;
};

#endif /* SMODELDATA_H */