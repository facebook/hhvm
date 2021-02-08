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

#include <memory>
#include <algorithm>

#include "hphp/util/trace.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-prune-arcs.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"

/*
 * This module supports the implementation of two region selectors: hotcfg and
 * wholecfg.  In hotcfg mode, it constructs a region that is a maximal CFG
 * given the constraints for what is currently supported within a region and
 * the JitPGOMinBlockCountPercent and JitPGOMinArcProbability runtime options
 * (which can be used to prune cold/unlikely code).  In wholecfg mode, these
 * two runtime options are ignored and nothing is pruned based on profile
 * counters.
 */

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

namespace {

const StaticString s_switchProfile("SwitchProfile");

//////////////////////////////////////////////////////////////////////

struct DFS {
  DFS(const ProfData* p, const TransCFG& c, const TransIDSet& entries,
      int32_t maxBCInstrs, bool inlining, bool* truncated)
    : m_profData(p)
    , m_cfg(c)
    , m_entries(entries)
    , m_numBCInstrs(maxBCInstrs)
    , m_inlining(inlining)
    , m_truncated(truncated)
  {}

  RegionDescPtr formRegion(TransID head) {
    m_region = std::make_shared<RegionDesc>();
    if (RuntimeOption::EvalJitPGORegionSelector == "wholecfg") {
      m_minBlockWeight = 0;
      m_minArcProb = 0;
    } else {
      auto const minBlkPerc = RuntimeOption::EvalJitPGOMinBlockCountPercent;
      m_minBlockWeight = minBlkPerc * m_cfg.weight(head) / 100.0;
      m_minArcProb = RuntimeOption::EvalJitPGOMinArcProbability;
    }
    ITRACE(3, "formRegion: starting at head = {} (weight = {})\n"
           "   minBlockWeight = {:.2}\n"
           "   minArcProb = {:.2}\n",
           head, m_cfg.weight(head), m_minBlockWeight, m_minArcProb);
    Trace::Indent indent;
    visit(head);
    // If we couldn't add the head block (due to exceeding the bytecode budget),
    // return an empty region instead of one with a different entry.
    if (m_region->empty()) return m_region;

    // now visit potential retranslations for the head block
    DEBUG_ONLY auto const entrySrcKey = m_profData->transRec(head)->srcKey();
    for (auto bid : m_entries) {
      assertx(m_profData->transRec(bid)->srcKey() == entrySrcKey);
      visit(bid);
    }

    for (auto& arc : m_arcs) {
      m_region->addArc(arc.src, arc.dst);
    }
    always_assert_flog(
      m_region->empty() || m_region->entry()->id() == head,
      "formRegion() produced region with wrong entry: "
      "entry id ({}) != head ({})",
      m_region->entry()->id(), head
    );
    return m_region;
  }

private:
  static constexpr int kMaxNonDefaultCases = 4;
  static constexpr int kMinSwitchPercent = 75;

