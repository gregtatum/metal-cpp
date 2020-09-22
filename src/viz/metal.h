#pragma once
#include "mtlpp/mtlpp.hpp"
#include "viz/assert.h"
#include "viz/cocoa-app.h"
#include "viz/debug.h"
#include "viz/metal.h"
#include "viz/shader-utils.h"
#include <exception>  // std::exception
#include <functional> // std::ref
#include <optional>
#include <span>    // std::span
#include <sstream> // std::ostringstream
#include <typeinfo>
#include <variant>
#include <vector> // std::vector

using namespace mtlpp;

namespace viz {

class Texture2D;
class BufferView;

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

using BufferVariant = std::variant<std::reference_wrapper<viz::BufferView>,
                                   std::reference_wrapper<viz::Texture2D>>;
using BufferRefs = std::vector<BufferVariant>;

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

class BufferView
{
public:
  // Move only, this is pointing to a resource.
  BufferView(BufferView&& other) = default;
  BufferView& operator=(BufferView&& other) = default;

  BufferView(Device& device, mtlpp::ResourceOptions options, uint32_t length)
    : buffer(device.NewBuffer(length, options))
    , resourceOptions(options)
  {}

  BufferView(Device& device,
             mtlpp::ResourceOptions options,
             const void* pointer,
             uint32_t length)
    : buffer(device.NewBuffer(pointer, length, options))
    , resourceOptions(options)
  {}

  BufferVariant Ref() { return BufferVariant{ std::ref(*this) }; }

  mtlpp::Buffer buffer;
  mtlpp::ResourceOptions resourceOptions;
};

template<typename T>
class BufferViewStruct : public BufferView
{
public:
  // Move only
  BufferViewStruct(BufferViewStruct&& other) = default;
  BufferViewStruct& operator=(BufferViewStruct&& other) = default;

  BufferViewStruct(Device& device, mtlpp::ResourceOptions options)
    : BufferView(device, options, sizeof(T))
    , data(static_cast<T*>(buffer.GetContents()))

  {
    *data = T{};
  }

  BufferViewStruct(T&& value, Device& device, mtlpp::ResourceOptions options)
    : BufferView(device, options, sizeof(T))
    , data(static_cast<T*>(buffer.GetContents()))
  {
    *data = value;
  }

  T* data;
};

template<typename T>
class BufferViewList : public BufferView
{
public:
  // Move only
  BufferViewList(BufferViewList&& other) = default;
  BufferViewList& operator=(BufferViewList&& other) = default;

  // Initialize the BufferViewList by pointing at some data.
  template<typename List>
  BufferViewList(List&& list, Device& device, mtlpp::ResourceOptions options)
    : BufferView(device, options, &list[0], sizeof(list[0]) * list.size())
    , data(std::span<T>{ static_cast<T*>(buffer.GetContents()), list.size() })
  {}

  // Initialize the BufferViewList through a callback.
  template<typename Fn>
  BufferViewList(Fn fn,
                 Device& device,
                 mtlpp::ResourceOptions options,
                 size_t size)
    : BufferView(device, options, sizeof(T) * size)
    , data(std::span<T>{ static_cast<T*>(buffer.GetContents()), size })
  {
    for (size_t i = 0; i < size; i++) {
      data[i] = fn(i);
    }
  }

  // The span points to the data in the buffer.
  std::span<T> data;
};

struct Texture2DInitializer
{
  mtlpp::Device& device;
  mtlpp::PixelFormat pixelFormat;
  uint32_t width;
  uint32_t height;
  bool mipmapped = false;

