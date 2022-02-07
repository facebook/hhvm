/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/vm/jit/vm-reg-liveness.h"

#include "hphp/util/dataflow-worklist.h"

#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP::jit {

TRACE_SET_MOD(hhir_store);

namespace {

//////////////////////////////////////////////////////////////////////

KnownRegState livenessUnion(KnownRegState a, KnownRegState b) {
  return (a == KnownRegState::MaybeLive || a != b)
    ? KnownRegState::MaybeLive
    : a;
}

std::string livenessToStr(KnownRegState r) {
  switch (r) {
    case KnownRegState::Dead:      return "Dead";
    case KnownRegState::MaybeLive: return "MaybeLive";
    case KnownRegState::Live:      return "Live";
    default:
      always_assert(false);
  }
}

using PostOrderId = uint32_t;

//////////////////////////////////////////////////////////////////////

}

StateVector<IRInstruction,KnownRegState> analyzeVMRegLiveness(
    IRUnit& unit, const BlockList& poBlockList) {
  const BlockList rpoBlocks{poBlockList.rbegin(), poBlockList.rend()};
  auto const rpoIDs = numberBlocks(unit, rpoBlocks);

  auto vmRegsLiveness = StateVector<IRInstruction, KnownRegState>(
      unit, KnownRegState::Dead);

  using Sorter = std::less<PostOrderId>;
  auto incompleteQ =
    dataflow_worklist<PostOrderId, Sorter>(unit.numBlocks());

  for (auto rpoId = uint32_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    incompleteQ.push(rpoId);
  }

  StateVector<Block,KnownRegState> liveIn{unit, KnownRegState::Dead};
  StateVector<Block,KnownRegState> liveOut{unit, KnownRegState::Dead};

  while (!incompleteQ.empty()) {
    auto const rpoId = incompleteQ.pop();
    auto const blk = rpoBlocks[rpoId];

    FTRACE(2, "  scanning B{}\n", blk->id());
    auto live = liveIn[blk];

    FTRACE(2, "    in: {}\n", livenessToStr(live));

    for (auto const& inst : *blk) {
      if (inst.is(StVMRegState)) {
        assertx(inst.src(0)->hasConstVal(TInt));
        auto const regState =
          static_cast<VMRegState>(inst.src(0)->intVal());
        assertx(
          regState == eagerlyCleanState() ||
          regState == VMRegState::DIRTY);
        live = (regState == eagerlyCleanState())
          ? KnownRegState::Live
          : KnownRegState::Dead;
      } else if (inst.is(BeginCatch)) {
        // VMRegs are live within catch traces.
        live = KnownRegState::Live;
      } else if (inst.is(InterpOne)) {
        // InterpOne marks the regState dirty before returning.
        live = KnownRegState::Dead;
      }
      vmRegsLiveness[inst] = live;
    }
    FTRACE(2, "    out: {}\n", livenessToStr(live));

    if (live == liveOut[blk]) continue;
    liveOut[blk] = live;

    blk->forEachSucc([&](Block* succ) {
      auto newLive = KnownRegState::Dead;
      bool first = true;
      succ->forEachPred([&](Block* pred) {
        if (first) {
          newLive = liveOut[pred];
          first = false;
        } else {
          newLive = livenessUnion(newLive, liveOut[pred]);
        }
      });

      if (newLive != liveIn[succ]) {
        liveIn[succ] = newLive;
        incompleteQ.push(rpoIDs[succ]);
      }
    });
  }

  return vmRegsLiveness;
}

}
