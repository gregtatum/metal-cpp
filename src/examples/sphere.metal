#include <metal_math>
#include <metal_stdlib>
using namespace metal;
#include "./sphere.h"
#include "viz/draw/big-triangle.h"

vertex Varying
smallSphereVert(const device packed_float3* vertexArray [[buffer(0)]],
                const device packed_float3* normalArray [[buffer(1)]],
                constant ModelUniforms& model [[buffer(2)]],
                constant SpherePropsUniforms& sphereProps [[buffer(3)]],
                unsigned int i [[vertex_id]])
{
  Varying varying;
  float3 position = vertexArray[i];
  // float3 modelNormal = normalArray[i];
  // float3 normal = normalize(model.matrices.normalModelView * modelNormal);
  varying.color = half4(half3(0.0, 1.0, 0.2) * sphereProps.brightness, 1.0);
  varying.projection =
    model.matrices.modelViewProj * float4(position * sphereProps.scale, 1.0);

  return varying;
}

fragment half4
smallSphereFrag(Varying varying [[stage_in]],
                constant ModelUniforms& model [[buffer(0)]],
                constant SpherePropsUniforms& sphereProps [[buffer(1)]])
{
  return half4(varying.color.xyz, 1.0);
}

vertex Varying
bigSphereVert(const device packed_float3* vertexArray [[buffer(0)]],
              const device packed_float3* normalArray [[buffer(1)]],
              constant SceneUniforms& scene [[buffer(2)]],
              constant ModelUniforms& model [[buffer(3)]],
              unsigned int i [[vertex_id]])
{
  auto position = vertexArray[i];
  Varying varying;
  varying.position = position;
  varying.projection = model.matrices.modelViewProj * float4(position, 1.0);
  return varying;
}

fragment half4
bigSphereFrag(Varying varying [[stage_in]],
              constant SceneUniforms& scene [[buffer(0)]],
              constant ModelUniforms& model [[buffer(1)]])
{
  float position = varying.position.y - varying.position.x * 0.05;
  // Build up several frequencies of lines that move over the sphere.
  float frequency1 = sin(scene.seconds + position * 20.0) * 0.5 + 0.5;
  frequency1 *= frequency1;
  frequency1 *= frequency1;
  frequency1 *= frequency1;
  frequency1 *= frequency1;

  // This one is much smoother.
  float frequency2 =
    sin(scene.seconds * 2.77f + position * 60.0f) * 0.5f + 0.5f;

  // This one is especially sharp.
  float frequency3 =
    sin(scene.seconds * 3.33f + position * 133.0f) * 0.5f + 0.5f;
  frequency3 *= frequency3;
  frequency3 *= frequency3;
  frequency3 *= frequency3;
  frequency3 *= frequency3;

  float frequency = frequency1 + frequency2 + frequency3;

  float weirdPosition = varying.position.y * varying.position.x * 5.0;
  float weirdFrequency =
    mix(0.5, 1.5, abs(sin(scene.seconds * 2.0f + weirdPosition)));

  float brightness = 0.12f;

  auto color = half3(0.0, brightness, 0.0) * (weirdFrequency + frequency);
  return half4(color, 1.0);
}
