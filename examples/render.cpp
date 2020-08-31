#include <GLKit/GLKMath.h>
#include <assert.h> // assert
#include <cmath>
#include <math.h> // fmod
#include <string> // std::string

#include "app.h"
#include "bunny.h"
#include "metal.h"
#include "mtlpp/mtlpp.hpp"
#include "render.h"

/**
 * Let's make something pretty now.
 */

simd::float4x4
ToSimd(const GLKMatrix4&& matrix)
{
  return simd::float4x4(
    simd::float4{ matrix.m[0], matrix.m[1], matrix.m[2], matrix.m[3] },
    simd::float4{ matrix.m[4], matrix.m[5], matrix.m[6], matrix.m[7] },
    simd::float4{ matrix.m[8], matrix.m[9], matrix.m[10], matrix.m[11] },
    simd::float4{ matrix.m[12], matrix.m[13], matrix.m[14], matrix.m[15] });
}

int
main()
{

  Device device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();

  auto [library, libraryError] = viz::CreateLibraryForExample(device);

  if (libraryError) {
    libraryError.print();
    return 1;
  }

  auto vert = library.NewFunction("vert");
  auto frag = library.NewFunction("frag");

  auto positionBuffer =
    device.NewBuffer(BUNNY_POSITIONS,
                     sizeof(BUNNY_POSITIONS),
                     mtlpp::ResourceOptions::CpuCacheModeWriteCombined);
  auto cellBuffer =
    device.NewBuffer(BUNNY_CELLS,
                     sizeof(BUNNY_CELLS),
                     mtlpp::ResourceOptions::CpuCacheModeWriteCombined);
  auto cellCount = sizeof(BUNNY_CELLS) / sizeof(BUNNY_CELLS[0]);
  auto cellOffset = 0;

  auto uniformsBuffer = device.NewBuffer(
    sizeof(Uniforms), mtlpp::ResourceOptions::CpuCacheModeWriteCombined);
  Uniforms* uniforms = static_cast<Uniforms*>(uniformsBuffer.GetContents());
  *uniforms = Uniforms{};

  // This creates a render pipeline configuration that can be used to create a
  // pipeline state object. Right now, we only care about setting up a vertex
  // and fragment shader.
  mtlpp::RenderPipelineDescriptor options;
  options.SetLabel("Basic rendering function");
  options.SetVertexFunction(library.NewFunction("vert"));
  options.SetFragmentFunction(library.NewFunction("frag"));
  // Create a depth attachment for the view, this will allow for depth testing
  // of the primitives.
  options.SetDepthAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float);
  options.GetColorAttachments()[0].SetPixelFormat(
    mtlpp::PixelFormat::BGRA8Unorm);

  auto pipeline = device.NewRenderPipelineState(options, nullptr);

  mtlpp::DepthStencilDescriptor depthDescriptor{};
  depthDescriptor.SetDepthCompareFunction(mtlpp::CompareFunction::LessEqual);
  depthDescriptor.SetDepthWriteEnabled(true);
  auto depthState = device.NewDepthStencilState(depthDescriptor);

  viz::TickFn tickFn = [&](viz::Tick& tick) -> void {
    uniforms->alpha = fmod(tick.seconds / 5.0, 1.0);
    if (tick.mouse) {
      uniforms->center.x = (tick.mouse->x - tick.width / 2) / tick.width;
      uniforms->center.y = (tick.mouse->y - tick.height / 2) / tick.height;
    } else {
      uniforms->center.x = (tick.width / 2) / tick.width;
      uniforms->center.y = (tick.height / 2) / tick.height;
    }

    auto cameraHeight = 1;

    uniforms->model =
      ToSimd(GLKMatrix4Multiply(GLKMatrix4MakeYRotation(tick.seconds),
                                GLKMatrix4MakeScale(0.2, 0.2, 0.2)));
    uniforms->view = ToSimd(GLKMatrix4MakeLookAt(
      // eye
      0,
      cameraHeight,
      -3,
      // center
      0,
      cameraHeight,
      0,
      // up
      0,
      1,
      0));
    uniforms->projection = ToSimd(GLKMatrix4MakePerspective(
      M_PI * 0.3, tick.width / tick.height, 0.05, 100.0));

    mtlpp::CommandBuffer commandBuffer = commandQueue.CommandBuffer();

    mtlpp::RenderCommandEncoder renderCommandEncoder =
      commandBuffer.RenderCommandEncoder(tick.renderPassDescriptor);
    renderCommandEncoder.SetRenderPipelineState(pipeline);
    renderCommandEncoder.SetCullMode(mtlpp::CullMode::Front);
    renderCommandEncoder.SetDepthStencilState(depthState);

    renderCommandEncoder.SetVertexBuffer(positionBuffer, 0, 0);
    renderCommandEncoder.SetVertexBuffer(uniformsBuffer, 0, 1);

    renderCommandEncoder.SetFragmentBuffer(uniformsBuffer, 0, 0);
    renderCommandEncoder.DrawIndexed(mtlpp::PrimitiveType::Triangle,
                                     cellCount,
                                     mtlpp::IndexType::UInt32,
                                     cellBuffer,
                                     cellOffset);
    renderCommandEncoder.EndEncoding();
    commandBuffer.Present(tick.drawable);

    commandBuffer.Commit();
    commandBuffer.WaitUntilCompleted();
  };

  viz::InitApp(device, &tickFn);
}
