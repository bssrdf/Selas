#pragma once

//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "GeometryLib/Ray.h"
#include "StringLib/FixedString.h"
#include "MathLib/FloatStructs.h"
#include "MathLib/IntStructs.h"

namespace Selas
{
    class CSerializer;
    class CSampler;

    struct CameraSettings
    {
        FixedString128 name;
        float3 position;
        float  fov;
        float3 lookAt;
        float  znear;
        float3 up;
        float  zfar;
    };

    //=============================================================================================================================
    struct RayCastCameraSettings
    {
        float4x4 imageToWorld;
        float4x4 worldToClip;
        float3   position;
        float3   forward;
        float    viewportWidth;
        float    viewportHeight;
        float    znear;
        float    zfar;
        uint     width;
        uint     height;

        // -- the distance from the camera that you'd have to travel before the area of a single pixel is 1.
        float    virtualImagePlaneDistance;
    };

    void Serialize(CSerializer* serializer, CameraSettings& data);

    void InvalidCameraSettings(CameraSettings* settings);
    void DefaultCameraSettings(CameraSettings* settings);

    bool ValidCamera(const CameraSettings& settings);

    int2 WorldToImage(const RayCastCameraSettings* __restrict camera, float3 world);
    float3 ImageToWorld(const RayCastCameraSettings* __restrict camera, float x, float y);

    Ray JitteredCameraRay(const RayCastCameraSettings* __restrict camera, CSampler* sampler, float viewX, float viewY);
    Ray JitteredCameraRay(const RayCastCameraSettings* __restrict camera, int32 x, int32 y, int32 s, int32 m, int32 n, int32 p);

    void InitializeRayCastCamera(const CameraSettings& settings, uint width, uint height, RayCastCameraSettings& camera);
}