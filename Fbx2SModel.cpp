/**
 * Part of code based on https://github.com/facebookincubator/FBX2glTF/blob/master/src/fbx/Fbx2Raw.cpp
 * Copyright (c) 2014-present, Facebook, Inc.
 */


#include "Fbx2SModel.h"

#include <fbxsdk.h>

#include "util/FbxBlendShapesAccess.hpp"
#include "util/FbxLayerElementAccess.hpp"
#include "util/FbxMaterialsAccess.hpp"
#include "util/FbxSkinningAccess.hpp"

#include "util/File_Utils.hpp"
#include "util/String_Utils.hpp"

struct RawNode
{
    bool                     isJoint;
    long                     id;
    std::string              name;
    long                     parentId;
    std::vector<long>        childIds;
    FbxVector4               translation;
    FbxQuaternion            rotation;
    FbxVector4               scale;
    long                     surfaceId;
};

struct RawBlendChannel
{
    float defaultDeform;
    bool hasNormals;
    bool hasTangents;
    std::string name;
};

struct RawSurface
{
    long                         id;
    std::string                  name;                            // The name of this surface
    long                         skeletonRootId;                  // The id of the root node of the skeleton.
    //Bounds<float, 3>             bounds;
    std::vector<long>            jointIds;
    //std::vector<Vec3f>           jointGeometryMins;
    //std::vector<Vec3f>           jointGeometryMaxs;
    std::vector<FbxAMatrix>           inverseBindMatrices;
    std::vector<RawBlendChannel> blendChannels;
    bool                         discrete;
};


bool verboseOutput = true;
float scaleFactor = 1;
long rootNodeId;
std::vector<RawSurface> surfaces;
std::vector<RawNode> nodes;

int AddNode(const long id, const char *name, const long parentId)
{
    assert(name[0] != '\0');

    for (size_t i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id ) {
            return (int) i;
        }
    }

    RawNode joint;
    joint.isJoint     = false;
    joint.id          = id;
    joint.name        = name;
    joint.parentId    = parentId;
    joint.surfaceId   = 0;
    joint.translation = FbxVector4(0, 0, 0, 1);
    joint.rotation    = FbxQuaternion(0, 0, 0, 1);
    joint.scale       = FbxVector4(1, 1, 1, 1);

    nodes.emplace_back(joint);
    return (int) nodes.size() - 1;
}

int GetNodeById(const long nodeId)
{
    for (size_t i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == nodeId) {
            return (int) i;
        }
    }
    return -1;
}

int AddSurface(const char *name, const long surfaceId)
{
    assert(name[0] != '\0');

    for (size_t i = 0; i < surfaces.size(); i++) {
        if (surfaces[i].id == surfaceId) {
            return (int) i;
        }
    }
    RawSurface  surface;
    surface.id = surfaceId;
    surface.name     = name;
    //surface.bounds.Clear();
    surface.discrete  = false;

    surfaces.emplace_back(surface);
    return (int) (surfaces.size() - 1);
}

int GetSurfaceById(const long surfaceId)
{
    for (size_t i = 0; i < surfaces.size(); i++) {
        if (surfaces[i].id == surfaceId) {
            return (int)i;
        }
    }
    return -1;
}

