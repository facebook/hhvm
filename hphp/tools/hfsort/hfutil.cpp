/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/tools/hfsort/hfutil.h"

#include <set>

#include <stdio.h>
#include <assert.h>
#include <zlib.h>
#include <ctype.h>

namespace HPHP { namespace hfsort {

using std::string;
using std::vector;
using std::map;

CallGraph cg;

void CallGraph::clear() {
  for (auto* arc : arcs) {
    delete arc;
  }
  arcs = vector<Arc*>();
  for (auto* cluster : clusters) {
    delete cluster;
  }
  clusters = vector<Cluster*>();
  funcs = vector<Func>();
  addr2FuncId = map<uint64_t, FuncId>();

}

bool compareClustersDensity(const Cluster* c1, const Cluster* c2) {
  return (double) c1->samples / c1->size > (double) c2->samples / c2->size;
}

void freezeClusters(vector<Cluster*>& clusters) {
  uint32_t totalSize = 0;
  sort(clusters.begin(), clusters.end(), compareClustersDensity);
  for (Cluster* cluster : clusters) {
    uint32_t newSize = totalSize + cluster->size;
    if (newSize > kFrozenPages * kPageSize) break;
    cluster->frozen = true;
    totalSize = newSize;
    FuncId fid = cluster->funcs[0];
    HFTRACE(1, "freezing cluster for func %d, size = %u, samples = %u, %s)\n",
            fid, cg.funcs[fid].size, cg.funcs[fid].samples,
            cg.funcs[fid].mangledNames[0].c_str());
  }
}

vector<Cluster*> clusterize() {
  vector<FuncId>   sortedFuncs;
  vector<Cluster*> funcCluster; // indexed by FuncId, keeps it's current cluster
  vector<Cluster*> clusters;

  for (size_t f = 0; f < cg.funcs.size(); f++) {
    if (cg.funcs[f].samples > 0) {
      Cluster* cluster = new Cluster(cg.funcs[f].id);
      sortedFuncs.push_back(f);
      funcCluster.push_back(cluster);
      clusters.push_back(cluster);
    } else {
      funcCluster.push_back(nullptr);
    }
  }

  freezeClusters(clusters);

  sort(sortedFuncs.begin(), sortedFuncs.end(),
       [&](FuncId f1, FuncId f2) {
         auto& func1 = cg.funcs[f1];
         auto& func2 = cg.funcs[f2];
         return
           (uint64_t)func1.samples * func2.size >
           (uint64_t)func2.samples * func1.size;
       });

  // Process each function, and consider merging its cluster with the
  // one containing its most likely predecessor.
  for (FuncId fid : sortedFuncs) {

    Cluster* cluster = funcCluster[fid];
    if (cluster->frozen) continue;

    // Find best predecessor.
    FuncId bestPred = InvalidId;
    double bestProb = 0;

    for (Arc* arc : cg.funcs[fid].inArcs) {
      if (bestPred == InvalidId || arc->normalizedWeight > bestProb) {
        bestPred = arc->src;
        bestProb = arc->normalizedWeight;
      }
    }

    // Check if the merge is good for the callee.
    //   Don't merge if the probability of getting to the callee from the
    //   caller is too low.
    if (bestProb < kMinArcProbability) continue;

    assert(bestPred != InvalidId);

    Cluster* predCluster = funcCluster[bestPred];

    // Skip if no predCluster (predecessor w/ no samples), or if same
    // as cluster, of it's frozen.
    if (predCluster == nullptr || predCluster == cluster ||
        predCluster->frozen) {
      continue;
    }

    // Skip if merged cluster would be bigger than the threshold.
    if (cluster->size + predCluster->size > kMaxClusterSize) continue;

    // Check if the merge is good for the caller.
    //   Don't merge if the caller's density is significantly better
    //   than the density resulting from the merge.
    double newDensity = ((double)predCluster->samples + cluster->samples) /
      (predCluster->size + cluster->size);
    if ((double)predCluster->samples / predCluster->size >
        newDensity * kCallerDegradeFactor) {
      continue;
    }

    HFTRACE(1, "merging %s -> %s: %u\n", predCluster->toString().c_str(),
            cluster->toString().c_str(), cg.funcs[fid].samples);

    for (FuncId f : cluster->funcs) {
      funcCluster[f] = predCluster;
    }

    predCluster->merge(*cluster);
  }

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged (so their first func is it's original func).
  clusters.clear();
  for (FuncId fid : sortedFuncs) {
    Cluster* c = funcCluster[fid];
    if (c->funcs[0] == fid) {
      clusters.push_back(c);
    }
  }

  return clusters;
}

void orderFuncs(Cluster* c1, Cluster* c2) {
  FuncId c1head = c1->funcs[0];
  FuncId c1tail = c1->funcs[c1->funcs.size() - 1];
  FuncId c2head = c2->funcs[0];
  FuncId c2tail = c2->funcs[c2->funcs.size() - 1];

  double c1headc2head = 0;
  double c1headc2tail = 0;
  double c1tailc2head = 0;
  double c1tailc2tail = 0;

  for (Arc* arc : cg.arcs) {
    if ((arc->src == c1head && arc->dst == c2head) ||
        (arc->dst == c1head && arc->src == c2head)) {
      c1headc2head += arc->weight;
    } else if ((arc->src == c1head && arc->dst == c2tail) ||
               (arc->dst == c1head && arc->src == c2tail)) {
      c1headc2tail += arc->weight;
    } else if ((arc->src == c1tail && arc->dst == c2head) ||
               (arc->dst == c1tail && arc->src == c2head)) {
      c1tailc2head += arc->weight;
    } else if ((arc->src == c1tail && arc->dst == c2tail) ||
               (arc->dst == c1tail && arc->src == c2tail)) {
      c1tailc2tail += arc->weight;
    }
  }

  double max = std::max(std::max(c1headc2head, c1headc2tail),
                        std::max(c1tailc2head, c1tailc2tail));

  if (c1headc2head == max) {
    // flip c1
    std::reverse(c1->funcs.begin(), c1->funcs.end());
  } else if (c1headc2tail == max) {
    // flip c1 c2
    std::reverse(c1->funcs.begin(), c1->funcs.end());
    std::reverse(c2->funcs.begin(), c2->funcs.end());
  } else if (c1tailc2tail == max) {
    // flip c2
    std::reverse(c2->funcs.begin(), c2->funcs.end());
  }
}

/*
 * Pettis-Hansen code layout algorithm
 * reference: K. Pettis and R. C. Hansen, "Profile Guided Code Positioning",
 *            PLDI '90
 */

std::vector<Cluster*> pettisAndHansen() {
  vector<Cluster*> funcCluster; // indexed by FuncId, keeps it's current cluster
  vector<Cluster*> clusters;
  vector<FuncId>   funcs;

  for (size_t f = 0; f < cg.funcs.size(); f++) {
    if (cg.funcs[f].samples > 0) {
      Cluster* cluster = new Cluster(cg.funcs[f].id);
      funcCluster.push_back(cluster);
      funcs.push_back(f);
    } else {
      funcCluster.push_back(nullptr);
    }
  }

  vector<ClusterArc*> carcs;

  // Create a vector of cluster arcs

  for (Arc* arc : cg.arcs) {
    if (arc->weight == 0) continue;

    Cluster* s = funcCluster[arc->src];
    Cluster* d = funcCluster[arc->dst];

    // ignore if s or d is nullptr

    if (s == nullptr || d == nullptr) continue;

    // ignore self-edges

    if (s == d) continue;

    ClusterArc* newcarc = new ClusterArc(s, d, arc->weight);

    for (ClusterArc* carc : carcs) {
      if (*newcarc != *carc) continue;

      carc->weight += newcarc->weight;

      delete newcarc;
      newcarc = nullptr;
      break;
    }

    if (newcarc) carcs.push_back(newcarc);
  }

  // Find an arc with max weight and merge its nodes

  while (!carcs.empty()) {
    auto maxpos = max_element(carcs.begin(), carcs.end(),
        [&](ClusterArc* carc1, ClusterArc* carc2) {
          return carc1->weight < carc2->weight;
        });

    ClusterArc* max = *maxpos;
    carcs.erase(maxpos);

    Cluster* c1 = max->c1;
    Cluster* c2 = max->c2;

    if (c1->size + c2->size > kMaxClusterSize) continue;

    if (c1->frozen || c2->frozen) continue;

    // order functions and merge cluster

    orderFuncs(c1, c2);

    HFTRACE(1, "merging %s -> %s: %.1f\n", c2->toString().c_str(),
            c1->toString().c_str(), max->weight);

    c1->merge(*c2, max->weight);

    // update carcs: merge c1arcs to c2arcs

    map<Cluster*, ClusterArc*> c1arcs;
    vector<std::pair<Cluster*, ClusterArc*> > c2arcs;

    for(ClusterArc* carc : carcs) {
      if (carc->c1 == c1) c1arcs[carc->c2] = carc;
      if (carc->c2 == c1) c1arcs[carc->c1] = carc;
      if (carc->c1 == c2) c2arcs.push_back(std::make_pair(carc->c2, carc));
      if (carc->c2 == c2) c2arcs.push_back(std::make_pair(carc->c1, carc));
    }

    for (auto it : c2arcs) {
      Cluster*    c = it.first;
      ClusterArc* c2arc = it.second;
      auto        pos = c1arcs.find(c);

      if (pos != c1arcs.end()) {
        c1arcs[c]->weight += c2arc->weight;
      } else {
        ClusterArc* newcarc = new ClusterArc(c, c1, c2arc->weight);
        carcs.push_back(newcarc);
      }

      carcs.erase(std::find(carcs.begin(), carcs.end(), c2arc));

      delete c2arc;
    }

    // update funcCluster

    for (FuncId f : c2->funcs) funcCluster[f] = c1;

    delete max;
  }

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged.

  std::set<Cluster*> liveClusters;

  for (FuncId fid : funcs) liveClusters.insert(funcCluster[fid]);
  for (Cluster* c : liveClusters) clusters.push_back(c);

  return clusters;
}

} }
