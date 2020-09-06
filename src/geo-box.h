#include "viz-math.h"

namespace viz {

using Cells = std::vector<std::array<uint32_t, 3>>;
using Positions = std::vector<viz::Vector3>;
using Normals = std::vector<viz::Vector3>;
using UVs = std::vector<viz::Vector2>;

struct Mesh
{
  Positions positions = {};
  UVs uvs = {};
  Normals normals = {};
  Cells cells = {};
};

Mesh
generateBox(viz::Vector3& size, viz::Vector3& segments);

} // namespace viz