static void ReadMesh(SModelData &modeldata, FbxScene *pScene, FbxNode *pNode, const std::map<const FbxTexture *, FbxString> &textureLocations){

    FbxGeometryConverter meshConverter(pScene->GetFbxManager());
    meshConverter.Triangulate(pNode->GetNodeAttribute(), true);
    FbxMesh *pMesh = pNode->GetMesh();

    // Obtains the surface Id
    const long surfaceId = pMesh->GetUniqueID();

    // Associate the node to this surface
    int nodeId = GetNodeById(pNode->GetUniqueID());
    if (nodeId >= 0) {
        RawNode &node = nodes[nodeId];
        node.surfaceId = surfaceId;
    }

    if (GetSurfaceById(surfaceId) >= 0) {
        // This surface is already loaded
        return;
    }

    const char *meshName = (pNode->GetName()[0] != '\0') ? pNode->GetName() : pMesh->GetName();
    const int rawSurfaceIndex = AddSurface(meshName, surfaceId);

    const FbxVector4 *controlPoints = pMesh->GetControlPoints();
    const FbxLayerElementAccess<FbxVector4> normalLayer(pMesh->GetElementNormal(), pMesh->GetElementNormalCount());
    const FbxLayerElementAccess<FbxVector4> binormalLayer(pMesh->GetElementBinormal(), pMesh->GetElementBinormalCount());
    const FbxLayerElementAccess<FbxVector4> tangentLayer(pMesh->GetElementTangent(), pMesh->GetElementTangentCount());
    const FbxLayerElementAccess<FbxColor>   colorLayer(pMesh->GetElementVertexColor(), pMesh->GetElementVertexColorCount());
    const FbxLayerElementAccess<FbxVector2> uvLayer0(pMesh->GetElementUV(0), pMesh->GetElementUVCount());
    const FbxLayerElementAccess<FbxVector2> uvLayer1(pMesh->GetElementUV(1), pMesh->GetElementUVCount());
    const FbxSkinningAccess                 skinning(pMesh, pScene, pNode);
    const FbxMaterialsAccess                materials(pMesh, textureLocations);
    const FbxBlendShapesAccess              blendShapes(pMesh);

    if (verboseOutput) {
        printf(
            "mesh %d: %s (skinned: %s)\n", rawSurfaceIndex, meshName,
            skinning.IsSkinned() ? nodes[GetNodeById(skinning.GetRootNode())].name.c_str() : "NO");
    }

    // The FbxNode geometric transformation describes how a FbxNodeAttribute is offset from
    // the FbxNode's local frame of reference. These geometric transforms are applied to the
    // FbxNodeAttribute after the FbxNode's local transforms are computed, and are not
    // inherited across the node hierarchy.
    // Apply the geometric transform to the mesh geometry (vertices, normal etc.).
    const FbxVector4 meshTranslation           = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 meshRotation              = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 meshScaling               = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
    const FbxAMatrix meshTransform(meshTranslation, meshRotation, meshScaling);
    const FbxMatrix  transform                 = meshTransform;

    // Remove translation & scaling from transforms that will bi applied to normals, tangents & binormals
    const FbxMatrix  normalTransform(FbxVector4(), meshRotation, meshScaling);
    const FbxMatrix  inverseTransposeTransform = normalTransform.Inverse().Transpose();
/*
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_POSITION);
    if (normalLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_NORMAL); }
    if (tangentLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_TANGENT); }
    if (binormalLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_BINORMAL); }
    if (colorLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_COLOR); }
    if (uvLayer0.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV0); }
    if (uvLayer1.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV1); }
    if (skinning.IsSkinned()) {
        raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS);
        raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_INDICES);
    }
*/
    RawSurface &rawSurface = surfaces[rawSurfaceIndex];

    FbxAMatrix scaleMatrix;
    scaleMatrix.SetS(FbxVector4(scaleFactor, scaleFactor, scaleFactor, 1.0));
    FbxAMatrix invScaleMatrix = scaleMatrix.Inverse();

    rawSurface.skeletonRootId = (skinning.IsSkinned()) ? skinning.GetRootNode() : pNode->GetUniqueID();
    for (int jointIndex = 0; jointIndex < skinning.GetNodeCount(); jointIndex++) {
        const long jointId = skinning.GetJointId(jointIndex);
        nodes[GetNodeById(jointId)].isJoint = true;

        rawSurface.jointIds.emplace_back(jointId);
        rawSurface.inverseBindMatrices.push_back(invScaleMatrix * skinning.GetInverseBindMatrix(jointIndex) * scaleMatrix);
        //rawSurface.jointGeometryMins.emplace_back(FLT_MAX, FLT_MAX, FLT_MAX);
        //rawSurface.jointGeometryMaxs.emplace_back(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }
/*
    rawSurface.blendChannels.clear();
    std::vector<const FbxBlendShapesAccess::TargetShape *> targetShapes;
    for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx ++) {
        for (size_t targetIx = 0; targetIx < blendShapes.GetTargetShapeCount(channelIx); targetIx ++) {
            const FbxBlendShapesAccess::TargetShape &shape = blendShapes.GetTargetShape(channelIx, targetIx);
            targetShapes.push_back(&shape);

            rawSurface.blendChannels.push_back(RawBlendChannel {
                static_cast<float>(blendShapes.GetBlendChannel(channelIx).deformPercent),
                shape.normals.LayerPresent(),
                shape.tangents.LayerPresent(),
            });
        }
    }
*/
    int polygonVertexIndex = 0;
    for (int polygonIndex = 0; polygonIndex < pMesh->GetPolygonCount(); polygonIndex++) {
        FBX_ASSERT(pMesh->GetPolygonSize(polygonIndex) == 3);
        /*
        const std::shared_ptr<FbxMaterialInfo> fbxMaterial = materials.GetMaterial(polygonIndex);

        int textures[RAW_TEXTURE_USAGE_MAX];
        std::fill_n(textures, (int) RAW_TEXTURE_USAGE_MAX, -1);

        std::shared_ptr<RawMatProps> rawMatProps;
        FbxString materialName;

        if (fbxMaterial == nullptr) {
            materialName = "DefaultMaterial";
            rawMatProps.reset(new RawTraditionalMatProps(RAW_SHADING_MODEL_LAMBERT,
                Vec3f(0, 0, 0), Vec4f(.5, .5, .5, 1), Vec3f(0, 0, 0), Vec3f(0, 0, 0), 0.5));

        } else {
            materialName = fbxMaterial->name;

            const auto maybeAddTexture = [&](const FbxFileTexture *tex, RawTextureUsage usage) {
                if (tex != nullptr) {
                    // dig out the inferred filename from the textureLocations map
                    FbxString inferredPath = textureLocations.find(tex)->second;
                    textures[usage] = raw.AddTexture(tex->GetName(), tex->GetFileName(), inferredPath.Buffer(), usage);
                }
            };

            std::shared_ptr<RawMatProps> matInfo;
            if (fbxMaterial->shadingModel == FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH) {
                FbxRoughMetMaterialInfo *fbxMatInfo = static_cast<FbxRoughMetMaterialInfo *>(fbxMaterial.get());

                maybeAddTexture(fbxMatInfo->texColor, RAW_TEXTURE_USAGE_ALBEDO);
                maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
                maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
                maybeAddTexture(fbxMatInfo->texRoughness, RAW_TEXTURE_USAGE_ROUGHNESS);
                maybeAddTexture(fbxMatInfo->texMetallic, RAW_TEXTURE_USAGE_METALLIC);
                maybeAddTexture(fbxMatInfo->texAmbientOcclusion, RAW_TEXTURE_USAGE_OCCLUSION);
                rawMatProps.reset(new RawMetRoughMatProps(
                    RAW_SHADING_MODEL_PBR_MET_ROUGH, toVec4f(fbxMatInfo->colBase), toVec3f(fbxMatInfo->colEmissive),
                    fbxMatInfo->emissiveIntensity, fbxMatInfo->metallic, fbxMatInfo->roughness));
            } else {

                FbxTraditionalMaterialInfo *fbxMatInfo = static_cast<FbxTraditionalMaterialInfo *>(fbxMaterial.get());
                RawShadingModel shadingModel;
                if (fbxMaterial->shadingModel == "Lambert") {
                    shadingModel = RAW_SHADING_MODEL_LAMBERT;
                } else if (fbxMaterial->shadingModel == "Blinn") {
                    shadingModel = RAW_SHADING_MODEL_BLINN;
                } else if (fbxMaterial->shadingModel == "Phong") {
                    shadingModel = RAW_SHADING_MODEL_PHONG;
                } else if (fbxMaterial->shadingModel == "Constant") {
                    shadingModel = RAW_SHADING_MODEL_PHONG;
                } else {
                    shadingModel = RAW_SHADING_MODEL_UNKNOWN;
                }
                maybeAddTexture(fbxMatInfo->texDiffuse, RAW_TEXTURE_USAGE_DIFFUSE);
                maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
                maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
                maybeAddTexture(fbxMatInfo->texShininess, RAW_TEXTURE_USAGE_SHININESS);
                maybeAddTexture(fbxMatInfo->texAmbient, RAW_TEXTURE_USAGE_AMBIENT);
                maybeAddTexture(fbxMatInfo->texSpecular, RAW_TEXTURE_USAGE_SPECULAR);
                rawMatProps.reset(new RawTraditionalMatProps(shadingModel,
                    toVec3f(fbxMatInfo->colAmbient), toVec4f(fbxMatInfo->colDiffuse), toVec3f(fbxMatInfo->colEmissive),
                    toVec3f(fbxMatInfo->colSpecular), fbxMatInfo->shininess));
            }
        }


        RawVertex rawVertices[3];
                */
        bool vertexTransparency = false;
        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++, polygonVertexIndex++) {
            const int controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, vertexIndex);

            // Note that the default values here must be the same as the RawVertex default values!
            const FbxVector4 fbxPosition = transform.MultNormalize(controlPoints[controlPointIndex]);
            const FbxVector4 fbxNormal   = normalLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxVector4 fbxTangent  = tangentLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxVector4 fbxBinormal = binormalLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxColor   fbxColor    = colorLayer
                .GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxColor(0.0f, 0.0f, 0.0f, 0.0f));
            const FbxVector2 fbxUV0      = uvLayer0.GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));
            const FbxVector2 fbxUV1      = uvLayer1.GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));
