/*
Implementation of SLIC-like algorithm. Some modifications:
* Not doing post-connectivity
* Not doing the "smallest gradient" in initial selection
* Previous cluster center is used as part of averaging (counted as one point)
  to prevent "zeroing" of clusters with 0 members
*/

#include "globals.hpp"
#include <math.h>
#include <vector>

#include <stdio.h>

#include "colorspace/ColorSpace.h"
#include "colorspace/Conversion.h"
#include <array>
#include <blaze/Math.h>

using blaze::DynamicMatrix;
using blaze::StaticMatrix;

void allocateGlobals() {
  lmat = vector<vector<double>>(height, vector<double>(width));
  amat = vector<vector<double>>(height, vector<double>(width));
  bmat = vector<vector<double>>(height, vector<double>(width));
  distances = vector<vector<double>>(height, vector<double>(width));
  clusters = vector<vector<int>>(height, vector<int>(width));
  step = static_cast<int>(sqrt((static_cast<double>(width) *
                                static_cast<double>(height) / maxClusters)));
  step2 = static_cast<double>(step) * static_cast<double>(step);
  numClusters = (width / step) * (height / step);
  clusterMembers = vector<double>(numClusters);
  std::array<double, 5> emptyAr = {0, 0, 0, 0, 0};
  centers = vector<std::array<double, 5>>(numClusters, emptyAr);
  spAdjMap = std::unordered_map<std::pair<int, int>, double, pair_hash>();
}

void convertToLabSpace(std::vector<uint8_t> &imageFileData) {
  for (int ri = 0; ri < height; ri++) {
    for (int ci = 0; ci < width; ci++) {
      ColorSpace::Rgb rgb(imageFileData[(ri * width + ci) * 4 + 0],
                          imageFileData[(ri * width + ci) * 4 + 1],
                          imageFileData[(ri * width + ci) * 4 + 2]);
      ColorSpace::Lab lab;
      rgb.To<ColorSpace::Lab>(&lab);
      lmat[ri][ci] = lab.l;
      amat[ri][ci] = lab.a;
      bmat[ri][ci] = lab.b;
    }
  }
}

void selectInitialCenters() {
  // std::cout << "num clusters: " << numClusters << std::endl;
  // std::cout << "step size: " << step << std::endl;
  for (int sri = 0; sri < height / step; sri++) {
    for (int sci = 0; sci < width / step; sci++) {
      int cci = sri * (width / step) + sci;
      int ri = step * sri;
      int ci = step * sci;

      centers[cci][0] = static_cast<double>(ri);
      centers[cci][1] = static_cast<double>(ci);
      // TODO find local minimum
      centers[cci][2] = lmat[ri][ci];
      centers[cci][3] = amat[ri][ci];
      centers[cci][4] = bmat[ri][ci];
    }
  }
}

double compute_distance(int cci, int ri, int ci) {
  const double centerRi = centers[cci][0];
  const double centerCi = centers[cci][1];
  double ds = sqrt(pow(centerRi - ri, 2) + pow(centerCi - ci, 2));
  double dc = sqrt(pow(centers[cci][2] - lmat[ri][ci], 2) +
                   pow(centers[cci][3] - amat[ri][ci], 2) +
                   pow(centers[cci][4] - bmat[ri][ci], 2));
  return sqrt(pow(dc / weightFactor, 2) + pow(ds / step, 2));
}

double compute_distance2(int cci, int ri, int ci) {
  const double centerRi = centers[cci][0];
  const double centerCi = centers[cci][1];
  double ds2 = pow(centerRi - ri, 2) + pow(centerCi - ci, 2);
  double dc2 = pow(centers[cci][2] - lmat[ri][ci], 2) +
               pow(centers[cci][3] - amat[ri][ci], 2) +
               pow(centers[cci][4] - bmat[ri][ci], 2);
  return pow(dc2 / weightFactor2, 2) + pow(ds2 / step2, 2);
}

double cluster_color_similarity(int cci1, int cci2) {
  double dc = pow(centers[cci1][2] - centers[cci2][2], 2) +
              pow(centers[cci1][3] - centers[cci2][3], 2) +
              pow(centers[cci1][4] - centers[cci2][4], 2);
  double sim = exp(-1.0 / (2.0 * pow(stdClusterLabNorm, 2)) * dc);
  // std::cout << cci1 << "," << cci2 << ": " << dc << "/" << sim << std::endl;
  // std::cout << centers[cci1][ 0] << "," << centers[cci2][ 0] << std::endl;
  // std::cout << centers[cci1][ 1] << "," << centers[cci2][ 1] << std::endl;
  // std::cout << centers[cci1][ 2] << "," << centers[cci2][ 2] << std::endl;
  // std::cout << centers[cci1][ 3] << "," << centers[cci2][ 3] << std::endl;
  // std::cout << centers[cci1][ 4] << "," << centers[cci2][ 4] << std::endl;
  return sim;
}

void setPixelsToClosestClusterCenter() {
  // Iterate over each center, set pixel distances
  // distances = std::numeric_limits<double>::max();

  for (int ri = 0; ri < height; ri++) {
    std::fill(distances[ri].begin(), distances[ri].end(),
              std::numeric_limits<double>::max());
  }

  for (int cci = 0; cci < numClusters; cci++) {
    const int maxRi = blaze::min(centers[cci][0] + step + 1, height);
    const int minRi = blaze::max(centers[cci][0] - step, 0);
    for (int ri = minRi; ri < maxRi; ri++) {
      const int maxCi = blaze::min(centers[cci][1] + step + 1, width);
      const int minCi = blaze::max(centers[cci][1] - step, 0);
      for (int ci = minCi; ci < maxCi; ci++) {
        // TODO speedup with row-wise min?
        const double distanceToClusterCci = compute_distance2(cci, ri, ci);
        if (distanceToClusterCci < distances[ri][ci]) {
          distances[ri][ci] = distanceToClusterCci;
          clusters[ri][ci] = cci;
        }
      }
    }
  }
}

