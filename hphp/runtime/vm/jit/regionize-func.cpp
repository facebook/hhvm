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

#include <algorithm>
#include <vector>

#include "hphp/util/assertions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

//////////////////////////////////////////////////////////////////////

namespace {

typedef hphp_hash_map<TransID, RegionDescPtr>            TransIDToRegionMap;
typedef hphp_hash_map<RegionDescPtr, TransIDVec,
                      smart_pointer_hash<RegionDescPtr>> RegionToTransIDsMap;

/**
 * Returns the set of of TransIDs that are in `bid's retranslation
 * chain in `region'.
 */
TransIDSet findRetransSet(const RegionDesc& region, RegionDesc::BlockId bid) {
  TransIDSet set;
  set.insert(bid);
  while (auto next = region.nextRetrans(bid)) {
    bid = next.value();
    set.insert(bid);
  }
  return set;
}

/**
 * Add `arc' to `coveredArcs', as well as any other arcs going from
 * arc.src into a retranslation of arc.dst in the `region'.
 */
void markCoveredArc(const TransCFG::Arc& arc,
                    const TransCFG& cfg,
                    const RegionDesc& dstRegion,
                    TransCFG::ArcPtrSet& coveredArcs) {
  auto dstRetransSet = findRetransSet(dstRegion, arc.dst());
  for (auto outArc : cfg.outArcs(arc.src())) {
    if (hasTransID(outArc->dst())) {
      auto dstTid = getTransID(outArc->dst());
      if (dstRetransSet.count(dstTid)) {
        coveredArcs.insert(outArc);
      }
    }
  }
}

/**
 * Add to sets coveredNodes and coveredArcs the cfg arcs that are now
 * covered given the new region containing the translations in
 * selectedVec.
 */
void markCovered(const TransCFG& cfg, const RegionDescPtr region,
                 const TransIDVec& selectedVec, const TransIDSet heads,
                 TransIDToRegionMap  headToRegion,
                 TransIDSet& coveredNodes,
                 TransCFG::ArcPtrSet& coveredArcs) {
  assertx(selectedVec.size() > 0);
  TransID newHead = selectedVec[0];
  assertx(!region->empty());
  assertx(newHead == getTransID(region->entry()->id()));

  // Mark all region's nodes as covered.
  coveredNodes.insert(selectedVec.begin(), selectedVec.end());

  // Mark all incoming arcs into newHead from covered nodes as covered.
  for (auto arc : cfg.inArcs(newHead)) {
    TransID src = arc->src();
    if (coveredNodes.count(src)) {
      markCoveredArc(*arc, cfg, *region, coveredArcs);
    }
  }

  // Mark all CFG arcs within the region as covered.
  region->forEachArc([&](RegionDesc::BlockId src, RegionDesc::BlockId dst) {
    if (!hasTransID(dst)) return;
    TransID srcTid = region->block(src)->profTransID();
    TransID dstTid = region->block(dst)->profTransID();
    assertx(cfg.hasArc(srcTid, dstTid));
    bool foundArc = false;
    for (auto arc : cfg.outArcs(srcTid)) {
      if (arc->dst() == dstTid) {
        markCoveredArc(*arc, cfg, *region, coveredArcs);
        foundArc = true;
      }
    }
    always_assert(foundArc);
  });

  // Mark all outgoing arcs from the region to a head node as covered.
  for (auto node : selectedVec) {
    for (auto arc : cfg.outArcs(node)) {
      if (heads.count(arc->dst())) {
        markCoveredArc(*arc, cfg, *headToRegion[arc->dst()], coveredArcs);
      }
    }
  }
}

/**
 * Returns the sum of the weights of the arcs going from srcs to dst.
 */
int64_t interRegionWeight(const TransIDVec& srcs,
                          TransID           dst,
                          const TransCFG&   cfg) {
  int64_t totalWeight = 0;
  for (auto src : srcs) {
    for (auto arc : cfg.outArcs(src)) {
      if (arc->dst() == dst) {
        totalWeight += arc->weight();
      }
    }
  }
  return totalWeight;
}

const TransIDVec& getRegionTransIDVec(const RegionToTransIDsMap& map,
                                      RegionDescPtr region) {
  auto it = map.find(region);
  assertx(it != map.end());
  return it->second;
}

/**
 * Sorts the regions vector in a linear order to be used for
 * translation.  The goal is to obtain an order that improves locality
 * when the function is executed.  Each region is translated separately.
 */
void sortRegions(RegionVec&                  regions,
                 const Func*                 func,
                 const TransCFG&             cfg,
                 const ProfData*             profData,
                 const TransIDToRegionMap&   headToRegion,
                 const RegionToTransIDsMap&  regionToTransIds) {
  RegionVec sorted;
  RegionSet selected;

  if (regions.empty()) return;

  // First, pick the region starting at the lowest bytecode offset.
  // This will normally correspond to the main function entry (for
  // normal, regular bytecode), but it may not be for irregular
  // functions written in hhas (like array_map and array_filter).  If
  // there multiple regions starting at the lowest bytecode offset,
  // pick the one with the largest profile weight.
  RegionDescPtr entryRegion = nullptr;
  int64_t    maxEntryWeight = -1;
  Offset     lowestOffset   = kInvalidOffset;
  for (const auto& pair : regionToTransIds) {
    auto  r    = pair.first;
    auto& tids = pair.second;
    TransID firstTid = tids[0];
    Offset firstOffset = profData->transSrcKey(firstTid).offset();
    int64_t weight = cfg.weight(firstTid);
    if (lowestOffset == kInvalidOffset || firstOffset < lowestOffset ||
        (firstOffset == lowestOffset && weight > maxEntryWeight)) {
      entryRegion    = r;
      maxEntryWeight = weight;
      lowestOffset   = firstOffset;
    }
  }

  assertx(entryRegion);
  sorted.push_back(entryRegion);
  selected.insert(entryRegion);

  RegionDescPtr region = entryRegion;
  // Select the remaining regions, iteratively picking the most likely
  // region to execute next.
  for (auto i = 1; i < regions.size(); i++) {
    int64_t      maxWeight = -1;
    int64_t  maxHeadWeight = -1;
    RegionDescPtr bestNext = nullptr;
    auto    regionTransIds = getRegionTransIDVec(regionToTransIds, region);
    for (auto next : regions) {
      if (selected.count(next)) continue;
      auto nextTransIds = getRegionTransIDVec(regionToTransIds, next);
      int64_t weight = interRegionWeight(regionTransIds, nextTransIds[0], cfg);
      int64_t headWeight = cfg.weight(nextTransIds[0]);
      if ((weight >  maxWeight) ||
          (weight == maxWeight && headWeight > maxHeadWeight)) {
        maxWeight     = weight;
        maxHeadWeight = headWeight;
        bestNext      = next;
      }
    }
    assertx(bestNext);
    sorted.push_back(bestNext);
    selected.insert(bestNext);
    region = bestNext;
  }

  assertx(sorted.size() == regions.size());
  regions = sorted;

  if (debug && Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    for (size_t i = 0; i < regions.size(); i++) {
      auto r = regions[i];
      auto tids = getRegionTransIDVec(regionToTransIds, r);
      std::string transIds = folly::join(", ", tids);
      FTRACE(6, "sortRegions: region[{}]: {}\n", i, transIds);
    }
  }
}

bool allArcsCovered(const TransCFG::ArcPtrVec& arcs,
                    const TransCFG::ArcPtrSet& coveredArcs) {
  for (auto arc : arcs) {
    if (!coveredArcs.count(arc)) {
      return false;
    }
  }
  return true;
}

}

