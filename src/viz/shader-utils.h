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

struct VizTickUniforms
{
  // The amount of time passed in milliseconds.
  float milliseconds;
  float seconds;
  // The change in time in milliseconds.
  float dt;
  // A generational counter for how many ticks have been called.
  uint32_t tick;
  // The width of the window.
  float width;
  // The height of the window.
  float height;
  bool isMouseDown;
};

// Begin the utilities, which will only be defined in CPU-land.
#ifndef __METAL_VERSION__
#include "viz/debug.h"
#include "viz/macros.h"
#include "viz/math.h"

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

using namespace viz;

ModelMatrices
GetModelMatrices(Matrix4 model, Matrix4 view, Matrix4 projection);
}

#endif //  __METAL_VERSION__

simd::float3x3
Mat3RotateX(float radians);
simd::float3x3
Mat3RotateY(float radians);
simd::float3x3
Mat3RotateZ(float radians);
simd::float4x4
Mat4RotateX(float radians);
simd::float4x4
Mat4RotateY(float radians);
simd::float4x4
Mat4RotateZ(float radians);
