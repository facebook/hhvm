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
#include "hphp/runtime/vm/jit/opt.h"

#include <iterator>

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Insert `inst' after `definer'.
 *
 * If `definer' ends its block, it must have a fallthrough block, and `inst'
 * will be inserted at the beginning of that block, as long as the block has no
 * other predecessors.
 *
 * Returns: true if it inserted the intruction.
 */
bool insertAfter(IRInstruction* definer, IRInstruction* inst) {
  assertx(!definer->isTerminal());
  if (definer->isControlFlow()) {
    assertx(definer->next());
    if (definer->next()->numPreds() == 1) {
      definer->next()->prepend(inst);
      return true;
    }
    return false;
  }

  auto const block = definer->block();
  auto const pos = block->iteratorTo(definer);
  block->insert(std::next(pos), inst);
  return true;
}

/*
 * Insert a DbgAssertRefCount instruction after each place we define a
 * maybe-refcounted SSATmp.
 */
void insertRefCountAsserts(IRUnit& unit, IRInstruction& inst) {
  for (auto dst : inst.dsts()) {
    auto const t = dst->type();
    if (t <= TGen && t.maybe(TCounted)) {
      insertAfter(&inst, unit.gen(DbgAssertRefCount, inst.marker(), dst));
    }
  }
}

void insertStkAssert(IRUnit& unit,
                     IRInstruction* where,
                     SSATmp* sp,
                     IRSPOffset off) {
  auto const addr = unit.gen(
    LdStkAddr,
    where->marker(),
    TPtrToStkGen,
    IRSPOffsetData { off },
    sp
  );
  if (!insertAfter(where, addr)) return;
  auto const check = unit.gen(DbgAssertPtr, where->marker(), addr->dst());
  insertAfter(addr, check);
}

//////////////////////////////////////////////////////////////////////

void visit(IRUnit& unit, Block* block) {
  for (auto it = block->begin(); it != block->end();) {
    auto& inst = *it;
    ++it;

    switch (inst.op()) {
    case Call:
      {
        auto const extra = inst.extra<Call>();
        insertStkAssert(unit, &inst, inst.src(0),
          extra->spOffset + extra->numParams + kNumActRecCells - 1);
      }
      break;
    default:
      insertRefCountAsserts(unit, inst);
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

void insertAsserts(IRUnit& unit) {
  // Note: it doesn't matter what order we visit the blocks for this pass.
  for (auto& block : poSortCfg(unit)) visit(unit, block);
}

//////////////////////////////////////////////////////////////////////

}}
