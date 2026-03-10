// Copyright 2019, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "macro-assembler-aarch64.h"

namespace vixl {
namespace aarch64 {

void MacroAssembler::AddSubHelper(AddSubHelperOption option,
                                  const ZRegister& zd,
                                  const ZRegister& zn,
                                  IntegerOperand imm) {
  VIXL_ASSERT(imm.FitsInLane(zd));

  // Simple, encodable cases.
  if (TrySingleAddSub(option, zd, zn, imm)) return;

  VIXL_ASSERT((option == kAddImmediate) || (option == kSubImmediate));
  bool add_imm = (option == kAddImmediate);

  // Try to translate Add(..., -imm) to Sub(..., imm) if we can encode it in one
  // instruction. Also interpret the immediate as signed, so we can convert
  // Add(zd.VnH(), zn.VnH(), 0xffff...) to Sub(..., 1), etc.
  IntegerOperand signed_imm(imm.AsIntN(zd.GetLaneSizeInBits()));
  if (signed_imm.IsNegative()) {
    AddSubHelperOption n_option = add_imm ? kSubImmediate : kAddImmediate;
    IntegerOperand n_imm(signed_imm.GetMagnitude());
    // IntegerOperand can represent -INT_MIN, so this is always safe.
    VIXL_ASSERT(n_imm.IsPositiveOrZero());
    if (TrySingleAddSub(n_option, zd, zn, n_imm)) return;
  }

  // Otherwise, fall back to dup + ADD_z_z/SUB_z_z.
  UseScratchRegisterScope temps(this);
  ZRegister scratch = temps.AcquireZ().WithLaneSize(zn.GetLaneSizeInBits());
  Dup(scratch, imm);

  SingleEmissionCheckScope guard(this);
  if (add_imm) {
    add(zd, zn, scratch);
  } else {
    sub(zd, zn, scratch);
  }
}

bool MacroAssembler::TrySingleAddSub(AddSubHelperOption option,
                                     const ZRegister& zd,
                                     const ZRegister& zn,
                                     IntegerOperand imm) {
  VIXL_ASSERT(imm.FitsInLane(zd));

  int imm8;
  int shift = -1;
  if (imm.TryEncodeAsShiftedUintNForLane<8, 0>(zd, &imm8, &shift) ||
      imm.TryEncodeAsShiftedUintNForLane<8, 8>(zd, &imm8, &shift)) {
    MovprfxHelperScope guard(this, zd, zn);
    switch (option) {
      case kAddImmediate:
        add(zd, zd, imm8, shift);
        return true;
      case kSubImmediate:
        sub(zd, zd, imm8, shift);
        return true;
    }
  }
  return false;
}

void MacroAssembler::IntWideImmHelper(IntArithImmFn imm_fn,
                                      SVEArithPredicatedFn reg_macro,
                                      const ZRegister& zd,
                                      const ZRegister& zn,
                                      IntegerOperand imm,
                                      bool is_signed) {
  if (is_signed) {
    // E.g. MUL_z_zi, SMIN_z_zi, SMAX_z_zi
    if (imm.IsInt8()) {
      MovprfxHelperScope guard(this, zd, zn);
      (this->*imm_fn)(zd, zd, imm.AsInt8());
      return;
    }
  } else {
    // E.g. UMIN_z_zi, UMAX_z_zi
    if (imm.IsUint8()) {
      MovprfxHelperScope guard(this, zd, zn);
      (this->*imm_fn)(zd, zd, imm.AsUint8());
      return;
    }
  }

  UseScratchRegisterScope temps(this);
  PRegister pg = temps.AcquireGoverningP();
  Ptrue(pg.WithSameLaneSizeAs(zd));

  // Try to re-use zd if we can, so we can avoid a movprfx.
  ZRegister scratch =
      zd.Aliases(zn) ? temps.AcquireZ().WithLaneSize(zn.GetLaneSizeInBits())
                     : zd;
  Dup(scratch, imm);

  // The vector-form macro for commutative operations will swap the arguments to
  // avoid movprfx, if necessary.
  (this->*reg_macro)(zd, pg.Merging(), zn, scratch);
}

void MacroAssembler::Mul(const ZRegister& zd,
                         const ZRegister& zn,
                         IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  IntArithImmFn imm_fn = &Assembler::mul;
  SVEArithPredicatedFn reg_fn = &MacroAssembler::Mul;
  IntWideImmHelper(imm_fn, reg_fn, zd, zn, imm, true);
}

void MacroAssembler::Smin(const ZRegister& zd,
                          const ZRegister& zn,
                          IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInSignedLane(zd));
  IntArithImmFn imm_fn = &Assembler::smin;
  SVEArithPredicatedFn reg_fn = &MacroAssembler::Smin;
  IntWideImmHelper(imm_fn, reg_fn, zd, zn, imm, true);
}

void MacroAssembler::Smax(const ZRegister& zd,
                          const ZRegister& zn,
                          IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInSignedLane(zd));
  IntArithImmFn imm_fn = &Assembler::smax;
  SVEArithPredicatedFn reg_fn = &MacroAssembler::Smax;
  IntWideImmHelper(imm_fn, reg_fn, zd, zn, imm, true);
}

void MacroAssembler::Umax(const ZRegister& zd,
                          const ZRegister& zn,
                          IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInUnsignedLane(zd));
  IntArithImmFn imm_fn = &Assembler::umax;
  SVEArithPredicatedFn reg_fn = &MacroAssembler::Umax;
  IntWideImmHelper(imm_fn, reg_fn, zd, zn, imm, false);
}

void MacroAssembler::Umin(const ZRegister& zd,
                          const ZRegister& zn,
                          IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInUnsignedLane(zd));
  IntArithImmFn imm_fn = &Assembler::umin;
  SVEArithPredicatedFn reg_fn = &MacroAssembler::Umin;
  IntWideImmHelper(imm_fn, reg_fn, zd, zn, imm, false);
}

void MacroAssembler::Addpl(const Register& xd,
                           const Register& xn,
                           int64_t multiplier) {
  VIXL_ASSERT(allow_macro_instructions_);

  // This macro relies on `Rdvl` to handle some out-of-range cases. Check that
  // `VL * multiplier` cannot overflow, for any possible value of VL.
  VIXL_ASSERT(multiplier <= (INT64_MAX / kZRegMaxSizeInBytes));
  VIXL_ASSERT(multiplier >= (INT64_MIN / kZRegMaxSizeInBytes));

  if (xd.IsZero()) return;
  if (xn.IsZero() && xd.IsSP()) {
    // TODO: This operation doesn't make much sense, but we could support it
    // with a scratch register if necessary.
    VIXL_UNIMPLEMENTED();
  }

  // Handling xzr requires an extra move, so defer it until later so we can try
  // to use `rdvl` instead (via `Addvl`).
  if (IsInt6(multiplier) && !xn.IsZero()) {
    SingleEmissionCheckScope guard(this);
    addpl(xd, xn, static_cast<int>(multiplier));
    return;
  }

  // If `multiplier` is a multiple of 8, we can use `Addvl` instead.
  if ((multiplier % kZRegBitsPerPRegBit) == 0) {
    Addvl(xd, xn, multiplier / kZRegBitsPerPRegBit);
    return;
  }

  if (IsInt6(multiplier)) {
    VIXL_ASSERT(xn.IsZero());  // Other cases were handled with `addpl`.
    // There is no simple `rdpl` instruction, and `addpl` cannot accept xzr, so
    // materialise a zero.
    MacroEmissionCheckScope guard(this);
    movz(xd, 0);
    addpl(xd, xd, static_cast<int>(multiplier));
    return;
  }

  // TODO: Some probable cases result in rather long sequences. For example,
  // `Addpl(sp, sp, 33)` requires five instructions, even though it's only just
  // outside the encodable range. We should look for ways to cover such cases
  // without drastically increasing the complexity of this logic.

  // For other cases, calculate xn + (PL * multiplier) using discrete
  // instructions. This requires two scratch registers in the general case, so
  // try to re-use the destination as a scratch register.
  UseScratchRegisterScope temps(this);
  temps.Include(xd);
  temps.Exclude(xn);

  Register scratch = temps.AcquireX();
  // Because there is no `rdpl`, so we have to calculate PL from VL. We can't
  // scale the multiplier because (we already know) it isn't a multiple of 8.
  Rdvl(scratch, multiplier);

  MacroEmissionCheckScope guard(this);
  if (xn.IsZero()) {
    asr(xd, scratch, kZRegBitsPerPRegBitLog2);
  } else if (xd.IsSP() || xn.IsSP()) {
    // TODO: MacroAssembler::Add should be able to handle this.
    asr(scratch, scratch, kZRegBitsPerPRegBitLog2);
    add(xd, xn, scratch);
  } else {
    add(xd, xn, Operand(scratch, ASR, kZRegBitsPerPRegBitLog2));
  }
}

