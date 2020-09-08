#pragma once
#include <exception> // std::exception
#include <iostream>  // std::cout

namespace viz {

class ErrorMessage : public std::exception
{
public:
  explicit ErrorMessage(const char* message);
  const char* what();
  const char* mMessage;
};

/**
 * This function should be called from the `int main()` function. It will ensure
 * that the exceptions are caught, displayed, and properly unwound when thrown.
 */
int
display_exceptions(void (*fn)());

void
ReleaseAssert(bool assertion, const char* message);
void
DebugAssert(bool assertion, const char* message);

} // namespace viz
