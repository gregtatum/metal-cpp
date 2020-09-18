#include <Foundation/NSError.h>
#include <Metal/MTLCaptureManager.h>
#include <Metal/MTLDevice.h>
#include <iostream>
#include <sstream> // std::ostringstream
#include <string>
// Make sure including metal.h is last, otherwise there is ambiguous
// resolution of some internal types.
#include "viz/assert.h"
#include "viz/metal.h"
#include "viz/utils.h" // getExecutablePath
#include <sys/select.h>

// Create some macros for easier to read bridging.
#define OBJC(_type, _value) (__bridge _type) _value.GetPtr()
#define CPP(_type, _value)                                                     \
  static_cast<_type>(ns::Handle{ (__bridge void*)_value })

namespace viz {

NicerNSError::operator bool() const
{
  return mError.GetPtr() != nil;
}

const char*
NicerNSError::what() const noexcept
{
  if (mErrorMessage.size() == 0) {
    std::ostringstream stream;
    auto error = OBJC(NSError*, mError);
    if (error == nil) {
      // There is no error.
      return "No NSError is associated with this error.";
    }

    stream << "Error Code " << [error code] << std::endl;
    stream << "Description:" << std::endl;

    stream << [[error localizedDescription] UTF8String] << std::endl;

    if ([error localizedRecoveryOptions]) {
      stream << "Recovery Options:" << std::endl;
      for (NSString* string in [error localizedRecoveryOptions]) {
        stream << [string UTF8String] << std::endl;
      }
    }
    if ([error localizedRecoverySuggestion]) {
      stream << "Recovery Suggestions:" << std::endl;
      stream << [[error localizedRecoverySuggestion] UTF8String] << std::endl;
    }
    if ([error localizedFailureReason]) {
      stream << "Failure Reason:" << std::endl;
      stream << [[error localizedFailureReason] UTF8String] << std::endl;
    }
    mErrorMessage = stream.str();
  }
  return mErrorMessage.c_str();
}

/**
 * Objective C functions rely on passing in an error. This wrapper throws a nice
 * error instead if the function fails.
 */
template<typename Fn>
void
throwIfError(Fn callback)
{
  NSError* nsError = nil;
  callback(nsError);
  NicerNSError error{ CPP(ns::Error, nsError) };
  if (error) {
    throw error;
  }
}

template<typename Returns, typename Fn>
Returns
returnOrThrow(Fn callback)
{
  NSError* nsError = nil;
  Returns value = callback(nsError);
  NicerNSError error{ CPP(ns::Error, nsError) };
  if (error) {
    throw error;
  }
  return value;
}

std::pair<Library, NicerNSError>
CreateLibraryFromSource(Device device,
                        const char* source,
                        const CompileOptions& options)
{
  NSError* error = nil;

  auto mtlDevice = OBJC(id<MTLDevice>, device);
  auto mtlOptions = OBJC(MTLCompileOptions*, options);

  id<MTLLibrary> library =
    [mtlDevice newLibraryWithSource:[NSString stringWithUTF8String:source]
                            options:mtlOptions
                              error:&error];

  return std::pair(CPP(mtlpp::Library, library),
                   NicerNSError(CPP(ns::Error, error)));
};

std::pair<mtlpp::Library, NicerNSError>
CreateLibraryFromMetalLib(Device device, const char* metallib)
{
  NSError* error = nil;

  auto mtlDevice = OBJC(id<MTLDevice>, device);

  id<MTLLibrary> library =
    [mtlDevice newLibraryWithFile:[NSString stringWithUTF8String:metallib]
                            error:&error];

  return std::pair(CPP(mtlpp::Library, library),
                   NicerNSError(CPP(ns::Error, error)));
}

mtlpp::Library
CreateLibraryForExample(Device device)
{
  NSError* nsError = nil;

  auto path = getExecutablePath() + ".metallib";

  auto mtlDevice = OBJC(id<MTLDevice>, device);

  id<MTLLibrary> library =
    [mtlDevice newLibraryWithFile:[NSString stringWithUTF8String:path.c_str()]
                            error:&nsError];

  auto error = NicerNSError(CPP(ns::Error, nsError));
  if (error) {
    throw error;
  }

  return CPP(mtlpp::Library, library);
}

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

  return returnOrThrow<mtlpp::RenderPipelineState>([&](NSError*& error) {
    auto mtlDevice = OBJC(id<MTLDevice>, initializer.device);
    auto mtlDescriptor = OBJC(MTLRenderPipelineDescriptor*, descriptor);
    auto renderPipelineState =
      [mtlDevice newRenderPipelineStateWithDescriptor:mtlDescriptor
                                                error:&error];
    return CPP(mtlpp::RenderPipelineState, renderPipelineState);
  });
}

void
startMetalTrace(mtlpp::Device& device)
{
  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];

  ReleaseAssert(
    [captureManager supportsDestination:MTLCaptureDestinationGPUTraceDocument],
    "Capture to a GPU trace file is not supported. Does the Info.plist have "
    "MetalCaptureEnabled set to true?");

  // Place the trace right next to the binary file.
  auto output = getExecutablePath() + ".gputrace";
  NSString* nsOutput = [NSString stringWithUTF8String:output.c_str()];

  // Create the descriptor to describe how we are doing this capture.
  MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
  captureDescriptor.captureObject = OBJC(id<MTLDevice>, device);
  captureDescriptor.destination = MTLCaptureDestinationGPUTraceDocument;
  captureDescriptor.outputURL = [NSURL fileURLWithPath:nsOutput];

  throwIfError([&](NSError*& error) {
    [captureManager startCaptureWithDescriptor:captureDescriptor error:&error];
  });
}

