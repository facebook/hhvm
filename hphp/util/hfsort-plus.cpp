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
#include "hphp/util/hash.h"
#include "hphp/util/trace.h"

#include <folly/Format.h>

#include <set>

namespace HPHP { namespace hfsort {

#define HFTRACE(LEVEL, ...)                                     \
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hfsort, LEVEL)) { \
    HPHP::Trace::traceRelease(__VA_ARGS__);                     \
}

namespace {

// The size of a cache page. Since we optimize both for iTLB cache (2MB pages)
// and i-cache (64b pages), using a value that fits both
constexpr uint64_t kCachePageSize = 4096;

// Capacity of the iTLB cache: larger values yield more iTLB-friendly result,
// while smaller values result in better i-cache performance
constexpr uint64_t kCacheEntries = 16;

// The maximum distance (in bytes) of an arc considered for optimization
constexpr uint64_t kMaxArcDistance = 4096;

// A threshold used to filter out edges whose relative weight is smaller than
// the value
constexpr double kArcThreshold = 0.00000001;

// The minimum probability of a call for merging two chains
constexpr double kMergeProbability = 0.9;

// Epsilon for comparison of doubles
constexpr double EPS = 1e-8;

class Edge;
using ArcList = std::vector<const Arc*>;

/*
 * A chain (ordered sequence) of nodes (functions) in the call graph
 */
class Chain {
public:
  Chain(const Chain&) = delete;
  Chain(Chain&&) = default;
  Chain& operator=(const Chain&) = delete;
  Chain& operator=(Chain&&) = default;

  explicit Chain(size_t id, TargetId node, size_t samples, size_t size)
    : id(id),
      samples(samples),
      size(size),
      nodes(1, node) {}

  double density() const {
    return static_cast<double>(samples) / size;
  }

  Edge* getEdge(Chain* other) const {
    for (auto it : edges) {
      if (it.first == other)
        return it.second;
    }
    return nullptr;
  }

  void removeEdge(Chain* other) {
    auto it = edges.begin();
    while (it != edges.end()) {
      if (it->first == other) {
        edges.erase(it);
        return;
      }
      it++;
    }
  }

  void addEdge(Chain* other, Edge* edge) {
    edges.push_back(std::make_pair(other, edge));
  }

  void merge(Chain* other) {
    nodes.insert(nodes.end(), other->nodes.begin(), other->nodes.end());
    samples += other->samples;
    size += other->size;
  }

  void mergeEdges(Chain* other);

  void clear() {
    nodes.clear();
    edges.clear();
  }

public:
  size_t id;
  uint64_t samples;
  uint64_t size;
  // Cached score for the chain
  double score{0};
  // Cached short-calls for the chain
  double shortCalls{0};
  // Nodes in the chain
  std::vector<TargetId> nodes;
  // Adjacent chains and corresponding edges (lists of arcs)
  std::vector<std::pair<Chain*, Edge*>> edges;
};

/*
 * An edge in the call graph representing Arcs between two Chains.
 * When functions are merged into chains, the edges are combined too so that
 * there is always at most one edge between a pair of chains
 */
class Edge {
public:
  Edge(const Edge&) = delete;
  Edge(Edge&&) = default;
  Edge& operator=(const Edge&) = delete;
  Edge& operator=(Edge&&) = default;

  explicit Edge(Chain* srcChain, Chain* dstChain, const Arc *arc)
    : srcChain(srcChain),
      dstChain(dstChain),
      arcs(1, arc) {}

  void changeEndpoint(Chain* from, Chain* to) {
    if (from == srcChain) {
      srcChain = to;
    }
    if (from == dstChain) {
      dstChain = to;
    }
  }

  void moveArcs(Edge* other) {
    arcs.insert(arcs.end(), other->arcs.begin(), other->arcs.end());
    other->arcs.clear();
  }

