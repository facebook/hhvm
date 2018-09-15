/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/hfsort.h"

#include <set>
#include <unordered_map>

#include <folly/Format.h>

#include "hphp/util/hash.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace hfsort {
namespace {

#define HFTRACE(LEVEL, ...)                         \
  if (Trace::moduleEnabled(Trace::hfsort, LEVEL)) { \
    Trace::traceRelease(__VA_ARGS__);               \
  }

// The size of a cache page
// Since we optimize both for iTLB cache (2MB pages) and i-cache (64b pages),
// using a value that fits both
constexpr uint32_t kPageSize = uint32_t(1) << 12;

// Capacity of the iTLB cache: larger values yield more iTLB-friendly result,
// while smaller values result in better i-cache performance
constexpr uint32_t kITLBEntries = 16;

constexpr size_t kInvalidAddr = -1;

// A cache of precomputed results for a pair of clusters
class PrecomputedResults {
 public:
  PrecomputedResults() {}

  bool contains(Cluster* first, Cluster* second) const {
    if (invalidKeys.count(first) || invalidKeys.count(second)) {
      return false;
    }
    auto key = std::make_pair(first, second);
    return cache.find(key) != cache.end();
  }

  double get(Cluster* first, Cluster* second) const {
    auto key = std::make_pair(first, second);
    auto it = cache.find(key);
    assert(it != cache.end());
    return it->second;
  }

  void set(Cluster* first, Cluster* second, double value) {
    auto key = std::make_pair(first, second);
    cache[key] = value;
  }

  void validateAll() {
    invalidKeys.clear();
  }

  void invalidate(Cluster* cluster) {
    invalidKeys.insert(cluster);
  }

 private:
  std::unordered_map<std::pair<Cluster*, Cluster*>, double> cache;
  std::unordered_set<Cluster*> invalidKeys;
};

// A wrapper for algorthm-wide variables
struct AlgoState {
  // the call graph
  const TargetGraph* cg;
  // the total number of samples in the graph
  double totalSamples;
  // target_id => cluster
  std::vector<Cluster*> funcCluster;
  // current address of the function from the beginning of its cluster
  std::vector<size_t> addr;
};

}

/*
 * Sorting clusters by their density in decreasing order
 */
void sortByDensity(std::vector<Cluster*>& clusters) {
  std::sort(
    clusters.begin(),
    clusters.end(),
    [&] (const Cluster* c1, const Cluster* c2) {
      double d1 = c1->density();
      double d2 = c2->density();
      // making sure the sorting is deterministic
      if (d1 != d2) return d1 > d2;
      if (c1->size != c2->size) return c1->size < c2->size;
      if (c1->samples != c2->samples) return c1->samples > c2->samples;
      return c1->targets[0] < c2->targets[0];
    }
  );
}

/*
 * Density of a cluster formed by merging a given pair of clusters
 */
double density(Cluster* clusterPred, Cluster* clusterSucc) {
  double combinedSamples = clusterPred->samples + clusterSucc->samples;
  double combinedSize = clusterPred->size + clusterSucc->size;
  return combinedSamples / combinedSize;
}

/*
 * The probability that a page with a given weight is not present in the cache.
 *
 * Assume that the hot function are called in a random order; then the
 * probability of a TLB page being accessed after a function call is
 * p=pageSamples/totalSamples. The probability that the page is not accessed
 * is (1-p), and the probability that it is not in the cache (i.e. not accessed
 * during the last kITLBEntries function calls) is (1-p)^kITLBEntries
 */
double missProbability(const AlgoState& state, double pageSamples) {
  double p = pageSamples / state.totalSamples;
  double x = kITLBEntries;
  // avoiding precision issues for small values
  if (p < 0.0001) return (1.0 - x * p + x * (x - 1.0) * p * p / 2.0);
  return pow(1.0 - p, x);
}

