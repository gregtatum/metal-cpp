#include "macros.h"
#include "viz-debug.h"
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
generateBox(viz::Vector3 size, viz::Vector3 segments);

VIZ_DEBUG_OBJ(mesh, Mesh, {
  VIZ_DEBUG_OBJ_HEADER(Mesh, mesh, tabDepth);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, positions, Positions);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, uvs, UVs);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, normals, Normals);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, cells, Cells);
  VIZ_DEBUG_OBJ_FOOTER(tabDepth);
});

} // namespace viz