  /*
   * Look up profiling data for the Switch at the end of tid and decide which
   * outgoing arcs, if any, to include in the region. Arcs that survive this
   * function may still be trimmed by the other checks in visit().
   */
  void trimSwitchArcs(TransID tid,
                      const RegionDesc& profRegion,
                      std::vector<TransCFG::Arc*>& arcs) {
    ITRACE(5, "Analyzing Switch ending profTrans {}\n", tid);
    Trace::Indent indent;

    auto sk = profRegion.blocks().back()->last();
    assertx(sk.op() == OpSwitch);
    TargetProfile<SwitchProfile> profile({tid},
                                         TransKind::Optimize,
                                         sk,
                                         s_switchProfile.get());
    assertx(!profile.profiling());
    if (!profile.optimizing()) {
      // We don't have profile data for this Switch, most likely because it saw
      // some weird input type during profiling.
      arcs.clear();
      return;
    }

    NormalizedInstruction ni{sk, sk.unit()};
    std::vector<Offset> offsets;
    for (auto off : ni.immVec.range32()) offsets.push_back(sk.offset() + off);
    auto const data = sortedSwitchProfile(profile, offsets.size());
    uint32_t totalHits = 0;
    for (auto const& item : data) totalHits += item.count;
    if (totalHits == 0) {
      // This switch was never executed during profiling.
      arcs.clear();
      return;
    }

    // Allow arcs if the hottest kMaxNonDefaultCases account for at least
    // kMinSwitchPercent % of total profiling hits.
    uint32_t includedCases = 0;
    uint32_t includedHits = 0;
    jit::fast_set<SrcKey, SrcKey::Hasher> allowedSks;
    for (auto const& item : data) {
      // We always have bounds checks for the default, so it doesn't count
      // against the case limit.
      if (item.caseIdx == data.size() - 1) {
        ITRACE(5, "Adding {} hits from default case @ {}\n",
               item.count, offsets[item.caseIdx]);
        includedHits += item.count;
        allowedSks.insert(SrcKey{sk, offsets[item.caseIdx]});
        continue;
      }

      if (includedCases == kMaxNonDefaultCases) {
        if (includedHits * 100 / totalHits < kMinSwitchPercent) {
          ITRACE(5, "Profile data not biased towards hot cases; bailing\n");
          arcs.clear();
          return;
        }
        break;
      }

      ITRACE(5, "Adding {} hits from case {} @ {}\n",
             item.count, item.caseIdx, offsets[item.caseIdx]);
      ++includedCases;
      includedHits += item.count;
      allowedSks.insert(SrcKey{sk, offsets[item.caseIdx]});
    }

    ITRACE(5, "Including {} cases, representing {} / {} samples\n",
           includedCases, includedHits, totalHits);
    auto firstDead = std::remove_if(
      begin(arcs), end(arcs), [&](const TransCFG::Arc* arc) {
        auto const rec = m_profData->transRec(arc->dst());
        const bool ok = allowedSks.count(rec->srcKey());
        ITRACE(5, "Arc {} -> {} {}included\n",
               arc->src(), arc->dst(), ok ? "" : "not ");
        return !ok;
      }
    );
    arcs.erase(firstDead, end(arcs));
  }

