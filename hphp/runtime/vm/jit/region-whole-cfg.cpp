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

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(pgo);

using boost::container::flat_set;
using boost::container::flat_map;

/*
 * Constructs a region, beginning with triggerId, that includes as much of the
 * TransCFG as possible.  Excludes multiple translations of the same SrcKey.
 */
RegionDescPtr selectWholeCFG(TransID triggerId,
                             const ProfData* profData,
                             TransCFG& cfg,
                             TransIDSet& selectedSet,
                             TransIDVec* selectedVec) {
  auto region = std::make_shared<RegionDesc>();
  selectedSet.clear();
  if (selectedVec) selectedVec->clear();

  std::stack<TransID> worklist;
  flat_set<TransID> visited;
  flat_map<SrcKey, TransID> srcKeyToTransID;
  flat_map<TransID, RegionDesc::BlockId> transBlocks;

  auto addToRegion = [&](TransID tid) {
    if (!visited.count(tid)) {
      auto transRegion = profData->transRegion(tid);
      auto sk = profData->transSrcKey(tid);
      region->blocks.insert(region->blocks.end(),
                            transRegion->blocks.begin(),
                            transRegion->blocks.end());
      region->arcs.insert(region->arcs.end(),
                          transRegion->arcs.begin(),
                          transRegion->arcs.end());
      selectedSet.insert(tid);
      if (selectedVec) selectedVec->push_back(tid);
      srcKeyToTransID[sk] = tid;
      transBlocks[tid] = transRegion->blocks.front().get()->id();
    }
  };

  // Initialize the region and bookkeeping.
  addToRegion(triggerId);
  worklist.push(triggerId);
  visited.insert(triggerId);

  // Traverse the CFG depth-first, adding blocks that meet the conditions.
  while (!worklist.empty()) {
    auto tid = worklist.top();
    worklist.pop();

    if (breaksRegion(*(profData->transLastInstr(tid)))) {
      continue;
    }

    for (auto const arc : cfg.outArcs(tid)) {
      auto dst = arc->dst();

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
      auto nRefDeps = dstRegion->blocks[0]->reffinessPreds().size();
      if (nRefDeps > 0) {
        continue;
      }

      // Add the block and arc to region.
      addToRegion(dst);
      auto predBlockId = profData->transRegion(tid)->blocks.back().get()->id();
      auto dstBlockId = dstRegion->blocks.front().get()->id();
      region->addArc(predBlockId, dstBlockId);

      // Push the dst if we haven't already processed it.
      if (visited.count(dst) == 0) {
        worklist.push(dst);
        visited.insert(dst);
      }
    }
  }
  return region;
}

}}