/*
 * Expected hit ratio of the iTLB cache under the given order of clusters
 *
 * Given an ordering of hot functions (and hence, their assignment to the
 * iTLB pages), we can divide all functions calls into two categories:
 * - 'short' ones that have a caller-callee distance less than a page;
 * - 'long' ones where the distance exceeds a page.
 * The short calls are likely to result in a iTLB cache hit. For the long ones,
 * the hit/miss result depends on the 'hotness' of the page (i.e., how often
 * the page is accessed). Assuming that functions are sent to the iTLB cache
 * in a random order, the probability that a page is present in the cache is
 * proportional to the number of samples corresponding to the functions on the
 * page. The following procedure detects short and long calls, and estimates
 * the expected number of cache misses for the long ones.
 */
double expectedCacheHitRatio(const AlgoState& state,
                             const std::vector<Cluster*>& clusters_) {
  // copy and sort by density
  std::vector<Cluster*> clusters(clusters_);
  sortByDensity(clusters);

  // generate function addresses with an alignment
  std::vector<size_t> addr(state.cg->targets.size(), kInvalidAddr);
  size_t curAddr = 0;
  // 'hotness' of the pages
  std::vector<double> pageSamples;
  for (auto cluster : clusters) {
    for (auto targetId : cluster->targets) {
      if (curAddr & 0xf) curAddr = (curAddr & ~0xf) + 16;
      addr[targetId] = curAddr;
      curAddr += state.cg->targets[targetId].size;
      // update page weight
      size_t page = addr[targetId] / kPageSize;
      while (pageSamples.size() <= page) pageSamples.push_back(0.0);
      pageSamples[page] += state.cg->targets[targetId].samples;
    }
  }

  // computing expected number of misses for every function
  double misses = 0;
  for (auto cluster : clusters) {
    for (auto targetId : cluster->targets) {
      size_t page = addr[targetId] / kPageSize;
      double samples = state.cg->targets[targetId].samples;
      // probability that the page is not present in the cache
      double missProb = missProbability(state, pageSamples[page]);

      for (auto pred : state.cg->targets[targetId].preds) {
        if (state.cg->targets[pred].samples == 0) continue;
        auto arc = state.cg->arcs.find(Arc(pred, targetId));

        // the source page
        size_t srcPage = (addr[pred] + (size_t)arc->avgCallOffset) / kPageSize;
        if (page != srcPage) {
          // this is a miss
          misses += arc->weight * missProb;
        }
        samples -= arc->weight;
      }

      // the remaining samples come from the jitted code
      misses += samples * missProb;
    }
  }

  return 100.0 * (1.0 - misses / state.totalSamples);
}

/*
 * Get adjacent clusters (the ones that share an arc) with the given one
 */
std::unordered_set<Cluster*> adjacentClusters(const AlgoState& state,
                                              Cluster* cluster) {
  std::unordered_set<Cluster*> result;
  for (auto targetId : cluster->targets) {
    for (auto succ : state.cg->targets[targetId].succs) {
      auto succCluster = state.funcCluster[succ];
      if (succCluster != nullptr && succCluster != cluster) {
        result.insert(succCluster);
      }
    }
    for (auto pred : state.cg->targets[targetId].preds) {
      auto predCluster = state.funcCluster[pred];
      if (predCluster != nullptr && predCluster != cluster) {
        result.insert(predCluster);
      }
    }
  }
  return result;
}

/*
 * The expected number of calls for an edge withing the same TLB page
 */
double expectedCalls(int src_addr, int dst_addr, double edgeWeight) {
  int dist = std::abs(src_addr - dst_addr);
  if (dist > kPageSize) {
    return 0;
  }
  return (double(kPageSize - dist) / kPageSize) * edgeWeight;
}

/*
 * The expected number of calls within a given cluster with both endpoints on
 * the same TLB cache page
 */