  void visit(TransID tid) {
    // Skip tid if its weight is below the JitPGOMinBlockPercent
    // percentage of the weight of the block where this region
    // started.
    auto tidWeight = m_cfg.weight(tid);
    if (tidWeight < m_minBlockWeight) {
      ITRACE(5, "- visit: skipping {} due to low weight ({})\n",
             tid, tidWeight);
      return;
    }

    auto rec = m_profData->transRec(tid);
    auto tidRegion = rec->region();
    auto tidInstrs = tidRegion->instrSize();
    if (tidInstrs > m_numBCInstrs) {
      ITRACE(5, "- visit: skipping {} due to region size\n", tid);
      if (m_truncated) *m_truncated = true;
      if (!m_inlining) {
        logLowPriPerfWarning(
          "selectHotCFG",
          RO::EvalSelectHotCFGSampleRate * kDefaultPerfWarningRate,
          [&](StructuredLogEntry& cols) {
            cols.setInt("maxBCInstrSize", m_numBCInstrs);
            cols.setInt("tidRegionInstrSize", tidInstrs);
            auto sd = rec->func()->fullName();
            cols.setStr("funcName", sd->data());
          }
        );
      }
      return;
    }

    if (!m_visited.insert(tid).second) return;
    m_visiting.insert(tid);
    m_numBCInstrs -= tidInstrs;
    ITRACE(5, "- visit: adding {} ({})\n", tid, tidWeight);

    m_region->append(*tidRegion);

    auto const termSk = rec->lastSrcKey();
    auto const termOp = termSk.op();
    if (!breaksRegion(termSk)) {
      auto srcBlockId = tidRegion->blocks().back().get()->id();

      auto arcs = m_cfg.outArcs(tid);

      // We have special profiling and logic to decide which arcs from a Switch
      // are eligible for inclusion in the region.
      if (termOp == OpSwitch) {
        trimSwitchArcs(srcBlockId, *tidRegion, arcs);
      }

      for (auto const arc : arcs) {
        auto dst = arc->dst();

        // Skip if the probability of taking this arc is below the specified
        // threshold.
        if (arc->weight() < m_minArcProb * tidWeight) {
          ITRACE(5, "- visit: skipping arc {} -> {} due to low probability "
                 "({:.2})\n", tid, dst, arc->weight() / (tidWeight + 0.001));
          continue;
        }

        // Skip dst if we already generated a region starting at that SrcKey.
        auto dstRec = m_profData->transRec(dst);
        auto dstSK = dstRec->srcKey();
        if (!m_inlining && m_profData->optimized(dstSK)) {
          ITRACE(5, "- visit: skipping {} because SrcKey was already "
                 "optimize", showShort(dstSK));
          continue;
        }
        always_assert(dst == dstRec->region()->entry()->id());

        visit(dst);

        // Record the arc if dstBlockId was included in the region.  (Note that
        // it may not be included in the region due to the
        // EvalJitMaxRegionInstrs limit.)
        if (m_visited.count(dst)) {
          m_arcs.push_back({srcBlockId, dst});
        }
      }
    }

    always_assert(m_numBCInstrs >= 0);

    m_visiting.erase(tid);
  }

private:
  const ProfData*              m_profData;
  const TransCFG&              m_cfg;
  const TransIDSet&            m_entries;
  RegionDescPtr                m_region;
  int32_t                      m_numBCInstrs;
  jit::hash_set<TransID>       m_visiting;
  jit::hash_set<TransID>       m_visited;
  jit::vector<RegionDesc::Arc> m_arcs;
  double                       m_minBlockWeight;
  double                       m_minArcProb;
  bool                         m_inlining;
  bool*                        m_truncated;
};

//////////////////////////////////////////////////////////////////////

/*
 * blockProfCount() for a given block is the number of times that block was
 * executed from all possible paths. However, the region we've formed might not
 * contain all of those paths. Therefore the block counts might reflect
 * execution that cannot happen in this region. For example, a block with a
 * single predecessor might have a much higher prof count than its predecessor
 * (because there was another predecessor which got pruned off).
 *
 * Attempt to scale the prof counts in the blocks to reflect this (in an
 * approximate way). For each block, sum together the combined prof count of all
 * its original predecessors, as well as sum only the precessors in the
 * region. Scale the block's prof count by the ratio between the two.
 */
void scaleProfCounts(HotTransContext& ctx, RegionDescPtr region) {
  jit::fast_map<TransID, double> scales;
  jit::fast_set<TransID> inRegion;
  jit::fast_set<TransID> inEntryChain;
  jit::fast_map<TransID, TransIDSet> merged;

  ITRACE(4, "scaleProfCounts:\n");

  for (auto const& block : region->blocks()) {
    auto const bid = block->id();
    inRegion.emplace(bid);
    for (auto const m : region->merged(bid)) {
      inRegion.emplace(m);
      merged[bid].emplace(m);
    }
  }

  TransID ebid = region->entry()->id();
  while (true) {
    inEntryChain.insert(ebid);
    auto const next = region->nextRetrans(ebid);
    if (!next) break;
    ebid = *next;
  }

  auto const setScale = [&] (TransID tid, double scale) {
    assertx(!scales.count(tid));
    scales.emplace(tid, scale);
    for (auto const b : merged[tid]) {
      assertx(!scales.count(b));
      scales.emplace(b, scale);
    }
  };

  for (auto const& block : region->blocks()) {
    auto const bid = block->id();
    ITRACE(5, "  {} (weight {})\n", bid, region->blockProfCount(bid));

    assertx(region->blockProfCountScale(bid) == 1.0);

    if (inEntryChain.count(bid)) {
      ITRACE(5, "    entry block. No scaling\n");
      setScale(bid, 1.0);
      continue;
    }

    int64_t totalPredWeight = 0;
    int64_t includedPredWeight = 0;
    auto const& inArcs = ctx.cfg->inArcs(bid);
    for (auto const inArc : inArcs) {
      auto const pred = inArc->src();
      auto const predWeight = ctx.cfg->weight(pred);
      ITRACE(5, "    pred {} (weight {})\n", pred, predWeight);

      if (!inRegion.count(pred)) {
        ITRACE(5, "      not in region\n");
        totalPredWeight += predWeight;
        continue;
      }

      // The predecessor wasn't processed yet. This can only happen if its loop
      // back-edge. In this case, don't add the prof count to either
      // total. We'll just totally ignore the loop and assume the loop counts
      // will scale according to the non-loop predecessors.
      auto const it = scales.find(pred);
      if (it == scales.end()) {
        ITRACE(5, "      not processed yet. Part of cycle, so ignoring\n");
        continue;
      }

      ITRACE(5, "      scale = {}\n", it->second);
      totalPredWeight += predWeight;
      includedPredWeight += (predWeight * it->second);
    }

    auto const scale = [&] {
      if (totalPredWeight == 0) return 0.0;
      return includedPredWeight / (double)totalPredWeight;
    }();

    ITRACE(5, "    total pred = {}, included pred = {}, scale = {:.6f}\n",
           totalPredWeight, includedPredWeight, scale);
    setScale(bid, scale);
  }

  for (auto const& block : region->blocks()) {
    auto const bid = block->id();
    auto const it = scales.find(bid);
    assertx(it != scales.end());
    ITRACE(
      4, "  {}: scale = {:.6f} (old weight = {}, new weight = {})\n",
      bid,
      it->second,
      region->blockProfCount(bid),
      int64_t(region->blockProfCount(bid) * it->second)
    );
    region->setBlockProfCountScale(bid, it->second);
  }
}

/*
 * Finds the TransID with highest profile count among ctx's entries.
 */
TransID selectMainEntry(const HotTransContext& ctx) {
  TransID ret = kInvalidTransID;
  int64_t maxCount = -1;
  auto const profData = ctx.profData;
  for (auto tid : ctx.entries) {
    auto const count = profData->transCounter(tid);
    if (count > maxCount) {
      maxCount = count;
      ret = tid;
    }
  }
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

RegionDescPtr selectHotCFG(HotTransContext& ctx, bool* truncated) {
  if (truncated) *truncated = false;
  ITRACE(1, "selectHotCFG: starting with maxBCInstrs = {}\n", ctx.maxBCInstrs);
  assertx(ctx.entries.size() >= 1);
  auto const mainEntryTid = selectMainEntry(ctx);
  ITRACE(1, "  selected mainEntry: {}\n", mainEntryTid);
  assertx(mainEntryTid != kInvalidTransID);
  auto const region =
    DFS(ctx.profData, *ctx.cfg, ctx.entries, ctx.maxBCInstrs, ctx.inlining,
        truncated).formRegion(mainEntryTid);

  if (region->empty()) return nullptr;

  ITRACE(3, "selectHotCFG: before region_prune_arcs:\n{}\n",
         show(*region));
  region_prune_arcs(*region, ctx.inputTypes);

  ITRACE(3, "selectHotCFG: before chainRetransBlocks:\n{}\n",
         show(*region));
  region->chainRetransBlocks();

  // Relax the region guards.
  if (RuntimeOption::EvalRegionRelaxGuards) {
    ITRACE(3, "selectHotCFG: before optimizeProfiledGuards:\n{}\n",
           show(*region));
    optimizeProfiledGuards(*region, *ctx.profData);
  }

  ITRACE(1, "selectHotCFG: final version after optimizeProfiledGuards:\n{}\n",
         show(*region));

  scaleProfCounts(ctx, region);

  return region;
}

}}
