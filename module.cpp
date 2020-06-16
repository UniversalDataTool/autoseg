#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

#include "globals.hpp"
#include "min_cut.cpp"
#include "polygon_fill.cpp"
#include "super_pixel.cpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <stdio.h>
#include <vector>

using namespace emscripten;

std::shared_ptr<std::vector<uint8_t>> image;
bool loggedVersion = false;
bool polygonImpliedClassPoints = true;

void setImageSize(int w, int h) {
  if (!loggedVersion) {
    printf("mmgc1_0.0.2\n");
    loggedVersion = true;
  }
  image = std::make_shared<std::vector<uint8_t>>(w * h * 4);
  width = w;
  height = h;
}

void setVerboseMode(bool trueForOn) { verboseMode = trueForOn; }

void setPolygonImpliedClassPoints(bool trueForOn) {
  polygonImpliedClassPoints = trueForOn;
}

void clearClassElements() {
  classPoints.clear();
  polygons.clear();
}

void setMaxClusters(int newMaxClusters) { maxClusters = newMaxClusters; }
void setWeightFactor(int newWeightFactor) { weightFactor = newWeightFactor; }

void setClassColor(int cls, uint32_t color) {
  while (classToColor.size() <= cls) {
    classToColor.push_back(0xFFFF00FF);
  }
  classToColor[cls] = color;
}

void addClassPoint(int cls, int ri, int ci) {
  classPoints.push_back(ClassPoint{cls, ri, ci});
}

void computeSuperPixels() { superpixel(*image); }

void computeMasks() {
  if (polygonImpliedClassPoints) {
    addPolygonImpliedClassPoints();
  }
  prepareForMinCuts();
  for (int i = 0; i < totalClasses; i++) {
    printf("running min cut for cls: %d\n", i);
    minCutCls(i);
  }
  if (verboseMode) {
    for (int clsi = 0; clsi < totalClasses; clsi++) {
      for (int cci = 0; cci < numClusters; cci++) {
        printf("clsi: %4d  cci: %4d (ri: %4.0f, ci: %4.0f),    on: %d\n", clsi,
               cci, centers[cci][0], centers[cci][1], clsMasks[cci][clsi]);
      }
    }
  }
  resolveMasks();
  overlayPolygonsOnColoredMask();
}

// This is definitely not the right way to do this
int getColoredMask() { return reinterpret_cast<int>(&coloredMask[0]); }
int getImageAddr() { return reinterpret_cast<int>(&(*image)[0]); }

EMSCRIPTEN_BINDINGS(my_module) {
  register_vector<ClassPoint>("vector<ClassPoint>");
  value_object<ClassPoint>("ClassPoint")
      .field("cls", &ClassPoint::cls)
      .field("ri", &ClassPoint::ri)
      .field("ci", &ClassPoint::ci);
  function("setImageSize", &setImageSize);
  function("clearClassElements", &clearClassElements);
  function("addClassPoint", &addClassPoint);
  function("setClassColor", &setClassColor);
  function("getImageAddr", &getImageAddr, allow_raw_pointers());
  function("getColoredMask", &getColoredMask, allow_raw_pointers());
  function("computeSuperPixels", &computeSuperPixels);
  function("computeMasks", &computeMasks);
  function("setVerboseMode", &setVerboseMode);
  function("setMaxClusters", &setMaxClusters);
  function("addPolygon", &addPolygon);
  function("addLineToPolygon", &addLineToPolygon);
  function("setPolygonImpliedClassPoints", &setPolygonImpliedClassPoints);
}
