/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/MapUtil.h>

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>

/*
 * This module implements two code layout strategies for sorting the
 * Vasm blocks:
 *
 *  1) rpoLayout() implements a simple layout that sorts the blocks in
 *     reverse post-order.  The final list of blocks is also
 *     partitioned so that any blocks assigned to the Main code area
 *     appear before the blocks assigned to the Cold area, which in
 *     turn appear before all blocks assigned to the Frozen area.
 *     This method is used when no profiling information is available.
 *
 *  2) pgoLayout() is enabled for Optimize, PGO-based regions.  This
 *     implements the algorithm described in "Profile Guided Code
 *     Positioning" (PLDI'1990) by Pettis & Hansen (more specifically,
 *     Algo2, from section 4.2.1).  This implementation uses estimated
 *     arc weights derived from a combination of profile counters
 *     inserted at the bytecode-level blocks (in Profile translations)
 *     and the JIT-time Likely/Unlikely/Unused hints (encoded in the
 *     "area" field of Vblocks).
 *
 */

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(layout);

///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> rpoLayout(const Vunit& unit) {
  auto blocks = sortBlocks(unit);
  // Partition into main/cold/frozen areas without changing relative order, and
  // the end{} block will be last.
  auto coldIt = std::stable_partition(blocks.begin(), blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Main &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  std::stable_partition(coldIt, blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Cold &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  return blocks;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * This keeps track of the weights of blocks and arcs in a Vunit.
 */
struct Scale {
  Scale(const Vunit& unit, const ProfData& prof)
      : m_unit(unit)
      , m_prof(prof)
      , m_blocks(sortBlocks(unit))
      , m_preds(computePreds(unit)) {
    computeWeights();
  }
  int64_t weight(Vlabel blk) const;
  int64_t weight(Vlabel src, Vlabel dst) const;

  std::string toString() const;

 private:
  void    computeWeights();
  void    computeBlockWeights();
  void    computeArcWeights();
  TransID findProfTransID(Vlabel blk) const;
  int64_t findProfWeight(Vlabel blk)  const;

  static uint64_t arcId(Vlabel src, Vlabel dst) { return (src << 32) + dst; }

  const Vunit&                     m_unit;
  const ProfData&                  m_prof;
  const jit::vector<Vlabel>        m_blocks;
  const PredVector                 m_preds;
  jit::vector<int64_t>             m_blkWgts;
  jit::hash_map<uint64_t, int64_t> m_arcWgts; // keyed using arcId()
};

int64_t Scale::weight(Vlabel blk) const {
  return m_blkWgts[blk];
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

int64_t Scale::findProfWeight(Vlabel blk) const {
  auto profTransID = findProfTransID(blk);
  return profTransID == kInvalidTransID ? 0
                                        : m_prof.absTransCounter(profTransID);
}

void Scale::computeBlockWeights() {
  m_blkWgts.resize(m_unit.blocks.size(), 0);

  // We divide the corresponding region block's profile counter by the
  // following factors, depending on the code area the block is
  // assigned to.
  static int areaWeightFactors[] = { 1,  /* main   */
                                     10, /* cold   */
                                     100 /* frozen */ };

  static_assert(
    sizeof(areaWeightFactors) / sizeof(areaWeightFactors[0]) == kNumAreas,
    "need to update areaWeightFactors");

  for (auto b : m_blocks) {
    auto a = unsigned(m_unit.blocks[b].area);
    assertx(a < 3);
    m_blkWgts[b] = findProfWeight(b) / areaWeightFactors[a];
  }
}

void Scale::computeArcWeights() {
  for (auto b : m_blocks) {
    auto succSet = succs(m_unit.blocks[b]);
    for (auto s : succSet) {
      // If the arc is non-critical, we can figure out its weight by
      // looking at its incident blocks.  For critical arcs, we
      // currently just approximate it as half of the smallest weight
      // of its incident blocks.
      auto arcid = arcId(b, s);
      m_arcWgts[arcid] = succSet.size()    == 1 ? weight(b)
                       : m_preds[s].size() == 1 ? weight(s)
                       : std::min(weight(b), weight(s)) / 2;
      FTRACE(3, "arc({} -> {}) => weight = {}  "
             "[|succs(b)| = {} ; |preds(s)| = {}] "
             "[weight(b) = {} ; weight(s) = {}]\n", b, s, m_arcWgts[arcid],
             succSet.size(), m_preds[s].size(), weight(b), weight(s));
    }
  }
}

void Scale::computeWeights() {
  computeBlockWeights();
  computeArcWeights();
}

std::string Scale::toString() const {
  std::ostringstream out;
  out << "digraph {\n";
  int64_t maxWgt = 0;
  for (auto b : m_blocks) {
    maxWgt = std::max(maxWgt, weight(b));
  }
  for (auto b : m_blocks) {
    unsigned coldness = 255 - (255 * weight(b) / maxWgt);
    out << folly::format(
      "{} [label=\"{}\\nw: {}\\nptid: {}\\narea: {}\","
      "shape=box,style=filled,fillcolor=\"#ff{:02x}{:02x}\"]\n",
      b, b, weight(b), findProfTransID(b), unsigned(m_unit.blocks[b].area),
      coldness, coldness);
    for (auto s : succs(m_unit.blocks[b])) {
      out << folly::format("{} -> {} [label={}];\n", b, s, weight(b, s));
    }
  }
  out << "}\n";
  return out.str();
}

///////////////////////////////////////////////////////////////////////////////

struct Clusterizer {
  Clusterizer(const Vunit& unit, const Scale& scale)
      : m_unit(unit)
      , m_scale(scale)
      , m_blocks(sortBlocks(unit)) {
    initClusters();
    clusterize();
    sortClusters();
    FTRACE(1, "{}", toString());
  }

  jit::vector<Vlabel> getBlockList() const;

 private:
  using Cluster = jit::vector<Vlabel>;

  void initClusters();
  void clusterize();
  void sortClusters();

  std::string toString() const;

  const Vunit&              m_unit;
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
  m_clusterOrder = std::move(dfsSort.sort(m_blockCluster[m_unit.entry]));
}

///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> pgoLayout(const Vunit& unit) {
  // compute block & arc weights
  Scale scale(unit, *(mcg->tx().profData()));
  FTRACE(1, "profileGuidedLayout: Weighted CFG:\n{}\n", scale.toString());

  // cluster the blocks based on weights and sort the clusters
  Clusterizer clusterizer(unit, scale);
  return clusterizer.getBlockList();
}

}

///////////////////////////////////////////////////////////////////////////////

jit::vector<Vlabel> layoutBlocks(const Vunit& unit) {
  Timer timer(Timer::vasm_layout);
  return mcg->tx().mode() == TransKind::Optimize ? pgoLayout(unit)
                                                 : rpoLayout(unit);
}

} }
