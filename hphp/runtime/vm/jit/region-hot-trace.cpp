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

static const Trace::Module TRACEMOD = Trace::pgo;

/**
 * Remove from pConds the elements that correspond to stack positions
 * that have been popped given the current SP offset from FP.
 */
static void discardPoppedTypes(PostConditions& pConds, int curSpOffset) {
  for (auto it = pConds.begin(); it != pConds.end(); ) {
    if (it->location.tag() == RegionDesc::Location::Tag::Stack &&
        it->location.stackOffsetFromFp() > curSpOffset) {
      it = pConds.erase(it);
    } else {
      it++;
    }
  }
}

static void mergePostConds(PostConditions& dst,
                           const PostConditions& src) {
  for (const auto &post : src) {
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

  PostConditions accumPostConds;
  // Maps BlockIds to the set of BC offsets for its successor blocks.
  // Used to prevent multiple successors with the same SrcKey for now.
  // This can go away once task #4157613 is done.
  hphp_hash_map<RegionDesc::BlockId, SrcKeySet> succSKSet;

  // Maps from BlockIds to accumulated post conditions for that block.
  // Used to determine if we can add branch-over edges by checking the
  // pre-conditions of the successor block.
  hphp_hash_map<RegionDesc::BlockId, PostConditions> blockPostConds;

  while (!selectedSet.count(tid)) {

    RegionDescPtr blockRegion = profData->transRegion(tid);
    if (blockRegion == nullptr) break;

    // If the debugger is attached, only allow single-block regions.
    if (prevId != kInvalidTransID && isDebuggerAttachedProcess()) {
      FTRACE(2, "selectHotTrace: breaking region at Translation {} "
             "because of debugger is attached\n", tid);
      break;
    }

    // Break if block is not the first and requires reffiness checks.
    // Task #2589970: fix translateRegion to support mid-region reffiness checks
    if (prevId != kInvalidTransID) {
      auto nRefDeps = blockRegion->entry()->reffinessPreds().size();
      if (nRefDeps > 0) {
        FTRACE(2, "selectHotTrace: breaking region because of refDeps ({}) at "
               "Translation {}\n", nRefDeps, tid);
        break;
      }
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

    // Break trace if translation tid cannot follow the execution of
    // the entire translation prevId.  This can only happen if the
    // execution of prevId takes a side exit that leads to the
    // execution of tid.
    if (prevId != kInvalidTransID) {
      Op* lastInstr = profData->transLastInstr(prevId);
      const Unit* unit = profData->transFunc(prevId)->unit();
      OffsetSet succOffs = instrSuccOffsets(lastInstr, unit);
      if (!succOffs.count(profData->transSrcKey(tid).offset())) {
        if (HPHP::Trace::moduleEnabled(HPHP::Trace::pgo, 2)) {
          FTRACE(2, "selectHotTrace: WARNING: Breaking region @: {}\n",
                 show(*region));
          FTRACE(2, "selectHotTrace: next translation selected: tid = {}\n{}\n",
                 tid, show(*blockRegion));
          FTRACE(2, "\nsuccOffs = {}\n", folly::join(", ", succOffs));
        }
        break;
      }
    }

    bool hasPredBlock = !region->empty();
    RegionDesc::BlockId predBlockId = (hasPredBlock ?
                                       region->blocks().back().get()->id() : 0);
    auto const& newFirstBlock = blockRegion->entry();
    auto newFirstBlockId = newFirstBlock->id();
    auto newFirstBlockSk = newFirstBlock->start();
    auto newLastBlockId  = blockRegion->blocks().back()->id();

    // Make sure we don't end up with multiple successors for the same
    // SrcKey. Task #4157613 will allow the following check to go away.
    // This needs to be done before we insert blockRegion into region,
    // to avoid creating unreachable blocks.
    if (RuntimeOption::EvalHHIRBytecodeControlFlow && hasPredBlock &&
        succSKSet[predBlockId].count(newFirstBlockSk)) {
      break;
    }

    // Add blockRegion's blocks and arcs to region.
    region->append(*blockRegion);

    if (hasPredBlock) {
      if (RuntimeOption::EvalHHIRBytecodeControlFlow) {
        // This is checked above.
        assert(succSKSet[predBlockId].count(newFirstBlockSk) == 0);
        succSKSet[predBlockId].insert(newFirstBlockSk);
      }
      region->addArc(predBlockId, newFirstBlockId);
    }

    // With bytecode control-flow, we add all forward arcs in the TransCFG
    // that are induced by the blocks in the region, as a simple way
    // to expose control-flow for now.
    // This can go away once Task #4075822 is done.
    if (RuntimeOption::EvalHHIRBytecodeControlFlow) {
      assert(hasTransId(newFirstBlockId));
      auto newTransId = getTransId(newFirstBlockId);
      auto& blocks = region->blocks();
      for (auto iOther = 0; iOther < blocks.size(); iOther++) {
        auto other = blocks[iOther];
        auto otherFirstBlockId = other.get()->id();
        if (!hasTransId(otherFirstBlockId)) continue;
        auto otherTransId = getTransId(otherFirstBlockId);
        auto otherFirstBlockSk = other.get()->start();
        auto otherRegion = profData->transRegion(otherTransId);
        auto otherLastBlockId = otherRegion->blocks().back()->id();
        // When loops are off, stop once we hit the newTransId we just inserted.
        if (!RuntimeOption::EvalJitLoops && otherTransId == newTransId) break;
        if (cfg.hasArc(otherTransId, newTransId) &&
            // Task #4157613 will allow the following check to go away
            !succSKSet[otherLastBlockId].count(newFirstBlockSk) &&
            preCondsAreSatisfied(newFirstBlock,
                                 blockPostConds[otherLastBlockId])) {
          region->addArc(otherLastBlockId, newFirstBlockId);
          succSKSet[otherLastBlockId].insert(newFirstBlockSk);
        }
        // When Eval.JitLoops is set, insert back-edges in the
        // region if they exist in the TransCFG.
        if (RuntimeOption::EvalJitLoops &&
            cfg.hasArc(newTransId, otherTransId) &&
            // Task #4157613 will allow the following check to go away
            !succSKSet[newLastBlockId].count(otherFirstBlockSk)) {
          region->addArc(newLastBlockId, otherFirstBlockId);
          succSKSet[newLastBlockId].insert(otherFirstBlockSk);
        }
      }
    }

    if (cfg.outArcs(tid).size() > 1) {
      region->setSideExitingBlock(blockRegion->entry()->id());
    }
    selectedSet.insert(tid);
    if (selectedVec) selectedVec->push_back(tid);

    Op lastOp = *(profData->transLastInstr(tid));
    if (breaksRegion(lastOp)) {
      FTRACE(2, "selectHotTrace: breaking region because of last instruction "
             "in Translation {}: {}\n", tid, opcodeToName(lastOp));
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
    assert(maxArc != nullptr);
    prevId = tid;
    tid = maxArc->dst();
  }

  return region;
}

} }
