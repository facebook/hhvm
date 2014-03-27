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
#include "hphp/runtime/vm/jit/region-selection.h"
#include <algorithm>
#include <vector>

#include "hphp/util/assertions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/region-hot-trace.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(pgo);


typedef hphp_hash_map<TransID, RegionDescPtr>            TransIDToRegionMap;
typedef hphp_hash_map<RegionDescPtr, TransIDVec,
                      smart_pointer_hash<RegionDescPtr>> RegionToTransIDsMap;

/**
 * Add to sets coveredNodes and coveredArcs the cfg arcs that are now
 * covered given the new region containing the translations in
 * selectedVec.
 */
static void markCovered(const TransCFG& cfg, const TransIDVec selectedVec,
                        const TransIDSet heads, TransIDSet& coveredNodes,
                        TransCFG::ArcPtrSet& coveredArcs) {
  assert(selectedVec.size() > 0);
  TransID newHead = selectedVec[0];

  // Mark all region's nodes as covered.
  coveredNodes.insert(selectedVec.begin(), selectedVec.end());

  // Mark all incoming arcs into newHead from covered nodes as covered.
  for (auto arc : cfg.inArcs(newHead)) {
    TransID src = arc->src();
    if (coveredNodes.count(src)) {
      coveredArcs.insert(arc);
    }
  }

  // Mark all arcs between consecutive region nodes as covered.
  for (size_t i = 0; i < selectedVec.size() - 1; i++) {
    TransID node = selectedVec[i];
    TransID next = selectedVec[i + 1];
    bool foundArc = false;
    for (auto arc : cfg.outArcs(node)) {
      if (arc->dst() == next) {
        coveredArcs.insert(arc);
        foundArc = true;
      }
    }
    always_assert(foundArc);
  }

  // Mark all outgoing arcs from the region to a head node as covered.
  for (auto node : selectedVec) {
    for (auto arc : cfg.outArcs(node)) {
      if (heads.count(arc->dst())) {
        coveredArcs.insert(arc);
      }
    }
  }
}

/**
 * Returns the sum of the weights of the arcs going from srcs to dst.
 */
static int64_t interRegionWeight(const TransIDVec& srcs,
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

static const TransIDVec& getRegionTransIDVec(const RegionToTransIDsMap& map,
                                             RegionDescPtr region) {
  auto it = map.find(region);
  assert(it != map.end());
  return it->second;
}

/**
 * Sorts the regions vector in a linear order to be used for
 * translation.  The goal is to obtain an order that improves locality
 * when the function is executed.
 */
static void sortRegion(RegionVec&                  regions,
                       const Func*                 func,
                       const TransCFG&             cfg,
                       const ProfData*             profData,
                       const TransIDToRegionMap&   headToRegion,
                       const RegionToTransIDsMap&  regionToTransIds) {
  RegionVec sorted;
  RegionSet selected;

  if (regions.size() == 0) return;

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

  assert(entryRegion);
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
    assert(bestNext);
    sorted.push_back(bestNext);
    selected.insert(bestNext);
    region = bestNext;
  }

  assert(sorted.size() == regions.size());
  regions = sorted;

  if (debug && Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    for (size_t i = 0; i < regions.size(); i++) {
      auto r = regions[i];
      auto tids = getRegionTransIDVec(regionToTransIds, r);
      std::string transIds = folly::join(", ", tids);
      FTRACE(6, "sortRegion: region[{}]: {}\n", i, transIds);
    }
  }
}

static bool allArcsCovered(const TransCFG::ArcPtrVec& arcs,
                           const TransCFG::ArcPtrSet& coveredArcs) {
  for (auto arc : arcs) {
    if (!coveredArcs.count(arc)) {
      return false;
    }
  }
  return true;
}

/**
 * Regionize a func, so that each node and each arc in its TransCFG is
 * "covered".  A node is covered if any region contains it.  An arc T1->T2
 * is covered if either:
 *
 *   a) T1 and T2 are in the same region R and T2 immediately follows
 *      T1 in R.
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
void regionizeFunc(const Func*       func,
                   JIT::MCGenerator* mcg,
                   RegionVec&        regions) {
  Timer _t(Timer::regionizeFunc);
  assert(RuntimeOption::EvalJitPGO);
  FuncId funcId = func->getFuncId();
  ProfData* profData = mcg->tx().profData();
  TransCFG cfg(funcId, profData, mcg->tx().getSrcDB(),
               mcg->getJmpToTransIDMap());

  if (Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    auto dotFileName = folly::to<std::string>(
      "/tmp/func-cfg-", funcId, ".dot");
    cfg.print(dotFileName, funcId, profData, nullptr);
    FTRACE(5, "regionizeFunc: initial CFG for func {} saved to file {}\n",
           funcId, dotFileName);
  }

  TransCFG::ArcPtrVec arcs = cfg.arcs();
  std::vector<TransID>    nodes = cfg.nodes();

  std::sort(nodes.begin(), nodes.end(),
            [&](TransID tid1, TransID tid2) -> bool {
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
      RegionDescPtr region = selectHotTrace(newHead, profData, cfg,
                                            selectedSet, &selectedVec);
      profData->setOptimized(profData->transSrcKey(newHead));
      assert(selectedVec.size() > 0 && selectedVec[0] == newHead);
      regions.push_back(region);
      heads.insert(newHead);
      markCovered(cfg, selectedVec, heads, coveredNodes, coveredArcs);
      regionToTransIds[region] = selectedVec;
      headToRegion[newHead] = region;

      FTRACE(6, "regionizeFunc: selected trace: {}\n",
             folly::join(", ", selectedVec));
    }
  }

  assert(coveredNodes.size() == cfg.nodes().size());
  assert(coveredArcs.size() == arcs.size());

  sortRegion(regions, func, cfg, profData, headToRegion, regionToTransIds);

  if (debug && Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    FTRACE(5, "\n--------------------------------------------\n"
           "regionizeFunc({}): computed regions:\n", funcId);
    for (auto region : regions) {
      FTRACE(5, "{}\n\n", show(*region));
    }
  }
}

} }
