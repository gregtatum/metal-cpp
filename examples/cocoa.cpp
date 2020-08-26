#include "app.h"
#include "mtlpp/mtlpp.hpp"
#include <functional>
#include <stdio.h> // printf

int
main()
{
  auto device = mtlpp::Device::CreateSystemDefaultDevice();
  viz::TickFn tickFn = [](viz::Tick& tick) -> void {
    if (tick.mouse) {
      printf("!!! mouse %s\n", tick.isMouseDown ? "true" : "false");
    } else {
      printf("!!! no mouse\n");
    }
  };
  viz::InitApp(device, &tickFn);
}