void MacroAssembler::Addvl(const Register& xd,
                           const Register& xn,
                           int64_t multiplier) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(xd.IsX());
  VIXL_ASSERT(xn.IsX());

  // Check that `VL * multiplier` cannot overflow, for any possible value of VL.
  VIXL_ASSERT(multiplier <= (INT64_MAX / kZRegMaxSizeInBytes));
  VIXL_ASSERT(multiplier >= (INT64_MIN / kZRegMaxSizeInBytes));

  if (xd.IsZero()) return;
  if (xn.IsZero() && xd.IsSP()) {
    // TODO: This operation doesn't make much sense, but we could support it
    // with a scratch register if necessary. `rdvl` cannot write into `sp`.
    VIXL_UNIMPLEMENTED();
  }

  if (IsInt6(multiplier)) {
    SingleEmissionCheckScope guard(this);
    if (xn.IsZero()) {
      rdvl(xd, static_cast<int>(multiplier));
    } else {
      addvl(xd, xn, static_cast<int>(multiplier));
    }
    return;
  }

  // TODO: Some probable cases result in rather long sequences. For example,
  // `Addvl(sp, sp, 42)` requires four instructions, even though it's only just
  // outside the encodable range. We should look for ways to cover such cases
  // without drastically increasing the complexity of this logic.

  // For other cases, calculate xn + (VL * multiplier) using discrete
  // instructions. This requires two scratch registers in the general case, so
  // we try to re-use the destination as a scratch register.
  UseScratchRegisterScope temps(this);
  temps.Include(xd);
  temps.Exclude(xn);

  Register a = temps.AcquireX();
  Mov(a, multiplier);

  MacroEmissionCheckScope guard(this);
  Register b = temps.AcquireX();
  rdvl(b, 1);
  if (xn.IsZero()) {
    mul(xd, a, b);
  } else if (xd.IsSP() || xn.IsSP()) {
    mul(a, a, b);
    add(xd, xn, a);
  } else {
    madd(xd, a, b, xn);
  }
}

void MacroAssembler::CalculateSVEAddress(const Register& xd,
                                         const SVEMemOperand& addr,
                                         int vl_divisor_log2) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!addr.IsScatterGather());
  VIXL_ASSERT(xd.IsX());

  // The lower bound is where a whole Z register is accessed.
  VIXL_ASSERT(!addr.IsMulVl() || (vl_divisor_log2 >= 0));
  // The upper bound is for P register accesses, and for instructions like
  // "st1b { z0.d } [...]", where one byte is accessed for every D-sized lane.
  VIXL_ASSERT(vl_divisor_log2 <= static_cast<int>(kZRegBitsPerPRegBitLog2));

  SVEOffsetModifier mod = addr.GetOffsetModifier();
  Register base = addr.GetScalarBase();

  if (addr.IsEquivalentToScalar()) {
    // For example:
    //   [x0]
    //   [x0, #0]
    //   [x0, xzr, LSL 2]
    Mov(xd, base);
  } else if (addr.IsScalarPlusImmediate()) {
    // For example:
    //   [x0, #42]
    //   [x0, #42, MUL VL]
    int64_t offset = addr.GetImmediateOffset();
    VIXL_ASSERT(offset != 0);  // Handled by IsEquivalentToScalar.
    if (addr.IsMulVl()) {
      int vl_divisor = 1 << vl_divisor_log2;
      // For all possible values of vl_divisor, we can simply use `Addpl`. This
      // will select `addvl` if necessary.
      VIXL_ASSERT((kZRegBitsPerPRegBit % vl_divisor) == 0);
      Addpl(xd, base, offset * (kZRegBitsPerPRegBit / vl_divisor));
    } else {
      // IsScalarPlusImmediate() ensures that no other modifiers can occur.
      VIXL_ASSERT(mod == NO_SVE_OFFSET_MODIFIER);
      Add(xd, base, offset);
    }
  } else if (addr.IsScalarPlusScalar()) {
    // For example:
    //   [x0, x1]
    //   [x0, x1, LSL #4]
    Register offset = addr.GetScalarOffset();
    VIXL_ASSERT(!offset.IsZero());  // Handled by IsEquivalentToScalar.
    if (mod == SVE_LSL) {
      Add(xd, base, Operand(offset, LSL, addr.GetShiftAmount()));
    } else {
      // IsScalarPlusScalar() ensures that no other modifiers can occur.
      VIXL_ASSERT(mod == NO_SVE_OFFSET_MODIFIER);
      Add(xd, base, offset);
    }
  } else {
    // All other forms are scatter-gather addresses, which cannot be evaluated
    // into an X register.
    VIXL_UNREACHABLE();
  }
}

void MacroAssembler::Cpy(const ZRegister& zd,
                         const PRegister& pg,
                         IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInLane(zd));
  int imm8;
  int shift;
  if (imm.TryEncodeAsShiftedIntNForLane<8, 0>(zd, &imm8, &shift) ||
      imm.TryEncodeAsShiftedIntNForLane<8, 8>(zd, &imm8, &shift)) {
    SingleEmissionCheckScope guard(this);
    cpy(zd, pg, imm8, shift);
    return;
  }

  // The fallbacks rely on `cpy` variants that only support merging predication.
  // If zeroing predication was requested, zero the destination first.
  if (pg.IsZeroing()) {
    SingleEmissionCheckScope guard(this);
    dup(zd, 0);
  }
  PRegisterM pg_m = pg.Merging();

  // Try to encode the immediate using fcpy.
  VIXL_ASSERT(imm.FitsInLane(zd));
  if (zd.GetLaneSizeInBits() >= kHRegSize) {
    double fp_imm = 0.0;
    switch (zd.GetLaneSizeInBits()) {
      case kHRegSize:
        fp_imm =
            FPToDouble(RawbitsToFloat16(imm.AsUint16()), kIgnoreDefaultNaN);
        break;
      case kSRegSize:
        fp_imm = RawbitsToFloat(imm.AsUint32());
        break;
      case kDRegSize:
        fp_imm = RawbitsToDouble(imm.AsUint64());
        break;
      default:
        VIXL_UNREACHABLE();
        break;
    }
    // IsImmFP64 is equivalent to IsImmFP<n> for the same arithmetic value, so
    // we can use IsImmFP64 for all lane sizes.
    if (IsImmFP64(fp_imm)) {
      SingleEmissionCheckScope guard(this);
      fcpy(zd, pg_m, fp_imm);
      return;
    }
  }

  // Fall back to using a scratch register.
  UseScratchRegisterScope temps(this);
  Register scratch = temps.AcquireRegisterToHoldLane(zd);
  Mov(scratch, imm);

  SingleEmissionCheckScope guard(this);
  cpy(zd, pg_m, scratch);
}

// TODO: We implement Fcpy (amongst other things) for all FP types because it
// allows us to preserve user-specified NaNs. We should come up with some
// FPImmediate type to abstract this, and avoid all the duplication below (and
// elsewhere).

void MacroAssembler::Fcpy(const ZRegister& zd,
                          const PRegisterM& pg,
                          double imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(pg.IsMerging());

  if (IsImmFP64(imm)) {
    SingleEmissionCheckScope guard(this);
    fcpy(zd, pg, imm);
    return;
  }

  // As a fall-back, cast the immediate to the required lane size, and try to
  // encode the bit pattern using `Cpy`.
  Cpy(zd, pg, FPToRawbitsWithSize(zd.GetLaneSizeInBits(), imm));
}

void MacroAssembler::Fcpy(const ZRegister& zd,
                          const PRegisterM& pg,
                          float imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(pg.IsMerging());

  if (IsImmFP32(imm)) {
    SingleEmissionCheckScope guard(this);
    fcpy(zd, pg, imm);
    return;
  }

  // As a fall-back, cast the immediate to the required lane size, and try to
  // encode the bit pattern using `Cpy`.
  Cpy(zd, pg, FPToRawbitsWithSize(zd.GetLaneSizeInBits(), imm));
}

void MacroAssembler::Fcpy(const ZRegister& zd,
                          const PRegisterM& pg,
                          Float16 imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(pg.IsMerging());

  if (IsImmFP16(imm)) {
    SingleEmissionCheckScope guard(this);
    fcpy(zd, pg, imm);
    return;
  }

  // As a fall-back, cast the immediate to the required lane size, and try to
  // encode the bit pattern using `Cpy`.
  Cpy(zd, pg, FPToRawbitsWithSize(zd.GetLaneSizeInBits(), imm));
}

