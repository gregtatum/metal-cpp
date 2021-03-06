#include <metal_math>
#include <metal_stdlib>
using namespace metal;
#include "./sphere.h"
#include "viz/draw/big-triangle.h"
#include "viz/shaders/noise.h"

vertex Varying
smallSphereVert(const device packed_float3* vertexArray [[buffer(0)]],
                const device packed_float3* normalArray [[buffer(1)]],
                constant ModelUniforms* model [[buffer(2)]],
                unsigned int vid [[vertex_id]],
                unsigned int iid [[instance_id]])
{
  Varying varying;
  float3 position = vertexArray[vid];
  varying.color = half4(half3(0.0, 1.0, 0.2) * model[iid].brightness, 1.0);
  varying.projection =
    model[iid].matrices.modelViewProj * float4(position, 1.0);

  return varying;
}

fragment half4
smallSphereFrag(Varying varying [[stage_in]],
                constant ModelUniforms& model [[buffer(0)]])
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

fragment half4
background(BigTriangleVarying varying [[stage_in]],
           constant VizTickUniforms& tick [[buffer(0)]])
{
  float l = length(varying.coordinate);
  float brightness = max(0.0, 1.3 - pow(l, 0.5));

  half n1 =
    noise::simplex(float3(varying.coordinate * 2.0, tick.seconds * 0.1));
  half n2 = noise::simplex(
    float3(varying.coordinate * 2.2 + 100.0 + n1, tick.seconds * 0.3));
  half n3 = noise::simplex(
    float3(varying.coordinate * 20.0 + 200.0 - n2 * 0.1, tick.seconds * 0.5));

  brightness *= 0.5 + (0.5 * 0.3333) * (n1 + n2 + n3);

  return half4(0.0, brightness, brightness * 0.4, 1.0);
}
