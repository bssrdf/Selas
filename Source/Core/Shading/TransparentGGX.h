#pragma once

//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "MathLib/FloatStructs.h"

namespace Selas
{
    class CSampler;
    struct HitParameters;
    struct SurfaceParameters;
    struct BsdfSample;
    
    // -- BSDF evaluation for next event estimation
    float3 EvaluateTransparentGGXBsdf(const SurfaceParameters& surface, const float3& v, const float3& l,
                                      float& forwardPdf, float& reversePdf);

    // -- Shaders
    bool SampleTransparentGgx(CSampler* sampler, const SurfaceParameters& surface, const float3& v, BsdfSample& sample);
}