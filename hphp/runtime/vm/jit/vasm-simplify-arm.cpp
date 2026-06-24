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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-util-arm.h"

#include "hphp/vixl/hphp-compat.h"

#include <algorithm>

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

constexpr auto kInvalidIndex = size_t(-1);

bool inst_defines_reg(const Vunit& unit, const Vinstr& inst, Vreg reg) {
  auto defines = false;
  visitDefs(unit, inst, [&] (Vreg r) {
    if (r.isValid() && r == reg) defines = true;
  });
  return defines;
}

size_t find_def_index(Env& env, Vlabel b, Vreg reg, size_t before) {
  if (!reg.isValid() || reg.isPhys()) return kInvalidIndex;
  auto const& code = env.unit.blocks[b].code;
  auto const end = std::min(before, code.size());
  for (auto i = end; i != 0; --i) {
    auto const idx = i - 1;
    if (inst_defines_reg(env.unit, code[idx], reg)) return idx;
  }
  return kInvalidIndex;
}

bool phys_source_clobbered(Env& env,
                           Vlabel b,
                           Vreg src,
                           size_t begin,
                           size_t end) {
  if (!src.isPhys()) return false;
  auto const& code = env.unit.blocks[b].code;
  for (auto i = begin; i < end; ++i) {
    if (!isPure(code[i])) return true;
    if (inst_defines_reg(env.unit, code[i], src)) return true;
  }
  return false;
}

struct ShiftedExtract {
  uint64_t shift{0};
  Vreg64 src{Vreg::kInvalidReg};
  Vreg64 dst{Vreg::kInvalidReg};
};

bool get_shifted_extract(Env& env,
                         const shrqi& inst,
                         ShiftedExtract& out) {
  if (inst.fl != 0 || env.use_counts[inst.sf] != 0) return false;
  out.shift = inst.s0.l();
  out.src = inst.s1;
  out.dst = inst.d;
  return true;
}

struct XorInfo {
  uint64_t imm{0};
  Vreg64 src{Vreg::kInvalidReg};
  Vreg64 dst{Vreg::kInvalidReg};
  VregSF sf{InvalidReg};
  Vflags fl{0};
};

// xorqi.s0 is Immed (32-bit); zero-extend so masks with bit 31 set don't
// pick up spurious 1s in the upper 32 bits via sign-extension. xorqi64.s0
// is already a full 64-bit immediate.
XorInfo get_xor_info(const xorqi& inst) {
  return XorInfo{inst.s0.ul(), inst.s1, inst.d, inst.sf, inst.fl};
}

XorInfo get_xor_info(const xorqi64& inst) {
  return XorInfo{static_cast<uint64_t>(inst.s0.q()),
                 inst.s1, inst.d, inst.sf, inst.fl};
}

// Narrow test immediates are logical masks, not sign-extended integers, so
// zero-extend them. There is no generic fallback: any unhandled test op
// passed to simplify_shifted_bit_test will fail to compile.
uint64_t get_test_imm(const testbi& inst) { return inst.s0.ub(); }
uint64_t get_test_imm(const testwi& inst) { return inst.s0.uw(); }
uint64_t get_test_imm(const testli& inst) { return inst.s0.ul(); }
uint64_t get_test_imm(const testqi& inst) { return inst.s0.ul(); }
uint64_t get_test_imm(const testqi64& inst) {
  return static_cast<uint64_t>(inst.s0.q());
}

bool test_uses_only_z_flag(Env& env, VregSF sf, Vflags fl) {
  if (env.use_counts[sf] == 0) return true;
  return fl == static_cast<Vflags>(StatusFlags::Z);
}

struct TestInfo {
  uint64_t imm{0};
  Vreg src;
  VregSF sf{InvalidReg};
  Vflags fl{0};
};

template<typename Test>
TestInfo get_test_info(const Test& inst) {
  return TestInfo{get_test_imm(inst), inst.s1, inst.sf, inst.fl};
}

