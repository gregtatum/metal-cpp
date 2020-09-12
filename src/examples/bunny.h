#include "viz/shader-utils.h"
#include <simd/simd.h>

struct Uniforms
{
  ModelMatrices matrices;
};

#ifdef __METAL_VERSION__

struct Varying
{
  half4 color;
  float4 position [[position]];
};

#endif //  __METAL_VERSION__
