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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

// if inst is testb{r,r,d}, return true,d
bool match_testb(Vinstr& inst, Vreg r) {
  return inst.op == Vinstr::testb &&
         inst.testb_.s0 == r &&
         inst.testb_.s1 == r;
}

bool match_jcc(Vinstr& inst, Vreg flags) {
  return inst.op == Vinstr::jcc && inst.jcc_.sf == flags &&
         (inst.jcc_.cc == CC_E || inst.jcc_.cc == CC_NE);
}

#define CMOV_MATCH(type)                                                \
  bool match_##type(Vinstr& inst, Vreg flags) {                         \
    return inst.op == Vinstr::type && inst.type##_.sf == flags &&       \
      (inst.type##_.cc == CC_E || inst.type##_.cc == CC_NE);            \
  }
CMOV_MATCH(cmovb)
CMOV_MATCH(cmovw)
CMOV_MATCH(cmovl)
CMOV_MATCH(cmovq)
#undef CMOV_MATCH

bool sets_flags(const Vunit& unit, const Vinstr& inst) {
  // Some special cases that also clobber flags:
  switch (inst.op) {
  case Vinstr::vcall:
  case Vinstr::vinvoke:
  case Vinstr::call:
  case Vinstr::callm:
  case Vinstr::callr:
  case Vinstr::calls:
  case Vinstr::callstub:
  case Vinstr::callfaststub:
  case Vinstr::callphp:
  case Vinstr::callarray:
  case Vinstr::contenter:
    return true;
  default:
    break;
  }

  Vreg flags;
  visitDefs(unit, inst, [&] (Vreg r, Width w) {
    if (w == Width::Flags) flags = r;
  });
  return flags.isValid();
}
}

/*
 * Branch fusion:
 * Analyze blocks one at a time, looking for the sequence:
 *
 *   setcc cc, f1 => b
 *   ...
 *   testb b, b => f2
 *   ...
 *   jcc E|NE, f2 (OR cmov E|NE, f, t, d)
 *
 * If found, and f2 is only used by the jcc, then change the code to:
 *
 *   setcc cc, f1 => b
 *   ...
 *   nop
 *   ...
 *   jcc !cc|cc, f1 (OR cmov !cc|cc, f, t, d)
 *
 * Later, vasm-dead will clean up the nop, and the setcc if b became dead.
 *
 * During the search, any other instruction that has a status flag result
 * will reset the pattern matcher. No instruction can "kill" flags,
 * since flags are SSA variables. However the transformation we want to
 * make extends the setcc flags lifetime, and we don't want it to overlap
 * another flag's lifetime.
 */
void fuseBranches(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  jit::vector<unsigned> uses(unit.next_vr);
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&] (Vreg r) { uses[r]++; });
    }
  }
  bool should_print = false;
  for (auto b : blocks) {
    auto& code = unit.blocks[b].code;
    ConditionCode cc;
    Vreg setcc_flags, setcc_dest, testb_flags;
    unsigned testb_index;
    for (unsigned i = 0, n = code.size(); i < n; ++i) {
      if (code[i].op == Vinstr::setcc) {
        cc = code[i].setcc_.cc;
        setcc_flags = code[i].setcc_.sf;
        setcc_dest = code[i].setcc_.d;
        continue;
      }
      if (setcc_flags.isValid() &&
          match_testb(code[i], setcc_dest) &&
          uses[code[i].testb_.sf] == 1) {
        testb_flags = code[i].testb_.sf;
        testb_index = i;
        continue;
      }
      if (match_jcc(code[i], testb_flags)) {
        code[testb_index] = nop{}; // erase the testb
        auto& jcc = code[i].jcc_;
        jcc.cc = jcc.cc == CC_NE ? cc : ccNegate(cc);
        jcc.sf = setcc_flags;
        should_print = true;
        continue;
      }

      // Check for different cmov flavors
#define CMOV_IMPL(type)                                 \
      if (match_##type(code[i], testb_flags)) {         \
        code[testb_index] = nop{};                      \
        auto& cmov = code[i].type##_;                   \
        cmov.cc = cmov.cc == CC_NE ? cc : ccNegate(cc); \
        cmov.sf = setcc_flags;                          \
        should_print = true;                            \
        continue;                                       \
      }
      CMOV_IMPL(cmovb);
      CMOV_IMPL(cmovw);
      CMOV_IMPL(cmovl);
      CMOV_IMPL(cmovq);
#undef CMOV_IMPL

      if (setcc_flags.isValid() && sets_flags(unit, code[i])) {
        setcc_flags = testb_flags = Vreg{};
      }
    }
  }
  if (should_print) {
    printUnit(kVasmFusionLevel, "after vasm-fusion", unit);
  }
}

}}