template<typename Test>
bool simplify_shifted_bit_test(Env& env,
                               const Test& inst,
                               Vlabel b,
                               size_t testIdx) {
  auto const& code = env.unit.blocks[b].code;

  auto const vtest = get_test_info(inst);
  if (!test_uses_only_z_flag(env, vtest.sf, vtest.fl)) return false;

  auto const xorIdx = find_def_index(env, b, vtest.src, testIdx);
  if (xorIdx == kInvalidIndex) return false;

  XorInfo vxor;
  switch (code[xorIdx].op) {
    case Vinstr::xorqi:   vxor = get_xor_info(code[xorIdx].xorqi_);   break;
    case Vinstr::xorqi64: vxor = get_xor_info(code[xorIdx].xorqi64_); break;
    default: return false;
  }
  if (Vreg{vxor.dst} != vtest.src) return false;
  if (env.use_counts[vxor.dst] != 1) return false;
  if (env.use_counts[vxor.sf] != 0) return false;

  auto const shiftIdx = find_def_index(env, b, vxor.src, xorIdx);
  if (shiftIdx == kInvalidIndex) return false;

  ShiftedExtract extract;
  if (code[shiftIdx].op != Vinstr::shrqi) return false;
  if (!get_shifted_extract(env, code[shiftIdx].shrqi_, extract)) return false;
  if (Vreg{extract.dst} != Vreg{vxor.src}) return false;
  if (phys_source_clobbered(env, b, extract.src, shiftIdx + 1, testIdx)) {
    return false;
  }

  if (extract.shift == 0 || extract.shift >= 64) return false;

  auto const testImm = vtest.imm;
  // Bits above 63 - shift are constants after the extract, so rewriting the
  // test onto the pre-shift source would silently drop them.
  if ((testImm >> (64 - extract.shift)) != 0) return false;

  auto const shiftedXor = vxor.imm << extract.shift;
  auto const shiftedTest = testImm << extract.shift;
  if (!vixl::Assembler::IsImmLogical(shiftedXor, vixl::kXRegSize) ||
      !vixl::Assembler::IsImmLogical(shiftedTest, vixl::kXRegSize)) {
    return false;
  }

  simplify_impl(env, b, xorIdx,
                xorqi64{Immed64{static_cast<int64_t>(shiftedXor)},
                        extract.src, vxor.dst, vxor.sf, vxor.fl});
  simplify_impl(env, b, testIdx,
                testqi64{Immed64{static_cast<int64_t>(shiftedTest)},
                         vxor.dst, vtest.sf, vtest.fl});
  return true;
}

bool simplify(Env& env, const testbi& inst, Vlabel b, size_t i) {
  return simplify_shifted_bit_test(env, inst, b, i);
}

bool simplify(Env& env, const testwi& inst, Vlabel b, size_t i) {
  return simplify_shifted_bit_test(env, inst, b, i);
}

bool simplify(Env& env, const testli& inst, Vlabel b, size_t i) {
  return simplify_shifted_bit_test(env, inst, b, i);
}

bool simplify(Env& env, const testqi& inst, Vlabel b, size_t i) {
  return simplify_shifted_bit_test(env, inst, b, i);
}

bool simplify(Env& env, const testqi64& inst, Vlabel b, size_t i) {
  return simplify_shifted_bit_test(env, inst, b, i);
}

