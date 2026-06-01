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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/vixl/hphp-compat.h"

namespace HPHP::jit {

// Immediate-folding. If an instruction takes a register operand defined
// as a constant, and there is valid immediate-form of that instruction,
// then change the instruction and embed the immediate.
template<typename Folder>
void foldImms(Vunit& unit) {
  assertx(check(unit)); // especially, SSA
  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);

  // Use flag for each registers.  If a SR is uses then
  // certain optimizations will not fire since they do not
  // set the condition codes as the original instruction(s)
  // would.
  jit::vector<uint8_t> uses(unit.next_vr);
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg r) {
          auto& u = uses[r];
          if (u != 255) ++u;
        });
    }
  }

  Folder folder(unit, std::move(uses));
  folder.vals.resize(unit.next_vr);
  folder.valid.resize(unit.next_vr);
  // figure out which Vregs are constants and stash their values.
  for (auto& entry : unit.constToReg) {
    folder.valid.set(entry.second);
    folder.vals[entry.second] = entry.first.val;
  }
  // now mutate instructions
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
#define O(name, imms, uses, defs)           \
        case Vinstr::name: {                \
          auto const irctx = inst.irctx();  \
          folder.fold(inst.name##_, inst);  \
          inst.set_irctx(irctx);            \
          break;                            \
        }
        VASM_OPCODES
#undef O
      }
    }
  }
  printUnit(kVasmImmsLevel, "after foldImms", unit);
}

}