void MacroAssembler::Dup(const ZRegister& zd, IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInLane(zd));
  unsigned lane_size = zd.GetLaneSizeInBits();
  int imm8;
  int shift;
  if (imm.TryEncodeAsShiftedIntNForLane<8, 0>(zd, &imm8, &shift) ||
      imm.TryEncodeAsShiftedIntNForLane<8, 8>(zd, &imm8, &shift)) {
    SingleEmissionCheckScope guard(this);
    dup(zd, imm8, shift);
  } else if (IsImmLogical(imm.AsUintN(lane_size), lane_size)) {
    SingleEmissionCheckScope guard(this);
    dupm(zd, imm.AsUintN(lane_size));
  } else {
    UseScratchRegisterScope temps(this);
    Register scratch = temps.AcquireRegisterToHoldLane(zd);
    Mov(scratch, imm);

    SingleEmissionCheckScope guard(this);
    dup(zd, scratch);
  }
}

void MacroAssembler::NoncommutativeArithmeticHelper(
    const ZRegister& zd,
    const PRegisterM& pg,
    const ZRegister& zn,
    const ZRegister& zm,
    SVEArithPredicatedFn fn,
    SVEArithPredicatedFn rev_fn) {
  if (zd.Aliases(zn)) {
    // E.g. zd = zd / zm
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zd, pg, zn, zm);
  } else if (zd.Aliases(zm)) {
    // E.g. zd = zn / zd
    SingleEmissionCheckScope guard(this);
    (this->*rev_fn)(zd, pg, zm, zn);
  } else {
    // E.g. zd = zn / zm
    MovprfxHelperScope guard(this, zd, pg, zn);
    (this->*fn)(zd, pg, zd, zm);
  }
}

void MacroAssembler::FPCommutativeArithmeticHelper(
    const ZRegister& zd,
    const PRegisterM& pg,
    const ZRegister& zn,
    const ZRegister& zm,
    SVEArithPredicatedFn fn,
    FPMacroNaNPropagationOption nan_option) {
  ResolveFPNaNPropagationOption(&nan_option);

  if (zd.Aliases(zn)) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zd, pg, zd, zm);
  } else if (zd.Aliases(zm)) {
    switch (nan_option) {
      case FastNaNPropagation: {
        // Swap the arguments.
        SingleEmissionCheckScope guard(this);
        (this->*fn)(zd, pg, zd, zn);
        return;
      }
      case StrictNaNPropagation: {
        UseScratchRegisterScope temps(this);
        // Use a scratch register to keep the argument order exactly as
        // specified.
        ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zn);
        {
          MovprfxHelperScope guard(this, scratch, pg, zn);
          (this->*fn)(scratch, pg, scratch, zm);
        }
        Mov(zd, scratch);
        return;
      }
      case NoFPMacroNaNPropagationSelected:
        VIXL_UNREACHABLE();
        return;
    }
  } else {
    MovprfxHelperScope guard(this, zd, pg, zn);
    (this->*fn)(zd, pg, zd, zm);
  }
}

// Instructions of the form "inst zda, zn, zm, #num", where they are
// non-commutative and no reversed form is provided.
#define VIXL_SVE_NONCOMM_ARITH_ZZZZI_LIST(V) \
  V(Cmla, cmla)                              \
  V(Sqrdcmlah, sqrdcmlah)

#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN)                     \
  void MacroAssembler::MASMFN(const ZRegister& zd,               \
                              const ZRegister& za,               \
                              const ZRegister& zn,               \
                              const ZRegister& zm,               \
                              int imm) {                         \
    if ((zd.Aliases(zn) || zd.Aliases(zm)) && !zd.Aliases(za)) { \
      UseScratchRegisterScope temps(this);                       \
      VIXL_ASSERT(AreSameLaneSize(zn, zm));                      \
      ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zn);  \
      Mov(ztmp, zd.Aliases(zn) ? zn : zm);                       \
      MovprfxHelperScope guard(this, zd, za);                    \
      ASMFN(zd,                                                  \
            (zd.Aliases(zn) ? ztmp : zn),                        \
            (zd.Aliases(zm) ? ztmp : zm),                        \
            imm);                                                \
    } else {                                                     \
      MovprfxHelperScope guard(this, zd, za);                    \
      ASMFN(zd, zn, zm, imm);                                    \
    }                                                            \
  }
VIXL_SVE_NONCOMM_ARITH_ZZZZI_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

// Instructions of the form "inst zda, zn, zm, #num, #num", where they are
// non-commutative and no reversed form is provided.
#define VIXL_SVE_NONCOMM_ARITH_ZZZZII_LIST(V) \
  V(Cmla, cmla)                               \
  V(Sqrdcmlah, sqrdcmlah)

// This doesn't handle zm when it's out of the range that can be encoded in
// instruction. The range depends on element size: z0-z7 for H, z0-15 for S.
#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN)                     \
  void MacroAssembler::MASMFN(const ZRegister& zd,               \
                              const ZRegister& za,               \
                              const ZRegister& zn,               \
                              const ZRegister& zm,               \
                              int index,                         \
                              int rot) {                         \
    if ((zd.Aliases(zn) || zd.Aliases(zm)) && !zd.Aliases(za)) { \
      UseScratchRegisterScope temps(this);                       \
      ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zd);  \
      {                                                          \
        MovprfxHelperScope guard(this, ztmp, za);                \
        ASMFN(ztmp, zn, zm, index, rot);                         \
      }                                                          \
      Mov(zd, ztmp);                                             \
    } else {                                                     \
      MovprfxHelperScope guard(this, zd, za);                    \
      ASMFN(zd, zn, zm, index, rot);                             \
    }                                                            \
  }
VIXL_SVE_NONCOMM_ARITH_ZZZZII_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

// Instructions of the form "inst zda, pg, zda, zn", where they are
// non-commutative and no reversed form is provided.
#define VIXL_SVE_NONCOMM_ARITH_ZPZZ_LIST(V) \
  V(Addp, addp)                             \
  V(Bic, bic)                               \
  V(Faddp, faddp)                           \
  V(Fmaxnmp, fmaxnmp)                       \
  V(Fminnmp, fminnmp)                       \
  V(Fmaxp, fmaxp)                           \
  V(Fminp, fminp)                           \
  V(Fscale, fscale)                         \
  V(Smaxp, smaxp)                           \
  V(Sminp, sminp)                           \
  V(Suqadd, suqadd)                         \
  V(Umaxp, umaxp)                           \
  V(Uminp, uminp)                           \
  V(Usqadd, usqadd)

#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN)                       \
  void MacroAssembler::MASMFN(const ZRegister& zd,                 \
                              const PRegisterM& pg,                \
                              const ZRegister& zn,                 \
                              const ZRegister& zm) {               \
    VIXL_ASSERT(allow_macro_instructions_);                        \
    if (zd.Aliases(zm) && !zd.Aliases(zn)) {                       \
      UseScratchRegisterScope temps(this);                         \
      ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zm); \
      Mov(scratch, zm);                                            \
      MovprfxHelperScope guard(this, zd, pg, zn);                  \
      ASMFN(zd, pg, zd, scratch);                                  \
    } else {                                                       \
      MovprfxHelperScope guard(this, zd, pg, zn);                  \
      ASMFN(zd, pg, zd, zm);                                       \
    }                                                              \
  }
VIXL_SVE_NONCOMM_ARITH_ZPZZ_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

// Instructions of the form "inst zda, pg, zda, zn", where they are
// non-commutative and a reversed form is provided.
#define VIXL_SVE_NONCOMM_ARITH_REVERSE_ZPZZ_LIST(V) \
  V(Asr, asr)                                       \
  V(Fdiv, fdiv)                                     \
  V(Fsub, fsub)                                     \
  V(Lsl, lsl)                                       \
  V(Lsr, lsr)                                       \
  V(Sdiv, sdiv)                                     \
  V(Shsub, shsub)                                   \
  V(Sqrshl, sqrshl)                                 \
  V(Sqshl, sqshl)                                   \
  V(Sqsub, sqsub)                                   \
  V(Srshl, srshl)                                   \
  V(Sub, sub)                                       \
  V(Udiv, udiv)                                     \
  V(Uhsub, uhsub)                                   \
  V(Uqrshl, uqrshl)                                 \
  V(Uqshl, uqshl)                                   \
  V(Uqsub, uqsub)                                   \
  V(Urshl, urshl)

