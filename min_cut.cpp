#include "globals.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/connected_components.hpp>
// #include <iostream>
#include <vector>

// #include <boost/graph/graphviz.hpp>

using namespace boost;

typedef adjacency_list_traits<vecS, vecS, directedS> Traits;

// typedef adjacency_list<vecS, vecS, directedS,
//                        property<vertex_name_t, std::string>,
//                        property<edge_capacity_t, int,
//                                 property<edge_residual_capacity_t, int,
//                                          property<edge_reverse_t,
//                                          Traits::edge_descriptor>>>>
//     Graph;

typedef adjacency_list<
    vecS, vecS, directedS,
    property<vertex_name_t, std::string,
             property<vertex_index_t, long,
                      property<vertex_color_t, boost::default_color_type,
                               property<vertex_distance_t, long,
                                        property<vertex_predecessor_t,
                                                 Traits::edge_descriptor>>>>>,
    property<edge_capacity_t, double,
             property<edge_residual_capacity_t, double,
                      property<edge_reverse_t, Traits::edge_descriptor>>>>
    Graph;

// Prepare to do multiple min cuts (cache graphs etc.)
void prepareForMinCuts() {
  totalClasses = 0;
  for (int i = 0; i < classPoints.size(); i++) {
    if (classPoints[i].cls + 1 > totalClasses) {
      totalClasses = classPoints[i].cls + 1;
    }
    if (verboseMode) {
      printf("clspt %d: (%d, %d)\n", classPoints[i].cls, classPoints[i].ri,
             classPoints[i].ci);
    }
  }
  clsMasks = vector<vector<int>>(numClusters, vector<int>(totalClasses, 0));
}

