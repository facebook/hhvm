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

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>
#include <folly/MapUtil.h>

#include <algorithm>
#include <sstream>

/*
 * This module implements two code layout strategies for sorting a Vunit's
 * blocks:
 *
 *  1) rpoLayout() implements a simple layout that sorts the blocks in reverse
 *     post-order.  The final list of blocks is also partitioned so that any
 *     blocks assigned to the Main code area appear before the blocks assigned
 *     to the Cold area, which in turn appear before all blocks assigned to the
 *     Frozen area.  This method is used when no profiling information is
 *     available.
 *
 *  2) pgoLayout() is enabled for Optimize, PGO-based regions.  This implements
 *     the algorithm described in "Profile Guided Code Positioning" (PLDI'1990)
 *     by Pettis & Hansen (more specifically, Algo2, from section 4.2.1).  This
 *     implementation uses estimated arc weights derived from a combination of
 *     profile counters inserted at the bytecode-level blocks (in Profile
 *     translations) and the JIT-time Likely/Unlikely/Unused hints (encoded in
 *     the "area" field of Vblocks).
 */

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(layout);

///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> rpoLayout(Vunit& unit) {
  auto labels = sortBlocks(unit);

  auto const blk = [&] (Vlabel b) -> const Vblock& { return unit.blocks[b]; };

  // Partition into main/cold/frozen areas without changing relative order, and
  // the fallthru{} block will be last if there is one.
  auto coldIt = std::stable_partition(labels.begin(), labels.end(),
    [&] (Vlabel b) {
      return blk(b).area_idx == AreaIndex::Main &&
             blk(b).code.back().op != Vinstr::fallthru;
    });
  std::stable_partition(coldIt, labels.end(),
    [&] (Vlabel b) {
      return blk(b).area_idx == AreaIndex::Cold &&
             blk(b).code.back().op != Vinstr::fallthru;
    });

  if (unit.context) {
    auto const kind = unit.context->kind;
    if ((isPrologue(kind)  && !RO::EvalJitLayoutPrologueSplitHotCold) ||
        (isProfiling(kind) && !RO::EvalJitLayoutProfileSplitHotCold)) {
      for (auto b : labels) {
        if (unit.blocks[b].area_idx == AreaIndex::Cold) {
          unit.blocks[b].area_idx = AreaIndex::Main;
        }
      }
    }
  }

  // We put fallthru{} blocks at the end, but we also need to make sure it's
  // still partitioned with those blocks that share a code area.  This should
  // always be true, so just assert it.
  DEBUG_ONLY auto const n = labels.size();
  assertx(n < 2 ||
    IMPLIES(
      blk(labels.back()).code.back().op == Vinstr::fallthru,
      blk(labels.back()).area_idx == blk(labels[n - 2]).area_idx
    )
  );

  return labels;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * This keeps track of the weights of blocks and arcs in a Vunit.
 */
struct Scale {
  explicit Scale(const Vunit& unit)
      : m_unit(unit)
      , m_blocks(sortBlocks(unit))
      , m_preds(computePreds(unit)) {
    computeArcWeights();
  }

  explicit Scale(const Vunit& unit, const jit::vector<Vlabel>& blockOrder)
      : m_unit(unit)
      , m_blocks(blockOrder)
      , m_preds(computePreds(unit)) {
    computeArcWeights();
  }

  int64_t weight(Vlabel blk) const;
  int64_t weight(Vlabel src, Vlabel dst) const;

  std::string toString() const;

 private:
  static const int64_t kUnknownWeight = std::numeric_limits<int64_t>::max();

  void    computeArcWeights();
  TransID findProfTransID(Vlabel blk) const;
  int64_t findProfCount(Vlabel blk)   const;

  static uint64_t arcId(Vlabel src, Vlabel dst) { return (src << 32) + dst; }

  const Vunit&                     m_unit;
  const jit::vector<Vlabel>        m_blocks;
  const PredVector                 m_preds;
  jit::hash_map<uint64_t, int64_t> m_arcWgts; // keyed using arcId()
};

int64_t Scale::weight(Vlabel blk) const {
  return m_unit.blocks[blk].weight;
}

