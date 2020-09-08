#pragma once
#include "mtlpp/mtlpp.hpp"
#include <optional>
#include <vector>

namespace viz {

struct RenderPipelineInitializer
{
  mtlpp::Device& device;
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
InitializeRenderPipeline(RenderPipelineInitializer&& initializer)
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

  return initializer.device.NewRenderPipelineState(descriptor, nullptr);
}

struct DepthStencilInitializer
{
  mtlpp::Device& device;
  std::optional<mtlpp::CompareFunction> depthCompareFunction = std::nullopt;
  std::optional<bool> depthWriteEnabled = std::nullopt;
};

mtlpp::DepthStencilState
InitializeDepthStencil(DepthStencilInitializer&& initializer)
{
  mtlpp::DepthStencilDescriptor depthDescriptor{};

  auto& [device, depthCompareFunction, depthWriteEnabled] = initializer;

  if (depthCompareFunction)
    depthDescriptor.SetDepthCompareFunction(depthCompareFunction.value());
  if (depthWriteEnabled)
    depthDescriptor.SetDepthWriteEnabled(depthWriteEnabled.value());

  return device.NewDepthStencilState(depthDescriptor);
}

struct RenderInitializer
{
  // Required dependencies.
  mtlpp::CommandQueue& commandQueue;
  mtlpp::RenderPipelineState& renderPipelineState;
  mtlpp::RenderPassDescriptor& renderPassDescriptor;
  mtlpp::Drawable& drawable;

  // DrawIndexed options plus buffers.
  mtlpp::PrimitiveType drawPrimitiveType;
  uint32_t drawIndexCount;
  mtlpp::IndexType drawIndexType;
  const mtlpp::Buffer& drawIndexBuffer;
  std::optional<std::vector<mtlpp::Buffer*>> vertexBuffers;
  std::optional<std::vector<mtlpp::Buffer*>> fragmentBuffers;

  // General draw config
  std::optional<mtlpp::CullMode> cullMode;
  std::optional<mtlpp::DepthStencilState> depthStencilState;
};

void
Render(RenderInitializer&& initializer)
{
  auto& [commandQueue,
         renderPipelineState,
         renderPassDescriptor,
         drawable,
         drawPrimitiveType,
         drawIndexCount,
         drawIndexType,
         drawIndexBuffer,
         vertexBuffers,
         fragmentBuffers,
         cullMode,
         depthStencilState] = initializer;

  mtlpp::CommandBuffer commandBuffer = commandQueue.CommandBuffer();

  mtlpp::RenderCommandEncoder renderCommandEncoder =
    commandBuffer.RenderCommandEncoder(renderPassDescriptor);

  renderCommandEncoder.SetRenderPipelineState(renderPipelineState);

  if (cullMode) {
    renderCommandEncoder.SetCullMode(cullMode.value());
  }

  if (depthStencilState) {
    renderCommandEncoder.SetDepthStencilState(depthStencilState.value());
  }

  if (vertexBuffers) {
    auto& buffers = vertexBuffers.value();
    for (size_t i = 0; i < buffers.size(); i++) {
      auto& buffer = buffers[i];
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetVertexBuffer(*buffer, 0, i);
    }
  }

  if (fragmentBuffers) {
    auto& buffers = fragmentBuffers.value();
    for (size_t i = 0; i < buffers.size(); i++) {
      auto& buffer = buffers[i];
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetFragmentBuffer(*buffer, 0, i);
    }
  }

  renderCommandEncoder.DrawIndexed(drawPrimitiveType,
                                   drawIndexCount,
                                   drawIndexType,
                                   drawIndexBuffer,
                                   // offset
                                   0);

  renderCommandEncoder.EndEncoding();
  commandBuffer.Present(initializer.drawable);

  commandBuffer.Commit();
  commandBuffer.WaitUntilCompleted();
}
} // viz
