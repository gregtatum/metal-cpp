#pragma once
#include "mtlpp/mtlpp.hpp"
#include <exception> // std::exception
#include <span>      // std::span
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

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