int64_t Scale::weight(Vlabel src, Vlabel dst) const {
  return folly::get_default(m_arcWgts, arcId(src, dst), 0);
}

TransID Scale::findProfTransID(Vlabel blk) const {
  for (auto& i : m_unit.blocks[blk].code) {
    if (!i.origin) continue;
    auto profTransID = i.origin->marker().profTransID();
    if (profTransID == kInvalidTransID) continue;
    return profTransID;
  }
  return kInvalidTransID;
}

int64_t Scale::findProfCount(Vlabel blk) const {
  for (auto& i : m_unit.blocks[blk].code) {
    if (i.origin) {
      return i.origin->block()->profCount();
    }
  }
  return 1;
}

void Scale::computeArcWeights() {
  FTRACE(3, "[vasm-layout] computeArcWeights:\n");

  // First, we can easily compute the weight of the non-crititical arcs by
  // looking at its incident blocks.
  for (auto b : m_blocks) {
    auto succSet = succs(m_unit.blocks[b]);
    for (auto s : succSet) {
      auto arcid = arcId(b, s);
      m_arcWgts[arcid] = succSet.size()    == 1 ? weight(b)
                       : m_preds[s].size() == 1 ? weight(s)
                       : kUnknownWeight;
      assertx(m_arcWgts[arcid] >= 0);
      if (m_arcWgts[arcid] != kUnknownWeight) {
        FTRACE(3, "  - arc({} -> {}) [non-critical] => weight = {}  "
               "[|succs(b)| = {} ; |preds(s)| = {}] "
               "[weight(b) = {} ; weight(s) = {}]\n", b, s, m_arcWgts[arcid],
               succSet.size(), m_preds[s].size(), weight(b), weight(s));
      }
    }
  }

  // Next, do an iterative pass trying to infer the remaining arcs using the
  // fact that some arc weights are already known and the invariant that arc
  // weights incoming / outgoing a specific block should add up to that block's
  // weight.
  bool inferred = true;
  do {
    inferred = false;

    for (auto b : m_blocks) {
      const auto total = weight(b);

      // If b has a single successor with unknown weight, infer its weight.
      auto succSet = succs(m_unit.blocks[b]);
      unsigned numUnknown = 0;
      uint64_t unknownArcId = 0;
      uint64_t knownTotal = 0;
      for (auto s : succSet) {
        auto arcid = arcId(b, s);
        if (m_arcWgts[arcid] == kUnknownWeight) {
          numUnknown++;
          unknownArcId = arcid;
        } else {
          knownTotal += m_arcWgts[arcid];
        }
      }
      if (numUnknown == 1) {
        m_arcWgts[unknownArcId] = total > knownTotal ? total - knownTotal : 0;
        inferred = true;
        FTRACE(3, "  - arc({} -> {}) [inferred-succs] => weight = {}\n",
               b, unknownArcId & 0xffffffff, m_arcWgts[unknownArcId]);
      }

      // If b has a single predecessor with unknown weight, infer its weight.
      numUnknown = 0;
      unknownArcId = 0;
      knownTotal = 0;
      for (auto p : m_preds[b]) {
        auto arcid = arcId(p, b);
        if (m_arcWgts[arcid] == kUnknownWeight) {
          numUnknown++;
          unknownArcId = arcid;
        } else {
          knownTotal += m_arcWgts[arcid];
        }
      }
      if (numUnknown == 1) {
        m_arcWgts[unknownArcId] = total > knownTotal ? total - knownTotal : 0;
        inferred = true;
        FTRACE(3, "  - arc({} -> {}) [inferred-preds] => weight = {}\n",
               unknownArcId >> 32, b, m_arcWgts[unknownArcId]);
      }
    }
  } while (inferred);

  // Finally, for each arc whose weight is still unknown at this point, we
  // currently just approximate it as half of the smallest weight of its
  // incident blocks.
  for (auto b : m_blocks) {
    auto succSet = succs(m_unit.blocks[b]);
    for (auto s : succSet) {
      auto arcid = arcId(b, s);
      if (m_arcWgts[arcid] == kUnknownWeight) {
        m_arcWgts[arcid] = std::min(weight(b), weight(s)) / 2;
        FTRACE(3, "  - arc({} -> {}) [guessed] => weight = {}\n",
               b, s, m_arcWgts[arcid]);
      }
    }
  }
}

