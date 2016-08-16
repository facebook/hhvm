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

#include <folly/Format.h>
#include <folly/Memory.h>

namespace HPHP { namespace hfsort {

bool CallGraph::addFunc(Func f) {
  if (!f.valid()) return false;
  auto it = addr2FuncId.find(f.addr);
  if (it != addr2FuncId.end()) {
    Func& base = funcs[it->second];
    base.mangledNames.push_back(f.mangledNames[0]);
    if (f.size > base.size) base.size = f.size;
    HFTRACE(2, "Func: adding '%s' to (%u)\n",
            f.mangledNames[0].c_str(),
            base.id);
    return true;
  }
  funcs.push_back(f);
  addr2FuncId[f.addr] = f.id;
  HFTRACE(2, "Func: adding (%u): %016lx %s %u\n",
          f.id,
          (long)f.addr,
          f.mangledNames[0].c_str(),
          f.size);
  return true;
}

Arc* CallGraph::incArcWeight(FuncId src, FuncId dst, double w) {
  for (auto arc : funcs[src].outArcs) {
    if (arc->dst == dst) {
      arc->weight += w;
      return arc;
    }
  }

  arcs.emplace_back(folly::make_unique<Arc>(src, dst, w));
  auto arc = arcs.back().get();
  funcs[src].outArcs.push_back(arc);
  funcs[dst].inArcs.push_back(arc);
  return arc;
}

FuncId CallGraph::addrToFuncId(uint64_t addr) const {
  auto it = addr2FuncId.upper_bound(addr);
  if (it == addr2FuncId.begin()) return InvalidId;
  --it;
  const auto &f = funcs[it->second];
  assert(f.addr <= addr);
  if (f.addr + f.size <= addr) {
    return InvalidId;
  }
  return it->second;
}

void CallGraph::printDot(char* fileName) const {
  FILE* file = fopen(fileName, "wt");
  if (!file) return;
  SCOPE_EXIT { fclose(file); };

  fprintf(file, "digraph g {\n");
  for (size_t f = 0; f < funcs.size(); f++) {
    if (funcs[f].samples == 0) continue;
    fprintf(file, "f%lu [label=\"%s\\nsamples=%u\"]\n",
            f, funcs[f].mangledNames[0].c_str(), funcs[f].samples);
  }
  for (size_t f = 0; f < funcs.size(); f++) {
    if (funcs[f].samples == 0) continue;
    for (size_t i = 0; i < funcs[f].outArcs.size(); i++) {
      auto arc = funcs[f].outArcs[i];
      fprintf(
        file,
        "f%lu -> f%u [label=normWgt=%.3lf,weight=%.0lf,callOffset=%.1lf]\n",
        f,
        arc->dst,
        arc->normalizedWeight,
        arc->weight,
        arc->avgCallOffset);
    }
  }
  fprintf(file, "}\n");
}

std::string Func::toString() const {
  return folly::sformat("func = {:5} : samples = {:6} : size = {:6} : {}\n",
                        id, samples, size, mangledNames[0]);
}

Cluster::Cluster(const Func& f) {
  funcs.push_back(f.id);
  size = f.size;
  arcWeight = 0;
  samples = f.samples;
  frozen = false;
  HFTRACE(1, "new Cluster: %s: %s\n", toString().c_str(), f.toString().c_str());
}

std::string Cluster::toString() const {
  return folly::sformat("funcs = [{}]", folly::join(", ", funcs));
}

////////////////////////////////////////////////////////////////////////////////

bool compareClustersDensity(const Cluster& c1, const Cluster& c2) {
  return (double) c1.samples / c1.size > (double) c2.samples / c2.size;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
void freezeClusters(const CallGraph& cg, std::vector<Cluster>& clusters) {
  uint32_t totalSize = 0;
  std::sort(clusters.begin(), clusters.end(), compareClustersDensity);
  for (auto& cluster : clusters) {
    uint32_t newSize = totalSize + cluster.size;
    if (newSize > kFrozenPages * kPageSize) break;
    cluster.frozen = true;
    totalSize = newSize;
    auto fid = cluster.funcs[0];
    HFTRACE(1, "freezing cluster for func %d, size = %u, samples = %u, %s)\n",
            fid, cg.funcs[fid].size, cg.funcs[fid].samples,
            cg.funcs[fid].mangledNames[0].c_str());
  }
}

void mergeInto(Cluster& into, Cluster&& other, const double aw = 0) {
  into.funcs.insert(into.funcs.end(), other.funcs.begin(), other.funcs.end());
  into.size += other.size;
  into.samples += other.samples;
  into.arcWeight += (other.arcWeight + aw);

  other.size = 0;
  other.samples = 0;
  other.arcWeight = 0;
  other.funcs.clear();
}
}

std::vector<Cluster> clusterize(const CallGraph& cg) {
  std::vector<FuncId> sortedFuncs;

  // indexed by FuncId, keeps it's current cluster
  std::vector<Cluster*> funcCluster(cg.funcs.size(), nullptr);
  std::vector<Cluster> clusters;
  clusters.reserve(cg.funcs.size());

  for (size_t f = 0; f < cg.funcs.size(); f++) {
    if (cg.funcs[f].samples == 0) continue;
    clusters.emplace_back(cg.funcs[f]);
    sortedFuncs.push_back(f);
  }

  freezeClusters(cg, clusters);

  // the size and order of clusters is fixed until we reshuffle it immediately
  // before returning
  for (auto& cluster : clusters) {
    funcCluster[cluster.funcs.front()] = &cluster;
  }

  std::sort(
    sortedFuncs.begin(),
    sortedFuncs.end(),
    [&] (FuncId f1, FuncId f2) {
      auto& func1 = cg.funcs[f1];
      auto& func2 = cg.funcs[f2];
      return
        (uint64_t)func1.samples * func2.size >
        (uint64_t)func2.samples * func1.size;
    }
  );

  // Process each function, and consider merging its cluster with the
  // one containing its most likely predecessor.
  for (auto fid : sortedFuncs) {
    auto cluster = funcCluster[fid];
    if (cluster->frozen) continue;

    // Find best predecessor.
    FuncId bestPred = InvalidId;
    double bestProb = 0;

    for (auto arc : cg.funcs[fid].inArcs) {
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

    auto predCluster = funcCluster[bestPred];

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

    for (auto f : cluster->funcs) {
      funcCluster[f] = predCluster;
    }

    mergeInto(*predCluster, std::move(*cluster));
  }

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged (so their first func is its original func).
  std::vector<Cluster> sortedClusters;
  for (auto func : sortedFuncs) {
    auto cluster = funcCluster[func];
    if (!cluster || cluster->funcs.empty()) continue;
    if (cluster->funcs[0] != func) continue;
    sortedClusters.emplace_back(std::move(*cluster));
    cluster->funcs.clear();
  }

  return sortedClusters;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
struct ClusterArc {
  ClusterArc(Cluster* c1, Cluster* c2, double w)
    : c1(c1)
    , c2(c2)
    , weight(w)
  {}

  Cluster* c1;
  Cluster* c2;
  double weight;
};

void orderFuncs(const CallGraph& cg, Cluster* c1, Cluster* c2) {
  FuncId c1head = c1->funcs[0];
  FuncId c1tail = c1->funcs[c1->funcs.size() - 1];
  FuncId c2head = c2->funcs[0];
  FuncId c2tail = c2->funcs[c2->funcs.size() - 1];

  double c1headc2head = 0;
  double c1headc2tail = 0;
  double c1tailc2head = 0;
  double c1tailc2tail = 0;

  for (auto& arc : cg.arcs) {
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
}

std::vector<Cluster> pettisAndHansen(const CallGraph& cg) {
  // indexed by FuncId, keeps its current cluster
  std::vector<Cluster*> funcCluster(cg.funcs.size(), nullptr);
  std::vector<Cluster> clusters;
  std::vector<FuncId> funcs;

  clusters.reserve(cg.funcs.size());

  for (size_t f = 0; f < cg.funcs.size(); f++) {
    if (cg.funcs[f].samples == 0) continue;
    clusters.emplace_back(cg.funcs[f]);
    funcCluster[f] = &clusters.back();
    funcs.push_back(f);
  }

  std::vector<std::unique_ptr<ClusterArc>> carcs;

  // Create a vector of cluster arcs

  for (auto& arc : cg.arcs) {
    if (arc->weight == 0) continue;

    auto const s = funcCluster[arc->src];
    auto const d = funcCluster[arc->dst];

    // ignore if s or d is nullptr

    if (s == nullptr || d == nullptr) continue;

    // ignore self-edges

    if (s == d) continue;

    bool insert = [&] {
      for (auto& a : carcs) {
        if ((a->c1 == s && a->c2 == d) || (a->c1 == d && a->c2 == s)) {
          a->weight += arc->weight;
          return false;
        }
      }
      return true;
    }();

    if (insert) {
      carcs.emplace_back(
        folly::make_unique<ClusterArc>(s, d, arc->weight)
      );
    }
  }

  // Find an arc with max weight and merge its nodes

  while (!carcs.empty()) {
    auto maxpos = std::max_element(
      carcs.begin(),
      carcs.end(),
      [&] (const std::unique_ptr<ClusterArc>& carc1,
           const std::unique_ptr<ClusterArc>& carc2) {
        return carc1->weight < carc2->weight;
      }
    );

    auto max = std::move(*maxpos);
    carcs.erase(maxpos);

    auto const c1 = max->c1;
    auto const c2 = max->c2;

    if (c1->size + c2->size > kMaxClusterSize) continue;

    if (c1->frozen || c2->frozen) continue;

    // order functions and merge cluster

    orderFuncs(cg, c1, c2);

    HFTRACE(1, "merging %s -> %s: %.1f\n", c2->toString().c_str(),
            c1->toString().c_str(), max->weight);

    auto funcList = c2->funcs;
    mergeInto(*c1, std::move(*c2), max->weight);

    // update carcs: merge c1arcs to c2arcs

    std::map<Cluster*, ClusterArc*> c1arcs;
    std::vector<std::pair<Cluster*, ClusterArc*>> c2arcs;

    for (auto& carc : carcs) {
      auto carcp = carc.get();
      if (carc->c1 == c1) c1arcs[carc->c2] = carcp;
      if (carc->c2 == c1) c1arcs[carc->c1] = carcp;
      if (carc->c1 == c2) c2arcs.push_back(std::make_pair(carc->c2, carcp));
      if (carc->c2 == c2) c2arcs.push_back(std::make_pair(carc->c1, carcp));
    }

    for (auto it : c2arcs) {
      auto const c = it.first;
      auto const c2arc = it.second;
      auto const pos = c1arcs.find(c);

      if (pos != c1arcs.end()) {
        c1arcs[c]->weight += c2arc->weight;
      } else {
        carcs.emplace_back(
          folly::make_unique<ClusterArc>(c, c1, c2arc->weight)
        );
      }

      carcs.erase(std::find_if(
        carcs.begin(),
        carcs.end(),
        [&] (const std::unique_ptr<ClusterArc>& a) { return a.get() == c2arc; }
      ));
    }

    // update funcCluster

    for (auto f : funcList) funcCluster[f] = c1;
  }

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged.

  std::set<Cluster*> liveClusters;
  std::vector<Cluster> outClusters;

  for (auto fid : funcs) liveClusters.insert(funcCluster[fid]);
  for (auto c : liveClusters) outClusters.push_back(std::move(*c));

  return outClusters;
}

}}
