#include <GLKit/GLKMath.h>
#include <assert.h> // assert
#include <cmath>
#include <math.h> // fmod
#include <span>
#include <string> // std::string
#include <vector>

// Load the main viz.h file in.
#include "viz.h"
// Now load other extraneous things.
#include "bunny-model.h"
#include "bunny.h"

using namespace viz;

struct Buffers
{
  mtlpp::Buffer positions;
  mtlpp::Buffer cells;
  mtlpp::Buffer normals;
  mtlpp::Buffer uniforms;
  uint32_t cellsSize;
};

Buffers
CreateBuffers(Device& device)
{
  auto cpuWrite = mtlpp::ResourceOptions::CpuCacheModeWriteCombined;
  std::span<float> positions = BUNNY_POSITIONS;
  std::span<int> cells = BUNNY_CELLS;
  auto normals = ComputeAngleNormalsPacked(cells, positions);

  return Buffers{
    .positions = CreateBufferFromList(device, cpuWrite, positions),
    .cells = CreateBufferFromList(device, cpuWrite, cells),
    .normals = CreateBufferFromList(device, cpuWrite, normals),
    .uniforms = CreateBufferFromStructType<Uniforms>(device, cpuWrite),
    .cellsSize = static_cast<uint32_t>(cells.size()),
  };
}

int
main()
{
  try {
    Device device = mtlpp::Device::CreateSystemDefaultDevice();
    auto commandQueue = device.NewCommandQueue();
    auto library = CreateLibraryForExample(device);
    auto buffers = CreateBuffers(device);

    Uniforms* uniforms = InitializeBuffer(buffers.uniforms, Uniforms{});

    // This creates a render pipeline configuration that can be used to create a
    // pipeline state object. Right now, we only care about setting up a vertex
    // and fragment shader.

    auto pipeline = viz::InitializeRenderPipeline(
      device,
      {
        .label = "Basic rendering function",
        .vertexFunction = library.NewFunction("vert"),
        .fragmentFunction = library.NewFunction("frag"),
        .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
        .colorAttachmentPixelFormats =
          std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
      });

    auto depthState = InitializeDepthStencil(
      device,
      {
        .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
        .depthWriteEnabled = true,
      });

    auto cameraHeight = 1;
    auto view = Matrix4::MakeLookAt(
      // clang-format off
        0, cameraHeight, -3, // eye
        0, cameraHeight, 0, // center
        0, 1, 0 // up
      // clang-format on
    );

    TickFn tickFn = [&](Tick& tick) -> void {
      auto projection = Matrix4::MakePerspective(
        M_PI * 0.3, tick.width / tick.height, 0.05, 100.0);

      auto model = Matrix4::MakeYRotation(tick.seconds) *
                   Matrix4::MakeScale(0.2, 0.2, 0.2);

      uniforms->matrices = GetModelMatrices(model, view, projection);

      Render({
        .commandQueue = commandQueue,
        .pipeline = pipeline,
        .renderPassDescriptor = tick.renderPassDescriptor,
        .drawable = tick.drawable,
        .primitiveType = mtlpp::PrimitiveType::Triangle,
        .indexCount = buffers.cellsSize,
        .indexType = mtlpp::IndexType::UInt32,
        .indexBuffer = buffers.cells,
        .cullMode = mtlpp::CullMode::Front,
        .depthStencilState = depthState,
        .vertexBuffers = std::vector({
          &buffers.positions,
          &buffers.normals,
          &buffers.uniforms,
        }),
        .fragmentBuffers = std::vector({ &buffers.uniforms }),
      });
    };

    InitApp(device, &tickFn);
  } catch (viz::Error& error) {
    error.print();
  }
}
