#include "globals.hpp"
#include "min_cut.cpp"
#include "read_bin_file.cpp"
#include "super_pixel.cpp"
#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>

using namespace boost;

int main() {
  std::vector<BYTE> *imageFileData = readFile("assets/orange.bin");
  width = 320;
  height = 249;

  superpixel(*imageFileData);

  classPoints.push_back(ClassPoint{0, 100, 125});
  classPoints.push_back(ClassPoint{0, 160, 125});
  classPoints.push_back(ClassPoint{0, 180, 125});
  classPoints.push_back(ClassPoint{0, 130, 200});
  classPoints.push_back(ClassPoint{0, 67, 190});
  classPoints.push_back(ClassPoint{1, 10, 10});
  classPoints.push_back(ClassPoint{1, 100, 10});
  classPoints.push_back(ClassPoint{1, 150, 10});
  classPoints.push_back(ClassPoint{1, 240, 300});
  classPoints.push_back(ClassPoint{1, 100, 300});

  // classPoints.push_back(ClassPoint{0, 0, 0});
  // classPoints.push_back(ClassPoint{1, 0, 1});

  // numClusters = 2;
  // clusters.resize(1, 2);
  // clusters(0, 0) = 0;
  // clusters(0, 1) = 1;
  // clusters(1, 0) = 2;
  // clusters(1, 1) = 3;

  // spAdjMap.clear();
  // spAdjMap[0][1] = 0.2;
  // spAdjMap[1][0] = 0.2;
  // spAdjMap[0][2] = 0.3;
  // spAdjMap[1][2] = 0.5;
  // spAdjMap[1][3] = 0.5;
  // spAdjMap[2][3] = 0.7;

  prepareForMinCuts();
  minCutCls(0);
  minCutCls(1);
  resolveMasks();

  std::ofstream fout;
  fout.open("mincut.bin", std::ios::binary | std::ios::out);
  for (int i = 0; i < coloredMask.size(); i++) {
    fout.write((char *)&coloredMask[i], 4);
  }
  fout.close();

  return 0;
}
