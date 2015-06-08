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

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

/*
 * This pass hoists fallbackccs and the compares feeding them above
 * stores that may appear before them.  It's only applied to the first
 * block in the unit.  The pattern matched here is something like:
 *
 *    store
 *    ...
 *    store
 *    cmpbim | testbim
 *    fallbackcc
 *    ...
 *    cmpbim | testbim
 *    fallbackcc
 *
 * This pass transforms such code sequence by moving all those stores
 * below the last fallbackcc.  This transformation is safe because a
 * fallbackcc is used when no VM-visible state changes (if it's taken,
 * it means that no progress was made). And we know that the compares
 * that feed the fallbackccs are reading the VM state (locals and
 * stacks) that can't possibly be written by any store above it in
 * this same block.
 */
void hoistFallbackccs(Vunit& unit) {
  if (!RuntimeOption::EvalJitHoistFallbackccs) return;

  auto& block = unit.blocks[unit.entry];
  auto& code = block.code;

  // Find the last fallbackcc instruction in block, and the comparison
  // feeding the first fallbackcc.
  size_t iLastFbcc = code.size();
  size_t iFirstCmp = code.size();
  for (auto i = code.size(); i--; ) {
    if (code[i].op == Vinstr::fallbackcc) {
      if (iLastFbcc == code.size()) iLastFbcc = i;
      assertx(i > 0);
      if (code[i - 1].op == Vinstr::cmpbim ||
          code[i - 1].op == Vinstr::testbim ||
          code[i - 1].op == Vinstr::cmpqim) {
        iFirstCmp = i - 1;
      }
    }
  }
  if (iLastFbcc == code.size()) return; // no fallbackccs
  if (iFirstCmp == 0 || iFirstCmp == code.size()) {
    // nothing before the fallbackccs
    return;
  }

  auto isStore = [&](Vinstr& instr) {
    return instr.op == Vinstr::store || instr.op == Vinstr::storeups;
  };

  // Look for a sequence of stores before the last fallbackcc.
  if (!isStore(code[iFirstCmp - 1])) return;
  size_t iLastStore = iFirstCmp - 1;
  size_t iFirstStore = iLastStore;
  while (iFirstStore > 0 && isStore(code[iFirstStore - 1])) iFirstStore--;

  // Move all the instructions from [iFirstStore, iLastStore] to after
  // the last fallbackcc.
  printUnit(kVasmHoistFbccsLevel, "before vasm-hoistFallbackccs", unit);
  size_t nMoving = iLastStore - iFirstStore + 1;
  jit::vector<Vinstr> tmp;
  for (size_t i = iFirstStore; i <= iLastStore; i++) {
    tmp.push_back(code[i]);
  }
  for (size_t i = iLastStore + 1; i <= iLastFbcc; i++) {
    code[i - nMoving] = code[i];
  }
  for (size_t i = 0; i < tmp.size(); i++) {
    code[iLastFbcc - nMoving + 1 + i] = tmp[i];
  }
  printUnit(kVasmHoistFbccsLevel, "after vasm-hoistFallbackccs", unit);
}

} }
