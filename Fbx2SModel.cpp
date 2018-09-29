#include "Fbx2SModel.h"

#include "util/RawModel.hpp"
#include "util/Fbx2Raw.hpp"

bool convertFbx2SModel(SModelData &modeldata, std::string path){

    RawModel raw;

    bool loadreturn = LoadFBXFile(raw, path.c_str(), "png;jpg;jpeg");

    return loadreturn;
}