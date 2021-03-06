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

void WriteSModel::writeVector4(const Vector4 &vec){
    os->write((char*)&vec, 4 * sizeof(float));
}

void WriteSModel::writeVector3(const Vector3 &vec){
    os->write((char*)&vec, 3 * sizeof(float));
}

void WriteSModel::writeVector2(const Vector2 &vec){
    os->write((char*)&vec, 2 * sizeof(float));
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
    os->write((char*)&boneData.boneIndex, sizeof(int));
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
        writeVector3(vertexData.position);
    if (vertexMask & VERTEX_ELEMENT_UV0)
        writeVector2(vertexData.texcoord0);
    if (vertexMask & VERTEX_ELEMENT_UV1)
        writeVector2(vertexData.texcoord1);
    if (vertexMask & VERTEX_ELEMENT_NORMAL)
        writeVector3(vertexData.normal);
    if (vertexMask & VERTEX_ELEMENT_TANGENT)
        writeVector3(vertexData.tangent);
    if (vertexMask & VERTEX_ELEMENT_BITANGENT)
        writeVector3(vertexData.bitangent);
    if (vertexMask & VERTEX_ELEMENT_BONE_INDICES)
        writeVector4(vertexData.boneIndices);
    if (vertexMask & VERTEX_ELEMENT_BONE_WEIGHTS)
        writeVector4(vertexData.boneWeights);
}