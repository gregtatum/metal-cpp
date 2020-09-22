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
  // Allow copy
  Vec2(const Vec2& other) = default;
  Vec2& operator=(const Vec2& other) = default;

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
  // Not copyable or movable, this should be instantiated once, but then only
  // shared via reference. It's referenced from at least one other thread.
  Tick(const Tick&) = delete;
  Tick& operator=(const Tick&) = delete;

  Tick(double aWidth, double aHeight)
    : width(aWidth)
    , height(aHeight){};

  // Called on every tick.
  void Update();

  // The amount of time passed in milliseconds.
  float milliseconds = 0;
  float seconds = 0;
  // The change in time in milliseconds.
  float dt = 16.666;
  // A generational counter for how many ticks have been called.
  std::atomic_uint32_t tick = 0;
  // std::cin can signal to the process to trace some frames by sending a
  // number.
  std::atomic_uint32_t traceFrames = 0;
  // This is mutated by the AutoDraw mechanism to control tracing.
  int32_t traceFramesUntilTick = -1;

  // The width of the window.
  float width = 0;
  // The height of the window.
  float height = 0;

  std::optional<Vec2<float>> mouse = {};
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
