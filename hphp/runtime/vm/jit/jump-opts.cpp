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

#include <iterator>
#include <utility>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

// Return true if this block ends with a trivial Jmp: a Jmp
// whose target's only predecessor is b.
bool isTrivialJmp(IRInstruction* branch, Block* taken) {
  return branch->op() == Jmp && taken->numPreds() == 1;
}

// Coalesce two blocks joined by a trivial jump by moving the second block's
// instructions to the first block and deleting the jump.  If the second block
// starts with BeginCatch or DefLabel, they will also be deleted.
void eliminateJmp(Block* lastBlock, IRInstruction* jmp, Block* target,
                  IRUnit& unit) {
  assert(isTrivialJmp(jmp, target));
  auto lastInst = lastBlock->iteratorTo(jmp); // iterator to last instruction
  if (jmp->numSrcs() != 0) {
    auto& defLabel = target->front();
    assert(defLabel.numDsts() == jmp->numSrcs());
    for (auto i = 0; i < jmp->numSrcs(); i++) {
      lastBlock->insert(lastInst,
                        unit.genWithDst(defLabel.dst(i), Mov,
                                        jmp->marker(), jmp->src(i)));
    }
  }
  lastInst = lastBlock->iteratorTo(jmp); // iterator to last instruction
  lastBlock->splice(lastInst, target, target->skipHeader(), target->end());
  jmp->setTaken(nullptr); // unlink edge
  lastBlock->erase(lastInst); // delete the jmp
}

}

//////////////////////////////////////////////////////////////////////

void optimizeJumps(IRUnit& unit) {
  Timer _t(Timer::optimize_jumpOpts);

  postorderWalk(unit, [&](Block* b) {
    auto branch = &b->back();
    auto taken = branch->taken();
    if (isTrivialJmp(branch, taken)) {
      eliminateJmp(b, branch, taken, unit);
    }
  });
}

}}
