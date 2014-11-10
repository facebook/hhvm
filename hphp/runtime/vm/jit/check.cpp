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

#include "hphp/runtime/vm/jit/check.h"

#include <bitset>
#include <iostream>
#include <unordered_set>

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

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
 * 3. If this block is a catch block, it must have at most one predecessor.
 * 4. The last instruction must be isBlockEnd() and the middle instructions
 *    must not be isBlockEnd().  Therefore, blocks cannot be empty.
 * 5. If the last instruction isTerminal(), block->next() must be null.
 * 6. Every instruction must have a catch block attached to it if and only if it
 *    has the MayRaiseError flag.
 * 7. Any path from this block to a Block that expects values must be
 *    from a Jmp instruciton.
 * 8. Every instruction's BCMarker must point to a valid bytecode instruction.
 * 9. If a DefLabel defines a value of type StkPtr, it must appear as the first
 *    dest. This is necessary for the state tracking in FrameState::update.
 */
bool checkBlock(Block* b) {
  auto it = b->begin();
  auto end = b->end();
  assert(!b->empty());

  // Invariant #1
  if (it->op() == DefLabel) {
    // Invariant #9
    for (unsigned i = 0, n = it->numDsts(); i < n; ++i) {
      assert(IMPLIES(it->dst(i)->isA(Type::StkPtr), i == 0));
    }
    ++it;
  }

  // Invariant #1
  if (it != end && it->op() == BeginCatch) {
    ++it;
  }

  // Invariants #2, #4
  assert(it != end && b->back().isBlockEnd());
  --end;
  for (DEBUG_ONLY IRInstruction& inst : folly::range(it, end)) {
    assert(inst.op() != DefLabel);
    assert(inst.op() != BeginCatch);
    assert(!inst.isBlockEnd());
  }
  for (DEBUG_ONLY IRInstruction& inst : *b) {
    // Invariant #8
    assert(inst.marker().valid());
    assert(inst.block() == b);
    // Invariant #6
    assert_log(inst.mayRaiseError() ==
               (inst.taken() && inst.taken()->isCatch()),
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

  // Invariant #3
  if (b->isCatch()) {
    // keyed off a tca, so there needs to be exactly one
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
 * 6. The entry block must not have any predecessors.
 * 7. The entry block starts with a DefFP instruction.
 */
bool checkCfg(const IRUnit& unit) {
  auto const blocksIds = rpoSortCfgWithIds(unit);
  auto const& blocks = blocksIds.blocks;
  jit::hash_set<const Edge*> edges;

  // Entry block can't have predecessors.
  assert(unit.entry()->numPreds() == 0);

  // Entry block starts with DefFP
  assert(!unit.entry()->empty() && unit.entry()->begin()->op() == DefFP);

  // Check valid successor/predecessor edges.
  for (Block* b : blocks) {
    auto checkEdge = [&] (const Edge* e) {
      assert(e->from() == b);
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
  auto const children = findDomChildren(unit, blocksIds);
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
  // CallBuiltin is ok because it is not a php-level call.  (It will
  // call a C++ helper and we can push/pop around it normally.)
  auto isCall = [&] (Opcode op) {
    return op == Call || op == CallArray || op == ContEnter;
  };

  auto ignoreSrc = [&](IRInstruction& inst, SSATmp* src) {
    /*
     * ReDefSP, TakeStack, and FramePtr/StkPtr-typed tmps are used
     * only for stack analysis in the simplifier and therefore may
     * live across calls. In particular, ReDefSP are used to bridge
     * the logical stack of the caller when a callee is inlined so
     * that analysis does not scan into the callee stack when
     * searching for a type of value in the caller.
     *
     * Tmps defined by DefConst are always available and may be
     * assigned to registers if needed by the instructions using the
     * const.
     */
    return (inst.is(ReDefSP) && src->isA(Type::StkPtr)) ||
           inst.is(TakeStack) ||
           src->isA(Type::StkPtr) ||
           src->isA(Type::FramePtr) ||
           src->inst()->is(DefConst);
  };

  StateVector<Block,IdSet<SSATmp>> livein(unit, IdSet<SSATmp>());
  bool isValid = true;
  postorderWalk(unit, [&](Block* block) {
    auto& live = livein[block];
    if (auto taken = block->taken()) live = livein[taken];
    if (auto next  = block->next()) live |= livein[next];
    for (auto it = block->end(); it != block->begin();) {
      auto& inst = *--it;
      for (auto& dst : inst.dsts()) {
        live.erase(dst);
      }
      if (isCall(inst.op())) {
        live.forEach([&](uint32_t tmp) {
          auto msg = folly::format("checkTmpsSpanningCalls failed\n"
                                   "  instruction: {}\n"
                                   "  src:         t{}\n",
                                   inst.toString(), tmp).str();
          std::cerr << msg;
          FTRACE(1, "{}", msg);
          isValid = false;
        });
      }
      for (auto* src : inst.srcs()) {
        if (!ignoreSrc(inst, src)) live.add(src);
      }
    }
  });

  return isValid;
}

}}
