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

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/util/arch.h"
#include "hphp/util/assertions.h"

#include <cstdlib>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

// track ldimmq values
struct ImmState {
  ImmState() : val{0}, base{} {}
  ImmState(Immed64 a, Vreg b) : val{a}, base{b} {}

  Immed64 val;
  Vreg base;
};

struct ImmStateAry {
  void clear();
  ImmState& operator[](size_t i) { return immState_ary[i % size]; }

  int size;
  enum { max_history = 16};
  ImmState immState_ary[max_history];
};

void ImmStateAry::clear() {
  for (int i=0; i < size; i++) {
    immState_ary[i].base = Vreg{};
  }
}

struct Env {
  Vunit& unit;
  ImmStateAry& immStateAry;
};

bool isMultiword(int64_t imm) {
  switch (arch()) {
  case Arch::X64:
  case Arch::PPC64:
    break;
  case Arch::ARM:
    uint64_t val = std::abs(imm);
    if (val > (1 << 16)) return true;
    break;
  }
  return false;
}

// candidate around +/- uimm12
bool reuseCandidate(Env& env, int64_t p, Vreg& reg, int& delta) {
  delta = 0;
  for (int i = 0; i < env.immStateAry.size; i++) {
    ImmState elem = env.immStateAry[i];
    if (!elem.base.isValid()) continue;
    int64_t q = elem.val.q();
    if ((p >= q) && (p < (q + 4095))) {
      delta = safe_cast<int>(p - q);
      reg = elem.base;
      return true;
    }
    if ((p < q) && (q < (p + 4095))) {
      delta = -safe_cast<int>(q - p);
      reg = elem.base;
      return true;
    }
  }
  return false;
}

template <typename Inst>
void reuseImmq(Env& env, const Inst& inst, Vlabel b, size_t i) { 
  env.immStateAry[i] = ImmState{};
  return;
}

// purge table across any instruction with RegSet or control flow change
#define Y(vasm_opc) \
void reuseImmq(Env& env, const vasm_opc& c, Vlabel b, size_t i) { \
  env.immStateAry.clear(); \
  return; \
} 

Y(fallthru)
Y(call)
Y(callarray)
Y(callfaststub)
Y(callm)
Y(callr)
Y(calls)
Y(callstub)
Y(calltc)
Y(contenter)
Y(jcc)
Y(jcci)
Y(jmpi)
Y(jmpm)
Y(jmpr)
Y(landingpad)
Y(leavetc)
Y(nothrow)
Y(phpret)
Y(ret)
Y(syncpoint)
Y(ud2)
Y(unwind)

#undef Y


template<typename ReuseImm>
void reuseimm_impl(Vunit& unit, Vlabel b, size_t i, ReuseImm reuse) {
  vmodify(unit, b, i, [&] (Vout& v) { reuse(v); return 1; });
}

void reuseImmq(Env& env, const ldimmq& ld, Vlabel b, size_t i) {
  if (isMultiword(ld.s.q())) {
    int off;
    Vreg base;
    if (reuseCandidate(env, ld.s.q(), base, off)) {
     if (off >= 0 ) {
       reuseimm_impl(env.unit, b, i, [&] (Vout& v) {
         v << addqi{off, base, ld.d, v.makeReg()};
       });
     } else {
       reuseimm_impl(env.unit, b, i, [&] (Vout& v) {
         v << subqi{-off, base, ld.d, v.makeReg()};
       });
      }
    return;
    }
  }
  env.immStateAry[i] = ImmState{ld.s, ld.d};
}

void reuseImmq(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto const& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)                      \
    case Vinstr::name:                    \
      reuseImmq(env, inst.name##_, b, i); \
      return;

  VASM_OPCODES
#undef O
  }
  not_reached();
}

}

// Opportunistically reuse immediate q values
void reuseImmq(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  if (RuntimeOption::EvalJitLdimmqSpan <= 0) {
    return;
  }

  ImmStateAry immStateAry;
  Env env { unit, immStateAry };

  auto const labels = sortBlocks(unit);
  env.immStateAry.size =
    std::min<int>(RuntimeOption::EvalJitLdimmqSpan, ImmStateAry::max_history);

  for (auto const b : labels) {
    env.immStateAry.clear();

    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      reuseImmq(env, b, i);
    }
  }

  printUnit(kVasmSimplifyLevel, "after vasm reuse immq", unit);
}

}}

