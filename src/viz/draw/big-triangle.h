#pragma once
#include <simd/simd.h>

#ifdef __METAL_VERSION__

struct BigTriangleVarying
{
  float2 uv;
  float4 position [[position]];
};

#else
#include "viz.h"

namespace viz {

class BigTriangle
{
public:
  BigTriangle() = delete;
  explicit BigTriangle(const char* label,
                       Device& device,
                       mtlpp::Library& library,
                       mtlpp::Function&& fragmentFunction);

  void Draw(AutoDraw& draw);
  BufferViewList<viz::Vector2> mPositions;
  BufferViewList<std::array<uint32_t, 3>> mCells;
  BufferViewStruct<VizTickUniforms> mTickUniforms;
  mtlpp::RenderPipelineState mPipeline;
  mtlpp::DepthStencilState mDepth;
  const char* mLabel;
};

} // namespace viz

#endif
