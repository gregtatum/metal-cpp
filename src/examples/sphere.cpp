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
#include "viz/draw/big-triangle.h"
#include "viz/geo/icosphere.h"

using namespace viz;

struct Scene
{
  BufferViewList<Vector3> positions;
  BufferViewList<std::array<uint32_t, 3>> cells;
  BufferViewList<Vector3> normals;
  BufferViewStruct<SceneUniforms> sceneUniforms;

  BufferViewList<ModelUniforms> smallSphereUniforms;
  mtlpp::RenderPipelineState smallSpherePipeline;

  BufferViewStruct<ModelUniforms> bigSphereUniforms;
  mtlpp::RenderPipelineState bigSpherePipeline;

  BigTriangle background;

  uint32_t cellsSize;
  mtlpp::DepthStencilState writeDepth;
  mtlpp::DepthStencilState ignoreDepth;
  // Shared POD:
  Matrix4 view = {};
  Matrix4 projection = {};
};

size_t SMALL_SPHERE_COUNT = 75;
float SMALL_SPHERE_RADIUS_MAX = 0.15f;
float SMALL_SPHERE_RADIUS_MIN = 0.02f;
float SMALL_SPHERE_BRIGHTNESS_MIN = 0.7f;
float SMALL_SPHERE_BRIGHTNESS_MAX = 0.8f;
float BIG_SPHERE_RADIUS = 0.9f;
float SPHERE_ROTATE_X = -0.01f;
float SPHERE_ROTATE_Y = 0.03f;

Scene
CreateScene(Device& device)
{
  auto mesh = viz::generateIcosphere({ .subdivisions = 3, .radius = 1.0f });
  auto cpuWrite = mtlpp::ResourceOptions::CpuCacheModeWriteCombined;
  auto library = CreateLibraryForExample(device);

  // Initialize the small spheres.
  auto initializeSmallSpheres = [&](size_t i) -> ModelUniforms {
    return {
      .position = RandomSpherical({ .radius = BIG_SPHERE_RADIUS }),
      .radius = RandomPow(SMALL_SPHERE_RADIUS_MIN, SMALL_SPHERE_RADIUS_MAX, 3),
    };
  };

  // Initialize the big sphere.
  auto bigSphereUniforms = BufferViewStruct<ModelUniforms>(device, cpuWrite);
  bigSphereUniforms.data->position = { 0.0f, 0.0f, 0.0f };
  bigSphereUniforms.data->radius = BIG_SPHERE_RADIUS;

  return Scene{
    .positions = BufferViewList<Vector3>(device, cpuWrite, mesh.positions),
    .cells =
      BufferViewList<std::array<uint32_t, 3>>(device, cpuWrite, mesh.cells),
    .normals = BufferViewList<Vector3>(device, cpuWrite, mesh.normals),
    .sceneUniforms = BufferViewStruct<SceneUniforms>(device, cpuWrite),

    .smallSphereUniforms = BufferViewList<ModelUniforms>(
      device, cpuWrite, SMALL_SPHERE_COUNT, initializeSmallSpheres),
    .smallSpherePipeline = viz::InitializeRenderPipeline({
      .device = device,
      .label = "Small Sphere Pipeline",
      .vertexFunction = library.NewFunction("smallSphereVert"),
      .fragmentFunction = library.NewFunction("smallSphereFrag"),
      .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
      .colorAttachmentPixelFormats =
        std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
    }),

    .bigSphereUniforms = bigSphereUniforms,
    .bigSpherePipeline = viz::InitializeRenderPipeline({
      .device = device,
      .label = "Big Sphere Pipeline",
      .vertexFunction = library.NewFunction("bigSphereVert"),
      .fragmentFunction = library.NewFunction("bigSphereFrag"),
      .depthAttachmentPixelFormat = mtlpp::PixelFormat::Depth32Float,
      .colorAttachmentPixelFormats =
        std::vector{ mtlpp::PixelFormat::BGRA8Unorm },
    }),

    .background =
      BigTriangle{
        "Background", device, library, library.NewFunction("background") },

    .cellsSize = static_cast<uint32_t>(mesh.cells.size() * 3),
    .writeDepth = InitializeDepthStencil({
      .device = device,
      .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
      .depthWriteEnabled = true,
    }),
    .ignoreDepth = InitializeDepthStencil({
      .device = device,
      .depthCompareFunction = mtlpp::CompareFunction::LessEqual,
      .depthWriteEnabled = false,
    })
  };
}

