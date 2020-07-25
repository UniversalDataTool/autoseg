#ifndef GLOBALS_CPP
#define GLOBALS_CPP

#include "globals.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

using std::vector;

int width, height;

vector<vector<double>> lmat;
vector<vector<double>> amat;
vector<vector<double>> bmat;

vector<std::array<double, 5>> centers;
vector<vector<double>> distances;
vector<vector<int>> clusters;
int numClusters;
int step;
double step2;
int totalClasses = 0;
int maxClusters = DEFAULT_MAX_CLUSTERS;
int weightFactor = DEFAULT_WEIGHT_FACTOR;
double weightFactor2 = static_cast<double>(DEFAULT_WEIGHT_FACTOR) *
                       static_cast<double>(DEFAULT_WEIGHT_FACTOR);

vector<ClassPoint> classPoints;

vector<int> spCls;

vector<vector<int>> clsMasks;

vector<vector<double>> mrClassDist;

std::unordered_map<std::pair<int, int>, double, pair_hash> spAdjMap;

vector<uint32_t> classToColor{0xffff00ff, 0xff0000ff};

vector<int> spClsMask;

vector<uint32_t> coloredMask;

double stdClusterLabNorm;

bool verboseMode = false;

vector<double> clusterMembers;

vector<ClassPolygon> polygons;

#endif
