/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/runtime/vm/jit/check.h"

#include <boost/next_prior.hpp>
#include <unordered_set>

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/irfactory.h"
#include "hphp/runtime/vm/jit/linearscan.h"
#include "hphp/runtime/vm/jit/physreg.h"

namespace HPHP {  namespace JIT {

namespace {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

enum Limits : unsigned {
  kNumRegisters = Transl::kNumRegs,
  kNumSlots = NumPreAllocatedSpillLocs
};

struct RegState {
  RegState() {
    memset(regs, 0, sizeof(regs));
    memset(slots, 0, sizeof(slots));
  }
  SSATmp* regs[kNumRegisters];  // which tmp is in each register
  SSATmp* slots[kNumSlots]; // which tmp is in each spill slot
  SSATmp*& tmp(const RegisterInfo& info, int i) {
    if (info.spilled()) {
      auto slot = info.spillInfo(i).slot();
      assert(unsigned(slot) < kNumSlots);
      return slots[slot];
    }
    auto r = info.reg(i);
    assert(r != Transl::InvalidReg && unsigned(int(r)) < kNumRegisters);
    return regs[int(r)];
  }
};

// Return the number of parameters required for this block
DEBUG_ONLY static int numBlockParams(Block* b) {
  return b->empty() || b->front()->op() != DefLabel ? 0 :
         b->front()->numDsts();
}

/*
 * Check one block for being well formed. Invariants verified:
 * 1. The block begins with an optional DefLabel, followed by an optional
 *    BeginCatch, followed by either a Marker or no more instructions.
 * 2. DefLabel and BeginCatch may not appear anywhere in a block other than
 *    where specified in #1.
 * 3. If the optional BeginCatch is present, the block must belong to an exit
 *    trace and must be the first block in its Trace's block list.
 * 4. If any instruction is isBlockEnd(), it must be last.
 * 5. If the last instruction isTerminal(), block->next must be null.
 * 6. If the DefLabel produces a value, all of its incoming edges must be from
 *    blocks listed in the block list for this block's Trace.
 * 7. Any path from this block to a Block that expects values must be
 *    from a Jmp_ instruciton.
 */
bool checkBlock(Block* b) {
  auto it = b->begin();
  auto end = b->end();
  if (it == end) return true;

  // Invariant #1
  if (it->op() == DefLabel) ++it;

  // Invariant #1, #3
  if (it != end && it->op() == BeginCatch) {
    ++it;
    assert(!b->trace()->isMain());
    assert(b == b->trace()->front());
  }

  // Invariant #3
  if (it == end) return true;
  assert(it->op() == Marker);

  // Invariants #2, #4
  if (++it == end) return true;
  if (b->back()->isBlockEnd()) --end;
  while (it != end && it->op() == Marker) ++it;
  for (DEBUG_ONLY IRInstruction& inst : folly::makeRange(it, end)) {
    assert(inst.op() != DefLabel);
    assert(inst.op() != BeginCatch);
    assert(!inst.isBlockEnd());
  }
  for (DEBUG_ONLY IRInstruction& inst : *b) {
    assert(inst.block() == b);
  }

  // Invariant #5
  assert(IMPLIES(b->back()->isTerminal(), !b->next()));

  // Invariant #7
  if (b->taken()) {
    // only Jmp_ can branch to a join block expecting values.
    DEBUG_ONLY IRInstruction* branch = b->back();
    DEBUG_ONLY auto numArgs = branch->op() == Jmp_ ? branch->numSrcs() : 0;
    assert(numBlockParams(b->taken()) == numArgs);
  }

  // Invariant #6
  if (b->front()->op() == DefLabel) {
    for (int i = 0; i < b->front()->numDsts(); ++i) {
      auto const traceBlocks = b->trace()->blocks();
      b->forEachSrc(i, [&](IRInstruction* inst, SSATmp*) {
        assert(std::find(traceBlocks.begin(), traceBlocks.end(),
                         inst->block()) != traceBlocks.end());
      });
    }
  }

  return true;
}

/*
 * Check that every catch trace has at most one incoming branch and a single
 * block.
 */
bool checkCatchTraces(Trace* trace, const IRFactory& irFactory) {
  forEachTraceBlock(trace, [&](Block* b) {
    auto trace = b->trace();
    if (trace->isCatch()) {
      assert(trace->blocks().size() == 1);
      assert(b->preds().size() <= 1);
    }
  });
  return true;
}

}

const Edge* takenEdge(IRInstruction* inst) {
  return inst->m_taken.to() ? &inst->m_taken : nullptr;
}

const Edge* takenEdge(Block* b) {
  return takenEdge(b->back());
}

const Edge* nextEdge(Block* b) {
  return b->m_next.to() ? &b->m_next : nullptr;
}

//////////////////////////////////////////////////////////////////////

/*
 * Build the CFG, then the dominator tree, then use it to validate SSA.
 * 1. Each src must be defined by some other instruction, and each dst must
 *    be defined by the current instruction.
 * 2. Each src must be defined earlier in the same block or in a dominator.
 * 3. Each dst must not be previously defined.
 * 4. Treat tmps defined by DefConst as always defined.
 * 5. Each predecessor of a reachable block must be reachable (deleted
 *    blocks must not have out-edges to reachable blocks).
 */
bool checkCfg(Trace* trace, const IRFactory& factory) {
  forEachTraceBlock(trace, checkBlock);

  // Check valid successor/predecessor edges.
  auto const blocks = rpoSortCfg(trace, factory);
  std::unordered_set<const Edge*> edges;
  for (Block* b : blocks) {
    auto checkEdge = [&] (const Edge* e) {
      assert(e->from() == b);
      edges.insert(e);
      for (auto& p : e->to()->preds()) if (&p == e) return;
      assert(false); // did not find edge.
    };
    if (auto *e = nextEdge(b))  checkEdge(e);
    if (auto *e = takenEdge(b)) checkEdge(e);
  }
  for (Block* b : blocks) {
    for (DEBUG_ONLY auto const &e : b->preds()) {
      assert(edges.find(&e) != edges.end());
      assert(&e == takenEdge(e.from()) || &e == nextEdge(e.from()));
      assert(e.to() == b);
    }
  }

  checkCatchTraces(trace, factory);

  // visit dom tree in preorder, checking all tmps
  auto const children = findDomChildren(blocks);
  StateVector<SSATmp, bool> defined0(&factory, false);
  forPreorderDoms(blocks.front(), children, defined0,
                  [] (Block* block, StateVector<SSATmp, bool>& defined) {
    for (IRInstruction& inst : *block) {
      for (DEBUG_ONLY SSATmp* src : inst.srcs()) {
        assert(src->inst() != &inst);
        assert_log(src->inst()->op() == DefConst ||
                   defined[src],
                   [&]{ return folly::format(
                       "src '{}' in '{}' came from '{}', which is not a "
                       "DefConst and is not defined at this use site",
                       src->toString(), inst.toString(),
                       src->inst()->toString()).str();
                   });
      }
      for (SSATmp& dst : inst.dsts()) {
        assert(dst.inst() == &inst && inst.op() != DefConst);
        assert(!defined[dst]);
        defined[dst] = true;
      }
    }
  });
  return true;
}

bool checkTmpsSpanningCalls(Trace* trace, const IRFactory& irFactory) {
  auto const blocks   = rpoSortCfg(trace, irFactory);
  auto const children = findDomChildren(blocks);

  // CallBuiltin is ok because it is not a php-level call.  (It will
  // call a C++ helper and we can push/pop around it normally.)
  auto isCall = [&] (Opcode op) {
    return op == Call || op == CallArray;
  };

  typedef StateVector<SSATmp,bool> State;

  bool isValid = true;
  forPreorderDoms(
    blocks.front(), children, State(&irFactory, false),
    [&] (Block* b, State& state) {
      for (auto& inst : *b) {
        for (auto& src : inst.srcs()) {
          if (src->isA(Type::FramePtr)) continue;
          if (src->isConst()) continue;
          if (!state[src]) {
            FTRACE(1, "checkTmpsSpanningCalls failed\n"
                      "  instruction: {}\n"
                      "  src:         {}\n",
                      inst.toString(),
                      src->toString());
            isValid = false;
          }
        }

        /*
         * Php calls kill all live temporaries.  We can't keep them
         * alive across the call because we currently have no
         * callee-saved registers in our abi, and all translations
         * share the same spill slots.
         */
        if (isCall(inst.op())) state.reset();

        for (auto& d : inst.dsts()) {
          state[d] = true;
        }
      }
    }
  );

  return isValid;
}

bool checkRegisters(Trace* trace, const IRFactory& factory,
                    const RegAllocInfo& regs) {
  assert(checkCfg(trace, factory));

  auto blocks = rpoSortCfg(trace, factory);
  auto children = findDomChildren(blocks);
  forPreorderDoms(blocks.front(), children, RegState(),
                  [&] (Block* block, RegState& state) {
    for (IRInstruction& inst : *block) {
      for (SSATmp* src : inst.srcs()) {
        auto const &info = regs[src];
        if (!info.spilled() &&
            (info.reg(0) == Transl::rVmSp ||
             info.reg(0) == Transl::rVmFp)) {
          // hack - ignore rbx and rbp
          continue;
        }
        for (unsigned i = 0, n = info.numAllocatedRegs(); i < n; ++i) {
          assert(state.tmp(info, i) == src);
        }
      }
      for (SSATmp& dst : inst.dsts()) {
        auto const &info = regs[dst];
        for (unsigned i = 0, n = info.numAllocatedRegs(); i < n; ++i) {
          state.tmp(info, i) = &dst;
        }
      }
    }
  });

  return true;
}

}}

