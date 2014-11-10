/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <stack>

#include <folly/MapUtil.h>

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

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

  void visit(TransID tid) {
    auto tidRegion = m_profData->transRegion(tid);
    auto tidInstrs = tidRegion->instrSize();
    if (m_numBCInstrs + tidInstrs > RuntimeOption::EvalJitMaxRegionInstrs) {
      return;
    }

    if (m_visited.count(tid)) return;
    m_visited.insert(tid);
    m_visiting.insert(tid);

    auto sk = m_profData->transSrcKey(tid);
    m_srcKeyToTransID[sk] = tid;

    if (!breaksRegion(*(m_profData->transLastInstr(tid)))) {

      auto srcBlockId = tidRegion->blocks().back().get()->id();

      for (auto const arc : m_cfg.outArcs(tid)) {
        auto dst = arc->dst();

        // If dst is in the visiting set then this arc forms a cycle. Don't
        // include it unless we've asked for loops.
        if (!RuntimeOption::EvalJitLoops && m_visiting.count(dst)) continue;

        // Skip dst if region already has another block with the same SrcKey.
        // Task #4157613: add support for regions with multiple blocks with
        // the same SrcKey.
        auto dstSK = m_profData->transSrcKey(dst);
        if (folly::get_default(m_srcKeyToTransID, dstSK, dst) != dst) continue;

        // Skip dst if we already generated a region starting at that SrcKey.
        if (m_profData->optimized(dstSK)) continue;

        auto dstBlockId = m_profData->transRegion(dst)->entry()->id();
        m_arcs.push_back({srcBlockId, dstBlockId});

        visit(dst);
      }
    }

    // Now insert the region for tid in the front of m_region.  We do
    // this last so that the region ends up in (quasi-)topological order
    // (it'll be in topological order for acyclic regions).
    m_region->prepend(*tidRegion);
    m_selectedSet.insert(tid);
    if (m_selectedVec) m_selectedVec->push_back(tid);
    m_numBCInstrs += tidRegion->instrSize();
    always_assert(m_numBCInstrs <= RuntimeOption::EvalJitMaxRegionInstrs);

    m_visiting.erase(tid);
  }


 private:
  const ProfData*                m_profData;
  const TransCFG&                m_cfg;
  TransIDSet&                    m_selectedSet;
  TransIDVec*                    m_selectedVec;
  RegionDescPtr                  m_region;
  uint32_t                       m_numBCInstrs;
  jit::hash_set<TransID>         m_visiting;
  jit::hash_set<TransID>         m_visited;
  jit::flat_map<SrcKey, TransID> m_srcKeyToTransID;
  jit::vector<RegionDesc::Arc>   m_arcs;
};

/*
 * Constructs a region, beginning with triggerId, that includes as much of the
 * TransCFG as possible.  Excludes multiple translations of the same SrcKey.
 */
RegionDescPtr selectWholeCFG(TransID triggerId,
                             const ProfData* profData,
                             const TransCFG& cfg,
                             TransIDSet& selectedSet,
                             TransIDVec* selectedVec) {
  return DFS(profData, cfg, selectedSet, selectedVec).formRegion(triggerId);
}

}}
