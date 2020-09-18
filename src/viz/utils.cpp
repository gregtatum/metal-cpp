#include "./utils.h"
#include <filesystem>
#include <mach-o/dyld.h> //_NSGetExecutablePath
#include <string>
#include <sys/syslimits.h> // PATH_MAX

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

/**
 * Uses _NSGetExecutablePath, and then slices off the binary name from the path.
 *
 * https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
 */
std::string
getExecutableName()
{
  auto string = getExecutablePath();
  size_t start = 0;
  for (;;) {
    auto index = string.find("/", start);
    if (index == std::string::npos) {
      break;
    }
    start = index + 1;
  }

  return string.substr(start);
}
