#include "viz/shader-utils.h"

#ifndef __METAL_VERSION__

namespace viz {

using namespace viz;

ModelMatrices
GetModelMatrices(Matrix4 model, Matrix4 view, Matrix4 projection)
{
  auto modelView = view * model;

  ModelMatrices matrices;

  matrices.model = model;
  matrices.view = view;
  matrices.projection = projection;

  matrices.modelView = modelView;
  matrices.modelViewProj = projection * modelView;
  matrices.normalModel = model.ToNormalMatrix();
  matrices.normalView = view.ToNormalMatrix();
  matrices.normalModelView = modelView.ToNormalMatrix();

  return matrices;
}
}

#endif //  __METAL_VERSION__

simd::float3x3
Mat3RotateX(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float3{ 1.0f, 0.0f, 0.0f },
           simd::float3{ 0.0f, c, s },
           simd::float3{ 0.0f, -s, c } };
}

simd::float3x3
Mat3RotateY(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float3{ c, 0.0f, -s },
           simd::float3{ 0.0f, 1.0f, 0.0f },
           simd::float3{ s, 0.0f, c } };
}

simd::float3x3
Mat3RotateZ(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float3{ c, s, 0.0f },
           simd::float3{ -s, c, 0.0f },
           simd::float3{ 0.0f, 0.0f, 1.0f } };
}

simd::float4x4
Mat4RotateX(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float4{ 1.0f, 0.0f, 0.0f, 0.0f },
           simd::float4{ 0.0f, c, s, 0.0f },
           simd::float4{ 0.0f, -s, c, 0.0f },
           simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f } };
}

simd::float4x4
Mat4RotateY(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float4{ c, 0.0f, -s, 0.0f },
           simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
           simd::float4{ s, 0.0f, c, 0.0f },
           simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f } };
}

simd::float4x4
Mat4RotateZ(float radians)
{
  float c = cos(radians);
  float s = sin(radians);
  return { simd::float4{ c, s, 0.0f, 0.0f },
           simd::float4{ -s, c, 0.0f, 0.0f },
           simd::float4{ 0.0f, 0.0f, 1.0f, 0.0f },
           simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f } };
}
