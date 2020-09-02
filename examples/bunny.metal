#include <metal_stdlib>
using namespace metal;
#include "./bunny.h"

vertex Varying
vert(const device packed_float3* vertexArray [[buffer(0)]],
     const device packed_float3* normalArray [[buffer(1)]],
     constant Uniforms& uniforms [[buffer(2)]],
     unsigned int i [[vertex_id]])
{
  Varying varying;
  float3 position = vertexArray[i];
  float3 modelNormal = normalArray[i];

  float3 normal = normalize(uniforms.matrices.normalModelView * modelNormal);

  float3 hue = normal * 0.5 + 0.5;
  float3 brightness = dot(normal, normalize(float3(-1.0, -1.0, -0.5)));
  float3 color = hue * mix(0.4, 0.8, brightness);

  varying.color = half4(static_cast<half3>(color), 1.0);
  varying.position = uniforms.matrices.modelViewProj * float4(position, 1.0);

  return varying;
}

fragment half4
frag(Varying varying [[stage_in]], constant Uniforms& uniforms [[buffer(0)]])
{
  return half4(varying.color.xyz, 1.0);
}
