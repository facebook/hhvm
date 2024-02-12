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

#include "hphp/runtime/base/configs/jit.h"
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

namespace HPHP::jit {

namespace {

// track ldimmq values
struct ImmState {
  ImmState() {}
  ImmState(Immed64 a, Vreg b) : val{a}, base{b} {}

  void reset() { base = Vreg{}; }

  Immed64 val{0};
  Vreg base;
};

struct Env {
  Vunit& unit;
  std::vector<ImmState> immStateVec;
};

bool isMultiword(int64_t imm) {
  switch (arch()) {
  case Arch::X64:
    break;
  case Arch::ARM:
    uint64_t val = std::abs(imm);
    if (val > (1 << 16)) return true;
    break;
  }
  return false;
}

// candidate around +/- uimm12
Optional<int> reuseCandidate(Env& env, int64_t p, Vreg& reg) {
  for (auto const& elem : env.immStateVec) {
    if (!elem.base.isValid()) continue;
    int64_t q = elem.val.q();
    if (((p >= q) && (p < (q + 4095))) ||
        ((p < q) && (q < (p + 4095)))) {
      reg = elem.base;
      return make_optional(safe_cast<int>(p - q));
    }
  }
  return std::nullopt;
}

template <typename Inst>
void reuseImmq(Env& env, const Inst& /*inst*/, Vlabel /*b*/, size_t i) {
  // leaky bucket
  env.immStateVec[i % Cfg::Jit::LdimmqSpan].reset();
}

template<typename ReuseImm>
void reuseimm_impl(Vunit& unit, Vlabel b, size_t i, ReuseImm reuse) {
  vmodify(unit, b, i, [&] (Vout& v) { reuse(v); return 1; });
}

void reuseImmq(Env& env, const ldimmq& ld, Vlabel b, size_t i) {
  if (isMultiword(ld.s.q())) {
    Vreg base;
    auto const off = reuseCandidate(env, ld.s.q(), base);

    if (off.has_value()) {
      reuseimm_impl(env.unit, b, i, [&] (Vout& v) {
        if (off.value() == 0) {
          v << copy{base, ld.d};
        } else {
          v << addqi{off.value(), base, ld.d, v.makeReg()};
        }
      });
      return;
    }
  }
  env.immStateVec[i % Cfg::Jit::LdimmqSpan] = ImmState{ld.s, ld.d};
}

void reuseImmq(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto const& inst = env.unit.blocks[b].code[i];

  if (isCall(inst)) {
    for (auto& elem : env.immStateVec) elem.reset();
    return;
  }

  switch (inst.op) {
#define O(name, ...)                      \
    case Vinstr::name:                    \
      reuseImmq(env, inst.name##_, b, i); \
      break;

    VASM_OPCODES
#undef O
  }
}

}

// Opportunistically reuse immediate q values
void reuseImmq(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  if (Cfg::Jit::LdimmqSpan <= 0) return;

  Env env { unit };
  env.immStateVec.resize(Cfg::Jit::LdimmqSpan);

  auto const labels = sortBlocks(unit);

  for (auto const b : labels) {
    for (auto& elem : env.immStateVec) elem.reset();
    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      reuseImmq(env, b, i);
    }
  }

  printUnit(kVasmSimplifyLevel, "after vasm reuse immq", unit);
}

}