  void setMergeGain(Chain* predChain, double forwardGain, double backwardGain) {
    // When forward and backward gains are the same, prioritize merging that
    // preserves the original order of the functions in the binary
    if (std::abs(forwardGain - backwardGain) < EPS) {
      if (srcChain->id < dstChain->id) {
        isGainForward = true;
        cachedGain = predChain == srcChain ? forwardGain : backwardGain;
      } else {
        isGainForward = false;
        cachedGain = predChain == srcChain ? backwardGain : forwardGain;
      }
    } else if (forwardGain > backwardGain) {
      isGainForward = predChain == srcChain;
      cachedGain = forwardGain;
    } else {
      isGainForward = predChain != srcChain;
      cachedGain = backwardGain;
    }
  }

  double gain() const {
    return cachedGain;
  }

  Chain* predChain() const {
    return isGainForward ? srcChain : dstChain;
  }

  Chain* succChain() const {
    return isGainForward ? dstChain : srcChain;
  }

private:
  Chain* srcChain{nullptr};
  Chain* dstChain{nullptr};

public:
  // Original arcs in the binary with corresponding execution counts
  ArcList arcs;
  // Cached gain of merging the pair of chains
  double cachedGain{-1.0};
  // Since the gain of merging (Src, Dst) and (Dst, Src) might be different,
  // we store a flag indicating which of the options results in a higher gain
  bool isGainForward;
};

void Chain::mergeEdges(Chain* other) {
  assert(this != other && "cannot merge a chain with itself");

  // Update edges adjacent to chain other
  for (auto edgeIt : other->edges) {
    const auto dstChain = edgeIt.first;
    const auto dstEdge = edgeIt.second;
    const auto targetChain = dstChain == other ? this : dstChain;

    // Find the corresponding edge in the current chain
    auto curEdge = getEdge(targetChain);
    if (curEdge == nullptr) {
      dstEdge->changeEndpoint(other, this);
      this->addEdge(targetChain, dstEdge);
      if (dstChain != this && dstChain != other) {
        dstChain->addEdge(this, dstEdge);
      }
    } else {
      curEdge->moveArcs(dstEdge);
    }
    // Cleanup leftover edge
    if (dstChain != other) {
      dstChain->removeEdge(other);
    }
  }
}

/*
 * HFSortPlus - layout of hot functions with iTLB cache optimization.
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
 * page. The following algorithm detects short and long calls, and optimizes
 * the expected number of cache misses for the long ones.
 */
class HFSortPlus {
public:
  explicit HFSortPlus(const TargetGraph& cg) : cg(cg) {
    initialize();
  }

  /*
   * Run the algorithm and return an ordered set of function clusters.
   */
  std::vector<Cluster> run() {
    HFTRACE(1, "Starting hfsort+ for %lu clusters\n", hotChains.size());

    // Pass 1
    runPassOne();

    // Pass 2
    runPassTwo();

    // Sorting chains by density in decreasing order
    auto densityComparator = [](const Chain* c1, const Chain* c2) {
      if (c1->density() != c2->density()) {
        return c1->density() > c2->density();
      }
      // Making sure the comparison is deterministic
      return c1->id < c2->id;
    };
    std::stable_sort(hotChains.begin(), hotChains.end(), densityComparator);

    // Return the set of clusters that are left, which are the ones that
    // didn't get merged (so their first func is its original func)
    std::vector<Cluster> clusters;
    clusters.reserve(hotChains.size());
    for (auto Chain : hotChains) {
      clusters.emplace_back(Cluster(Chain->nodes, cg));
    }
    HFTRACE(1, "Completed hfsort+ with %lu clusters\n", hotChains.size());
    return clusters;
  }

private:
  /*
   * Initialize the set of active chains, function id to chain mapping,
   * total number of samples and function addresses.
   */
  void initialize() {
    // Find hot functions with samples in the data
    outWeight.resize(cg.targets.size(), 0);
    inWeight.resize(cg.targets.size(), 0);
    for (size_t f = 0; f < cg.targets.size(); ++f) {
      for (auto succ : cg.targets[f].succs) {
        const auto arc = cg.arcs.find(Arc(f, succ));
        outWeight[f] += arc->weight;
        inWeight[succ] += arc->weight;
      }
    }
    size_t numTargets = 0;
    for (size_t f = 0; f < cg.targets.size(); ++f) {
      assert(cg.targets[f].samples >= inWeight[f] && "incorrect input weights");
      if (inWeight[f] > 0 || outWeight[f] > 0) {
        numTargets++;
      }
    }

    // Initialize chains
    allChains.reserve(numTargets);
    hotChains.reserve(numTargets);
    nodeChain.resize(cg.targets.size(), nullptr);
    addr.resize(cg.targets.size(), 0);
    totalSamples = 0.0;
    for (size_t f = 0; f < cg.targets.size(); ++f) {
      if (inWeight[f] == 0 && outWeight[f] == 0) continue;
      allChains.emplace_back(f, f, cg.targets[f].samples, cg.targets[f].size);
      hotChains.push_back(&allChains.back());
      nodeChain[f] = &allChains.back();
      totalSamples += cg.targets[f].samples;
    }

    allEdges.reserve(cg.arcs.size());
    for (auto& chain : hotChains) {
      auto f = chain->nodes.back();
      for (auto succ : cg.targets[f].succs) {
        if (f == succ) continue;
        const Arc& arc = *cg.arcs.find(Arc(f, succ));
        if (arc.weight == 0.0 || arc.weight / totalSamples < kArcThreshold) {
          continue;
        }

        auto curEdge = nodeChain[f]->getEdge(nodeChain[succ]);
        if (curEdge != nullptr) {
          // This edge is already present in the graph
          assert(nodeChain[succ]->getEdge(nodeChain[f]) != nullptr);
          curEdge->arcs.push_back(&arc);
        } else {
          // This is a new edge
          allEdges.emplace_back(nodeChain[f], nodeChain[succ], &arc);
          nodeChain[f]->addEdge(nodeChain[succ], &allEdges.back());
          nodeChain[succ]->addEdge(nodeChain[f], &allEdges.back());
        }
      }
    }

    for (auto& chain : hotChains) {
      chain->shortCalls = shortCalls(chain);
      chain->score = score(chain);
    }
  }