// Perform a min cut against a cls
void minCutCls(int targetCls) {
  Traits::vertex_descriptor s, t;

  Graph g;

  property_map<Graph, edge_capacity_t>::type capacity = get(edge_capacity, g);
  property_map<Graph, edge_reverse_t>::type rev = get(edge_reverse, g);
  property_map<Graph, edge_residual_capacity_t>::type residual_capacity =
      get(edge_residual_capacity, g);

  double flow;

  std::vector<Traits::vertex_descriptor> verts;

  verts.reserve(numClusters + 2);
  for (int i = 0; i < numClusters + 2; i++) {
    verts.push_back(add_vertex(g));
  }

  for (auto &sp1 : spAdjMap) {
    const int vi1 = sp1.first.first, vi2 = sp1.first.second;
    Traits::edge_descriptor e1, e2;
    bool in1, in2;
    boost::tie(e1, in1) = add_edge(verts[vi1], verts[vi2], g);
    boost::tie(e2, in2) = add_edge(verts[vi2], verts[vi1], g);
    if (!in1 || !in2) {
      // std::cerr << "unable to add edge" << std::endl;
      continue;
    }
    capacity[e1] = sp1.second;
    capacity[e2] = sp1.second;
    rev[e1] = e2;
    rev[e2] = e1;
    // for (auto &sp2 : sp1.second) {
    //   const int vi1 = sp1.first, vi2 = sp2.first;
    //   if (vi1 >= vi2)
    //     continue;
    //   Traits::edge_descriptor e1, e2;
    //   bool in1, in2;
    //   boost::tie(e1, in1) = add_edge(verts[vi1], verts[vi2], g);
    //   boost::tie(e2, in2) = add_edge(verts[vi2], verts[vi1], g);
    //   if (!in1 || !in2) {
    //     // std::cerr << "unable to add edge" << std::endl;
    //     continue;
    //   }
    //   capacity[e1] = sp2.second;
    //   capacity[e2] = sp2.second;
    //   rev[e1] = e2;
    //   rev[e2] = e1;
    // }
  }

  s = verts[numClusters];
  t = verts[numClusters + 1];

  std::unordered_set<int> visitedSP;
  // Add source/target edges
  for (int cpi = 0; cpi < classPoints.size(); cpi++) {
    const ClassPoint cp = classPoints[cpi];
    const int cci = clusters[cp.ri][cp.ci];
    if (visitedSP.find(cci) != visitedSP.end())
      continue;
    visitedSP.insert(cci);
    const auto v = verts[cci];

    Traits::vertex_descriptor *sourceorsink;
    if (cp.cls == targetCls) {
      sourceorsink = &s;
    } else {
      sourceorsink = &t;
    }

    Traits::edge_descriptor e1, e2;
    bool in1, in2;
    boost::tie(e1, in1) = add_edge(v, *sourceorsink, g);
    boost::tie(e2, in2) = add_edge(*sourceorsink, v, g);
    if (!in1 || !in2) {
      continue;
    }

    rev[e1] = e2;
    rev[e2] = e1;

    capacity[e1] = 1000;
    capacity[e2] = 1000;
  }

  // write_graphviz(std::cout, g, default_writer(),
  // make_label_writer(capacity));

  flow = boykov_kolmogorov_max_flow(g, s, t);

  if (verboseMode) {
    printf("flow: %f\n", flow);
  }

  // remove edges with zero residual capacity

  // auto epair = edges(g);
  // for (auto ei = epair.first; ei != epair.second; ei++)
  // {
  //     std::cout << "edge: " << source(*ei, g) << "-" << target(*ei, g) << "
  //     " << residual_capacity[*ei] << std::endl;
  // }

  // write_graphviz(std::cout, g, default_writer());

  auto eiter = edges(g);
  std::vector<Traits::edge_descriptor> edgesToRemove;
  for (auto ei = eiter.first; ei != eiter.second; ei++) {
    if (residual_capacity[*ei] == 0) {
      edgesToRemove.push_back(*ei);
      edgesToRemove.push_back(rev[*ei]);
    }
  }

  for (auto edge : edgesToRemove) {
    remove_edge(edge, g);
  }

  // write_graphviz(std::cout, g, default_writer(),
  // make_label_writer(residual_capacity));

  // std::cout << "edges remaining..." << std::endl;

  // auto epair = edges(g);
  // for (auto ei = epair.first; ei != epair.second; ei++)
  // {
  //     std::cout << "edge: " << source(*ei, g) << "-" << target(*ei, g) <<
  //     std::endl;
  //     // remove_edge(*ei, g);
  // }

  // write_graphviz(std::cout, g, default_writer());

  std::vector<int> components(num_vertices(g));
  int totalConnectedComponents = connected_components(g, &components[0]);

  // for (int i = 0; i < components.size(); i++)
  // {
  //     std::cout << "Vertex " << i << " is in component " << components[i] <<
  //     std::endl;
  // }

  int clsComponentIndex = components[numClusters];
  int othComponentIndex = components[numClusters + 1];

  for (int cci = 0; cci < numClusters; cci++) {
    clsMasks[cci][targetCls] = std::numeric_limits<int>::max();
  }

  for (int cci = 0; cci < numClusters; cci++) {
    if (components[cci] == clsComponentIndex) {
      // If connected to source, take distance
      clsMasks[cci][targetCls] = get(vertex_distance, g)[verts[cci]];
    } else {
      // If connected to sink, take inverse distance
      clsMasks[cci][targetCls] =
          maxClusters - get(vertex_distance, g)[verts[cci]];
    }
  }

  // std::cout << clsMasks << std::endl;
}

void resolveMasks() {
  vector<int> resolvedMasks(numClusters);
  for (int cci = 0; cci < numClusters; cci++) {
    int smallestCls = -1;
    int smallestDist = std::numeric_limits<int>::max();
    for (int cls = 0; cls < totalClasses; cls++) {
      if (clsMasks[cci][cls] < smallestDist) {
        smallestDist = clsMasks[cci][cls];
        smallestCls = cls;
      }
    }
    resolvedMasks[cci] = smallestCls;
  }
  if (verboseMode) {
    for (int cci = 0; cci < numClusters; cci++) {
      printf("cci %d: %d\n", cci, resolvedMasks[cci]);
    }
  }
  coloredMask = std::vector<uint32_t>(width * height);
  for (int ri = 0; ri < height; ri++) {
    for (int ci = 0; ci < width; ci++) {
      const int imgi = ri * width + ci;
      const int cci = clusters[ri][ci];
      const int cls = resolvedMasks[cci];

      if (cls == -1) {
        coloredMask[imgi] = 0xffffff88;
      } else {
        coloredMask[imgi] = classToColor[cls];
      }
    }
  }
}
