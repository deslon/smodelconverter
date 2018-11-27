#ifndef WriteSModel_h
#define WriteSModel_h

#include "SModelData.h"
#include <ostream>
#include <vector>

class WriteSModel{

private:

    std::ostream* os;

    void writeString(const std::string &str);
    void writeUintVector(const std::vector<unsigned int> &vec);
    void writeVector4(const Vector4 &vec);
    void writeVector3(const Vector3 &vec);
    void writeVector2(const Vector2 &vec);
    void writeMeshDataVector(const std::vector<MeshData> &vec);
    void writeMaterialDataVector(const std::vector<MaterialData> &vec);
    void writeSkeleton(const BoneData* skeleton);
    void writeBoneData(const BoneData &boneData);
    void writeVerticesVector(const std::vector<VertexData> &vec, int vertexMask);
    void writeVertexData(const VertexData &vertexData, int vertexMask);

public:

    WriteSModel(std::ostream* os);

    void writeModel(const SModelData &modelData);

};


#endif /* WriteSModel_h */