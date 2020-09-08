#include <Foundation/NSError.h>
#include <Metal/MTLDevice.h>
#include <mach-o/dyld.h>   //_NSGetExecutablePath
#include <sstream>         // std::ostringstream
#include <string>          // std::string
#include <sys/syslimits.h> // PATH_MAX
// Make sure including metal.h is last, otherwise there is ambiguous
// resolution of some internal types.
#include "metal-objc.h"

viz::NicerNSError::operator bool() const
{
  return mError.GetPtr() != nil;
}

const char*
viz::NicerNSError::what() const noexcept
{
  if (mErrorMessage.size() == 0) {
    std::ostringstream stream;
    auto error = (__bridge NSError*)mError.GetPtr();
    if (error == nil) {
      // There is no error.
      return "No NSError is associated with this error.";
    }

    stream << "Error Code " << [error code] << std::endl;
    stream << "Description:" << std::endl;

    stream << [[error localizedDescription] UTF8String] << std::endl;

    if ([error localizedRecoveryOptions]) {
      stream << "Recovery Options:" << std::endl;
      for (NSString* string in [error localizedRecoveryOptions]) {
        stream << [string UTF8String] << std::endl;
      }
    }
    if ([error localizedRecoverySuggestion]) {
      stream << "Recovery Suggestions:" << std::endl;
      stream << [[error localizedRecoverySuggestion] UTF8String] << std::endl;
    }
    if ([error localizedFailureReason]) {
      stream << "Failure Reason:" << std::endl;
      stream << [[error localizedFailureReason] UTF8String] << std::endl;
    }
    mErrorMessage = stream.str();
  }
  return mErrorMessage.c_str();
}

std::pair<Library, viz::NicerNSError>
viz::CreateLibraryFromSource(Device device,
                             const char* source,
                             const CompileOptions& options)
{
  // Error
  NSError* error = nil;

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithSource:[NSString stringWithUTF8String:source]
                 options:(__bridge MTLCompileOptions*)options.GetPtr()
                   error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   viz::NicerNSError(ns::Handle{ (__bridge void*)error }));
};

std::pair<mtlpp::Library, viz::NicerNSError>
viz::CreateLibraryFromMetalLib(Device device, const char* metallib)
{
  // Error
  NSError* error = nil;

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:metallib]
                 error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   viz::NicerNSError(ns::Handle{ (__bridge void*)error }));
}

/**
 * This is a macOS-specific way to get the executable path.
 *
 * https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
 */
std::string
getExecutablePath()
{
  char path[PATH_MAX + 1];
  uint32_t length = PATH_MAX;
  int result = _NSGetExecutablePath(path, &length);
  assert(result != -1);
  return std::string{ path };
}

mtlpp::Library
viz::CreateLibraryForExample(Device device)
{
  // Error
  NSError* nsError = nil;

  auto path = getExecutablePath() + ".metallib";

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:path.c_str()]
                 error:&nsError];

  auto error = viz::NicerNSError(ns::Handle{ (__bridge void*)nsError });
  if (error) {
    throw error;
  }

  return ns::Handle{ (__bridge void*)library };
}
