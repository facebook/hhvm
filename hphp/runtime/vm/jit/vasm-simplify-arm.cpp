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

template <typename reg_type>
static bool get_const_int(Env& env, reg_type op, uint64_t &Val) {
  auto const op_it = env.unit.regToConst.find(op);
  if (op_it == env.unit.regToConst.end()) return false;
  auto const op_const = op_it->second;
  assert(op_const.kind != Vconst::Double);
  if (op_const.isUndef) return false;
  Val = op_const.val;
  return true;
}
struct OperandMatch {
  VregShiftExtend operand;
  size_t startIndex;
  size_t len;
};

struct ExtendInfo {
  Vreg reg;
  vixl::Extend kind;
};

Optional<ExtendInfo> match_extend_inst(const Vinstr& inst,
                                       Width width,
                                       Vreg dst) {
  switch (inst.op) {
    case Vinstr::movzbl: {
      if (width != Width::Long) return {};
      auto const& mov = inst.movzbl_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::UXTB};
    }
    case Vinstr::movzwl: {
      if (width != Width::Long) return {};
      auto const& mov = inst.movzwl_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::UXTH};
    }
    case Vinstr::movsbl: {
      if (width != Width::Long) return {};
      auto const& mov = inst.movsbl_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::SXTB};
    }
    case Vinstr::movswl: {
      if (width != Width::Long) return {};
      auto const& mov = inst.movswl_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::SXTH};
    }
    case Vinstr::movzbq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movzbq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::UXTB};
    }
    case Vinstr::movzwq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movzwq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::UXTH};
    }
    case Vinstr::movzlq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movzlq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::UXTW};
    }
    case Vinstr::movsbq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movsbq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::SXTB};
    }
    case Vinstr::movswq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movswq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::SXTH};
    }
    case Vinstr::movslq: {
      if (width != Width::Quad) return {};
      auto const& mov = inst.movslq_;
      if (mov.d != dst) return {};
      return ExtendInfo{mov.s, vixl::SXTW};
    }
    default:
      break;
  }
  return {};
}

template<class Reg>
Optional<OperandMatch> match_operand_fold(Env& env,
                                          Vlabel b,
                                          size_t i,
                                          Reg operand,
                                          Width width,
                                          bool allowExtend) {
  if (i == 0) return {};

  auto& code = env.unit.blocks[b].code;
  auto const idx1 = i - 1;
  auto const& inst1 = code[idx1];

  auto const single_use = [&] (Vreg r) {
    return env.use_counts[r] == 1;
  };

  auto const maybe_extend_before_shift =
    [&] (auto const& shl, uint8_t amount, Width w) -> Optional<OperandMatch> {
      if (!allowExtend) return {};
      if (idx1 == 0) return {};
      if (shl.d != operand) return {};
      auto const idx0 = idx1 - 1;
      auto const& inst0 = code[idx0];
      auto const extend = match_extend_inst(inst0, w, shl.s1);
      if (!extend) return {};
      if (!single_use(shl.s1)) return {};
      if (!single_use(shl.d) || env.use_counts[shl.sf] != 0) return {};
      if (amount > 4) return {};
      return OperandMatch{VregExtend(extend->reg, extend->kind, amount), idx0, 3};
    };

  if (width == Width::Long && inst1.op == Vinstr::shlli) {
    auto const& shl = inst1.shlli_;
    auto const amount64 = shl.s0.l();
    always_assert_flog(amount64 >= 0 && amount64 <= 31,
                       "bad shift amount {} for shlli", amount64);
    auto const amount = static_cast<uint8_t>(amount64);
    if (auto match = maybe_extend_before_shift(shl, amount, width)) {
      return match;
    }
    if (shl.d == operand && single_use(shl.d) && env.use_counts[shl.sf] == 0) {
      return OperandMatch{VregShift(shl.s1, vixl::LSL, amount), idx1, 2};
    }
  } else if (width == Width::Quad && inst1.op == Vinstr::shlqi) {
    auto const& shl = inst1.shlqi_;
    auto const amount64 = shl.s0.q();
    always_assert_flog(amount64 >= 0 && amount64 <= 63,
                       "bad shift amount {} for shlqi", amount64);
    auto const amount = static_cast<uint8_t>(amount64);
    if (auto match = maybe_extend_before_shift(shl, amount, width)) {
      return match;
    }
    if (shl.d == operand && single_use(shl.d) && env.use_counts[shl.sf] == 0) {
      return OperandMatch{VregShift(shl.s1, vixl::LSL, amount), idx1, 2};
    }
  }

  if (allowExtend) {
    if (auto extend = match_extend_inst(inst1, width, operand)) {
      if (!single_use(operand)) return {};
      return OperandMatch{VregExtend(extend->reg, extend->kind, 0), idx1, 2};
    }
  }

  return {};
}