void
saveMetalTraceToDisk()
{
  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
  [captureManager stopCapture];
  std::cout << "GPU trace completed: " << getExecutablePath() + ".gputrace"
            << std::endl;
}

AutoDraw::AutoDraw(mtlpp::Device& aDevice,
                   mtlpp::CommandQueue& aCommandQueue,
                   Tick& aTick)
  : mCommandQueue(aCommandQueue)
  , mTick(aTick)
  , mTickUniforms(BufferViewStruct<VizTickUniforms>(
      aDevice,
      mtlpp::ResourceOptions::CpuCacheModeWriteCombined,
      VizTickUniforms{
        .milliseconds = aTick.milliseconds,
        .seconds = aTick.seconds,
        .dt = aTick.dt,
        .tick = aTick.tick,
        .width = aTick.width,
        .height = aTick.height,
        .isMouseDown = aTick.isMouseDown,
      }))
{
  if (mTick.traceFrames > 0) {
    mTick.traceFramesUntilTick =
      // Subtract one, as this this current tick already counts to
      // the total frames.
      mTick.tick + mTick.traceFrames - 1;
    std::cout << "Starting to trace " << mTick.traceFrames << " frames.\n";
    mTick.traceFrames.store(0);

    startMetalTrace(aDevice);
  }
  mCommandBuffer = mCommandQueue.CommandBuffer();
}

AutoDraw::~AutoDraw()
{
  mCommandBuffer.Present(mTick.drawable);
  mCommandBuffer.Commit();
  if (mTick.traceFramesUntilTick == mTick.tick) {
    saveMetalTraceToDisk();
  }

  mCommandBuffer.WaitUntilCompleted();
}

/**
 * Call the clear functions to clear the next draw call. Typically this will
 * be used before adding any commands, so that the buffer will be cleared.
 */
void
AutoDraw::Clear()
{
  mClear = true;
}
void
AutoDraw::Clear(mtlpp::ClearColor&& aClearColor)
{
  mClear = true;
  mClearColor = aClearColor;
}
void
AutoDraw::Clear(double red, double green, double blue, double alpha)
{
  this->Clear(mtlpp::ClearColor(red, green, blue, alpha));
}
void
AutoDraw::Clear(double red, double green, double blue)
{
  this->Clear(mtlpp::ClearColor(red, green, blue, 1.0));
}

BufferViewStruct<VizTickUniforms>&
AutoDraw::GetTickUniforms()
{
  return mTickUniforms;
}

void
AutoDraw::DrawIndexed(DrawIndexedInitializer&& initializer)
{
  auto& [label,
         renderPipelineState,
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
    ReleaseAssert(buffer, "FragmentBuffers must exist.");
    renderCommandEncoder.SetFragmentBuffer(*buffer, 0, i);
  }

  // Handle the optional configs.
  if (cullMode) {
    renderCommandEncoder.SetCullMode(cullMode.value());
  }

  if (depthStencilState) {
    renderCommandEncoder.SetDepthStencilState(depthStencilState.value());
  }

  // Disable debug printing now that GPU tracing is available:
  if (false && mTick.tick == 0) {
    // clang-format off
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << label << std::endl;
    std::cout << "> DrawIndexed" << std::endl;
    std::cout << "> "; Debug(drawPrimitiveType);
    std::cout << "> Index Count: "; Debug(drawIndexCount);
    std::cout << "> Index Type: "; Debug(drawIndexType);
    // std::cout << "> Index Buffer: "; Debug(drawIndexBuffer, 1);
    // std::cout << "> Vertex: "; Debug(vertexBuffers, 1);
    // std::cout << "> Fragment: "; Debug(fragmentBuffers, 1);
    if (cullMode) {
      std::cout << "> "; Debug(cullMode.value());
    }
    if (depthStencilState) {
      std::cout << "> "; Debug(depthStencilState.value());
    }
    // clang-format on
  }

  renderCommandEncoder.DrawIndexed(drawPrimitiveType,
                                   drawIndexCount,
                                   drawIndexType,
                                   drawIndexBuffer,
                                   // offset
                                   0);

  renderCommandEncoder.EndEncoding();
}

void
AutoDraw::Draw(DrawInitializer&& initializer)
{
  auto& [label,
         renderPipelineState,
         drawPrimitiveType,
         vertexStart,
         vertexCount,
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

  ReleaseAssert(vertexBuffers.size() > 0,
                "There must be at least 1 vertex buffer for a draw call.");
  for (size_t i = 0; i < vertexBuffers.size(); i++) {
    auto& buffer = vertexBuffers[i];
    ReleaseAssert(buffer, "VertexBuffer must exist.");
    renderCommandEncoder.SetVertexBuffer(*buffer, 0, i);
  }

  ReleaseAssert(fragmentBuffers.size() > 0,
                "There must be at least 1 fragment buffer for a draw call.");
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

  // Disable for now
  if (false && mTick.tick == 0) {
    // clang-format off
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << label << std::endl;
    std::cout << "> Draw" << std::endl;
    std::cout << "> "; Debug(drawPrimitiveType);
    std::cout << "> Vertex Start: "; Debug(vertexStart);
    std::cout << "> Vertex Count: "; Debug(vertexCount);
    // std::cout << "> Vertex: "; Debug(vertexBuffers, 1);
    // std::cout << "> Fragment: "; Debug(fragmentBuffers, 1);
    if (cullMode) {
      std::cout << "> "; Debug(cullMode.value());
    }
    if (depthStencilState) {
      std::cout << "> "; Debug(depthStencilState.value());
    }
    // clang-format on
  }
  renderCommandEncoder.Draw(drawPrimitiveType, vertexStart, vertexCount);

  renderCommandEncoder.EndEncoding();
}

} // namespace viz

#undef CPP
#undef OBJC
