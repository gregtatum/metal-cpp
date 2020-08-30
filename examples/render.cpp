#include "render.h"
#include "app.h"
#include "metal.h"
#include "mtlpp/mtlpp.hpp"
#include <assert.h> // assert
#include <math.h>   // fmod
#include <string>   // std::string
/**
 * Let's make something pretty now.
 */

int
main()
{
  // clang-format off
  const float verteces[] = {
    0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
  };
  // clang-format on

  Device device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();

  auto [library, libraryError] = viz::CreateLibraryForExample(device);

  if (libraryError) {
    libraryError.print();
    return 1;
  }

  auto vert = library.NewFunction("vert");
  auto frag = library.NewFunction("frag");

  auto vertexBuffer =
    device.NewBuffer(verteces,
                     sizeof(verteces),
                     mtlpp::ResourceOptions::CpuCacheModeWriteCombined);

  auto uniformsBuffer = device.NewBuffer(
    sizeof(Uniforms), mtlpp::ResourceOptions::CpuCacheModeWriteCombined);
  Uniforms* uniforms = static_cast<Uniforms*>(uniformsBuffer.GetContents());
  *uniforms = Uniforms{};

  // This creates a render pipeline configuratin that can be used to create a
  // pipeline state object. Right now, we only care about setting up a vertex
  // and fragment shader.
  mtlpp::RenderPipelineDescriptor options;
  options.SetLabel("Basic rendering function");
  options.SetVertexFunction(library.NewFunction("vert"));
  options.SetFragmentFunction(library.NewFunction("frag"));
  options.GetColorAttachments()[0].SetPixelFormat(
    mtlpp::PixelFormat::BGRA8Unorm);

  auto pipeline = device.NewRenderPipelineState(options, nullptr);

  viz::TickFn tickFn = [&](viz::Tick& tick) -> void {
    uniforms->alpha = fmod(tick.time / 5000.0, 1.0);
    if (tick.mouse) {
      uniforms->center.x = (tick.mouse->x - tick.width / 2) / tick.width;
      uniforms->center.y = (tick.mouse->y - tick.height / 2) / tick.height;
    } else {
      uniforms->center.x = (tick.width / 2) / tick.width;
      uniforms->center.y = (tick.height / 2) / tick.height;
    }

    mtlpp::CommandBuffer commandBuffer = commandQueue.CommandBuffer();

    mtlpp::RenderCommandEncoder renderCommandEncoder =
      commandBuffer.RenderCommandEncoder(tick.renderPassDescriptor);
    renderCommandEncoder.SetRenderPipelineState(pipeline);
    renderCommandEncoder.SetVertexBuffer(vertexBuffer, 0, 0);
    renderCommandEncoder.SetVertexBuffer(uniformsBuffer, 0, 1);
    renderCommandEncoder.SetFragmentBuffer(uniformsBuffer, 0, 0);
    renderCommandEncoder.Draw(mtlpp::PrimitiveType::Triangle, 0, 3);
    renderCommandEncoder.EndEncoding();
    commandBuffer.Present(tick.drawable);

    commandBuffer.Commit();
    commandBuffer.WaitUntilCompleted();
  };

  viz::InitApp(device, &tickFn);
}
