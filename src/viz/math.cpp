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

/**
 * Generates numbers that tend to be lower, based on the exponent provided.
 */
float
RandomPow(size_t pow)
{
  auto n = Random();
  auto result = n;
  for (size_t i = 1; i < pow; i++) {
    result *= n;
  }
  return result;
};

float
RandomPow(float range, size_t pow)
{
  auto n = RandomPow(pow);
  return n * range;
};

float
RandomPow(float rangeMin, float rangeMax, size_t pow)
{
  auto n = RandomPow(pow);
  auto range = rangeMax - rangeMin;
  return n * range + rangeMin;
};

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
