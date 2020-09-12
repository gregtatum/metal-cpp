#include "viz/geo/mesh.h"
#include "viz/math.h"

namespace viz {

struct IcosphereInitializer
{
  size_t subdivisions = 0;
  float radius = 1;
};

Mesh
generateIcosphere(IcosphereInitializer initializer);

} // namespace viz
