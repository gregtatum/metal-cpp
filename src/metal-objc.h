#pragma once
#include "mtlpp/mtlpp.hpp"
#include "viz-debug.h"
#include <exception> // std::exception
#include <span>      // std::span
#include <sstream>   // std::ostringstream
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
struct BufferViewStruct
{
  BufferViewStruct(Device& device, mtlpp::ResourceOptions options)
  {
    buffer = device.NewBuffer(sizeof(T), options);
    resourceOptions = options;
    data = static_cast<T*>(buffer.GetContents());
    *data = T{};
  }

  BufferViewStruct(Device& device, mtlpp::ResourceOptions options, T&& value)
  {
    buffer = device.NewBuffer(sizeof(T), options);
    resourceOptions = options;
    data = static_cast<T*>(buffer.GetContents());
    *data = value;
  }

  mtlpp::Buffer buffer;
  mtlpp::ResourceOptions resourceOptions;
  T* data;
};

template<typename T>
struct BufferViewList
{
  template<typename List>
  BufferViewList(Device& device, mtlpp::ResourceOptions options, List& list)
  {
    uint32_t bytes = sizeof(list[0]) * list.size();
    buffer = device.NewBuffer(&list[0], bytes, options);
    resourceOptions = options;
    data = std::span<T>{ static_cast<T*>(buffer.GetContents()), list.size() };
  }

  mtlpp::Buffer buffer;
  mtlpp::ResourceOptions resourceOptions;
  // The span points to the data in the buffer.
  std::span<T> data;
};

namespace traits {
struct bufferviewstruct
{};
struct bufferviewlist
{};
struct mtlpp_resourceoptions
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

} // viz