/*
            RawVertex &vertex = rawVertices[vertexIndex];
            vertex.position[0]   = (float) fbxPosition[0] * scaleFactor;
            vertex.position[1]   = (float) fbxPosition[1] * scaleFactor;
            vertex.position[2]   = (float) fbxPosition[2] * scaleFactor;
            vertex.normal[0]     = (float) fbxNormal[0];
            vertex.normal[1]     = (float) fbxNormal[1];
            vertex.normal[2]     = (float) fbxNormal[2];
            vertex.tangent[0]    = (float) fbxTangent[0];
            vertex.tangent[1]    = (float) fbxTangent[1];
            vertex.tangent[2]    = (float) fbxTangent[2];
            vertex.tangent[3]    = (float) fbxTangent[3];
            vertex.binormal[0]   = (float) fbxBinormal[0];
            vertex.binormal[1]   = (float) fbxBinormal[1];
            vertex.binormal[2]   = (float) fbxBinormal[2];
            vertex.color[0]      = (float) fbxColor.mRed;
            vertex.color[1]      = (float) fbxColor.mGreen;
            vertex.color[2]      = (float) fbxColor.mBlue;
            vertex.color[3]      = (float) fbxColor.mAlpha;
            vertex.uv0[0]        = (float) fbxUV0[0];
            vertex.uv0[1]        = (float) fbxUV0[1];
            vertex.uv1[0]        = (float) fbxUV1[0];
            vertex.uv1[1]        = (float) fbxUV1[1];
            vertex.jointIndices = skinning.GetVertexIndices(controlPointIndex);
            vertex.jointWeights = skinning.GetVertexWeights(controlPointIndex);
            vertex.polarityUv0  = false;

            // flag this triangle as transparent if any of its corner vertices substantially deviates from fully opaque
            vertexTransparency |= colorLayer.LayerPresent() && (fabs(fbxColor.mAlpha - 1.0) > 1e-3);

            rawSurface.bounds.AddPoint(vertex.position);

            if (!targetShapes.empty()) {
                vertex.blendSurfaceIx = rawSurfaceIndex;
                for (const auto *targetShape : targetShapes) {
                    RawBlendVertex blendVertex;
                    // the morph target data must be transformed just as with the vertex positions above
                    const FbxVector4 &shapePosition = transform.MultNormalize(targetShape->positions[controlPointIndex]);
                    blendVertex.position = toVec3f(shapePosition - fbxPosition) * scaleFactor;
                    if (targetShape->normals.LayerPresent()) {
                        const FbxVector4 &normal = targetShape->normals.GetElement(
                            polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
                        blendVertex.normal = toVec3f(normal - fbxNormal);
                    }
                    if (targetShape->tangents.LayerPresent()) {
                        const FbxVector4 &tangent = targetShape->tangents.GetElement(
                            polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
                        blendVertex.tangent = toVec4f(tangent - fbxTangent);
                    }
                    vertex.blends.push_back(blendVertex);
                }
            } else {
                vertex.blendSurfaceIx = -1;
            }

            if (skinning.IsSkinned()) {
                const int jointIndices[FbxSkinningAccess::MAX_WEIGHTS]   = {
                    vertex.jointIndices[0],
                    vertex.jointIndices[1],
                    vertex.jointIndices[2],
                    vertex.jointIndices[3]
                };
                const float jointWeights[FbxSkinningAccess::MAX_WEIGHTS] = {
                    vertex.jointWeights[0],
                    vertex.jointWeights[1],
                    vertex.jointWeights[2],
                    vertex.jointWeights[3]
                };
                const FbxMatrix skinningMatrix                           =
                    skinning.GetJointSkinningTransform(jointIndices[0]) * jointWeights[0] +
                    skinning.GetJointSkinningTransform(jointIndices[1]) * jointWeights[1] +
                    skinning.GetJointSkinningTransform(jointIndices[2]) * jointWeights[2] +
                    skinning.GetJointSkinningTransform(jointIndices[3]) * jointWeights[3];

                const FbxVector4 globalPosition = skinningMatrix.MultNormalize(fbxPosition);
                for (int i = 0; i < FbxSkinningAccess::MAX_WEIGHTS; i++) {
                    if (jointWeights[i] > 0.0f) {
                        const FbxVector4 localPosition =
                            skinning.GetJointInverseGlobalTransforms(jointIndices[i]).MultNormalize(globalPosition);

                        Vec3f &mins = rawSurface.jointGeometryMins[jointIndices[i]];
                        mins[0] = std::min(mins[0], (float) localPosition[0]);
                        mins[1] = std::min(mins[1], (float) localPosition[1]);
                        mins[2] = std::min(mins[2], (float) localPosition[2]);

                        Vec3f &maxs = rawSurface.jointGeometryMaxs[jointIndices[i]];
                        maxs[0] = std::max(maxs[0], (float) localPosition[0]);
                        maxs[1] = std::max(maxs[1], (float) localPosition[1]);
                        maxs[2] = std::max(maxs[2], (float) localPosition[2]);
                    }
                }
            }
 */           
        }