  /*
   * The probability that a page with a given density is not in the cache.
   *
   * Assume that the hot functions are called in a random order; then the
   * probability of an i-TLB page being accessed after a function call is
   * p = pageSamples / totalSamples. The probability that the page is not
   * accessed is (1 - p), and the probability that it is not in the cache
   * (i.e. not accessed during the last kCacheEntries function calls)
   * is (1 - p)^kCacheEntries
   */
  double missProbability(double chainDensity) const {
    double pageSamples = chainDensity * kCachePageSize;

    if (pageSamples >= totalSamples) {
      return 0;
    }

    double p = pageSamples / totalSamples;
    return pow(1.0 - p, double(kCacheEntries));
  }

  /*
   * The expected number of calls on different i-TLB pages for an arc of the
   * call graph with a specified weight.
   */
  double expectedCalls(uint64_t srcAddr, uint64_t dstAddr, double weight) const {
    uint64_t dist = srcAddr >= dstAddr ? srcAddr - dstAddr : dstAddr - srcAddr;
    if (dist >= kMaxArcDistance) {
      return 0;
    }

    double d = double(dist) / double(kMaxArcDistance);
    // Increasing the importance of shorter calls
    return (1.0 - d * d) * weight;
  }

  /*
   * The expected number of calls within a given chain with both endpoints on
   * the same cache page.
   */
  double shortCalls(Chain* chain) const {
    auto edge = chain->getEdge(chain);
    if (edge == nullptr) {
      return 0;
    }

    double calls = 0;
    for (auto arc : edge->arcs) {
      uint64_t srcAddr = addr[arc->src] + uint64_t(arc->avgCallOffset);
      uint64_t dstAddr = addr[arc->dst];
      calls += expectedCalls(srcAddr, dstAddr, arc->weight);
    }
    return calls;
  }

  /*
   * The number of calls between the two chains with both endpoints on
   * the same i-TLB page, assuming that a given pair of chains gets merged.
   */
  double shortCalls(Chain* chainPred,
                    Chain* chainSucc,
                    Edge* edge) const {
    double calls = 0;
    for (auto arc : edge->arcs) {
      auto srcChain = nodeChain[arc->src];
      uint64_t srcAddr;
      uint64_t dstAddr;
      if (srcChain == chainPred) {
        srcAddr = addr[arc->src] + uint64_t(arc->avgCallOffset);
        dstAddr = addr[arc->dst] + chainPred->size;
      } else {
        srcAddr = addr[arc->src] + uint64_t(arc->avgCallOffset) + chainPred->size;
        dstAddr = addr[arc->dst];
      }
      calls += expectedCalls(srcAddr, dstAddr, arc->weight);
    }

    calls += chainPred->shortCalls;
    calls += chainSucc->shortCalls;

    return calls;
  }