std::string Scale::toString() const {
  std::ostringstream out;
  out << "digraph LayoutCFG {\n";
  int64_t maxWgt = 1;
  for (auto b : m_blocks) {
    maxWgt = std::max(maxWgt, weight(b));
  }
  for (auto b : m_blocks) {
    auto const& block = m_unit.blocks[b];
    unsigned coldness = 255 - (255 * weight(b) / maxWgt);
    out << folly::format(
      "{} [label=\"{}\\nw: {}\\nptid: {}\\narea: {}\\nprof: {}\\ninsts: {}\","
      "shape=box,style=filled,fillcolor=\"#ff{:02x}{:02x}\"]\n",
      b, b, weight(b), findProfTransID(b), unsigned(block.area_idx),
      findProfCount(b), block.code.size(), coldness, coldness);
    for (auto s : succs(block)) {
      out << folly::format("{} -> {} [label={}];\n", b, s, weight(b, s));
    }
  }
  out << "}\n";
  return out.str();
}

///////////////////////////////////////////////////////////////////////////////

struct Clusterizer {
  Clusterizer(Vunit& unit, const Scale& scale)
      : m_unit(unit)
      , m_scale(scale)
      , m_blocks(sortBlocks(unit)) {
    initClusters();
    clusterize();
    sortClusters();
    if (RuntimeOption::EvalJitPGOLayoutSplitHotCold) {
      splitHotColdClusters();
    }
    FTRACE(1, "{}", toString());
  }

  jit::vector<Vlabel> getBlockList() const;

  std::string toString() const;

 private:
  using Cluster = jit::vector<Vlabel>;

  void initClusters();
  void clusterize();
  void sortClusters();
  void splitHotColdClusters();

  Vunit&                    m_unit;
  const Scale&              m_scale;
  const jit::vector<Vlabel> m_blocks;
  jit::vector<Cluster>      m_clusters;
  jit::vector<Vlabel>       m_blockCluster; // maps block to current cluster
  jit::vector<Vlabel>       m_clusterOrder; // final sorted list of cluster ids
};

jit::vector<Vlabel> Clusterizer::getBlockList() const {
  jit::vector<Vlabel> list;
  for (auto cid : m_clusterOrder) {
    for (auto b : m_clusters[cid]) {
      list.push_back(b);
    }
  }
  return list;
}

std::string Clusterizer::toString() const {
  std::ostringstream out;
  out << "clusterize: final clusters:\n";
  for (auto cid : m_clusterOrder) {
    out << folly::sformat("  - cluster {}: ", cid);
    for (auto b : m_clusters[cid]) {
      out << folly::sformat("{}, ", b);
    }
    out << "\n";;
  }
  return out.str();
}

void Clusterizer::initClusters() {
  m_clusters.resize(m_unit.blocks.size());
  m_blockCluster.resize(m_unit.blocks.size());
  for (auto b : m_blocks) {
    m_clusters[b].push_back(b);
    m_blockCluster[b] = b;
  }
}

void Clusterizer::clusterize() {
  struct ArcInfo {
    Vlabel  src;
    Vlabel  dst;
    int64_t wgt;
  };
  jit::vector<ArcInfo> arcInfos;
  for (auto b : m_blocks) {
    for (auto s : succs(m_unit.blocks[b])) {
      arcInfos.push_back({b, s, m_scale.weight(b, s)});
    }
  }

  // sort arcs in decreasing weight order
  std::sort(arcInfos.begin(), arcInfos.end(),
            [&](const ArcInfo& a1, const ArcInfo& a2) {
              return a1.wgt > a2.wgt;
            });

  for (auto& arcInfo : arcInfos) {
    auto src = arcInfo.src;
    auto dst = arcInfo.dst;

    // Only merge blocks in the same area.
    if (m_unit.blocks[src].area_idx != m_unit.blocks[dst].area_idx) continue;

    auto srcCid = m_blockCluster[src];
    auto dstCid = m_blockCluster[dst];
    if (srcCid == dstCid) continue;
    auto& srcC = m_clusters[srcCid];
    auto& dstC = m_clusters[dstCid];

    // src must be the last in its cluster
    if (srcC.back() != src) continue;
    // dst must be the first in its cluster
    if (dstC.front() != dst) continue;

    // merge the clusters by append the blocks in dstC to srcC
    for (auto d : dstC) {
      srcC.push_back(d);
      m_blockCluster[d] = srcCid;
    }
    dstC.clear();
  }
}

