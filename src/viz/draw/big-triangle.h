#include <simd/simd.h>

struct BigTriangleVaryings
{
  simd::float2 uv;
};

#ifdef __METAL_VERSION__

struct Varying
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
  explicit BigTriangle(Device& device,
                       mtlpp::Library& library,
                       mtlpp::Function&& fragmentFunction);

  void Draw(AutoDraw& draw);
  BufferViewList<std::vector<viz::Vector3>> mPositions;
  BufferViewList<std::vector<std::array<uint32_t, 3>>> mCells;
  BufferViewStruct<VizTickUniforms> mTickUniforms;
  mtlpp::RenderPipelineState mPipeline;
  mtlpp::DepthStencilState mDepth;
};

} // namespace viz

#endif
