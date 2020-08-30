#include <metal_stdlib>
using namespace metal;
#include "./render.h"

struct Varying
{
  half4 color;
  float4 position [[position]];
};

vertex Varying
vert(const device packed_float3* vertexArray [[buffer(0)]],
     constant Uniforms& uniforms [[buffer(1)]],
     unsigned int vID [[vertex_id]])
{
  Varying varying;
  varying.position =
    float4(vertexArray[vID].xy + uniforms.center, vertexArray[vID].z, 1.0);
  varying.color = (static_cast<half4>(varying.position) + 1.0) * 0.5;

  return varying;
}

fragment half4
frag(Varying varying [[stage_in]], constant Uniforms& uniforms [[buffer(0)]])
{
  return half4(varying.color.xyz, 1.0);
}