//////////////////////////////////////////////////////////////////////

/**
 * Regionize a func, so that each node and each arc in its TransCFG is
 * "covered".  A node is covered if any region contains it.  An arc T1->T2
 * is covered if either:
 *
 *   a) T1 and T2 are in the same region R and R contains arc T1->T2.
 *   b) T2 is the head (first translation) of a region.
 *
 * Basic algorithm:
 *
 *   1) sort nodes in decreasing weight order
 *   2) for each node N:
 *      2.1) if N and all its incoming arcs are covered, then continue
 *      2.2) select a region starting at this node and mark nodes/arcs as
 *           covered appropriately
 */
void regionizeFunc(const Func* func,
                   MCGenerator* mcg,
                   RegionVec& regions) {
  const Timer rf_timer(Timer::regionizeFunc);
  assertx(RuntimeOption::EvalJitPGO);

  PGORegionMode regionMode = pgoRegionMode(*func);

  auto const funcId = func->getFuncId();
  auto const profData = mcg->tx().profData();
  TransCFG cfg(funcId, profData, mcg->tx().getSrcDB(),
               mcg->getJmpToTransIDMap());

  if (Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    auto dotFileName = folly::to<std::string>(
      "/tmp/func-cfg-", funcId, ".dot");
    cfg.print(dotFileName, funcId, profData, nullptr);
    FTRACE(5, "regionizeFunc: initial CFG for func {} saved to file {}\n",
           funcId, dotFileName);
  }

  TransCFG::ArcPtrVec   arcs = cfg.arcs();
  std::vector<TransID> nodes = cfg.nodes();

  std::sort(nodes.begin(), nodes.end(),
            [&](TransID tid1, TransID tid2) -> bool {
              if (regionMode == PGORegionMode::WholeCFG ||
                  regionMode == PGORegionMode::HotCFG) {
                auto bcOff1 = profData->transStartBcOff(tid1);
                auto bcOff2 = profData->transStartBcOff(tid2);
                if (bcOff1 != bcOff2) return bcOff1 < bcOff2;
              }
              if (cfg.weight(tid1) != cfg.weight(tid2)) {
                return cfg.weight(tid1) > cfg.weight(tid2);
              }
              // In case of ties, pick older translations first, in an
              // attempt to start loops at their headers.
              return tid1 < tid2;
            });

  TransCFG::ArcPtrSet coveredArcs;
  TransIDSet          coveredNodes;
  TransIDSet          heads;
  TransIDToRegionMap  headToRegion;
  RegionToTransIDsMap regionToTransIds;
  regions.clear();

  for (auto node : nodes) {
    if (!coveredNodes.count(node) ||
        !allArcsCovered(cfg.inArcs(node),  coveredArcs)) {
      TransID newHead = node;
      FTRACE(6, "regionizeFunc: selecting trace to cover node {}\n", newHead);
      TransIDSet selectedSet;
      TransIDVec selectedVec;
      RegionDescPtr region;
      switch (regionMode) {
        case PGORegionMode::Hottrace:
          region = selectHotTrace(newHead, profData, cfg,
                                  selectedSet, &selectedVec);
          break;

        case PGORegionMode::WholeCFG:
        case PGORegionMode::HotCFG:
          region = selectHotCFG(newHead, profData, cfg, selectedSet,
                                &selectedVec);
          break;

        case PGORegionMode::Hotblock:
          always_assert(0 && "Invalid value for EvalJitPGORegionSelector");
      }
      FTRACE(6, "regionizeFunc: selected region to cover node {}\n{}\n",
             newHead, show(*region));
      profData->setOptimized(profData->transSrcKey(newHead));
      assertx(selectedVec.size() > 0 && selectedVec[0] == newHead);
      regions.push_back(region);
      heads.insert(newHead);
      regionToTransIds[region] = selectedVec;
      headToRegion[newHead] = region;
      markCovered(cfg, region, selectedVec, heads, headToRegion,
                  coveredNodes, coveredArcs);

      FTRACE(6, "regionizeFunc: selected trace: {}\n",
             folly::join(", ", selectedVec));
    }
  }

  assertx(coveredNodes.size() == cfg.nodes().size());
  assertx(coveredArcs.size() == arcs.size());

  sortRegions(regions, func, cfg, profData, headToRegion, regionToTransIds);

  if (debug && Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    FTRACE(5, "\n--------------------------------------------\n"
           "regionizeFunc({}): computed regions:\n", funcId);
    for (auto region : regions) {
      FTRACE(5, "{}\n\n", show(*region));
    }
  }
}

} }
