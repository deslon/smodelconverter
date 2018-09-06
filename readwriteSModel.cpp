#include "readwriteSModel.h"

//---------------------- WRITE FUNCTIONS ------------------------------

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

    if (size == 1){
        writeBoneData(os, *skeleton);
    }
}

void writeBoneData(std::ostream& os, const BoneData &boneData){
    writeString(os, boneData.name);
    os.write((char*)&boneData.bindPosition, 3 * sizeof(float));
    os.write((char*)&boneData.bindRotation, 4 * sizeof(float));
    os.write((char*)&boneData.bindScale, 3 * sizeof(float));
    os.write((char*)&boneData.offsetMatrix, 16 * sizeof(float));

    size_t size = boneData.children.size();
    os.write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeBoneData(os, boneData.children[i]);
    }
}

//---------------------- READ FUNCTIONS ------------------------------

void readModel(std::istream& is, ModelData &modelData){
    char* sig= new char[6];
    int version;

    is.read(sig, sizeof(char) * 6);
    is.read((char*)&version, sizeof(int));

    if (std::string(sig)=="SMODEL" && version==1){
        readString(is, modelData.name);
        readVector3Vector(is, modelData.vertices);
        readVector2VectorVector(is, modelData.texcoords);
        readVector3Vector(is, modelData.normals);
        readVector3Vector(is, modelData.tangents);
        readVector3Vector(is, modelData.bitangents);
        readMeshDataVector(is, modelData.meshes);
        readBoneWeightDataVector(is, modelData.bonesWeights);
        readSkeleton(is, modelData.skeleton);
    }
}

void readString(std::istream& is, std::string &str){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    str.resize(size);
    is.read((char*)&str[0], size);
}

void readUintVector(std::istream& is, std::vector<unsigned int> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void readVector3Vector(std::istream& is, std::vector<Vector3> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * 3 * sizeof(float));
}

void readVector2Vector(std::istream& is, std::vector<Vector2> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);
    is.read((char*)&vec[0], vec.size() * 2 * sizeof(float));
}

void readVector2VectorVector(std::istream& is, std::vector<std::vector<Vector2>> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readVector2Vector(is, vec[i]);
    }
}

void readMeshDataVector(std::istream& is, std::vector<MeshData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readString(is, vec[i].name);
        readUintVector(is, vec[i].indices);
        readMaterialDataVector(is, vec[i].materials);
    }
}

void readMaterialDataVector(std::istream& is, std::vector<MaterialData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        is.read((char*)&vec[i].type, sizeof(int));
        readString(is, vec[i].texture);
    }
}

void readBoneWeightDataVector(std::istream& is, std::vector<BoneWeightData> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        readString(is, vec[i].name);
        readBoneVertexWeightVector(is, vec[i].vertexWeights);
    }
}

void readBoneVertexWeightVector(std::istream& is, std::vector<BoneVertexWeight> &vec){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    vec.resize(size);

    for (size_t i = 0; i < size; ++i){
        is.read((char*)&vec[i].vertexId, sizeof(unsigned int));
        is.read((char*)&vec[i].weight, sizeof(float));
    }
}

void readSkeleton(std::istream& is, BoneData* &skeleton){
    size_t size = 0;
    is.read((char*)&size, sizeof(size));

    skeleton = NULL;

    if (size == 1){
        skeleton = new BoneData();
        readBoneData(is, *skeleton);
    }
}

void readBoneData(std::istream& is, BoneData &boneData){
    readString(is, boneData.name);
    is.read((char*)&boneData.bindPosition, 3 * sizeof(float));
    is.read((char*)&boneData.bindRotation, 4 * sizeof(float));
    is.read((char*)&boneData.bindScale, 3 * sizeof(float));
    is.read((char*)&boneData.offsetMatrix, 16 * sizeof(float));

    size_t size = 0;
    is.read((char*)&size, sizeof(size));
    boneData.children.resize(size);

    for (size_t i = 0; i < size; ++i){
        readBoneData(is, boneData.children[i]);
    }
}