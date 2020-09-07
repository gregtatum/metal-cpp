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

template<>
struct debug<Mesh>
{
  static void print(Mesh const& mesh, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "Mesh {\n";

    DebugIndent(tabDepth + 1);
    std::cout << "positions: ";
    debug<Positions>::print(mesh.positions, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "uvs: ";
    debug<UVs>::print(mesh.uvs, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "normals: ";
    debug<Normals>::print(mesh.normals, tabDepth + 1);

    std::cout << ",\n";
    DebugIndent(tabDepth + 1);
    std::cout << "cells: ";
    debug<Cells>::print(mesh.cells, tabDepth + 1);

    std::cout << ",\n}";
  }
};

template<typename T>
struct debug<std::array<T, 3>>
{
  static void print(std::array<T, 3> const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
  }
};

} // namespace viz
