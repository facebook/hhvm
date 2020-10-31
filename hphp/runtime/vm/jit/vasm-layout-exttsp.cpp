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

#include "hphp/runtime/vm/jit/vasm-layout.h"

#include "hphp/util/trace.h"

#include <algorithm>
#include <set>

/*
 * This module implements a code layout strategy for sorting a Vunit's blocks:
 *
 * The algorithm is a greedy heuristic that works with chains (ordered lists)
 * of basic blocks. Initially all chains are isolated basic blocks. On every
 * iteration, we pick a pair of chains whose merging yields the biggest increase
 * in the ExtTSP value, which models how i-cache "friendly" a specific chain is.
 * A pair of chains giving the maximum gain is merged into a new chain. The
 * procedure stops when there is only one chain left, or when merging does not
 * increase ExtTSP. In the latter case, the remaining chains are sorted by
 * density in decreasing order.
 *
 * Reference:
 *    A. Newell and S. Pupyrev, Improved Basic Block Reordering,
 *    IEEE Transactions on Computers, 2020
 */

namespace HPHP { namespace jit { namespace layout {
////////////////////////////////////////////////////////////////////////////////

namespace {
////////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(layout);

// Parameters of the ext-tsp model
constexpr double kForwardWeight = 0.1;
constexpr double kBackwardWeight = 0.1;
constexpr size_t kForwardDistance = 1024;
constexpr size_t kBackwardDistance = 640;
// The chain merging phase stops when the relative gain is below the threshold
constexpr double kMinRelativeMergeGain = 0.0005;
// The maximum size of a chain considered for splitting
constexpr double kChainSplitThreshold = 128;
// Epsilon for comparison of doubles
constexpr double EPS = 1e-8;

struct Block;
struct Chain;
struct Edge;

/*
 * Calculate Ext-TSP value, which quantifies the expected number of i-cache
 * misses for a given ordering of basic blocks
 */
double extTSPScore(
    uint64_t srcAddr,
    uint64_t srcSize,
    uint64_t dstAddr,
    uint64_t executionCount) {
  // Fallthrough
  if (srcAddr + srcSize == dstAddr) {
    // Assume that FallthroughWeight = 1.0 after normalization
    return static_cast<double>(executionCount);
  }
  // Forward
  if (srcAddr + srcSize < dstAddr) {
    const auto dist = dstAddr - (srcAddr + srcSize);
    if (dist <= kForwardDistance) {
      double prob = 1.0 - static_cast<double>(dist) / kForwardDistance;
      return kForwardWeight * prob * executionCount;
    }
    return 0;
  }
  // Backward
  const auto dist = srcAddr + srcSize - dstAddr;
  if (dist <= kBackwardDistance) {
    double prob = 1.0 - static_cast<double>(dist) / kBackwardDistance;
    return kBackwardWeight * prob * executionCount;
  }
  return 0;
}

using BlockPair = std::pair<Block*, Block*>;
using JumpList = jit::vector<std::pair<BlockPair, uint64_t>>;
using BlockIter = jit::vector<Block*>::const_iterator;

enum MergeType {
  X_Y = 0,
  X1_Y_X2 = 1,
  Y_X2_X1 = 2,
  X2_X1_Y = 3,
};

struct MergeGain {
  explicit MergeGain() {}
  explicit MergeGain(double score, size_t mergeOffset, MergeType mergeType)
    : score(score)
    , mergeOffset(mergeOffset)
    , mergeType(mergeType)
  {}

  // returns true iff 'other' is preferred other 'this'
  bool operator < (const MergeGain& other) const {
    return (other.score > EPS && other.score > score + EPS);
  }

  double score{-1.0};
  size_t mergeOffset{0};
  MergeType mergeType{MergeType::X_Y};
};

/*
 * A node in CFG corresponding to a BinaryBlock.
 * The class wraps several mutable fields utilized in the ExtTSP algorithm.
 */
struct Block {
  // Delete copy constructor to make sure objects are moved rather than copied
  Block(const Block&) = delete;
  Block(Block&&) = default;
  Block& operator=(const Block&) = delete;
  Block& operator=(Block&&) = default;

