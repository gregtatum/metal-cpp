#pragma once
#include "mtlpp/mtlpp.hpp"
#include <optional>
#include <vector>

namespace viz {

struct RenderPipelineInitializer
{
  std::optional<const char*> label = std::nullopt;
  std::optional<const mtlpp::Function> vertexFunction = std::nullopt;
  std::optional<const mtlpp::Function> fragmentFunction = std::nullopt;
  std::optional<const mtlpp::PixelFormat> depthAttachmentPixelFormat =
    std::nullopt;
  std::vector<mtlpp::PixelFormat> colorAttachmentPixelFormats = {};
  // TODO - Add more options as they are needed
};

/**
 * This function provides a single line instantiation of a render pipeline state
 * object.
 */
mtlpp::RenderPipelineState
InitializeRenderPipeline(mtlpp::Device& device,
                         RenderPipelineInitializer&& initializer)
{
  mtlpp::RenderPipelineDescriptor descriptor;

  if (initializer.label)
    descriptor.SetLabel(initializer.label.value());
  if (initializer.vertexFunction)
    descriptor.SetVertexFunction(initializer.vertexFunction.value());
  if (initializer.fragmentFunction)
    descriptor.SetFragmentFunction(initializer.fragmentFunction.value());
  if (initializer.depthAttachmentPixelFormat)
    descriptor.SetDepthAttachmentPixelFormat(
      initializer.depthAttachmentPixelFormat.value());
  for (size_t i = 0; i < initializer.colorAttachmentPixelFormats.size(); i++) {
    descriptor.GetColorAttachments()[i].SetPixelFormat(
      initializer.colorAttachmentPixelFormats[i]);
  }

  return device.NewRenderPipelineState(descriptor, nullptr);
}

struct DepthStencilInitializer
{
  std::optional<mtlpp::CompareFunction> depthCompareFunction = std::nullopt;
  std::optional<bool> depthWriteEnabled = std::nullopt;
};

mtlpp::DepthStencilState
InitializeDepthStencil(mtlpp::Device& device,
                       DepthStencilInitializer&& initializer)
{
  mtlpp::DepthStencilDescriptor depthDescriptor{};

  if (initializer.depthCompareFunction)
    depthDescriptor.SetDepthCompareFunction(
      initializer.depthCompareFunction.value());
  if (initializer.depthWriteEnabled)
    depthDescriptor.SetDepthWriteEnabled(initializer.depthWriteEnabled.value());

  return device.NewDepthStencilState(depthDescriptor);
}

struct RenderCommandInitializer
{
  // Non-optional:
  mtlpp::CommandQueue& commandQueue;
  mtlpp::RenderPipelineState& pipeline;
  mtlpp::RenderPassDescriptor& renderPassDescriptor;
  mtlpp::Drawable& drawable;

  // DrawIndexed only support until otherwise needed.
  mtlpp::PrimitiveType primitiveType;
  uint32_t indexCount;
  mtlpp::IndexType indexType;
  const mtlpp::Buffer& indexBuffer;

  // Optional:
  std::optional<mtlpp::CullMode> cullMode;
  std::optional<mtlpp::DepthStencilState> depthStencilState;
  std::optional<std::vector<mtlpp::Buffer*>> vertexBuffers;
  std::optional<std::vector<mtlpp::Buffer*>> fragmentBuffers;
};

void
Render(RenderCommandInitializer&& initializer)
{
  mtlpp::CommandBuffer commandBuffer = initializer.commandQueue.CommandBuffer();

  mtlpp::RenderCommandEncoder renderCommandEncoder =
    commandBuffer.RenderCommandEncoder(initializer.renderPassDescriptor);

  if (initializer.cullMode) {
    printf("initializer.cullMode\n");
    renderCommandEncoder.SetCullMode(initializer.cullMode.value());
  }

  if (initializer.depthStencilState) {
    printf("initializer.depthStencilState\n");
    renderCommandEncoder.SetDepthStencilState(
      initializer.depthStencilState.value());
  }

  if (initializer.vertexBuffers) {
    auto& buffers = initializer.vertexBuffers.value();
    for (size_t i = 0; i < buffers.size(); i++) {
      auto& buffer = buffers[i];
      printf("initializer.vertexBuffers %zu\n", i);
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetVertexBuffer(*buffer, 0, i);
    }
  }

  if (initializer.fragmentBuffers) {
    auto& buffers = initializer.fragmentBuffers.value();
    for (size_t i = 0; i < buffers.size(); i++) {
      auto& buffer = buffers[i];
      printf("initializer.fragmentBuffers %zu\n", i);
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetFragmentBuffer(*buffer, 0, i);
    }
  }

  renderCommandEncoder.DrawIndexed(initializer.primitiveType,
                                   initializer.indexCount,
                                   initializer.indexType,
                                   initializer.indexBuffer,
                                   // offset
                                   0);

  renderCommandEncoder.EndEncoding();
  commandBuffer.Present(initializer.drawable);

  commandBuffer.Commit();
  commandBuffer.WaitUntilCompleted();
}

} // viz