double shortCalls(const AlgoState& state, Cluster* cluster) {
  double calls = 0;
  for (auto targetId : cluster->targets) {
    for (auto succ : state.cg->targets[targetId].succs) {
      if (state.funcCluster[succ] == cluster) {
        auto arc = state.cg->arcs.find(Arc(targetId, succ));

        int src_addr = state.addr[targetId] + arc->avgCallOffset;
        int dst_addr = state.addr[succ];

        calls += expectedCalls(src_addr, dst_addr, arc->weight);
      }
    }
  }

  return calls;
}

/*
 * The number of calls between the two clusters with both endpoints on
 * the same TLB page, assuming that a given pair of clusters gets merged
 */
double shortCalls(const AlgoState& state,
                  Cluster* clusterPred,
                  Cluster* clusterSucc) {
  double calls = 0;
  for (auto targetId : clusterPred->targets) {
    for (auto succ : state.cg->targets[targetId].succs) {
      if (state.funcCluster[succ] == clusterSucc) {
        auto arc = state.cg->arcs.find(Arc(targetId, succ));

        int src_addr = state.addr[targetId] + arc->avgCallOffset;
        int dst_addr = state.addr[succ] + clusterPred->size;

        calls += expectedCalls(src_addr, dst_addr, arc->weight);
      }
    }
  }

  for (auto targetId : clusterPred->targets) {
    for (auto pred : state.cg->targets[targetId].preds) {
      if (state.funcCluster[pred] == clusterSucc) {
        auto arc = state.cg->arcs.find(Arc(pred, targetId));

        int src_addr = state.addr[pred] + arc->avgCallOffset +
          clusterPred->size;
        int dst_addr = state.addr[targetId];

        calls += expectedCalls(src_addr, dst_addr, arc->weight);
      }
    }
  }

  return calls;
}

/*
 * The gain of merging two clusters.
 *
 * We assume that the final clusters are sorted by their density, and hence
 * every cluster is likely to be adjacent with clusters of the same density.
 * Thus, the 'hotness' of every cluster can be estimated by density*pageSize,
 * which is used to compute the probability of cache misses for long calls
 * of a given cluster.
 * The result is also scaled by the size of the resulting cluster in order to
 * increse the chance of merging short clusters, which is helpful for
 * the i-cache performance.
 */
double mergeGain(const AlgoState& state,
                 Cluster* clusterPred,
                 Cluster* clusterSucc) {
  // cache misses on the first cluster
  double longCallsPred = clusterPred->samples - shortCalls(state, clusterPred);
  double probPred = missProbability(state, clusterPred->density() * kPageSize);
  double expectedMissesPred = longCallsPred * probPred;

  // cache misses on the second cluster
  double longCallsSucc = clusterSucc->samples - shortCalls(state, clusterSucc);
  double probSucc = missProbability(state, clusterSucc->density() * kPageSize);
  double expectedMissesSucc = longCallsSucc * probSucc;

  // cache misses on the merged cluster
  double longCallsNew = longCallsPred + longCallsSucc -
                        shortCalls(state, clusterPred, clusterSucc);
  double newDensity = density(clusterPred, clusterSucc);
  double probNew = missProbability(state, newDensity * kPageSize);
  double missesNew = longCallsNew * probNew;

  double gain = expectedMissesPred + expectedMissesSucc - missesNew;
  // scaling the result to increase the importance of merging short clusters
  return gain / (clusterPred->size + clusterSucc->size);
}

 /*
  * Merge two clusters
  */
void mergeInto(AlgoState& state, Cluster* into, Cluster* other) {
  auto& targets = other->targets;
  into->targets.insert(into->targets.end(), targets.begin(), targets.end());
  into->size += other->size;
  into->samples += other->samples;

  size_t curAddr = 0;
  for (auto targetId : into->targets) {
    state.funcCluster[targetId] = into;
    state.addr[targetId] = curAddr;
    curAddr += state.cg->targets[targetId].size;
  }

  other->size = 0;
  other->samples = 0;
  other->targets.clear();
}

/*
 * HFSortPlus - layout of hot functions with iTLB cache optimization
 */
