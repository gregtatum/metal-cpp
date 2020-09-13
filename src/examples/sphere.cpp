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
#include "./sphere.h"
#include "viz/geo/icosphere.h"

using namespace viz;

struct Scene
{
  BufferViewList<Vector3> positions;
  BufferViewList<std::array<uint32_t, 3>> cells;
  BufferViewList<Vector3> normals;
  std::vector<BufferViewStruct<Uniforms>> smallSpheres;
  BufferViewStruct<Uniforms> bigSphere;
  uint32_t cellsSize;
  mtlpp::DepthStencilState depthState;
  // Shared POD:
  Matrix4 view = {};
  Matrix4 projection = {};
};

size_t SMALL_SPHERE_COUNT = 20;
float SMALL_SPHERE_RADIUS_MIN = 0.05f;
float SMALL_SPHERE_RADIUS_MAX = 0.1f;
float BIG_SPHERE_RADIUS = 0.5f;

Scene
CreateBuffers(Device& device)
{
  auto mesh = viz::generateIcosphere({ .subdivisions = 2, .radius = 1.0f });
  auto cpuWrite = mtlpp::ResourceOptions::CpuCacheModeWriteCombined;

  // Initialize the small spheres.
  std::vector<BufferViewStruct<Uniforms>> smallSpheres{};
  for (size_t i = 0; i < SMALL_SPHERE_COUNT; i++) {
    auto smallSphere = BufferViewStruct<Uniforms>(device, cpuWrite);
    smallSphere.data->position =
      RandomSpherical({ .radius = BIG_SPHERE_RADIUS });
    smallSphere.data->radius =
      Random(SMALL_SPHERE_RADIUS_MIN, SMALL_SPHERE_RADIUS_MAX);
    smallSpheres.push_back(smallSphere);
  }

  // Initialize the big sphere.
  auto bigSphere = BufferViewStruct<Uniforms>(device, cpuWrite);
  bigSphere.data->position = { 0.0f, 0.0f, 0.0f };
  bigSphere.data->radius = BIG_SPHERE_RADIUS;

  return Scene{
    .positions = BufferViewList<Vector3>(device, cpuWrite, mesh.positions),
    .cells =
      BufferViewList<std::array<uint32_t, 3>>(device, cpuWrite, mesh.cells),
    .normals = BufferViewList<Vector3>(device, cpuWrite, mesh.normals),
    .smallSpheres = smallSpheres,
    .bigSphere = bigSphere,
    .cellsSize = static_cast<uint32_t>(mesh.cells.size() * 3),
    .depthState = InitializeDepthStencil({
      .device = device,
      .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
      .depthWriteEnabled = true,
    })
  };
}

void
DrawLittleSpheres(AutoDraw& draw, Tick& tick, Scene& scene)
{
  for (auto& smallSphere : scene.smallSpheres) {
    auto& uniforms = smallSphere.data;

    auto model = Matrix4::MakeTranslation(uniforms->position) *
                 Matrix4::MakeScale(uniforms->radius);

    uniforms->matrices = GetModelMatrices(model, scene.view, scene.projection);
    uniforms->seconds = tick.seconds;

    draw.Render({
      .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
      .drawIndexCount = scene.cellsSize,
      .drawIndexType = mtlpp::IndexType::UInt32,
      .drawIndexBuffer = scene.cells.buffer,
      .vertexBuffers = std::vector({
        &scene.positions.buffer,
        &scene.normals.buffer,
        &smallSphere.buffer,
      }),
      .fragmentBuffers = std::vector({ &smallSphere.buffer }),

      // General draw config
      .cullMode = mtlpp::CullMode::None,
      .depthStencilState = scene.depthState,
    });
  }
}

void
DrawBigSphere(AutoDraw& draw, Tick& tick, Scene& scene)
{
  auto& uniforms = scene.bigSphere.data;

  auto model = Matrix4::MakeTranslation(uniforms->position) *
               Matrix4::MakeScale(uniforms->radius);

  uniforms->matrices = GetModelMatrices(model, scene.view, scene.projection);
  uniforms->seconds = tick.seconds;

  draw.Render({
    .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
    .drawIndexCount = scene.cellsSize,
    .drawIndexType = mtlpp::IndexType::UInt32,
    .drawIndexBuffer = scene.cells.buffer,
    .vertexBuffers = std::vector({
      &scene.positions.buffer,
      &scene.normals.buffer,
      &scene.bigSphere.buffer,
    }),
    .fragmentBuffers = std::vector({ &scene.bigSphere.buffer }),

    // General draw config
    .cullMode = mtlpp::CullMode::None,
    .depthStencilState = scene.depthState,
  });
}

void
run()
{
  Device device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();
  auto library = CreateLibraryForExample(device);
  auto scene = CreateBuffers(device);

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

  auto cameraHeight = 1.5;
  scene.view = Matrix4::MakeLookAt(
    // clang-format off
        0, cameraHeight, -3, // eye
        0, 0, -1.0, // center
        0, 1, 0 // up
    // clang-format on
  );

  TickFn tickFn = [&](Tick& tick) -> void {
    AutoDraw draw{ commandQueue, pipeline, tick };

    draw.Clear(0.0, 0.05, 0.1);

    scene.projection = Matrix4::MakePerspective(
      M_PI * 0.3, tick.width / tick.height, 0.05, 100.0);

    DrawLittleSpheres(draw, tick, scene);
    DrawBigSphere(draw, tick, scene);
  };

  InitApp(device, &tickFn);
}

int
main()
{
  display_exceptions(run);
}
