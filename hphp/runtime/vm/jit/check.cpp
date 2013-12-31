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
#include "hphp/runtime/vm/jit/check.h"

#include <boost/next_prior.hpp>
#include <unordered_set>

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP {  namespace JIT {

namespace {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

struct RegState {
  RegState();
  SSATmp*& tmp(const PhysLoc&, int i);
  void merge(const RegState& other);
  PhysReg::Map<SSATmp*> regs;  // which tmp is in each register
  SSATmp* slots[NumPreAllocatedSpillLocs]; // which tmp is in each spill slot
};

RegState::RegState() {
  memset(slots, 0, sizeof(slots));
}

SSATmp*& RegState::tmp(const PhysLoc& loc, int i) {
  if (loc.spilled()) {
    assert(loc.slot(i) < NumPreAllocatedSpillLocs);
    return slots[loc.slot(i)];
  }
  auto r = loc.reg(i);
  assert(r != JIT::InvalidReg);
  return regs[r];
}

void RegState::merge(const RegState& other) {
  for (auto r : regs) {
    if (regs[r] != other.regs[r]) regs[r] = nullptr;
  }
  for (unsigned i = 0; i < NumPreAllocatedSpillLocs; i++) {
    if (slots[i] != other.slots[i]) slots[i] = nullptr;
  }
}

// Return the number of parameters required for this block
DEBUG_ONLY static int numBlockParams(Block* b) {
  return b->empty() || b->front().op() != DefLabel ? 0 :
         b->front().numDsts();
}

/*
 * Check one block for being well formed. Invariants verified:
 * 1. The block begins with an optional DefLabel, followed by an optional
 *    BeginCatch.
 * 2. DefLabel and BeginCatch may not appear anywhere in a block other than
 *    where specified in #1.
 * 3. If the optional BeginCatch is present, the block must belong to an exit
 *    trace and must be the first block in its Trace's block list.
 * 4. The last instruction must be isBlockEnd() and the middle instructions
 *    must not be isBlockEnd().  Therefore, blocks cannot be empty.
 * 5. If the last instruction isTerminal(), block->next() must be null.
 * 6. If the DefLabel produces a value, all of its incoming edges must be from
 *    blocks listed in the block list for this block's Trace.
 * 7. Any path from this block to a Block that expects values must be
 *    from a Jmp instruciton.
 * 8. Every instruction's BCMarker must point to a valid bytecode instruction.
 * 9. If this block is a catch block, it must have at most one predecessor
 *    and the trace containing it must contain exactly this block.
 * 10.Every instruction with the MayRaiseError flag must have a catch trace
 *    attached to it.
 */
bool checkBlock(Block* b) {
  auto it = b->begin();
  auto end = b->end();
  assert(!b->empty());

  // Invariant #1
  if (it->op() == DefLabel) ++it;

  // Invariant #1, #3
  if (it != end && it->op() == BeginCatch) {
    ++it;
    assert(!b->trace()->isMain());
    assert(b == b->trace()->front());
  }

  // Invariants #2, #4
  assert(it != end && b->back().isBlockEnd());
  --end;
  for (DEBUG_ONLY IRInstruction& inst : folly::makeRange(it, end)) {
    assert(inst.op() != DefLabel);
    assert(inst.op() != BeginCatch);
    assert(!inst.isBlockEnd());
  }
  for (DEBUG_ONLY IRInstruction& inst : *b) {
    // Invariant #8
    assert(inst.marker().valid());
    assert(inst.block() == b);
    // Invariant #10
    assert_log(IMPLIES(inst.mayRaiseError() && b->trace()->isMain() &&
                       !inst.is(CoerceStk), // CoerceStk is special: t3213636
                       inst.taken() && inst.taken()->isCatch()),
               [&]{ return inst.toString(); });
  }

  // Invariant #5
  assert(IMPLIES(b->back().isTerminal(), !b->next()));

  // Invariant #7
  if (b->taken()) {
    // only Jmp can branch to a join block expecting values.
    DEBUG_ONLY IRInstruction* branch = &b->back();
    DEBUG_ONLY auto numArgs = branch->op() == Jmp ? branch->numSrcs() : 0;
    assert(numBlockParams(b->taken()) == numArgs);
  }

  // Invariant #6
  if (b->front().op() == DefLabel) {
    for (int i = 0; i < b->front().numDsts(); ++i) {
      DEBUG_ONLY auto const& traceBlocks = b->trace()->blocks();
      b->forEachSrc(i, [&](IRInstruction* inst, SSATmp*) {
        assert(std::find(traceBlocks.begin(), traceBlocks.end(),
                         inst->block()) != traceBlocks.end());
      });
    }
  }

  // Invariant #9
  if (b->isCatch()) {
    // keyed off a tca, so there needs to be exactly one
    assert(b->trace()->blocks().size() == 1);
    assert(b->preds().size() <= 1);
  }

  return true;
}
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
bool checkCfg(const IRUnit& unit) {
  // Check valid successor/predecessor edges.
  auto const blocks = rpoSortCfg(unit);
  std::unordered_set<const Edge*> edges;
  for (Block* b : blocks) {
    auto checkEdge = [&] (const Edge* e) {
      assert(e->inst()->block() == b);
      edges.insert(e);
      for (auto& p : e->to()->preds()) if (&p == e) return;
      assert(false); // did not find edge.
    };
    checkBlock(b);
    if (auto *e = b->nextEdge())  checkEdge(e);
    if (auto *e = b->takenEdge()) checkEdge(e);
  }
  for (Block* b : blocks) {
    for (DEBUG_ONLY auto const &e : b->preds()) {
      assert(&e == e.inst()->takenEdge() || &e == e.inst()->nextEdge());
      assert(e.to() == b);
    }
  }

  // visit dom tree in preorder, checking all tmps
  auto const children = findDomChildren(unit, blocks);
  StateVector<SSATmp, bool> defined0(unit, false);
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

bool checkTmpsSpanningCalls(const IRUnit& unit) {
  auto const blocks = rpoSortCfg(unit);
  auto const children = findDomChildren(unit, blocks);

  // CallBuiltin is ok because it is not a php-level call.  (It will
  // call a C++ helper and we can push/pop around it normally.)
  auto isCall = [&] (Opcode op) {
    return op == Call || op == CallArray;
  };

  typedef StateVector<SSATmp,bool> State;

  bool isValid = true;
  forPreorderDoms(
    blocks.front(), children, State(unit, false),
    [&] (Block* b, State& state) {
      for (auto& inst : *b) {
        for (auto& src : inst.srcs()) {
          /*
           * These SSATmp's are used only for stack analysis in the
           * simplifier and therefore may live across calls.  In particular
           * these instructions are used to bridge the logical stack of the
           * caller when a callee is inlined so that analysis does not scan
           * into the callee stack when searching for a type of value in the
           * caller.
           */
          if (inst.op() == ReDefSP && src->isA(Type::StkPtr)) continue;
          if (inst.op() == ReDefGeneratorSP && src->isA(Type::StkPtr)) {
            continue;
          }

          if (src->isA(Type::FramePtr)) continue;
          if (src->isConst()) continue;
          if (!state[src]) {
            auto msg = folly::format("checkTmpsSpanningCalls failed\n"
                                     "  instruction: {}\n"
                                     "  src:         {}\n",
                                     inst.toString(),
                                     src->toString()).str();
            std::cerr << msg;
            FTRACE(1, "{}", msg);
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

bool checkNoShuffles(const IRUnit& unit) {
  postorderWalk(unit, [] (Block* b) {
    for (DEBUG_ONLY auto& inst : *b) assert(inst.op() != Shuffle);
  });
  return true;
}

/*
 * Check that each destination register or spill slot is unique,
 * and that sources have the same number or less operands than
 * destinations.
 */
bool checkShuffle(const IRInstruction& inst, const RegAllocInfo& regs) {
  auto n = inst.numSrcs();
  assert(n == inst.extra<Shuffle>()->size);
  RegSet destRegs;
  std::bitset<NumPreAllocatedSpillLocs> destSlots;
  std::bitset<NumPreAllocatedSpillLocs> srcSlots;
  for (uint32_t i = 0; i < n; ++i) {
    DEBUG_ONLY auto& rs = regs[inst][inst.src(i)];
    DEBUG_ONLY auto& rd = inst.extra<Shuffle>()->dests[i];
    if (rd.numAllocated() == 0) continue; // dest was unused; ignore.
    // rs could have less assigned registers/slots than rd, in these cases:
    // - when rs is empty, because the source is a constant.
    // - when rd needs 2 and rs needs 0 or 1, and the source is either constant
    //   or hasKnownType() without being constant, and the dest type is more
    //   general than the src type due to a control-flow join.
    assert(rs.numAllocated() <= rd.numAllocated());
    assert(!rs.spilled() || !rd.spilled());
    assert(rs.isFullSIMD() == rd.isFullSIMD());
    for (int j = 0; j < rd.numAllocated(); ++j) {
      if (rd.spilled()) {
        assert(!destSlots.test(rd.slot(j)));
        destSlots.set(rd.slot(j));
      } else {
        assert(!destRegs.contains(rd.reg(j))); // no duplicate dests
        destRegs.add(rd.reg(j));
      }
    }
    // don't let any spill slot appear on both sides of the copy.
    assert((srcSlots & destSlots).none());
  }
  return true;
}

bool checkRegisters(const IRUnit& unit, const RegAllocInfo& regs) {
  assert(checkCfg(unit));
  auto blocks = rpoSortCfg(unit);
  StateVector<Block, RegState> states(unit, RegState());
  StateVector<Block, bool> reached(unit, false);
  for (auto* block : blocks) {
    RegState state = states[block];
    for (IRInstruction& inst : *block) {
      if (inst.op() == Jmp) continue; // handled by Shuffle
      for (SSATmp* src : inst.srcs()) {
        auto const &rs = regs[inst][src];
        if (!rs.spilled() &&
            ((arch() == Arch::X64 && (rs.reg(0) == X64::rVmSp ||
                                      rs.reg(0) == X64::rVmFp)) ||
             (arch() == Arch::ARM && (rs.reg(0) == ARM::rVmSp ||
                                      rs.reg(0) == ARM::rVmFp)))) {
          // hack - ignore rbx and rbp
          continue;
        }
        assert(rs.numWords() == src->numWords() ||
               (src->inst()->op() == DefConst && rs.numWords() == 0));
        DEBUG_ONLY auto allocated = rs.numAllocated();
        if (allocated == 2) {
          if (rs.spilled()) {
            assert(rs.slot(0) != rs.slot(1));
          } else {
            assert(rs.reg(0) != rs.reg(1));
          }
        }
        for (unsigned i = 0, n = rs.numAllocated(); i < n; ++i) {
          assert(state.tmp(rs, i) == src);
        }
      }
      auto update = [&](SSATmp* tmp, const PhysLoc& loc) {
        for (unsigned i = 0, n = loc.numAllocated(); i < n; ++i) {
          state.tmp(loc, i) = tmp;
        }
      };
      if (inst.op() == Shuffle) {
        checkShuffle(inst, regs);
        for (uint32_t i = 0; i < inst.numSrcs(); ++i) {
          update(inst.src(i), inst.extra<Shuffle>()->dests[i]);
        }
      } else {
        for (auto& d : inst.dsts()) {
          update(&d, regs[inst][d]);
        }
      }
    }
    // State contains the PhysLoc->SSATmp reverse mappings at block end;
    // propagate the state to succ
    auto updateEdge = [&](Block* succ) {
      if (!reached[succ]) {
        states[succ] = state;
      } else {
        states[succ].merge(state);
      }
    };
    if (auto* next = block->next()) updateEdge(next);
    if (auto* taken = block->taken()) updateEdge(taken);
  }

  return true;
}

}}