void
DrawSmallSpheres(AutoDraw& draw, Tick& tick, Scene& scene)
{
  for (auto& uniforms : scene.smallSphereUniforms.data) {

    auto model = Matrix4::MakeYRotation(tick.seconds * SPHERE_ROTATE_Y) *
                 Matrix4::MakeXRotation(tick.seconds * SPHERE_ROTATE_X) *
                 Matrix4::MakeTranslation(uniforms.position) *
                 Matrix4::MakeScale(uniforms.radius);

    uniforms.matrices = GetModelMatrices(model, scene.view, scene.projection);
    uniforms.brightness =
      Random(SMALL_SPHERE_BRIGHTNESS_MIN, SMALL_SPHERE_BRIGHTNESS_MAX);
  }

  draw.DrawIndexed({
    .label = "DrawSmallSpheres",
    .renderPipelineState = scene.smallSpherePipeline,
    .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
    .drawIndexCount = scene.cellsSize,
    .drawIndexType = mtlpp::IndexType::UInt32,
    .drawIndexBuffer = scene.cells.buffer,
    .vertexBuffers = std::vector({
      &scene.positions.buffer,
      &scene.normals.buffer,
      &scene.smallSphereUniforms.buffer,
    }),
    .fragmentBuffers = std::vector({ &scene.smallSphereUniforms.buffer }),

    // Optional values.
    .instanceCount = SMALL_SPHERE_COUNT,
    .cullMode = mtlpp::CullMode::Front,
    .depthStencilState = scene.writeDepth,
  });
}

void
DrawBigSphere(AutoDraw& draw, Tick& tick, Scene& scene)
{
  auto& uniforms = scene.bigSphereUniforms.data;

  auto model = Matrix4::MakeTranslation(uniforms->position) *
               Matrix4::MakeScale(uniforms->radius);

  scene.bigSphereUniforms.data->matrices =
    GetModelMatrices(model, scene.view, scene.projection);

  draw.DrawIndexed({
    .label = "DrawBigSphere",
    .renderPipelineState = scene.bigSpherePipeline,
    .drawPrimitiveType = mtlpp::PrimitiveType::Triangle,
    .drawIndexCount = scene.cellsSize,
    .drawIndexType = mtlpp::IndexType::UInt32,
    .drawIndexBuffer = scene.cells.buffer,
    .vertexBuffers = std::vector({
      &scene.positions.buffer,
      &scene.normals.buffer,
      &scene.sceneUniforms.buffer,
      &scene.bigSphereUniforms.buffer,
    }),
    .fragmentBuffers = std::vector(
      { &scene.sceneUniforms.buffer, &scene.bigSphereUniforms.buffer }),

    // General draw config
    .cullMode = mtlpp::CullMode::Front,
    .depthStencilState = scene.writeDepth,
  });
}

void
run()
{
  Device device = mtlpp::Device::CreateSystemDefaultDevice();
  auto commandQueue = device.NewCommandQueue();
  auto scene = CreateScene(device);

  auto cameraHeight = 0;
  scene.view = Matrix4::MakeLookAt(
    // clang-format off
        0, cameraHeight, -3, // eye
        0, 0, -1.0, // center
        0, 1, 0 // up
    // clang-format on
  );

  TickFn tickFn = [&](Tick& tick) -> void {
    AutoDraw draw{ device, commandQueue, tick };

    draw.Clear(0.0f, 0.0f, 0.0f);

    scene.sceneUniforms.data->seconds = tick.seconds;

    scene.projection = Matrix4::MakePerspective(
      M_PI * 0.3, tick.width / tick.height, 0.05, 100.0);

    DrawBigSphere(draw, tick, scene);
    DrawSmallSpheres(draw, tick, scene);
    scene.background.Draw(draw);
  };

  InitApp(device, &tickFn);
}

int
main()
{
  display_exceptions(run);
}
