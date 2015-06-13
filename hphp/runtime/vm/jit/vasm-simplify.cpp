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

#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {

namespace {

struct Simplifier {
  Vunit& unit;
  jit::vector<uint32_t> used;
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  explicit Simplifier(Vunit& unit_in, jit::vector<uint32_t>&& used_in)
  : unit(unit_in), used(std::move(used_in)) { }

  // If there is an adjacent setcc/xorbi pair where the xor is being used
  // to invert the result of the setcc, just invert the setcc condition and
  // omit the xor.
  bool match_setcc_xorbi(const Vblock& block,
                         size_t iIdx,
                         Vinstr& sinst,
                         Vreg& xdst) {
    auto const& code = block.code;
    if (code[iIdx].op == Vinstr::setcc) {
      auto const& inst = code[iIdx].setcc_;
      auto const dst = inst.d;
      if (used[inst.d] == 1 && // Make sure setcc result is only used by xor.
          iIdx + 1 < code.size() &&
          code[iIdx + 1].op == Vinstr::xorbi) {
        auto const& xor = code[iIdx+1].xorbi_;
        if (xor.s0.b() == 1 && xor.s1 == dst && used[xor.sf] == 0) {
          sinst = code[iIdx];  // setcc instruction being modified.
          xdst = xor.d;        // destination for xor result.
          return true;
        }
      }
    }
    return false;
  }

  // Perform any simplification steps for the instructions beginning
  // at the given index.  Add additional simplification matchers here.
  size_t simplify(Vout& v, const Vblock& block, size_t iIdx) {
    Vinstr inst;
    Vreg dst;
    if (match_setcc_xorbi(block, iIdx, inst, dst)) {
      // Rewrite setcc/xorbi pair as setcc with inverted condition.
      v << setcc{ccNegate(inst.setcc_.cc), inst.setcc_.sf, dst};
      return iIdx + 1;
    }
    return 0;
  }
};


/*
 * The main work routine for the simplifier.  It runs a platform specific
 * simplify method on each instruction in the code stream.  The simplify
 * method is allowed to look ahead in the instruction stream in order to
 * match more than a single instruction if needed.  The simplify routines
 * are responsible for checking for the end of the instruction buffer.  If
 * a simplify routine finds a match, it emits the new instructions into
 * the provided emitter and returns the number of instructions to be replaced
 * in the original buffer by the newly emitted instructions.  The work()
 * routine then replaces the old instructions and calls work() again on
 * modified stream in case there are more simplifications to be had.
 */
size_t work(Simplifier& s, Vlabel b, size_t iIdx) {
  if (iIdx >= s.unit.blocks[b].code.size()) return iIdx;

  auto scratch = s.unit.makeScratchBlock();
  SCOPE_EXIT { s.unit.freeScratchBlock(scratch); };
  Vout v(s.unit, scratch, s.unit.blocks[b].code[iIdx].origin);

  auto& blocks = s.unit.blocks;
  auto& code = blocks[b].code;

  size_t mIdx = s.simplify(v, blocks[b], iIdx);
  if (!blocks[scratch].code.empty()) {
    vector_splice(code, iIdx, mIdx - iIdx + 1, blocks[scratch].code);
    return work(s, b, iIdx);
  }
  return iIdx + 1;
}

}

/*
 * Perform a simplification pass for an entire unit.
 */
void simplify(Vunit& unit) {
  assertx(check(unit)); // especially, SSA
  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);

  // Use count for each register.  Used to keep peepholes from
  // firing if results are used beyond the scope of the instructions
  // being checked.
  jit::vector<uint32_t> used(unit.next_vr);
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg r) { ++used[r]; });
    }
  }

  Simplifier simplifier(unit, std::move(used));
  simplifier.vals.resize(unit.next_vr);
  simplifier.valid.resize(unit.next_vr);
  // figure out which Vregs are constants and stash their values.
  for (auto& entry : unit.constToReg) {
    simplifier.valid.set(entry.second);
    simplifier.vals[entry.second] = entry.first.val;
  }

  // now mutate instructions
  for (auto b : blocks) {
    size_t iIdx = 0;
    while (iIdx < unit.blocks[b].code.size()) {
      iIdx = work(simplifier, b, iIdx);
    }
  }
  printUnit(kVasmSimplifyLevel, "after vasm simplify", unit);
}

}
}
