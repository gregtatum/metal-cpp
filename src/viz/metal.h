#pragma once
#include "mtlpp/mtlpp.hpp"
#include "viz/assert.h"
#include "viz/cocoa-app.h"
#include "viz/debug.h"
#include "viz/metal.h"
#include "viz/shader-utils.h"
#include <exception> // std::exception
#include <optional>
#include <span>    // std::span
#include <sstream> // std::ostringstream
#include <typeinfo>
#include <vector> // std::vector

using namespace mtlpp;

namespace viz {

/**
 * This class encapsulates the underlying NSError object. I wasn't satisfied
 * with the way mtlpp was handling this.
 */
class NicerNSError : public std::exception
{
public:
  // Make sure and move the error, or else it will ge re-initialized.
  explicit NicerNSError(ns::Error&& error)
    : mError(error){};
  explicit operator bool() const;

  const char* what() const noexcept;

  ns::Error mError;
  // The message is lazily initialized, and is mutated during the const
  // "what()" method, hence the mutable term.
  mutable std::string mErrorMessage;
};

std::pair<mtlpp::Library, viz::NicerNSError>
CreateLibraryFromMetalLib(Device device, const char* metallib);

std::pair<mtlpp::Library, viz::NicerNSError>
CreateLibraryFromSource(Device device,
                        const char* source,
                        const CompileOptions& options);

/**
 * This function hooks up a metallib file to the current example. It assumes the
 * metallib was successfully compiled, and is located as a sibling in the same
 * directory as the binary. ".metallib" will be added to the binary name.
 *
 * e.g.
 * /path/to/bin/example
 * /path/to/bin/example.metallib
 */
mtlpp::Library
CreateLibraryForExample(Device device);

/**
 * This class is a discriminating union of anything that can be used as an input
 * to a fragment or vertex shader function.
 */
class ShaderInput
{
  enum Tag
  {
    Buffer,
    Texture
  };

public:
  ShaderInput(mtlpp::Buffer* buffer);
  ShaderInput(mtlpp::Texture* texture);
  void SetFragment(mtlpp::RenderCommandEncoder& renderCommandEncoder,
                   uint32_t index);
  void SetVertex(mtlpp::RenderCommandEncoder& renderCommandEncoder,
                 uint32_t index);

private:
  void* pointer;
  Tag tag;
};

/**
 * Creates a metal Buffer with a std::vector or std::span. It handles computing
 * the byte size.
 */
template<typename List>
mtlpp::Buffer
CreateBufferFromList(Device& device, mtlpp::ResourceOptions options, List& list)
{
  uint32_t bytes = sizeof(list[0]) * list.size();
  return device.NewBuffer(&list[0], bytes, options);
}

template<typename T>
class BufferViewStruct
{
public:
  BufferViewStruct(Device& device, mtlpp::ResourceOptions options)
    : buffer(device.NewBuffer(sizeof(T), options))
    , shaderInput(&buffer)
    , resourceOptions(options)
    , data(static_cast<T*>(buffer.GetContents()))

  {
    *data = T{};
  }

  BufferViewStruct(Device& device, mtlpp::ResourceOptions options, T&& value)
    : buffer(device.NewBuffer(sizeof(T), options))
    , shaderInput(&buffer)
    , resourceOptions(options)
    , data(static_cast<T*>(buffer.GetContents()))
  {
    *data = value;
  }

  mtlpp::Buffer buffer;
  mtlpp::ResourceOptions resourceOptions;
  T* data;
  ShaderInput shaderInput;
};

template<typename T>
class BufferViewList
{
public:
  BufferViewList(const BufferViewList&) = delete;

  // Initialize the BufferViewList by pointing at some data.
  template<typename List>
  BufferViewList(Device& device, mtlpp::ResourceOptions options, List&& list)
    : buffer(device.NewBuffer(&list[0], sizeof(list[0]) * list.size(), options))
    , shaderInput(&buffer)
    , resourceOptions(options)
    , data(std::span<T>{ static_cast<T*>(buffer.GetContents()), list.size() })
  {}