  double score(Chain* chain) const {
    double longCalls = chain->samples - chain->shortCalls;
    return longCalls * missProbability(chain->density());
  }

  /*
   * The gain of merging two chains.
   *
   * We assume that the final chains are sorted by their density, and hence
   * every chain is likely to be adjacent with chains of the same density.
   * Thus, the 'hotness' of every chain can be estimated by density*pageSize,
   * which is used to compute the probability of cache misses for long calls
   * of a given chain.
   * The result is also scaled by the size of the resulting chain in order to
   * increase the chance of merging short chains, which is helpful for
   * the i-cache performance.
   */
  double mergeGain(Chain* chainPred, Chain* chainSucc, Edge* edge) const {
    // Cache misses on the chains before merging
    double curScore = chainPred->score + chainSucc->score;

    // Cache misses on the merged chain
    double longCalls = chainPred->samples + chainSucc->samples -
                       shortCalls(chainPred, chainSucc, edge);
    const double mergedSamples = chainPred->samples + chainSucc->samples;
    const double mergedSize = chainPred->size + chainSucc->size;
    double newScore = longCalls * missProbability(mergedSamples / mergedSize);

    double gain = curScore - newScore;
    // Scale the result to increase the importance of merging short chains
    gain /= std::min(chainPred->size, chainSucc->size);

    return gain;
  }

  /*
   * Run the first optimization pass of the algorithm:
   * Merge chains that call each other with a high probability.
   */
  void runPassOne() {
    // Find candidate pairs of chains for merging
    std::vector<const Arc*> arcsToMerge;
    for (auto chainPred : hotChains) {
      auto f = chainPred->nodes.back();
      for (auto succ : cg.targets[f].succs) {
        if (f == succ) continue;

        const Arc& arc = *cg.arcs.find(Arc(f, succ));
        if (arc.weight == 0.0 || arc.weight / totalSamples < kArcThreshold) {
          continue;
        }

        const double callsFromPred = outWeight[f];
        const double callsToSucc = inWeight[succ];
        const double callsPredSucc = arc.weight;

        // Probability that the first chain is calling the second one
        const double probOut =
          callsFromPred > 0 ? callsPredSucc / callsFromPred : 0;
        assert(0.0 <= probOut && probOut <= 1.0 && "incorrect out-probability");

        // Probability that the second chain is called from the first one
        const double probIn =
          callsToSucc > 0 ? callsPredSucc / callsToSucc : 0;
        assert(0.0 <= probIn && probIn <= 1.0 && "incorrect in-probability");

        if (std::min(probOut, probIn) >= kMergeProbability) {
          arcsToMerge.push_back(&arc);
        }
      }
    }

    // Sort the pairs by the weight in reverse order
    std::sort(
      arcsToMerge.begin(),
      arcsToMerge.end(),
      [](const Arc* l, const Arc* r) {
        return l->weight > r->weight;
      });

    // Merge the pairs of chains
    for (auto arc : arcsToMerge) {
      auto chainPred = nodeChain[arc->src];
      auto chainSucc = nodeChain[arc->dst];
      if (chainPred == chainSucc) continue;
      if (chainPred->nodes.back() == arc->src &&
          chainSucc->nodes.front() == arc->dst) {
        mergeChains(chainPred, chainSucc);
      }
    }
  }

