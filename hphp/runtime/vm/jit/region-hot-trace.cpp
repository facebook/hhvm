/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace JIT {

static const Trace::Module TRACEMOD = Trace::pgo;

/**
 * This function returns true for control-flow bytecode instructions that
 * are not support in the middle of a region yet.
 */
static bool breaksRegion(Op opc) {
  switch (opc) {
    case OpMIterNext:
    case OpMIterNextK:
    case OpSwitch:
    case OpSSwitch:
    case OpContSuspend:
    case OpContRetC:
    case OpRetC:
    case OpRetV:
    case OpExit:
    case OpFatal:
    case OpMIterInit:
    case OpMIterInitK:
    case OpIterBreak:
    case OpDecodeCufIter:
    case OpThrow:
    case OpUnwind:
    case OpEval:
    case OpNativeImpl:
    case OpContHandle:
      return true;

    default:
      return false;
  }
}

/**
 * Returns the set of bytecode offsets for the instructions that may
 * be executed immediately after opc.
 */
static OffsetSet findSuccOffsets(Op* opc, const Unit* unit) {
  OffsetSet succBcOffs;
  Op* bcStart = (Op*)(unit->entry());

  if (!instrIsControlFlow(*opc)) {
    Offset succOff = opc + instrLen(opc) - bcStart;
    succBcOffs.insert(succOff);
    return succBcOffs;
  }

  if (instrAllowsFallThru(*opc)) {
    Offset succOff = opc + instrLen(opc) - bcStart;
    succBcOffs.insert(succOff);
  }

  if (isSwitch(*opc)) {
    foreachSwitchTarget(opc, [&](Offset& offset) {
        succBcOffs.insert(offset);
      });
  } else {
    Offset target = instrJumpTarget(bcStart, opc - bcStart);
    if (target != InvalidAbsoluteOffset) {
      succBcOffs.insert(target);
    }
  }
  return succBcOffs;
}

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
  JIT::RegionDescPtr region = std::make_shared<JIT::RegionDesc>();
  TransID tid    = triggerId;
  TransID prevId = InvalidID;
  selectedSet.clear();
  if (selectedVec) selectedVec->clear();

  PostConditions accumPostConds;

  while (!setContains(selectedSet, tid)) {

    RegionDescPtr blockRegion = profData->transRegion(tid);
    if (blockRegion == nullptr) break;

    // If the debugger is attached, only allow single-block regions.
    if (prevId != InvalidID && isDebuggerAttachedProcess()) {
      FTRACE(2, "selectHotTrace: breaking region at Translation {} "
             "because of debugger is attached\n", tid);
      break;
    }

    // Break if block is not the first and requires reffiness checks.
    // Task #2589970: fix translateRegion to support mid-region reffiness checks
    if (prevId != InvalidID) {
      auto nRefDeps = blockRegion->blocks[0]->reffinessPreds().size();
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
    if (prevId != InvalidID) {
      const Func* func = profData->transFunc(tid);
      Offset  bcOffset = profData->transStartBcOff(tid);
      if (func->base() == bcOffset) {
        FTRACE(2, "selectHotTrace: breaking region because reached the main "
               "function body entry at Translation {} (BC offset {})\n",
               tid, bcOffset);
        break;
      }
    }

    if (prevId != InvalidID) {
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
    if (prevId != InvalidID) {
      Op* lastInstr = profData->transLastInstr(prevId);
      const Unit* unit = profData->transFunc(prevId)->unit();
      OffsetSet succOffs = findSuccOffsets(lastInstr, unit);
      if (!setContains(succOffs, profData->transSrcKey(tid).offset())) {
        if (HPHP::Trace::moduleEnabled(HPHP::Trace::pgo, 2)) {
          FTRACE(2, "selectHotTrace: WARNING: Breaking region @: {}\n",
                 JIT::show(*region));
          FTRACE(2, "selectHotTrace: next translation selected: tid = {}\n{}\n",
                 tid, JIT::show(*blockRegion));
          FTRACE(2, "\nsuccOffs = {}\n", folly::join(", ", succOffs));
        }
        break;
      }
    }
    region->blocks.insert(region->blocks.end(), blockRegion->blocks.begin(),
                          blockRegion->blocks.end());
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

    auto lastNewBlock = blockRegion->blocks.back();
    discardPoppedTypes(accumPostConds,
                       blockRegion->blocks[0]->initialSpOffset());
    mergePostConds(accumPostConds, lastNewBlock->postConds());

    TransCFG::ArcPtrVec possibleOutArcs;
    for (auto arc : outArcs) {
      RegionDesc::BlockPtr possibleNext =
        profData->transRegion(arc->dst())->blocks[0];
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
