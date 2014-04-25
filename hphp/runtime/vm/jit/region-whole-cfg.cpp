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

#include "hphp/runtime/vm/jit/region-whole-cfg.h"

#include "hphp/runtime/vm/jit/trans-cfg.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(pgo);

RegionDescPtr selectWholeCFG(TransID triggerId,
                             const ProfData* profData,
                             TransCFG& cfg,
                             TransIDSet& selectedSet,
                             TransIDVec* selectedVec) {
  auto region = std::make_shared<RegionDesc>();
  selectedSet.clear();
  if (selectedVec) selectedVec->clear();

  smart::map<TransID, RegionDesc::BlockId> transIdToBlockIdMap;

  for (auto tid : cfg.nodes()) {
    RegionDescPtr blockRegion = profData->transRegion(tid);
    assert(blockRegion->blocks.size() == 1);
    for (auto block : blockRegion->blocks) {
      region->blocks.insert(region->blocks.end(), block);
      transIdToBlockIdMap[tid] = block->id();
    }
    selectedSet.insert(tid);
    if (selectedVec) selectedVec->push_back(tid);
  }
  for (auto arc : cfg.arcs()) {
    region->addArc(transIdToBlockIdMap[arc->src()],
                   transIdToBlockIdMap[arc->dst()]);
  }
  return region;
}

} // namespace JIT
} // namespace HPHP
