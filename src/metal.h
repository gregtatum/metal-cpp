#include "mtlpp/mtlpp.hpp"
#include <stdexcept>

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
CreateLibrary(Device device, const char* source, const CompileOptions& options);

} // viz
