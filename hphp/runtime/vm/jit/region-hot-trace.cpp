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

static bool postCondMismatch(const RegionDesc::TypePred& postCond,
                             const RegionDesc::TypePred& preCond) {
  return postCond.location == preCond.location &&
         !preCond.type.maybe(postCond.type);
}

static bool preCondsAreSatisfied(const RegionDesc::BlockPtr& block,
                                 const PostConditions& prevPostConds) {
  const auto& preConds = block->typePreds();
  for (const auto& it : preConds) {
    for (const auto& post : prevPostConds) {
      if (postCondMismatch(post, it.second)) {
        FTRACE(6, "preCondsAreSatisfied: postcondition check failed!\n"
               "  postcondition was {}, precondition was {}\n",
               show(post), show(it.second));
        return false;
      }
    }
  }
  return true;
}

RegionDescPtr selectHotTrace(TransID triggerId,
                             const ProfData* profData,
                             TransCFG& cfg,
                             TransIDSet& selectedSet) {
  JIT::RegionDescPtr region = smart::make_unique<JIT::RegionDesc>();
  TransID tid    = triggerId;
  TransID prevId = InvalidID;
  selectedSet.clear();

  PostConditions accumPostConds;

  while (!setContains(selectedSet, tid)) {

    RegionDesc::BlockPtr block = profData->transBlock(tid);
    if (block == nullptr) break;

    // If the debugger is attached, only allow single-block regions.
    if (prevId != InvalidID && isDebuggerAttachedProcess()) {
      FTRACE(5, "selectHotRegion: breaking region at Translation {} "
             "because of debugger is attached\n", tid);
      break;
    }

    // Break if block is not the first and requires reffiness checks.
    // Task #2589970: fix translateRegion to support mid-region reffiness checks
    if (prevId != InvalidID) {
      auto nRefDeps = block->reffinessPreds().size();
      if (nRefDeps > 0) {
        FTRACE(5, "selectHotRegion: breaking region because of refDeps ({}) at "
               "Translation {}\n", nRefDeps, tid);
        break;
      }
    }

    // Break trace if translation tid cannot follow the execution of
    // the entire translation prevTd.  This can only happen if the
    // execution of prevId takes a side exit that leads to the
    // execution of tid.
    if (prevId != InvalidID) {
      Op* lastInstr = profData->transLastInstr(prevId);
      const Unit* unit = profData->transFunc(prevId)->unit();
      OffsetSet succOffs = findSuccOffsets(lastInstr, unit);
      if (!setContains(succOffs, profData->transSrcKey(tid).offset())) {
        if (HPHP::Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
          FTRACE(5, "selectHotTrace: WARNING: Breaking region @: {}\n",
                 JIT::show(*region));
          FTRACE(5, "selectHotTrace: next translation selected: tid = {}\n{}\n",
                 tid, JIT::show(*block));
          std::string succStr("succOffs = ");
          for (auto succ : succOffs) {
            succStr += lexical_cast<std::string>(succ);
          }
          FTRACE(5, "\n{}\n", succStr);
        }
        break;
      }
    }
    region->blocks.emplace_back(block);
    selectedSet.insert(tid);

    Op lastOp = *(profData->transLastInstr(tid));
    if (breaksRegion(lastOp)) {
      FTRACE(5, "selectHotTrace: breaking region because of last instruction "
             "in Translation {}: {}\n", tid, opcodeToName(lastOp));
      break;
    }

    auto outArcs = cfg.outArcs(tid);
    if (outArcs.size() == 0) {
      FTRACE(5, "selectHotTrace: breaking region because there's no successor "
             "for Translation {}\n", tid);
      break;
    }

    mergePostConds(accumPostConds, block->postConds());

    TransCFG::ArcPtrVec possibleOutArcs;
    for (auto arc : outArcs) {
      RegionDesc::BlockPtr possibleNext = profData->transBlock(arc->dst());
      if (preCondsAreSatisfied(possibleNext, accumPostConds)) {
        possibleOutArcs.emplace_back(arc);
      }
    }

    if (possibleOutArcs.size() == 0) {
      FTRACE(5, "selectHotTrace: breaking region because postcondition check "
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
