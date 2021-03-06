#include "big-triangle.h"
#include <vector>

namespace viz {

BigTriangle::BigTriangle(Device& device,
                         mtlpp::Library& library,
                         const char* label,
                         const char* fragmentFunction)
  : mPositions(BufferViewList<Vector2>(
      std::vector<Vector2>{
        { -1.0, -1.0 },
        { -1.0, 4.0 },
        { 4.0, -1.0 },
      },
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined))
  , mCells(BufferViewList<std::array<uint32_t, 3>>(
      std::vector<std::array<uint32_t, 3>>{ { 0, 1, 2 } },
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined))
  , mTickUniforms(BufferViewStruct<VizTickUniforms>(
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined))
  , mPipeline(viz::InitializeRenderPipeline({
      .device = device,
      .library = library,
      .label = "Big Triangle Pipeline",
      .vertexFunction = "bigTriangleVert",
      // This can be used here initially:
      // .fragmentFunction = "background",
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
    .vertexInputs =
      std::vector({ mPositions.Ref(), draw.GetTickUniforms().Ref() }),
    .fragmentInputs = std::vector({ draw.GetTickUniforms().Ref() }),
    // General draw config
    .cullMode = mtlpp::CullMode::None,
    .depthStencilState = mDepth,
  });
}

} // namespace viz
