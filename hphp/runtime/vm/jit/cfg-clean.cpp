/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_cfg);

namespace {

bool absorbDstBlock(IRUnit& unit, Block* block) {
  auto& term = block->back();
  if (!term.is(Jmp)) return false;

  auto dstBlk = block->taken();
  if (dstBlk == block) return false;
  if (dstBlk->numPreds() != 1) return false;

  // Replace DefLabel with Mov's
  if (dstBlk->begin()->is(DefLabel)) {
    auto& defLabel = *dstBlk->begin();
    for (auto i = 0; i < defLabel.numDsts(); ++i) {
      auto mov = unit.gen(Mov, defLabel.marker(), term.src(i));
      mov->setDst(defLabel.dst(i));
      mov->dst()->setInstruction(mov);
      block->push_back(mov);
    }
    defLabel.convertToNop();
  }

  term.convertToNop();
  block->splice(block->end(), dstBlk, dstBlk->skipHeader(), dstBlk->end());
  // dstBlk is in a zombie now, will be removed later.
  FTRACE(1, "Merged B{} into B{}\n", dstBlk->id(), block->id());
  return true;
}

bool foldJmp(IRUnit& unit, Block* block) {
  auto& term = block->back();

  if (term.is(Jmp)) {
    auto dstBlk = block->taken();
    if (dstBlk == block) return false;
    // If we reach here, dstBlk cannot be merged into block as it has other
    // predecessors. We could duplicate whatever code in dstBlk in block, but we
    // only do it if dstBlk contains only a cheap instruction.
    if (dstBlk->begin()->is(Jmp, JmpZero, JmpNZero, CheckInit)) {
      block->back().become(unit, &*dstBlk->begin());
      FTRACE(1, "Duplicated B{} into B{}\n", dstBlk->id(), block->id());
      // dstBlk is probably still reachable from other blocks.
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
 * predecessor of its successor;
 * (3) fold Jmp, if it fits the Jmp to Jmp pattern.
 *
 * The reverse post order is not essential to the transformation; in the current
 * implementation it helps skipping some blocks after a change happens.
 */
void cleanCfg(IRUnit& unit) {
  PassTracer tracer { &unit, Trace::hhir_cfg, "cleanCfg" };
  Timer timer(Timer::optimize_cleancfg);
  do {
    auto const blocks = rpoSortCfg(unit);
    for (auto block : blocks) {
      // Skip malformed unreachable blocks that can appear transiently.
      if (block->empty()) continue;

      // keep working on the current block until no further changes are made.
      // Since we are visiting in reverse post order, we are sure that after a
      // block is changed here, no more opportunity is exposed in its upstream
      // blocks.
      while (true) {
        simplify(unit, &(block->back()));
        if (absorbDstBlock(unit, block)) continue;
        if (foldJmp(unit, block)) continue;
        break;
      }
    }
  } while (removeUnreachable(unit));
}

}}