  /*
   * Run the second optimization pass of the hfsort+ algorithm:
   * Merge pairs of chains while there is an improvement in the
   * expected cache miss ratio.
   */
  void runPassTwo() {
    // Creating a priority queue containing all edges ordered by the merge gain
    auto gainComparator = [&](Edge* A, Edge* B) {
      if (std::abs(A->gain() - B->gain()) > EPS) {
        return A->gain() > B->gain();
      }
      // Making sure the comparison is deterministic
      if (A->predChain()->id != B->predChain()->id) {
        return A->predChain()->id < B->predChain()->id;
      }
      return A->succChain()->id < B->succChain()->id;
    };
    std::set<Edge*, decltype(gainComparator)> queue(gainComparator);

    // Inserting the edges into the queue
    for (auto chainPred : hotChains) {
      for (auto edgeIt : chainPred->edges) {
        auto chainSucc = edgeIt.first;
        auto chainEdge = edgeIt.second;
        // Ignore loop edges
        if (chainPred == chainSucc) continue;
        // Ignore already processed edges
        if (chainEdge->gain() != -1.0) continue;

        // Compute the gain of merging the two chains
        auto forwardGain = mergeGain(chainPred, chainSucc, chainEdge);
        auto backwardGain = mergeGain(chainSucc, chainPred, chainEdge);
        chainEdge->setMergeGain(chainPred, forwardGain, backwardGain);
        if (chainEdge->gain() > 0.0) {
          queue.insert(chainEdge);
        }
      }
    }

    // Merge the chains while the gain of merging is positive
    while (!queue.empty()) {
      // Extract the best (top) edge for merging
      auto it = *queue.begin();
      queue.erase(queue.begin());
      Edge* bestEdge = it;
      Chain* bestChainPred = bestEdge->predChain();
      Chain* bestChainSucc = bestEdge->succChain();
      if (bestChainPred == bestChainSucc || bestEdge->gain() <= 0.0) {
        continue;
      }

      // Remove outdated edges
      for (auto edgeIt : bestChainPred->edges) {
        queue.erase(edgeIt.second);
      }
      for (auto edgeIt : bestChainSucc->edges) {
        queue.erase(edgeIt.second);
      }

      // Merge the best pair of chains
      mergeChains(bestChainPred, bestChainSucc);

      // Insert newly created edges into the queue
      for (auto edgeIt : bestChainPred->edges) {
        auto chainSucc = edgeIt.first;
        auto chainEdge = edgeIt.second;
        // Ignore loop edges
        if (bestChainPred == chainSucc) continue;

        // Compute the gain of merging the two chains
        auto forwardGain = mergeGain(bestChainPred, chainSucc, chainEdge);
        auto backwardGain = mergeGain(chainSucc, bestChainPred, chainEdge);
        chainEdge->setMergeGain(bestChainPred, forwardGain, backwardGain);
        if (chainEdge->gain() > 0.0) {
          queue.insert(chainEdge);
        }
      }
    }
  }

  /*
   * Merge chain From into chain Into and update the list of active chains.
   */
  void mergeChains(Chain* into, Chain* from) {
    assert(into != from && "cannot merge a chain with itself");
    into->merge(from);

    // Update the chains and addresses for functions merged from from
    size_t curAddr = 0;
    for (auto f : into->nodes) {
      nodeChain[f] = into;
      addr[f] = curAddr;
      curAddr += cg.targets[f].size;
    }

    // Merge edges
    into->mergeEdges(from);
    from->clear();

    // Update cached scores for the new chain
    into->shortCalls = shortCalls(into);
    into->score = score(into);

    // Remove chain from from the list of active chains
    auto it = std::remove(hotChains.begin(), hotChains.end(), from);
    hotChains.erase(it, hotChains.end());
  }

private:
  // The call graph
  const TargetGraph& cg;

  // All chains of functions
  std::vector<Chain> allChains;

  // Active chains. The vector gets updated at runtime when chains are merged
  std::vector<Chain*> hotChains;

  // All edges between chains
  std::vector<Edge> allEdges;

  // Node_id => chain
  std::vector<Chain*> nodeChain;

  // Current address of the function from the beginning of its chain
  std::vector<uint64_t> addr;

  // Total weight of outgoing arcs for each function
  std::vector<double> outWeight;

  // Total weight of incoming arcs for each function
  std::vector<double> inWeight;

  // The total number of samples in the graph
  double totalSamples;
};

}

/*
 * HFSortPlus - layout of hot functions with iTLB cache optimization.
 */
std::vector<Cluster> hfsortPlus(const TargetGraph& cg) {
  return HFSortPlus(cg).run();
}

}}
