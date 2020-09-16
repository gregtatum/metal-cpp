#include "big-triangle.h"
#include <vector>

namespace viz {

BigTriangle::BigTriangle(const char* label,
                         Device& device,
                         mtlpp::Library& library,
                         mtlpp::Function&& fragmentFunction)
  : mPositions(
      BufferViewList<Vector2>(device,
                              mtlpp::ResourceOptions::CpuCacheModeWriteCombined,
                              std::vector<Vector2>{
                                { -1.0, -1.0 },
                                { -1.0, 4.0 },
                                { 4.0, -1.0 },
                              }))
  , mCells(BufferViewList<std::array<uint32_t, 3>>(
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined,
      std::vector<std::array<uint32_t, 3>>{ { 0, 1, 2 } }))
  , mTickUniforms(BufferViewStruct<VizTickUniforms>(
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined))
  , mPipeline(viz::InitializeRenderPipeline({
      .device = device,
      .label = "Big Triangle",
      .vertexFunction = library.NewFunction("bigTriangleVert"),
      .fragmentFunction = library.NewFunction("bigTriangleFragExample"),
      .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
      .colorAttachmentPixelFormats =
        std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
    }))
  , mDepth(InitializeDepthStencil({
      .device = device,
      .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
      .depthWriteEnabled = true,
    }))
  , mLabel(label)
{}

void
BigTriangle::Draw(AutoDraw& draw)
{
  draw.Draw({
    .label = mLabel,
    .renderPipelineState = mPipeline,
    .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
    .vertexStart = 0,
    .vertexCount = 3,
    .vertexBuffers = std::vector({
      &mPositions.buffer,
    }),
    .fragmentBuffers = std::vector({ &draw.GetTickUniforms().buffer }),
    // General draw config
    .cullMode = mtlpp::CullMode::Front,
    .depthStencilState = mDepth,
  });
}

} // namespace viz
