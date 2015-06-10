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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

jit::vector<uint32_t> count_predecessors(Vunit& unit) {
  auto ret = jit::vector<uint32_t>(unit.blocks.size(), 0);
  PostorderWalker{unit}.dfs([&] (Vlabel b) {
    for (auto& s : succs(unit.blocks[b])) {
      ++ret[s];
    }
  });
  return ret;
}

/*
 * Return true if block b matches one of these patterns:
 *
 *   bindjmp       syncvmsp r    lea [r + d] => t
 *                 bindjmp       syncvmsp t
 *                               bindjmp
 *
 * If we find a match, return a suitable Vptr describing
 * how r (the stack pointer) was adjusted.
 */
bool match_bindjmp(const Vunit& unit, Vlabel b, Vptr* addr) {
  const auto& code = unit.blocks[b].code;
  if (code[0].op == Vinstr::bindjmp) {
    *addr = Vptr{}; // invalid vptr means no syncvmsp was matched.
    return true;
  }
  if (code[0].op == Vinstr::syncvmsp &&
      code[1].op == Vinstr::bindjmp) {
    const auto& isync = code[0].syncvmsp_;
    *addr = isync.s[0]; // sync takes ptr register, return [ptr+0]
    return true;
  }
  if (code[0].op == Vinstr::lea &&
      code[1].op == Vinstr::syncvmsp &&
      code[2].op == Vinstr::bindjmp) {
    const auto& ilea = code[0].lea_;
    const auto& isync = code[1].syncvmsp_;
    *addr = ilea.s; // lea takes [ptr+offset], return the whole Vptr
    return isync.s == ilea.d; // ... assuming syncvmsp uses lea.d
  }
  return false;
}

/*
 * determine whether block t0 and t1 both end with bindjmp and are compatible
 * enough to fold jcc{cc, f, {t0, t1}} => bindjcc1st{cc,f}. t0 and t1 must
 * pass the match_bindjmp test, have compatible stack pointer adjustments,
 * and be for the same function.
 */
bool match_bindjcc1st(const Vunit& unit, Vlabel t0, Vlabel t1) {
  Vptr addr0, addr1; // adjusted stk addresses (sp+offset on each path)
  if (match_bindjmp(unit, t0, &addr0) &&
      match_bindjmp(unit, t1, &addr1)) {
    const auto& bj0 = unit.blocks[t0].code.back().bindjmp_;
    const auto& bj1 = unit.blocks[t1].code.back().bindjmp_;
    return addr0 == addr1 &&
           bj0.target.resumed() == bj1.target.resumed() &&
           bj0.target.funcID() == bj1.target.funcID() &&
           bj0.spOff == bj1.spOff;
  }
  return false;
}

}

/*
 * optimizeExits does two conversions to eliminate common branch-to-exit flows.
 *
 * 1. If we see a jcc that leads to two "identical" blocks ending with
 * bindjmp, then copy the identical part of the targets before the jcc,
 * and replace the jcc with a bindjcc1st instruction using the bytecode
 * destinations from the two original bindjmps. For the sake of this pass,
 * "identical" means matching lea & syncvmsp instructions, and both bindjmp's
 * are for the same function.
 *
 * This leads to more efficient code because the service request stubs will
 * patch jumps in the main trace instead of off-trace.
 *
 * 2. Otherwise, if we see a jcc but only one of the branches is
 * a normal exit, then convert the jcc to a bindexit with the jcc's condition
 * and the original bindjmp's dest.
 */
void optimizeExits(Vunit& unit) {
  auto const pred_counts = count_predecessors(unit);

  PostorderWalker{unit}.dfs([&](Vlabel b) {
    auto& code = unit.blocks[b].code;
    assertx(!code.empty());
    if (code.back().op != Vinstr::jcc) return;

    auto const ijcc = code.back().jcc_;
    auto const t0 = ijcc.targets[0];
    auto const t1 = ijcc.targets[1];
    if (t0 == t1) {
      code.back() = jmp{t0};
      return;
    }
    if (pred_counts[t0] != 1 || pred_counts[t1] != 1) return;

    // copy all but the last instruction in blocks[t] to just before
    // the last instruction in code.
    auto hoist_sync = [&](Vlabel t) {
      const auto& tcode = unit.blocks[t].code;
      code.insert(std::prev(code.end()),
                  tcode.begin(), std::prev(tcode.end()));
    };

    if (match_bindjcc1st(unit, t0, t1)) {
      // hoist the sync instructions from t0 to before the jcc,
      // and replace the jcc with bindjcc1st.
      const auto& bj0 = unit.blocks[t0].code.back().bindjmp_;
      const auto& bj1 = unit.blocks[t1].code.back().bindjmp_;
      hoist_sync(t0);
      code.back() = bindjcc1st{ijcc.cc, ijcc.sf, {{bj0.target, bj1.target}},
                               bj0.spOff,
                               bj0.args | bj1.args};
      return;
    }

    auto fold_exit = [&](ConditionCode cc, Vlabel exit, Vlabel next) {
      const auto& bj = unit.blocks[exit].code.back().bindjmp_;
      auto origin = code.back().origin;
      hoist_sync(exit);
      code.back() = bindjcc{cc, ijcc.sf, bj.target, bj.spOff,
                            bj.trflags, bj.args};
      code.emplace_back(jmp{next});
      code.back().origin = origin;
    };

    // Try to replace jcc to normal exit with bindexit followed by jmp,
    // as long as the sp adjustment is harmless to hoist (disp==0)
    Vptr sp;
    if (match_bindjmp(unit, t1, &sp) && sp == sp.base[0]) {
      fold_exit(ijcc.cc, t1, t0);
    } else if (match_bindjmp(unit, t0, &sp) && sp == sp.base[0]) {
      fold_exit(ccNegate(ijcc.cc), t0, t1);
    }
  });
  printUnit(kVasmExitsLevel, "after vasm-exits", unit);
}

} }
