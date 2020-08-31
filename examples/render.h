#include <simd/simd.h>

struct Uniforms
{
  float alpha = 0.5;
  vector_float2 center;
  simd::float4x4 model;
  simd::float4x4 view;
  simd::float4x4 projection;
};