#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN)                          \
  void MacroAssembler::MASMFN(const ZRegister& zd,                    \
                              const PRegisterM& pg,                   \
                              const ZRegister& zn,                    \
                              const ZRegister& zm) {                  \
    VIXL_ASSERT(allow_macro_instructions_);                           \
    NoncommutativeArithmeticHelper(zd,                                \
                                   pg,                                \
                                   zn,                                \
                                   zm,                                \
                                   static_cast<SVEArithPredicatedFn>( \
                                       &Assembler::ASMFN),            \
                                   static_cast<SVEArithPredicatedFn>( \
                                       &Assembler::ASMFN##r));        \
  }
VIXL_SVE_NONCOMM_ARITH_REVERSE_ZPZZ_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

void MacroAssembler::Fadd(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fadd),
                                nan_option);
}

void MacroAssembler::Fabd(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fabd),
                                nan_option);
}

void MacroAssembler::Fmul(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fmul),
                                nan_option);
}

void MacroAssembler::Fmulx(const ZRegister& zd,
                           const PRegisterM& pg,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fmulx),
                                nan_option);
}

void MacroAssembler::Fmax(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fmax),
                                nan_option);
}

void MacroAssembler::Fmin(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fmin),
                                nan_option);
}

void MacroAssembler::Fmaxnm(const ZRegister& zd,
                            const PRegisterM& pg,
                            const ZRegister& zn,
                            const ZRegister& zm,
                            FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fmaxnm),
                                nan_option);
}

void MacroAssembler::Fminnm(const ZRegister& zd,
                            const PRegisterM& pg,
                            const ZRegister& zn,
                            const ZRegister& zm,
                            FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPCommutativeArithmeticHelper(zd,
                                pg,
                                zn,
                                zm,
                                static_cast<SVEArithPredicatedFn>(
                                    &Assembler::fminnm),
                                nan_option);
}

void MacroAssembler::Fdup(const ZRegister& zd, double imm) {
  VIXL_ASSERT(allow_macro_instructions_);

  switch (zd.GetLaneSizeInBits()) {
    case kHRegSize:
      Fdup(zd, Float16(imm));
      break;
    case kSRegSize:
      Fdup(zd, static_cast<float>(imm));
      break;
    case kDRegSize:
      uint64_t bits = DoubleToRawbits(imm);
      if (IsImmFP64(bits)) {
        SingleEmissionCheckScope guard(this);
        fdup(zd, imm);
      } else {
        Dup(zd, bits);
      }
      break;
  }
}

void MacroAssembler::Fdup(const ZRegister& zd, float imm) {
  VIXL_ASSERT(allow_macro_instructions_);

  switch (zd.GetLaneSizeInBits()) {
    case kHRegSize:
      Fdup(zd, Float16(imm));
      break;
    case kSRegSize:
      if (IsImmFP32(imm)) {
        SingleEmissionCheckScope guard(this);
        fdup(zd, imm);
      } else {
        Dup(zd, FloatToRawbits(imm));
      }
      break;
    case kDRegSize:
      Fdup(zd, static_cast<double>(imm));
      break;
  }
}

void MacroAssembler::Fdup(const ZRegister& zd, Float16 imm) {
  VIXL_ASSERT(allow_macro_instructions_);

  switch (zd.GetLaneSizeInBits()) {
    case kHRegSize:
      if (IsImmFP16(imm)) {
        SingleEmissionCheckScope guard(this);
        fdup(zd, imm);
      } else {
        Dup(zd, Float16ToRawbits(imm));
      }
      break;
    case kSRegSize:
      Fdup(zd, FPToFloat(imm, kIgnoreDefaultNaN));
      break;
    case kDRegSize:
      Fdup(zd, FPToDouble(imm, kIgnoreDefaultNaN));
      break;
  }
}

void MacroAssembler::Index(const ZRegister& zd,
                           const Operand& start,
                           const Operand& step) {
  class IndexOperand : public Operand {
   public:
    static IndexOperand Prepare(MacroAssembler* masm,
                                UseScratchRegisterScope* temps,
                                const Operand& op,
                                const ZRegister& zd_inner) {
      // Look for encodable immediates.
      int imm;
      if (op.IsImmediate()) {
        if (IntegerOperand(op).TryEncodeAsIntNForLane<5>(zd_inner, &imm)) {
          return IndexOperand(imm);
        }
        Register scratch = temps->AcquireRegisterToHoldLane(zd_inner);
        masm->Mov(scratch, op);
        return IndexOperand(scratch);
      } else {
        // Plain registers can be encoded directly.
        VIXL_ASSERT(op.IsPlainRegister());
        return IndexOperand(op.GetRegister());
      }
    }

    int GetImm5() const {
      int64_t imm = GetImmediate();
      VIXL_ASSERT(IsInt5(imm));
      return static_cast<int>(imm);
    }

   private:
    explicit IndexOperand(const Register& reg) : Operand(reg) {}
    explicit IndexOperand(int64_t imm) : Operand(imm) {}
  };

  UseScratchRegisterScope temps(this);
  IndexOperand start_enc = IndexOperand::Prepare(this, &temps, start, zd);
  IndexOperand step_enc = IndexOperand::Prepare(this, &temps, step, zd);

  SingleEmissionCheckScope guard(this);
  if (start_enc.IsImmediate()) {
    if (step_enc.IsImmediate()) {
      index(zd, start_enc.GetImm5(), step_enc.GetImm5());
    } else {
      index(zd, start_enc.GetImm5(), step_enc.GetRegister());
    }
  } else {
    if (step_enc.IsImmediate()) {
      index(zd, start_enc.GetRegister(), step_enc.GetImm5());
    } else {
      index(zd, start_enc.GetRegister(), step_enc.GetRegister());
    }
  }
}

void MacroAssembler::Insr(const ZRegister& zdn, IntegerOperand imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(imm.FitsInLane(zdn));

  if (imm.IsZero()) {
    SingleEmissionCheckScope guard(this);
    insr(zdn, xzr);
    return;
  }

  UseScratchRegisterScope temps(this);
  Register scratch = temps.AcquireRegisterToHoldLane(zdn);

  // TODO: There are many cases where we could optimise immediates, such as by
  // detecting repeating patterns or FP immediates. We should optimise and
  // abstract this for use in other SVE mov-immediate-like macros.
  Mov(scratch, imm);

  SingleEmissionCheckScope guard(this);
  insr(zdn, scratch);
}

void MacroAssembler::Mla(const ZRegister& zd,
                         const PRegisterM& pg,
                         const ZRegister& za,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(za)) {
    // zda = zda + (zn * zm)
    SingleEmissionCheckScope guard(this);
    mla(zd, pg, zn, zm);
  } else if (zd.Aliases(zn)) {
    // zdn = za + (zdn * zm)
    SingleEmissionCheckScope guard(this);
    mad(zd, pg, zm, za);
  } else if (zd.Aliases(zm)) {
    // Multiplication is commutative, so we can swap zn and zm.
    // zdm = za + (zdm * zn)
    SingleEmissionCheckScope guard(this);
    mad(zd, pg, zn, za);
  } else {
    // zd = za + (zn * zm)
    ExactAssemblyScope guard(this, 2 * kInstructionSize);
    movprfx(zd, pg, za);
    mla(zd, pg, zn, zm);
  }
}

void MacroAssembler::Mls(const ZRegister& zd,
                         const PRegisterM& pg,
                         const ZRegister& za,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(za)) {
    // zda = zda - (zn * zm)
    SingleEmissionCheckScope guard(this);
    mls(zd, pg, zn, zm);
  } else if (zd.Aliases(zn)) {
    // zdn = za - (zdn * zm)
    SingleEmissionCheckScope guard(this);
    msb(zd, pg, zm, za);
  } else if (zd.Aliases(zm)) {
    // Multiplication is commutative, so we can swap zn and zm.
    // zdm = za - (zdm * zn)
    SingleEmissionCheckScope guard(this);
    msb(zd, pg, zn, za);
  } else {
    // zd = za - (zn * zm)
    ExactAssemblyScope guard(this, 2 * kInstructionSize);
    movprfx(zd, pg, za);
    mls(zd, pg, zn, zm);
  }
}

void MacroAssembler::CompareHelper(Condition cond,
                                   const PRegisterWithLaneSize& pd,
                                   const PRegisterZ& pg,
                                   const ZRegister& zn,
                                   IntegerOperand imm) {
  UseScratchRegisterScope temps(this);
  ZRegister zm = temps.AcquireZ().WithLaneSize(zn.GetLaneSizeInBits());
  Dup(zm, imm);
  SingleEmissionCheckScope guard(this);
  cmp(cond, pd, pg, zn, zm);
}

