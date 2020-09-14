#include "viz/shader-utils.h"
#include <simd/simd.h>

struct SpherePropsUniforms
{
  float brightness;
  float scale;
};

struct SceneUniforms
{
  float seconds;
};

struct ModelUniforms
{
  ModelMatrices matrices;
  simd::float3 position;
  float radius;
  float brightness;
};

#ifndef __METAL_VERSION__
#include "viz/debug.h"
namespace viz {
VIZ_DEBUG_OBJ(uniforms, ModelUniforms, {
  VIZ_DEBUG_OBJ_HEADER(ModelUniforms, uniforms, tabDepth);
  VIZ_DEBUG_OBJ_PROP(uniforms, tabDepth, matrices, ModelMatrices);
  VIZ_DEBUG_OBJ_PROP(uniforms, tabDepth, position, simd::float3);
  VIZ_DEBUG_OBJ_PROP(uniforms, tabDepth, radius, float);
  VIZ_DEBUG_OBJ_FOOTER(tabDepth);
});
}
#endif

#ifdef __METAL_VERSION__

struct Varying
{
  half4 color;
  float3 position;
  float4 projection [[position]];
};

#endif //  __METAL_VERSION__
