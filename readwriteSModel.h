#ifndef readwriteSModel_h
#define readwriteSModel_h

#include "modeldata.h"
#include <ostream>
#include <istream>
#include <vector>

void writeModel(std::ostream& os, const ModelData &modelData);
void writeString(std::ostream& os, const std::string &str);
void writeVector3Vector(std::ostream& os, const std::vector<Vector3> &vec);
void writeVector2Vector(std::ostream& os, const std::vector<Vector2> &vec);
void writeVector2VectorVector(std::ostream& os, const std::vector<std::vector<Vector2>> &vec);
void writeMeshDataVector(std::ostream& os, const std::vector<MeshData> &vec);
void writeMaterialDataVector(std::ostream& os, const std::vector<MaterialData> &vec);
void writeBoneWeightDataVector(std::ostream& os, const std::vector<BoneWeightData> &vec);
void writeBoneVertexWeightVector(std::ostream& os, const std::vector<BoneVertexWeight> &vec);
void writeSkeleton(std::ostream& os, const BoneData* skeleton);
void writeBoneData(std::ostream& os, const BoneData boneData);


#endif /* readwriteSModel_h */