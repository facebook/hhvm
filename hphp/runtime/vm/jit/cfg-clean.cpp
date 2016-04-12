/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_cfg);

namespace {

// Different from simplify(), this works even when `block' is unreachable.
// Returns true iff changes are made.
bool convertCondBranchToJmp(IRUnit& unit, Block* block) {
  auto takenBlk = block->taken();
  if (takenBlk == nullptr) return false;
  auto nextBlk = block->next();
  if (nextBlk == nullptr) return false;

  auto& term = block->back();
  // Only look at some conditional branches with no side effects.
  if (!term.is(JmpZero,
               JmpNZero,
               CheckTypeMem,
               CheckLoc,
               CheckStk,
               CheckInit,
               CheckInitMem,
               CheckInitProps,
               CheckInitSProps,
               CheckStaticLocInit,
               CheckRefInner,
               CheckCtxThis)) {
    return false;
  }

  // Case 1: both ways lead to the same block.
  bool isUnconditional = (nextBlk == takenBlk);

  // Case 2: branch constantly goes one way.
  if (!isUnconditional && term.numSrcs() == 1) {
    auto const src = term.src(0);
    if (src->hasConstVal() && term.is(JmpZero, JmpNZero)) {
      auto const v = src->isA(TBool) ? src->boolVal()
                                     : static_cast<bool>(src->intVal());
      if (v == term.is(JmpZero)) {
        // Branch never taken.
        takenBlk = block->next();
      }
      isUnconditional = true;
    } else if (term.is(CheckInit)) {
      if (!src->type().maybe(TUninit)) {
        // Never taken.
        takenBlk = block->next();
        isUnconditional = true;
      } else if (src->isA(TUninit)) {
        // Always taken.
        isUnconditional = true;
      }
    }
  }

  if (isUnconditional) {
    assert(takenBlk);
    auto const marker = term.marker();
    term.convertToNop();                // Removes edges to original dests.
    block->push_back(unit.gen(Jmp, marker, takenBlk));
    FTRACE(1, "Replaced conditional branch in B{} with Jmp to B{}.\n",
           block->id(), takenBlk->id());
    return true;
  }
  return false;
}

// If `block' only goes to `takenBlk' and `takenBlk' can only be reached
// directly from `block', merge them and leave `takenBlk' unreachable.
// Returns true iff merging happend.
bool absorbDstBlock(IRUnit& unit, Block* block) {
  auto& term = block->back();
  if (!term.is(Jmp)) return false;

  auto takenBlk = block->taken();
  if (takenBlk == block) return false;
  if (takenBlk->numPreds() != 1) return false;

  // Replace DefLabel with Mov's
  if (takenBlk->begin()->is(DefLabel)) {
    auto& defLabel = *takenBlk->begin();
    for (auto i = 0; i < defLabel.numDsts(); ++i) {
      auto mov = unit.gen(Mov, defLabel.marker(), term.src(i));
      mov->setDst(defLabel.dst(i));
      mov->dst()->setInstruction(mov);
      block->push_back(mov);
    }
    defLabel.convertToNop();
  }

  term.convertToNop();
  block->splice(block->end(), takenBlk, takenBlk->skipHeader(),
                takenBlk->end());

  // takenBlk is in a zombie now, it will be removed later.
  assertx(takenBlk->numPreds() == 0);
  takenBlk->instrs().clear();
  FTRACE(1, "Merged B{} into B{}\n", takenBlk->id(), block->id());
  return true;
}

// Jmp->(Jmp2|Jcc)->...   ==> (Jmp2|Jcc)->...
// Jcc->(non-phi)Jmp->... ==> Jcc->...
// Returns true iff changes were made.
bool foldJmp(IRUnit& unit, Block* block) {
  auto& term = block->back();

  if (term.is(Jmp)) {
    auto takenBlk = block->taken();
    if (takenBlk == block) return false;
    // If we reach here, takenBlk cannot be merged into block as it has other
    // predecessors. We could duplicate whatever code in takenBlk in block, but
    // we only do it if takenBlk contains only a cheap instruction.
    if (takenBlk->begin()->is(Jmp, JmpZero, JmpNZero, CheckInit)) {
      block->back().become(unit, &*takenBlk->begin());
      FTRACE(1, "Duplicated B{} into B{}\n", takenBlk->id(), block->id());
      // takenBlk is probably still reachable from other blocks.
      return true;
    }
    return false;
  }

  if (auto next = term.next()) {
    auto jmp = next->begin();
    if (jmp->is(Jmp) && jmp->numSrcs() == 0) {
      assertx(jmp->taken());
      FTRACE(1, "Setting {} next to skip {}\n", term, *jmp);
      term.setNext(jmp->taken());
      return true;
    }
  }

  if (auto taken = term.taken()) {
    auto jmp = taken->begin();
    if (jmp->is(Jmp) && jmp->numSrcs() == 0) {
      assertx(jmp->taken());
      FTRACE(1, "Setting {} taken to skip {}\n", term, *jmp);
      term.setTaken(jmp->taken());
      return true;
    }
  }

  return false;
}

}

/*
 * This pass tries to merge blocks and cleanup the CFG.
 *
 * In each pass, it visits blocks in reverse post order and tries to
 * (1) convert the conditional branch at the end of the block into a Jmp;
 * (2) merge the block with its unique successor block, if it is the unique
 *     predecessor of its successor;
 * (3) fold Jmps, if it fits the Jmp -> Jmp|Jcc or Jcc -> Jmp pattern.
 *
 * The reverse post order is not essential to the transformation; in the current
 * implementation it helps skipping some blocks after a change happens.
 */
void cleanCfg(IRUnit& unit) {
  PassTracer tracer { &unit, Trace::hhir_cfg, "cleanCfg" };
  Timer timer(Timer::optimize_cleancfg);
  bool changed = false;
  do {
    auto const blocks = rpoSortCfg(unit);
    for (auto block : blocks) {
      // Skip malformed unreachable blocks that can appear transiently.
      if (block->empty()) continue;
      if (block->numPreds() == 0 && block != unit.entry()) continue;

      // Keep working on the current block until no further changes are made.
      for ( ; ; changed = true) {
        if (convertCondBranchToJmp(unit, block)) continue;
        if (absorbDstBlock(unit, block)) continue;
        if (foldJmp(unit, block)) continue;
        break;
      }
    }
  } while (removeUnreachable(unit));

  // If any block is removed, reflow all types.
  if (changed) reflowTypes(unit);
}

}}