std::vector<Cluster> hfsortPlus(const TargetGraph& cg) {
  // create a cluster for every function
  std::vector<Cluster> allClusters;
  allClusters.reserve(cg.targets.size());
  for (size_t f = 0; f < cg.targets.size(); f++) {
    allClusters.emplace_back(f, cg.targets[f]);
  }

  // initialize objects used by the algorithm
  std::vector<Cluster*> clusters;
  clusters.reserve(cg.targets.size());
  AlgoState state;
  state.cg = &cg;
  state.totalSamples = 0;
  state.funcCluster = std::vector<Cluster*>(cg.targets.size(), nullptr);
  state.addr = std::vector<size_t>(cg.targets.size(), kInvalidAddr);
  for (size_t f = 0; f < cg.targets.size(); f++) {
    if (cg.targets[f].samples == 0) continue;

    clusters.push_back(&allClusters[f]);
    state.funcCluster[f] = &allClusters[f];
    state.addr[f] = 0;
    state.totalSamples += cg.targets[f].samples;
  }

  HFTRACE(1, "Starting hfsort+ for %lu clusters\n", clusters.size());
  HFTRACE(1, "Initial expected iTLB cache hit ratio: %.4lf\n",
    expectedCacheHitRatio(state, clusters));

  // the cache keeps precomputed values of mergeGain for pairs of clusters;
  // when a pair of clusters (x,y) gets merged, we need to invalidate the pairs
  // containing both x and y (and recompute them on the next iteration)
  PrecomputedResults cache;

  int steps = 0;
  // merge pairs of clusters while there is an improvement
  while (clusters.size() > 1) {
    if (steps % 500 == 0) {
      HFTRACE(1, "step = %d  clusters = %lu  expected_hit_rate = %.4lf\n",
        steps,
        clusters.size(),
        expectedCacheHitRatio(state, clusters));
    }
    steps++;

    Cluster* bestClusterPred = nullptr;
    Cluster* bestClusterSucc = nullptr;
    double bestGain = -1;
    for (auto clusterPred : clusters) {
      // get candidates for merging with the current cluster
      auto candidateClusters = adjacentClusters(state, clusterPred);

      // find the best candidate
      for (auto clusterSucc : candidateClusters) {
        // get a cost of merging two clusters
        if (!cache.contains(clusterPred, clusterSucc)) {
          double value = mergeGain(state, clusterPred, clusterSucc);
          cache.set(clusterPred, clusterSucc, value);
        }

        double gain = cache.get(clusterPred, clusterSucc);
        // breaking ties by density to make the hottest clusters be merged first
        if (gain > bestGain || (std::abs(gain - bestGain) < 1e-8 &&
                                density(clusterPred, clusterSucc) >
                                density(bestClusterPred, bestClusterSucc))) {
          bestGain = gain;
          bestClusterPred = clusterPred;
          bestClusterSucc = clusterSucc;
        }
      }
    }
    cache.validateAll();

    if (bestGain <= 0.0) break;

    cache.invalidate(bestClusterPred);
    cache.invalidate(bestClusterSucc);

    // merge the best pair of clusters
    mergeInto(state, bestClusterPred, bestClusterSucc);
    // remove bestClusterSucc from the list of active clusters
    auto iter = std::remove(clusters.begin(), clusters.end(), bestClusterSucc);
    clusters.erase(iter, clusters.end());
  }

  HFTRACE(1, "Completed hfsort+ with %lu clusters\n", clusters.size());
  HFTRACE(1, "Final expected iTLB cache hit ratio: %.4lf\n",
    expectedCacheHitRatio(state, clusters));

  // Return the set of clusters that are left, which are the ones that
  // didn't get merged (so their first func is its original func).
  sortByDensity(clusters);
  std::vector<Cluster> result;
  for (auto cluster : clusters) {
    result.emplace_back(std::move(*cluster));
  }
  return result;
}

}}