bool simplify(Env& env, const testq& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.sf] != 1) return false;

  uint64_t Val;
  if (!get_const_int(env, inst.s0, Val) || Val != 0x8000000000000000ull) {
    return false;
  }

  return if_inst<Vinstr::jcc>(env, b, i + 1, [&] (const jcc& jcci) {
    if (jcci.sf != inst.sf || jcci.cc != CC_E) return false;
    return simplify_impl(env, b, i, [&] (Vout& v) {
      auto const sf = v.makeReg();
      v << cmpqi{0, inst.s1, sf, inst.fl};
      v << jcc{CC_GE, sf, {jcci.targets[0], jcci.targets[1]}, jcci.tag};
      return 2;
    });
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

template <typename VptrT, typename Encodable>
int is_adjacent_vptr(const VptrT& a,
                     const VptrT& b,
                     int32_t step,
                     Encodable encodable) {
  auto const& min_disp_ptr = a.disp < b.disp ? a : b;
  if (a.base.isValid() && b.base.isValid() &&
      !a.index.isValid() && !b.index.isValid() &&
      a.base == b.base &&
      a.scale == 1 && b.scale == 1 &&
      a.width == b.width &&
      (a.disp - b.disp == step || b.disp - a.disp == step) &&
      encodable(min_disp_ptr)) {
    return a.disp < b.disp ? -1 : 1;
  }
  return 0;
}

template <typename Emit>
void emit_with_conservative_irctx(Vout& v, bool sameOrigin, Emit emit) {
  if (sameOrigin) {
    emit();
    return;
  }

  auto const saved = v.irctx();
  v.setIrctx({nullptr, Vinstr::kInvalidVoff});
  emit();
  v.setIrctx(saved);
}

bool simplify(Env& env, const store& inst, Vlabel b, size_t i) {
  // store{s, d}; store{s, d} --> storepair{s0, s1, d}
  return if_inst<Vinstr::store>(env, b, i + 1, [&](const store& st) {
    if (!inst.s.isGP()) return false;
    if (!st.s.isGP()) return false;
    const auto rv = is_adjacent_vptr(inst.d, st.d, 8, encodablePair64);
    if (rv != 0) {
      // If the pair combines stores from different IR instructions, clear the
      // origin on the merged store so later alias/rematerialization logic
      // treats it conservatively as an unknown memory write, rather than
      // attributing both stores to only the first origin.
      auto const sameOrigin =
        env.unit.blocks[b].code[i].origin ==
        env.unit.blocks[b].code[i + 1].origin;
      return simplify_impl(env, b, i, [&] (Vout& v) {
        emit_with_conservative_irctx(v, sameOrigin, [&] {
          if (rv < 0) {
            v << storepair{inst.s, st.s, inst.d};
          } else {
            v << storepair{st.s, inst.s, st.d};
          }
        });
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
    const auto rv = is_adjacent_vptr(inst.m, st.m, 4, encodablePair32);
    if (rv != 0) {
      auto const sameOrigin =
        env.unit.blocks[b].code[i].origin ==
        env.unit.blocks[b].code[i + 1].origin;
      return simplify_impl(env, b, i, [&] (Vout& v) {
        emit_with_conservative_irctx(v, sameOrigin, [&] {
          if (rv < 0) {
            v << storepairl{inst.s, st.s, inst.m};
          } else {
            v << storepairl{st.s, inst.s, st.m};
          }
        });
        return 2;
      });
    }
    return false;
  });
}

bool simplify(Env& env, const storeups& inst, Vlabel b, size_t i) {
  // storeups{s, d}; storeups{s, d} --> storepairups{s0, s1, d}
  return if_inst<Vinstr::storeups>(env, b, i + 1, [&](const storeups& st) {
    auto const rv = is_adjacent_vptr(inst.m, st.m, 16, encodablePair128);
    if (rv == 0) return false;

    auto const sameOrigin =
      env.unit.blocks[b].code[i].origin == env.unit.blocks[b].code[i + 1].origin;
    return simplify_impl(env, b, i, [&](Vout& v) {
      emit_with_conservative_irctx(v, sameOrigin, [&] {
        if (rv < 0) {
          v << storepairups{inst.s, st.s, inst.m};
        } else {
          v << storepairups{st.s, inst.s, st.m};
        }
      });
      return 2;
    });
  });
}

bool simplify(Env& env, const push& inst, Vlabel b, size_t i) {
  // push{s1}; push{s2} --> pushp{s1, s2}
  return if_inst<Vinstr::push>(env, b, i + 1, [&](const push& p) {
    if (!inst.s.isGP()) return false;
    if (!p.s.isGP()) return false;
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << pushp{inst.s, p.s};
      return 2;
    });
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const loadups& inst, Vlabel b, size_t i) {
  // loadups{s, d}; loadups{s, d} --> loadpairups{s, d0, d1}
  return if_inst<Vinstr::loadups>(env, b, i + 1, [&](const loadups& ld) {
    if (inst.d == ld.d) return false;

    auto const rv = is_adjacent_vptr(inst.s, ld.s, 16, encodablePair128);
    if (rv == 0) return false;

    auto const sameOrigin =
      env.unit.blocks[b].code[i].origin == env.unit.blocks[b].code[i + 1].origin;
    return simplify_impl(env, b, i, [&](Vout& v) {
      emit_with_conservative_irctx(v, sameOrigin, [&] {
        if (rv < 0) {
          v << loadpairups{inst.s, inst.d, ld.d};
        } else {
          v << loadpairups{ld.s, ld.d, inst.d};
        }
      });
      return 2;
    });
  });
}

bool simplify(Env& env, const load& inst, Vlabel b, size_t i) {
  // load{d, m}; load{d, m} --> loadpair{s, d0, d1}
  return if_inst<Vinstr::load>(env, b, i + 1, [&](const load& ld) {
    if (inst.d == ld.d) return false;
    if (!inst.d.isGP()) return false;
    if (!ld.d.isGP()) return false;

    bool hasDependentUse = false;
    visitUses(env.unit, ld, [&](Vreg r) {
      if (inst.d == r) hasDependentUse = true;
    });
    if (hasDependentUse) return false;

    const auto rv = is_adjacent_vptr(inst.s, ld.s, 8, encodablePair64);
    if (rv != 0) {
      auto const sameOrigin =
        env.unit.blocks[b].code[i].origin ==
        env.unit.blocks[b].code[i + 1].origin;
      return simplify_impl(env, b, i, [&] (Vout& v) {
        emit_with_conservative_irctx(v, sameOrigin, [&] {
          if (rv < 0) {
            v << loadpair{inst.s, inst.d, ld.d};
          } else {
            v << loadpair{ld.s, ld.d, inst.d};
          }
        });
        return 2;
      });
    }
    return false;
  });
}

bool simplify(Env& env, const pop& inst, Vlabel b, size_t i) {
  // pop{d1}; pop{d2} --> popp{d1, d2}
  return if_inst<Vinstr::pop>(env, b, i + 1, [&](const pop& p) {
    if (inst.d == p.d) return false;
    if (!inst.d.isGP()) return false;
    if (!p.d.isGP()) return false;
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << popp{inst.d, p.d};
      return 2;
    });
  });
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const loadl& inst, Vlabel b, size_t i) {
  // loadl{d, m}; loadl{d, m} --> loadpairl{s, d0, d1}
  bool simplified = if_inst<Vinstr::loadl>(env, b, i + 1, [&](const loadl& ld) {
    if (inst.d == ld.d) return false;
    if (!inst.d.isGP()) return false;
    if (!ld.d.isGP()) return false;

    bool hasDependentUse = false;
    visitUses(env.unit, ld, [&](Vreg r) {
      if (inst.d == r) hasDependentUse = true;
    });
    if (hasDependentUse) return false;
    const auto rv = is_adjacent_vptr(inst.s, ld.s, 4, encodablePair32);
    if (rv != 0) {
      auto const sameOrigin =
        env.unit.blocks[b].code[i].origin ==
        env.unit.blocks[b].code[i + 1].origin;
      return simplify_impl(env, b, i, [&] (Vout& v) {
        emit_with_conservative_irctx(v, sameOrigin, [&] {
          if (rv < 0) {
            v << loadpairl{inst.s, inst.d, ld.d};
          } else {
            v << loadpairl{ld.s, ld.d, inst.d};
          }
        });
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

constexpr int64_t kSimpleUpdateMin = -256;
constexpr int64_t kSimpleUpdateMax = 255;

bool validUpdateOffset(int64_t offset, int laneSize, int lanes) {
  if (lanes > 1) {
    if (laneSize <= 0 || (offset % laneSize) != 0) return false;
    auto const scaled = offset / laneSize;
    return scaled >= -64 && scaled <= 63;
  }
  return offset >= kSimpleUpdateMin && offset <= kSimpleUpdateMax;
}

struct UpdateMatch {
  size_t index;
  int64_t offset;
};

Optional<int64_t> extractBaseUpdateOffset(const Vinstr& inst,
                                          Env& env,
                                          Vreg64 base) {
  switch (inst.op) {
    case Vinstr::addqi: {
      auto const& add = inst.addqi_;
      if (add.d != base || add.s1 != base) return {};
      if (add.sf.isValid() && env.use_counts[add.sf]) return {};
      return add.s0.l();
    }
    case Vinstr::lea: {
      auto const& lea = inst.lea_;
      if (lea.d != base || lea.s.base != base) return {};
      if (lea.s.index.isValid()) return {};
      return lea.s.disp;
    }
    case Vinstr::subqi: {
      auto const& sub = inst.subqi_;
      if (sub.d != base || sub.s1 != base) return {};
      if (sub.sf.isValid() && env.use_counts[sub.sf]) return {};
      return -sub.s0.l();
    }
    default:
      return {};
  }
}

template <typename Reg>
bool canFoldPairUpdate(Reg r0, Reg r1) {
  return Vreg(r0).isGP() && Vreg(r1).isGP();
}

bool canFoldPairUpdate(Vreg128 r0, Vreg128 r1) {
  return r0.isSIMD() && r1.isSIMD();
}

Optional<UpdateMatch> matchPreUpdate(Env& env,
                                     Vlabel b,
                                     size_t i,
                                     Vreg64 base) {
  if (i == 0) return {};
  auto const idx = i - 1;
  auto const& inst = env.unit.blocks[b].code[idx];
  auto const offset = extractBaseUpdateOffset(inst, env, base);
  if (!offset) return {};
  return UpdateMatch{idx, *offset};
}

Optional<UpdateMatch> matchPostUpdate(Env& env,
                                      Vlabel b,
                                      size_t i,
                                      Vreg64 base) {
  auto const& code = env.unit.blocks[b].code;
  if (i + 1 >= code.size()) return {};
  auto const idx = i + 1;
  auto const offset = extractBaseUpdateOffset(code[idx], env, base);
  if (!offset) return {};
  return UpdateMatch{idx, *offset};
}

template<class Ptr>
bool isSimpleAddress(const Ptr& ptr, Vreg64 base) {
  return base.isValid() &&
         ptr.base == base &&
         !ptr.index.isValid() &&
         ptr.disp == 0;
}

template<class EmitFn>
bool foldPreUpdateImpl(Env& env,
                       Vlabel b,
                       size_t i,
                       Vreg64 base,
                       int laneSize,
                       int lanes,
                       EmitFn emitFn) {
  auto const match = matchPreUpdate(env, b, i, base);
  if (!match) return false;
  if (!validUpdateOffset(match->offset, laneSize, lanes)) return false;
  auto const offset = match->offset;
  return simplify_impl(env, b, match->index, [&] (Vout& v) {
    emitFn(v, offset);
    return 2;
  });
}

template<class EmitFn>
bool foldPostUpdateImpl(Env& env,
                        Vlabel b,
                        size_t i,
                        Vreg64 base,
                        int laneSize,
                        int lanes,
                        EmitFn emitFn) {
  auto const match = matchPostUpdate(env, b, i, base);
  if (!match) return false;
  if (!validUpdateOffset(match->offset, laneSize, lanes)) return false;
  auto const offset = match->offset;
  assertx(match->index == i + 1);
  return simplify_impl(env, b, i, [&] (Vout& v) {
    emitFn(v, offset);
    return 2;
  });
}

bool psimplify(Env& env, const push& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

bool psimplify(Env& env, const pop& inst, Vlabel b, size_t i) {
  return simplify(env, inst, b, i);
}

/*
 * Only run this in post-RA simplify as this prevents greedy optimization
 * of shorter popp sequences. After RA, we would have exposed the most number
 * of opportunities.
 */
bool psimplify(Env& env, const popp& inst, Vlabel b, size_t i) {
  auto const& code = env.unit.blocks[b].code;

  auto j = i;
  while (j < code.size() && code[j].op == Vinstr::popp) {
    if (code[j].popp_.d0 == code[j].popp_.d1) return false;
    ++j;
  }
  uint32_t count = j - i;

  // Stay within the ldp offset range of [-512, 504]
  if (count < 2 || (count - 1) * 16 > 504) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    for (uint32_t idx = 0; idx < count; idx++) {
      auto const& pp = code[i + idx].popp_;
      auto const disp = idx * 16;
      v << loadpair{Vptr128{rsp(), disp}, pp.d0, pp.d1};
    }
    auto const total = static_cast<int32_t>(count * 16);
    v << lea{rsp()[total], rsp()};
    return count;
  });
}

/*
 * Only run this in post-RA simplify as this prevents greedy optimization
 * of shorter pushp sequences. After RA, we would have exposed the most number
 * of opportunities.
 */
bool psimplify(Env& env, const pushp& inst, Vlabel b, size_t i) {
  auto const& code = env.unit.blocks[b].code;

  auto j = i;
  while (j < code.size() && code[j].op == Vinstr::pushp) ++j;
  uint32_t count = j - i;

  // Stay within the stp offset range of [-512, 504]
  if (count < 2 || (count - 1) * 16 > 504) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    auto disp = 16 * count;
    v << lea{rsp()[-disp], rsp()};
    for (auto idx = 0; idx < count; idx++) {
      disp -= 16;
      auto const& pp = code[i + idx].pushp_;
      v << storepair{pp.s1, pp.s0, Vptr128{rsp(), disp}};
    }
    return count;
  });
}

#define DEFINE_STORE_UPDATE_SIMPLIFY(name, pre, post, reg, size, ptr_field)    \
  static bool fold_pre_##name(Env& env, const name& inst, Vlabel b, size_t i) {\
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (Vreg(inst.s) == Vreg(base)) return false;                              \
    return foldPreUpdateImpl(env, b, i, base, size, 1,                         \
      [&](Vout& v, int64_t off) {                                              \
        v << pre{Immed(static_cast<int32_t>(off)), base, base, inst.s};        \
      });                                                                      \
  }                                                                            \
  static bool fold_post_##name(Env& env, const name& inst, Vlabel b, size_t i) {\
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (Vreg(inst.s) == Vreg(base)) return false;                              \
    return foldPostUpdateImpl(env, b, i, base, size, 1,                        \
      [&](Vout& v, int64_t off) {                                              \
        v << post{Immed(static_cast<int32_t>(off)), base, base, inst.s};       \
      });                                                                      \
  }

VASM_STORE_UPDATE_SINGLE_LIST(DEFINE_STORE_UPDATE_SIMPLIFY)
#undef DEFINE_STORE_UPDATE_SIMPLIFY

#define DEFINE_STORE_PAIR_UPDATE_SIMPLIFY(name, pre, post, reg, size, lanes, ptr_field) \
  static bool fold_pre_##name(Env& env, const name& inst, Vlabel b, size_t i) {    \
    auto const& addr = inst.ptr_field;                                            \
    auto const base = addr.base;                                                  \
    if (!isSimpleAddress(addr, base)) return false;                               \
    if (!canFoldPairUpdate(inst.s0, inst.s1)) return false;                       \
    if (Vreg(inst.s0) == Vreg(base) || Vreg(inst.s1) == Vreg(base)) return false; \
    return foldPreUpdateImpl(env, b, i, base, size, lanes,                        \
      [&](Vout& v, int64_t off) {                                                 \
        v << pre{Immed(static_cast<int32_t>(off)), base, base, inst.s0, inst.s1}; \
      });                                                                         \
  }                                                                               \
  static bool fold_post_##name(Env& env, const name& inst, Vlabel b, size_t i) {  \
    auto const& addr = inst.ptr_field;                                            \
    auto const base = addr.base;                                                  \
    if (!isSimpleAddress(addr, base)) return false;                               \
    if (!canFoldPairUpdate(inst.s0, inst.s1)) return false;                       \
    if (Vreg(inst.s0) == Vreg(base) || Vreg(inst.s1) == Vreg(base)) return false; \
    return foldPostUpdateImpl(env, b, i, base, size, lanes,                       \
      [&](Vout& v, int64_t off) {                                                 \
        v << post{Immed(static_cast<int32_t>(off)), base, base, inst.s0, inst.s1};\
      });                                                                         \
  }

VASM_STORE_UPDATE_PAIR_LIST(DEFINE_STORE_PAIR_UPDATE_SIMPLIFY)
#undef DEFINE_STORE_PAIR_UPDATE_SIMPLIFY

#define DEFINE_LOAD_UPDATE_SIMPLIFY(name, pre, post, reg, size, ptr_field)      \
  static bool fold_pre_##name(Env& env, const name& inst, Vlabel b, size_t i) { \
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (Vreg(inst.d) == Vreg(base)) return false;                              \
    return foldPreUpdateImpl(env, b, i, base, size, 1,                         \
      [&](Vout& v, int64_t off) {                                              \
        v << pre{Immed(static_cast<int32_t>(off)), base, base, inst.d};        \
      });                                                                      \
  }                                                                            \
  static bool fold_post_##name(Env& env, const name& inst, Vlabel b, size_t i) {\
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (Vreg(inst.d) == Vreg(base)) return false;                              \
    return foldPostUpdateImpl(env, b, i, base, size, 1,                        \
      [&](Vout& v, int64_t off) {                                              \
        v << post{Immed(static_cast<int32_t>(off)), base, base, inst.d};       \
      });                                                                      \
  }

