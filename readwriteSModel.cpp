#include "readwriteSModel.h"

void writeModel(std::ostream& os, const ModelData &modelData){
    int version = 1;
    
    os.write("SMODEL", sizeof(char) * 6);
    os.write((char*)&version, sizeof(int));

    writeString(os, modelData.name);
    writeVector3Vector(os, modelData.vertices);
    writeVector2VectorVector(os, modelData.texcoords);
    writeVector3Vector(os, modelData.normals);
    writeVector3Vector(os, modelData.tangents);
    writeVector3Vector(os, modelData.bitangents);
    writeMeshDataVector(os, modelData.meshes);
    writeBoneWeightDataVector(os, modelData.bonesWeights);
    writeSkeleton(os, modelData.skeleton);
}

void writeString(std::ostream& os, const std::string &str){
    size_t size = str.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&str[0], size);
}

void writeUintVector(std::ostream& os, const std::vector<unsigned int> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void writeVector3Vector(std::ostream& os, const std::vector<Vector3> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * 3 * sizeof(float));
}

void writeVector2Vector(std::ostream& os, const std::vector<Vector2> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));
    os.write((char*)&vec[0], vec.size() * 2 * sizeof(float));
}

void writeVector2VectorVector(std::ostream& os, const std::vector<std::vector<Vector2>> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeVector2Vector(os, vec[i]);
    }
}

void writeMeshDataVector(std::ostream& os, const std::vector<MeshData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeString(os, vec[i].name);
        writeUintVector(os, vec[i].indices);
        writeMaterialDataVector(os, vec[i].materials);
    }
}

void writeMaterialDataVector(std::ostream& os, const std::vector<MaterialData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os.write((char*)&vec[i].type, sizeof(int));
        writeString(os, vec[i].texture);
    }
}

void writeBoneWeightDataVector(std::ostream& os, const std::vector<BoneWeightData> &vec){
    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeString(os, vec[i].name);
        writeBoneVertexWeightVector(os, vec[i].vertexWeights);
    }
}

void writeBoneVertexWeightVector(std::ostream& os, const std::vector<BoneVertexWeight> &vec){

    size_t size = vec.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os.write((char*)&vec[i].vertexId, sizeof(unsigned int));
        os.write((char*)&vec[i].weight, sizeof(float));
    }
}

void writeSkeleton(std::ostream& os, const BoneData* skeleton){
    size_t size = 0;

    if (skeleton)
        size = 1;

    os.write((char*)&size, sizeof(size));

    if (skeleton)
        writeBoneData(os, *skeleton);
}

void writeBoneData(std::ostream& os, const BoneData boneData){
    writeString(os, boneData.name);
    os.write((char*)&boneData.bindPosition, 3 * sizeof(float));
    os.write((char*)&boneData.bindRotation, 4 * sizeof(float));
    os.write((char*)&boneData.bindScale, 2 * sizeof(float));
    os.write((char*)&boneData.offsetMatrix, 16 * sizeof(float));

    size_t size = boneData.children.size();
    os.write((char*)&size, sizeof(size));
    for (size_t i = 0; i < size; ++i){
        writeBoneData(os, boneData.children[i]);
    }
}