  // Initialize the BufferViewList through a callback.
  template<typename Fn>
  BufferViewList(Device& device,
                 mtlpp::ResourceOptions options,
                 size_t size,
                 Fn fn)
    : buffer(device.NewBuffer(sizeof(T) * size, options))
    , shaderInput(&buffer)
    , resourceOptions(options)
    , data(std::span<T>{ static_cast<T*>(buffer.GetContents()), size })
  {
    for (size_t i = 0; i < size; i++) {
      data[i] = fn(i);
    }
  }

  mtlpp::Buffer buffer;
  mtlpp::ResourceOptions resourceOptions;
  // The span points to the data in the buffer.
  std::span<T> data;
  ShaderInput shaderInput;
};

namespace traits {
struct bufferviewstruct
{};
struct bufferviewlist
{};
struct mtlpp_resourceoptions
{};
struct mtlpp_primitivetype
{};
struct mtlpp_cullmode
{};
struct mtlpp_depthstencilstate
{};
struct mtlpp_indextype
{};

template<typename T>
struct tag<BufferViewStruct<T>>
{
  typedef bufferviewstruct type;
  typedef T child;
};

template<typename T>
struct tag<BufferViewList<T>>
{
  typedef bufferviewlist type;
  typedef T child;
};

template<>
struct tag<mtlpp::ResourceOptions>
{
  typedef mtlpp_resourceoptions type;
  typedef empty child;
};

template<>
struct tag<mtlpp::PrimitiveType>
{
  typedef mtlpp_primitivetype type;
  typedef empty child;
};

template<>
struct tag<mtlpp::CullMode>
{
  typedef mtlpp_cullmode type;
  typedef empty child;
};

template<>
struct tag<mtlpp::DepthStencilState>
{
  typedef mtlpp_depthstencilstate type;
  typedef empty child;
};

template<>
struct tag<mtlpp::IndexType>
{
  typedef mtlpp_indextype type;
  typedef empty child;
};

}

namespace dispatch {

template<typename T>
struct debug<traits::bufferviewlist, BufferViewList<T>>
{
  static void apply(BufferViewList<T> const& bufferViewList,
                    size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "BufferViewList {\n";

    DebugIndent(tabDepth + 1);
    std::cout << "resourceOptions: ";
    ::viz::DebugWithoutNewline<mtlpp::ResourceOptions>(
      bufferViewList.resourceOptions, tabDepth + 1);
    std::cout << ",\n";

    DebugIndent(tabDepth + 1);
    std::cout << "data: ";
    ::viz::DebugWithoutNewline<std::span<T>>(bufferViewList.data, tabDepth + 1);
    std::cout << ",\n";

    DebugIndent(tabDepth);
    std::cout << "}";
  }
};

template<>
struct debug<traits::mtlpp_resourceoptions, mtlpp::ResourceOptions>
{
  static void apply(mtlpp::ResourceOptions const& options, size_t tabDepth = 0)
  {
    std::cout << "ResourceOptions::";

    DebugAssert(ResourceOptions::CpuCacheModeDefaultCache ==
                  ResourceOptions::StorageModeShared,
                "The default mode is StorageModeShared");

    switch (options) {
      // This is also CpuCacheModeDefaultCache, which I assume the default
      // is StorageModeShared.
      case ResourceOptions::StorageModeShared:
        std::cout << "StorageModeShared";
        break;
      case ResourceOptions::CpuCacheModeWriteCombined:
        std::cout << "CpuCacheModeWriteCombined";
        break;
        std::cout << "StorageModeShared";
        break;
      case ResourceOptions::StorageModeManaged:
        std::cout << "StorageModeManaged";
        break;
      case ResourceOptions::StorageModePrivate:
        std::cout << "StorageModePrivate";
        break;
      case ResourceOptions::StorageModeMemoryless:
        std::cout << "StorageModeMemoryless";
        break;
      case ResourceOptions::HazardTrackingModeUntracked:
        std::cout << "HazardTrackingModeUntracked";
        break;
      default:
        throw ErrorMessage("Unknown ResourceOption");
    }
  }
};

template<>
struct debug<traits::mtlpp_primitivetype, mtlpp::PrimitiveType>
{
  static void apply(mtlpp::PrimitiveType const& options, size_t tabDepth = 0)
  {
    std::cout << "PrimitiveType::";

    switch (options) {
      case PrimitiveType::Point:
        std::cout << "Point";
        break;
      case PrimitiveType::Line:
        std::cout << "Line";
        break;
      case PrimitiveType::LineStrip:
        std::cout << "LineStrip";
        break;
      case PrimitiveType::Triangle:
        std::cout << "Triangle";
        break;
      case PrimitiveType::TriangleStrip:
        std::cout << "TriangleStrip";
        break;
      default:
        throw ErrorMessage("Unknown PrimitiveType");
    }
  }
};

template<>
struct debug<traits::mtlpp_cullmode, mtlpp::CullMode>
{
  static void apply(mtlpp::CullMode const& cullMode, size_t tabDepth = 0)
  {
    std::cout << "CullMode::";

    switch (cullMode) {
      case CullMode::None:
        std::cout << "None";
        break;
      case CullMode::Front:
        std::cout << "Front";
        break;
      case CullMode::Back:
        std::cout << "Back";
        break;
      default:
        throw ErrorMessage("Unknown CullMode");
    }
  }
};

template<>
struct debug<traits::mtlpp_indextype, mtlpp::IndexType>
{
  static void apply(mtlpp::IndexType const& IndexType, size_t tabDepth = 0)
  {
    std::cout << "IndexType::";

    switch (IndexType) {
      case IndexType::UInt16:
        std::cout << "UInt16";
        break;
      case IndexType::UInt32:
        std::cout << "UInt32";
        break;
      default:
        throw ErrorMessage("Unknown IndexType");
    }
  }
};

template<>
struct debug<traits::mtlpp_depthstencilstate, mtlpp::DepthStencilState>
{
  static void apply(mtlpp::DepthStencilState const& state, size_t tabDepth = 0)
  {
    std::cout << "DepthStencilState { ";
    if (state.GetLabel()) {
      std::cout << state.GetLabel().GetCStr() << " }";
    } else {
      std::cout << "}";
    }
  }
};

} // namespace dispatch

template<typename T>
std::pair<mtlpp::Buffer, T*>
CreateBufferFromStruct(Device& device,
                       mtlpp::ResourceOptions options,
                       T&& value)
{

  mtlpp::Buffer buffer = device.NewBuffer(sizeof(T), options);
  T* pointer = static_cast<T*>(buffer.GetContents());
  *pointer = value;

  return { buffer, pointer };
}

template<typename T>
mtlpp::Buffer
CreateBufferFromStructType(Device& device, mtlpp::ResourceOptions options)
{

  return device.NewBuffer(sizeof(T), options);
}

template<typename T>
T*
InitializeBuffer(mtlpp::Buffer& buffer, T&& value)
{
  T* pointer = static_cast<T*>(buffer.GetContents());
  *pointer = value;
  return pointer;
}

struct RenderPipelineInitializer
{
  mtlpp::Device& device;
  mtlpp::Library& library;
  std::optional<const char*> label = std::nullopt;
  std::optional<const std::string> vertexFunction = std::nullopt;
  std::optional<const std::string> fragmentFunction = std::nullopt;
  std::optional<const mtlpp::PixelFormat> depthAttachmentPixelFormat =
    std::nullopt;
  std::vector<mtlpp::PixelFormat> colorAttachmentPixelFormats = {};
  // TODO - Add more options as they are needed
};

/**
 * This function provides a single line instantiation of a render pipeline state
 * object.
 */
mtlpp::RenderPipelineState
InitializeRenderPipeline(RenderPipelineInitializer&& initializer);

struct DepthStencilInitializer
{
  mtlpp::Device& device;
  std::optional<mtlpp::CompareFunction> depthCompareFunction = std::nullopt;
  std::optional<bool> depthWriteEnabled = std::nullopt;
};

mtlpp::DepthStencilState
InitializeDepthStencil(DepthStencilInitializer&& initializer);

struct DrawIndexedInitializer
{
  const char* label;
  mtlpp::RenderPipelineState renderPipelineState;
  mtlpp::PrimitiveType primitiveType;
  uint32_t indexCount;
  mtlpp::IndexType indexType;
  const mtlpp::Buffer& indexBuffer;
  std::vector<ShaderInput> vertexInputs;
  std::vector<ShaderInput> fragmentInputs;

