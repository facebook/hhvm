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

#include "hphp/runtime/vm/jit/code-gen.h"

#include <iterator>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/layout.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

/*
 * Compute and save registers that are live *across* each inst, not including
 * registers whose lifetimes end at inst, nor registers defined by inst.
 */
LiveRegs computeLiveRegs(const IRUnit& unit, const RegAllocInfo& regs) {
  StateVector<Block, RegSet> live_in(unit, RegSet());
  LiveRegs live_regs(unit, RegSet());
  for (bool changed = true; changed;) {
    changed = false;
    postorderWalk(unit,
      [&](Block* block) {
        RegSet live;
        if (Block* taken = block->taken()) live = live_in[taken];
        if (Block* next = block->next()) live |= live_in[next];
        for (auto it = block->end(); it != block->begin(); ) {
          IRInstruction& inst = *--it;
          live -= regs.dstRegs(inst);
          live_regs[inst] = live;
          live |= regs.srcRegs(inst);
        }
        changed |= (live != live_in[block]);
        live_in[block] = live;
      });
  }
  return live_regs;
}

void genCode(IRUnit& unit) {
  if (dumpIREnabled()) {
    AsmInfo ai(unit);
    mcg->backEnd().genCodeImpl(unit, &ai);
  } else {
    mcg->backEnd().genCodeImpl(unit, nullptr);
  }
}

}}
