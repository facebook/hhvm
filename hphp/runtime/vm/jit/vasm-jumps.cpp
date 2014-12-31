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

#include "hphp/runtime/vm/jit/vasm-x64.h"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>

#include "hphp/runtime/vm/jit/vasm-print.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

PredVector computePreds(const Vunit& unit) {
  PredVector preds(unit.blocks.size());
  PostorderWalker walker(unit);
  walker.dfs([&](Vlabel b) {
    for (auto s: succs(unit.blocks[b])) {
      preds[s].push_back(b);
    }
  });
  return preds;
}

// true if inst is a 2-way conditional branch
bool isBranch(const Vinstr& inst) {
  return inst.op == Vinstr::jcc ||
         inst.op == Vinstr::tbcc ||
         inst.op == Vinstr::cbcc;
}

void optimizeExits(Vunit& unit) {
  auto isNormalExit = [&](Vlabel b) {
    auto& code = unit.blocks[b].code;
    if (code.size() != 2) return false;
    if (code[0].op != Vinstr::syncvmsp) return false;
    if (code[1].op != Vinstr::bindjmp) return false;
    return true;
  };

  PostorderWalker{unit}.dfs([&](Vlabel b) {
    auto& block = unit.blocks[b];
    auto& code = block.code;
    assert(!code.empty());
    auto ss = succs(block);
    if (code.back().op == Vinstr::jcc) {
      if (ss[0] == ss[1]) return;
      auto jcc_i = code.back().jcc_;
      // Normalize branches so that targets[1] is the exit path.
      if (isNormalExit(jcc_i.targets[0])) {
        jcc_i = jcc{ccNegate(jcc_i.cc), jcc_i.sf,
          {jcc_i.targets[1], jcc_i.targets[0]}};
      }
      if (!isNormalExit(jcc_i.targets[1])) return;

      // Replace jcc to normal exit with bindexit followed by jmp.
      const auto& sync = unit.blocks[jcc_i.targets[1]].code[0].syncvmsp_;
      const auto& bj = unit.blocks[jcc_i.targets[1]].code[1].bindjmp_;
      const auto t0 = jcc_i.targets[0];
      const auto jcc_origin = code.back().origin;
      code.back() = sync;
      code.emplace_back(bindexit{jcc_i.cc, jcc_i.sf,
        bj.target, bj.trflags, bj.args});
      code.back().origin = jcc_origin;
      code.emplace_back(jmp{t0});
      code.back().origin = jcc_origin;
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