template<class Reg, class Emit>
bool fold_shift_operand(Env& env,
                        Vlabel b,
                        size_t i,
                        Width width,
                        Reg primary,
                        Reg secondary,
                        bool commutative,
                        bool allowExtend,
                        Emit emit) {
  if (auto match = match_operand_fold(env, b, i, primary, width, allowExtend)) {
    return simplify_impl(env, b, match->startIndex, [&] (Vout& v) {
      emit(v, match->operand, secondary);
      return match->len;
    });
  }

  if (commutative) {
    if (auto match = match_operand_fold(env, b, i, secondary, width, allowExtend)) {
      return simplify_impl(env, b, match->startIndex, [&] (Vout& v) {
        emit(v, match->operand, primary);
        return match->len;
      });
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const shrqi& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.d] != 1 || env.use_counts[inst.sf] ||
      inst.s0.l() > 32) return false;

  return if_inst<Vinstr::testl>(env, b, i + 1, [&] (const testl& tstl) {
    if (tstl.s1 != inst.d || env.use_counts[tstl.sf] != 1) return false;

    uint64_t Val;
    if (!get_const_int(env, tstl.s0, Val) || folly::popcount(Val) != 1) {
      return false;
    }

    auto shift_amt = inst.s0.l();
    return simplify_impl(env, b, i, [&] (Vout& v) {
      uint64_t NewVal = Val << shift_amt;
      v << testq{env.unit.makeConst(NewVal), inst.s1, tstl.sf, tstl.fl};
      return 2;
    });
  });
}

bool simplify(Env& env, const testq& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.sf] == 1) {
    uint64_t Val;
    if (get_const_int(env, inst.s0, Val) &&
        Val == 0x8000000000000000ull) {
      if (auto const matched = if_inst<Vinstr::jcc>(env, b, i + 1,
            [&] (const jcc& jcci) {
              if (jcci.sf != inst.sf || jcci.cc != CC_E) return false;
              return simplify_impl(env, b, i, [&] (Vout& v) {
                auto const sf = v.makeReg();
                v << cmpqi{0, inst.s1, sf, inst.fl};
                v << jcc{CC_GE, sf, {jcci.targets[0], jcci.targets[1]}, jcci.tag};
                return 2;
              });
            })) {
        return matched;
      }
    }
  }

  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << testshiftq{operand, other, inst.sf, inst.fl};
    });
}

///////////////////////////////////////////////////////////////////////////////

template <Vinstr::Opcode ExtOp, typename ExtMov, typename ExtLoad, typename Load>
bool simplify_load_ext(Env& env, const Load& inst, Vlabel b, size_t i) {
  return if_inst<ExtOp>(env, b, i + 1, [&] (const ExtMov& mov) {
      if (env.use_counts[inst.d] != 1 || inst.d != mov.s) return false;

      return simplify_impl(env, b, i, [&] (Vout& v) {
        v << ExtLoad{inst.s, mov.d};
        return 2;
      });
  });
}

