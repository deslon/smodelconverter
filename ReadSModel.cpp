#include "ReadSModel.h"

ReadSModel::ReadSModel(std::istream* is){
    this->is = is;
}

bool ReadSModel::readModel(SModelData &modelData){
    char* sig= new char[6];
    int version;

    is->read(sig, sizeof(char) * 6);
    is->read((char*)&version, sizeof(int));

    if (std::string(sig)=="SMODEL" && version==1){
        readString(modelData.name);
        readVector3Vector(modelData.vertices);
        readVector2VectorVector(modelData.texcoords);
        readVector3Vector(modelData.normals);
        readVector3Vector(modelData.tangents);
        readVector3Vector(modelData.bitangents);
        readMeshDataVector(modelData.meshes);
        readBoneWeightDataVector(modelData.boneWeights);
        readSkeleton(modelData.skeleton);

        return true;
    }

    return false;
}

void ReadSModel::readString(std::string &str){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    str.resize(size);
    is->read((char*)&str[0], size);
}

void ReadSModel::readUintVector(std::vector<unsigned int> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);
    is->read((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void ReadSModel::readVector3Vector(std::vector<Vector3> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);
    is->read((char*)&vec[0], vec.size() * 3 * sizeof(float));
}

void ReadSModel::readVector2Vector(std::vector<Vector2> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);
    is->read((char*)&vec[0], vec.size() * 2 * sizeof(float));
}

void ReadSModel::readVector2VectorVector(std::vector<std::vector<Vector2>> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readVector2Vector(vec[i]);
    }
}

void ReadSModel::readMeshDataVector(std::vector<MeshData> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readString(vec[i].name);
        readUintVector(vec[i].indices);
        readMaterialDataVector(vec[i].materials);
    }
}

void ReadSModel::readMaterialDataVector(std::vector<MaterialData> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        is->read((char*)&vec[i].type, sizeof(int));
        readString(vec[i].texture);
    }
}

void ReadSModel::readBoneWeightDataVector(std::vector<BoneWeightData> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readString(vec[i].name);
        readBoneVertexWeightDataVector(vec[i].vertexWeights);
    }
}

void ReadSModel::readBoneVertexWeightDataVector(std::vector<BoneVertexWeightData> &vec){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        is->read((char*)&vec[i].vertexId, sizeof(unsigned int));
        is->read((char*)&vec[i].weight, sizeof(float));
    }
}

void ReadSModel::readSkeleton(BoneData* &skeleton){
    size_t size = 0;
    is->read((char*)&size, sizeof(size));

    skeleton = NULL;

    if (size == 1){
        skeleton = new BoneData();
        readBoneData(*skeleton);
    }
}

void ReadSModel::readBoneData(BoneData &boneData){
    readString(boneData.name);
    is->read((char*)&boneData.bindPosition, 3 * sizeof(float));
    is->read((char*)&boneData.bindRotation, 4 * sizeof(float));
    is->read((char*)&boneData.bindScale, 3 * sizeof(float));
    is->read((char*)&boneData.offsetMatrix, 16 * sizeof(float));

    size_t size = 0;
    is->read((char*)&size, sizeof(size));
    boneData.children.resize(size);

    for (size_t i = 0; i < size; ++i){
        readBoneData(boneData.children[i]);
    }
}