void MacroAssembler::Pfirst(const PRegisterWithLaneSize& pd,
                            const PRegister& pg,
                            const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(pd.IsLaneSizeB());
  VIXL_ASSERT(pn.IsLaneSizeB());
  if (pd.Is(pn)) {
    SingleEmissionCheckScope guard(this);
    pfirst(pd, pg, pn);
  } else {
    UseScratchRegisterScope temps(this);
    PRegister temp_pg = pg;
    if (pd.Aliases(pg)) {
      temp_pg = temps.AcquireP();
      Mov(temp_pg.VnB(), pg.VnB());
    }
    Mov(pd, pn);
    SingleEmissionCheckScope guard(this);
    pfirst(pd, temp_pg, pd);
  }
}

void MacroAssembler::Pnext(const PRegisterWithLaneSize& pd,
                           const PRegister& pg,
                           const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(AreSameFormat(pd, pn));
  if (pd.Is(pn)) {
    SingleEmissionCheckScope guard(this);
    pnext(pd, pg, pn);
  } else {
    UseScratchRegisterScope temps(this);
    PRegister temp_pg = pg;
    if (pd.Aliases(pg)) {
      temp_pg = temps.AcquireP();
      Mov(temp_pg.VnB(), pg.VnB());
    }
    Mov(pd.VnB(), pn.VnB());
    SingleEmissionCheckScope guard(this);
    pnext(pd, temp_pg, pd);
  }
}

void MacroAssembler::Ptrue(const PRegisterWithLaneSize& pd,
                           SVEPredicateConstraint pattern,
                           FlagsUpdate s) {
  VIXL_ASSERT(allow_macro_instructions_);
  switch (s) {
    case LeaveFlags:
      Ptrue(pd, pattern);
      return;
    case SetFlags:
      Ptrues(pd, pattern);
      return;
  }
  VIXL_UNREACHABLE();
}

void MacroAssembler::Sub(const ZRegister& zd,
                         IntegerOperand imm,
                         const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);

  int imm8;
  int shift = -1;
  if (imm.TryEncodeAsShiftedUintNForLane<8, 0>(zd, &imm8, &shift) ||
      imm.TryEncodeAsShiftedUintNForLane<8, 8>(zd, &imm8, &shift)) {
    MovprfxHelperScope guard(this, zd, zm);
    subr(zd, zd, imm8, shift);
  } else {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithLaneSize(zm.GetLaneSizeInBits());
    Dup(scratch, imm);

    SingleEmissionCheckScope guard(this);
    sub(zd, scratch, zm);
  }
}

void MacroAssembler::SVELoadBroadcastImmHelper(const ZRegister& zt,
                                               const PRegisterZ& pg,
                                               const SVEMemOperand& addr,
                                               SVELoadBroadcastFn fn,
                                               int divisor) {
  VIXL_ASSERT(addr.IsScalarPlusImmediate());
  int64_t imm = addr.GetImmediateOffset();
  if ((imm % divisor == 0) && IsUint6(imm / divisor)) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
  } else {
    UseScratchRegisterScope temps(this);
    Register scratch = temps.AcquireX();
    CalculateSVEAddress(scratch, addr, zt);
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, SVEMemOperand(scratch));
  }
}

void MacroAssembler::SVELoadStoreScalarImmHelper(const CPURegister& rt,
                                                 const SVEMemOperand& addr,
                                                 SVELoadStoreFn fn) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(rt.IsZRegister() || rt.IsPRegister());

  if (addr.IsPlainScalar() ||
      (addr.IsScalarPlusImmediate() && IsInt9(addr.GetImmediateOffset()) &&
       addr.IsMulVl())) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(rt, addr);
    return;
  }

  if (addr.IsEquivalentToScalar()) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(rt, SVEMemOperand(addr.GetScalarBase()));
    return;
  }

  UseScratchRegisterScope temps(this);
  Register scratch = temps.AcquireX();
  CalculateSVEAddress(scratch, addr, rt);
  SingleEmissionCheckScope guard(this);
  (this->*fn)(rt, SVEMemOperand(scratch));
}

template <typename Tg, typename Tf>
void MacroAssembler::SVELoadStoreNTBroadcastQOHelper(
    const ZRegister& zt,
    const Tg& pg,
    const SVEMemOperand& addr,
    Tf fn,
    int imm_bits,
    int shift_amount,
    SVEOffsetModifier supported_modifier,
    int vl_divisor_log2) {
  VIXL_ASSERT(allow_macro_instructions_);
  int imm_divisor = 1 << shift_amount;

  if (addr.IsPlainScalar() ||
      (addr.IsScalarPlusImmediate() &&
       IsIntN(imm_bits, addr.GetImmediateOffset() / imm_divisor) &&
       ((addr.GetImmediateOffset() % imm_divisor) == 0) &&
       (addr.GetOffsetModifier() == supported_modifier))) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
    return;
  }

  if (addr.IsScalarPlusScalar() && !addr.GetScalarOffset().IsZero() &&
      addr.IsEquivalentToLSL(zt.GetLaneSizeInBytesLog2())) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
    return;
  }

  if (addr.IsEquivalentToScalar()) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, SVEMemOperand(addr.GetScalarBase()));
    return;
  }

  if (addr.IsMulVl() && (supported_modifier != SVE_MUL_VL) &&
      (vl_divisor_log2 == -1)) {
    // We don't handle [x0, #imm, MUL VL] if the in-memory access size is not VL
    // dependent.
    VIXL_UNIMPLEMENTED();
  }

  UseScratchRegisterScope temps(this);
  Register scratch = temps.AcquireX();
  CalculateSVEAddress(scratch, addr, vl_divisor_log2);
  SingleEmissionCheckScope guard(this);
  (this->*fn)(zt, pg, SVEMemOperand(scratch));
}

template <typename Tg, typename Tf>
void MacroAssembler::SVELoadStore1Helper(int msize_in_bytes_log2,
                                         const ZRegister& zt,
                                         const Tg& pg,
                                         const SVEMemOperand& addr,
                                         Tf fn) {
  if (addr.IsPlainScalar() ||
      (addr.IsScalarPlusScalar() && !addr.GetScalarOffset().IsZero() &&
       addr.IsEquivalentToLSL(msize_in_bytes_log2)) ||
      (addr.IsScalarPlusImmediate() && IsInt4(addr.GetImmediateOffset()) &&
       addr.IsMulVl())) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
    return;
  }

  if (addr.IsEquivalentToScalar()) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, SVEMemOperand(addr.GetScalarBase()));
    return;
  }

  if (addr.IsVectorPlusImmediate()) {
    uint64_t offset = addr.GetImmediateOffset();
    if (IsMultiple(offset, (1 << msize_in_bytes_log2)) &&
        IsUint5(offset >> msize_in_bytes_log2)) {
      SingleEmissionCheckScope guard(this);
      (this->*fn)(zt, pg, addr);
      return;
    }
  }

  if (addr.IsScalarPlusVector()) {
    VIXL_ASSERT(addr.IsScatterGather());
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
    return;
  }

  UseScratchRegisterScope temps(this);
  if (addr.IsScatterGather()) {
    // In scatter-gather modes, zt and zn/zm have the same lane size. However,
    // for 32-bit accesses, the result of each lane's address calculation still
    // requires 64 bits; we can't naively use `Adr` for the address calculation
    // because it would truncate each address to 32 bits.

    if (addr.IsVectorPlusImmediate()) {
      // Synthesise the immediate in an X register, then use a
      // scalar-plus-vector access with the original vector.
      Register scratch = temps.AcquireX();
      Mov(scratch, addr.GetImmediateOffset());
      SingleEmissionCheckScope guard(this);
      SVEOffsetModifier om =
          zt.IsLaneSizeS() ? SVE_UXTW : NO_SVE_OFFSET_MODIFIER;
      (this->*fn)(zt, pg, SVEMemOperand(scratch, addr.GetVectorBase(), om));
      return;
    }

    VIXL_UNIMPLEMENTED();
  } else {
    Register scratch = temps.AcquireX();
    // TODO: If we have an immediate offset that is a multiple of
    // msize_in_bytes, we can use Rdvl/Rdpl and a scalar-plus-scalar form to
    // save an instruction.
    int vl_divisor_log2 = zt.GetLaneSizeInBytesLog2() - msize_in_bytes_log2;
    CalculateSVEAddress(scratch, addr, vl_divisor_log2);
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, SVEMemOperand(scratch));
  }
}

