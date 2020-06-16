
#include "globals.hpp"
#include "min_cut.cpp"
#include "polygon_fill.cpp"
#include "read_bin_file.cpp"
#include "super_pixel.cpp"
#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>

using namespace boost;

// Can't test both at same time
// #define TEST_OVERLAY 1
#define TEST_CLASSPOINTS 1

int main() {
  std::vector<BYTE> *imageFileData = readFile("assets/orange.bin");
  width = 320;
  height = 249;

#ifdef TEST_OVERLAY
  const int pi1 = addPolygon(0);
  const int pi2 = addPolygon(1);

  addLineToPolygon(pi1, 0, 0, 249, 40);
  addLineToPolygon(pi1, 249, 40, 249, 0);
  addLineToPolygon(pi1, 249, 0, 0, 0);

  addLineToPolygon(pi2, 80, 117, 144, 180);
  addLineToPolygon(pi2, 144, 180, 80, 180);
  addLineToPolygon(pi2, 80, 180, 80, 117);

  coloredMask = std::vector<uint32_t>(width * height);
  overlayPolygonsOnColoredMask();
#endif

#ifdef TEST_CLASSPOINTS

  superpixel(*imageFileData);

  const int pi1 = addPolygon(0);
  const int pi2 = addPolygon(1);

  addLineToPolygon(pi1, 0, 0, 249, 40);
  addLineToPolygon(pi1, 249, 40, 249, 0);
  addLineToPolygon(pi1, 249, 0, 0, 0);

  addLineToPolygon(pi2, 80, 117, 144, 180);
  addLineToPolygon(pi2, 144, 180, 80, 180);
  addLineToPolygon(pi2, 80, 180, 80, 117);

  addPolygonImpliedClassPoints();
  prepareForMinCuts();
  minCutCls(0);
  minCutCls(1);
  resolveMasks();
#endif

  std::ofstream fout;
  fout.open("polygon_fill.bin", std::ios::binary | std::ios::out);
  for (int i = 0; i < coloredMask.size(); i++) {
    fout.write((char *)&coloredMask[i], 4);
  }
  fout.close();

  return 0;
}