void computeNewClusterCenters() {
  std::fill(clusterMembers.begin(), clusterMembers.end(), 1.0);
  for (int ri = 0; ri < height; ri++) {
    for (int ci = 0; ci < width; ci++) {
      const int cci = clusters[ri][ci];
      if (cci == -1)
        continue;
      centers[cci][0] += ri;
      centers[cci][1] += ci;
      centers[cci][2] += lmat[ri][ci];
      centers[cci][3] += amat[ri][ci];
      centers[cci][4] += bmat[ri][ci];
      clusterMembers[cci] += 1.0;
    }
  }
}

void normalizeClusterCenters() {
  // Normalize clusters
  // TODO this should be a single matrix operation, but I couldn't figure
  // out how to do it with blaze
  for (int cci = 0; cci < numClusters; cci++) {
    const double members = clusterMembers[cci];
    centers[cci][0] /= members;
    centers[cci][1] /= members;
    centers[cci][2] /= members;
    centers[cci][3] /= members;
    centers[cci][4] /= members;
  }
}

void assignLostPixels() {
  // Any pixels that have a cluster of -1, set find their nearest cluster
  for (int ri = 0; ri < height; ri++) {
    for (int ci = 0; ci < width; ci++) {
      if (clusters[ri][ci] == -1) {
        // find closest cluster manually
        double bestDist = std::numeric_limits<double>::max();
        int bestCci = -1;
        for (int cci = 0; cci < numClusters; cci++) {
          const double d = compute_distance2(cci, ri, ci);
          if (d < bestDist) {
            bestDist = d;
            bestCci = cci;
          }
        }
        clusters[ri][ci] = bestCci;
      }
    }
  }
}

void computeClusterNorms() {
  blaze::DynamicVector<double> norms =
      blaze::DynamicVector<double>(numClusters * (numClusters - 1) / 2);
  int currentNormCci = 0;
  for (int cci1 = 0; cci1 < numClusters - 1; cci1++) {
    for (int cci2 = cci1 + 1; cci2 < numClusters; cci2++) {
      norms[currentNormCci] = sqrt(pow(centers[cci1][2] - centers[cci2][2], 2) +
                                   pow(centers[cci1][3] - centers[cci2][3], 2) +
                                   pow(centers[cci1][4] - centers[cci2][4], 2));
      currentNormCci++;
    }
  }
  stdClusterLabNorm = blaze::stddev(norms);
}

void computeAdjacencyMap() {
  // Compute superpixel adjacency map
  for (int ri = 0; ri < height - 1; ri++) {
    for (int ci = 0; ci < width - 1; ci++) {
      const int cci = clusters[ri][ci];
      const int adjRight = clusters[ri][ci + 1];
      const int adjDown = clusters[ri + 1][ci];

      const auto adjRightConn = adj_pair(cci, adjRight);
      const auto adjDownConn = adj_pair(cci, adjDown);

      const bool adjRightConnected =
          spAdjMap.find(adjRightConn) != spAdjMap.end();
      const bool adjDownConnected =
          spAdjMap.find(adjDownConn) != spAdjMap.end();

      if (!adjRightConnected && cci != adjRight) {
        const double cs = cluster_color_similarity(cci, adjRight);
        spAdjMap[adjRightConn] = cs;
      }
      if (!adjDownConnected && cci != adjDown) {
        const double cs = cluster_color_similarity(cci, adjDown);
        spAdjMap[adjDownConn] = cs;
      }
    }
  }
}

void superpixel(std::vector<uint8_t> &imageFileData) {
  allocateGlobals();

  if (verboseMode) {
    printf("width: %d, height: %d\n", width, height);
  }

  convertToLabSpace(imageFileData);

  if (verboseMode) {
    printf("num clusters: %d\n", numClusters);
  }

  selectInitialCenters();

  for (int iteration = 0; iteration < SLIC_ITERATIONS; iteration++) {
    if (verboseMode) {
      printf("\n\nITERATION %d\n\n", iteration);
    }
    if (verboseMode && numClusters < 30) {
      for (int i = 0; i < numClusters; i++) {
        printf("Cluster Center %4d: %4.1f %4.1f\n", i, centers[i][0],
               centers[i][1]);
      }
    }

    if (verboseMode && width < 50 && height < 50) {
      printf("\n\ndistances:");
      for (int ri = 0; ri < height; ri++) {
        printf("\n%3d: ", ri);
        for (int ci = 0; ci < width; ci++) {
          printf("%4.4f", distances[ri][ci]);
        }
      }

      printf("\n\nclusters:");
      for (int ri = 0; ri < height; ri++) {
        printf("\n%3d: ", ri);
        for (int ci = 0; ci < width; ci++) {
          printf("%3d", clusters[ri][ci]);
        }
      }
      printf("\n");
    }

    setPixelsToClosestClusterCenter();

    computeNewClusterCenters();

    normalizeClusterCenters();
  }

  // Iterate over each center, set pixel to closest center
  setPixelsToClosestClusterCenter();

  assignLostPixels();

  computeClusterNorms();

  if (verboseMode) {
    printf("stdClusterLabNorm: %f\n", stdClusterLabNorm);
  }

  computeAdjacencyMap();
}
