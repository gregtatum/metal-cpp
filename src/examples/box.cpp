#include <GLKit/GLKMath.h>
#include <assert.h> // assert
#include <cmath>
#include <iostream> // std::cout
#include <math.h>   // fmod
#include <span>
#include <string> // std::string
#include <vector>

// Load the main viz.h file in.
#include "viz.h"
// Now load other extraneous things.
#include "./box.h"
#include "viz/debug.h"
#include "viz/geo/box.h"

using namespace viz;

struct Buffers
{
  BufferViewList<Vector3> positions;
  BufferViewList<std::array<uint32_t, 3>> cells;
  BufferViewList<Vector3> normals;
  std::vector<BufferViewStruct<Uniforms>> uniformsList;
  uint32_t cellsSize;
};

size_t COUNT_SIDE = 50;
size_t COUNT = COUNT_SIDE * COUNT_SIDE;

Buffers
CreateBuffers(Device& device)
{
  auto mesh = viz::generateBox({ 1.0, 10.0, 1.0 }, { 1, 1, 1 });
  auto cpuWrite = mtlpp::ResourceOptions::CpuCacheModeWriteCombined;

  std::vector<BufferViewStruct<Uniforms>> uniformsList{};
  for (size_t i = 0; i < COUNT; i++) {
    uniformsList.push_back(BufferViewStruct<Uniforms>(device, cpuWrite));
  }

  return Buffers{
    .positions = BufferViewList<Vector3>(device, cpuWrite, mesh.positions),
    .cells =
      BufferViewList<std::array<uint32_t, 3>>(device, cpuWrite, mesh.cells),
    .normals = BufferViewList<Vector3>(device, cpuWrite, mesh.normals),
    .uniformsList = uniformsList,
    .cellsSize = static_cast<uint32_t>(mesh.cells.size() * 3),
  };
}

void
run()
{
  Device device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();
  auto library = CreateLibraryForExample(device);
  auto buffers = CreateBuffers(device);

  // This creates a render pipeline configuration that can be used to create a
  // pipeline state object. Right now, we only care about setting up a vertex
  // and fragment shader.
  auto pipeline = viz::InitializeRenderPipeline({
    .device = device,
    .library = library,
    .label = "Basic rendering function",
    .vertexFunction = "vert",
    .fragmentFunction = "frag",
    .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
    .colorAttachmentPixelFormats =
      std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
  });

  auto depthState = InitializeDepthStencil({
    .device = device,
    .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
    .depthWriteEnabled = true,
  });

  auto cameraHeight = 1.5;
  auto view = Matrix4::MakeLookAt(
    // clang-format off
        0, cameraHeight, -3, // eye
        0, 0, -1.0, // center
        0, 1, 0 // up
    // clang-format on
  );

  TickFn tickFn = [&](Tick& tick) -> void {
    AutoDraw draw{ commandQueue, pipeline, tick };

    draw.Clear(0.0, 0.05, 0.1);

    auto projection = Matrix4::MakePerspective(
      M_PI * 0.3, tick.width / tick.height, 0.05, 100.0);

    float count = static_cast<float>(buffers.uniformsList.size());
    float extent = 2.0f;

    for (size_t i = 0; i < COUNT_SIDE; i++) {
      float ui = static_cast<float>(i) / static_cast<float>(COUNT_SIDE);
      for (size_t j = 0; j < COUNT_SIDE; j++) {
        float uj = static_cast<float>(j) / static_cast<float>(COUNT_SIDE);

        auto& uniforms = buffers.uniformsList[i + j * COUNT_SIDE];

        float x = std::lerp(-extent, extent, ui);
        float y = 0.0f;
        float z = std::lerp(-extent, extent, uj);

        auto model = Matrix4::MakeTranslation(x, y, z) *
                     Matrix4::MakeScale(1.0f / COUNT_SIDE);

        uniforms.data->matrices = GetModelMatrices(model, view, projection);
        uniforms.data->position = { x, y, z };
        uniforms.data->seconds = tick.seconds;

        draw.DrawIndexed({
          .label = "Box",
          // DrawIndexed options plus buffers.
          .primitiveType = mtlpp::PrimitiveType::Triangle,
          .indexCount = buffers.cellsSize,
          .indexType = mtlpp::IndexType::UInt32,
          .indexBuffer = buffers.cells.buffer,
          .vertexInputs = std::vector({
            buffers.positions.Ref(),
            buffers.normals.Ref(),
            uniforms.Ref(),
          }),
          .fragmentInputs = std::vector({ uniforms.Ref() }),

          // General draw config
          .cullMode = mtlpp::CullMode::None,
          .depthStencilState = depthState,
        });
      }
    }
  };

  InitApp(device, &tickFn);
}

int
main()
{
  display_exceptions(run);
}
