#ifndef SANIMDATA_H
#define SANIMDATA_H

#include <vector>
#include <string>

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

struct ChannelData
{
    std::string nodeName;
    std::vector<Vector4> translations;
    std::vector<Quaternion> rotations;
    std::vector<Vector4> scales;
    std::vector<float> weights;
};

struct SAnimData {
    std::string name;
    std::vector<float>      times;
    std::vector<ChannelData> channels;
};

#endif /* SANIMADATA_H */