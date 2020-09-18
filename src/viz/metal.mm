#include <Foundation/NSError.h>
#include <Metal/MTLCaptureManager.h>
#include <Metal/MTLDevice.h>
#include <sstream> // std::ostringstream
// Make sure including metal.h is last, otherwise there is ambiguous
// resolution of some internal types.
#include "viz/assert.h"
#include "viz/metal.h"
#include "viz/utils.h" // getExecutablePath

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
    auto error = (__bridge NSError*)mError.GetPtr();
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
  NicerNSError error{ ns::Handle{ (__bridge void*)nsError } };
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
  NicerNSError error{ ns::Handle{ (__bridge void*)nsError } };
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
  // Error
  NSError* error = nil;

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithSource:[NSString stringWithUTF8String:source]
                 options:(__bridge MTLCompileOptions*)options.GetPtr()
                   error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   NicerNSError(ns::Handle{ (__bridge void*)error }));
};

std::pair<mtlpp::Library, NicerNSError>
CreateLibraryFromMetalLib(Device device, const char* metallib)
{
  // Error
  NSError* error = nil;

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:metallib]
                 error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   NicerNSError(ns::Handle{ (__bridge void*)error }));
}

mtlpp::Library
CreateLibraryForExample(Device device)
{
  // Error
  NSError* nsError = nil;

  auto path = getExecutablePath() + ".metallib";

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:path.c_str()]
                 error:&nsError];

  auto error = NicerNSError(ns::Handle{ (__bridge void*)nsError });
  if (error) {
    throw error;
  }

  return ns::Handle{ (__bridge void*)library };
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

  return initializer.device.NewRenderPipelineState(descriptor, nullptr);
}

void
captureMetalTrace(mtlpp::Device& device)
{

  auto bundle = [NSBundle mainBundle];

  NSLog(@"bundle executablePath: %@", [bundle executablePath]);
  NSLog(@"bundle bundleURL: %@", [bundle bundleURL]);
  NSLog(@"bundle bundlePath: %@", [bundle bundlePath]);
  NSLog(@"bundle bundleIdentifier: %@", [bundle bundleIdentifier]);
  NSLog(@"bundle infoDictionary: %@", [bundle infoDictionary]);

  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];

  ReleaseAssert(
    [captureManager supportsDestination:MTLCaptureDestinationGPUTraceDocument],
    "Capture to a GPU trace file is not supported");

  MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
  captureDescriptor.captureObject = (__bridge id<MTLDevice>)device.GetPtr();
  captureDescriptor.destination = MTLCaptureDestinationGPUTraceDocument;
  auto output = getExecutablePath() + ".gputrace";
  NSString* nsOutput = [NSString stringWithUTF8String:output.c_str()];
  captureDescriptor.outputURL = [NSURL fileURLWithPath:nsOutput];

  throwIfError([&](NSError*& error) {
    [captureManager startCaptureWithDescriptor:captureDescriptor error:&error];
  });
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
  if (mTick.tick == 0) {
    NSLog(@"Before capturing");
    captureMetalTrace(aDevice);
  }
  mCommandBuffer = mCommandQueue.CommandBuffer();
}

AutoDraw::~AutoDraw()
{
  mCommandBuffer.Present(mTick.drawable);
  mCommandBuffer.Commit();
  if (mTick.tick == 0) {
    MTLCaptureManager* captureManager =
      [MTLCaptureManager sharedCaptureManager];
    [captureManager stopCapture];
    NSLog(@"Done capturing");
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

  if (mTick.tick == 0) {
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

  if (mTick.tick == 0) {
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