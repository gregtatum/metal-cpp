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
#include "viz-math.h"

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
