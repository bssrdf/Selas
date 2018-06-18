#pragma once

//==============================================================================
// Joe Schutte
//==============================================================================

#include "SceneLib/SceneResource.h"
#include "ContainersLib/CArray.h"
#include "StringLib/FixedString.h"
#include "GeometryLib/AxisAlignedBox.h"
#include "MathLib/FloatStructs.h"
#include "SystemLib/BasicTypes.h"

namespace Selas
{
    #pragma warning(default : 4820)

    //==============================================================================
    struct ImportedMaterialData
    {
        FixedString256 shaderName;

        FilePathString albedoTextureName;
        FilePathString heightTextureName;
        FilePathString normalTextureName;
        FilePathString roughnessTextureName;
        FilePathString specularTextureName;
        FilePathString metalnessTextureName;

        float roughness;
        float albedo;
        float metalness;
        float ior;
        bool alphaTested;
    };

    //== Import ====================================================================
    struct ImportedMesh
    {
        CArray<float3> positions;
        CArray<float3> normals;
        CArray<float2> uv0;
        CArray<float3> tangents;
        CArray<float3> bitangents;

        CArray<uint32> indices;
        uint32         materialIndex;
    };

    struct ImportedModel
    {
        CArray<ImportedMesh*> meshes;
        CArray<FixedString256> materials;
        CameraSettings camera;
    };

    //== Build =====================================================================
    struct BuiltMeshData
    {
        uint32 indexCount;
        uint32 vertexCount;
        uint32 vertexOffset;
    };

    struct BuiltScene
    {
        // -- meta data
        CameraSettings camera;
        AxisAlignedBox aaBox;
        float4 boundingSphere;

        // -- material information
        CArray<FilePathString> textures;
        CArray<Material>       materials;

        // -- geometry information
        CArray<BuiltMeshData>       meshes;
        CArray<uint32>              indices;
        CArray<uint32>              alphaTestedIndices;
        CArray<float3>              positions;
        CArray<VertexAuxiliaryData> vertexData;
    };

}