#include "globals.hpp"

/*

Polygon Rastering Stuff

Conceptually, we want to be able to create implied class points which are inside
of a polygon (a bit away from the border) and overlay a polygon to guarantee
that a point within the polygon is marked appropriately.

These notebooks demonstrate how to render a polygon:
* https://observablehq.com/@wao/scan-line-polygon-filling-part-1
* https://observablehq.com/@wao/scan-line-polygon-filling-part-2

*/

int addPolygon(int cls) {
  polygons.push_back(ClassPolygon{cls});
  return polygons.size() - 1;
}

Line computeLine(int int_ri1, int int_ci1, int int_ri2, int int_ci2) {
  const double ri1 = static_cast<double>(int_ri1);
  double ri2 = static_cast<double>(int_ri2);
  const double ci1 = static_cast<double>(int_ci1);
  const double ci2 = static_cast<double>(int_ci2);

  if (ri1 == ri2) {
    ri2 += 0.1;
  }

  // Point Slope Form
  // y = mx = b
  double m = (ci2 - ci1) / (ri2 - ri1);
  double b = ci1 - m * ri1;

  // Standard Form
  // Ax + By = C
  // A(ri) + B(ci) = C
  double A = -m;
  double B = 1;
  double C = b;

  return Line{ri1, ri2, ci1, ci2, A, B, C};
}

void addLineToPolygon(int polygonIndex, int ri1, int ci1, int ri2, int ci2) {
  polygons[polygonIndex].lines.push_back(computeLine(ri1, ci1, ri2, ci2));
}

double getColumnIntersection(const Line &line, const int ri) {
  return line.C / line.B - line.A * ri;
}

bool isInBounds(const Line &line, const double ri, const double ci) {
  // TODO precompute and put in Line
  const double sm_ri = (line.ri1 < line.ri2 ? line.ri1 : line.ri2);
  const double lg_ri = (line.ri1 > line.ri2 ? line.ri1 : line.ri2);
  const double sm_ci = (line.ci1 < line.ci2 ? line.ci1 : line.ci2);
  const double lg_ci = (line.ci1 > line.ci2 ? line.ci1 : line.ci2);
  return ri >= sm_ri && ri <= lg_ri && ci >= sm_ci && ci <= lg_ci;
}

void getAllIntersections(const ClassPolygon &polygon,
                         vector<double> &intersections, int ri) {
  for (auto line = polygon.lines.begin(); line != polygon.lines.end(); ++line) {
    double ci = getColumnIntersection(*line, ri);
    if (isInBounds(*line, ri, ci)) {
      intersections.push_back(ci);
    }
  }
  std::sort(intersections.begin(), intersections.end());
}

void addPolygonImpliedClassPoints() {
  // BUG TODO this implementation is basically just a copy of
  // overlayPolygonsOnColoredMask and doesn't use a Y margin, which it should,
  // because people often make mistakes around edges, and we want to be
  // forgiving of 2-5 pixel mistakes
  const int MARGIN_X = 0;
  const double cpStepSize = 1; //((double)step) / 2;
  for (auto polygon = polygons.begin(); polygon != polygons.end(); ++polygon) {
    for (double ri = 0; ri < height - 1; ri += cpStepSize) {
      int currentIntersection = 0;
      bool filling = true;
      vector<double> intersections;
      getAllIntersections(*polygon, intersections, ri);
      if (intersections.size() <= 1)
        continue;
      while (currentIntersection < intersections.size() - 1) {
        if (filling) {
          double length = intersections[currentIntersection + 1] -
                          intersections[currentIntersection] - MARGIN_X * 2;
          if (length <= 0) {
            filling = !filling;
            currentIntersection++;
            continue;
          }

          // BUG TODO this should work to prevent the left biased, but I'm in a
          // rush today
          // const double extraOnEnd = ((length / spStepSize) % 1)
          // const double startPosition = intersections[currentIntersection] +
          //                              MARGIN_X + extraOnEnd / 2

          for (double ci =
                   intersections[currentIntersection] +
                   MARGIN_X; // poor man's X margin (what about Y margin!?!)
               ci <= intersections[currentIntersection + 1] - MARGIN_X &&
               ci < width - 1;
               ci += cpStepSize) {
            int int_ri = nearbyint(ri);
            int int_ci = nearbyint(ci);
            if (int_ci < 0)
              continue;
            classPoints.push_back(ClassPoint{polygon->cls, int_ri, int_ci});
          }
        }
        filling = !filling;
        currentIntersection++;
      }
    }
  }
}

void overlayPolygonsOnColoredMask() {
  // TODO check for not-closed polygons in javascript not here

  for (auto polygon = polygons.begin(); polygon != polygons.end(); ++polygon) {
    for (int ri = 0; ri < height; ri++) {
      int currentIntersection = 0;
      bool filling = true;
      vector<double> intersections;
      getAllIntersections(*polygon, intersections, ri);
      if (intersections.size() <= 1)
        continue;
      while (currentIntersection < intersections.size() - 1) {
        double length = intersections[currentIntersection + 1] -
                        intersections[currentIntersection];
        if (length <= 0) {
          // This occurs where two lines meet, i.e. at each corner
          currentIntersection++;
          continue;
        }
        if (filling) {
          int start = nearbyint(intersections[currentIntersection]);
          if (start < 0)
            start = 0;
          int end = nearbyint(intersections[currentIntersection + 1]);
          for (int ci = start; ci < end && ci < width; ci++) {
            const int imgi = ri * width + ci;
            coloredMask[imgi] = classToColor[polygon->cls];
          }
        }
        filling = !filling;
        currentIntersection++;
      }
    }
  }
}