/*
        if (textures[RAW_TEXTURE_USAGE_NORMAL] != -1) {
            // Distinguish vertices that are used by triangles with a different texture polarity to avoid degenerate tangent space smoothing.
            const bool polarity = TriangleTexturePolarity(rawVertices[0].uv0, rawVertices[1].uv0, rawVertices[2].uv0);
            rawVertices[0].polarityUv0 = polarity;
            rawVertices[1].polarityUv0 = polarity;
            rawVertices[2].polarityUv0 = polarity;
        }

        int rawVertexIndices[3];
        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            rawVertexIndices[vertexIndex] = raw.AddVertex(rawVertices[vertexIndex]);
        }

        const RawMaterialType materialType = GetMaterialType(raw, textures, vertexTransparency, skinning.IsSkinned());
        const int rawMaterialIndex = raw.AddMaterial(materialName, materialType, textures, rawMatProps);

        raw.AddTriangle(rawVertexIndices[0], rawVertexIndices[1], rawVertexIndices[2], rawMaterialIndex, rawSurfaceIndex);
       */ 
    }
    
}

static std::string GetInferredFileName(const std::string &fbxFileName, const std::string &directory, const std::vector<std::string> &directoryFileList){

    if (FileUtils::FileExists(fbxFileName)) {
        return fbxFileName;
    }
    // Get the file name with file extension.
    const std::string fileName = StringUtils::GetFileNameString(StringUtils::GetCleanPathString(fbxFileName));

    // Try to find a match with extension.
    for (const auto &file : directoryFileList) {
        if (StringUtils::CompareNoCase(fileName, file) == 0) {
            return std::string(directory) + file;
        }
    }

    // Get the file name without file extension.
    const std::string fileBase = StringUtils::GetFileBaseString(fileName);

    // Try to find a match without file extension.
    for (const auto &file : directoryFileList) {
        // If the two extension-less base names match.
        if (StringUtils::CompareNoCase(fileBase, StringUtils::GetFileBaseString(file)) == 0) {
            // Return the name with extension of the file in the directory.
            return std::string(directory) + file;
        }
    }

    return "";
}

