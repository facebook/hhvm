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

#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {

/*
 * Some vasm opcodes don't have equivalent single instructions on ARM, and the
 * equivalent instruction sequences require scratch registers. We have to lower
 * these to ARM-suitable vasm opcodes before register allocation.
 */

namespace {

template<typename Inst>
void lower(Inst& i, Vout& v) {
  v << i;
}

void lower(cmpbim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadzbl{i.s1, scratch};
  v << cmpli{i.s0, scratch, i.sf};
}

void lower(cmplim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadl{i.s1, scratch};
  v << cmpli{i.s0, scratch, i.sf};
}

void lower(cmpqm& i, Vout& v) {
  auto scratch = v.makeReg();
  v << load{i.s1, scratch};
  v << cmpq{i.s0, scratch, i.sf};
}

void lower(testbim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadzbl{i.s1, scratch};
  v << testli{i.s0, scratch, i.sf};
}

} // anonymous namespace

void lowerForARM(Vunit& unit) {
  assert(check(unit));

  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);

  for (auto b : blocks) {
    auto oldCode = std::move(unit.blocks[b].code);
    Vout v{unit, b};

    for (auto& inst : oldCode) {
      v.setOrigin(inst.origin);

      switch (inst.op) {
#define O(nm, imm, use, def) \
        case Vinstr::nm: \
          lower(inst.nm##_, v); \
          break;

        VASM_OPCODES
#undef O
      }
    }
  }

  assert(check(unit));
  printUnit(kVasmARMFoldLevel, "after lowerForARM", unit);
}

}}
