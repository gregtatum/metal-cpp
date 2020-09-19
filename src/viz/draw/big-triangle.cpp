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
      .label = "Big Triangle Pipeline",
      .vertexFunction = library.NewFunction("bigTriangleVert"),
      // This can be used here initially:
      // .fragmentFunction = library.NewFunction("background"),
      .fragmentFunction = fragmentFunction,
      .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
      .colorAttachmentPixelFormats =
        std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
    }))
  , mDepth(InitializeDepthStencil({
      .device = device,
      .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
      .depthWriteEnabled = false,
    }))
  , mLabel(label)
{}

void
BigTriangle::Draw(AutoDraw& draw)
{
  draw.Draw({
    .label = mLabel,
    .renderPipelineState = mPipeline,
    .primitiveType = mtlpp::PrimitiveType::Triangle,
    .vertexStart = 0,
    .vertexCount = 3,
    .vertexBuffers =
      std::vector({ &mPositions.buffer, &draw.GetTickUniforms().buffer }),
    .fragmentBuffers = std::vector({ &draw.GetTickUniforms().buffer }),
    // General draw config
    .cullMode = mtlpp::CullMode::None,
    .depthStencilState = mDepth,
  });
}

} // namespace viz