  // Optional config:
  std::optional<uint32_t> instanceCount;
  std::optional<mtlpp::CullMode> cullMode;
  std::optional<mtlpp::DepthStencilState> depthStencilState;
};

struct DrawInitializer
{
  const char* label;
  mtlpp::RenderPipelineState renderPipelineState;
  mtlpp::PrimitiveType primitiveType;
  uint32_t vertexStart;
  uint32_t vertexCount;
  std::vector<ShaderInput> vertexInputs;
  std::vector<ShaderInput> fragmentInputs;

  // Optional config:
  std::optional<mtlpp::CullMode> cullMode;
  std::optional<mtlpp::DepthStencilState> depthStencilState;
};

// struct RenderInstancedInitializer
// {
//   // Required dependencies.
//   mtlpp::CommandQueue& commandQueue;
//   mtlpp::RenderPipelineState& renderPipelineState;
//   mtlpp::RenderPassDescriptor& renderPassDescriptor;

//   // DrawIndexed options plus buffers.
//   mtlpp::PrimitiveType primitiveType;
//   uint32_t indexCount;
//   mtlpp::IndexType indexType;
//   const mtlpp::Buffer& indexBuffer;
//   std::optional<std::vector<mtlpp::Buffer*>> vertexBuffers;
//   std::optional<std::vector<mtlpp::Buffer*>> fragmentBuffers;

//   // General draw config
//   std::optional<mtlpp::CullMode> cullMode;
//   std::optional<mtlpp::DepthStencilState> depthStencilState;
// };

// void
// RenderInstanced(RenderInstancedInitializer&& initializer)
// {

//   renderCommandEncoder.DrawIndexed(
//     PrimitiveType primitiveType,
//     uint32_t indexCount,
//     IndexType indexType,
//     const Buffer& indexBuffer,
//     uint32_t indexBufferOffset,
//     uint32_t instanceCount
//   )
// }

/**
 * Automatically handles a draw pass based based on when the AutoDraw object is
 * in scope.
 */
class AutoDraw
{
public:
  AutoDraw(mtlpp::Device& aDevice,
           mtlpp::CommandQueue& aCommandQueue,
           Tick& aTick);

  ~AutoDraw();

  // Delete copy and move.
  AutoDraw(const AutoDraw&) = delete;
  AutoDraw& operator=(const AutoDraw) = delete;

  /**
   * Call the clear functions to clear the next draw call. Typically this will
   * be used before adding any commands, so that the buffer will be cleared.
   */
  void Clear();
  void Clear(mtlpp::ClearColor&& aClearColor);
  void Clear(double red, double green, double blue, double alpha);
  void Clear(double red, double green, double blue);
  BufferViewStruct<VizTickUniforms>& GetTickUniforms();
  void DrawIndexed(DrawIndexedInitializer&& initializer);
  void Draw(DrawInitializer&& initializer);

  // Dependencies.
  BufferViewStruct<VizTickUniforms> mTickUniforms;
  mtlpp::CommandQueue& mCommandQueue;
  Tick& mTick;

  // Owned data.
  mtlpp::CommandBuffer mCommandBuffer;
  bool mClear = false;
  std::optional<mtlpp::ClearColor> mClearColor;
};
} // viz
