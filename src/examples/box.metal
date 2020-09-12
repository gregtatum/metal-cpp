#include <metal_stdlib>
using namespace metal;
#include "./box.h"

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

  float3 hue = mix(float3{ 1.0, 0.0, 0.5 }, (normal * 0.5 + 0.5).grb, 0.5);
  float3 brightness = dot(normal, normalize(float3(-1.0, 1.0, 0.5)));
  float3 color = hue * hue * mix(0.5, 0.9, brightness);

  varying.color = half4(static_cast<half3>(color) * 1.8, 1.0);
  float3 rotatedPosition =
    position *
    //
    Mat3RotateX(uniforms.position.x +
                sin(uniforms.seconds + uniforms.position.x * 3.0)) *
    Mat3RotateZ(uniforms.position.z +
                sin(uniforms.seconds + uniforms.position.z * 5.2));
  varying.position =
    uniforms.matrices.modelViewProj * float4(rotatedPosition, 1.0);

  return varying;
}

fragment half4
frag(Varying varying [[stage_in]], constant Uniforms& uniforms [[buffer(0)]])
{
  return half4(varying.color.xyz, 1.0);
}