  explicit Block(Vlabel label, uint64_t size, uint64_t count, uint64_t index)
    : label(label)
    , size(size)
    , executionCount(count)
    , index(index)
  {}

  bool adjacent(const Block* other) const {
    return hasOutJump(other) || hasInJump(other);
  }

  bool hasOutJump(const Block* other) const {
    for (auto jump : outJumps) {
      if (jump.first == other) {
        return true;
      }
    }
    return false;
  }

  bool hasInJump(const Block* other) const {
    for (auto jump : inJumps) {
      if (jump.first == other) {
        return true;
      }
    }
    return false;
  }

  // Corresponding label of the basic block
  const Vlabel label;
  // Current chain of the block
  Chain* curChain{nullptr};
  // (Estimated) size of the block in the binary
  uint64_t size{0};
  // Execution count of the block in the binary
  uint64_t executionCount{0};
  // An original index of the node in CFG
  size_t index{0};
  // The index of the block in the current chain
  size_t curIndex{0};
  // An offset of the block in the current chain
  mutable uint64_t estimatedAddr{0};
  // Fallthrough successor of the node in CFG
  Block* fallthroughSucc{nullptr};
  // Fallthrough predecessor of the node in CFG
  Block* fallthroughPred{nullptr};
  // Outgoing jumps from the block
  jit::vector<std::pair<Block*, uint64_t>> outJumps;
  // Incoming jumps to the block
  jit::vector<std::pair<Block*, uint64_t>> inJumps;
  // Total execution count of incoming jumps
  uint64_t inWeight{0};
  // Total execution count of outgoing jumps
  uint64_t outWeight{0};
};

/*
 * A chain (ordered sequence) of CFG nodes (basic blocks).
 */
struct Chain {
  Chain(const Chain&) = delete;
  Chain(Chain&&) = default;
  Chain& operator=(const Chain&) = delete;
  Chain& operator=(Chain&&) = default;

  explicit Chain(size_t id, Block* block)
    : id(id)
    , isEntry(block->index == 0)
    , executionCount(block->executionCount)
    , size(block->size)
    , score(0)
    , blocks(1, block)
  {}

  double density() const {
    return static_cast<double>(executionCount) / size;
  }

