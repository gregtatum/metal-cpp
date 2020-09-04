#pragma once
#include <exception>

namespace viz {

class ErrorMessage : public std::exception
{
public:
  explicit ErrorMessage(const char* message)
    : mMessage(message){};

  const char* what() { return mMessage; }

  const char* mMessage;
};

void
ReleaseAssert(bool assertion, const char* message = "Assertion failed")
{
  if (!assertion) {
    throw ErrorMessage(message);
  }
}

#ifdef NDEBUG

void
DebugAssert(bool assertion, const char* message = "Assertion failed")
{
  if (!assertion) {
    throw ErrorMessage(message);
  }
}

#else
void
DebugAssert(bool assertion, const char* message = "Assertion")
{
  // Do nothing.
}
#endif

} // namespace viz
