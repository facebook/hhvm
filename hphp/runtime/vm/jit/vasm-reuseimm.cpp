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

#include <cstdlib>

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/util/assertions.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

struct qTrack {
  Immed64 val;
  Vreg base;
  int  ttl;
};

struct Env {
  Vunit& unit;
  int tick;
  std::deque<qTrack> qdeque; // leaky bucket of recent immediates
};

static bool isMultiword(int64_t imm) {
  uint64_t val = abs(imm);
  if (val > (1 << 16)) return true;
  return false;
}

// candidate around +/- uimm12
static bool reuseCandidate(Env& env, int64_t p, Vreg& reg, int& delta)
{
  delta = 0;
  for (const auto& itr : env.qdeque) {
    int64_t q = itr.val.q();
    if ((p >= q) && (p < (q+4095))) {
      delta = safe_cast<int>(p-q);
      reg = itr.base;
      return true;
    }
    if ((p < q) && (q < (p+4095))) {
      delta = safe_cast<int>(q-p);
      delta = -delta;
      reg = itr.base;
      return true;
    }
  }
  return false;
}

template <typename Inst>
void reuseImmq(Env& env, const Inst& inst, Vlabel b, size_t i) { return; }

// purge table across any instruction with RegSet or flow change
#define Y(vasm_opc) \
void reuseImmq(Env& env, const vasm_opc& c, Vlabel b, size_t i) { \
  env.qdeque.clear(); \
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
void reuseImm_impl(Vunit& unit, Vlabel b, size_t i, ReuseImm reuse) {
  vmodify(unit, b, i, [&] (Vout& v) { reuse(v); return 1; });
}

void reuseImmq(Env& env, const ldimmq& ld, Vlabel b, size_t i) {
  if (isMultiword(ld.s.q())) {
    int off;
    Vreg base;
    if (reuseCandidate(env, ld.s.q(), base, off)) {
     if (off >= 0 ) {
       reuseImm_impl(env.unit, b, i, [&] (Vout& v) {
         v << addqi{off, base, ld.d, v.makeReg()};
        });
     } else {
       reuseImm_impl(env.unit, b, i, [&] (Vout& v) {
         v << subqi{-off, base, ld.d, v.makeReg()};
        });
      }
    return;
    }
  }
  env.qdeque.push_front(
      qTrack{ld.s, ld.d, (env.tick + RuntimeOption::EvalJitLdimmqSpan)}
    ); 
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

// Opportunistically reuse immediate q values
void reuseImmq(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  Env env {unit};

  auto const labels = sortBlocks(unit);

  for (auto const b : labels) {
    env.tick = 0;
    env.qdeque.clear();

    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      reuseImmq(env, b, i);
      env.tick++;
      if (!env.qdeque.empty()) {
        auto& p = env.qdeque.back();
        if (p.ttl < env.tick) {
          env.qdeque.pop_back(); 
        }
      }
    }
  }

  printUnit(kVasmSimplifyLevel, "after vasm reuse immq", unit);
}

}}