  Edge* getEdge(Chain* other) const {
    for (auto it : edges) {
      if (it.first == other) {
        return it.second;
      }
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

  void merge(Chain* other, const jit::vector<Block*>& mergedBlocks) {
    blocks = mergedBlocks;
    if (other->isEntry) isEntry = true;
    executionCount += other->executionCount;
    size += other->size;
    // Update block's chains
    for (size_t idx = 0; idx < blocks.size(); idx++) {
      blocks[idx]->curChain = this;
      blocks[idx]->curIndex = idx;
    }
  }

  void mergeEdges(Chain* other);

  void clear() {
    blocks.clear();
    edges.clear();
  }

  size_t id;
  bool isEntry;
  uint64_t executionCount;
  uint64_t size;
  // Cached ext-tsp score for the chain
  double score;
  // Blocks of the chain
  jit::vector<Block*> blocks;
  // Adjacent chains and corresponding edges (lists of jumps)
  jit::vector<std::pair<Chain*, Edge*>> edges;
};

/*
 * An edge in CFG reprsenting jumps between chains of BinaryBlocks.
 * When blocks are merged into chains, the edges are combined too so that
 * there is always at most one edge between a pair of chains.
 */
struct Edge {
  Edge(const Edge&) = delete;
  Edge(Edge&&) = default;
  Edge& operator=(const Edge&) = delete;
  Edge& operator=(Edge&&) = default;

  explicit Edge(Block* srcBlock, Block* dstBlock, uint64_t EC)
    : srcChain(srcBlock->curChain)
    , dstChain(dstBlock->curChain)
    , jumps(1, std::make_pair(std::make_pair(srcBlock, dstBlock), EC))
  {}

  void changeEndpoint(Chain* from, Chain* to) {
    if (from == srcChain) {
      srcChain = to;
    }
    if (from == dstChain) {
      dstChain = to;
    }
  }

  void appendJump(Block* srcBlock, Block* dstBlock, uint64_t EC) {
    jumps.push_back(std::make_pair(std::make_pair(srcBlock, dstBlock), EC));
  }

  void moveJumps(Edge* other) {
    jumps.insert(jumps.end(), other->jumps.begin(), other->jumps.end());
    other->jumps.clear();
  }

  bool hasCachedMergeGain(Chain* src, Chain* dst) const {
    return src == srcChain ? cacheValidForward : cacheValidBackward;
  }

  MergeGain getCachedMergeGain(Chain* src, Chain* dst) const {
    return src == srcChain ? cachedGainForward : cachedGainBackward;
  }

  void setCachedMergeGain(Chain* src, Chain* dst, MergeGain mergeGain) {
    if (src == srcChain) {
      cachedGainForward = mergeGain;
      cacheValidForward = true;
    } else {
      cachedGainBackward = mergeGain;
      cacheValidBackward = true;
    }
  }

  void invalidateCache() {
    cacheValidForward = false;
    cacheValidBackward = false;
  }

 private:
  Chain* srcChain{nullptr};
  Chain* dstChain{nullptr};

 public:
  // Original jumps in the binary with correspinding execution counts
  JumpList jumps;
  // Cached ext-tsp value for merging the pair of chains
  // Since the gain of merging (src, dst) and (dst, src) might be different,
  // we store both values here
  MergeGain cachedGainForward;
  MergeGain cachedGainBackward;
  // Whether the cached value must be recomputed
  bool cacheValidForward{false};
  bool cacheValidBackward{false};
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
      curEdge->moveJumps(dstEdge);
    }
    // Cleanup leftover edge
    if (dstChain != other) {
      dstChain->removeEdge(other);
    }
  }
}

/*
 * A wrapper around three chains of basic blocks; it is used to avoid extra
 * instantiation of the vectors.
 */
struct MergedChain {
  MergedChain(BlockIter begin1,
              BlockIter end1,
              BlockIter begin2 = BlockIter(),
              BlockIter end2 = BlockIter(),
              BlockIter begin3 = BlockIter(),
              BlockIter end3 = BlockIter())
    : begin1(begin1)
    , end1(end1)
    , begin2(begin2)
    , end2(end2)
    , begin3(begin3)
    , end3(end3)
  {}

  template <typename F>
  void forEach(const F& func) const {
    for (auto it = begin1; it != end1; it++) {
      func(*it);
    }
    for (auto it = begin2; it != end2; it++) {
      func(*it);
    }
    for (auto it = begin3; it != end3; it++) {
      func(*it);
    }
  }

  jit::vector<Block*> getBlocks() const {
    jit::vector<Block*> result;
    result.reserve(
        std::distance(begin1, end1) +
        std::distance(begin2, end2) +
        std::distance(begin3, end3));
    result.insert(result.end(), begin1, end1);
    result.insert(result.end(), begin2, end2);
    result.insert(result.end(), begin3, end3);
    return result;
  }

  const Block* getFirstBlock() const {
    return *begin1;
  }

 private:
  BlockIter begin1;
  BlockIter end1;
  BlockIter begin2;
  BlockIter end2;
  BlockIter begin3;
  BlockIter end3;
};

/*
 * The implementation of the ext-tsp layout algorithm.
 */
struct ExtTSP {
  ExtTSP(const Vunit& unit,
         const Scale& scale,
         const jit::vector<Vlabel>& labels)
    : unit(unit)
    , scale(scale)
  {
    initialize(labels);
  }

