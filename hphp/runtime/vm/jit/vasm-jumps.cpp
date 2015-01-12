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
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

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
           bj0.target.getFuncId() == bj1.target.getFuncId();
  }
  return false;
}

}

/*
 * optimizeExits does two conversions to eliminate common branch-to-exit flows.
 *
 * 1. If we see a jcc that leads to two "idential" blocks ending with
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
  PostorderWalker{unit}.dfs([&](Vlabel b) {
    auto& code = unit.blocks[b].code;
    assert(!code.empty());
    if (code.back().op != Vinstr::jcc) return;

    auto ijcc = code.back().jcc_;
    auto t0 = ijcc.targets[0], t1 = ijcc.targets[1];
    if (t0 == t1) {
      code.back() = jmp{t0};
      return;
    }

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
      code.back() = bindjcc1st{ijcc.cc, ijcc.sf, {bj0.target, bj1.target},
                               bj0.args | bj1.args};
      return;
    }

    auto fold_exit = [&](ConditionCode cc, Vlabel exit, Vlabel next) {
      const auto& bj = unit.blocks[exit].code.back().bindjmp_;
      auto origin = code.back().origin;
      hoist_sync(exit);
      code.back() = bindjcc{cc, ijcc.sf, bj.target, bj.trflags, bj.args};
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
}

void optimizeJmps(Vunit& unit) {
  auto isEmpty = [&](Vlabel b, Vinstr::Opcode op) {
    auto& code = unit.blocks[b].code;
    return code.size() == 1 && op == code[0].op;
  };
  bool changed = false;
  bool ever_changed = false;
  jit::vector<int> npreds(unit.blocks.size(), 0);
  do {
    if (changed) {
      fill(npreds.begin(), npreds.end(), 0);
    }
    changed = false;
    PostorderWalker{unit}.dfs([&](Vlabel b) {
      for (auto s : succs(unit.blocks[b])) {
        npreds[s]++;
      }
    });
    // give entry an extra predecessor to prevent cloning it
    npreds[unit.entry]++;
    PostorderWalker{unit}.dfs([&](Vlabel b) {
      auto& block = unit.blocks[b];
      auto& code = block.code;
      assert(!code.empty());
      if (code.back().op == Vinstr::jcc) {
        auto ss = succs(block);
        if (ss[0] == ss[1]) {
          // both edges have same target, change to jmp
          code.back() = jmp{ss[0]};
          changed = true;
        } else {
          auto jcc_i = code.back().jcc_;
          if (isEmpty(jcc_i.targets[0], Vinstr::fallback)) {
            jcc_i = jcc{ccNegate(jcc_i.cc), jcc_i.sf,
                        {jcc_i.targets[1], jcc_i.targets[0]}};
          }
          if (isEmpty(jcc_i.targets[1], Vinstr::fallback)) {
            // replace jcc with fallbackcc and jmp
            const auto& fb_i = unit.blocks[jcc_i.targets[1]].code[0].fallback_;
            const auto t0 = jcc_i.targets[0];
            const auto jcc_origin = code.back().origin;
            code.pop_back();
            code.emplace_back(
              fallbackcc{jcc_i.cc, jcc_i.sf, fb_i.dest, fb_i.trflags});
            code.back().origin = jcc_origin;
            code.emplace_back(jmp{t0});
            code.back().origin = jcc_origin;
            changed = true;
          }
        }
      }
      if (code.back().op == Vinstr::jmp) {
        auto& s = code.back().jmp_.target;
        if (isEmpty(s, Vinstr::jmp)) {
          // skip over s
          s = unit.blocks[s].code.back().jmp_.target;
          changed = true;
        } else if (npreds[s] == 1 || isEmpty(s, Vinstr::jcc)) {
          // overwrite jmp with copy of s
          auto& code2 = unit.blocks[s].code;
          code.pop_back();
          code.insert(code.end(), code2.begin(), code2.end());
          changed = true;
        }
      } else {
        for (auto& s : succs(block)) {
          if (isEmpty(s, Vinstr::jmp)) {
            // skip over s
            s = unit.blocks[s].code.back().jmp_.target;
            changed = true;
          }
        }
      }
    });
    ever_changed |= changed;
  } while (changed);
  if (ever_changed) {
    printUnit(kVasmJumpsLevel, "after vasm-jumps", unit);
  }
}

}}