  std::optional<mtlpp::TextureType> textureType;
  std::optional<uint32_t> mipmapLevelCount;
  std::optional<uint32_t> sampleCount;
  std::optional<uint32_t> arrayLength;
  std::optional<mtlpp::ResourceOptions> resourceOptions;
  std::optional<mtlpp::CpuCacheMode> cpuCacheMode;
  std::optional<mtlpp::StorageMode> storageMode;
  std::optional<mtlpp::TextureUsage> usage;
};

/**
 * Initialize and draw to a texture.
 */
class Texture2D
{
  // Move only
  Texture2D(Texture2D&& other) = default;
  Texture2D& operator=(Texture2D&& other) = default;

public:
  explicit Texture2D(Texture2DInitializer&& initializer);

  // Set the data with a user-provided callback function.
  // The function signature is:
  //
  // (uint32_t x, uint32_t y) -> Format
  template<typename Format, typename Fn>
  void SetData(Fn fn);

  mtlpp::TextureDescriptor descriptor;
  mtlpp::Texture texture;
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
struct mtlpp_pixelformat
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
struct tag<mtlpp::PixelFormat>
{
  typedef mtlpp_pixelformat type;
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
  static void apply(mtlpp::IndexType const& state, size_t tabDepth = 0)
  {
    std::cout << "IndexType::";

    switch (state) {
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

template<>
struct debug<traits::mtlpp_pixelformat, mtlpp::PixelFormat>
{
  static void apply(mtlpp::PixelFormat const& state, size_t tabDepth = 0)
  {
    std::cout << "PixelFormat::";

    switch (state) {
      case PixelFormat::Invalid:
        std::cout << "Invalid";
        break;
      case PixelFormat::A8Unorm:
        std::cout << "A8Unorm";
        break;
      case PixelFormat::R8Unorm:
        std::cout << "R8Unorm";
        break;
      case PixelFormat::R8Unorm_sRGB:
        std::cout << "R8Unorm_sRGB";
        break;
      case PixelFormat::R8Snorm:
        std::cout << "R8Snorm";
        break;
      case PixelFormat::R8Uint:
        std::cout << "R8Uint";
        break;
      case PixelFormat::R8Sint:
        std::cout << "R8Sint";
        break;
      case PixelFormat::R16Unorm:
        std::cout << "R16Unorm";
        break;
      case PixelFormat::R16Snorm:
        std::cout << "R16Snorm";
        break;
      case PixelFormat::R16Uint:
        std::cout << "R16Uint";
        break;
      case PixelFormat::R16Sint:
        std::cout << "R16Sint";
        break;
      case PixelFormat::R16Float:
        std::cout << "R16Float";
        break;
      case PixelFormat::RG8Unorm:
        std::cout << "RG8Unorm";
        break;
      case PixelFormat::RG8Unorm_sRGB:
        std::cout << "RG8Unorm_sRGB";
        break;
      case PixelFormat::RG8Snorm:
        std::cout << "RG8Snorm";
        break;
      case PixelFormat::RG8Uint:
        std::cout << "RG8Uint";
        break;
      case PixelFormat::RG8Sint:
        std::cout << "RG8Sint";
        break;
      case PixelFormat::B5G6R5Unorm:
        std::cout << "B5G6R5Unorm";
        break;
      case PixelFormat::A1BGR5Unorm:
        std::cout << "A1BGR5Unorm";
        break;
      case PixelFormat::ABGR4Unorm:
        std::cout << "ABGR4Unorm";
        break;
      case PixelFormat::BGR5A1Unorm:
        std::cout << "BGR5A1Unorm";
        break;
      case PixelFormat::R32Uint:
        std::cout << "R32Uint";
        break;
      case PixelFormat::R32Sint:
        std::cout << "R32Sint";
        break;
      case PixelFormat::R32Float:
        std::cout << "R32Float";
        break;
      case PixelFormat::RG16Unorm:
        std::cout << "RG16Unorm";
        break;
      case PixelFormat::RG16Snorm:
        std::cout << "RG16Snorm";
        break;
      case PixelFormat::RG16Uint:
        std::cout << "RG16Uint";
        break;
      case PixelFormat::RG16Sint:
        std::cout << "RG16Sint";
        break;
      case PixelFormat::RG16Float:
        std::cout << "RG16Float";
        break;
      case PixelFormat::RGBA8Unorm:
        std::cout << "RGBA8Unorm";
        break;
      case PixelFormat::RGBA8Unorm_sRGB:
        std::cout << "RGBA8Unorm_sRGB";
        break;
      case PixelFormat::RGBA8Snorm:
        std::cout << "RGBA8Snorm";
        break;
      case PixelFormat::RGBA8Uint:
        std::cout << "RGBA8Uint";
        break;
      case PixelFormat::RGBA8Sint:
        std::cout << "RGBA8Sint";
        break;
      case PixelFormat::BGRA8Unorm:
        std::cout << "BGRA8Unorm";
        break;
      case PixelFormat::BGRA8Unorm_sRGB:
        std::cout << "BGRA8Unorm_sRGB";
        break;
      case PixelFormat::RGB10A2Unorm:
        std::cout << "RGB10A2Unorm";
        break;
      case PixelFormat::RGB10A2Uint:
        std::cout << "RGB10A2Uint";
        break;
      case PixelFormat::RG11B10Float:
        std::cout << "RG11B10Float";
        break;
      case PixelFormat::RGB9E5Float:
        std::cout << "RGB9E5Float";
        break;
      case PixelFormat::BGR10_XR:
        std::cout << "BGR10_XR";
        break;
      case PixelFormat::BGR10_XR_sRGB:
        std::cout << "BGR10_XR_sRGB";
        break;
      case PixelFormat::RG32Uint:
        std::cout << "RG32Uint";
        break;
      case PixelFormat::RG32Sint:
        std::cout << "RG32Sint";
        break;
      case PixelFormat::RG32Float:
        std::cout << "RG32Float";
        break;
      case PixelFormat::RGBA16Unorm:
        std::cout << "RGBA16Unorm";
        break;
      case PixelFormat::RGBA16Snorm:
        std::cout << "RGBA16Snorm";
        break;
      case PixelFormat::RGBA16Uint:
        std::cout << "RGBA16Uint";
        break;
      case PixelFormat::RGBA16Sint:
        std::cout << "RGBA16Sint";
        break;
      case PixelFormat::RGBA16Float:
        std::cout << "RGBA16Float";
        break;
      case PixelFormat::BGRA10_XR:
        std::cout << "BGRA10_XR";
        break;
      case PixelFormat::BGRA10_XR_sRGB:
        std::cout << "BGRA10_XR_sRGB";
        break;
      case PixelFormat::RGBA32Uint:
        std::cout << "RGBA32Uint";
        break;
      case PixelFormat::RGBA32Sint:
        std::cout << "RGBA32Sint";
        break;
      case PixelFormat::RGBA32Float:
        std::cout << "RGBA32Float";
        break;
      case PixelFormat::BC1_RGBA:
        std::cout << "BC1_RGBA";
        break;
      case PixelFormat::BC1_RGBA_sRGB:
        std::cout << "BC1_RGBA_sRGB";
        break;
      case PixelFormat::BC2_RGBA:
        std::cout << "BC2_RGBA";
        break;
      case PixelFormat::BC2_RGBA_sRGB:
        std::cout << "BC2_RGBA_sRGB";
        break;
      case PixelFormat::BC3_RGBA:
        std::cout << "BC3_RGBA";
        break;
      case PixelFormat::BC3_RGBA_sRGB:
        std::cout << "BC3_RGBA_sRGB";
        break;
      case PixelFormat::BC4_RUnorm:
        std::cout << "BC4_RUnorm";
        break;
      case PixelFormat::BC4_RSnorm:
        std::cout << "BC4_RSnorm";
        break;
      case PixelFormat::BC5_RGUnorm:
        std::cout << "BC5_RGUnorm";
        break;
      case PixelFormat::BC5_RGSnorm:
        std::cout << "BC5_RGSnorm";
        break;
      case PixelFormat::BC6H_RGBFloat:
        std::cout << "BC6H_RGBFloat";
        break;
      case PixelFormat::BC6H_RGBUfloat:
        std::cout << "BC6H_RGBUfloat";
        break;
      case PixelFormat::BC7_RGBAUnorm:
        std::cout << "BC7_RGBAUnorm";
        break;
      case PixelFormat::BC7_RGBAUnorm_sRGB:
        std::cout << "BC7_RGBAUnorm_sRGB";
        break;
      case PixelFormat::PVRTC_RGB_2BPP:
        std::cout << "PVRTC_RGB_2BPP";
        break;
      case PixelFormat::PVRTC_RGB_2BPP_sRGB:
        std::cout << "PVRTC_RGB_2BPP_sRGB";
        break;
      case PixelFormat::PVRTC_RGB_4BPP:
        std::cout << "PVRTC_RGB_4BPP";
        break;
      case PixelFormat::PVRTC_RGB_4BPP_sRGB:
        std::cout << "PVRTC_RGB_4BPP_sRGB";
        break;
      case PixelFormat::PVRTC_RGBA_2BPP:
        std::cout << "PVRTC_RGBA_2BPP";
        break;
      case PixelFormat::PVRTC_RGBA_2BPP_sRGB:
        std::cout << "PVRTC_RGBA_2BPP_sRGB";
        break;
      case PixelFormat::PVRTC_RGBA_4BPP:
        std::cout << "PVRTC_RGBA_4BPP";
        break;
      case PixelFormat::PVRTC_RGBA_4BPP_sRGB:
        std::cout << "PVRTC_RGBA_4BPP_sRGB";
        break;
      case PixelFormat::EAC_R11Unorm:
        std::cout << "EAC_R11Unorm";
        break;
      case PixelFormat::EAC_R11Snorm:
        std::cout << "EAC_R11Snorm";
        break;
      case PixelFormat::EAC_RG11Unorm:
        std::cout << "EAC_RG11Unorm";
        break;
      case PixelFormat::EAC_RG11Snorm:
        std::cout << "EAC_RG11Snorm";
        break;
      case PixelFormat::EAC_RGBA8:
        std::cout << "EAC_RGBA8";
        break;
      case PixelFormat::EAC_RGBA8_sRGB:
        std::cout << "EAC_RGBA8_sRGB";
        break;
      case PixelFormat::ETC2_RGB8:
        std::cout << "ETC2_RGB8";
        break;
      case PixelFormat::ETC2_RGB8_sRGB:
        std::cout << "ETC2_RGB8_sRGB";
        break;
      case PixelFormat::ETC2_RGB8A1:
        std::cout << "ETC2_RGB8A1";
        break;
      case PixelFormat::ETC2_RGB8A1_sRGB:
        std::cout << "ETC2_RGB8A1_sRGB";
        break;
      case PixelFormat::ASTC_4x4_sRGB:
        std::cout << "ASTC_4x4_sRGB";
        break;
      case PixelFormat::ASTC_5x4_sRGB:
        std::cout << "ASTC_5x4_sRGB";
        break;
      case PixelFormat::ASTC_5x5_sRGB:
        std::cout << "ASTC_5x5_sRGB";
        break;
      case PixelFormat::ASTC_6x5_sRGB:
        std::cout << "ASTC_6x5_sRGB";
        break;
      case PixelFormat::ASTC_6x6_sRGB:
        std::cout << "ASTC_6x6_sRGB";
        break;
      case PixelFormat::ASTC_8x5_sRGB:
        std::cout << "ASTC_8x5_sRGB";
        break;
      case PixelFormat::ASTC_8x6_sRGB:
        std::cout << "ASTC_8x6_sRGB";
        break;
      case PixelFormat::ASTC_8x8_sRGB:
        std::cout << "ASTC_8x8_sRGB";
        break;
      case PixelFormat::ASTC_10x5_sRGB:
        std::cout << "ASTC_10x5_sRGB";
        break;
      case PixelFormat::ASTC_10x6_sRGB:
        std::cout << "ASTC_10x6_sRGB";
        break;
      case PixelFormat::ASTC_10x8_sRGB:
        std::cout << "ASTC_10x8_sRGB";
        break;
      case PixelFormat::ASTC_10x10_sRGB:
        std::cout << "ASTC_10x10_sRGB";
        break;
      case PixelFormat::ASTC_12x10_sRGB:
        std::cout << "ASTC_12x10_sRGB";
        break;
      case PixelFormat::ASTC_12x12_sRGB:
        std::cout << "ASTC_12x12_sRGB";
        break;
      case PixelFormat::ASTC_4x4_LDR:
        std::cout << "ASTC_4x4_LDR";
        break;
      case PixelFormat::ASTC_5x4_LDR:
        std::cout << "ASTC_5x4_LDR";
        break;
      case PixelFormat::ASTC_5x5_LDR:
        std::cout << "ASTC_5x5_LDR";
        break;
      case PixelFormat::ASTC_6x5_LDR:
        std::cout << "ASTC_6x5_LDR";
        break;
      case PixelFormat::ASTC_6x6_LDR:
        std::cout << "ASTC_6x6_LDR";
        break;
      case PixelFormat::ASTC_8x5_LDR:
        std::cout << "ASTC_8x5_LDR";
        break;
      case PixelFormat::ASTC_8x6_LDR:
        std::cout << "ASTC_8x6_LDR";
        break;
      case PixelFormat::ASTC_8x8_LDR:
        std::cout << "ASTC_8x8_LDR";
        break;
      case PixelFormat::ASTC_10x5_LDR:
        std::cout << "ASTC_10x5_LDR";
        break;
      case PixelFormat::ASTC_10x6_LDR:
        std::cout << "ASTC_10x6_LDR";
        break;
      case PixelFormat::ASTC_10x8_LDR:
        std::cout << "ASTC_10x8_LDR";
        break;
      case PixelFormat::ASTC_10x10_LDR:
        std::cout << "ASTC_10x10_LDR";
        break;
      case PixelFormat::ASTC_12x10_LDR:
        std::cout << "ASTC_12x10_LDR";
        break;
      case PixelFormat::ASTC_12x12_LDR:
        std::cout << "ASTC_12x12_LDR";
        break;
      case PixelFormat::GBGR422:
        std::cout << "GBGR422";
        break;
      case PixelFormat::BGRG422:
        std::cout << "BGRG422";
        break;
      case PixelFormat::Depth16Unorm:
        std::cout << "Depth16Unorm";
        break;
      case PixelFormat::Depth32Float:
        std::cout << "Depth32Float";
        break;
      case PixelFormat::Stencil8:
        std::cout << "Stencil8";
        break;
      case PixelFormat::Depth24Unorm_Stencil8:
        std::cout << "Depth24Unorm_Stencil8";
        break;
      case PixelFormat::Depth32Float_Stencil8:
        std::cout << "Depth32Float_Stencil8";
        break;
      case PixelFormat::X32_Stencil8:
        std::cout << "X32_Stencil8";
        break;
      case PixelFormat::X24_Stencil8:
        std::cout << "X24_Stencil8";
        break;
      default:
        throw ErrorMessage("Unknown PixelFormat");
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
  BufferRefs vertexInputs;
  BufferRefs fragmentInputs;

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
  BufferRefs vertexInputs;
  BufferRefs fragmentInputs;

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
  // Not copyable or movable
  AutoDraw(const AutoDraw&) = delete;
  AutoDraw& operator=(const AutoDraw&) = delete;

  AutoDraw(mtlpp::Device& aDevice,
           mtlpp::CommandQueue& aCommandQueue,
           Tick& aTick);

  ~AutoDraw();

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