template <typename Tf>
void MacroAssembler::SVELoadFFHelper(int msize_in_bytes_log2,
                                     const ZRegister& zt,
                                     const PRegisterZ& pg,
                                     const SVEMemOperand& addr,
                                     Tf fn) {
  if (addr.IsScatterGather()) {
    // Scatter-gather first-fault loads share encodings with normal loads.
    SVELoadStore1Helper(msize_in_bytes_log2, zt, pg, addr, fn);
    return;
  }

  // Contiguous first-faulting loads have no scalar-plus-immediate form at all,
  // so we don't do immediate synthesis.

  // We cannot currently distinguish "[x0]" from "[x0, #0]", and this
  // is not "scalar-plus-scalar", so we have to permit `IsPlainScalar()` here.
  if (addr.IsPlainScalar() || (addr.IsScalarPlusScalar() &&
                               addr.IsEquivalentToLSL(msize_in_bytes_log2))) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zt, pg, addr);
    return;
  }

  VIXL_UNIMPLEMENTED();
}

void MacroAssembler::Ld1b(const ZRegister& zt,
                          const PRegisterZ& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kBRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1b));
}

void MacroAssembler::Ld1h(const ZRegister& zt,
                          const PRegisterZ& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kHRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1h));
}

void MacroAssembler::Ld1w(const ZRegister& zt,
                          const PRegisterZ& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kWRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1w));
}

void MacroAssembler::Ld1d(const ZRegister& zt,
                          const PRegisterZ& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kDRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1d));
}

void MacroAssembler::Ld1sb(const ZRegister& zt,
                           const PRegisterZ& pg,
                           const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kBRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1sb));
}

void MacroAssembler::Ld1sh(const ZRegister& zt,
                           const PRegisterZ& pg,
                           const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kHRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1sh));
}

void MacroAssembler::Ld1sw(const ZRegister& zt,
                           const PRegisterZ& pg,
                           const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kSRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVELoad1Fn>(&Assembler::ld1sw));
}

void MacroAssembler::St1b(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kBRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVEStore1Fn>(&Assembler::st1b));
}

void MacroAssembler::St1h(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kHRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVEStore1Fn>(&Assembler::st1h));
}

void MacroAssembler::St1w(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kSRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVEStore1Fn>(&Assembler::st1w));
}

void MacroAssembler::St1d(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadStore1Helper(kDRegSizeInBytesLog2,
                      zt,
                      pg,
                      addr,
                      static_cast<SVEStore1Fn>(&Assembler::st1d));
}

void MacroAssembler::Ldff1b(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kBRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1b));
}

void MacroAssembler::Ldff1h(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kHRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1h));
}

void MacroAssembler::Ldff1w(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kSRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1w));
}

void MacroAssembler::Ldff1d(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kDRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1d));
}

void MacroAssembler::Ldff1sb(const ZRegister& zt,
                             const PRegisterZ& pg,
                             const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kBRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1sb));
}

void MacroAssembler::Ldff1sh(const ZRegister& zt,
                             const PRegisterZ& pg,
                             const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kHRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1sh));
}

void MacroAssembler::Ldff1sw(const ZRegister& zt,
                             const PRegisterZ& pg,
                             const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVELoadFFHelper(kSRegSizeInBytesLog2,
                  zt,
                  pg,
                  addr,
                  static_cast<SVELoad1Fn>(&Assembler::ldff1sw));
}

#define VIXL_SVE_LD1R_LIST(V) \
  V(qb, 4) V(qh, 4) V(qw, 4) V(qd, 4) V(ob, 5) V(oh, 5) V(ow, 5) V(od, 5)

#define VIXL_DEFINE_MASM_FUNC(SZ, SH)                          \
  void MacroAssembler::Ld1r##SZ(const ZRegister& zt,           \
                                const PRegisterZ& pg,          \
                                const SVEMemOperand& addr) {   \
    VIXL_ASSERT(allow_macro_instructions_);                    \
    SVELoadStoreNTBroadcastQOHelper(zt,                        \
                                    pg,                        \
                                    addr,                      \
                                    &MacroAssembler::ld1r##SZ, \
                                    4,                         \
                                    SH,                        \
                                    NO_SVE_OFFSET_MODIFIER,    \
                                    -1);                       \
  }

VIXL_SVE_LD1R_LIST(VIXL_DEFINE_MASM_FUNC)

#undef VIXL_DEFINE_MASM_FUNC
#undef VIXL_SVE_LD1R_LIST

void MacroAssembler::Ldnt1b(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    ldnt1b(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::ldnt1b,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}

void MacroAssembler::Ldnt1d(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    ldnt1d(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::ldnt1d,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}

void MacroAssembler::Ldnt1h(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    ldnt1h(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::ldnt1h,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}

void MacroAssembler::Ldnt1w(const ZRegister& zt,
                            const PRegisterZ& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    ldnt1w(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::ldnt1w,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}

void MacroAssembler::Stnt1b(const ZRegister& zt,
                            const PRegister& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    stnt1b(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::stnt1b,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}
void MacroAssembler::Stnt1d(const ZRegister& zt,
                            const PRegister& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    stnt1d(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::stnt1d,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}
void MacroAssembler::Stnt1h(const ZRegister& zt,
                            const PRegister& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    stnt1h(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::stnt1h,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}
void MacroAssembler::Stnt1w(const ZRegister& zt,
                            const PRegister& pg,
                            const SVEMemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (addr.IsVectorPlusScalar()) {
    SingleEmissionCheckScope guard(this);
    stnt1w(zt, pg, addr);
  } else {
    SVELoadStoreNTBroadcastQOHelper(zt,
                                    pg,
                                    addr,
                                    &MacroAssembler::stnt1w,
                                    4,
                                    0,
                                    SVE_MUL_VL);
  }
}

void MacroAssembler::SVEDotIndexHelper(ZZZImmFn fn,
                                       const ZRegister& zd,
                                       const ZRegister& za,
                                       const ZRegister& zn,
                                       const ZRegister& zm,
                                       int index) {
  if (zd.Aliases(za)) {
    // zda = zda + (zn . zm)
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zd, zn, zm, index);

  } else if (zd.Aliases(zn) || zd.Aliases(zm)) {
    // zdn = za + (zdn . zm[index])
    // zdm = za + (zn . zdm[index])
    // zdnm = za + (zdnm . zdnm[index])
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, za);
      (this->*fn)(scratch, zn, zm, index);
    }

    Mov(zd, scratch);
  } else {
    // zd = za + (zn . zm)
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, zm, index);
  }
}

void MacroAssembler::FourRegDestructiveHelper(Int3ArithFn fn,
                                              const ZRegister& zd,
                                              const ZRegister& za,
                                              const ZRegister& zn,
                                              const ZRegister& zm) {
  if (!zd.Aliases(za) && (zd.Aliases(zn) || zd.Aliases(zm))) {
    // zd = za . zd . zm
    // zd = za . zn . zd
    // zd = za . zd . zd
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, za);
      (this->*fn)(scratch, zn, zm);
    }

    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, zm);
  }
}

void MacroAssembler::FourRegDestructiveHelper(Int4ArithFn fn,
                                              const ZRegister& zd,
                                              const ZRegister& za,
                                              const ZRegister& zn,
                                              const ZRegister& zm) {
  if (!zd.Aliases(za) && (zd.Aliases(zn) || zd.Aliases(zm))) {
    // zd = za . zd . zm
    // zd = za . zn . zd
    // zd = za . zd . zd
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, za);
      (this->*fn)(scratch, scratch, zn, zm);
    }

    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zd, zn, zm);
  }
}

void MacroAssembler::FourRegOneImmDestructiveHelper(ZZZImmFn fn,
                                                    const ZRegister& zd,
                                                    const ZRegister& za,
                                                    const ZRegister& zn,
                                                    const ZRegister& zm,
                                                    int imm) {
  if (!zd.Aliases(za) && (zd.Aliases(zn) || zd.Aliases(zm))) {
    // zd = za . zd . zm[i]
    // zd = za . zn . zd[i]
    // zd = za . zd . zd[i]
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, za);
      (this->*fn)(scratch, zn, zm, imm);
    }

    Mov(zd, scratch);
  } else {
    // zd = za . zn . zm[i]
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, zm, imm);
  }
}

void MacroAssembler::AbsoluteDifferenceAccumulate(Int3ArithFn fn,
                                                  const ZRegister& zd,
                                                  const ZRegister& za,
                                                  const ZRegister& zn,
                                                  const ZRegister& zm) {
  if (zn.Aliases(zm)) {
    // If zn == zm, the difference is zero.
    if (!zd.Aliases(za)) {
      Mov(zd, za);
    }
  } else if (zd.Aliases(za)) {
    SingleEmissionCheckScope guard(this);
    (this->*fn)(zd, zn, zm);
  } else if (zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithLaneSize(zn.GetLaneSizeInBits());
    Mov(ztmp, zn);
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, ztmp, zm);
  } else if (zd.Aliases(zm)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithLaneSize(zn.GetLaneSizeInBits());
    Mov(ztmp, zm);
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, ztmp);
  } else {
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, zm);
  }
}