static void FindFbxTextures(FbxScene *pScene, const char *fbxFileName, const char *extensions, std::map<const FbxTexture *, FbxString> &textureLocations){

    // Get the folder the FBX file is in.
    const std::string folder = StringUtils::GetFolderString(fbxFileName);

    // Check if there is a filename.fbm folder to which embedded textures were extracted.
    const std::string fbmFolderName = folder + StringUtils::GetFileBaseString(fbxFileName) + ".fbm/";

    // Search either in the folder with embedded textures or in the same folder as the FBX file.
    const std::string searchFolder = FileUtils::FolderExists(fbmFolderName) ? fbmFolderName : folder;

    // Get a list with all the texture files from either the folder with embedded textures or the same folder as the FBX file.
    std::vector<std::string> fileList = FileUtils::ListFolderFiles(searchFolder.c_str(), extensions);

    // Try to match the FBX texture names with the actual files on disk.
    for (int i = 0; i < pScene->GetTextureCount(); i++) {
        const FbxFileTexture *pFileTexture = FbxCast<FbxFileTexture>(pScene->GetTexture(i));
        if (pFileTexture == nullptr) {
            continue;
        }
        const std::string inferredName = GetInferredFileName(pFileTexture->GetFileName(), searchFolder, fileList);
        if (inferredName.empty()) {
            printf("Warning: could not find a local image file for texture: %s.\n"
            "Original filename: %s\n", pFileTexture->GetName(), pFileTexture->GetFileName());
        }
        // always extend the mapping, even for files we didn't find
        textureLocations.emplace(pFileTexture, inferredName.c_str());
    }
}

