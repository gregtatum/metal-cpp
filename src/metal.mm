#include <Foundation/NSError.h>
#include <Metal/MTLDevice.h>
#include <mach-o/dyld.h>   //_NSGetExecutablePath
#include <sys/syslimits.h> // PATH_MAX
// Make sure including metal.h is last, otherwise there is ambiguous
// resolution of some internal types.
#include "metal.h"
#include "termcolor.hpp"

viz::Error::operator bool() const
{
  return mError.GetPtr() != nil;
}

void
viz::Error::print()
{
  auto error = (__bridge NSError*)mError.GetPtr();
  if (error == nil) {
    // There is no error.
    return;
  }

#define HEADER(text)                                                           \
  std::cout << termcolor::bold << termcolor::magenta << text                   \
            << termcolor::reset << std::endl;

  HEADER("Error Code " << [error code]);
  HEADER("Description:");

  std::cout << [[error localizedDescription] UTF8String] << std::endl;

  if ([error localizedRecoveryOptions]) {
    HEADER("Recovery Options:");
    for (NSString* string in [error localizedRecoveryOptions]) {
      std::cout << [string UTF8String] << std::endl;
    }
  }
  if ([error localizedRecoverySuggestion]) {
    HEADER("Recovery Suggestions:");
    std::cout << [[error localizedRecoverySuggestion] UTF8String] << std::endl;
  }
  if ([error localizedFailureReason]) {
    HEADER("Failure Reason:");
    std::cout << [[error localizedFailureReason] UTF8String] << std::endl;
  }
#undef HEADER
}

std::pair<Library, viz::Error>
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
                   viz::Error(ns::Handle{ (__bridge void*)error }));
};

std::pair<mtlpp::Library, viz::Error>
viz::CreateLibraryFromMetalLib(Device device, const char* metallib)
{
  // Error
  NSError* error = nil;

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:metallib]
                 error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   viz::Error(ns::Handle{ (__bridge void*)error }));
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

std::pair<mtlpp::Library, viz::Error>
viz::CreateLibraryForExample(Device device)
{
  // Error
  NSError* error = nil;

  auto path = getExecutablePath() + ".metallib";

  id<MTLLibrary> library = [(__bridge id<MTLDevice>)device.GetPtr()
    newLibraryWithFile:[NSString stringWithUTF8String:path.c_str()]
                 error:&error];

  return std::pair(ns::Handle{ (__bridge void*)library },
                   viz::Error(ns::Handle{ (__bridge void*)error }));
}
