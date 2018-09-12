#ifndef SMODELDATA_H
#define SMODELDATA_H

#include <vector>
#include <string>

struct Quaternion {
    float w;
    float x;
    float y;
    float z;
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

struct BoneVertexWeightData{
    unsigned int vertexId;
    float weight;
};

struct BoneWeightData {
    std::string name;
    std::vector<BoneVertexWeightData> vertexWeights;
};

struct MeshData {
    std::string name;
    std::vector<unsigned int> indices;
    std::vector<MaterialData> materials;
};

struct BoneData {
    std::string name;
    Vector3 bindPosition;
    Quaternion bindRotation;
    Vector3 bindScale;
    float offsetMatrix[4][4];
    std::vector<BoneData> children;
};

struct SModelData {
    std::string name;
    std::vector<Vector3> vertices;
    std::vector<std::vector<Vector2>> texcoords;
    std::vector<Vector3> normals;
    std::vector<Vector3> tangents;
    std::vector<Vector3> bitangents;
    std::vector<MeshData> meshes;
    std::vector<BoneWeightData> boneWeights;
    BoneData* skeleton;
};

#endif /* SMODELDATA_H */