#define VIXL_SVE_4REG_LIST(V)                       \
  V(Saba, saba, AbsoluteDifferenceAccumulate)       \
  V(Uaba, uaba, AbsoluteDifferenceAccumulate)       \
  V(Sabalb, sabalb, AbsoluteDifferenceAccumulate)   \
  V(Sabalt, sabalt, AbsoluteDifferenceAccumulate)   \
  V(Uabalb, uabalb, AbsoluteDifferenceAccumulate)   \
  V(Uabalt, uabalt, AbsoluteDifferenceAccumulate)   \
  V(Sdot, sdot, FourRegDestructiveHelper)           \
  V(Udot, udot, FourRegDestructiveHelper)           \
  V(Adclb, adclb, FourRegDestructiveHelper)         \
  V(Adclt, adclt, FourRegDestructiveHelper)         \
  V(Sbclb, sbclb, FourRegDestructiveHelper)         \
  V(Sbclt, sbclt, FourRegDestructiveHelper)         \
  V(Smlalb, smlalb, FourRegDestructiveHelper)       \
  V(Smlalt, smlalt, FourRegDestructiveHelper)       \
  V(Smlslb, smlslb, FourRegDestructiveHelper)       \
  V(Smlslt, smlslt, FourRegDestructiveHelper)       \
  V(Umlalb, umlalb, FourRegDestructiveHelper)       \
  V(Umlalt, umlalt, FourRegDestructiveHelper)       \
  V(Umlslb, umlslb, FourRegDestructiveHelper)       \
  V(Umlslt, umlslt, FourRegDestructiveHelper)       \
  V(Bcax, bcax, FourRegDestructiveHelper)           \
  V(Bsl, bsl, FourRegDestructiveHelper)             \
  V(Bsl1n, bsl1n, FourRegDestructiveHelper)         \
  V(Bsl2n, bsl2n, FourRegDestructiveHelper)         \
  V(Eor3, eor3, FourRegDestructiveHelper)           \
  V(Nbsl, nbsl, FourRegDestructiveHelper)           \
  V(Fmlalb, fmlalb, FourRegDestructiveHelper)       \
  V(Fmlalt, fmlalt, FourRegDestructiveHelper)       \
  V(Fmlslb, fmlslb, FourRegDestructiveHelper)       \
  V(Fmlslt, fmlslt, FourRegDestructiveHelper)       \
  V(Sqdmlalb, sqdmlalb, FourRegDestructiveHelper)   \
  V(Sqdmlalbt, sqdmlalbt, FourRegDestructiveHelper) \
  V(Sqdmlalt, sqdmlalt, FourRegDestructiveHelper)   \
  V(Sqdmlslb, sqdmlslb, FourRegDestructiveHelper)   \
  V(Sqdmlslbt, sqdmlslbt, FourRegDestructiveHelper) \
  V(Sqdmlslt, sqdmlslt, FourRegDestructiveHelper)   \
  V(Sqrdmlah, sqrdmlah, FourRegDestructiveHelper)   \
  V(Sqrdmlsh, sqrdmlsh, FourRegDestructiveHelper)   \
  V(Fmmla, fmmla, FourRegDestructiveHelper)         \
  V(Smmla, smmla, FourRegDestructiveHelper)         \
  V(Ummla, ummla, FourRegDestructiveHelper)         \
  V(Usmmla, usmmla, FourRegDestructiveHelper)       \
  V(Usdot, usdot, FourRegDestructiveHelper)

#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN, HELPER) \
  void MacroAssembler::MASMFN(const ZRegister& zd,   \
                              const ZRegister& za,   \
                              const ZRegister& zn,   \
                              const ZRegister& zm) { \
    VIXL_ASSERT(allow_macro_instructions_);          \
    HELPER(&Assembler::ASMFN, zd, za, zn, zm);       \
  }
VIXL_SVE_4REG_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

#define VIXL_SVE_4REG_1IMM_LIST(V)                      \
  V(Fmla, fmla, FourRegOneImmDestructiveHelper)         \
  V(Fmls, fmls, FourRegOneImmDestructiveHelper)         \
  V(Fmlalb, fmlalb, FourRegOneImmDestructiveHelper)     \
  V(Fmlalt, fmlalt, FourRegOneImmDestructiveHelper)     \
  V(Fmlslb, fmlslb, FourRegOneImmDestructiveHelper)     \
  V(Fmlslt, fmlslt, FourRegOneImmDestructiveHelper)     \
  V(Mla, mla, FourRegOneImmDestructiveHelper)           \
  V(Mls, mls, FourRegOneImmDestructiveHelper)           \
  V(Smlalb, smlalb, FourRegOneImmDestructiveHelper)     \
  V(Smlalt, smlalt, FourRegOneImmDestructiveHelper)     \
  V(Smlslb, smlslb, FourRegOneImmDestructiveHelper)     \
  V(Smlslt, smlslt, FourRegOneImmDestructiveHelper)     \
  V(Sqdmlalb, sqdmlalb, FourRegOneImmDestructiveHelper) \
  V(Sqdmlalt, sqdmlalt, FourRegOneImmDestructiveHelper) \
  V(Sqdmlslb, sqdmlslb, FourRegOneImmDestructiveHelper) \
  V(Sqdmlslt, sqdmlslt, FourRegOneImmDestructiveHelper) \
  V(Sqrdmlah, sqrdmlah, FourRegOneImmDestructiveHelper) \
  V(Sqrdmlsh, sqrdmlsh, FourRegOneImmDestructiveHelper) \
  V(Umlalb, umlalb, FourRegOneImmDestructiveHelper)     \
  V(Umlalt, umlalt, FourRegOneImmDestructiveHelper)     \
  V(Umlslb, umlslb, FourRegOneImmDestructiveHelper)     \
  V(Umlslt, umlslt, FourRegOneImmDestructiveHelper)

#define VIXL_DEFINE_MASM_FUNC(MASMFN, ASMFN, HELPER) \
  void MacroAssembler::MASMFN(const ZRegister& zd,   \
                              const ZRegister& za,   \
                              const ZRegister& zn,   \
                              const ZRegister& zm,   \
                              int imm) {             \
    VIXL_ASSERT(allow_macro_instructions_);          \
    HELPER(&Assembler::ASMFN, zd, za, zn, zm, imm);  \
  }
VIXL_SVE_4REG_1IMM_LIST(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

void MacroAssembler::Sdot(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int index) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVEDotIndexHelper(&Assembler::sdot, zd, za, zn, zm, index);
}

void MacroAssembler::Udot(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int index) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVEDotIndexHelper(&Assembler::udot, zd, za, zn, zm, index);
}

void MacroAssembler::Sudot(const ZRegister& zd,
                           const ZRegister& za,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           int index) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVEDotIndexHelper(&Assembler::sudot, zd, za, zn, zm, index);
}

void MacroAssembler::Usdot(const ZRegister& zd,
                           const ZRegister& za,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           int index) {
  VIXL_ASSERT(allow_macro_instructions_);
  SVEDotIndexHelper(&Assembler::usdot, zd, za, zn, zm, index);
}

void MacroAssembler::Cdot(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int index,
                          int rot) {
  // This doesn't handle zm when it's out of the range that can be encoded in
  // instruction. The range depends on element size: z0-z7 for B, z0-15 for H.
  if ((zd.Aliases(zn) || zd.Aliases(zm)) && !zd.Aliases(za)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, ztmp, za);
      cdot(ztmp, zn, zm, index, rot);
    }
    Mov(zd, ztmp);
  } else {
    MovprfxHelperScope guard(this, zd, za);
    cdot(zd, zn, zm, index, rot);
  }
}

void MacroAssembler::Cdot(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int rot) {
  if ((zd.Aliases(zn) || zd.Aliases(zm)) && !zd.Aliases(za)) {
    UseScratchRegisterScope temps(this);
    VIXL_ASSERT(AreSameLaneSize(zn, zm));
    ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zn);
    Mov(ztmp, zd.Aliases(zn) ? zn : zm);
    MovprfxHelperScope guard(this, zd, za);
    cdot(zd, (zd.Aliases(zn) ? ztmp : zn), (zd.Aliases(zm) ? ztmp : zm), rot);
  } else {
    MovprfxHelperScope guard(this, zd, za);
    cdot(zd, zn, zm, rot);
  }
}

