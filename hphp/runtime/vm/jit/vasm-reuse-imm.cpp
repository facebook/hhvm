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
#include "hphp/util/configs/jit.h"

#include <cstdlib>

TRACE_SET_MOD(vasm)

namespace HPHP::jit {

namespace {

// track ldimmq (single quad) and ldimm128 (two quads) values
struct ImmState {
  ImmState() {}
  ImmState(Immed64 a, Vreg b) : val{a}, base{b} {}
  ImmState(Immed64 a, Immed64 a2, Vreg b)
    : is128{true}, val{a}, valHi{a2}, base{b} {}

  void reset() { base = Vreg{}; }

  bool is128{false};
  Immed64 val{0};
  Immed64 valHi{0};
  Vreg base;
};

struct Env {
  Vunit& unit;
  std::vector<ImmState> immStateVec;
};

bool isMultiword(int64_t imm) {
  return ARCH_MATCH(
    [](arch::X64) { return false; },
    [&](arch::ARM) {
      uint64_t val = std::abs(imm);
      if (val > (1 << 16)) return true;
      return false;
    }
  );
}

// candidate around +/- uimm12
Optional<int> reuseCandidate(Env& env, int64_t p, Vreg& reg) {
  for (auto const& elem : env.immStateVec) {
    if (!elem.base.isValid()) continue;
    if (elem.is128) continue;
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
          v << lea{base[off.value()], ld.d};
        }
      });
      return;
    }
  }
  env.immStateVec[i % Cfg::Jit::LdimmqSpan] = ImmState{ld.s, ld.d};
}

// Exact match for a previously materialized 128-bit immediate.
Optional<Vreg> reuse128Candidate(Env& env, Immed64 s0, Immed64 s1) {
  for (auto const& elem : env.immStateVec) {
    if (!elem.base.isValid()) continue;
    if (!elem.is128) continue;
    if (elem.val.q() == s0.q() && elem.valHi.q() == s1.q()) return elem.base;
  }
  return std::nullopt;
}

void reuseImmq(Env& env, const ldimm128& ld, Vlabel b, size_t i) {
  auto const s0 = ld.s0;
  auto const s1 = ld.s1;
  auto const d = ld.d;
  auto const base = reuse128Candidate(env, s0, s1);
  if (base.has_value()) {
    reuseimm_impl(env.unit, b, i, [&] (Vout& v) {
      v << copy{*base, d};
    });
  }
  // Keep the copied 128-bit value reusable. Unlike ldimmq offset reuse, this
  // does not build a GP copy chain, and rematerializing ldimm128 can be several
  // instructions on ARM.
  env.immStateVec[i % Cfg::Jit::LdimmqSpan] = ImmState{s0, s1, d};
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