bool simplify(Env& env, const loadb& inst, Vlabel b, size_t i) {
  if (simplify_load_ext<Vinstr::movzbl, movzbl, loadzbl>(env, inst, b, i)) {
    return true;
  }
  if (simplify_load_ext<Vinstr::movzbq, movzbq, loadzbq>(env, inst, b, i)) {
    return true;
  }
  if (simplify_load_ext<Vinstr::movsbl, movsbl, loadsbl>(env, inst, b, i)) {
    return true;
  }
  if (simplify_load_ext<Vinstr::movsbq, movsbq, loadsbq>(env, inst, b, i)) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const addl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, true, true,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << addshiftl{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const addq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, true, true,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << addshiftq{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const andl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << andshiftl{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const andq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << andshiftq{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const orq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << orshiftq{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const subl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, false, true,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << subshiftl{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const subq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, false, true,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << subshiftq{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const cmpl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, false, true,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << cmpshiftl{operand, other, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const cmpq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, false, true,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << cmpshiftq{operand, other, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const testl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << testshiftl{operand, other, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const xorl& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Long, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg32 other) {
      v << xorshiftl{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const xorq& inst, Vlabel b, size_t i) {
  return fold_shift_operand(env, b, i, Width::Quad, inst.s0, inst.s1, true, false,
    [&](Vout& v, VregShiftExtend operand, Vreg64 other) {
      v << xorshiftq{operand, other, inst.d, inst.sf, inst.fl};
    });
}

bool simplify(Env& env, const lea& inst, Vlabel b, size_t i) {
  auto const ptr = inst.s;
  if (!ptr.index.isValid()) return false;

  auto const tryMatch = match_operand_fold(env, b, i,
    Vreg64{ptr.index}, Width::Quad, false);
  if (!tryMatch) return false;

  auto const operand = tryMatch->operand;
  if (!operand.isShift() || operand.shiftKind() != vixl::LSL) return false;

  auto const amount = operand.amount;
  auto const newScale = static_cast<uint32_t>(ptr.scale) << amount;
  if (newScale == 0 || newScale > 8 || (newScale & (newScale - 1))) return false;

  auto const emission = simplify_impl(env, b, tryMatch->startIndex,
    [&] (Vout& v) {
      auto newPtr = ptr;
      newPtr.index = Vreg64{operand.reg};
      newPtr.scale = static_cast<uint8_t>(newScale);
      auto newLea = inst;
      newLea.s = newPtr;
      v << newLea;
      return tryMatch->len;
    });

  return emission;
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

bool simplify(Env& env, const movtqb& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.d] != 1) return false;

  // movtqb{s, tmp}; movzbq{tmp, d} --> movzbq{s, d}
  bool simplified = if_inst<Vinstr::movzbq>(env, b, i + 1, [&](const movzbq& ext) {
      if (ext.s != inst.d) return false;

      return simplify_impl(env, b, i, [&] (Vout& v) {
        v << movzbq{Vreg8((Vreg)inst.s), ext.d};
        return 2;
      });
    });

  if (simplified) return true;

  // movtqb{s, tmp}; andbi{imm, tmp, d} --> copy{s, tmp}; andbi{imm, tmp, d}
  // the copy vasm could be a nop if tmp == d
  return if_inst<Vinstr::andbi>(env, b, i + 1, [&](const andbi& vandbi) {
      if (vandbi.s1 != inst.d) return false;

      return simplify_impl(env, b, i, [&] (Vout& v) {
        v << copy{Vreg8((Vreg)inst.s), vandbi.s1};
        return 1;
      });
    });
}

bool simplify(Env& env, const movzbq& inst, Vlabel b, size_t i) {
  auto const def_op = env.def_insts[inst.s];

  // Check if `inst.s' was defined by an andbi instruction, which
  // automatically clears the high bits.
  if (def_op != Vinstr::andbi) {
    return false;
  }

  // If so, the movzbq{} is redundant
  return simplify_impl(env, b, i, [&] (Vout& v) {
    v << copy{inst.s, inst.d};
    return 1;
  });
}

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

bool simplify(Env& env, const movsbq& inst, Vlabel b, size_t i) {
  // movsbq{s, tmp}; shlqi{imm, tmp, d} -> sbfizq{imm, 8, s, d}
  return if_inst<Vinstr::shlqi>(env, b, i + 1, [&] (const shlqi& sh) {
    if (inst.d != sh.s1) return false;
    if (env.use_counts[inst.d] != 1) return false;
    if (sh.fl) return false;
    if (sh.sf.isValid() && env.use_counts[sh.sf]) return false;

    auto const shift = sh.s0.l();
    if (shift < 0 || shift > 56) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      auto const src = Vreg64(Vreg(inst.s));
      v << sbfizq{sh.s0, Immed{8}, src, sh.d};
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