void MacroAssembler::FPMulAddHelper(const ZRegister& zd,
                                    const PRegisterM& pg,
                                    const ZRegister& za,
                                    const ZRegister& zn,
                                    const ZRegister& zm,
                                    SVEMulAddPredicatedZdaFn fn_zda,
                                    SVEMulAddPredicatedZdnFn fn_zdn,
                                    FPMacroNaNPropagationOption nan_option) {
  ResolveFPNaNPropagationOption(&nan_option);

  if (zd.Aliases(za)) {
    // zda = (-)zda + ((-)zn * zm) for fmla, fmls, fnmla and fnmls.
    SingleEmissionCheckScope guard(this);
    (this->*fn_zda)(zd, pg, zn, zm);
  } else if (zd.Aliases(zn)) {
    // zdn = (-)za + ((-)zdn * zm) for fmad, fmsb, fnmad and fnmsb.
    SingleEmissionCheckScope guard(this);
    (this->*fn_zdn)(zd, pg, zm, za);
  } else if (zd.Aliases(zm)) {
    switch (nan_option) {
      case FastNaNPropagation: {
        // We treat multiplication as commutative in the fast mode, so we can
        // swap zn and zm.
        // zdm = (-)za + ((-)zdm * zn) for fmad, fmsb, fnmad and fnmsb.
        SingleEmissionCheckScope guard(this);
        (this->*fn_zdn)(zd, pg, zn, za);
        return;
      }
      case StrictNaNPropagation: {
        UseScratchRegisterScope temps(this);
        // Use a scratch register to keep the argument order exactly as
        // specified.
        ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zn);
        {
          MovprfxHelperScope guard(this, scratch, pg, za);
          // scratch = (-)za + ((-)zn * zm)
          (this->*fn_zda)(scratch, pg, zn, zm);
        }
        Mov(zd, scratch);
        return;
      }
      case NoFPMacroNaNPropagationSelected:
        VIXL_UNREACHABLE();
        return;
    }
  } else {
    // zd = (-)za + ((-)zn * zm) for fmla, fmls, fnmla and fnmls.
    MovprfxHelperScope guard(this, zd, pg, za);
    (this->*fn_zda)(zd, pg, zn, zm);
  }
}

void MacroAssembler::Fmla(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPMulAddHelper(zd,
                 pg,
                 za,
                 zn,
                 zm,
                 &Assembler::fmla,
                 &Assembler::fmad,
                 nan_option);
}

void MacroAssembler::Fmls(const ZRegister& zd,
                          const PRegisterM& pg,
                          const ZRegister& za,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPMulAddHelper(zd,
                 pg,
                 za,
                 zn,
                 zm,
                 &Assembler::fmls,
                 &Assembler::fmsb,
                 nan_option);
}

void MacroAssembler::Fnmla(const ZRegister& zd,
                           const PRegisterM& pg,
                           const ZRegister& za,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPMulAddHelper(zd,
                 pg,
                 za,
                 zn,
                 zm,
                 &Assembler::fnmla,
                 &Assembler::fnmad,
                 nan_option);
}

void MacroAssembler::Fnmls(const ZRegister& zd,
                           const PRegisterM& pg,
                           const ZRegister& za,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           FPMacroNaNPropagationOption nan_option) {
  VIXL_ASSERT(allow_macro_instructions_);
  FPMulAddHelper(zd,
                 pg,
                 za,
                 zn,
                 zm,
                 &Assembler::fnmls,
                 &Assembler::fnmsb,
                 nan_option);
}

void MacroAssembler::Ftmad(const ZRegister& zd,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           int imm3) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(zm) && !zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zm);
    Mov(scratch, zm);
    MovprfxHelperScope guard(this, zd, zn);
    ftmad(zd, zd, scratch, imm3);
  } else {
    MovprfxHelperScope guard(this, zd, zn);
    ftmad(zd, zd, zm, imm3);
  }
}

void MacroAssembler::Fcadd(const ZRegister& zd,
                           const PRegisterM& pg,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           int rot) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(zm) && !zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, pg, zn);
      fcadd(scratch, pg, scratch, zm, rot);
    }
    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, pg, zn);
    fcadd(zd, pg, zd, zm, rot);
  }
}

void MacroAssembler::Fcmla(const ZRegister& zd,
                           const PRegisterM& pg,
                           const ZRegister& za,
                           const ZRegister& zn,
                           const ZRegister& zm,
                           int rot) {
  VIXL_ASSERT(allow_macro_instructions_);
  if ((zd.Aliases(zn) || zd.Aliases(zm)) && !zd.Aliases(za)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, ztmp, za);
      fcmla(ztmp, pg, zn, zm, rot);
    }
    Mov(zd, pg, ztmp);
  } else {
    MovprfxHelperScope guard(this, zd, pg, za);
    fcmla(zd, pg, zn, zm, rot);
  }
}

void MacroAssembler::Splice(const ZRegister& zd,
                            const PRegister& pg,
                            const ZRegister& zn,
                            const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (CPUHas(CPUFeatures::kSVE2) && AreConsecutive(zn, zm) && !zd.Aliases(zn)) {
    SingleEmissionCheckScope guard(this);
    splice(zd, pg, zn, zm);
  } else if (zd.Aliases(zm) && !zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, zn);
      splice(scratch, pg, scratch, zm);
    }
    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, zn);
    splice(zd, pg, zd, zm);
  }
}

void MacroAssembler::Clasta(const ZRegister& zd,
                            const PRegister& pg,
                            const ZRegister& zn,
                            const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(zm) && !zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, zn);
      clasta(scratch, pg, scratch, zm);
    }
    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, zn);
    clasta(zd, pg, zd, zm);
  }
}

void MacroAssembler::Clastb(const ZRegister& zd,
                            const PRegister& pg,
                            const ZRegister& zn,
                            const ZRegister& zm) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (zd.Aliases(zm) && !zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister scratch = temps.AcquireZ().WithSameLaneSizeAs(zd);
    {
      MovprfxHelperScope guard(this, scratch, zn);
      clastb(scratch, pg, scratch, zm);
    }
    Mov(zd, scratch);
  } else {
    MovprfxHelperScope guard(this, zd, zn);
    clastb(zd, pg, zd, zm);
  }
}

void MacroAssembler::ShiftRightAccumulate(IntArithImmFn fn,
                                          const ZRegister& zd,
                                          const ZRegister& za,
                                          const ZRegister& zn,
                                          int shift) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (!zd.Aliases(za) && zd.Aliases(zn)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zn);
    Mov(ztmp, zn);
    {
      MovprfxHelperScope guard(this, zd, za);
      (this->*fn)(zd, ztmp, shift);
    }
  } else {
    MovprfxHelperScope guard(this, zd, za);
    (this->*fn)(zd, zn, shift);
  }
}

void MacroAssembler::Srsra(const ZRegister& zd,
                           const ZRegister& za,
                           const ZRegister& zn,
                           int shift) {
  ShiftRightAccumulate(&Assembler::srsra, zd, za, zn, shift);
}

void MacroAssembler::Ssra(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          int shift) {
  ShiftRightAccumulate(&Assembler::ssra, zd, za, zn, shift);
}

void MacroAssembler::Ursra(const ZRegister& zd,
                           const ZRegister& za,
                           const ZRegister& zn,
                           int shift) {
  ShiftRightAccumulate(&Assembler::ursra, zd, za, zn, shift);
}

void MacroAssembler::Usra(const ZRegister& zd,
                          const ZRegister& za,
                          const ZRegister& zn,
                          int shift) {
  ShiftRightAccumulate(&Assembler::usra, zd, za, zn, shift);
}

void MacroAssembler::ComplexAddition(ZZZImmFn fn,
                                     const ZRegister& zd,
                                     const ZRegister& zn,
                                     const ZRegister& zm,
                                     int rot) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (!zd.Aliases(zn) && zd.Aliases(zm)) {
    UseScratchRegisterScope temps(this);
    ZRegister ztmp = temps.AcquireZ().WithSameLaneSizeAs(zm);
    Mov(ztmp, zm);
    {
      MovprfxHelperScope guard(this, zd, zn);
      (this->*fn)(zd, zd, ztmp, rot);
    }
  } else {
    MovprfxHelperScope guard(this, zd, zn);
    (this->*fn)(zd, zd, zm, rot);
  }
}

void MacroAssembler::Cadd(const ZRegister& zd,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int rot) {
  ComplexAddition(&Assembler::cadd, zd, zn, zm, rot);
}

void MacroAssembler::Sqcadd(const ZRegister& zd,
                            const ZRegister& zn,
                            const ZRegister& zm,
                            int rot) {
  ComplexAddition(&Assembler::sqcadd, zd, zn, zm, rot);
}

}  // namespace aarch64
}  // namespace vixl
