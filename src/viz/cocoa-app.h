#pragma once

#include "mtlpp/mtlpp.hpp"
#include <chrono>
#include <functional>
#include <optional>
#include <vector>

namespace viz {

template<typename T>
class Vec2
{
public:
  Vec2(T aX, T aY)
    : x(aX)
    , y(aY){};
  T x;
  T y;
};

// This is not guaranteed to be monotonically increasing.
using WallClock = std::chrono::time_point<std::chrono::system_clock>;

class Tick
{
public:
  Tick(double aWidth, double aHeight)
    : width(aWidth)
    , height(aHeight){};

  // Called on every tick.
  void Update();

  // The amount of time passed in milliseconds.
  double milliseconds = 0;
  double seconds = 0;
  // The change in time in milliseconds.
  double dt = 16.666;
  // A generational counter for how many ticks have been called.
  uint32_t tick = 0;
  // The width of the window.
  double width = 0;
  // The height of the window.
  double height = 0;

  std::optional<Vec2<double>> mouse = {};
  bool isMouseDown = false;

  bool hasRunOnce = false;

  mtlpp::Drawable drawable;
  mtlpp::RenderPassDescriptor renderPassDescriptor;

private:
  WallClock startTime = std::chrono::system_clock::now();
  WallClock currentTime = std::chrono::system_clock::now();
};

using TickFn = std::function<void(Tick&)>;

void
InitApp(const mtlpp::Device& device, TickFn* loop);

} // namespace viz
