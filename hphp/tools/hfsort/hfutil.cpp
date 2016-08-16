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

bool CallGraph::addFunc(
  std::string name,
  uint64_t addr,
  uint32_t size,
  uint32_t group
) {
  if (name.empty()) return false;
  auto it = addr2TargetId.find(addr);
  if (it != addr2TargetId.end()) {
    auto& base = funcs[it->second];
    auto& baseTarget = targets[it->second];
    base.mangledNames.push_back(name);
    if (size > baseTarget.size) baseTarget.size = size;
    HFTRACE(2, "Func: adding '%s' to (%u)\n", name.c_str(), it->second);
    return true;
  }
  auto id = addTarget(size);
  funcs.emplace_back(name, addr, group);
  addr2TargetId[addr] = id;
  HFTRACE(2, "Func: adding (%u): %016lx %s %u\n", id, (long)addr, name.c_str(),
          size);
  return true;
}

TargetId CallGraph::addrToTargetId(uint64_t addr) const {
  auto it = addr2TargetId.upper_bound(addr);
  if (it == addr2TargetId.begin()) return InvalidId;
  --it;
  const auto &f = funcs[it->second];
  const auto &fTarget = targets[it->second];
  assert(f.addr <= addr);
  if (f.addr + fTarget.size <= addr) {
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
    if (targets[f].samples == 0) continue;
    fprintf(file, "f%lu [label=\"%s\\nsamples=%u\"]\n",
            f, funcs[f].mangledNames[0].c_str(), targets[f].samples);
  }
  for (size_t f = 0; f < funcs.size(); f++) {
    if (targets[f].samples == 0) continue;
    for (auto dst : targets[f].succs) {
      auto& arc = *arcs.find(Arc(f, dst));
      fprintf(
        file,
        "f%lu -> f%u [label=normWgt=%.3lf,weight=%.0lf,callOffset=%.1lf]\n",
        f,
        dst,
        arc.normalizedWeight,
        arc.weight,
        arc.avgCallOffset);
    }
  }
  fprintf(file, "}\n");
}

std::string CallGraph::toString(TargetId id) const {
  return folly::sformat("func = {:5} : samples = {:6} : size = {:6} : {}\n",
                        id, targets[id].samples, targets[id].size,
                        funcs[id].mangledNames[0]);
}

TargetId TargetGraph::addTarget(uint32_t size, uint32_t samples) {
  auto id = targets.size();
  targets.emplace_back(size, samples);
  return id;
}

const Arc& TargetGraph::incArcWeight(TargetId src, TargetId dst, double w) {
  auto res = arcs.emplace(src, dst, w);
  if (!res.second) {
    res.first->weight += w;
    return *res.first;
  }
  targets[src].succs.push_back(dst);
  targets[dst].preds.push_back(src);
  return *res.first;
}

Cluster::Cluster(TargetId id, const Target& f) {
  targets.push_back(id);
  size = f.size;
  arcWeight = 0;
  samples = f.samples;
  frozen = false;
  HFTRACE(1, "new Cluster: %s\n", toString().c_str());
}

std::string Cluster::toString() const {
  return folly::sformat("funcs = [{}]", folly::join(", ", targets));
}

////////////////////////////////////////////////////////////////////////////////

bool compareClustersDensity(const Cluster& c1, const Cluster& c2) {
  return (double) c1.samples / c1.size > (double) c2.samples / c2.size;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
void freezeClusters(const TargetGraph& cg, std::vector<Cluster>& clusters) {
  uint32_t totalSize = 0;
  std::sort(clusters.begin(), clusters.end(), compareClustersDensity);
  for (auto& cluster : clusters) {
    uint32_t newSize = totalSize + cluster.size;
    if (newSize > kFrozenPages * kPageSize) break;
    cluster.frozen = true;
    totalSize = newSize;
    auto fid = cluster.targets[0];
    HFTRACE(1, "freezing cluster for func %d, size = %u, samples = %u)\n",
            fid, cg.targets[fid].size, cg.targets[fid].samples);
  }
}

void mergeInto(Cluster& into, Cluster&& other, const double aw = 0) {
  into.targets.insert(into.targets.end(), other.targets.begin(), other.targets.end());
  into.size += other.size;
  into.samples += other.samples;
  into.arcWeight += (other.arcWeight + aw);

  other.size = 0;
  other.samples = 0;
  other.arcWeight = 0;
  other.targets.clear();
}
}

