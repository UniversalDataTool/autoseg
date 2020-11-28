
#include "globals.hpp"
#include "min_cut.cpp"
#include "polygon_fill.cpp"
#include "read_bin_file.cpp"
#include "super_pixel.cpp"
#include <iostream>
#include <vector>

#include <math.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>

using namespace boost;

// Can't test both at same time
#define TEST_OVERLAY 1
// #define TEST_CLASSPOINTS 1

int main() {
  std::vector<BYTE> *imageFileData = readFile("assets/orange-320x249.bin");
  width = 320;
  height = 249;

#ifdef TEST_OVERLAY
  const int pi1 = addPolygon(0);
  const int pi2 = addPolygon(1);
  // const int pi3 = addPolygon(1);

  addLineToPolygon(pi1, 0, 0, 249, 40);
  addLineToPolygon(pi1, 249, 40, 249, 0);
  addLineToPolygon(pi1, 249, 0, 0, 0);

  // addLineToPolygon(pi2, 100, 117, 144, 180);
  // addLineToPolygon(pi2, 144, 180, 80, 180);
  // addLineToPolygon(pi2, 80, 180, 100, 117);

  // addLineToPolygon(pi3, 200, 50, 180, 30);
  // addLineToPolygon(pi3, 180, 30, 100, 100);
  // addLineToPolygon(pi3, 100, 100, 110, 120);
  // addLineToPolygon(pi3, 110, 120, 130, 110);
  // addLineToPolygon(pi3, 130, 110, 120, 115);
  // addLineToPolygon(pi3, 120, 115, 200, 150);
  // addLineToPolygon(pi3, 200, 150, 150, 100);
  // addLineToPolygon(pi3, 150, 100, 200, 50);

  for (double a = 0.1; a < 2 * 3.14159; a += 0.50) {
    double b = a + 0.50;
    if (b > 2 * 3.14159) {
      b = 0.1;
    }
    double cx = 320 / 2.0;
    double cy = 249 / 2.0;

    std::cout << a << "," << (int)(cx + cos(a) * 50) << ","
              << (int)(cy + sin(a) * 50) << std::endl;
    std::cout << b << "," << (int)(cx + cos(b) * 50) << ","
              << (int)(cy + sin(b) * 50) << std::endl;
    addLineToPolygon(pi2, cx + cos(a) * 50, cy + sin(a) * 50, cx + cos(b) * 50,
                     cy + sin(b) * 50);
  }

  coloredMask = std::vector<uint32_t>(width * height, 0);
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
  fout.open("polygon_fill-320x249.bin", std::ios::binary | std::ios::out);
  for (int i = 0; i < coloredMask.size(); i++) {
    fout.write((char *)&coloredMask[i], 4);
  }
  fout.close();

  return 0;
}
