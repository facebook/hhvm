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

#include "hphp/runtime/vm/jit/vasm-simplify-internal.h"

#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/vixl/a64/assembler-a64.h"

namespace HPHP::jit::arm {

namespace {

///////////////////////////////////////////////////////////////////////////////

template <typename Inst>
bool simplify(Env&, const Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Inst>
bool psimplify(Env&, const Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool operand_one(Env& env, Vreg op) {
  auto const op_it = env.unit.regToConst.find(op);
  if (op_it == env.unit.regToConst.end()) return false;

  auto const op_const = op_it->second;
  if (op_const.isUndef) return false;
  if (op_const.val != 1) return false;
  return true;
}

// Reduce use of immediate one possibly removing def as dead code.
// Specific to ARM using hard-coded zero register.
template <typename Out, typename Inst>
bool cmov_fold_one(Env& env, const Inst& inst, Vlabel b, size_t i) {
  if (operand_one(env, inst.f)) {
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << Out{inst.cc, inst.sf, PhysReg(vixl::wzr), inst.t, inst.d};
      return 1;
    });
  }
  if (operand_one(env, inst.t)) {
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << Out{ccNegate(inst.cc), inst.sf, PhysReg(vixl::wzr), inst.f, inst.d};
      return 1;
    });
  }
  return false;
}

bool simplify(Env& env, const cmovb& inst, Vlabel b, size_t i) {
  return cmov_fold_one<csincb>(env, inst, b, i);
}

bool simplify(Env& env, const cmovw& inst, Vlabel b, size_t i) {
  return cmov_fold_one<csincw>(env, inst, b, i);
}

bool simplify(Env& env, const cmovl& inst, Vlabel b, size_t i) {
  return cmov_fold_one<csincl>(env, inst, b, i);
}

bool simplify(Env& env, const cmovq& inst, Vlabel b, size_t i) {
  return cmov_fold_one<csincq>(env, inst, b, i);
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const loadb& inst, Vlabel b, size_t i) {
  if (if_inst<Vinstr::movzbl>(env, b, i + 1, [&] (const movzbl& mov) {
      // loadb{s, tmp}; movzbl{tmp, d}; -> loadzbl{s, d};
      if (!(env.use_counts[inst.d] == 1 && inst.d == mov.s)) return false;

      return simplify_impl(env, b, i, [&] (Vout& v) {
        v << loadzbl{inst.s, mov.d};
        return 2;
      }); })) {
      return true;
    }

  return if_inst<Vinstr::movsbl>(env, b, i + 1, [&] (const movsbl& mov) {
      // loadb{s, tmp}; movsbl{tmp, d}; -> loadsbl{s, d};
      if (!(env.use_counts[inst.d] == 1 && inst.d == mov.s)) return false;

      return simplify_impl(env, b, i, [&] (Vout& v) {
        v << loadsbl{inst.s, mov.d};
        return 2;
      }); });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const ldimmq& inst, Vlabel b, size_t i) {
  return if_inst<Vinstr::lea>(env, b, i + 1, [&] (const lea& ea) {
    // ldimmq{s, index}; lea{base[index], d} -> lea{base[s],d}
    if (!(env.use_counts[inst.d] == 1 &&
          inst.s.q() <= 4095  &&
          inst.s.q() >= -4095 &&
          inst.d == ea.s.index &&
          ea.s.disp == 0 &&
          ea.s.base.isValid())) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << lea{ea.s.base[inst.s.l()], ea.d};
      return 2;
    });
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const movzbl& inst, Vlabel b, size_t i) {
  // movzbl{s, d}; shrli{2, s, d} --> ubfmli{2, 7, s, d}
  return if_inst<Vinstr::shrli>(env, b, i + 1, [&](const shrli& sh) {
    if (!(sh.s0.l() == 2 &&
      env.use_counts[inst.d] == 1 &&
      env.use_counts[sh.sf] == 0 &&
      inst.d == sh.s1)) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << copy{inst.s, inst.d};
      v << ubfmli{2, 7, inst.d, sh.d};
      return 2;
    });
  });
}

///////////////////////////////////////////////////////////////////////////////

int is_adjacent_vptr64(const Vptr64& a, const Vptr64& b, int32_t step, int32_t min_disp, int32_t max_disp) {
  const int32_t min_disp_val = a.disp < b.disp ? a.disp : b.disp;
  if (a.base.isValid() && b.base.isValid() &&
      !a.index.isValid() && !b.index.isValid() &&
      a.base == b.base &&
      a.scale == 1 && b.scale == 1 &&
      a.width == b.width &&
      (a.disp - b.disp == step || b.disp - a.disp == step) &&
      (min_disp_val >= min_disp && min_disp_val <= max_disp && (min_disp_val % step) == 0)) {
    return a.disp < b.disp ? -1 : 1;
  }
  return 0;
}

