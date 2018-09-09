#ifndef readwriteSModel_h
#define readwriteSModel_h

#include "modeldata.h"
#include <ostream>
#include <istream>
#include <vector>

void writeModel(std::ostream& os, const ModelData &modelData);
void writeString(std::ostream& os, const std::string &str);
void writeUintVector(std::ostream& os, const std::vector<unsigned int> &vec);
void writeVector3Vector(std::ostream& os, const std::vector<Vector3> &vec);
void writeVector2Vector(std::ostream& os, const std::vector<Vector2> &vec);
void writeVector2VectorVector(std::ostream& os, const std::vector<std::vector<Vector2>> &vec);
void writeMeshDataVector(std::ostream& os, const std::vector<MeshData> &vec);
void writeMaterialDataVector(std::ostream& os, const std::vector<MaterialData> &vec);
void writeBoneWeightDataVector(std::ostream& os, const std::vector<BoneWeightData> &vec);
void writeBoneVertexWeightDataVector(std::ostream& os, const std::vector<BoneVertexWeightData> &vec);
void writeSkeleton(std::ostream& os, const BoneData* skeleton);
void writeBoneData(std::ostream& os, const BoneData &boneData);

bool readModel(std::istream& is, ModelData &modelData);
void readString(std::istream& is, std::string &str);
void readUintVector(std::istream& is, std::vector<unsigned int> &vec);
void readVector3Vector(std::istream& is, std::vector<Vector3> &vec);
void readVector2Vector(std::istream& is, std::vector<Vector2> &vec);
void readVector2VectorVector(std::istream& is, std::vector<std::vector<Vector2>> &vec);
void readMeshDataVector(std::istream& is, std::vector<MeshData> &vec);
void readMaterialDataVector(std::istream& is, std::vector<MaterialData> &vec);
void readBoneWeightDataVector(std::istream& is, std::vector<BoneWeightData> &vec);
void readBoneVertexWeightDataVector(std::istream& is, std::vector<BoneVertexWeightData> &vec);
void readSkeleton(std::istream& is, BoneData* &skeleton);
void readBoneData(std::istream& is, BoneData &boneData);


#endif /* readwriteSModel_h */