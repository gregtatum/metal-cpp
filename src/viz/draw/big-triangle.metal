#include <metal_stdlib>
using namespace metal;
#include "./big-triangle.h"

vertex Varying
bigTriangleVert(const device packed_float3* vertexArray [[buffer(0)]],
                unsigned int i [[vertex_id]])
{
  Varying varying;
  float3 position = vertexArray[i];
  varying.uv = position.xy * 0.5 + 0.5;
  varying.position = float4(position, 1.0);
  return varying;
}

fragment half4
bigTriangleFragExample(Varying varying [[stage_in]],
                       constant VizTickUniforms& tick [[buffer(0)]])
{
  return half4(varying.uv, 1.0, 1.0);
}
