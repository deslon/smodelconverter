#ifndef ReadSModel_h
#define ReadSModel_h

#include "SModelData.h"
#include <istream>
#include <vector>

class ReadSModel{

private:

    std::istream* is;

    void readString(std::string &str);
    void readUintVector(std::vector<unsigned int> &vec);
    void readVector3Vector(std::vector<Vector3> &vec);
    void readVector2Vector(std::vector<Vector2> &vec);
    void readMeshDataVector(std::vector<MeshData> &vec);
    void readMaterialDataVector(std::vector<MaterialData> &vec);
    void readBoneWeightDataVector(std::vector<BoneWeightData> &vec);
    void readBoneVertexWeightDataVector(std::vector<BoneVertexWeightData> &vec);
    void readSkeleton(BoneData* &skeleton);
    void readBoneData(BoneData &boneData);
    void readVerticesVector(std::vector<VertexData> &vec, int vertexMask);
    void readVertexData(VertexData &vertexData, int vertexMask);

public:

    ReadSModel(std::istream* is);

    bool readModel(SModelData &modelData);

};


#endif /* ReadSModel_h */