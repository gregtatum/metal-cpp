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

namespace traits {
struct mesh
{};
template<>
struct tag<Mesh>
{
  typedef mesh type;
  typedef empty child;
};

}

namespace dispatch {
template<>
struct debug<traits::mesh, Mesh>
{
  static void apply(Mesh const& mesh, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "Mesh {\n";

    DebugIndent(tabDepth + 1);
    std::cout << "positions: ";
    ::viz::DebugWithoutNewline<Positions>(mesh.positions, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "uvs: ";
    ::viz::DebugWithoutNewline<UVs>(mesh.uvs, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "normals: ";
    ::viz::DebugWithoutNewline<Normals>(mesh.normals, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "cells: ";
    ::viz::DebugWithoutNewline<Cells>(mesh.cells, tabDepth + 1);

    std::cout << ",\n}";
  }
};

} // namespace dispatch
} // namespace viz
