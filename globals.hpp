#ifndef GLOBALS_H
#define GLOBALS_H

#define DLOG(x)                                                                \
  do {                                                                         \
  } while (0)
// #define DLOG(x) (x)

// #include <blaze/Math.h>
#include <array>
#include <blaze/Math.h>
#include <cstdint>
#include <unordered_map>
#include <vector>

// using blaze::DynamicMatrix;
using std::vector;

#define SLIC_ITERATIONS 10
#define DEFAULT_MAX_CLUSTERS 128
#define DEFAULT_WEIGHT_FACTOR 20

extern int width, height;

extern vector<vector<double>> lmat;
extern vector<vector<double>> amat;
extern vector<vector<double>> bmat;

extern vector<std::array<double, 5>> centers;
extern vector<vector<double>> distances;
extern vector<vector<int>> clusters;
extern int numClusters;
extern int step;
extern double step2;

extern int maxClusters;
extern int weightFactor;
extern double weightFactor2;

// super pixel class (numberSuperpixels)
extern vector<int> spCls;

// Classification Masks (numberSuperpixels, numberClasses)
extern vector<vector<int>> clsMasks;

// mask resolution class distance (numberSuperpixels, numberClasses)
extern vector<vector<double>> mrClassDist;

inline std::pair<int, int> adj_pair(int a, int b) {
  return a < b ? std::pair<int, int>(a, b) : std::pair<int, int>(b, a);
}
struct pair_hash {
  std::size_t operator()(const std::pair<int, int> &p) const {
    return p.first ^ p.second;
    // return (size_t)(((long)p.first) << 32) | (unsigned int)p.second;
  }
};
// inline size_t pair_hash(std::pair<int, int> p) {}
extern std::unordered_map<std::pair<int, int>, double, pair_hash> spAdjMap;

extern double stdClusterLabNorm;

struct ClassPoint {
  int cls;
  int ri;
  int ci;
};

struct Line {
  double ri1;
  double ri2;
  double ci1;
  double ci2;
  double A;
  double B;
  double C;
};

struct ClassPolygon {
  int cls;
  vector<Line> lines;
};

extern int totalClasses;

extern vector<ClassPoint> classPoints;

extern vector<uint32_t> classToColor;

extern vector<int> spClsMask;

extern vector<uint32_t> coloredMask;

extern bool verboseMode;

extern vector<double> clusterMembers;

extern vector<ClassPolygon> polygons;

#endif