VASM_LOAD_UPDATE_SINGLE_LIST(DEFINE_LOAD_UPDATE_SIMPLIFY)
#undef DEFINE_LOAD_UPDATE_SIMPLIFY

#define DEFINE_LOAD_PAIR_UPDATE_SIMPLIFY(name, pre, post, reg, size, lanes, ptr_field) \
  static bool fold_pre_##name(Env& env, const name& inst, Vlabel b, size_t i) {  \
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (!canFoldPairUpdate(inst.d0, inst.d1)) return false;                    \
    if (Vreg(inst.d0) == Vreg(base) || Vreg(inst.d1) == Vreg(base)) return false;\
    return foldPreUpdateImpl(env, b, i, base, size, lanes,                     \
      [&](Vout& v, int64_t off) {                                              \
        v << pre{Immed(static_cast<int32_t>(off)), base, base, inst.d0, inst.d1};\
      });                                                                      \
  }                                                                            \
  static bool fold_post_##name(Env& env, const name& inst, Vlabel b, size_t i) {\
    auto const& addr = inst.ptr_field;                                         \
    auto const base = addr.base;                                               \
    if (!isSimpleAddress(addr, base)) return false;                            \
    if (!canFoldPairUpdate(inst.d0, inst.d1)) return false;                    \
    if (Vreg(inst.d0) == Vreg(base) || Vreg(inst.d1) == Vreg(base)) return false;\
    return foldPostUpdateImpl(env, b, i, base, size, lanes,                    \
      [&](Vout& v, int64_t off) {                                              \
        v << post{Immed(static_cast<int32_t>(off)), base, base, inst.d0, inst.d1};\
      });                                                                      \
  }

