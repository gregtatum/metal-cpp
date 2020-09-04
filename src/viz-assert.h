#pragma once
#include <exception> // std::exception
#include <iostream>  // std::cout

namespace viz {

class ErrorMessage : public std::exception
{
public:
  explicit ErrorMessage(const char* message)
    : mMessage(message){};

  const char* what() { return mMessage; }

  const char* mMessage;
};

/**
 * This function should be called from the `int main()` function. It will ensure
 * that the exceptions are caught, displayed, and properly unwound when thrown.
 */
int
display_exceptions(void (*fn)())
{
  try {
    (*fn)();
    return 0;
  } catch (std::exception error) {
    std::cout << error.what();
    return 1;
  }
}

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
