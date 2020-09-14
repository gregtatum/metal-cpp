#pragma once
#include <simd/simd.h>

/**
 * Provides a simple interface to get any matrix that a model may want.
 * Use GetModelMatrices below to easily initialize this matrix in C++ code.
 */
struct ModelMatrices
{
  simd::float3x3 normalModelView;
  simd::float3x3 normalModel;
  simd::float3x3 normalView;
  simd::float4x4 model;
  simd::float4x4 view;
  simd::float4x4 projection;
  simd::float4x4 modelView;
  simd::float4x4 modelViewProj;
};

// Begin the utilities, which will only be defined in CPU-land.
#ifndef __METAL_VERSION__
#include "viz/debug.h"
namespace viz {
VIZ_DEBUG_OBJ(modelMatrices, ModelMatrices, {
  VIZ_DEBUG_OBJ_HEADER(ModelMatrices, modelMatrices, tabDepth);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, normalModelView, simd::float3x3);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, normalModel, simd::float3x3);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, normalView, simd::float3x3);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, model, simd::float4x4);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, view, simd::float4x4);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, projection, simd::float4x4);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, modelView, simd::float4x4);
  VIZ_DEBUG_OBJ_PROP(modelMatrices, tabDepth, modelViewProj, simd::float4x4);
  VIZ_DEBUG_OBJ_FOOTER(tabDepth);
});
}

#include "viz/math.h"
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
