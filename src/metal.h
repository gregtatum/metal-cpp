#include "mtlpp/mtlpp.hpp"

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

} // viz
