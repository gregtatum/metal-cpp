#include "app.h"
#include "metal.h"
#include "mtlpp/mtlpp.hpp"
#include "termcolor.hpp"

const char source[] =
#include "./render-pipeline.metal"
  ;

void
print_error(const char* name, const char* error)
{
  std::cout << termcolor::cyan << name << termcolor::reset << std::endl;
  std::cout << error << std::endl;
}

int
main()
{
  const float verteces[] = {
    0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
  };

  auto device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();

  auto [library, libraryError] =
    viz::CreateLibrary(device, source, mtlpp::CompileOptions());

  if (libraryError) {
    libraryError.print();
    return 1;
  }

  auto vert = library.NewFunction("vert");
  auto frag = library.NewFunction("frag");

  auto vertexBuffer =
    device.NewBuffer(verteces,
                     sizeof(verteces),
                     mtlpp::ResourceOptions::CpuCacheModeDefaultCache);

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
    mtlpp::CommandBuffer commandBuffer = commandQueue.CommandBuffer();

    mtlpp::RenderCommandEncoder renderCommandEncoder =
      commandBuffer.RenderCommandEncoder(tick.renderPassDescriptor);
    renderCommandEncoder.SetRenderPipelineState(pipeline);
    renderCommandEncoder.SetVertexBuffer(vertexBuffer, 0, 0);
    renderCommandEncoder.Draw(mtlpp::PrimitiveType::Triangle, 0, 3);
    renderCommandEncoder.EndEncoding();
    commandBuffer.Present(tick.drawable);

    commandBuffer.Commit();
    commandBuffer.WaitUntilCompleted();
  };

  viz::InitApp(device, &tickFn);
}