using SuccInfos = jit::hash_map<uint32_t, int64_t>; // cluster id => weight

struct DFSSortClusters {
  DFSSortClusters(const jit::vector<SuccInfos>&& succInfos, const Vunit& unit)
    : m_clusterSuccs(succInfos)
    , m_visited(unit.blocks.size()) { }

  jit::vector<Vlabel> sort(uint32_t initialCid);

 private:
  void dfs(uint32_t cid);

  jit::vector<SuccInfos>  m_clusterSuccs;
  boost::dynamic_bitset<> m_visited;
  jit::vector<Vlabel>     m_list;
};

void DFSSortClusters::dfs(uint32_t cid) {

  if (m_visited.test(cid)) return;
  m_visited.set(cid);
  m_list.push_back(Vlabel(cid));

  // find the best successor, which is the one to which cid has the
  // highest weight among the ones that haven't been visited yet
  int64_t  maxWgt = 0;
  uint32_t bestSucc = uint32_t(-1);
  for (auto& sInfo : m_clusterSuccs[cid]) {
    auto succId = sInfo.first;
    if (m_visited.test(succId)) continue;
    auto wgt = sInfo.second;
    if (wgt >= maxWgt) {
      maxWgt   = wgt;
      bestSucc = succId;
    }
  }

  if (bestSucc == uint32_t(-1)) return;

  // visit bestSucc first
  dfs(bestSucc);

  // now visit the remaining ones
  for (auto& sInfo : m_clusterSuccs[cid]) {
    if (sInfo.first != bestSucc) {
      dfs(sInfo.first);
    }
  }
}

jit::vector<Vlabel> DFSSortClusters::sort(uint32_t initialCid) {
  dfs(initialCid);
  return std::move(m_list);
}

/*
 * This method creates a weighted graph of the clusters, and sorts
 * them according to a DFS pre-order that prioritizes the arcs with
 * heaviest weights, so as to try to have a cluster be followed by its
 * mostly likely successor cluster.
 */
void Clusterizer::sortClusters() {
  jit::vector<SuccInfos> clusterGraph;
  clusterGraph.resize(m_unit.blocks.size());

  for (auto b : m_blocks) {
    for (auto s : succs(m_unit.blocks[b])) {
      auto srcCid = m_blockCluster[b];
      auto dstCid = m_blockCluster[s];
      if (srcCid == dstCid) continue;
      auto wgt = m_scale.weight(b, s);
      clusterGraph[srcCid][dstCid] += wgt;
    }
  }

  DFSSortClusters dfsSort(std::move(clusterGraph), m_unit);
  m_clusterOrder = dfsSort.sort(m_blockCluster[m_unit.entry]);
}


void Clusterizer::splitHotColdClusters() {
  // compute the average weight of each cluster
  jit::vector<uint64_t> clusterAvgWgt(m_clusters.size());
  for (size_t c = 0; c < m_clusters.size(); c++) {
    uint64_t totalWeight = 0;
    uint64_t totalSize   = 0;
    for (auto b : m_clusters[c]) {
      const auto numInsts = m_unit.blocks[b].code.size();
      totalSize   += numInsts;
      totalWeight += numInsts * m_scale.weight(b);
    }
    clusterAvgWgt[c] = totalSize == 0 ? 0 : totalWeight / totalSize;
  }

  const auto entryAvgWgt = clusterAvgWgt[m_blockCluster[m_unit.entry]];
  const uint64_t hotThreshold = entryAvgWgt *
                                RuntimeOption::EvalJitLayoutHotThreshold;
  const uint64_t coldThreshold = entryAvgWgt *
                                 RuntimeOption::EvalJitLayoutColdThreshold;
  FTRACE(3, "splitHotColdClusters: entryAvgWgt = {} ; hotThreshold = {} "
         "coldThreshold = {}\n", entryAvgWgt, hotThreshold, coldThreshold);

  for (auto cid : m_clusterOrder) {
    if (m_clusters[cid].size() == 0) continue;
    const AreaIndex area =
      clusterAvgWgt[cid] >= hotThreshold  ? AreaIndex::Main :
      clusterAvgWgt[cid] >= coldThreshold ? AreaIndex::Cold :
                                            AreaIndex::Frozen;
    FTRACE(3, "  -> C{}: {} (avg wgt = {}): ",
           cid, area_names[unsigned(area)], clusterAvgWgt[cid]);
    for (auto b : m_clusters[cid]) {
      m_unit.blocks[b].area_idx = area;
      FTRACE(3, "{}, ", b);
    }
    FTRACE(3, "\n");
  }
}


