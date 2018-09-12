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
    void writeVector3Vector(const std::vector<Vector3> &vec);
    void writeVector2Vector(const std::vector<Vector2> &vec);
    void writeVector2VectorVector(const std::vector<std::vector<Vector2>> &vec);
    void writeMeshDataVector(const std::vector<MeshData> &vec);
    void writeMaterialDataVector(const std::vector<MaterialData> &vec);
    void writeBoneWeightDataVector(const std::vector<BoneWeightData> &vec);
    void writeBoneVertexWeightDataVector(const std::vector<BoneVertexWeightData> &vec);
    void writeSkeleton(const BoneData* skeleton);
    void writeBoneData(const BoneData &boneData);

public:

    WriteSModel(std::ostream* os);

    void writeModel(const SModelData &modelData);

};


#endif /* WriteSModel_h */