bool simplify(Env& env, const store& inst, Vlabel b, size_t i) {
  // store{s, d}; store{s, d} --> storepair{s0, s1, d}
  return if_inst<Vinstr::store>(env, b, i + 1, [&](const store& st) {
    if (!inst.s.isGP()) return false;
    if (!st.s.isGP()) return false;
    const auto rv = is_adjacent_vptr64(inst.d, st.d, 8, -512, 504);
    if (rv != 0) {
      return simplify_impl(env, b, i, [&] (Vout& v) {
        if (rv < 0) {
          v << storepair{inst.s, st.s, inst.d};
        } else {
          v << storepair{st.s, inst.s, st.d};
        }
        return 2;
      });
    }
    return false;
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const storel& inst, Vlabel b, size_t i) {
  // storel{s, d}; storel{s, d} --> storepairl{s0, s1, d}
  return if_inst<Vinstr::storel>(env, b, i + 1, [&](const storel& st) {
    if (!inst.s.isGP()) return false;
    if (!st.s.isGP()) return false;
    const auto rv = is_adjacent_vptr64(inst.m, st.m, 4, -256, 252);
    if (rv != 0) {
      return simplify_impl(env, b, i, [&] (Vout& v) {
        if (rv < 0) {
          v << storepairl{inst.s, st.s, inst.m};
        } else {
          v << storepairl{st.s, inst.s, st.m};
        }
        return 2;
      });
    }
    return false;
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const load& inst, Vlabel b, size_t i) {
  // load{d, m}; load{d, m} --> loadpair{s, d0, d1}
  return if_inst<Vinstr::load>(env, b, i + 1, [&](const load& ld) {
    if (inst.d == ld.d) return false;
    if (!inst.d.isGP()) return false;
    if (!ld.d.isGP()) return false;
    const auto rv = is_adjacent_vptr64(inst.s, ld.s, 8, -512, 504);
    if (rv != 0) {
      return simplify_impl(env, b, i, [&] (Vout& v) {
        if (rv < 0) {
          v << loadpair{inst.s, inst.d, ld.d};
        } else {
          v << loadpair{ld.s, ld.d, inst.d};
        }
        return 2;
      });
    }
    return false;
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const loadl& inst, Vlabel b, size_t i) {
  // loadl{d, m}; loadl{d, m} --> loadpairl{s, d0, d1}
  bool simplified = if_inst<Vinstr::loadl>(env, b, i + 1, [&](const loadl& ld) {
    if (inst.d == ld.d) return false;
    if (!inst.d.isGP()) return false;
    if (!ld.d.isGP()) return false;
    const auto rv = is_adjacent_vptr64(inst.s, ld.s, 4, -256, 252);
    if (rv != 0) {
      return simplify_impl(env, b, i, [&] (Vout& v) {
        if (rv < 0) {
          v << loadpairl{inst.s, inst.d, ld.d};
        } else {
          v << loadpairl{ld.s, ld.d, inst.d};
        }
        return 2;
      });
    }
    return false;
  });
  if (simplified) return true;

  // Eliminate IncRef/DecRef redundant load:
  // B1:
  //    ...
  //    loadl      [xI] => xJ
  //    cmpli      0, xJ => SF
  //    jcc        GE, SF, B2, else B3
  // B2:   preds: B1
  //    loadl      [xI] => xK
  //    incl/decl  xK => xK, SF
  //    storel     xK, [xI]
  if (i == 0 && env.preds[b].size() == 1) {
    auto const p = env.preds[b][0];
    if (env.unit.blocks[p].code.size() >= 3) {
      auto& predCode = env.unit.blocks[p].code;
      auto const lastIdx = predCode.size() - 1;
      auto const& pred1 = predCode[lastIdx];
      auto const& pred2 = predCode[lastIdx - 1];
      auto const& pred3 = predCode[lastIdx - 2];
      if (pred1.op == Vinstr::jcc && pred2.op == Vinstr::cmpli &&
          pred3.op == Vinstr::loadl && pred3.loadl_.s == inst.s) {
        return simplify_impl(env, b, i, [&] (Vout& v) {
          v << movl{pred3.loadl_.d, inst.d};
          return 1;
        });
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool psimplify(Env& env, const store& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

bool psimplify(Env& env, const storel& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

bool psimplify(Env& env, const load& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

bool psimplify(Env& env, const loadl& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

///////////////////////////////////////////////////////////////////////////////

}

bool simplify(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto const& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)    \
    case Vinstr::name:  \
      return simplify(env, inst.name##_, b, i); \

    VASM_OPCODES
#undef O
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

bool psimplify(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto const& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)    \
    case Vinstr::name:  \
      return psimplify(env, inst.name##_, b, i); \

    VASM_OPCODES
#undef O
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}