VASM_LOAD_UPDATE_PAIR_LIST(DEFINE_LOAD_PAIR_UPDATE_SIMPLIFY)
#undef DEFINE_LOAD_PAIR_UPDATE_SIMPLIFY

#define DEFINE_STORE_PS(name, pre, post, reg, size, ptr_field)                \
bool psimplify(Env& env, const name& inst, Vlabel b, size_t i) {              \
  if (fold_pre_##name(env, inst, b, i)) return true;                          \
  if (fold_post_##name(env, inst, b, i)) return true;                         \
  return simplify(env, inst, b, i);                                           \
}

VASM_STORE_UPDATE_SINGLE_LIST(DEFINE_STORE_PS)
#undef DEFINE_STORE_PS

#define DEFINE_STORE_PAIR_PS(name, pre, post, reg, size, lanes, ptr_field)    \
bool psimplify(Env& env, const name& inst, Vlabel b, size_t i) {              \
  if (fold_pre_##name(env, inst, b, i)) return true;                          \
  if (fold_post_##name(env, inst, b, i)) return true;                         \
  return simplify(env, inst, b, i);                                           \
}

VASM_STORE_UPDATE_PAIR_LIST(DEFINE_STORE_PAIR_PS)
#undef DEFINE_STORE_PAIR_PS

#define DEFINE_LOAD_PS(name, pre, post, reg, size, ptr_field)                 \
bool psimplify(Env& env, const name& inst, Vlabel b, size_t i) {              \
  if (fold_pre_##name(env, inst, b, i)) return true;                          \
  if (fold_post_##name(env, inst, b, i)) return true;                         \
  return simplify(env, inst, b, i);                                           \
}

VASM_LOAD_UPDATE_SINGLE_LIST(DEFINE_LOAD_PS)
#undef DEFINE_LOAD_PS

#define DEFINE_LOAD_PAIR_PS(name, pre, post, reg, size, lanes, ptr_field)     \
bool psimplify(Env& env, const name& inst, Vlabel b, size_t i) {              \
  if (fold_pre_##name(env, inst, b, i)) return true;                          \
  if (fold_post_##name(env, inst, b, i)) return true;                         \
  return simplify(env, inst, b, i);                                           \
}

VASM_LOAD_UPDATE_PAIR_LIST(DEFINE_LOAD_PAIR_PS)
#undef DEFINE_LOAD_PAIR_PS

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

bool psimplify(Env& env, const Abi& /*abi*/, Vlabel b, size_t i) {
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
