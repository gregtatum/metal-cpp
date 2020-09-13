#include "viz/geo/icosphere.h"
#include "viz/math.h"
#include <map>
#include <vector>

namespace viz {

// This function subdivides a mesh, but does not smooth or do anything to the
// newly created geometry.
void
subdivide(Mesh& mesh)
{
  Positions midpoints = {};
  auto oldCells = mesh.cells;
  mesh.cells = {};

  using PositionIndex = uint32_t;
  std::map<Vector3, PositionIndex> vectorToIndex{};

  auto getMidpointIndex = [&](Vector3& a, Vector3& b) {
    auto midpoint = a.lerp(b, 0.5f);

    // De-duplicate the midpoints.
    auto result = vectorToIndex.find(midpoint);
    if (result == vectorToIndex.end()) {
      PositionIndex index = mesh.positions.size();
      mesh.positions.push_back(midpoint);
      vectorToIndex.insert({ midpoint, index });
      return index;
    } else {
      return result->second;
    }
  };

  for (auto const& cell : oldCells) {
    auto v0 = mesh.positions[cell[0]];
    auto v1 = mesh.positions[cell[1]];
    auto v2 = mesh.positions[cell[2]];

    auto mid0 = getMidpointIndex(v0, v1);
    auto mid1 = getMidpointIndex(v1, v2);
    auto mid2 = getMidpointIndex(v2, v0);

    mesh.cells.push_back({ cell[0], mid0, mid2 });
    mesh.cells.push_back({ cell[1], mid1, mid0 });
    mesh.cells.push_back({ cell[2], mid2, mid1 });
    mesh.cells.push_back({ mid0, mid1, mid2 });
  }
}

Mesh
generateIcosphere(IcosphereInitializer initializer)
{
  Positions positions{};
  Cells cells{};
  float t = 0.5f + sqrt(5.0f) / 2.0f;

  // Start by inputing the raw values for the general shape.
  positions.push_back({ -1.0, +t, 0.0 });
  positions.push_back({ +1.0, +t, 0.0 });
  positions.push_back({ -1.0, -t, 0.0 });
  positions.push_back({ +1.0, -t, 0.0 });

  positions.push_back({ 0.0, -1.0, +t });
  positions.push_back({ 0.0, +1.0, +t });
  positions.push_back({ 0.0, -1.0, -t });
  positions.push_back({ 0.0, +1.0, -t });

  positions.push_back({ +t, 0.0, -1.0 });
  positions.push_back({ +t, 0.0, +1.0 });
  positions.push_back({ -t, 0.0, -1.0 });
  positions.push_back({ -t, 0.0, +1.0 });

  // Connect together the geometry.
  cells.push_back({ 0, 11, 5 });
  cells.push_back({ 0, 5, 1 });
  cells.push_back({ 0, 1, 7 });
  cells.push_back({ 0, 7, 10 });
  cells.push_back({ 0, 10, 11 });

  cells.push_back({ 1, 5, 9 });
  cells.push_back({ 5, 11, 4 });
  cells.push_back({ 11, 10, 2 });
  cells.push_back({ 10, 7, 6 });
  cells.push_back({ 7, 1, 8 });

  cells.push_back({ 3, 9, 4 });
  cells.push_back({ 3, 4, 2 });
  cells.push_back({ 3, 2, 6 });
  cells.push_back({ 3, 6, 8 });
  cells.push_back({ 3, 8, 9 });

  cells.push_back({ 4, 9, 5 });
  cells.push_back({ 2, 4, 11 });
  cells.push_back({ 6, 2, 10 });
  cells.push_back({ 8, 6, 7 });
  cells.push_back({ 9, 8, 1 });

  Mesh mesh{};
  mesh.cells = cells;
  mesh.positions = positions;

  // Subdividing iteratively will smooth out the sphere.
  for (size_t i = 0; i < initializer.subdivisions; i++) {
    subdivide(mesh);
  }

  // At this point, all of the triangles are still in their original positions.
  // Normalize them to turn them back into a sphere.
  for (auto& position : mesh.positions) {
    position.Normalize();
  }

  // The positions are all normalized at this point. Copy over the normals.
  mesh.normals = mesh.positions;

  // Only change the radius of the sphere if it's needed.
  if (initializer.radius != 1) {
    for (auto& position : mesh.positions) {
      position *= initializer.radius;
    }
  }

  return mesh;
}

} // namespace viz
