#pragma once
#include "viz/debug.h"
#include "viz/macros.h"
#include "viz/math.h"
#include "viz/metal.h"
#include <vector>

namespace viz {
using Cells = std::vector<std::array<uint32_t, 3>>;
using Positions = std::vector<viz::Vector3>;
using Normals = std::vector<viz::Vector3>;
using UVs = std::vector<viz::Vector2>;

/**
 * A mesh represents a simplicial complex, that can be used to draw geometry.
 */
struct Mesh
{
  Positions positions = {};
  UVs uvs = {};
  Normals normals = {};
  Cells cells = {};
};

/**
 * Use macros to generate the Debug<Mesh>() definition.
 */
VIZ_DEBUG_OBJ(mesh, Mesh, {
  VIZ_DEBUG_OBJ_HEADER(Mesh, mesh, tabDepth);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, positions, Positions);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, uvs, UVs);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, normals, Normals);
  VIZ_DEBUG_OBJ_PROP(mesh, tabDepth, cells, Cells);
  VIZ_DEBUG_OBJ_FOOTER(tabDepth);
});

/**
 * Conveniently create all the buffers needed for a mesh.
 */
class MeshBuffers
{
public:
  MeshBuffers(mtlpp::Device& device, Mesh& mesh, mtlpp::ResourceOptions options)
    : positions(BufferViewList<Vector3>(device, options, mesh.positions))
    , cells(
        BufferViewList<std::array<uint32_t, 3>>(device, options, mesh.cells))
    , normals(BufferViewList<Vector3>(device, options, mesh.normals))
    , indexCount(mesh.cells.size() * 3)
  {}

  BufferViewList<Vector3> positions;
  BufferViewList<std::array<uint32_t, 3>> cells;
  BufferViewList<Vector3> normals;
  uint32_t indexCount;
};

}
