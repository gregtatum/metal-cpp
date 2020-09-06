// #include "geo-box.h"
#include "geo-box.h"
#include "viz-math.h"
#include <vector>

namespace viz {

using Positions2D = std::vector<viz::Vector2>;

struct PanelConfig
{
  float wx;
  float wy;
  float sx;
  float sy;
};

struct Panel
{
  Positions2D positions2D;
  Positions positions;
  Cells cells;
  UVs uvs;
  Normals normals;
  float vertexCount;
};

std::vector<Positions2D>
generateGrid(PanelConfig const& config)
{
  auto stepY = config.wy / config.sy;
  auto halfY = config.wy / 2;
  auto lengthY = config.sy + 1;
  std::vector<std::vector<viz::Vector2>> grid{};

  for (auto i = 0; i < lengthY; ++i) {
    auto height = stepY * i - halfY;
    auto halfX = config.wx / 2;
    auto stepX = config.wx / config.sx;
    auto lengthX = config.sx + 1;
    std::vector<viz::Vector2> row{};

    for (auto j = 0; j < lengthX; j++) {
      auto width = stepX * j - halfX;
      row.push_back(viz::Vector2{ width, height });
    }

    grid.push_back(row);
  }

  return grid;
}

uint32_t
getCellIndex(PanelConfig const& config, uint32_t x, uint32_t y)
{
  return (config.sx + 1) * y + x;
}

Cells
generateCells(PanelConfig const& config)
{
  Cells cells{};
  for (auto x = 0; x < config.sx; x++) {

    for (auto y = 0; y < config.sy; y++) {

      auto a = getCellIndex(config, x + 0, y + 0); //   d __ c
      auto b = getCellIndex(config, x + 1, y + 0); //    |  |
      auto c = getCellIndex(config, x + 1, y + 1); //    |__|
      auto d = getCellIndex(config, x + 0, y + 1); //   a    b

      cells.push_back(std::array{ a, b, c });
      cells.push_back(std::array{ c, d, a });
    }
  }
  return cells;
}

Positions2D
flattenRowsIntoPositions(std::vector<std::vector<viz::Vector2>> const& rows)
{
  Positions2D positions{};

  for (auto i = 0; i < rows.size(); i++) {
    auto& row = rows[i];
    for (auto j = 0; j < row.size(); j++) {
      positions.push_back(row[j]);
    }
  }
  return positions;
}

UVs
generateUvs(Positions2D& positions, PanelConfig const& config)
{
  UVs uvs{};
  for (auto& position : positions) {
    auto u = position[0] / config.wx + 0.5;
    auto v = position[1] / config.wy + 0.5;
    uvs.push_back({ u, v });
  }
  return uvs;
}

Panel
generatePanel(Mesh& mesh, PanelConfig const&& config)
{
  auto rows = generateGrid(config);
  auto positions2D = flattenRowsIntoPositions(rows);

  return Panel{
    .positions2D = positions2D,
    .positions = Positions{},
    .cells = generateCells(config),
    .uvs = generateUvs(positions2D, config),
    .normals = Normals{},
    .vertexCount = (config.sx + 1) * (config.sy + 1),
  };
}

template<typename Fn>
Positions
map2DTo3D(Panel const& panel, Fn fn)
{
  Positions positions3D;
  for (auto& p2D : panel.positions) {
    positions3D.push_back(fn(p2D));
  }
  return positions3D;
}

Normals
makeNormals(viz::Vector3&& normal, size_t count)
{
  Normals normals{};

  for (auto i = 0; i < count; i++) {
    normals.push_back(normal);
  }

  return normals;
}

std::array<Panel, 6>
generateBoxPanels(Mesh& mesh, viz::Vector3& size, viz::Vector3& segments)
{

  //       yp  zm
  //        | /
  //        |/
  // xm ----+----- xp
  //       /|
  //      / |
  //    zp  ym

  auto zp = generatePanel(
    mesh, PanelConfig{ size[0], size[1], segments[0], segments[1] });
  auto xp = generatePanel(
    mesh, PanelConfig{ size[2], size[1], segments[2], segments[1] });
  auto yp = generatePanel(
    mesh, PanelConfig{ size[0], size[2], segments[0], segments[2] });

  auto zm = Panel{ zp };
  auto xm = Panel{ xp };
  auto ym = Panel{ yp };

  // clang-format off
	zp.positions = map2DTo3D(zp, [&](viz::Vector2 p){ return viz::Vector3 {       p[0],       p[1],  size[2]/2 }; });
	zm.positions = map2DTo3D(zm, [&](viz::Vector2 p){ return viz::Vector3 {       p[0],      -p[1], -size[2]/2 }; });
	xp.positions = map2DTo3D(xp, [&](viz::Vector2 p){ return viz::Vector3 {  size[0]/2,      -p[1],       p[0] }; });
	xm.positions = map2DTo3D(xm, [&](viz::Vector2 p){ return viz::Vector3 { -size[0]/2,       p[1],       p[0] }; });
	yp.positions = map2DTo3D(yp, [&](viz::Vector2 p){ return viz::Vector3 {       p[0],  size[1]/2,      -p[1] }; });
	ym.positions = map2DTo3D(ym, [&](viz::Vector2 p){ return viz::Vector3 {       p[0], -size[1]/2,       p[1] }; });
  // clang-format on

  zp.normals = makeNormals({ 0, 0, 1 }, zp.positions.size());
  zm.normals = makeNormals({ 0, 0, -1 }, zm.positions.size());
  xp.normals = makeNormals({ 1, 0, 0 }, xp.positions.size());
  xm.normals = makeNormals({ -1, 0, 0 }, xm.positions.size());
  yp.normals = makeNormals({ 0, 1, 0 }, yp.positions.size());
  ym.normals = makeNormals({ 0, -1, 0 }, ym.positions.size());

  return { zp, zm, xp, xm, yp, ym };
}

void
offsetCellIndices(std::array<Panel, 6>& panels)
{
  // e.g.
  // from: [[[0,1,2],[2,3,0]],[[0,1,2],[2,3,0]]]
  // to:   [[[0,1,2],[2,3,0]],[[6,7,8],[8,9,6]]]

  uint32_t offset = 0;

  for (auto& panel : panels) {
    for (auto& cell : panel.cells) {
      cell[0] += offset;
      cell[1] += offset;
      cell[2] += offset;
    }
    offset += panel.vertexCount;
  }
}

Mesh
generateBox(viz::Vector3& size, viz::Vector3& segments)
{
  Mesh mesh{};

  auto panels = generateBoxPanels(mesh, size, segments);

  // Mutate the panels to correctly offset the cell indexes.
  offsetCellIndices(panels);

  // Combine the panels together.
  for (auto const& panel : panels) {
    for (auto const& position : mesh.positions) {
      mesh.positions.push_back(position);
    }
    for (auto const& uv : mesh.uvs) {
      mesh.uvs.push_back(uv);
    }
    for (auto const& normal : mesh.normals) {
      mesh.normals.push_back(normal);
    }
    for (auto const& cell : mesh.cells) {
      mesh.cells.push_back(cell);
    }
  }

  return mesh;
}

} // namespace viz
