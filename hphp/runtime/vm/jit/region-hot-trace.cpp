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

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include <limits>

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

/**
 * Remove from pConds the elements that correspond to stack positions
 * that have been popped given the current SP offset from FP.
 */
static void discardPoppedTypes(
  TypedLocations& pConds,
  FPInvOffset curSpOffset
) {
  for (auto it = pConds.begin(); it != pConds.end(); ) {
    if (it->location.tag() == RegionDesc::Location::Tag::Stack &&
        it->location.offsetFromFP() > curSpOffset) {
      it = pConds.erase(it);
    } else {
      it++;
    }
  }
}

static void mergePostConds(TypedLocations& dst,
                           const PostConditions& src) {
  for (const auto &post : src.changed) {
    bool replace = false;
    for (auto it = dst.begin(); it != dst.end(); ++it) {
      if (post.location == it->location) {
        *it = post;
        replace = true;
      }
    }
    if (!replace) {
      dst.emplace_back(post);
    }
  }
}

RegionDescPtr selectHotTrace(TransID triggerId,
                             const ProfData* profData,
                             TransCFG& cfg,
                             TransIDSet& selectedSet,
                             TransIDVec* selectedVec) {
  auto region = std::make_shared<RegionDesc>();
  TransID tid    = triggerId;
  TransID prevId = kInvalidTransID;
  selectedSet.clear();
  if (selectedVec) selectedVec->clear();

  TypedLocations accumPostConds;

  // Maps from BlockIds to accumulated post conditions for that block.
  // Used to determine if we can add branch-over edges by checking the
  // pre-conditions of the successor block.
  hphp_hash_map<RegionDesc::BlockId, TypedLocations> blockPostConds;

  uint32_t numBCInstrs = 0;

  while (!selectedSet.count(tid)) {

    RegionDescPtr blockRegion = profData->transRegion(tid);
    if (blockRegion == nullptr) break;

    // Break if region would be larger than the specified limit.
    auto newInstrSize = numBCInstrs + blockRegion->instrSize();
    if (newInstrSize > RuntimeOption::EvalJitMaxRegionInstrs) {
      FTRACE(2, "selectHotTrace: breaking region at Translation {} because "
             "size ({}) would exceed of maximum translation limit\n",
             tid, newInstrSize);
      break;
    }

    // If the debugger is attached, only allow single-block regions.
    if (prevId != kInvalidTransID && isDebuggerAttachedProcess()) {
      FTRACE(2, "selectHotTrace: breaking region at Translation {} "
             "because of debugger is attached\n", tid);
      break;
    }

    // Break if block is not the first and it corresponds to the main
    // function body entry.  This is to prevent creating multiple
    // large regions containing the function body (starting at various
    // DV funclets).
    if (prevId != kInvalidTransID) {
      const Func* func = profData->transFunc(tid);
      Offset  bcOffset = profData->transStartBcOff(tid);
      if (func->base() == bcOffset) {
        FTRACE(2, "selectHotTrace: breaking region because reached the main "
               "function body entry at Translation {} (BC offset {})\n",
               tid, bcOffset);
        break;
      }
    }

    if (prevId != kInvalidTransID) {
      auto sk = profData->transSrcKey(tid);
      if (profData->optimized(sk)) {
        FTRACE(2, "selectHotTrace: breaking region because next sk already "
               "optimized, for Translation {}\n", tid);
        break;
      }
    }

    bool hasPredBlock = !region->empty();
    RegionDesc::BlockId predBlockId = (hasPredBlock ?
                                       region->blocks().back().get()->id() : 0);
    auto const& newFirstBlock = blockRegion->entry();
    auto newFirstBlockId = newFirstBlock->id();
    auto newLastBlockId  = blockRegion->blocks().back()->id();

    // Add blockRegion's blocks and arcs to region.
    region->append(*blockRegion);
    numBCInstrs += blockRegion->instrSize();

    if (hasPredBlock) {
      region->addArc(predBlockId, newFirstBlockId);
    }

    // When Eval.JitLoops is set, insert back-edges in the region if
    // they exist in the TransCFG.
    if (RuntimeOption::EvalJitLoops) {
      assertx(hasTransID(newFirstBlockId));
      auto newTransId = getTransID(newFirstBlockId);
      // Don't add the arc if the last opcode in the source block ends
      // the region.
      if (!breaksRegion(profData->transLastSrcKey(newTransId))) {
        auto& blocks = region->blocks();
        for (auto iOther = 0; iOther < blocks.size(); iOther++) {
          auto other = blocks[iOther];
          auto otherFirstBlockId = other.get()->id();
          if (!hasTransID(otherFirstBlockId)) continue;
          auto otherTransId = getTransID(otherFirstBlockId);
          if (cfg.hasArc(newTransId, otherTransId)) {
            region->addArc(newLastBlockId, otherFirstBlockId);
          }
        }
      }
    }

    if (cfg.outArcs(tid).size() > 1) {
      region->setSideExitingBlock(blockRegion->entry()->id());
    }
    selectedSet.insert(tid);
    if (selectedVec) selectedVec->push_back(tid);

    const auto lastSk = profData->transLastSrcKey(tid);
    if (breaksRegion(lastSk)) {
      FTRACE(2, "selectHotTrace: breaking region because of last instruction "
             "in Translation {}: {}\n", tid, opcodeToName(lastSk.op()));
      break;
    }

    auto outArcs = cfg.outArcs(tid);
    if (outArcs.size() == 0) {
      FTRACE(2, "selectHotTrace: breaking region because there's no successor "
             "for Translation {}\n", tid);
      break;
    }

    auto newLastBlock = blockRegion->blocks().back();
    discardPoppedTypes(accumPostConds,
                       blockRegion->entry()->initialSpOffset());
    mergePostConds(accumPostConds, newLastBlock->postConds());
    blockPostConds[newLastBlock->id()] = accumPostConds;

    TransCFG::ArcPtrVec possibleOutArcs;
    for (auto arc : outArcs) {
      RegionDesc::BlockPtr possibleNext =
        profData->transRegion(arc->dst())->entry();
      if (preCondsAreSatisfied(possibleNext, accumPostConds)) {
        possibleOutArcs.emplace_back(arc);
      }
    }

    if (possibleOutArcs.size() == 0) {
      FTRACE(2, "selectHotTrace: breaking region because postcondition check "
             "pruned all successors of Translation {}\n", tid);
      break;
    }

    auto maxWeight = std::numeric_limits<int64_t>::min();
    TransCFG::Arc* maxArc = nullptr;
    for (auto arc : possibleOutArcs) {
      if (arc->weight() >= maxWeight) {
        maxWeight = arc->weight();
        maxArc = arc;
      }
    }
    assertx(maxArc != nullptr);
    prevId = tid;
    tid = maxArc->dst();
  }

  FTRACE(3, "selectHotTrace: before chainRetransBlocks:\n{}\n", show(*region));
  region->chainRetransBlocks();
  FTRACE(3, "selectHotTrace: after chainRetransBlocks:\n{}\n", show(*region));

  return region;
}

} }
