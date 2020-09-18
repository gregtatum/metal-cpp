#include <metal_stdlib>
using namespace metal;
#include "./big-triangle.h"
#include "viz/shader-utils.h"

vertex BigTriangleVarying
bigTriangleVert(const device packed_float2* vertexArray [[buffer(0)]],
                constant VizTickUniforms& tick [[buffer(1)]],
                unsigned int i [[vertex_id]])
{
  BigTriangleVarying varying;
  float2 position = vertexArray[i];
  varying.uv = position * 0.5 + 0.5;
  varying.coordinate =
    float2(position.x * tick.width / tick.height, position.y);
  varying.position = float4(position, 1.0, 1.0);
  return varying;
}

fragment half4
bigTriangleFragExample(BigTriangleVarying varying [[stage_in]],
                       constant VizTickUniforms& tick [[buffer(0)]])
{
  return half4(half2(varying.uv), 1.0, 1.0);
}
