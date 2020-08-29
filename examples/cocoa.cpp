#include "app.h"
#include "mtlpp/mtlpp.hpp"
#include <functional>
#include <stdio.h> // printf

/**
 * This example is for a simple Cocoa window, and it outputs where
 * the mouse is on the screen to stdout.
 */
int
main()
{
  auto device = mtlpp::Device::CreateSystemDefaultDevice();
  viz::TickFn tickFn = [](viz::Tick& tick) -> void {
    printf("mouse ");
    if (tick.mouse) {
      printf("(%f,%f) ", tick.mouse->x, tick.mouse->y);
    }
    if (tick.isMouseDown) {
      printf("mousedown");
    }
    printf("\n");
  };
  viz::InitApp(device, &tickFn);
}
