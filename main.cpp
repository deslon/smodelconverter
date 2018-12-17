#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>

#include <iostream>
#include <fstream>

#include "Assimp2SModel.h"
#include "Fbx2SModel.h"
#include "SModelData.h"
#include "WriteSModel.h"
#include "ReadSModel.h"

void printModel(const SModelData &modelNode);
void printSkeleton(const BoneData &bone, std::map<unsigned int, std::string>& boneIdMap, int layerTree = 0);


SModelData modeldata;

int main (int argc, char *argv[]){
    if (argc < 2){
        fprintf(stdout,"Usage: %s <input file> [output file] [options]\n",argv[0]);
        return 1;
    }

    std::string path = argv[1];

    if (!convertFbx2SModel(modeldata, path)){
    //    return 1;
    //}

    //if (!convertAssimp2SModel(modeldata, path)){
        return 1;
    }

    std::ofstream os;
    WriteSModel writeSmodel(&os);

    os.open("test.smodel", std::ios::out | std::ios::binary);
    writeSmodel.writeModel(modeldata);
    os.close();

    printf("\n-----------Model summary---------------\n\n");
    //printModel(modeldata);
    printf("\n+++++++++\n");
/*

    std::ifstream is;
    ReadSModel readSmodel(&is);
    SModelData modeldataread;

    is.open("test.smodel", std::ios::in | std::ios::binary);
    readSmodel.readModel(modeldataread);

    printf("\n\n-----------Read test---------------\n\n");
    printModel(modeldataread);
    printf("\n+++++++++\n");
*/
  return 0;
}
void printModel(const SModelData &modelData){
    printf("Model name: %s, vertices: %i\n", modelData.name.c_str(), (int)modelData.vertices.size());

    for (int i = 0; i < modelData.meshes.size(); i++){
        printf(">Mesh (%s), indices: %i\n", 
            modelData.meshes[i].name.c_str(), 
            (int)modelData.meshes[i].indices.size());

        if (modelData.meshes[i].materials.size() > 0){
            printf("  >>Texture: %s\n", modelData.meshes[i].materials[0].texture.c_str());
        }
    }

    std::map<unsigned int, std::string> boneIndexMap;

    if (modelData.skeleton){
        printf("\n");
        printSkeleton(*modelData.skeleton, boneIndexMap);
    }
/*
    printf("\n");
    for (int b = 0; b < modelData.boneWeights.size(); b++){
        printf(">>Bone Weights (%u: %s): %i\n",
            modelData.boneWeights[b].boneId,
            boneIdMap[modelData.boneWeights[b].boneId].c_str(),
            (int)modelData.boneWeights[b].vertexWeights.size());
    }
    */
}

void printSkeleton(const BoneData &bone, std::map<unsigned int, std::string>& boneIndexMap, int layerTree){
    std::string strtree;
    for(int i = 0; i < layerTree; i++){
        strtree += "-";
    }
    if (bone.boneIndex > 0)
        boneIndexMap[bone.boneIndex] = bone.name;

    printf("%sBone name: %s\n", strtree.c_str(), bone.name.c_str());
    printf("%s  Position: %f %f %f\n", strtree.c_str(), bone.bindPosition.x, bone.bindPosition.y, bone.bindPosition.z);
    printf("%s  Rotation: %f %f %f %f\n", strtree.c_str(), bone.bindRotation.x, bone.bindRotation.y, bone.bindRotation.z, bone.bindRotation.w);
    printf("%s  Scale: %f %f %f\n", strtree.c_str(), bone.bindScale.x, bone.bindScale.y, bone.bindScale.z);
    printf("%s  Offset:\n", strtree.c_str());
    for (unsigned int i = 0; i < 4; i++)
        printf("%s    %f %f %f %f\n", strtree.c_str(), bone.offsetMatrix[0][i], bone.offsetMatrix[1][i], bone.offsetMatrix[2][i], bone.offsetMatrix[3][i]);

    for (int i = 0; i < bone.children.size(); i++){
        printSkeleton(bone.children[i], boneIndexMap, layerTree+1);
    }

}