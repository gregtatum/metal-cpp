#include "mtlpp/mtlpp.hpp"
#include "window.h"

const char[] source =
#include "./render-pipeline.metal"
  ;

int
main()
{
  const float verteces[] = {
    0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
  };

  auto device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();

  auto library = device.NewLibrary(source, mtlpp::CompileOptions(), nullptr);
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
  auto render = [](const Window& window) {
    //
  };
  Window win(device, &render, 320, 240);

  return 0;
}