static void ReadNodeAttributes(SModelData &modeldata, FbxScene *pScene, FbxNode *pNode, const std::map<const FbxTexture *, FbxString> &textureLocations)
{
    if (!pNode->GetVisibility()) {
        return;
    }

    FbxNodeAttribute *pNodeAttribute = pNode->GetNodeAttribute();
    if (pNodeAttribute != nullptr) {
        const FbxNodeAttribute::EType attributeType = pNodeAttribute->GetAttributeType();
        switch (attributeType) {
            case FbxNodeAttribute::eMesh:
            case FbxNodeAttribute::eNurbs:
            case FbxNodeAttribute::eNurbsSurface:
            case FbxNodeAttribute::eTrimNurbsSurface:
            case FbxNodeAttribute::ePatch: {
                ReadMesh(modeldata, pScene, pNode, textureLocations);
                break;
            }
            case FbxNodeAttribute::eCamera: {
                //ReadCamera(raw, pScene, pNode);
                break;
            }
            case FbxNodeAttribute::eUnknown:
            case FbxNodeAttribute::eNull:
            case FbxNodeAttribute::eMarker:
            case FbxNodeAttribute::eSkeleton:
            case FbxNodeAttribute::eCameraStereo:
            case FbxNodeAttribute::eCameraSwitcher:
            case FbxNodeAttribute::eLight:
            case FbxNodeAttribute::eOpticalReference:
            case FbxNodeAttribute::eOpticalMarker:
            case FbxNodeAttribute::eNurbsCurve:
            case FbxNodeAttribute::eBoundary:
            case FbxNodeAttribute::eShape:
            case FbxNodeAttribute::eLODGroup:
            case FbxNodeAttribute::eSubDiv:
            case FbxNodeAttribute::eCachedEffect:
            case FbxNodeAttribute::eLine: {
                break;
            }
        }
    }

    for (int child = 0; child < pNode->GetChildCount(); child++) {
        ReadNodeAttributes(modeldata, pScene, pNode->GetChild(child), textureLocations);
    }
}


/**
 * Compute the local scale vector to use for a given node. This is an imperfect hack to cope with
 * the FBX node transform's eInheritRrs inheritance type, in which ancestral scale is ignored
 */
static FbxVector4 computeLocalScale(FbxNode *pNode, FbxTime pTime = FBXSDK_TIME_INFINITE)
{
    const FbxVector4 lScale = pNode->EvaluateLocalTransform(pTime).GetS();

    if (pNode->GetParent() == nullptr ||
        pNode->GetTransform().GetInheritType() != FbxTransform::eInheritRrs) {
        return lScale;
    }
    // This is a very partial fix that is only correct for models that use identity scale in their rig's joints.
    // We could write better support that compares local scale to parent's global scale and apply the ratio to
    // our local translation. We'll always want to return scale 1, though -- that's the only way to encode the
    // missing 'S' (parent scale) in the transform chain.
    return FbxVector4(1, 1, 1, 1);
}


