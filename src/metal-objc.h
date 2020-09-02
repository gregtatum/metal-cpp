#pragma once
#include "mtlpp/mtlpp.hpp"
#include <span>
#include <vector>
using namespace mtlpp;

namespace viz {

/**
 * This class encapsulates the underlying NSError object. I wasn't satisfied
 * with the way mtlpp was handling this.
 */
class Error
{
public:
  // Make sure and move the error, or else it will ge re-initialized.
  explicit Error(ns::Error&& error)
    : mError(error){};
  explicit operator bool() const;
  void print();

  ns::Error mError;
};

std::pair<mtlpp::Library, viz::Error>
CreateLibraryFromMetalLib(Device device, const char* metallib);

std::pair<mtlpp::Library, viz::Error>
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
std::pair<mtlpp::Library, viz::Error>
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