  /*
   * Run the algorithm and return an ordering of basic block
   */
  void run(jit::vector<jit::vector<Vlabel>>& clusters) {
    // Pass 1: Merge blocks with their fallthrough successors
    mergeFallthroughs();

    // Pass 2: Merge pairs of chains while improving the ExtTSP metric
    mergeChainPairs();

    // Collect resulting clusters
    clusters.clear();
    clusters.reserve(allChains.size());
    for (auto& chain : allChains) {
      if (chain.blocks.size() > 0) {
        jit::vector<Vlabel> cluster;
        for (auto b : chain.blocks) {
          cluster.push_back(b->label);
        }
        clusters.push_back(cluster);
      }
    }
  }

 private:
  /*
   * Initialize algorithm's data structures
   */
  void initialize(const jit::vector<Vlabel>& labels) {
    // Initialize CFG nodes
    labelIndex.resize(unit.blocks.size(), 0);
    allBlocks.reserve(labels.size());
    for (size_t i = 0; i < labels.size(); i++) {
	    // Estimate the average size of an instruction (in bytes)
      const auto numInsts = 4 * unit.blocks[labels[i]].code.size();
      auto size = std::max<uint64_t>(numInsts, 1);
      auto executionCount = scale.weight(labels[i]);
      allBlocks.emplace_back(labels[i], size, executionCount, i);
      labelIndex[labels[i]] = i;
    }

    // Initialize edges for the blocks and compute their total in/out weights
    size_t numEdges = 0;
    for (auto& block : allBlocks) {
      auto succSet = succs(unit.blocks[block.label]);
      for (auto succ : succSet) {
        auto executionCount = scale.weight(block.label, succ);
        if (succ != block.label && executionCount > 0) {
          auto& succBlock = allBlocks[labelIndex[succ]];
          succBlock.inWeight += executionCount;
          succBlock.inJumps.push_back(std::make_pair(&block, executionCount));
          block.outWeight += executionCount;
          block.outJumps.push_back(std::make_pair(&succBlock, executionCount));
          numEdges++;
        }
      }
    }

    // Initialize execution count for every basic block, which is the
    // maximum over the sums of all in and out edge weights.
    // Also execution count of the entry point is set to at least 1
    for (auto& block : allBlocks) {
      size_t index = block.index;
      block.executionCount = std::max(block.executionCount, block.inWeight);
      block.executionCount = std::max(block.executionCount, block.outWeight);
      if (index == 0 && block.executionCount == 0) {
        block.executionCount = 1;
      }
    }

    // Initialize chains
    allChains.reserve(allBlocks.size());
    hotChains.reserve(allBlocks.size());
    for (auto& block : allBlocks) {
      allChains.emplace_back(block.index, &block);
      block.curChain = &allChains.back();
      if (block.executionCount > 0) {
        hotChains.push_back(&allChains.back());
      }
    }

    // Initialize edges
    allEdges.reserve(numEdges);
    for (auto& block : allBlocks) {
      for (auto& jump : block.outJumps) {
        const auto succBlock = jump.first;
        auto curEdge = block.curChain->getEdge(succBlock->curChain);
        // this edge is already present in the graph
        if (curEdge != nullptr) {
          assert(succBlock->curChain->getEdge(block.curChain) != nullptr);
          curEdge->appendJump(&block, succBlock, jump.second);
          continue;
        }
        // this is a new edge
        allEdges.emplace_back(&block, succBlock, jump.second);
        block.curChain->addEdge(succBlock->curChain, &allEdges.back());
        succBlock->curChain->addEdge(block.curChain, &allEdges.back());
      }
    }
    assert(allEdges.size() <= numEdges && "Incorrect number of created edges");
  }

