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
#include "box.h"
#include "geo-box.h"
#include "viz-debug.h"

using namespace viz;

struct Buffers
{
  BufferViewList<Vector3> positions;
  BufferViewList<std::array<uint32_t, 3>> cells;
  BufferViewList<Vector3> normals;
  BufferViewStruct<Uniforms> uniforms;
  uint32_t cellsSize;
};

Buffers
CreateBuffers(Device& device)
{
  auto mesh = viz::generateBox({ 1, 1, 1 }, { 1, 1, 1 });
  auto cpuWrite = mtlpp::ResourceOptions::CpuCacheModeWriteCombined;
  Debug(mesh);

  return Buffers{
    .positions = BufferViewList<Vector3>(device, cpuWrite, mesh.positions),
    .cells =
      BufferViewList<std::array<uint32_t, 3>>(device, cpuWrite, mesh.cells),
    .normals = BufferViewList<Vector3>(device, cpuWrite, mesh.normals),
    .uniforms = BufferViewStruct<Uniforms>(device, cpuWrite),
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

  Uniforms* uniforms = buffers.uniforms.data;

  // This creates a render pipeline configuration that can be used to create a
  // pipeline state object. Right now, we only care about setting up a vertex
  // and fragment shader.
  auto pipeline = viz::InitializeRenderPipeline({
    .device = device,
    .label = "Basic rendering function",
    .vertexFunction = library.NewFunction("vert"),
    .fragmentFunction = library.NewFunction("frag"),
    .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
    .colorAttachmentPixelFormats =
      std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
  });

  auto depthState = InitializeDepthStencil({
    .device = device,
    .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
    .depthWriteEnabled = true,
  });

  auto cameraHeight = 0;
  auto view = Matrix4::MakeLookAt(
    // clang-format off
        0, cameraHeight, -3, // eye
        0, cameraHeight, 0, // center
        0, 1, 0 // up
    // clang-format on
  );

  Debug(buffers.positions);

  TickFn tickFn = [&](Tick& tick) -> void {
    auto projection = Matrix4::MakePerspective(
      M_PI * 0.3, tick.width / tick.height, 0.05, 100.0);

    auto model = Matrix4::MakeXRotation(tick.seconds * 0.2) *
                 Matrix4::MakeYRotation(tick.seconds) * Matrix4::MakeScale(1.0);

    uniforms->matrices = GetModelMatrices(model, view, projection);

    Render({
      // Required pieces.
      .commandQueue = commandQueue,
      .renderPipelineState = pipeline,
      .renderPassDescriptor = tick.renderPassDescriptor,
      .drawable = tick.drawable,

      // DrawIndexed options plus buffers.
      .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
      .drawIndexCount = buffers.cellsSize,
      .drawIndexType = mtlpp::IndexType::UInt32,
      .drawIndexBuffer = buffers.cells.buffer,
      .vertexBuffers = std::vector({
        &buffers.positions.buffer,
        &buffers.normals.buffer,
        &buffers.uniforms.buffer,
      }),
      .fragmentBuffers = std::vector({ &buffers.uniforms.buffer }),

      // General draw config
      .cullMode = mtlpp::CullMode::None,
      .depthStencilState = depthState,
    });
  };

  InitApp(device, &tickFn);
}

int
main()
{
  display_exceptions(run);
}
