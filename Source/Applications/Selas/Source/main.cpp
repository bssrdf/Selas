
//==============================================================================
// Joe Schutte
//==============================================================================

#include "PathTracer.h"
#include "VCM.h"
#include "Shading/IntegratorContexts.h"

#include "Shading/SurfaceParameters.h"
#include "SceneLib/SceneResource.h"
#include "SceneLib/ImageBasedLightResource.h"
#include "TextureLib/StbImage.h"
#include "TextureLib/TextureFiltering.h"
#include "TextureLib/TextureResource.h"
#include "IoLib/Environment.h"
#include "StringLib/FixedString.h"
#include "SystemLib/Error.h"
#include "SystemLib/MemoryAllocation.h"
#include "SystemLib/BasicTypes.h"
#include "SystemLib/Memory.h"
#include "SystemLib/SystemTime.h"
#include "SystemLib/Logging.h"

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>

#include "xmmintrin.h"
#include "pmmintrin.h"
#include <stdio.h>

using namespace Selas;

//==============================================================================
static void IntersectionFilter(const RTCFilterFunctionNArguments* args)
{
    Assert_(args->N == 1);
    int* valid = args->valid;
    if(valid[0] != -1) {
        return;
    }

    SceneResource* scene = (SceneResource*)args->geometryUserPtr;

    RTCHit hit = rtcGetHitFromHitN(args->hit, args->N, 0);

    uint32 primitiveId = hit.primID;
    float2 baryCoords = { hit.u, hit.v };

    valid[0] = CalculatePassesAlphaTest(scene, primitiveId, baryCoords);
}

//==============================================================================
static void PopulateEmbreeScene(SceneResource* scene, RTCDevice& rtcDevice, RTCScene& rtcScene) {

    SceneResourceData* sceneData = scene->data;
    uint32 vertexCount = sceneData->totalVertexCount;
    uint32 indexCount = sceneData->solidIndexCount;
    uint32 triangleCount = indexCount / 3;

    RTCGeometry solidMeshHandle = rtcNewGeometry(rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetSharedGeometryBuffer(solidMeshHandle, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sceneData->positions, 0, sizeof(float3), sceneData->totalVertexCount);
    rtcSetSharedGeometryBuffer(solidMeshHandle, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sceneData->indices, 0, 3 * sizeof(uint32), sceneData->solidIndexCount / 3);
    rtcCommitGeometry(solidMeshHandle);
    rtcAttachGeometry(rtcScene, solidMeshHandle);
    rtcReleaseGeometry(solidMeshHandle);

    RTCGeometry atMeshHandle = rtcNewGeometry(rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetSharedGeometryBuffer(atMeshHandle, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sceneData->positions, 0, sizeof(float3), sceneData->totalVertexCount);
    rtcSetSharedGeometryBuffer(atMeshHandle, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sceneData->atIndices, 0, 3 * sizeof(uint32), sceneData->atIndexCount / 3);
    rtcCommitGeometry(atMeshHandle);
    rtcAttachGeometry(rtcScene, atMeshHandle);
    rtcSetGeometryIntersectFilterFunction(atMeshHandle, IntersectionFilter);
    rtcSetGeometryUserData(atMeshHandle, scene);
    rtcReleaseGeometry(atMeshHandle);

    rtcCommitScene(rtcScene);
}

//==============================================================================
int main(int argc, char *argv[])
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    Environment_Initialize(ProjectRootName_, argv[0]);

    int retvalue = 0;

    TextureFiltering::InitializeEWAFilterWeights();

    RTCDevice rtcDevice = rtcNewDevice(nullptr/*"verbose=3"*/);
    RTCScene rtcScene = rtcNewScene(rtcDevice);

    auto timer = SystemTime::Now();

    SceneResource sceneResource;
    ExitMainOnError_(ReadSceneResource("Scenes~SanMiguel~SanMiguel.fbx", &sceneResource));
    ExitMainOnError_(InitializeSceneResource(&sceneResource));

    ImageBasedLightResource iblResouce;
    ExitMainOnError_(ReadImageBasedLightResource("HDR~noon_grass_4k_upper.hdr", &iblResouce));

    float elapsedMs = SystemTime::ElapsedMillisecondsF(timer);
    WriteDebugInfo_("Scene load time %fms", elapsedMs);

    timer = SystemTime::Now();
    
    PopulateEmbreeScene(&sceneResource, rtcDevice, rtcScene);
    
    elapsedMs = SystemTime::ElapsedMillisecondsF(timer);
    WriteDebugInfo_("Scene build time %fms", elapsedMs);

    //sceneResource.data->camera.fov = 0.7f;
    //uint width = 256;
    //uint height = 256;
    Selas::uint width = 1280;
    Selas::uint height = 720;

    float3* imageData = AllocArray_(float3, width * height);
    Memory::Zero(imageData, sizeof(float3) * width * height);

    float sceneBoundingRadius = sceneResource.data->boundingSphere.w;

    SceneContext context;
    context.rtcScene = rtcScene;
    context.scene = &sceneResource;
    context.ibl = iblResouce.data;

    timer = SystemTime::Now();

    VCM::GenerateImage(context, width, height, imageData);

    elapsedMs = SystemTime::ElapsedMillisecondsF(timer);
    WriteDebugInfo_("Scene render time %fms", elapsedMs);

    StbImageWrite("D:\\temp\\test.hdr", width, height, 3, HDR, imageData);
    Free_(imageData);

    // -- delete the scene
    ShutdownSceneResource(&sceneResource);
    SafeFreeAligned_(iblResouce.data);

    rtcReleaseScene(rtcScene);
    rtcReleaseDevice(rtcDevice);

    return retvalue;
}