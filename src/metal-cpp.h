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

} // viz