  /*
   * For a pair of blocks, A and B, block B is the fallthrough successor of A,
   * if (i) all jumps (based on profile) from A goes to B and (ii) all jumps
   * to B are from A. Such blocks should be adjacent in an optimal ordering;
   * the method finds and merges such pairs of blocks
   */
  void mergeFallthroughs() {
    // Find fallthroughs based on edge weights
    for (auto& block : allBlocks) {
      auto succSet = succs(unit.blocks[block.label]);
      if (succSet.size() == 1) {
        auto succ = succSet[0];
        if (scale.predSize(succ) == 1 && unit.entry != succ) {
          size_t succIndex = labelIndex[succ];
          block.fallthroughSucc = &allBlocks[succIndex];
          allBlocks[succIndex].fallthroughPred = &block;
        }
      }
    }

    // There might be 'cycles' in the fallthrough dependencies (since profile
    // data isn't 100% accurate).
    // Break the cycles by choosing the block with smallest index as the tail
    for (auto& block : allBlocks) {
      if (block.fallthroughSucc == nullptr) continue;
      if (block.fallthroughPred == nullptr) continue;

      auto succBlock = block.fallthroughSucc;
      while (succBlock != nullptr && succBlock != &block) {
        succBlock = succBlock->fallthroughSucc;
      }
      if (succBlock == nullptr) continue;
      // break the cycle
      allBlocks[block.fallthroughPred->index].fallthroughSucc = nullptr;
      block.fallthroughPred = nullptr;
    }

    // Merge blocks with their fallthrough successors
    for (auto& block : allBlocks) {
      if (block.fallthroughPred == nullptr &&
          block.fallthroughSucc != nullptr) {
        auto curBlock = &block;
        while (curBlock->fallthroughSucc != nullptr) {
          const auto nextBlock = curBlock->fallthroughSucc;
          mergeChains(block.curChain, nextBlock->curChain, 0, MergeType::X_Y);
          curBlock = nextBlock;
        }
      }
    }
  }

  /*
   * Merge pairs of chains while improving the ExtTSP metric
   */
  void mergeChainPairs() {
    double totalScore = 0;
    while (hotChains.size() > 1) {
      Chain* bestChainPred = nullptr;
      Chain* bestChainSucc = nullptr;
      auto bestGain = MergeGain();
      // Iterate over all pairs of chains
      for (auto chainPred : hotChains) {
        // Get candidates for merging with the current chain
        for (auto edgeIter : chainPred->edges) {
          auto chainSucc = edgeIter.first;
          auto chainEdge = edgeIter.second;
          // Ignore loop edges
          if (chainPred == chainSucc) continue;

          // Compute the gain of merging the two chains
          auto curGain = mergeGain(chainPred, chainSucc, chainEdge);
          if (curGain.score <= EPS) continue;

          if (bestGain < curGain) {
            bestGain = curGain;
            bestChainPred = chainPred;
            bestChainSucc = chainSucc;
          }
        }
      }

      // Stop merging when there is no improvement
      if (bestGain.score <= EPS) {
        break;
      }
      if (totalScore > 1.0 &&
          bestGain.score / totalScore <= kMinRelativeMergeGain) {
        break;
      }

      // Merge the best pair of chains
      totalScore += bestGain.score;
      mergeChains(
          bestChainPred,
          bestChainSucc,
          bestGain.mergeOffset,
          bestGain.mergeType);
    }
  }

  /*
   * Compute ExtTSP score for a given order of basic blocks
   */
  double score(const MergedChain& mergedBlocks, const JumpList& jumps) const {
    if (jumps.empty()) {
      return 0.0;
    }
    uint64_t curAddr = 0;
    mergedBlocks.forEach([&](const Block* b) {
      b->estimatedAddr = curAddr;
      curAddr += b->size;
    });

    double score = 0;
    for (auto& jump : jumps) {
      const auto srcBlock = jump.first.first;
      const auto dstBlock = jump.first.second;
      score += extTSPScore(
          srcBlock->estimatedAddr,
          srcBlock->size,
          dstBlock->estimatedAddr,
          jump.second);
    }
    return score;
  }

