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
#include <boost/dynamic_bitset.hpp>
#include <algorithm>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {
using namespace x64;

namespace x64 {

PredVector computePreds(Vunit& unit) {
  PredVector preds(unit.blocks.size());
  PostorderWalker walker(unit);
  walker.dfs([&](Vlabel b) {
    for (auto s: succs(unit.blocks[b])) {
      preds[s].push_back(b);
    }
  });
  return preds;
}

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
    // give roots an extra predecessor to prevent cloning them.
    for (auto b : unit.roots) {
      npreds[b]++;
    }
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
    printUnit("after vasm-jumps", unit);
  }
}

}}
