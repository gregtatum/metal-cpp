#pragma once
#include <simd/simd.h>

#ifdef __METAL_VERSION__

struct BigTriangleVarying
{
  // 0 to 1 going from bottom right corner to the top left.
  // ^
  // |
  // .-->
  float2 uv;
  // Centered around 0, and transformed by the screen size.
  //
  // actual screen size
  // v
  //  ------ ----------- --------
  // |       |    ^    |         |
  // |       | <--•--> |         |
  // |       |    v    |         |
  //  ------ ----------- --------
  //         ^
  //         Coordinate space
  float2 coordinate;
  float4 position [[position]];
};

#else
#include "viz.h"

namespace viz {

/**
 * Draw one triangle that is bigger that covers the entire XY clip space and is
 * located at Z=1. This is an efficient way to draw to every pixel on the
 * screen. It does not affect the depth buffer.
 */
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
