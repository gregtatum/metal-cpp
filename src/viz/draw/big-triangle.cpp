#include "big-triangle.h"
#include <vector>

namespace viz {

BigTriangle::BigTriangle(Device& device,
                         mtlpp::Library& library,
                         mtlpp::Function&& fragmentFunction)
  : mPositions(BufferViewList<std::vector<Vector3>>(
      device,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined,
      std::vector<Vector3>{
        { -10.0, -10.0, 0.0 },
        { -10.0, 10.0, 0.0 },
        { 10.0, 0.0, 0.0 },
      }))
  , mCells(BufferViewList<std::vector<std::array<uint32_t, 3>>>(
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
{}

void
BigTriangle::Draw(AutoDraw& draw)
{
  draw.Render({
    .renderPipelineState = mPipeline,
    .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
    // 3 Triangles with 3 positions each = 9.
    .drawIndexCount = 9,
    .drawIndexType = mtlpp::IndexType::UInt32,
    .drawIndexBuffer = mCells.buffer,
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