static void ReadNodeHierarchy(FbxScene *pScene, FbxNode *pNode, const long parentId, const std::string &path){

    const FbxUInt64 nodeId = pNode->GetUniqueID();
    const char *nodeName = pNode->GetName();
    const int  nodeIndex = AddNode(nodeId, nodeName, parentId);
    RawNode    &node     = nodes[nodeIndex];

    FbxTransform::EInheritType lInheritType;
    pNode->GetTransformationInheritType(lInheritType);

    std::string newPath = path + "/" + nodeName;
    if (verboseOutput) {
        printf("node: %s\n", newPath.c_str());
    }

    static int warnRrSsCount = 0;
    static int warnRrsCount  = 0;
    if (lInheritType == FbxTransform::eInheritRrSs && parentId) {
        if (++warnRrSsCount == 1) {
            printf("Warning: node %s uses unsupported transform inheritance type 'eInheritRrSs'.\n", newPath.c_str());
            printf("         (Further warnings of this type squelched.)\n");
        }

    } else if (lInheritType == FbxTransform::eInheritRrs) {
        if (++warnRrsCount == 1) {
            printf(
                "Warning: node %s uses unsupported transform inheritance type 'eInheritRrs'\n"
                    "     This tool will attempt to partially compensate.\n"
                    "     If this was a Maya export, consider turning off 'Segment Scale Compensate' on all joints.\n"
                    "     (Further warnings of this type squelched.)\n",
                newPath.c_str());
        }
    }

    // Set the initial node transform.
    const FbxAMatrix    localTransform   = pNode->EvaluateLocalTransform();
    const FbxVector4    localTranslation = localTransform.GetT();
    const FbxQuaternion localRotation    = localTransform.GetQ();
    const FbxVector4    localScaling     = computeLocalScale(pNode);

    node.translation = localTranslation * scaleFactor;
    node.rotation    = localRotation;
    node.scale       = localScaling;

    if (parentId) {
        RawNode &parentNode = nodes[GetNodeById(parentId)];
        // Add unique child name to the parent node.
        if (std::find(parentNode.childIds.begin(), parentNode.childIds.end(), nodeId) == parentNode.childIds.end()) {
            parentNode.childIds.push_back(nodeId);
        }
    } else {
        // If there is no parent then this is the root node.
        rootNodeId = nodeId;
    }

    for (int child = 0; child < pNode->GetChildCount(); child++) {
        ReadNodeHierarchy(pScene, pNode->GetChild(child), nodeId, newPath);
    }
}


bool convertFbx2SModel(SModelData &modeldata, std::string path){


    FbxManager    *pManager    = FbxManager::Create();
    FbxIOSettings *pIoSettings = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(pIoSettings);

    FbxImporter *pImporter = FbxImporter::Create(pManager, "");

    if (!pImporter->Initialize(path.c_str(), -1, pManager->GetIOSettings())) {
        printf("Error returned: %s\n", pImporter->GetStatus().GetErrorString());
        pImporter->Destroy();
        pManager->Destroy();
        return false;
    }

    FbxScene *pScene = FbxScene::Create(pManager, "fbxScene");
    pImporter->Import(pScene);
    pImporter->Destroy();

    if (pScene == nullptr) {
        pImporter->Destroy();
        pManager->Destroy();
        return false;
    }

    std::map<const FbxTexture *, FbxString> textureLocations;
    FindFbxTextures(pScene, path.c_str(), "png;jpg;jpeg", textureLocations);

    // Use Y up for glTF
    FbxAxisSystem::MayaYUp.ConvertScene(pScene);

    // FBX's internal unscaled unit is centimetres, and if you choose not to work in that unit,
    // you will find scaling transfgrms on all the children of the root node. Those transforms are
    // superfluous and cause a lot of people a lot of trouble. Luckily we can get rid of them by
    // converting to CM here (which just gets rid of the scaling), and then we pre-multiply the
    // scale factor into every vertex position (and related attributes) instead.
    FbxSystemUnit sceneSystemUnit = pScene->GetGlobalSettings().GetSystemUnit();
    if (sceneSystemUnit != FbxSystemUnit::cm) {
        FbxSystemUnit::cm.ConvertScene(pScene);
    }
    // this is always 0.01, but let's opt for clarity.
    scaleFactor = FbxSystemUnit::m.GetConversionFactorFrom(FbxSystemUnit::cm);

    ReadNodeHierarchy(pScene, pScene->GetRootNode(), 0, "");
    ReadNodeAttributes(modeldata, pScene, pScene->GetRootNode(), textureLocations);
    //ReadAnimations(raw, pScene);

    pScene->Destroy();
    pManager->Destroy();

    return true;

}