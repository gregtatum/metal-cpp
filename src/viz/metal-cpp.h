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
  mtlpp::RenderPipelineState renderPipelineState;
  mtlpp::PrimitiveType drawPrimitiveType;
  uint32_t drawIndexCount;
  mtlpp::IndexType drawIndexType;
  const mtlpp::Buffer& drawIndexBuffer;
  std::vector<mtlpp::Buffer*> vertexBuffers;
  std::vector<mtlpp::Buffer*> fragmentBuffers;

  // General draw config
  std::optional<mtlpp::CullMode> cullMode;
  std::optional<mtlpp::DepthStencilState> depthStencilState;
};

// struct RenderInstancedInitializer
// {
//   // Required dependencies.
//   mtlpp::CommandQueue& commandQueue;
//   mtlpp::RenderPipelineState& renderPipelineState;
//   mtlpp::RenderPassDescriptor& renderPassDescriptor;

//   // DrawIndexed options plus buffers.
//   mtlpp::PrimitiveType drawPrimitiveType;
//   uint32_t drawIndexCount;
//   mtlpp::IndexType drawIndexType;
//   const mtlpp::Buffer& drawIndexBuffer;
//   std::optional<std::vector<mtlpp::Buffer*>> vertexBuffers;
//   std::optional<std::vector<mtlpp::Buffer*>> fragmentBuffers;

//   // General draw config
//   std::optional<mtlpp::CullMode> cullMode;
//   std::optional<mtlpp::DepthStencilState> depthStencilState;
// };

// void
// RenderInstanced(RenderInstancedInitializer&& initializer)
// {

//   renderCommandEncoder.DrawIndexed(
//     PrimitiveType primitiveType,
//     uint32_t indexCount,
//     IndexType indexType,
//     const Buffer& indexBuffer,
//     uint32_t indexBufferOffset,
//     uint32_t instanceCount
//   )
// }

/**
 * Automatically handles a draw pass based based on when the AutoDraw object is
 * in scope.
 */
class AutoDraw
{
public:
  AutoDraw(mtlpp::CommandQueue& aCommandQueue, Tick& aTick)
    : mCommandQueue(aCommandQueue)
    , mTick(aTick)
  {
    mCommandBuffer = mCommandQueue.CommandBuffer();
  }

  ~AutoDraw()
  {
    mCommandBuffer.Present(mTick.drawable);
    mCommandBuffer.Commit();
    mCommandBuffer.WaitUntilCompleted();
  }

  // Delete copy and move.
  AutoDraw(const AutoDraw&) = delete;
  AutoDraw& operator=(const AutoDraw) = delete;

  /**
   * Call the clear functions to clear the next draw call. Typically this will
   * be used before adding any commands, so that the buffer will be cleared.
   */
  void Clear() { mClear = true; }
  void Clear(mtlpp::ClearColor&& aClearColor)
  {
    mClear = true;
    mClearColor = aClearColor;
  }
  void Clear(double red, double green, double blue, double alpha)
  {
    this->Clear(mtlpp::ClearColor(red, green, blue, alpha));
  }
  void Clear(double red, double green, double blue)
  {
    this->Clear(mtlpp::ClearColor(red, green, blue, 1.0));
  }

  void Render(RenderInitializer&& initializer)
  {
    auto& [renderPipelineState,
           drawPrimitiveType,
           drawIndexCount,
           drawIndexType,
           drawIndexBuffer,
           vertexBuffers,
           fragmentBuffers,
           cullMode,
           depthStencilState] = initializer;

    // This command doesn't know about multiple color attachments. Handle the
    // clear statement. This was a big motiviation for originally creating this
    // class. Essentially, the first draw call probably needs to be cleared, but
    // the subsequent ones do not.
    auto colorAttachment = mTick.renderPassDescriptor.GetColorAttachments()[0];
    auto depthAttachment = mTick.renderPassDescriptor.GetDepthAttachment();
    if (mClear) {
      if (mClearColor) {
        colorAttachment.SetClearColor(mClearColor.value());
      }
      colorAttachment.SetLoadAction(mtlpp::LoadAction::Clear);
      depthAttachment.SetLoadAction(mtlpp::LoadAction::Clear);
      // Only clear once.
      mClear = false;
    } else {
      colorAttachment.SetLoadAction(mtlpp::LoadAction::Load);
      depthAttachment.SetLoadAction(mtlpp::LoadAction::Load);
    }

    mtlpp::RenderCommandEncoder renderCommandEncoder =
      mCommandBuffer.RenderCommandEncoder(mTick.renderPassDescriptor);

    renderCommandEncoder.SetRenderPipelineState(renderPipelineState);

    for (size_t i = 0; i < vertexBuffers.size(); i++) {
      auto& buffer = vertexBuffers[i];
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetVertexBuffer(*buffer, 0, i);
    }

    for (size_t i = 0; i < fragmentBuffers.size(); i++) {
      auto& buffer = fragmentBuffers[i];
      ReleaseAssert(buffer, "VertexBuffer must exist.");
      renderCommandEncoder.SetFragmentBuffer(*buffer, 0, i);
    }

    // Handle the optional configs.
    if (cullMode) {
      renderCommandEncoder.SetCullMode(cullMode.value());
    }

    if (depthStencilState) {
      renderCommandEncoder.SetDepthStencilState(depthStencilState.value());
    }

    renderCommandEncoder.DrawIndexed(drawPrimitiveType,
                                     drawIndexCount,
                                     drawIndexType,
                                     drawIndexBuffer,
                                     // offset
                                     0);

    renderCommandEncoder.EndEncoding();
  }

  // Dependencies.
  mtlpp::CommandQueue& mCommandQueue;
  Tick& mTick;

  // Owned data.
  mtlpp::CommandBuffer mCommandBuffer;
  bool mClear = false;
  std::optional<mtlpp::ClearColor> mClearColor;
};

} // viz