///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> pgoLayout(Vunit& unit) {
  // Make sure block weights are consistent.
  fixBlockWeights(unit);

  // Compute arc weights.
  Scale scale(unit);
  FTRACE(1, "profileGuidedLayout: Weighted CFG:\n{}\n", scale.toString());

  // Cluster the blocks based on weights and sort the clusters.
  Clusterizer clusterizer(unit, scale);
  auto labels = clusterizer.getBlockList();

  // Partition by actual code area without changing relative order.
  auto cold_iter = std::stable_partition(labels.begin(), labels.end(),
    [&] (Vlabel b) {
      return unit.blocks[b].area_idx == AreaIndex::Main;
    });
  if (cold_iter != labels.end()) {
    std::stable_partition(cold_iter, labels.end(),
      [&] (Vlabel b) {
        return unit.blocks[b].area_idx == AreaIndex::Cold;
      });
  }

  if (!RuntimeOption::EvalJitPGOLayoutSplitHotCold) {
    for (auto b : labels) {
      if (unit.blocks[b].area_idx == AreaIndex::Cold) {
        unit.blocks[b].area_idx = AreaIndex::Main;
      }
    }
  }

  if (Trace::moduleEnabled(Trace::layout, 1)) {
    FTRACE(1, "pgoLayout: final block list: ");
    for (DEBUG_ONLY auto b : labels) {
      FTRACE(1, "{}, ", b);
    }
    FTRACE(1, "\n");
  }

  if (RuntimeOption::EvalDumpLayoutCFG) {
    unit.annotations.emplace_back("LayoutCFG", Scale(unit, labels).toString());
  }

  return labels;
}

}

///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> layoutBlocks(Vunit& unit) {
  Timer timer(Timer::vasm_layout);

  return unit.context && unit.context->kind == TransKind::Optimize
    ? pgoLayout(unit)
    : rpoLayout(unit);
}

///////////////////////////////////////////////////////////////////////////////

void fixBlockWeights(Vunit& unit) {
  const auto preds(computePreds(unit));
  bool changed = false;

  auto const hasSelfEdge = [] (Vlabel b, auto const& l) {
    return std::find(l.begin(), l.end(), b) != l.end();
  };

  do {
    changed = false;
    for (size_t b = 0; b < unit.blocks.size(); b++) {
      auto& block = unit.blocks[b];

      // Rule 1: a block's weight can't exceed the sum of its predecessors,
      // except for the entry block.
      if (b != unit.entry && !hasSelfEdge(Vlabel{b}, preds[b])) {
        uint64_t predsTotal = 0;
        for (auto p : preds[b]) {
          predsTotal += unit.blocks[p].weight;
        }
        if (block.weight > predsTotal) {
          block.weight = predsTotal;
          changed = true;
        }
      }

      // Rule 2: a block's weight can't exceed the sum of its successors, except
      // for exit blocks.
      auto const successors = succs(block);
      if (successors.size() > 0 && !hasSelfEdge(Vlabel{b}, successors)) {
        uint64_t succsTotal = 0;
        for (auto s : successors) {
          succsTotal += unit.blocks[s].weight;
        }
        if (block.weight > succsTotal) {
          block.weight = succsTotal;
          changed = true;
        }
      }
    }
  } while (changed);
}

///////////////////////////////////////////////////////////////////////////////

}}
