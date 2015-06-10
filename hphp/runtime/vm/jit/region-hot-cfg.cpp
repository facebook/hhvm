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

#include <memory>
#include <algorithm>

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-prune-arcs.h"
#include "hphp/runtime/vm/jit/region-selection.h"
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
  DFS(const ProfData* p, const TransCFG& c, TransIDSet& ts, TransIDVec* tv)
    : m_profData(p)
    , m_cfg(c)
    , m_selectedSet(ts)
    , m_selectedVec(tv)
    , m_numBCInstrs(0)
  {}

  RegionDescPtr formRegion(TransID head) {
    m_region = std::make_shared<RegionDesc>();
    m_selectedSet.clear();
    if (m_selectedVec) m_selectedVec->clear();
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
    if (m_selectedVec) {
      std::reverse(m_selectedVec->begin(), m_selectedVec->end());
    }
    for (auto& arc : m_arcs) {
      m_region->addArc(arc.src, arc.dst);
    }
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
    assert(sk.op() == OpSwitch);
    TargetProfile<SwitchProfile> profile(tid, sk.offset(),
                                         s_switchProfile.get());
    assert(!profile.profiling());
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
    std::unordered_set<SrcKey, SrcKey::Hasher> allowedSks;
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
        const bool ok = allowedSks.count(m_profData->transSrcKey(arc->dst()));
        ITRACE(5, "Arc {} -> {} {}included\n",
               arc->src(), arc->dst(), ok ? "" : "not ");
        return !ok;
      }
    );
    arcs.erase(firstDead, end(arcs));
  }

  void visit(TransID tid) {
    auto tidRegion = m_profData->transRegion(tid);
    auto tidInstrs = tidRegion->instrSize();
    if (m_numBCInstrs + tidInstrs > RuntimeOption::EvalJitMaxRegionInstrs) {
      return;
    }

    // Skip tid if its weight is below the JitPGOMinBlockPercent
    // percentage of the weight of the block where this region
    // started.
    auto tidWeight = m_cfg.weight(tid);
    if (tidWeight < m_minBlockWeight) {
      ITRACE(5, "- visit: skipping {} due to low weight ({})\n",
             tid, tidWeight);
      return;
    }

    if (!m_visited.insert(tid).second) return;
    m_visiting.insert(tid);
    m_numBCInstrs += tidInstrs;
    ITRACE(5, "- visit: adding {} ({})\n", tid, tidWeight);

    auto const termSk = m_profData->transLastSrcKey(tid);
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

        // If dst is in the visiting set then this arc forms a cycle. Don't
        // include it unless we've asked for loops.
        if (!RuntimeOption::EvalJitLoops && m_visiting.count(dst)) {
          ITRACE(5, "- visit: skipping arc {} -> {} because it would create "
                 "a loop\n", tid, dst);
          continue;
        }

        // Skip dst if we already generated a region starting at that SrcKey.
        auto dstSK = m_profData->transSrcKey(dst);
        if (m_profData->optimized(dstSK)) {
          ITRACE(5, "- visit: skipping {} because SrcKey was already "
                 "optimize", showShort(dstSK));
          continue;
        }
        always_assert(dst == m_profData->transRegion(dst)->entry()->id());

        visit(dst);

        // Record the arc if dstBlockId was included in the region.  (Note that
        // it may not be included in the region due to the
        // EvalJitMaxRegionInstrs limit.)
        if (m_visited.count(dst)) {
          m_arcs.push_back({srcBlockId, dst});
        }
      }
    }

    // Now insert the region for tid in the front of m_region.  We do
    // this last so that the region ends up in (quasi-)topological order
    // (it'll be in topological order for acyclic regions).
    m_region->prepend(*tidRegion);
    m_selectedSet.insert(tid);
    if (m_selectedVec) m_selectedVec->push_back(tid);
    always_assert(m_numBCInstrs <= RuntimeOption::EvalJitMaxRegionInstrs);

    m_visiting.erase(tid);
  }

private:
  const ProfData*              m_profData;
  const TransCFG&              m_cfg;
  TransIDSet&                  m_selectedSet;
  TransIDVec*                  m_selectedVec;
  RegionDescPtr                m_region;
  uint32_t                     m_numBCInstrs;
  jit::hash_set<TransID>       m_visiting;
  jit::hash_set<TransID>       m_visited;
  jit::vector<RegionDesc::Arc> m_arcs;
  double                       m_minBlockWeight;
  double                       m_minArcProb;
};

//////////////////////////////////////////////////////////////////////

}

RegionDescPtr selectHotCFG(TransID head,
                           const ProfData* profData,
                           const TransCFG& cfg,
                           TransIDSet& selectedSet,
                           TransIDVec* selectedVec) {
  ITRACE(1, "selectHotCFG\n");
  auto const region =
    DFS(profData, cfg, selectedSet, selectedVec)
      .formRegion(head);
  ITRACE(3, "selectHotCFG: before region_prune_arcs:\n{}\n",
         show(*region));
  region_prune_arcs(*region);
  ITRACE(3, "selectHotCFG: before chainRetransBlocks:\n{}\n",
         show(*region));
  region->chainRetransBlocks();
  ITRACE(3, "selectHotCFG: after chainRetransBlocks:\n{}\n",
         show(*region));
  return region;
}

}}
