#include "math.h"

namespace viz {

float
Random()
{
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float
Random(float range)
{
  return Random() * range;
}

float
Random(float rangeMin, float rangeMax)
{
  return rangeMin + Random() * (rangeMax - rangeMin);
}

Vector3
RandomSpherical(RandomSphericalInitializer&& initializer)
{
  // http://mathworld.wolfram.com/SpherePointPicking.html
  auto [radius, center] = initializer;
  auto cosTheta = Random(-1.0f, 1.0f);
  auto sinTheta = sqrt(1 - cosTheta * cosTheta);
  auto phi = Random(M_PI * 2.0);

  return Vector3{
    center[0] + radius * sinTheta * cos(phi),
    center[1] + radius * sinTheta * sin(phi),
    center[2] + radius * cosTheta,
  };
}

} // namespace viz
