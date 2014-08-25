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

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

struct DFS {
  DFS(const ProfData* p, const TransCFG& c, TransIDSet& ts, TransIDVec* tv)
      : profData(p)
      , cfg(c)
      , selectedSet(ts)
      , selectedVec(tv)
      , region(std::make_shared<RegionDesc>())
    {}

  RegionDescPtr go(TransID tid) {
    auto sk = profData->transSrcKey(tid);
    srcKeyToTransID[sk] = tid;
    visiting.insert(tid);
    visited.insert(tid);

    if (breaksRegion(*(profData->transLastInstr(tid)))) {
      select(tid);
      visiting.erase(tid);
      return region;
    }

    for (auto const arc : cfg.outArcs(tid)) {
      auto dst = arc->dst();

      // If dst is in the visiting set then this arc forms a cycle. Don't
      // include it unless we've asked for loops.
      if (!RuntimeOption::EvalJitLoops &&
          visiting.find(dst) != visiting.end()) {
        continue;
      }

      // Don't select dst if SrcKey has already been used for a different
      // TransID.
      auto dstSK = profData->transSrcKey(dst);
      if (srcKeyToTransID.count(dstSK) > 0 && srcKeyToTransID[dstSK] != dst) {
        continue;
      }

      // Break if dst requires reffiness checks.
      // TODO(#2589970): Fix translateRegion to support mid-region reffiness
      // checks
      auto dstRegion = profData->transRegion(dst);
      auto nRefDeps = dstRegion->entry()->reffinessPreds().size();
      if (nRefDeps > 0) {
        continue;
      }

      // Add the block and arc to region.
      auto predBlockId =
          profData->transRegion(tid)->blocks().back().get()->id();
      auto dstBlockId = dstRegion->blocks().front().get()->id();
      region->addArc(predBlockId, dstBlockId);

      // Push the dst if we haven't already processed it.
      if (visited.count(dst) == 0) {
        go(dst);
      }
    }

    select(tid);
    visiting.erase(tid);
    return region;
  }

 private:
  void select(TransID tid) {
    auto transRegion = profData->transRegion(tid);
    region->prepend(*transRegion);
    selectedSet.insert(tid);
    if (selectedVec) selectedVec->insert(selectedVec->begin(), tid);
  }

 private:
  const ProfData* profData;
  const TransCFG& cfg;
  TransIDSet& selectedSet;
  TransIDVec* selectedVec;

  RegionDescPtr region;

  std::unordered_set<TransID> visiting;
  boost::container::flat_set<TransID> visited;
  boost::container::flat_map<SrcKey, TransID> srcKeyToTransID;
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
  selectedSet.clear();
  if (selectedVec) selectedVec->clear();
  return DFS(profData, cfg, selectedSet, selectedVec).go(triggerId);
}

}}