  /*
   * Compute the gain of merging two chains
   *
   * The function considers all possible ways of merging two chains and
   * computes the one having the largest increase in ExtTSP metric. The result
   * is a pair with the first element being the gain and the second element
   * being the corresponding merging type (encoded as an integer).
   */
  MergeGain mergeGain(Chain* chainPred, Chain* chainSucc, Edge* edge) const {
    if (edge->hasCachedMergeGain(chainPred, chainSucc)) {
      return edge->getCachedMergeGain(chainPred, chainSucc);
    }

    // Precompute jumps between chainPred and chainSucc
    auto jumps = edge->jumps;
    auto selfEdge = chainPred->getEdge(chainPred);
    if (selfEdge != nullptr) {
      jumps.insert(jumps.end(), selfEdge->jumps.begin(), selfEdge->jumps.end());
    }
    assert(jumps.size() > 0 && "trying to merge chains w/o jumps");

    // Merge two chains and update the best gain
    auto computeMergeGain = [&](const MergeGain& curGain,
                                const Chain* chainPred,
                                const Chain* chainSucc,
                                size_t mergeOffset,
                                MergeType mergeType) {
      auto mergedBlocks = mergeBlocks(
          chainPred->blocks, chainSucc->blocks, mergeOffset, mergeType);

      // Do not allow a merge that does not preserve the original entry block
      if ((chainPred->isEntry || chainSucc->isEntry) &&
          mergedBlocks.getFirstBlock()->index != 0) {
        return curGain;
      }

      // The gain for the new chain
      const auto newScore = score(mergedBlocks, jumps) - chainPred->score;
      auto newGain = MergeGain(newScore, mergeOffset, mergeType);
      return curGain < newGain ? newGain : curGain;
    };

    MergeGain gain = MergeGain();
    // Try to concatenate two chains w/o splitting
    gain = computeMergeGain(gain, chainPred, chainSucc, 0, MergeType::X_Y);

    // Attach (a part of) chainPred before the first block of chainSucc
    for (auto& jump : chainSucc->blocks.front()->inJumps) {
      const auto srcBlock = jump.first;
      if (srcBlock->curChain != chainPred) continue;
      if (srcBlock->fallthroughSucc != nullptr) continue;
      size_t offset = srcBlock->curIndex + 1;
      if (offset == chainPred->blocks.size()) continue;

      // X1 Y X2
      gain = computeMergeGain(
          gain, chainPred, chainSucc, offset, MergeType::X1_Y_X2);
      // X2 X1 Y
      gain = computeMergeGain(
          gain, chainPred, chainSucc, offset, MergeType::X2_X1_Y);
    }

    // Attach (a part of) chainPred after the last block of chainSucc
    for (auto& jump : chainSucc->blocks.back()->outJumps) {
      const auto dstBlock = jump.first;
      if (dstBlock->curChain != chainPred) continue;
      if (dstBlock->fallthroughPred != nullptr) continue;
      size_t offset = dstBlock->curIndex;
      if (offset == 0) continue;

      // X1 Y X2
      gain = computeMergeGain(
          gain, chainPred, chainSucc, offset, MergeType::X1_Y_X2);
      // Y X2 X1
      gain = computeMergeGain(
          gain, chainPred, chainSucc, offset, MergeType::Y_X2_X1);
    }

    // Try to break chainPred in various ways and concatenate with chainSucc
    if (chainPred->blocks.size() <= kChainSplitThreshold) {
      for (size_t offset = 1; offset < chainPred->blocks.size(); offset++) {
        auto b1 = chainPred->blocks[offset - 1];
        auto b2 = chainPred->blocks[offset];
        // Does the splitting break FT successors?
        if (b1->fallthroughSucc != nullptr) continue;
        if (b1->adjacent(b2)) continue;

        bool createX1YEdge = b1->hasOutJump(chainSucc->blocks.front());
        bool createYX2Edge = chainSucc->blocks.back()->hasOutJump(b2);
        if (!createX1YEdge && !createYX2Edge) {
          gain = computeMergeGain(
              gain, chainPred, chainSucc, offset, MergeType::X1_Y_X2);
        }
        if (!createYX2Edge) {
          gain = computeMergeGain(
              gain, chainPred, chainSucc, offset, MergeType::Y_X2_X1);
        }
        if (!createX1YEdge) {
          gain = computeMergeGain(
              gain, chainPred, chainSucc, offset, MergeType::X2_X1_Y);
        }
      }
    }

    edge->setCachedMergeGain(chainPred, chainSucc, gain);
    return gain;
  }