std::vector<Cluster> clusterize(const TargetGraph& cg) {
  std::vector<TargetId> sortedFuncs;

  // indexed by TargetId, keeps it's current cluster
  std::vector<Cluster*> funcCluster(cg.targets.size(), nullptr);
  std::vector<Cluster> clusters;
  clusters.reserve(cg.targets.size());

  for (size_t f = 0; f < cg.targets.size(); f++) {
    if (cg.targets[f].samples == 0) continue;
    clusters.emplace_back(f, cg.targets[f]);
    sortedFuncs.push_back(f);
  }

  freezeClusters(cg, clusters);

  // the size and order of clusters is fixed until we reshuffle it immediately
  // before returning
  for (auto& cluster : clusters) {
    funcCluster[cluster.targets.front()] = &cluster;
  }

  std::sort(
    sortedFuncs.begin(),
    sortedFuncs.end(),
    [&] (TargetId f1, TargetId f2) {
      auto& func1 = cg.targets[f1];
      auto& func2 = cg.targets[f2];
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
    TargetId bestPred = InvalidId;
    double bestProb = 0;

    for (auto src : cg.targets[fid].preds) {
      auto& arc = *cg.arcs.find(Arc(src, fid));
      if (bestPred == InvalidId || arc.normalizedWeight > bestProb) {
        bestPred = arc.src;
        bestProb = arc.normalizedWeight;
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
            cluster->toString().c_str(), cg.targets[fid].samples);

    for (auto f : cluster->targets) {
      funcCluster[f] = predCluster;
    }

    mergeInto(*predCluster, std::move(*cluster));
  }

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged (so their first func is its original func).
  std::vector<Cluster> sortedClusters;
  for (auto func : sortedFuncs) {
    auto cluster = funcCluster[func];
    if (!cluster || cluster->targets.empty()) continue;
    if (cluster->targets[0] != func) continue;
    sortedClusters.emplace_back(std::move(*cluster));
    cluster->targets.clear();
  }

  return sortedClusters;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
struct ClusterArc {
  ClusterArc(Cluster* ca, Cluster* cb, double w = 0)
    : c1(std::min(ca, cb))
    , c2(std::max(ca, cb))
    , weight(w)
  {}

  friend bool operator==(const ClusterArc& lhs, const ClusterArc& rhs) {
    return lhs.c1 == rhs.c1 && lhs.c2 == rhs.c2;
  }

  Cluster* const c1;
  Cluster* const c2;
  mutable double weight;
};

struct ClusterArcHash {
  int64_t operator()(const ClusterArc& arc) const {
    return hash_int64_pair(int64_t(arc.c1), int64_t(arc.c2));
  }
};

using ClusterArcSet = std::unordered_set<ClusterArc, ClusterArcHash>;

void orderFuncs(const TargetGraph& cg, Cluster* c1, Cluster* c2) {
  TargetId c1head = c1->targets[0];
  TargetId c1tail = c1->targets[c1->targets.size() - 1];
  TargetId c2head = c2->targets[0];
  TargetId c2tail = c2->targets[c2->targets.size() - 1];

  double c1headc2head = 0;
  double c1headc2tail = 0;
  double c1tailc2head = 0;
  double c1tailc2tail = 0;

  for (auto& arc : cg.arcs) {
    if ((arc.src == c1head && arc.dst == c2head) ||
        (arc.dst == c1head && arc.src == c2head)) {
      c1headc2head += arc.weight;
    } else if ((arc.src == c1head && arc.dst == c2tail) ||
               (arc.dst == c1head && arc.src == c2tail)) {
      c1headc2tail += arc.weight;
    } else if ((arc.src == c1tail && arc.dst == c2head) ||
               (arc.dst == c1tail && arc.src == c2head)) {
      c1tailc2head += arc.weight;
    } else if ((arc.src == c1tail && arc.dst == c2tail) ||
               (arc.dst == c1tail && arc.src == c2tail)) {
      c1tailc2tail += arc.weight;
    }
  }

  double max = std::max(std::max(c1headc2head, c1headc2tail),
                        std::max(c1tailc2head, c1tailc2tail));

  if (c1headc2head == max) {
    // flip c1
    std::reverse(c1->targets.begin(), c1->targets.end());
  } else if (c1headc2tail == max) {
    // flip c1 c2
    std::reverse(c1->targets.begin(), c1->targets.end());
    std::reverse(c2->targets.begin(), c2->targets.end());
  } else if (c1tailc2tail == max) {
    // flip c2
    std::reverse(c2->targets.begin(), c2->targets.end());
  }
}
}

std::vector<Cluster> pettisAndHansen(const TargetGraph& cg) {
  // indexed by TargetId, keeps its current cluster
  std::vector<Cluster*> funcCluster(cg.targets.size(), nullptr);
  std::vector<Cluster> clusters;
  std::vector<TargetId> funcs;

  clusters.reserve(cg.targets.size());

  for (size_t f = 0; f < cg.targets.size(); f++) {
    if (cg.targets[f].samples == 0) continue;
    clusters.emplace_back(f, cg.targets[f]);
    funcCluster[f] = &clusters.back();
    funcs.push_back(f);
  }

  ClusterArcSet carcs;

  auto insertOrInc = [&](Cluster* c1, Cluster* c2, double weight) {
    auto res = carcs.emplace(c1, c2, weight);
    if (!res.second) {
      res.first->weight += weight;
    }
  };

  // Create a vector of cluster arcs

  for (auto& arc : cg.arcs) {
    if (arc.weight == 0) continue;

    auto const s = funcCluster[arc.src];
    auto const d = funcCluster[arc.dst];

    // ignore if s or d is nullptr

    if (s == nullptr || d == nullptr) continue;

    // ignore self-edges

    if (s == d) continue;

    insertOrInc(s, d, arc.weight);
  }

  // Find an arc with max weight and merge its nodes

  while (!carcs.empty()) {
    auto maxpos = std::max_element(
      carcs.begin(),
      carcs.end(),
      [&] (const ClusterArc& carc1, const ClusterArc& carc2) {
        return carc1.weight < carc2.weight;
      }
    );

    auto max = *maxpos;
    carcs.erase(maxpos);

    auto const c1 = max.c1;
    auto const c2 = max.c2;

    if (c1->size + c2->size > kMaxClusterSize) continue;

    if (c1->frozen || c2->frozen) continue;

    // order functions and merge cluster

    orderFuncs(cg, c1, c2);

    HFTRACE(1, "merging %s -> %s: %.1f\n", c2->toString().c_str(),
            c1->toString().c_str(), max.weight);

    // update carcs: merge c1arcs to c2arcs

    std::unordered_map<ClusterArc, Cluster*, ClusterArcHash> c2arcs;
    for (auto& carc : carcs) {
      if (carc.c1 == c2) c2arcs.emplace(carc, carc.c2);
      if (carc.c2 == c2) c2arcs.emplace(carc, carc.c1);
    }

    for (auto it : c2arcs) {
      auto const c = it.second;
      auto const c2arc = it.first;

      insertOrInc(c, c1, c2arc.weight);
      carcs.erase(c2arc);
    }

    // update funcCluster

    for (auto f : c2->targets) funcCluster[f] = c1;
    mergeInto(*c1, std::move(*c2), max.weight);
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
