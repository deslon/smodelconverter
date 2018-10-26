#include "WriteSModel.h"

WriteSModel::WriteSModel(std::ostream* os){
    this->os = os;
}

void WriteSModel::writeModel(const SModelData &modelData){
    int version = 1;
    
    os->write("SMODEL", sizeof(char) * 6);
    os->write((char*)&version, sizeof(int));

    writeString(modelData.name);
    os->write((char*)&modelData.vertexMask, sizeof(int));
    writeVerticesVector(modelData.vertices, modelData.vertexMask);
    writeMeshDataVector(modelData.meshes);
    writeBoneWeightDataVector(modelData.boneWeights);
    writeSkeleton(modelData.skeleton);
}

void WriteSModel::writeString(const std::string &str){
    size_t size = str.size();
    os->write((char*)&size, sizeof(size));
    os->write((char*)&str[0], size);
}

void WriteSModel::writeUintVector(const std::vector<unsigned int> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));
    os->write((char*)&vec[0], vec.size() * sizeof(unsigned int));
}

void WriteSModel::writeVector3Vector(const std::vector<Vector3> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));
    os->write((char*)&vec[0], vec.size() * 3 * sizeof(float));
}

void WriteSModel::writeVector2Vector(const std::vector<Vector2> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));
    os->write((char*)&vec[0], vec.size() * 2 * sizeof(float));
}

void WriteSModel::writeMeshDataVector(const std::vector<MeshData> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeString(vec[i].name);
        writeUintVector(vec[i].indices);
        writeMaterialDataVector(vec[i].materials);
    }
}

void WriteSModel::writeMaterialDataVector(const std::vector<MaterialData> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os->write((char*)&vec[i].type, sizeof(int));
        writeString(vec[i].texture);
    }
}

void WriteSModel::writeBoneWeightDataVector(const std::vector<BoneWeightData> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os->write((char*)&vec[i].boneId, sizeof(unsigned int));
        writeBoneVertexWeightDataVector(vec[i].vertexWeights);
    }
}

void WriteSModel::writeBoneVertexWeightDataVector(const std::vector<BoneVertexWeightData> &vec){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        os->write((char*)&vec[i].vertexId, sizeof(unsigned int));
        os->write((char*)&vec[i].weight, sizeof(float));
    }
}

void WriteSModel::writeSkeleton(const BoneData* skeleton){
    size_t size = 0;

    if (skeleton)
        size = 1;

    os->write((char*)&size, sizeof(size));

    if (size == 1){
        writeBoneData(*skeleton);
    }
}

void WriteSModel::writeBoneData(const BoneData &boneData){
    writeString(boneData.name);
    os->write((char*)&boneData.boneId, sizeof(unsigned int));
    os->write((char*)&boneData.bindPosition, 3 * sizeof(float));
    os->write((char*)&boneData.bindRotation, 4 * sizeof(float));
    os->write((char*)&boneData.bindScale, 3 * sizeof(float));
    os->write((char*)&boneData.offsetMatrix, 16 * sizeof(float));

    size_t size = boneData.children.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeBoneData(boneData.children[i]);
    }
}

void WriteSModel::writeVerticesVector(const std::vector<VertexData> &vec, int vertexMask){
    size_t size = vec.size();
    os->write((char*)&size, sizeof(size));

    for (size_t i = 0; i < size; ++i){
        writeVertexData(vec[i], vertexMask);
    }
}

void WriteSModel::writeVertexData(const VertexData &vertexData, int vertexMask){
    if (vertexMask & VERTEX_ELEMENT_POSITION)
        writeVector3Vector(vertexData.positions);
    if (vertexMask & VERTEX_ELEMENT_UV0)
        writeVector2Vector(vertexData.texcoords0);
    if (vertexMask & VERTEX_ELEMENT_UV1)
        writeVector2Vector(vertexData.texcoords1);
    if (vertexMask & VERTEX_ELEMENT_NORMAL)
        writeVector3Vector(vertexData.normals);
    if (vertexMask & VERTEX_ELEMENT_TANGENT)
        writeVector3Vector(vertexData.tangents);
    if (vertexMask & VERTEX_ELEMENT_BITANGENT)
        writeVector3Vector(vertexData.bitangents);
}