  /*
   * Merge two chains of blocks respecting a given merge 'type' and 'offset'
   *
   * If mergeType == 0, then the result is a concatentation of two chains.
   * Otherwise, the first chain is cut into two sub-chains at the offset,
   * and merged using all possible ways of concatenating three chains.
   */
  MergedChain mergeBlocks(
      const jit::vector<Block*>& x,
      const jit::vector<Block*>& y,
      size_t mergeOffset,
      MergeType mergeType) const {
    // Split the first chain, X, into X1 and X2
    BlockIter beginX1 = x.begin();
    BlockIter endX1 = x.begin() + mergeOffset;
    BlockIter beginX2 = x.begin() + mergeOffset;
    BlockIter endX2 = x.end();
    BlockIter beginY = y.begin();
    BlockIter endY = y.end();

    // Construct a new chain from the three existing ones
    switch (mergeType) {
      case MergeType::X_Y:
        return MergedChain(beginX1, endX2, beginY, endY);
      case MergeType::X1_Y_X2:
        return MergedChain(beginX1, endX1, beginY, endY, beginX2, endX2);
      case MergeType::Y_X2_X1:
        return MergedChain(beginY, endY, beginX2, endX2, beginX1, endX1);
      case MergeType::X2_X1_Y:
        return MergedChain(beginX2, endX2, beginX1, endX1, beginY, endY);
    }
    always_assert(false);
  }

  /*
   * Merge chain from into chain into, update the list of active chains,
   * adjacency information, and the corresponding cached values
   */
  void mergeChains(
      Chain* into,
      Chain* from,
      size_t mergeOffset,
      MergeType mergeType) {
    // Merge the blocks
    auto mergedBlocks =
        mergeBlocks(into->blocks, from->blocks, mergeOffset, mergeType);
    into->merge(from, mergedBlocks.getBlocks());
    into->mergeEdges(from);
    from->clear();

    // Update cached ext-tsp score for the new chain
    auto selfEdge = into->getEdge(into);
    if (selfEdge != nullptr) {
      mergedBlocks = MergedChain(into->blocks.begin(), into->blocks.end());
      into->score = score(mergedBlocks, selfEdge->jumps);
    }

    // Remove chain from from the list of active chains
    auto it = std::remove(hotChains.begin(), hotChains.end(), from);
    hotChains.erase(it, hotChains.end());

    // Invalidate caches
    for (auto edgeIter : into->edges) {
      edgeIter.second->invalidateCache();
    }
  }

 private:
  // The binary function
  const Vunit& unit;
  // Provider of block/edge counts in Vunit
  const Scale& scale;
  // An index of each label in the original order
  jit::vector<uint64_t> labelIndex;

  // All CFG nodes (basic blocks)
  jit::vector<Block> allBlocks;

  // All chains of blocks
  jit::vector<Chain> allChains;

  // Active chains. The vector gets updated at runtime when chains are merged
  jit::vector<Chain*> hotChains;

  // All edges between chains
  jit::vector<Edge> allEdges;
};

////////////////////////////////////////////////////////////////////////////////
} // anonymous namespace

void Clusterizer::clusterizeExtTSP() {
  auto blocks = m_scale.blocks();
  assert(!blocks.empty());
  assert(blocks[0] == m_unit.entry);

  // Apply the algorithm
  FTRACE(3, "[vasm-layout-exttsp] starting layout of {} blocks\n",
         blocks.size());
  ExtTSP(m_unit, m_scale, blocks).run(m_clusters);
  FTRACE(3, "[vasm-layout-exttsp] completed layout with {} clusters\n",
         m_clusters.size());

  // Use computed clusters
  for (size_t i = 0; i < m_clusters.size(); i++) {
    for (auto b : m_clusters[i]) {
      m_blockCluster[b] = (Vlabel)i;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}}}
