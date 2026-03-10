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

#include "assembler-aarch64.h"

namespace vixl {
namespace aarch64 {

void Assembler::ResolveSVEImm8Shift(int* imm8, int* shift) {
  if (*shift < 0) {
    VIXL_ASSERT(*shift == -1);
    // Derive the shift amount from the immediate.
    if (IsInt8(*imm8)) {
      *shift = 0;
    } else if ((*imm8 % 256) == 0) {
      *imm8 /= 256;
      *shift = 8;
    }
  }

  VIXL_ASSERT(IsInt8(*imm8));
  VIXL_ASSERT((*shift == 0) || (*shift == 8));
}

// SVEAddressGeneration.

void Assembler::adr(const ZRegister& zd, const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsVectorPlusVector());
  VIXL_ASSERT(
      AreSameLaneSize(zd, addr.GetVectorBase(), addr.GetVectorOffset()));

  int lane_size = zd.GetLaneSizeInBits();
  VIXL_ASSERT((lane_size == kSRegSize) || (lane_size == kDRegSize));

  int shift_amount = addr.GetShiftAmount();
  VIXL_ASSERT((shift_amount >= 0) && (shift_amount <= 3));

  Instr op = 0xffffffff;
  Instr msz = shift_amount << 10;
  SVEOffsetModifier mod = addr.GetOffsetModifier();
  switch (mod) {
    case SVE_UXTW:
      VIXL_ASSERT(lane_size == kDRegSize);
      op = ADR_z_az_d_u32_scaled;
      break;
    case SVE_SXTW:
      VIXL_ASSERT(lane_size == kDRegSize);
      op = ADR_z_az_d_s32_scaled;
      break;
    case SVE_LSL:
    case NO_SVE_OFFSET_MODIFIER:
      op = (lane_size == kSRegSize) ? ADR_z_az_s_same_scaled
                                    : ADR_z_az_d_same_scaled;
      break;
    default:
      VIXL_UNIMPLEMENTED();
  }
  Emit(op | msz | Rd(zd) | Rn(addr.GetVectorBase()) |
       Rm(addr.GetVectorOffset()));
}

void Assembler::SVELogicalImmediate(const ZRegister& zdn,
                                    uint64_t imm,
                                    Instr op) {
  unsigned bit_n, imm_s, imm_r;
  unsigned lane_size = zdn.GetLaneSizeInBits();
  // Check that the immediate can be encoded in the instruction.
  if (IsImmLogical(imm, lane_size, &bit_n, &imm_s, &imm_r)) {
    Emit(op | Rd(zdn) | SVEBitN(bit_n) | SVEImmRotate(imm_r, lane_size) |
         SVEImmSetBits(imm_s, lane_size));
  } else {
    VIXL_UNREACHABLE();
  }
}

void Assembler::and_(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  SVELogicalImmediate(zd, imm, AND_z_zi);
}

void Assembler::dupm(const ZRegister& zd, uint64_t imm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  // DUPM_z_i is an SVEBroadcastBitmaskImmOp, but its encoding and constraints
  // are similar enough to SVEBitwiseLogicalWithImm_UnpredicatedOp, that we can
  // use the logical immediate encoder to get the correct behaviour.
  SVELogicalImmediate(zd, imm, DUPM_z_i);
}

void Assembler::eor(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  SVELogicalImmediate(zd, imm, EOR_z_zi);
}

void Assembler::orr(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  SVELogicalImmediate(zd, imm, ORR_z_zi);
}

// SVEBitwiseLogicalUnpredicated.
void Assembler::and_(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.IsLaneSizeD());
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  Emit(AND_z_zz | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::bic(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.IsLaneSizeD());
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  Emit(BIC_z_zz | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::eor(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.IsLaneSizeD());
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  Emit(EOR_z_zz | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::orr(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.IsLaneSizeD());
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  Emit(ORR_z_zz | Rd(zd) | Rn(zn) | Rm(zm));
}

// SVEBitwiseShiftPredicated.

void Assembler::SVEBitwiseShiftImmediatePred(const ZRegister& zdn,
                                             const PRegisterM& pg,
                                             Instr encoded_imm_and_tsz,
                                             Instr op) {
  Instr tszl_and_imm = ExtractUnsignedBitfield32(4, 0, encoded_imm_and_tsz)
                       << 5;
  Instr tszh = ExtractUnsignedBitfield32(6, 5, encoded_imm_and_tsz) << 22;
  Emit(op | tszh | tszl_and_imm | PgLow8(pg) | Rd(zdn));
}

void Assembler::asr(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    int shift) {
  // ASR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0000 100. .... .... ....
  //  tszh<23:22> | opc<19:18> = 00 | L<17> = 0 | U<16> = 0 | Pg<12:10> |
  //  tszl<9:8> | imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, ASR_z_p_zi);
}

void Assembler::asr(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // ASR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.D
  //  0000 0100 ..01 1000 100. .... .... ....
  //  size<23:22> | R<18> = 0 | L<17> = 0 | U<16> = 0 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm) ||
              ((zm.GetLaneSizeInBytes() == kDRegSizeInBytes) &&
               (zd.GetLaneSizeInBytes() != kDRegSizeInBytes)));
  Instr op = ASR_z_p_zw;
  if (AreSameLaneSize(zd, zn, zm)) {
    op = ASR_z_p_zz;
  }
  Emit(op | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::asrd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     int shift) {
  // ASRD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0100 100. .... .... ....
  //  tszh<23:22> | opc<19:18> = 01 | L<17> = 0 | U<16> = 0 | Pg<12:10> |
  //  tszl<9:8> | imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));

  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, ASRD_z_p_zi);
}

void Assembler::asrr(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // ASRR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0100 100. .... .... ....
  //  size<23:22> | R<18> = 1 | L<17> = 0 | U<16> = 0 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(ASRR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::lsl(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    int shift) {
  // LSL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0011 100. .... .... ....
  //  tszh<23:22> | opc<19:18> = 00 | L<17> = 1 | U<16> = 1 | Pg<12:10> |
  //  tszl<9:8> | imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, LSL_z_p_zi);
}

void Assembler::lsl(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // LSL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.D
  //  0000 0100 ..01 1011 100. .... .... ....
  //  size<23:22> | R<18> = 0 | L<17> = 1 | U<16> = 1 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm) ||
              ((zm.GetLaneSizeInBytes() == kDRegSizeInBytes) &&
               (zd.GetLaneSizeInBytes() != kDRegSizeInBytes)));
  Instr op = LSL_z_p_zw;
  if (AreSameLaneSize(zd, zn, zm)) {
    op = LSL_z_p_zz;
  }
  Emit(op | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::lslr(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // LSLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0111 100. .... .... ....
  //  size<23:22> | R<18> = 1 | L<17> = 1 | U<16> = 1 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(LSLR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::lsr(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    int shift) {
  // LSR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0001 100. .... .... ....
  //  tszh<23:22> | opc<19:18> = 00 | L<17> = 0 | U<16> = 1 | Pg<12:10> |
  //  tszl<9:8> | imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));

  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, LSR_z_p_zi);
}

void Assembler::lsr(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // LSR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.D
  //  0000 0100 ..01 1001 100. .... .... ....
  //  size<23:22> | R<18> = 0 | L<17> = 0 | U<16> = 1 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm) ||
              ((zm.GetLaneSizeInBytes() == kDRegSizeInBytes) &&
               (zd.GetLaneSizeInBytes() != kDRegSizeInBytes)));
  Instr op = LSR_z_p_zw;
  if (AreSameLaneSize(zd, zn, zm)) {
    op = LSR_z_p_zz;
  }
  Emit(op | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::lsrr(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // LSRR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0101 100. .... .... ....
  //  size<23:22> | R<18> = 1 | L<17> = 0 | U<16> = 1 | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(LSRR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

// SVEBitwiseShiftUnpredicated.

Instr Assembler::EncodeSVEShiftLeftImmediate(int shift, int lane_size_in_bits) {
  VIXL_ASSERT((shift >= 0) && (shift < lane_size_in_bits));
  return lane_size_in_bits + shift;
}

Instr Assembler::EncodeSVEShiftRightImmediate(int shift,
                                              int lane_size_in_bits) {
  VIXL_ASSERT((shift > 0) && (shift <= lane_size_in_bits));
  return (2 * lane_size_in_bits) - shift;
}

void Assembler::SVEBitwiseShiftImmediate(const ZRegister& zd,
                                         const ZRegister& zn,
                                         Instr encoded_imm_and_tsz,
                                         Instr op) {
  Instr tszl_and_imm = ExtractUnsignedBitfield32(4, 0, encoded_imm_and_tsz)
                       << 16;
  Instr tszh = ExtractUnsignedBitfield32(6, 5, encoded_imm_and_tsz) << 22;
  Emit(op | tszh | tszl_and_imm | Rd(zd) | Rn(zn));
}

void Assembler::asr(const ZRegister& zd, const ZRegister& zn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, ASR_z_zi);
}

void Assembler::asr(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kDRegSizeInBytes);

  Emit(ASR_z_zw | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::lsl(const ZRegister& zd, const ZRegister& zn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, LSL_z_zi);
}

void Assembler::lsl(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kDRegSizeInBytes);

  Emit(LSL_z_zw | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::lsr(const ZRegister& zd, const ZRegister& zn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, LSR_z_zi);
}

void Assembler::lsr(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kDRegSizeInBytes);

  Emit(LSR_z_zw | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

// SVEElementCount.

#define VIXL_SVE_INC_DEC_LIST(V) \
  V(cntb, CNTB_r_s)              \
  V(cnth, CNTH_r_s)              \
  V(cntw, CNTW_r_s)              \
  V(cntd, CNTD_r_s)              \
  V(decb, DECB_r_rs)             \
  V(dech, DECH_r_rs)             \
  V(decw, DECW_r_rs)             \
  V(decd, DECD_r_rs)             \
  V(incb, INCB_r_rs)             \
  V(inch, INCH_r_rs)             \
  V(incw, INCW_r_rs)             \
  V(incd, INCD_r_rs)             \
  V(sqdecb, SQDECB_r_rs_x)       \
  V(sqdech, SQDECH_r_rs_x)       \
  V(sqdecw, SQDECW_r_rs_x)       \
  V(sqdecd, SQDECD_r_rs_x)       \
  V(sqincb, SQINCB_r_rs_x)       \
  V(sqinch, SQINCH_r_rs_x)       \
  V(sqincw, SQINCW_r_rs_x)       \
  V(sqincd, SQINCD_r_rs_x)

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                     \
  void Assembler::FN(const Register& rdn, int pattern, int multiplier) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                              \
    VIXL_ASSERT(rdn.IsX());                                              \
    Emit(OP | Rd(rdn) | ImmSVEPredicateConstraint(pattern) |             \
         ImmUnsignedField<19, 16>(multiplier - 1));                      \
  }
VIXL_SVE_INC_DEC_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

#define VIXL_SVE_UQINC_UQDEC_LIST(V)                      \
  V(uqdecb, (rdn.IsX() ? UQDECB_r_rs_x : UQDECB_r_rs_uw)) \
  V(uqdech, (rdn.IsX() ? UQDECH_r_rs_x : UQDECH_r_rs_uw)) \
  V(uqdecw, (rdn.IsX() ? UQDECW_r_rs_x : UQDECW_r_rs_uw)) \
  V(uqdecd, (rdn.IsX() ? UQDECD_r_rs_x : UQDECD_r_rs_uw)) \
  V(uqincb, (rdn.IsX() ? UQINCB_r_rs_x : UQINCB_r_rs_uw)) \
  V(uqinch, (rdn.IsX() ? UQINCH_r_rs_x : UQINCH_r_rs_uw)) \
  V(uqincw, (rdn.IsX() ? UQINCW_r_rs_x : UQINCW_r_rs_uw)) \
  V(uqincd, (rdn.IsX() ? UQINCD_r_rs_x : UQINCD_r_rs_uw))

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                     \
  void Assembler::FN(const Register& rdn, int pattern, int multiplier) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                              \
    Emit(OP | Rd(rdn) | ImmSVEPredicateConstraint(pattern) |             \
         ImmUnsignedField<19, 16>(multiplier - 1));                      \
  }
VIXL_SVE_UQINC_UQDEC_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

#define VIXL_SVE_SQX_INC_DEC_LIST(V) \
  V(sqdecb, SQDECB)                  \
  V(sqdech, SQDECH)                  \
  V(sqdecw, SQDECW)                  \
  V(sqdecd, SQDECD)                  \
  V(sqincb, SQINCB)                  \
  V(sqinch, SQINCH)                  \
  V(sqincw, SQINCW)                  \
  V(sqincd, SQINCD)

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                  \
  void Assembler::FN(const Register& xd,                              \
                     const Register& wn,                              \
                     int pattern,                                     \
                     int multiplier) {                                \
    USE(wn);                                                          \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                           \
    VIXL_ASSERT(wn.IsW() && xd.Is(wn.X()));                           \
    Emit(OP##_r_rs_sx | Rd(xd) | ImmSVEPredicateConstraint(pattern) | \
         ImmUnsignedField<19, 16>(multiplier - 1));                   \
  }
VIXL_SVE_SQX_INC_DEC_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

#define VIXL_SVE_INC_DEC_VEC_LIST(V) \
  V(dech, DEC, H)                    \
  V(decw, DEC, W)                    \
  V(decd, DEC, D)                    \
  V(inch, INC, H)                    \
  V(incw, INC, W)                    \
  V(incd, INC, D)                    \
  V(sqdech, SQDEC, H)                \
  V(sqdecw, SQDEC, W)                \
  V(sqdecd, SQDEC, D)                \
  V(sqinch, SQINC, H)                \
  V(sqincw, SQINC, W)                \
  V(sqincd, SQINC, D)                \
  V(uqdech, UQDEC, H)                \
  V(uqdecw, UQDEC, W)                \
  V(uqdecd, UQDEC, D)                \
  V(uqinch, UQINC, H)                \
  V(uqincw, UQINC, W)                \
  V(uqincd, UQINC, D)

#define VIXL_DEFINE_ASM_FUNC(FN, OP, T)                                   \
  void Assembler::FN(const ZRegister& zdn, int pattern, int multiplier) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                               \
    VIXL_ASSERT(zdn.GetLaneSizeInBytes() == k##T##RegSizeInBytes);        \
    Emit(OP##T##_z_zs | Rd(zdn) | ImmSVEPredicateConstraint(pattern) |    \
         ImmUnsignedField<19, 16>(multiplier - 1));                       \
  }
VIXL_SVE_INC_DEC_VEC_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// SVEFPAccumulatingReduction.

void Assembler::fadda(const VRegister& vd,
                      const PRegister& pg,
                      const VRegister& vn,
                      const ZRegister& zm) {
  // FADDA <V><dn>, <Pg>, <V><dn>, <Zm>.<T>
  //  0110 0101 ..01 1000 001. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zm<9:5> | Vdn<4:0>

  USE(vn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.Is(vn));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zm.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zm, vd));

  Emit(FADDA_v_p_z | SVESize(zm) | Rd(vd) | PgLow8(pg) | Rn(zm));
}

// SVEFPArithmetic_Predicated.

void Assembler::fabd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FABD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 1000 100. .... .... ....
  //  size<23:22> | opc<19:16> = 1000 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FABD_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fadd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     double imm) {
  // FADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1000 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((imm == 0.5) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FADD_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fadd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0000 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0000 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FADD_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fdiv(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FDIV <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 1101 100. .... .... ....
  //  size<23:22> | opc<19:16> = 1101 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FDIV_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fdivr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FDIVR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 1100 100. .... .... ....
  //  size<23:22> | opc<19:16> = 1100 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FDIVR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmax(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     double imm) {
  // FMAX <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1110 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 110 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(((imm == 0.0) && (copysign(1.0, imm) == 1.0)) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FMAX_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fmax(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMAX <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0110 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0110 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMAX_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmaxnm(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       double imm) {
  // FMAXNM <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1100 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 100 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(((imm == 0.0) && (copysign(1.0, imm) == 1.0)) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FMAXNM_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fmaxnm(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMAXNM <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0100 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0100 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMAXNM_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmin(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     double imm) {
  // FMIN <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1111 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 111 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(((imm == 0.0) && (copysign(1.0, imm) == 1.0)) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FMIN_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fmin(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMIN <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0111 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0111 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMIN_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fminnm(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       double imm) {
  // FMINNM <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1101 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 101 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(((imm == 0.0) && (copysign(1.0, imm) == 1.0)) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FMINNM_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fminnm(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMINNM <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0101 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0101 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMINNM_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmul(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     double imm) {
  // FMUL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1010 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 010 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((imm == 0.5) || (imm == 2.0));

  Instr i1 = (imm == 2.0) ? (1 << 5) : 0;
  Emit(FMUL_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fmul(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMUL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0010 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0010 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMUL_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmulx(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FMULX <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 1010 100. .... .... ....
  //  size<23:22> | opc<19:16> = 1010 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMULX_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fscale(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FSCALE <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 1001 100. .... .... ....
  //  size<23:22> | opc<19:16> = 1001 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FSCALE_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fsub(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     double imm) {
  // FSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1001 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 001 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((imm == 0.5) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FSUB_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fsub(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0001 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0001 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FSUB_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fsubr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      double imm) {
  // FSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <const>
  //  0110 0101 ..01 1011 100. ..00 00.. ....
  //  size<23:22> | opc<18:16> = 011 | Pg<12:10> | i1<5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((imm == 0.5) || (imm == 1.0));

  Instr i1 = (imm == 1.0) ? (1 << 5) : 0;
  Emit(FSUBR_z_p_zs | SVESize(zd) | Rd(zd) | PgLow8(pg) | i1);
}

void Assembler::fsubr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0101 ..00 0011 100. .... .... ....
  //  size<23:22> | opc<19:16> = 0011 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FSUBR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::ftmad(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int imm3) {
  // FTMAD <Zdn>.<T>, <Zdn>.<T>, <Zm>.<T>, #<imm>
  //  0110 0101 ..01 0... 1000 00.. .... ....
  //  size<23:22> | imm3<18:16> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FTMAD_z_zzi | SVESize(zd) | Rd(zd) | Rn(zm) |
       ImmUnsignedField<18, 16>(imm3));
}

// SVEFPArithmeticUnpredicated.

void Assembler::fadd(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FADD <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0000 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 000 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FADD_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::fmul(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMUL <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0000 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 010 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMUL_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::frecps(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FRECPS <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0001 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 110 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRECPS_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::frsqrts(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // FRSQRTS <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0001 11.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 111 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRSQRTS_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::fsub(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FSUB <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0000 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 001 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FSUB_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ftsmul(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FTSMUL <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 0000 11.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 011 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FTSMUL_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

// SVEFPCompareVectors.

void Assembler::facge(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FACGE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 110. .... ...1 ....
  //  size<23:22> | Zm<20:16> | op<15> = 1 | o2<13> = 0 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FACGE_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::facgt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FACGT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 111. .... ...1 ....
  //  size<23:22> | Zm<20:16> | op<15> = 1 | o2<13> = 1 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FACGT_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fcmeq(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FCMEQ <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 011. .... ...0 ....
  //  size<23:22> | Zm<20:16> | op<15> = 0 | o2<13> = 1 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FCMEQ_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fcmge(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FCMGE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 010. .... ...0 ....
  //  size<23:22> | Zm<20:16> | op<15> = 0 | o2<13> = 0 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FCMGE_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fcmgt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FCMGT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 010. .... ...1 ....
  //  size<23:22> | Zm<20:16> | op<15> = 0 | o2<13> = 0 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FCMGT_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fcmne(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FCMNE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 011. .... ...1 ....
  //  size<23:22> | Zm<20:16> | op<15> = 0 | o2<13> = 1 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FCMNE_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fcmuo(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FCMUO <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..0. .... 110. .... ...0 ....
  //  size<23:22> | Zm<20:16> | op<15> = 1 | o2<13> = 0 | Pg<12:10> | Zn<9:5> |
  //  o3<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FCMUO_p_p_zz | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

// SVEFPCompareWithZero.

void Assembler::fcmeq(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMEQ <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0010 001. .... ...0 ....
  //  size<23:22> | eq<17> = 1 | lt<16> = 0 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMEQ_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcmge(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMGE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0000 001. .... ...0 ....
  //  size<23:22> | eq<17> = 0 | lt<16> = 0 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMGE_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcmgt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMGT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0000 001. .... ...1 ....
  //  size<23:22> | eq<17> = 0 | lt<16> = 0 | Pg<12:10> | Zn<9:5> | ne<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMGT_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcmle(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMLE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0001 001. .... ...1 ....
  //  size<23:22> | eq<17> = 0 | lt<16> = 1 | Pg<12:10> | Zn<9:5> | ne<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMLE_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcmlt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMLT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0001 001. .... ...0 ....
  //  size<23:22> | eq<17> = 0 | lt<16> = 1 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMLT_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcmne(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      double zero) {
  // FCMNE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #0.0
  //  0110 0101 ..01 0011 001. .... ...0 ....
  //  size<23:22> | eq<17> = 1 | lt<16> = 1 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(zero == 0.0);
  USE(zero);

  Emit(FCMNE_p_p_z0 | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn));
}

// SVEFPComplexAddition.

void Assembler::fcadd(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int rot) {
  // FCADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>, <const>
  //  0110 0100 ..00 000. 100. .... .... ....
  //  size<23:22> | rot<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((rot == 90) || (rot == 270));

  Instr rotate_bit = (rot == 90) ? 0 : (1 << 16);
  Emit(FCADD_z_p_zz | rotate_bit | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

// SVEFPComplexMulAdd.

void Assembler::fcmla(const ZRegister& zda,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int rot) {
  // FCMLA <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>, <const>
  //  0110 0100 ..0. .... 0... .... .... ....
  //  size<23:22> | Zm<20:16> | rot<14:13> | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 13;
  Emit(FCMLA_z_p_zzz | rotate_bit | SVESize(zda) | Rd(zda) | PgLow8(pg) |
       Rn(zn) | Rm(zm));
}

// SVEFPComplexMulAddIndex.

void Assembler::fcmla(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int index,
                      int rot) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 10;
  Emit(FCMLA_z_zzzi_h | SVEMulComplexIndexHelper(zm, index) | rotate_bit |
       Rd(zda) | Rn(zn));
}

// SVEFPFastReduction.

void Assembler::faddv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // FADDV <V><d>, <Pg>, <Zn>.<T>
  //  0110 0101 ..00 0000 001. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zn, vd));

  Emit(FADDV_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fmaxnmv(const VRegister& vd,
                        const PRegister& pg,
                        const ZRegister& zn) {
  // FMAXNMV <V><d>, <Pg>, <Zn>.<T>
  //  0110 0101 ..00 0100 001. .... .... ....
  //  size<23:22> | opc<18:16> = 100 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zn, vd));

  Emit(FMAXNMV_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fmaxv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // FMAXV <V><d>, <Pg>, <Zn>.<T>
  //  0110 0101 ..00 0110 001. .... .... ....
  //  size<23:22> | opc<18:16> = 110 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zn, vd));

  Emit(FMAXV_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fminnmv(const VRegister& vd,
                        const PRegister& pg,
                        const ZRegister& zn) {
  // FMINNMV <V><d>, <Pg>, <Zn>.<T>
  //  0110 0101 ..00 0101 001. .... .... ....
  //  size<23:22> | opc<18:16> = 101 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zn, vd));

  Emit(FMINNMV_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fminv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // FMINV <V><d>, <Pg>, <Zn>.<T>
  //  0110 0101 ..00 0111 001. .... .... ....
  //  size<23:22> | opc<18:16> = 111 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(AreSameLaneSize(zn, vd));

  Emit(FMINV_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

// SVEFPMulAdd.

void Assembler::fmad(const ZRegister& zdn,
                     const PRegisterM& pg,
                     const ZRegister& zm,
                     const ZRegister& za) {
  // FMAD <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0110 0101 ..1. .... 100. .... .... ....
  //  size<23:22> | Za<20:16> | opc<14:13> = 00 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMAD_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rn(zm) | Rm(za));
}

void Assembler::fmla(const ZRegister& zda,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMLA <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..1. .... 000. .... .... ....
  //  size<23:22> | Zm<20:16> | opc<14:13> = 00 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMLA_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fmls(const ZRegister& zda,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // FMLS <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..1. .... 001. .... .... ....
  //  size<23:22> | Zm<20:16> | opc<14:13> = 01 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMLS_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fmsb(const ZRegister& zdn,
                     const PRegisterM& pg,
                     const ZRegister& zm,
                     const ZRegister& za) {
  // FMSB <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0110 0101 ..1. .... 101. .... .... ....
  //  size<23:22> | Za<20:16> | opc<14:13> = 01 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FMSB_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rn(zm) | Rm(za));
}

void Assembler::fnmad(const ZRegister& zdn,
                      const PRegisterM& pg,
                      const ZRegister& zm,
                      const ZRegister& za) {
  // FNMAD <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0110 0101 ..1. .... 110. .... .... ....
  //  size<23:22> | Za<20:16> | opc<14:13> = 10 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FNMAD_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rn(zm) | Rm(za));
}

void Assembler::fnmla(const ZRegister& zda,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FNMLA <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..1. .... 010. .... .... ....
  //  size<23:22> | Zm<20:16> | opc<14:13> = 10 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FNMLA_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fnmls(const ZRegister& zda,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FNMLS <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0110 0101 ..1. .... 011. .... .... ....
  //  size<23:22> | Zm<20:16> | opc<14:13> = 11 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FNMLS_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::fnmsb(const ZRegister& zdn,
                      const PRegisterM& pg,
                      const ZRegister& zm,
                      const ZRegister& za) {
  // FNMSB <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0110 0101 ..1. .... 111. .... .... ....
  //  size<23:22> | Za<20:16> | opc<14:13> = 11 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FNMSB_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rn(zm) | Rm(za));
}

Instr Assembler::SVEMulIndexHelper(unsigned lane_size_in_bytes_log2,
                                   const ZRegister& zm,
                                   int index,
                                   Instr op_h,
                                   Instr op_s,
                                   Instr op_d) {
  Instr size = lane_size_in_bytes_log2 << SVESize_offset;
  Instr zm_with_index = Rm(zm);
  Instr op = 0xffffffff;
  // Allowable register number and lane index depends on the lane size.
  switch (lane_size_in_bytes_log2) {
    case kHRegSizeInBytesLog2:
      VIXL_ASSERT(zm.GetCode() <= 7);
      VIXL_ASSERT(IsUint3(index));
      // For H-sized lanes, size is encoded as 0b0x, where x is used as the top
      // bit of the index. So, if index is less than four, the top bit of index
      // is zero, and therefore size is 0b00. Otherwise, it's 0b01, the usual
      // encoding for H-sized lanes.
      if (index < 4) size = 0;
      // Top two bits of "zm" encode the index.
      zm_with_index |= (index & 3) << (Rm_offset + 3);
      op = op_h;
      break;
    case kSRegSizeInBytesLog2:
      VIXL_CHECK(zm.GetCode() <= 7);
      VIXL_CHECK(IsUint2(index));
      // Top two bits of "zm" encode the index.
      zm_with_index |= (index & 3) << (Rm_offset + 3);
      op = op_s;
      break;
    case kDRegSizeInBytesLog2:
      VIXL_CHECK(zm.GetCode() <= 15);
      VIXL_CHECK(IsUint1(index));
      // Top bit of "zm" encodes the index.
      zm_with_index |= (index & 1) << (Rm_offset + 4);
      op = op_d;
      break;
    default:
      VIXL_UNIMPLEMENTED();
  }
  return op | zm_with_index | size;
}

Instr Assembler::SVEMulLongIndexHelper(const ZRegister& zm, int index) {
  Instr imm_field;
  Instr zm_id;
  if (zm.IsLaneSizeH()) {
    VIXL_CHECK(zm.GetCode() <= 7);
    VIXL_CHECK(IsUint3(index));
    imm_field = ExtractUnsignedBitfield32(2, 1, index) << 19;
    zm_id = Rx<18, 16>(zm);
  } else {
    VIXL_ASSERT(zm.IsLaneSizeS());
    VIXL_CHECK(zm.GetCode() <= 15);
    VIXL_CHECK(IsUint2(index));
    imm_field = ExtractBit(index, 1) << 20;
    zm_id = Rx<19, 16>(zm);
  }

  // Synthesize the low part of immediate encoding.
  imm_field |= ExtractBit(index, 0) << 11;

  return zm_id | imm_field;
}

Instr Assembler::SVEMulComplexIndexHelper(const ZRegister& zm, int index) {
  Instr zm_idx_size;
  if (zm.IsLaneSizeH()) {
    // Zm<18:16> | i2<20:19>
    VIXL_CHECK(zm.GetCode() <= 7);
    VIXL_CHECK(IsUint2(index));
    zm_idx_size = (index << 19) | Rx<18, 16>(zm) | 0;
  } else {
    VIXL_ASSERT(zm.IsLaneSizeS());
    // Zm<19:16> | i1<20>
    VIXL_CHECK(zm.GetCode() <= 15);
    VIXL_CHECK(IsUint1(index));
    zm_idx_size = (index << 20) | Rx<19, 16>(zm) | (1 << 22);
  }
  return zm_idx_size;
}

// SVEFPMulAddIndex.

void Assembler::fmla(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  // The encoding of opcode, index, Zm, and size are synthesized in this
  // variable.
  Instr synthesized_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           FMLA_z_zzzi_h,
                                           FMLA_z_zzzi_s,
                                           FMLA_z_zzzi_d);

  Emit(synthesized_op | Rd(zda) | Rn(zn));
}

void Assembler::fmls(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  // The encoding of opcode, index, Zm, and size are synthesized in this
  // variable.
  Instr synthesized_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           FMLS_z_zzzi_h,
                                           FMLS_z_zzzi_s,
                                           FMLS_z_zzzi_d);

  Emit(synthesized_op | Rd(zda) | Rn(zn));
}

// SVEFPMulIndex.

// This prototype maps to 3 instruction encodings:
void Assembler::fmul(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     unsigned index) {
  // FMUL <Zd>.<T>, <Zn>.<T>, <Zm>.<T>[<imm>]
  //  0110 0100 ..1. .... 0010 00.. .... ....
  //  size<23:22> | opc<20:16> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  // The encoding of opcode, index, Zm, and size are synthesized in this
  // variable.
  Instr synthesized_op = SVEMulIndexHelper(zd.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           FMUL_z_zzi_h,
                                           FMUL_z_zzi_s,
                                           FMUL_z_zzi_d);

  Emit(synthesized_op | Rd(zd) | Rn(zn));
}

// SVEFPUnaryOpPredicated.

void Assembler::fcvt(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Instr op = 0xffffffff;
  switch (zn.GetLaneSizeInBytes()) {
    case kHRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kSRegSizeInBytes:
          op = FCVT_z_p_z_h2s;
          break;
        case kDRegSizeInBytes:
          op = FCVT_z_p_z_h2d;
          break;
      }
      break;
    case kSRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = FCVT_z_p_z_s2h;
          break;
        case kDRegSizeInBytes:
          op = FCVT_z_p_z_s2d;
          break;
      }
      break;
    case kDRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = FCVT_z_p_z_d2h;
          break;
        case kSRegSizeInBytes:
          op = FCVT_z_p_z_d2s;
          break;
      }
      break;
  }
  VIXL_ASSERT(op != 0xffffffff);

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcvtzs(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr op = 0xffffffff;
  switch (zn.GetLaneSizeInBytes()) {
    case kHRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = FCVTZS_z_p_z_fp162h;
          break;
        case kSRegSizeInBytes:
          op = FCVTZS_z_p_z_fp162w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZS_z_p_z_fp162x;
          break;
      }
      break;
    case kSRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kSRegSizeInBytes:
          op = FCVTZS_z_p_z_s2w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZS_z_p_z_s2x;
          break;
      }
      break;
    case kDRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kSRegSizeInBytes:
          op = FCVTZS_z_p_z_d2w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZS_z_p_z_d2x;
          break;
      }
      break;
  }
  VIXL_ASSERT(op != 0xffffffff);

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcvtzu(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr op = 0xffffffff;
  switch (zn.GetLaneSizeInBytes()) {
    case kHRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = FCVTZU_z_p_z_fp162h;
          break;
        case kSRegSizeInBytes:
          op = FCVTZU_z_p_z_fp162w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZU_z_p_z_fp162x;
          break;
      }
      break;
    case kSRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kSRegSizeInBytes:
          op = FCVTZU_z_p_z_s2w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZU_z_p_z_s2x;
          break;
      }
      break;
    case kDRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kSRegSizeInBytes:
          op = FCVTZU_z_p_z_d2w;
          break;
        case kDRegSizeInBytes:
          op = FCVTZU_z_p_z_d2x;
          break;
      }
      break;
  }
  VIXL_ASSERT(op != 0xffffffff);

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frecpx(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // FRECPX <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0110 0101 ..00 1100 101. .... .... ....
  //  size<23:22> | opc<17:16> = 00 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRECPX_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frinta(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTA_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frinti(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTI_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frintm(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTM_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frintn(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTN_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frintp(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTP_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frintx(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTX_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::frintz(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRINTZ_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fsqrt(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  // FSQRT <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0110 0101 ..00 1101 101. .... .... ....
  //  size<23:22> | opc<17:16> = 01 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FSQRT_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::scvtf(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr op = 0xffffffff;
  switch (zn.GetLaneSizeInBytes()) {
    case kHRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = SCVTF_z_p_z_h2fp16;
          break;
      }
      break;
    case kSRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = SCVTF_z_p_z_w2fp16;
          break;
        case kSRegSizeInBytes:
          op = SCVTF_z_p_z_w2s;
          break;
        case kDRegSizeInBytes:
          op = SCVTF_z_p_z_w2d;
          break;
      }
      break;
    case kDRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = SCVTF_z_p_z_x2fp16;
          break;
        case kSRegSizeInBytes:
          op = SCVTF_z_p_z_x2s;
          break;
        case kDRegSizeInBytes:
          op = SCVTF_z_p_z_x2d;
          break;
      }
      break;
  }
  VIXL_ASSERT(op != 0xffffffff);

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::ucvtf(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Instr op = 0xffffffff;
  switch (zn.GetLaneSizeInBytes()) {
    case kHRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = UCVTF_z_p_z_h2fp16;
          break;
      }
      break;
    case kSRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = UCVTF_z_p_z_w2fp16;
          break;
        case kSRegSizeInBytes:
          op = UCVTF_z_p_z_w2s;
          break;
        case kDRegSizeInBytes:
          op = UCVTF_z_p_z_w2d;
          break;
      }
      break;
    case kDRegSizeInBytes:
      switch (zd.GetLaneSizeInBytes()) {
        case kHRegSizeInBytes:
          op = UCVTF_z_p_z_x2fp16;
          break;
        case kSRegSizeInBytes:
          op = UCVTF_z_p_z_x2s;
          break;
        case kDRegSizeInBytes:
          op = UCVTF_z_p_z_x2d;
          break;
      }
      break;
  }
  VIXL_ASSERT(op != 0xffffffff);

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

// SVEFPUnaryOpUnpredicated.

void Assembler::frecpe(const ZRegister& zd, const ZRegister& zn) {
  // FRECPE <Zd>.<T>, <Zn>.<T>
  //  0110 0101 ..00 1110 0011 00.. .... ....
  //  size<23:22> | opc<18:16> = 110 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRECPE_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::frsqrte(const ZRegister& zd, const ZRegister& zn) {
  // FRSQRTE <Zd>.<T>, <Zn>.<T>
  //  0110 0101 ..00 1111 0011 00.. .... ....
  //  size<23:22> | opc<18:16> = 111 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FRSQRTE_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

// SVEIncDecByPredicateCount.

void Assembler::decp(const Register& rdn, const PRegisterWithLaneSize& pg) {
  // DECP <Xdn>, <Pg>.<T>
  //  0010 0101 ..10 1101 1000 100. .... ....
  //  size<23:22> | op<17> = 0 | D<16> = 1 | opc2<10:9> = 00 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rdn.IsX());

  Emit(DECP_r_p_r | SVESize(pg) | Rd(rdn) | Rx<8, 5>(pg));
}

void Assembler::decp(const ZRegister& zdn, const PRegister& pg) {
  // DECP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1101 1000 000. .... ....
  //  size<23:22> | op<17> = 0 | D<16> = 1 | opc2<10:9> = 00 | Pg<8:5> |
  //  Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(DECP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

void Assembler::incp(const Register& rdn, const PRegisterWithLaneSize& pg) {
  // INCP <Xdn>, <Pg>.<T>
  //  0010 0101 ..10 1100 1000 100. .... ....
  //  size<23:22> | op<17> = 0 | D<16> = 0 | opc2<10:9> = 00 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rdn.IsX());

  Emit(INCP_r_p_r | SVESize(pg) | Rd(rdn) | Rx<8, 5>(pg));
}

void Assembler::incp(const ZRegister& zdn, const PRegister& pg) {
  // INCP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1100 1000 000. .... ....
  //  size<23:22> | op<17> = 0 | D<16> = 0 | opc2<10:9> = 00 | Pg<8:5> |
  //  Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(INCP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

void Assembler::sqdecp(const Register& xd,
                       const PRegisterWithLaneSize& pg,
                       const Register& wn) {
  // SQDECP <Xdn>, <Pg>.<T>, <Wdn>
  //  0010 0101 ..10 1010 1000 100. .... ....
  //  size<23:22> | D<17> = 1 | U<16> = 0 | sf<10> = 0 | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  USE(wn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX() && wn.IsW() && xd.Aliases(wn));

  Emit(SQDECP_r_p_r_sx | SVESize(pg) | Rd(xd) | Rx<8, 5>(pg));
}

void Assembler::sqdecp(const Register& xdn, const PRegisterWithLaneSize& pg) {
  // SQDECP <Xdn>, <Pg>.<T>
  //  0010 0101 ..10 1010 1000 110. .... ....
  //  size<23:22> | D<17> = 1 | U<16> = 0 | sf<10> = 1 | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xdn.IsX());

  Emit(SQDECP_r_p_r_x | SVESize(pg) | Rd(xdn) | Rx<8, 5>(pg));
}

void Assembler::sqdecp(const ZRegister& zdn, const PRegister& pg) {
  // SQDECP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1010 1000 000. .... ....
  //  size<23:22> | D<17> = 1 | U<16> = 0 | opc<10:9> = 00 | Pg<8:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(SQDECP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

void Assembler::sqincp(const Register& xd,
                       const PRegisterWithLaneSize& pg,
                       const Register& wn) {
  // SQINCP <Xdn>, <Pg>.<T>, <Wdn>
  //  0010 0101 ..10 1000 1000 100. .... ....
  //  size<23:22> | D<17> = 0 | U<16> = 0 | sf<10> = 0 | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  USE(wn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX() && wn.IsW() && xd.Aliases(wn));

  Emit(SQINCP_r_p_r_sx | SVESize(pg) | Rd(xd) | Rx<8, 5>(pg));
}

void Assembler::sqincp(const Register& xdn, const PRegisterWithLaneSize& pg) {
  // SQINCP <Xdn>, <Pg>.<T>
  //  0010 0101 ..10 1000 1000 110. .... ....
  //  size<23:22> | D<17> = 0 | U<16> = 0 | sf<10> = 1 | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xdn.IsX());

  Emit(SQINCP_r_p_r_x | SVESize(pg) | Rd(xdn) | Rx<8, 5>(pg));
}

void Assembler::sqincp(const ZRegister& zdn, const PRegister& pg) {
  // SQINCP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1000 1000 000. .... ....
  //  size<23:22> | D<17> = 0 | U<16> = 0 | opc<10:9> = 00 | Pg<8:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(SQINCP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

void Assembler::uqdecp(const Register& rdn, const PRegisterWithLaneSize& pg) {
  // UQDECP <Wdn>, <Pg>.<T>
  // UQDECP <Xdn>, <Pg>.<T>
  //  0010 0101 ..10 1011 1000 10.. .... ....
  //  size<23:22> | D<17> = 1 | U<16> = 1 | sf<10> | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Instr op = rdn.IsX() ? UQDECP_r_p_r_x : UQDECP_r_p_r_uw;
  Emit(op | SVESize(pg) | Rd(rdn) | Rx<8, 5>(pg));
}

void Assembler::uqdecp(const ZRegister& zdn, const PRegister& pg) {
  // UQDECP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1011 1000 000. .... ....
  //  size<23:22> | D<17> = 1 | U<16> = 1 | opc<10:9> = 00 | Pg<8:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(UQDECP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

void Assembler::uqincp(const Register& rdn, const PRegisterWithLaneSize& pg) {
  // UQINCP <Wdn>, <Pg>.<T>
  //  0010 0101 ..10 1001 1000 100. .... ....
  //  size<23:22> | D<17> = 0 | U<16> = 1 | sf<10> = 0 | op<9> = 0 | Pg<8:5> |
  //  Rdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Instr op = rdn.IsX() ? UQINCP_r_p_r_x : UQINCP_r_p_r_uw;
  Emit(op | SVESize(pg) | Rd(rdn) | Rx<8, 5>(pg));
}

void Assembler::uqincp(const ZRegister& zdn, const PRegister& pg) {
  // UQINCP <Zdn>.<T>, <Pg>
  //  0010 0101 ..10 1001 1000 000. .... ....
  //  size<23:22> | D<17> = 0 | U<16> = 1 | opc<10:9> = 00 | Pg<8:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zdn.GetLaneSizeInBytes() != kBRegSizeInBytes);
  VIXL_ASSERT(pg.IsUnqualified());

  Emit(UQINCP_z_p_z | SVESize(zdn) | Rd(zdn) | Pg<8, 5>(pg));
}

// SVEIndexGeneration.

void Assembler::index(const ZRegister& zd, int start, int step) {
  // INDEX <Zd>.<T>, #<imm1>, #<imm2>
  //  0000 0100 ..1. .... 0100 00.. .... ....
  //  size<23:22> | step<20:16> | start<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(INDEX_z_ii | SVESize(zd) | ImmField<20, 16>(step) |
       ImmField<9, 5>(start) | Rd(zd));
}

void Assembler::index(const ZRegister& zd,
                      const Register& rn,
                      const Register& rm) {
  // INDEX <Zd>.<T>, <R><n>, <R><m>
  //  0000 0100 ..1. .... 0100 11.. .... ....
  //  size<23:22> | Rm<20:16> | Rn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(static_cast<unsigned>(rn.GetSizeInBits()) >=
              zd.GetLaneSizeInBits());
  VIXL_ASSERT(static_cast<unsigned>(rm.GetSizeInBits()) >=
              zd.GetLaneSizeInBits());

  Emit(INDEX_z_rr | SVESize(zd) | Rd(zd) | Rn(rn) | Rm(rm));
}

void Assembler::index(const ZRegister& zd, const Register& rn, int imm5) {
  // INDEX <Zd>.<T>, <R><n>, #<imm>
  //  0000 0100 ..1. .... 0100 01.. .... ....
  //  size<23:22> | imm5<20:16> | Rn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(static_cast<unsigned>(rn.GetSizeInBits()) >=
              zd.GetLaneSizeInBits());

  Emit(INDEX_z_ri | SVESize(zd) | Rd(zd) | Rn(rn) | ImmField<20, 16>(imm5));
}

void Assembler::index(const ZRegister& zd, int imm5, const Register& rm) {
  // INDEX <Zd>.<T>, #<imm>, <R><m>
  //  0000 0100 ..1. .... 0100 10.. .... ....
  //  size<23:22> | Rm<20:16> | imm5<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(static_cast<unsigned>(rm.GetSizeInBits()) >=
              zd.GetLaneSizeInBits());

  Emit(INDEX_z_ir | SVESize(zd) | Rd(zd) | ImmField<9, 5>(imm5) | Rm(rm));
}

// SVEIntArithmeticUnpredicated.

void Assembler::add(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // ADD <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0000 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 000 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(ADD_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqadd(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SQADD <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0001 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 100 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(SQADD_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqsub(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SQSUB <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0001 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 110 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(SQSUB_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sub(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // SUB <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0000 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 001 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(SUB_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uqadd(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UQADD <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0001 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 101 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(UQADD_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uqsub(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UQSUB <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0001 11.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 111 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(UQSUB_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

// SVEIntBinaryArithmeticPredicated.

void Assembler::add(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // ADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 0000 000. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(ADD_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::and_(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // AND <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 1010 000. .... .... ....
  //  size<23:22> | opc<18:16> = 010 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(AND_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::bic(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // BIC <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 1011 000. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(BIC_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::eor(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // EOR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 1001 000. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(EOR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::mul(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // MUL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0000 000. .... .... ....
  //  size<23:22> | H<17> = 0 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(MUL_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::orr(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // ORR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 1000 000. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(ORR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sabd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SABD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1100 000. .... .... ....
  //  size<23:22> | opc<18:17> = 10 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SABD_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sdiv(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SDIV <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0100 000. .... .... ....
  //  size<23:22> | R<17> = 0 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(SDIV_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sdivr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SDIVR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0110 000. .... .... ....
  //  size<23:22> | R<17> = 1 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(SDIVR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::smax(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SMAX <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1000 000. .... .... ....
  //  size<23:22> | opc<18:17> = 00 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SMAX_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::smin(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SMIN <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1010 000. .... .... ....
  //  size<23:22> | opc<18:17> = 01 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SMIN_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::smulh(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SMULH <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0010 000. .... .... ....
  //  size<23:22> | H<17> = 1 | U<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SMULH_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sub(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // SUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 0001 000. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SUB_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::subr(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 0011 000. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(SUBR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uabd(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UABD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1101 000. .... .... ....
  //  size<23:22> | opc<18:17> = 10 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(UABD_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::udiv(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UDIV <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0101 000. .... .... ....
  //  size<23:22> | R<17> = 0 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(UDIV_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::udivr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UDIVR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0111 000. .... .... ....
  //  size<23:22> | R<17> = 1 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(UDIVR_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::umax(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UMAX <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1001 000. .... .... ....
  //  size<23:22> | opc<18:17> = 00 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(UMAX_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::umin(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UMIN <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..00 1011 000. .... .... ....
  //  size<23:22> | opc<18:17> = 01 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(UMIN_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::umulh(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UMULH <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0000 0100 ..01 0011 000. .... .... ....
  //  size<23:22> | H<17> = 1 | U<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(UMULH_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

// SVEIntCompareScalars.

void Assembler::ctermeq(const Register& rn, const Register& rm) {
  // CTERMEQ <R><n>, <R><m>
  //  0010 0101 1.1. .... 0010 00.. ...0 0000
  //  op<23> = 1 | sz<22> | Rm<20:16> | Rn<9:5> | ne<4> = 0

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sz = rn.Is64Bits() ? 0x00400000 : 0x00000000;

  Emit(CTERMEQ_rr | sz | Rn(rn) | Rm(rm));
}

void Assembler::ctermne(const Register& rn, const Register& rm) {
  // CTERMNE <R><n>, <R><m>
  //  0010 0101 1.1. .... 0010 00.. ...1 0000
  //  op<23> = 1 | sz<22> | Rm<20:16> | Rn<9:5> | ne<4> = 1

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sz = rn.Is64Bits() ? 0x00400000 : 0x00000000;

  Emit(CTERMNE_rr | sz | Rn(rn) | Rm(rm));
}

void Assembler::whilele(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILELE <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 01.. ...1 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> = 0 | lt<10> = 1 | Rn<9:5> |
  //  eq<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(WHILELE_p_p_rr | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilelo(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILELO <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 11.. ...0 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> = 1 | lt<10> = 1 | Rn<9:5> |
  //  eq<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(WHILELO_p_p_rr | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilels(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILELS <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 11.. ...1 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> = 1 | lt<10> = 1 | Rn<9:5> |
  //  eq<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(WHILELS_p_p_rr | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilelt(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILELT <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 01.. ...0 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> = 0 | lt<10> = 1 | Rn<9:5> |
  //  eq<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(WHILELT_p_p_rr | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::CompareVectors(const PRegisterWithLaneSize& pd,
                               const PRegisterZ& pg,
                               const ZRegister& zn,
                               const ZRegister& zm,
                               SVEIntCompareVectorsOp op) {
  Emit(op | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::CompareVectors(const PRegisterWithLaneSize& pd,
                               const PRegisterZ& pg,
                               const ZRegister& zn,
                               int imm,
                               SVEIntCompareSignedImmOp op) {
  Emit(op | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) | ImmField<20, 16>(imm));
}

void Assembler::CompareVectors(const PRegisterWithLaneSize& pd,
                               const PRegisterZ& pg,
                               const ZRegister& zn,
                               unsigned imm,
                               SVEIntCompareUnsignedImmOp op) {
  Emit(op | SVESize(zn) | Pd(pd) | PgLow8(pg) | Rn(zn) |
       ImmUnsignedField<20, 14>(imm));
}

void Assembler::cmp(Condition cond,
                    const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  switch (cond) {
    case eq:
      cmpeq(pd, pg, zn, zm);
      break;
    case ge:
      cmpge(pd, pg, zn, zm);
      break;
    case gt:
      cmpgt(pd, pg, zn, zm);
      break;
    case le:
      cmple(pd, pg, zn, zm);
      break;
    case lt:
      cmplt(pd, pg, zn, zm);
      break;
    case ne:
      cmpne(pd, pg, zn, zm);
      break;
    case hi:
      cmphi(pd, pg, zn, zm);
      break;
    case hs:
      cmphs(pd, pg, zn, zm);
      break;
    case lo:
      cmplo(pd, pg, zn, zm);
      break;
    case ls:
      cmpls(pd, pg, zn, zm);
      break;
    default:
      VIXL_UNREACHABLE();
  }
}

// SVEIntCompareSignedImm.

void Assembler::cmpeq(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPEQ <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 100. .... ...0 ....
  //  size<23:22> | imm5<20:16> | op<15> = 1 | o2<13> = 0 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPEQ_p_p_zi);
}

void Assembler::cmpge(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPGE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 000. .... ...0 ....
  //  size<23:22> | imm5<20:16> | op<15> = 0 | o2<13> = 0 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPGE_p_p_zi);
}

void Assembler::cmpgt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPGT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 000. .... ...1 ....
  //  size<23:22> | imm5<20:16> | op<15> = 0 | o2<13> = 0 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPGT_p_p_zi);
}

void Assembler::cmple(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPLE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 001. .... ...1 ....
  //  size<23:22> | imm5<20:16> | op<15> = 0 | o2<13> = 1 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPLE_p_p_zi);
}

void Assembler::cmplt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPLT <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 001. .... ...0 ....
  //  size<23:22> | imm5<20:16> | op<15> = 0 | o2<13> = 1 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPLT_p_p_zi);
}

void Assembler::cmpne(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm5) {
  // CMPNE <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0101 ..0. .... 100. .... ...1 ....
  //  size<23:22> | imm5<20:16> | op<15> = 1 | o2<13> = 0 | Pg<12:10> | Zn<9:5>
  //  | ne<4> = 1 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm5, CMPNE_p_p_zi);
}

// SVEIntCompareUnsignedImm.

void Assembler::cmphi(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      unsigned imm7) {
  // CMPHI <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0100 ..1. .... ..0. .... ...1 ....
  //  size<23:22> | imm7<20:14> | lt<13> = 0 | Pg<12:10> | Zn<9:5> | ne<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm7, CMPHI_p_p_zi);
}

void Assembler::cmphs(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      unsigned imm7) {
  // CMPHS <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0100 ..1. .... ..0. .... ...0 ....
  //  size<23:22> | imm7<20:14> | lt<13> = 0 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm7, CMPHS_p_p_zi);
}

void Assembler::cmplo(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      unsigned imm7) {
  // CMPLO <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0100 ..1. .... ..1. .... ...0 ....
  //  size<23:22> | imm7<20:14> | lt<13> = 1 | Pg<12:10> | Zn<9:5> | ne<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm7, CMPLO_p_p_zi);
}

void Assembler::cmpls(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      unsigned imm7) {
  // CMPLS <Pd>.<T>, <Pg>/Z, <Zn>.<T>, #<imm>
  //  0010 0100 ..1. .... ..1. .... ...1 ....
  //  size<23:22> | imm7<20:14> | lt<13> = 1 | Pg<12:10> | Zn<9:5> | ne<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));

  CompareVectors(pd, pg, zn, imm7, CMPLS_p_p_zi);
}

// SVEIntCompareVectors.

// This prototype maps to 2 instruction encodings:
//  CMPEQ_p_p_zw
//  CMPEQ_p_p_zz
void Assembler::cmpeq(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPEQ_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPEQ_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

// This prototype maps to 2 instruction encodings:
//  CMPGE_p_p_zw
//  CMPGE_p_p_zz
void Assembler::cmpge(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPGE_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPGE_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

// This prototype maps to 2 instruction encodings:
//  CMPGT_p_p_zw
//  CMPGT_p_p_zz
void Assembler::cmpgt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPGT_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPGT_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

// This prototype maps to 2 instruction encodings:
//  CMPHI_p_p_zw
//  CMPHI_p_p_zz
void Assembler::cmphi(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPHI_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPHI_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

// This prototype maps to 2 instruction encodings:
//  CMPHS_p_p_zw
//  CMPHS_p_p_zz
void Assembler::cmphs(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPHS_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPHS_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

void Assembler::cmple(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  if (AreSameLaneSize(zn, zm)) {
    cmpge(pd, pg, zm, zn);
    return;
  }
  VIXL_ASSERT(zm.IsLaneSizeD());
  VIXL_ASSERT(!zn.IsLaneSizeD());

  CompareVectors(pd, pg, zn, zm, CMPLE_p_p_zw);
}

void Assembler::cmplo(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  if (AreSameLaneSize(zn, zm)) {
    cmphi(pd, pg, zm, zn);
    return;
  }
  VIXL_ASSERT(zm.IsLaneSizeD());
  VIXL_ASSERT(!zn.IsLaneSizeD());

  CompareVectors(pd, pg, zn, zm, CMPLO_p_p_zw);
}

void Assembler::cmpls(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  if (AreSameLaneSize(zn, zm)) {
    cmphs(pd, pg, zm, zn);
    return;
  }
  VIXL_ASSERT(zm.IsLaneSizeD());
  VIXL_ASSERT(!zn.IsLaneSizeD());

  CompareVectors(pd, pg, zn, zm, CMPLS_p_p_zw);
}

void Assembler::cmplt(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  if (AreSameLaneSize(zn, zm)) {
    cmpgt(pd, pg, zm, zn);
    return;
  }
  VIXL_ASSERT(zm.IsLaneSizeD());
  VIXL_ASSERT(!zn.IsLaneSizeD());

  CompareVectors(pd, pg, zn, zm, CMPLT_p_p_zw);
}

// This prototype maps to 2 instruction encodings:
//  CMPNE_p_p_zw
//  CMPNE_p_p_zz
void Assembler::cmpne(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, zn));
  SVEIntCompareVectorsOp op = CMPNE_p_p_zz;
  if (!AreSameLaneSize(zn, zm)) {
    VIXL_ASSERT(zm.IsLaneSizeD());
    op = CMPNE_p_p_zw;
  }
  CompareVectors(pd, pg, zn, zm, op);
}

// SVEIntMiscUnpredicated.

void Assembler::fexpa(const ZRegister& zd, const ZRegister& zn) {
  // FEXPA <Zd>.<T>, <Zn>.<T>
  //  0000 0100 ..10 0000 1011 10.. .... ....
  //  size<23:22> | opc<20:16> = 00000 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FEXPA_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::ftssel(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FTSSEL <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 1011 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<10> = 0 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FTSSEL_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::movprfx(const ZRegister& zd, const ZRegister& zn) {
  // MOVPRFX <Zd>, <Zn>
  //  0000 0100 0010 0000 1011 11.. .... ....
  //  opc<23:22> = 00 | opc2<20:16> = 00000 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(MOVPRFX_z_z | Rd(zd) | Rn(zn));
}

// SVEIntMulAddPredicated.

void Assembler::mad(const ZRegister& zdn,
                    const PRegisterM& pg,
                    const ZRegister& zm,
                    const ZRegister& za) {
  // MAD <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0000 0100 ..0. .... 110. .... .... ....
  //  size<23:22> | Zm<20:16> | op<13> = 0 | Pg<12:10> | Za<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));

  Emit(MAD_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rm(zm) | Rn(za));
}

void Assembler::mla(const ZRegister& zda,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // MLA <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..0. .... 010. .... .... ....
  //  size<23:22> | Zm<20:16> | op<13> = 0 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(MLA_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::mls(const ZRegister& zda,
                    const PRegisterM& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // MLS <Zda>.<T>, <Pg>/M, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..0. .... 011. .... .... ....
  //  size<23:22> | Zm<20:16> | op<13> = 1 | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(MLS_z_p_zzz | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::msb(const ZRegister& zdn,
                    const PRegisterM& pg,
                    const ZRegister& zm,
                    const ZRegister& za) {
  // MSB <Zdn>.<T>, <Pg>/M, <Zm>.<T>, <Za>.<T>
  //  0000 0100 ..0. .... 111. .... .... ....
  //  size<23:22> | Zm<20:16> | op<13> = 1 | Pg<12:10> | Za<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zdn, zm, za));

  Emit(MSB_z_p_zzz | SVESize(zdn) | Rd(zdn) | PgLow8(pg) | Rm(zm) | Rn(za));
}

// SVEIntMulAddUnpredicated.

void Assembler::sdot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));
  VIXL_ASSERT(AreSameLaneSize(zm, zn));

  Emit(SDOT_z_zzz | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::udot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));
  VIXL_ASSERT(AreSameLaneSize(zm, zn));

  Emit(UDOT_z_zzz | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// SVEIntReduction.

void Assembler::andv(const VRegister& vd,
                     const PRegister& pg,
                     const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(ANDV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::eorv(const VRegister& vd,
                     const PRegister& pg,
                     const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(EORV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::movprfx(const ZRegister& zd,
                        const PRegister& pg,
                        const ZRegister& zn) {
  // MOVPRFX <Zd>.<T>, <Pg>/<ZM>, <Zn>.<T>
  //  0000 0100 ..01 000. 001. .... .... ....
  //  size<23:22> | opc<18:17> = 00 | M<16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(pg.IsMerging() || pg.IsZeroing());
  VIXL_ASSERT(!pg.HasLaneSize());

  Instr m = pg.IsMerging() ? 0x00010000 : 0x00000000;
  Emit(MOVPRFX_z_p_z | SVESize(zd) | m | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::orv(const VRegister& vd,
                    const PRegister& pg,
                    const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(ORV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::saddv(const VRegister& dd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kDRegSizeInBytes);

  Emit(SADDV_r_p_z | SVESize(zn) | Rd(dd) | PgLow8(pg) | Rn(zn));
}

void Assembler::smaxv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(SMAXV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sminv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(SMINV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::uaddv(const VRegister& dd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(UADDV_r_p_z | SVESize(zn) | Rd(dd) | PgLow8(pg) | Rn(zn));
}

void Assembler::umaxv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(UMAXV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::uminv(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(UMINV_r_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

// SVEIntUnaryArithmeticPredicated.

void Assembler::abs(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  // ABS <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0110 101. .... .... ....
  //  size<23:22> | opc<18:16> = 110 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(ABS_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::cls(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  // CLS <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1000 101. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(CLS_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::clz(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  // CLZ <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1001 101. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(CLZ_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::cnot(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // CNOT <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1011 101. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(CNOT_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::cnt(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  // CNT <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1010 101. .... .... ....
  //  size<23:22> | opc<18:16> = 010 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(CNT_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fabs(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // FABS <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1100 101. .... .... ....
  //  size<23:22> | opc<18:16> = 100 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FABS_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fneg(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // FNEG <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1101 101. .... .... ....
  //  size<23:22> | opc<18:16> = 101 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(FNEG_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::neg(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  // NEG <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0111 101. .... .... ....
  //  size<23:22> | opc<18:16> = 111 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(NEG_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::not_(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // NOT <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 1110 101. .... .... ....
  //  size<23:22> | opc<18:16> = 110 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(NOT_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sxtb(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // SXTB <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0000 101. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kBRegSizeInBytes);

  Emit(SXTB_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sxth(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // SXTH <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0010 101. .... .... ....
  //  size<23:22> | opc<18:16> = 010 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kHRegSizeInBytes);

  Emit(SXTH_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sxtw(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // SXTW <Zd>.D, <Pg>/M, <Zn>.D
  //  0000 0100 ..01 0100 101. .... .... ....
  //  size<23:22> | opc<18:16> = 100 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kSRegSizeInBytes);

  Emit(SXTW_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::uxtb(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // UXTB <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0001 101. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kBRegSizeInBytes);

  Emit(UXTB_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::uxth(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // UXTH <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0100 ..01 0011 101. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kHRegSizeInBytes);

  Emit(UXTH_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::uxtw(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // UXTW <Zd>.D, <Pg>/M, <Zn>.D
  //  0000 0100 ..01 0101 101. .... .... ....
  //  size<23:22> | opc<18:16> = 101 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() > kSRegSizeInBytes);

  Emit(UXTW_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

// SVEIntWideImmPredicated.

void Assembler::cpy(const ZRegister& zd,
                    const PRegister& pg,
                    int imm8,
                    int shift) {
  // CPY <Zd>.<T>, <Pg>/<ZM>, #<imm>{, <shift>}
  //  0000 0101 ..01 .... 0... .... .... ....
  //  size<23:22> | Pg<19:16> | M<14> | sh<13> | imm8<12:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pg.IsMerging() || pg.IsZeroing());

  ResolveSVEImm8Shift(&imm8, &shift);

  Instr sh = (shift > 0) ? (1 << 13) : 0;
  Instr m = pg.IsMerging() ? (1 << 14) : 0;
  Emit(CPY_z_p_i | m | sh | SVESize(zd) | Rd(zd) | Pg<19, 16>(pg) |
       ImmField<12, 5>(imm8));
}

void Assembler::fcpy(const ZRegister& zd, const PRegisterM& pg, double imm) {
  // FCPY <Zd>.<T>, <Pg>/M, #<const>
  //  0000 0101 ..01 .... 110. .... .... ....
  //  size<23:22> | Pg<19:16> | imm8<12:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Instr imm_field = ImmUnsignedField<12, 5>(FP64ToImm8(imm));
  Emit(FCPY_z_p_i | SVESize(zd) | Rd(zd) | Pg<19, 16>(pg) | imm_field);
}

// SVEIntAddSubtractImmUnpredicated.

void Assembler::SVEIntAddSubtractImmUnpredicatedHelper(
    SVEIntAddSubtractImm_UnpredicatedOp op,
    const ZRegister& zd,
    int imm8,
    int shift) {
  if (shift < 0) {
    VIXL_ASSERT(shift == -1);
    // Derive the shift amount from the immediate.
    if (IsUint8(imm8)) {
      shift = 0;
    } else if (IsUint16(imm8) && ((imm8 % 256) == 0)) {
      imm8 /= 256;
      shift = 8;
    }
  }

  VIXL_ASSERT(IsUint8(imm8));
  VIXL_ASSERT((shift == 0) || (shift == 8));

  Instr shift_bit = (shift > 0) ? (1 << 13) : 0;
  Emit(op | SVESize(zd) | Rd(zd) | shift_bit | ImmUnsignedField<12, 5>(imm8));
}

void Assembler::add(const ZRegister& zd,
                    const ZRegister& zn,
                    int imm8,
                    int shift) {
  // ADD <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0000 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(ADD_z_zi, zd, imm8, shift);
}

void Assembler::dup(const ZRegister& zd, int imm8, int shift) {
  // DUP <Zd>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..11 1000 11.. .... .... ....
  //  size<23:22> | opc<18:17> = 00 | sh<13> | imm8<12:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  ResolveSVEImm8Shift(&imm8, &shift);
  VIXL_ASSERT((shift < 8) || !zd.IsLaneSizeB());

  Instr shift_bit = (shift > 0) ? (1 << 13) : 0;
  Emit(DUP_z_i | SVESize(zd) | Rd(zd) | shift_bit | ImmField<12, 5>(imm8));
}

void Assembler::fdup(const ZRegister& zd, double imm) {
  // FDUP <Zd>.<T>, #<const>
  //  0010 0101 ..11 1001 110. .... .... ....
  //  size<23:22> | opc<18:17> = 00 | o2<13> = 0 | imm8<12:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Instr encoded_imm = FP64ToImm8(imm) << 5;
  Emit(FDUP_z_i | SVESize(zd) | encoded_imm | Rd(zd));
}

void Assembler::mul(const ZRegister& zd, const ZRegister& zn, int imm8) {
  // MUL <Zdn>.<T>, <Zdn>.<T>, #<imm>
  //  0010 0101 ..11 0000 110. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | o2<13> = 0 | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(MUL_z_zi | SVESize(zd) | Rd(zd) | ImmField<12, 5>(imm8));
}

void Assembler::smax(const ZRegister& zd, const ZRegister& zn, int imm8) {
  // SMAX <Zdn>.<T>, <Zdn>.<T>, #<imm>
  //  0010 0101 ..10 1000 110. .... .... ....
  //  size<23:22> | opc<18:16> = 000 | o2<13> = 0 | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(SMAX_z_zi | SVESize(zd) | Rd(zd) | ImmField<12, 5>(imm8));
}

void Assembler::smin(const ZRegister& zd, const ZRegister& zn, int imm8) {
  // SMIN <Zdn>.<T>, <Zdn>.<T>, #<imm>
  //  0010 0101 ..10 1010 110. .... .... ....
  //  size<23:22> | opc<18:16> = 010 | o2<13> = 0 | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(SMIN_z_zi | SVESize(zd) | Rd(zd) | ImmField<12, 5>(imm8));
}

void Assembler::sqadd(const ZRegister& zd,
                      const ZRegister& zn,
                      int imm8,
                      int shift) {
  // SQADD <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0100 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 100 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(SQADD_z_zi, zd, imm8, shift);
}

void Assembler::sqsub(const ZRegister& zd,
                      const ZRegister& zn,
                      int imm8,
                      int shift) {
  // SQSUB <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0110 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 110 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(SQSUB_z_zi, zd, imm8, shift);
}

void Assembler::sub(const ZRegister& zd,
                    const ZRegister& zn,
                    int imm8,
                    int shift) {
  // SUB <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0001 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(SUB_z_zi, zd, imm8, shift);
}

void Assembler::subr(const ZRegister& zd,
                     const ZRegister& zn,
                     int imm8,
                     int shift) {
  // SUBR <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0011 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(SUBR_z_zi, zd, imm8, shift);
}

void Assembler::umax(const ZRegister& zd, const ZRegister& zn, int imm8) {
  // UMAX <Zdn>.<T>, <Zdn>.<T>, #<imm>
  //  0010 0101 ..10 1001 110. .... .... ....
  //  size<23:22> | opc<18:16> = 001 | o2<13> = 0 | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(UMAX_z_zi | SVESize(zd) | Rd(zd) | ImmUnsignedField<12, 5>(imm8));
}

void Assembler::umin(const ZRegister& zd, const ZRegister& zn, int imm8) {
  // UMIN <Zdn>.<T>, <Zdn>.<T>, #<imm>
  //  0010 0101 ..10 1011 110. .... .... ....
  //  size<23:22> | opc<18:16> = 011 | o2<13> = 0 | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(UMIN_z_zi | SVESize(zd) | Rd(zd) | ImmUnsignedField<12, 5>(imm8));
}

void Assembler::uqadd(const ZRegister& zd,
                      const ZRegister& zn,
                      int imm8,
                      int shift) {
  // UQADD <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0101 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 101 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(UQADD_z_zi, zd, imm8, shift);
}

void Assembler::uqsub(const ZRegister& zd,
                      const ZRegister& zn,
                      int imm8,
                      int shift) {
  // UQSUB <Zdn>.<T>, <Zdn>.<T>, #<imm>{, <shift>}
  //  0010 0101 ..10 0111 11.. .... .... ....
  //  size<23:22> | opc<18:16> = 111 | sh<13> | imm8<12:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  SVEIntAddSubtractImmUnpredicatedHelper(UQSUB_z_zi, zd, imm8, shift);
}

// SVEMemLoad.

void Assembler::SVELdSt1Helper(unsigned msize_in_bytes_log2,
                               const ZRegister& zt,
                               const PRegister& pg,
                               const SVEMemOperand& addr,
                               bool is_signed,
                               Instr op) {
  VIXL_ASSERT(addr.IsContiguous());

  Instr mem_op = SVEMemOperandHelper(msize_in_bytes_log2, 1, addr);
  Instr dtype =
      SVEDtype(msize_in_bytes_log2, zt.GetLaneSizeInBytesLog2(), is_signed);
  Emit(op | mem_op | dtype | Rt(zt) | PgLow8(pg));
}

void Assembler::SVELdSt234Helper(int num_regs,
                                 const ZRegister& zt1,
                                 const PRegister& pg,
                                 const SVEMemOperand& addr,
                                 Instr op) {
  VIXL_ASSERT((num_regs >= 2) && (num_regs <= 4));

  unsigned msize_in_bytes_log2 = zt1.GetLaneSizeInBytesLog2();
  Instr num = (num_regs - 1) << 21;
  Instr msz = msize_in_bytes_log2 << 23;
  Instr mem_op = SVEMemOperandHelper(msize_in_bytes_log2, num_regs, addr);
  Emit(op | mem_op | msz | num | Rt(zt1) | PgLow8(pg));
}

void Assembler::SVELd1Helper(unsigned msize_in_bytes_log2,
                             const ZRegister& zt,
                             const PRegisterZ& pg,
                             const SVEMemOperand& addr,
                             bool is_signed) {
  VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() >= msize_in_bytes_log2);
  if (is_signed) {
    // Sign-extension is only possible when the vector elements are larger than
    // the elements in memory.
    VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() != msize_in_bytes_log2);
  }

  if (addr.IsScatterGather()) {
    bool is_load = true;
    bool is_ff = false;
    SVEScatterGatherHelper(msize_in_bytes_log2,
                           zt,
                           pg,
                           addr,
                           is_load,
                           is_signed,
                           is_ff);
    return;
  }

  Instr op = 0xffffffff;
  if (addr.IsScalarPlusImmediate()) {
    op = SVEContiguousLoad_ScalarPlusImmFixed;
  } else if (addr.IsScalarPlusScalar()) {
    // Rm must not be xzr.
    VIXL_ASSERT(!addr.GetScalarOffset().IsZero());
    op = SVEContiguousLoad_ScalarPlusScalarFixed;
  } else {
    VIXL_UNIMPLEMENTED();
  }
  SVELdSt1Helper(msize_in_bytes_log2, zt, pg, addr, is_signed, op);
}

void Assembler::SVELdff1Helper(unsigned msize_in_bytes_log2,
                               const ZRegister& zt,
                               const PRegisterZ& pg,
                               const SVEMemOperand& addr,
                               bool is_signed) {
  VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() >= msize_in_bytes_log2);
  if (is_signed) {
    // Sign-extension is only possible when the vector elements are larger than
    // the elements in memory.
    VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() != msize_in_bytes_log2);
  }

  if (addr.IsScatterGather()) {
    bool is_load = true;
    bool is_ff = true;
    SVEScatterGatherHelper(msize_in_bytes_log2,
                           zt,
                           pg,
                           addr,
                           is_load,
                           is_signed,
                           is_ff);
    return;
  }

  if (addr.IsPlainScalar()) {
    // SVEMemOperand(x0) is treated as a scalar-plus-immediate form ([x0, #0]).
    // In these instructions, we want to treat it as [x0, xzr].
    SVEMemOperand addr_scalar_plus_scalar(addr.GetScalarBase(), xzr);
    // Guard against infinite recursion.
    VIXL_ASSERT(!addr_scalar_plus_scalar.IsPlainScalar());
    SVELdff1Helper(msize_in_bytes_log2,
                   zt,
                   pg,
                   addr_scalar_plus_scalar,
                   is_signed);
    return;
  }

  Instr op = 0xffffffff;
  if (addr.IsScalarPlusScalar()) {
    op = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed;
  } else {
    VIXL_UNIMPLEMENTED();
  }
  SVELdSt1Helper(msize_in_bytes_log2, zt, pg, addr, is_signed, op);
}

void Assembler::SVEScatterGatherHelper(unsigned msize_in_bytes_log2,
                                       const ZRegister& zt,
                                       const PRegister& pg,
                                       const SVEMemOperand& addr,
                                       bool is_load,
                                       bool is_signed,
                                       bool is_first_fault) {
  VIXL_ASSERT(addr.IsScatterGather());
  VIXL_ASSERT(zt.IsLaneSizeS() || zt.IsLaneSizeD());
  VIXL_ASSERT(is_load || !is_first_fault);
  VIXL_ASSERT(is_load || !is_signed);

  Instr op = 0xffffffff;
  if (addr.IsVectorPlusImmediate()) {
    VIXL_ASSERT(AreSameLaneSize(zt, addr.GetVectorBase()));
    if (is_load) {
      if (zt.IsLaneSizeS()) {
        op = SVE32BitGatherLoad_VectorPlusImmFixed;
      } else {
        op = SVE64BitGatherLoad_VectorPlusImmFixed;
      }
    } else {
      if (zt.IsLaneSizeS()) {
        op = SVE32BitScatterStore_VectorPlusImmFixed;
      } else {
        op = SVE64BitScatterStore_VectorPlusImmFixed;
      }
    }
  } else {
    VIXL_ASSERT(addr.IsScalarPlusVector());
    VIXL_ASSERT(AreSameLaneSize(zt, addr.GetVectorOffset()));
    SVEOffsetModifier mod = addr.GetOffsetModifier();
    if (zt.IsLaneSizeS()) {
      VIXL_ASSERT((mod == SVE_UXTW) || (mod == SVE_SXTW));
      unsigned shift_amount = addr.GetShiftAmount();
      if (shift_amount == 0) {
        if (is_load) {
          op = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed;
        } else {
          op = SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFixed;
        }
      } else if (shift_amount == 1) {
        VIXL_ASSERT(msize_in_bytes_log2 == kHRegSizeInBytesLog2);
        if (is_load) {
          op = SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed;
        } else {
          op = SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFixed;
        }
      } else {
        VIXL_ASSERT(shift_amount == 2);
        VIXL_ASSERT(msize_in_bytes_log2 == kSRegSizeInBytesLog2);
        if (is_load) {
          op = SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsFixed;
        } else {
          op = SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFixed;
        }
      }
    } else if (zt.IsLaneSizeD()) {
      switch (mod) {
        case NO_SVE_OFFSET_MODIFIER:
          if (is_load) {
            op = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed;
          } else {
            op = SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed;
          }
          break;
        case SVE_LSL:
          if (is_load) {
            op = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed;
          } else {
            op = SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFixed;
          }
          break;
        case SVE_UXTW:
        case SVE_SXTW: {
          unsigned shift_amount = addr.GetShiftAmount();
          if (shift_amount == 0) {
            if (is_load) {
              op =
                  SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed;
            } else {
              op =
                  SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed;
            }
          } else {
            VIXL_ASSERT(shift_amount == msize_in_bytes_log2);
            if (is_load) {
              op = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed;
            } else {
              op =
                  SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFixed;
            }
          }
          break;
        }
        default:
          VIXL_UNIMPLEMENTED();
      }
    }
  }

  Instr mem_op = SVEMemOperandHelper(msize_in_bytes_log2, 1, addr, is_load);
  Instr msz = ImmUnsignedField<24, 23>(msize_in_bytes_log2);
  Instr u = (!is_load || is_signed) ? 0 : (1 << 14);
  Instr ff = is_first_fault ? (1 << 13) : 0;
  Emit(op | mem_op | msz | u | ff | Rt(zt) | PgLow8(pg));
}

void Assembler::SVELd234Helper(int num_regs,
                               const ZRegister& zt1,
                               const PRegisterZ& pg,
                               const SVEMemOperand& addr) {
  if (addr.IsScalarPlusScalar()) {
    // Rm must not be xzr.
    VIXL_ASSERT(!addr.GetScalarOffset().IsZero());
  }

  Instr op;
  if (addr.IsScalarPlusImmediate()) {
    op = SVELoadMultipleStructures_ScalarPlusImmFixed;
  } else if (addr.IsScalarPlusScalar()) {
    op = SVELoadMultipleStructures_ScalarPlusScalarFixed;
  } else {
    // These instructions don't support any other addressing modes.
    VIXL_ABORT();
  }
  SVELdSt234Helper(num_regs, zt1, pg, addr, op);
}

// SVEMemContiguousLoad.

#define VIXL_DEFINE_LD1(MSZ, LANE_SIZE)                                  \
  void Assembler::ld1##MSZ(const ZRegister& zt,                          \
                           const PRegisterZ& pg,                         \
                           const SVEMemOperand& addr) {                  \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                              \
    SVELd1Helper(k##LANE_SIZE##RegSizeInBytesLog2, zt, pg, addr, false); \
  }
#define VIXL_DEFINE_LD2(MSZ, LANE_SIZE)                 \
  void Assembler::ld2##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const PRegisterZ& pg,        \
                           const SVEMemOperand& addr) { \
    USE(zt2);                                           \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2));              \
    VIXL_ASSERT(AreSameFormat(zt1, zt2));               \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVELd234Helper(2, zt1, pg, addr);                   \
  }
#define VIXL_DEFINE_LD3(MSZ, LANE_SIZE)                 \
  void Assembler::ld3##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const ZRegister& zt3,        \
                           const PRegisterZ& pg,        \
                           const SVEMemOperand& addr) { \
    USE(zt2, zt3);                                      \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2, zt3));         \
    VIXL_ASSERT(AreSameFormat(zt1, zt2, zt3));          \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVELd234Helper(3, zt1, pg, addr);                   \
  }
#define VIXL_DEFINE_LD4(MSZ, LANE_SIZE)                 \
  void Assembler::ld4##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const ZRegister& zt3,        \
                           const ZRegister& zt4,        \
                           const PRegisterZ& pg,        \
                           const SVEMemOperand& addr) { \
    USE(zt2, zt3, zt4);                                 \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2, zt3, zt4));    \
    VIXL_ASSERT(AreSameFormat(zt1, zt2, zt3, zt4));     \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVELd234Helper(4, zt1, pg, addr);                   \
  }

VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_LD1)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_LD2)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_LD3)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_LD4)

#define VIXL_DEFINE_LD1S(MSZ, LANE_SIZE)                                \
  void Assembler::ld1s##MSZ(const ZRegister& zt,                        \
                            const PRegisterZ& pg,                       \
                            const SVEMemOperand& addr) {                \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                             \
    SVELd1Helper(k##LANE_SIZE##RegSizeInBytesLog2, zt, pg, addr, true); \
  }
VIXL_SVE_LOAD_STORE_SIGNED_VARIANT_LIST(VIXL_DEFINE_LD1S)

// SVEMem32BitGatherAndUnsizedContiguous.

void Assembler::SVELd1BroadcastHelper(unsigned msize_in_bytes_log2,
                                      const ZRegister& zt,
                                      const PRegisterZ& pg,
                                      const SVEMemOperand& addr,
                                      bool is_signed) {
  VIXL_ASSERT(addr.IsScalarPlusImmediate());
  VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() >= msize_in_bytes_log2);
  if (is_signed) {
    // Sign-extension is only possible when the vector elements are larger than
    // the elements in memory.
    VIXL_ASSERT(zt.GetLaneSizeInBytesLog2() != msize_in_bytes_log2);
  }

  int64_t imm = addr.GetImmediateOffset();
  int divisor = 1 << msize_in_bytes_log2;
  VIXL_ASSERT(imm % divisor == 0);
  Instr dtype = SVEDtypeSplit(msize_in_bytes_log2,
                              zt.GetLaneSizeInBytesLog2(),
                              is_signed);

  Emit(SVELoadAndBroadcastElementFixed | dtype | RnSP(addr.GetScalarBase()) |
       ImmUnsignedField<21, 16>(imm / divisor) | Rt(zt) | PgLow8(pg));
}

// This prototype maps to 4 instruction encodings:
//  LD1RB_z_p_bi_u16
//  LD1RB_z_p_bi_u32
//  LD1RB_z_p_bi_u64
//  LD1RB_z_p_bi_u8
void Assembler::ld1rb(const ZRegister& zt,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kBRegSizeInBytesLog2, zt, pg, addr, false);
}

// This prototype maps to 3 instruction encodings:
//  LD1RH_z_p_bi_u16
//  LD1RH_z_p_bi_u32
//  LD1RH_z_p_bi_u64
void Assembler::ld1rh(const ZRegister& zt,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kHRegSizeInBytesLog2, zt, pg, addr, false);
}

// This prototype maps to 2 instruction encodings:
//  LD1RW_z_p_bi_u32
//  LD1RW_z_p_bi_u64
void Assembler::ld1rw(const ZRegister& zt,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kSRegSizeInBytesLog2, zt, pg, addr, false);
}

void Assembler::ld1rd(const ZRegister& zt,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kDRegSizeInBytesLog2, zt, pg, addr, false);
}

// This prototype maps to 3 instruction encodings:
//  LD1RSB_z_p_bi_s16
//  LD1RSB_z_p_bi_s32
//  LD1RSB_z_p_bi_s64
void Assembler::ld1rsb(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kBRegSizeInBytesLog2, zt, pg, addr, true);
}

// This prototype maps to 2 instruction encodings:
//  LD1RSH_z_p_bi_s32
//  LD1RSH_z_p_bi_s64
void Assembler::ld1rsh(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kHRegSizeInBytesLog2, zt, pg, addr, true);
}

void Assembler::ld1rsw(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  SVELd1BroadcastHelper(kWRegSizeInBytesLog2, zt, pg, addr, true);
}

void Assembler::ldr(const CPURegister& rt, const SVEMemOperand& addr) {
  // LDR <Pt/Zt>, [<Xn|SP>{, #<imm>, MUL VL}]

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rt.IsPRegister() || rt.IsZRegister());
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));
  int64_t imm9 = addr.GetImmediateOffset();
  VIXL_ASSERT(IsInt9(imm9));
  Instr imm9l = ExtractUnsignedBitfield32(2, 0, imm9) << 10;
  Instr imm9h = ExtractUnsignedBitfield32(8, 3, imm9) << 16;

  Instr op = LDR_z_bi;
  if (rt.IsPRegister()) {
    op = LDR_p_bi;
  }
  Emit(op | Rt(rt) | RnSP(addr.GetScalarBase()) | imm9h | imm9l);
}

// SVEMem64BitGather.

// This prototype maps to 3 instruction encodings:
//  LDFF1B_z_p_bz_d_64_unscaled
//  LDFF1B_z_p_bz_d_x32_unscaled
void Assembler::ldff1b(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const Register& xn,
                       const ZRegister& zm) {
  // LDFF1B { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D]
  //  1100 0100 010. .... 111. .... .... ....
  //  msz<24:23> = 00 | Zm<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1B_z_p_bz_d_64_unscaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  LDFF1B_z_p_ai_d
//  LDFF1B_z_p_ai_s
void Assembler::ldff1b(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const ZRegister& zn,
                       int imm5) {
  // LDFF1B { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0100 001. .... 111. .... .... ....
  //  msz<24:23> = 00 | imm5<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1B_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) | ImmField<20, 16>(imm5));
}

// This prototype maps to 4 instruction encodings:
//  LDFF1D_z_p_bz_d_64_scaled
//  LDFF1D_z_p_bz_d_64_unscaled
//  LDFF1D_z_p_bz_d_x32_scaled
//  LDFF1D_z_p_bz_d_x32_unscaled
void Assembler::ldff1d(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const Register& xn,
                       const ZRegister& zm) {
  // LDFF1D { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D, LSL #3]
  //  1100 0101 111. .... 111. .... .... ....
  //  msz<24:23> = 11 | Zm<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1D_z_p_bz_d_64_scaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

void Assembler::ldff1d(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const ZRegister& zn,
                       int imm5) {
  // LDFF1D { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0101 101. .... 111. .... .... ....
  //  msz<24:23> = 11 | imm5<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1D_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) | ImmField<20, 16>(imm5));
}

// This prototype maps to 6 instruction encodings:
//  LDFF1H_z_p_bz_d_64_scaled
//  LDFF1H_z_p_bz_d_64_unscaled
//  LDFF1H_z_p_bz_d_x32_scaled
//  LDFF1H_z_p_bz_d_x32_unscaled
void Assembler::ldff1h(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const Register& xn,
                       const ZRegister& zm) {
  // LDFF1H { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D, LSL #1]
  //  1100 0100 111. .... 111. .... .... ....
  //  msz<24:23> = 01 | Zm<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1H_z_p_bz_d_64_scaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  LDFF1H_z_p_ai_d
//  LDFF1H_z_p_ai_s
void Assembler::ldff1h(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const ZRegister& zn,
                       int imm5) {
  // LDFF1H { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0100 101. .... 111. .... .... ....
  //  msz<24:23> = 01 | imm5<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1H_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) | ImmField<20, 16>(imm5));
}

// This prototype maps to 3 instruction encodings:
//  LDFF1SB_z_p_bz_d_64_unscaled
//  LDFF1SB_z_p_bz_d_x32_unscaled
void Assembler::ldff1sb(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const Register& xn,
                        const ZRegister& zm) {
  // LDFF1SB { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D]
  //  1100 0100 010. .... 101. .... .... ....
  //  msz<24:23> = 00 | Zm<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SB_z_p_bz_d_64_unscaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  LDFF1SB_z_p_ai_d
//  LDFF1SB_z_p_ai_s
void Assembler::ldff1sb(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const ZRegister& zn,
                        int imm5) {
  // LDFF1SB { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0100 001. .... 101. .... .... ....
  //  msz<24:23> = 00 | imm5<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SB_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) |
       ImmField<20, 16>(imm5));
}

// This prototype maps to 6 instruction encodings:
//  LDFF1SH_z_p_bz_d_64_scaled
//  LDFF1SH_z_p_bz_d_64_unscaled
//  LDFF1SH_z_p_bz_d_x32_scaled
//  LDFF1SH_z_p_bz_d_x32_unscaled
void Assembler::ldff1sh(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const Register& xn,
                        const ZRegister& zm) {
  // LDFF1SH { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D, LSL #1]
  //  1100 0100 111. .... 101. .... .... ....
  //  msz<24:23> = 01 | Zm<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SH_z_p_bz_d_64_scaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  LDFF1SH_z_p_ai_d
//  LDFF1SH_z_p_ai_s
void Assembler::ldff1sh(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const ZRegister& zn,
                        int imm5) {
  // LDFF1SH { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0100 101. .... 101. .... .... ....
  //  msz<24:23> = 01 | imm5<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SH_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) |
       ImmField<20, 16>(imm5));
}

// This prototype maps to 4 instruction encodings:
//  LDFF1SW_z_p_bz_d_64_scaled
//  LDFF1SW_z_p_bz_d_64_unscaled
//  LDFF1SW_z_p_bz_d_x32_scaled
//  LDFF1SW_z_p_bz_d_x32_unscaled
void Assembler::ldff1sw(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const Register& xn,
                        const ZRegister& zm) {
  // LDFF1SW { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D, LSL #2]
  //  1100 0101 011. .... 101. .... .... ....
  //  msz<24:23> = 10 | Zm<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SW_z_p_bz_d_64_scaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

void Assembler::ldff1sw(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const ZRegister& zn,
                        int imm5) {
  // LDFF1SW { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0101 001. .... 101. .... .... ....
  //  msz<24:23> = 10 | imm5<20:16> | U<14> = 0 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1SW_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) |
       ImmField<20, 16>(imm5));
}

// This prototype maps to 6 instruction encodings:
//  LDFF1W_z_p_bz_d_64_scaled
//  LDFF1W_z_p_bz_d_64_unscaled
//  LDFF1W_z_p_bz_d_x32_scaled
//  LDFF1W_z_p_bz_d_x32_unscaled
void Assembler::ldff1w(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const Register& xn,
                       const ZRegister& zm) {
  // LDFF1W { <Zt>.D }, <Pg>/Z, [<Xn|SP>, <Zm>.D, LSL #2]
  //  1100 0101 011. .... 111. .... .... ....
  //  msz<24:23> = 10 | Zm<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> | Rn<9:5>
  //  | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1W_z_p_bz_d_64_scaled | Rt(zt) | PgLow8(pg) | RnSP(xn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  LDFF1W_z_p_ai_d
//  LDFF1W_z_p_ai_s
void Assembler::ldff1w(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const ZRegister& zn,
                       int imm5) {
  // LDFF1W { <Zt>.D }, <Pg>/Z, [<Zn>.D{, #<imm>}]
  //  1100 0101 001. .... 111. .... .... ....
  //  msz<24:23> = 10 | imm5<20:16> | U<14> = 1 | ff<13> = 1 | Pg<12:10> |
  //  Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LDFF1W_z_p_ai_d | Rt(zt) | PgLow8(pg) | Rn(zn) | ImmField<20, 16>(imm5));
}

void Assembler::SVEGatherPrefetchVectorPlusImmediateHelper(
    PrefetchOperation prfop,
    const PRegister& pg,
    const SVEMemOperand& addr,
    int prefetch_size) {
  VIXL_ASSERT(addr.IsVectorPlusImmediate());
  ZRegister zn = addr.GetVectorBase();
  VIXL_ASSERT(zn.IsLaneSizeS() || zn.IsLaneSizeD());

  Instr op = 0xffffffff;
  switch (prefetch_size) {
    case kBRegSize:
      op = zn.IsLaneSizeS() ? static_cast<Instr>(PRFB_i_p_ai_s)
                            : static_cast<Instr>(PRFB_i_p_ai_d);
      break;
    case kHRegSize:
      op = zn.IsLaneSizeS() ? static_cast<Instr>(PRFH_i_p_ai_s)
                            : static_cast<Instr>(PRFH_i_p_ai_d);
      break;
    case kSRegSize:
      op = zn.IsLaneSizeS() ? static_cast<Instr>(PRFW_i_p_ai_s)
                            : static_cast<Instr>(PRFW_i_p_ai_d);
      break;
    case kDRegSize:
      op = zn.IsLaneSizeS() ? static_cast<Instr>(PRFD_i_p_ai_s)
                            : static_cast<Instr>(PRFD_i_p_ai_d);
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  int64_t imm5 = addr.GetImmediateOffset();
  Emit(op | SVEImmPrefetchOperation(prfop) | PgLow8(pg) | Rn(zn) |
       ImmUnsignedField<20, 16>(imm5));
}

void Assembler::SVEGatherPrefetchScalarPlusImmediateHelper(
    PrefetchOperation prfop,
    const PRegister& pg,
    const SVEMemOperand& addr,
    int prefetch_size) {
  VIXL_ASSERT(addr.IsScalarPlusImmediate());
  int64_t imm6 = addr.GetImmediateOffset();

  Instr op = 0xffffffff;
  switch (prefetch_size) {
    case kBRegSize:
      op = PRFB_i_p_bi_s;
      break;
    case kHRegSize:
      op = PRFH_i_p_bi_s;
      break;
    case kSRegSize:
      op = PRFW_i_p_bi_s;
      break;
    case kDRegSize:
      op = PRFD_i_p_bi_s;
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  Emit(op | SVEImmPrefetchOperation(prfop) | PgLow8(pg) |
       RnSP(addr.GetScalarBase()) | ImmField<21, 16>(imm6));
}

void Assembler::SVEContiguousPrefetchScalarPlusScalarHelper(
    PrefetchOperation prfop,
    const PRegister& pg,
    const SVEMemOperand& addr,
    int prefetch_size) {
  VIXL_ASSERT(addr.IsScalarPlusScalar());
  Instr op = 0xffffffff;

  switch (prefetch_size) {
    case kBRegSize:
      VIXL_ASSERT(addr.GetOffsetModifier() == NO_SVE_OFFSET_MODIFIER);
      op = PRFB_i_p_br_s;
      break;
    case kHRegSize:
      VIXL_ASSERT(addr.GetOffsetModifier() == SVE_LSL);
      VIXL_ASSERT(addr.GetShiftAmount() == kHRegSizeInBytesLog2);
      op = PRFH_i_p_br_s;
      break;
    case kSRegSize:
      VIXL_ASSERT(addr.GetOffsetModifier() == SVE_LSL);
      VIXL_ASSERT(addr.GetShiftAmount() == kSRegSizeInBytesLog2);
      op = PRFW_i_p_br_s;
      break;
    case kDRegSize:
      VIXL_ASSERT(addr.GetOffsetModifier() == SVE_LSL);
      VIXL_ASSERT(addr.GetShiftAmount() == kDRegSizeInBytesLog2);
      op = PRFD_i_p_br_s;
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  VIXL_ASSERT(!addr.GetScalarOffset().IsZero());
  Emit(op | SVEImmPrefetchOperation(prfop) | PgLow8(pg) |
       RnSP(addr.GetScalarBase()) | Rm(addr.GetScalarOffset()));
}

void Assembler::SVEContiguousPrefetchScalarPlusVectorHelper(
    PrefetchOperation prfop,
    const PRegister& pg,
    const SVEMemOperand& addr,
    int prefetch_size) {
  VIXL_ASSERT(addr.IsScalarPlusVector());
  ZRegister zm = addr.GetVectorOffset();
  SVEOffsetModifier mod = addr.GetOffsetModifier();

  // All prefetch scalar-plus-vector addressing modes use a shift corresponding
  // to the element size.
  switch (prefetch_size) {
    case kBRegSize:
      VIXL_ASSERT(addr.GetShiftAmount() == kBRegSizeInBytesLog2);
      break;
    case kHRegSize:
      VIXL_ASSERT(addr.GetShiftAmount() == kHRegSizeInBytesLog2);
      break;
    case kSRegSize:
      VIXL_ASSERT(addr.GetShiftAmount() == kSRegSizeInBytesLog2);
      break;
    case kDRegSize:
      VIXL_ASSERT(addr.GetShiftAmount() == kDRegSizeInBytesLog2);
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  Instr sx = 0;
  Instr op = 0xffffffff;
  if ((mod == NO_SVE_OFFSET_MODIFIER) || (mod == SVE_LSL)) {
    VIXL_ASSERT(zm.IsLaneSizeD());

    switch (prefetch_size) {
      case kBRegSize:
        VIXL_ASSERT(mod == NO_SVE_OFFSET_MODIFIER);
        op = PRFB_i_p_bz_d_64_scaled;
        break;
      case kHRegSize:
        VIXL_ASSERT(mod == SVE_LSL);
        op = PRFH_i_p_bz_d_64_scaled;
        break;
      case kSRegSize:
        VIXL_ASSERT(mod == SVE_LSL);
        op = PRFW_i_p_bz_d_64_scaled;
        break;
      case kDRegSize:
        VIXL_ASSERT(mod == SVE_LSL);
        op = PRFD_i_p_bz_d_64_scaled;
        break;
      default:
        VIXL_UNIMPLEMENTED();
        break;
    }
  } else {
    VIXL_ASSERT((mod == SVE_SXTW) || (mod == SVE_UXTW));
    VIXL_ASSERT(zm.IsLaneSizeS() || zm.IsLaneSizeD());

    switch (prefetch_size) {
      case kBRegSize:
        op = zm.IsLaneSizeS() ? static_cast<Instr>(PRFB_i_p_bz_s_x32_scaled)
                              : static_cast<Instr>(PRFB_i_p_bz_d_x32_scaled);
        break;
      case kHRegSize:
        op = zm.IsLaneSizeS() ? static_cast<Instr>(PRFH_i_p_bz_s_x32_scaled)
                              : static_cast<Instr>(PRFH_i_p_bz_d_x32_scaled);
        break;
      case kSRegSize:
        op = zm.IsLaneSizeS() ? static_cast<Instr>(PRFW_i_p_bz_s_x32_scaled)
                              : static_cast<Instr>(PRFW_i_p_bz_d_x32_scaled);
        break;
      case kDRegSize:
        op = zm.IsLaneSizeS() ? static_cast<Instr>(PRFD_i_p_bz_s_x32_scaled)
                              : static_cast<Instr>(PRFD_i_p_bz_d_x32_scaled);
        break;
      default:
        VIXL_UNIMPLEMENTED();
        break;
    }

    if (mod == SVE_SXTW) {
      sx = 1 << 22;
    }
  }

  Emit(op | SVEImmPrefetchOperation(prfop) | PgLow8(pg) | sx |
       RnSP(addr.GetScalarBase()) | Rm(zm));
}

void Assembler::SVEPrefetchHelper(PrefetchOperation prfop,
                                  const PRegister& pg,
                                  const SVEMemOperand& addr,
                                  int prefetch_size) {
  if (addr.IsVectorPlusImmediate()) {
    // For example:
    //   [z0.s, #0]
    SVEGatherPrefetchVectorPlusImmediateHelper(prfop, pg, addr, prefetch_size);

  } else if (addr.IsScalarPlusImmediate()) {
    // For example:
    //   [x0, #42, mul vl]
    SVEGatherPrefetchScalarPlusImmediateHelper(prfop, pg, addr, prefetch_size);

  } else if (addr.IsScalarPlusVector()) {
    // For example:
    //   [x0, z0.s, sxtw]
    SVEContiguousPrefetchScalarPlusVectorHelper(prfop, pg, addr, prefetch_size);

  } else if (addr.IsScalarPlusScalar()) {
    // For example:
    //   [x0, x1]
    SVEContiguousPrefetchScalarPlusScalarHelper(prfop, pg, addr, prefetch_size);

  } else {
    VIXL_UNIMPLEMENTED();
  }
}

void Assembler::prfb(PrefetchOperation prfop,
                     const PRegister& pg,
                     const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  SVEPrefetchHelper(prfop, pg, addr, kBRegSize);
}

void Assembler::prfd(PrefetchOperation prfop,
                     const PRegister& pg,
                     const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  SVEPrefetchHelper(prfop, pg, addr, kDRegSize);
}

void Assembler::prfh(PrefetchOperation prfop,
                     const PRegister& pg,
                     const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  SVEPrefetchHelper(prfop, pg, addr, kHRegSize);
}

void Assembler::prfw(PrefetchOperation prfop,
                     const PRegister& pg,
                     const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  SVEPrefetchHelper(prfop, pg, addr, kSRegSize);
}

void Assembler::SVELd1St1ScaImmHelper(const ZRegister& zt,
                                      const PRegister& pg,
                                      const SVEMemOperand& addr,
                                      Instr regoffset_op,
                                      Instr immoffset_op,
                                      int imm_divisor) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsScalarPlusScalar() || addr.IsScalarPlusImmediate());

  Instr op;
  if (addr.IsScalarPlusScalar()) {
    op = regoffset_op | Rm(addr.GetScalarOffset());
  } else {
    int64_t imm = addr.GetImmediateOffset();
    VIXL_ASSERT(((imm % imm_divisor) == 0) && IsInt4(imm / imm_divisor));
    op = immoffset_op | ImmField<19, 16>(imm / imm_divisor);
  }
  Emit(op | Rt(zt) | PgLow8(pg) | RnSP(addr.GetScalarBase()));
}

void Assembler::SVELd1VecScaHelper(const ZRegister& zt,
                                   const PRegister& pg,
                                   const SVEMemOperand& addr,
                                   uint32_t msize_bytes_log2,
                                   bool is_signed) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(addr.IsVectorPlusScalar());
  ZRegister zn = addr.GetVectorBase();
  VIXL_ASSERT(zn.IsLaneSizeS() || zn.IsLaneSizeD());
  VIXL_ASSERT(AreSameLaneSize(zn, zt));

  uint32_t esize = zn.GetLaneSizeInBytesLog2();
  uint32_t b14_13 = 0;
  if (!is_signed) b14_13 = zn.IsLaneSizeS() ? 0x1 : 0x2;

  Instr op = 0x04008000;  // LDNT1 with vector plus scalar addressing mode.
  op |= (esize << 30) | (msize_bytes_log2 << 23) | (b14_13 << 13);
  Emit(op | Rt(zt) | PgLow8(pg) |
       SVEMemOperandHelper(msize_bytes_log2, 1, addr, true));
}

void Assembler::SVESt1VecScaHelper(const ZRegister& zt,
                                   const PRegister& pg,
                                   const SVEMemOperand& addr,
                                   uint32_t msize_bytes_log2) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(addr.IsVectorPlusScalar());
  ZRegister zn = addr.GetVectorBase();
  VIXL_ASSERT(zn.IsLaneSizeS() || zn.IsLaneSizeD());
  VIXL_ASSERT(AreSameLaneSize(zn, zt));

  uint32_t bit22 = zn.IsLaneSizeS() ? (1 << 22) : 0;
  Instr op = 0xe4002000;  // STNT1 with vector plus scalar addressing mode.
  op |= bit22 | (msize_bytes_log2 << 23);
  Emit(op | Rt(zt) | PgLow8(pg) |
       SVEMemOperandHelper(msize_bytes_log2, 1, addr, true));
}

#define VIXL_SVE_LD1R_LIST(V)                                  \
  V(qb, 0, B, LD1RQB_z_p_br_contiguous, LD1RQB_z_p_bi_u8, 16)  \
  V(qh, 1, H, LD1RQH_z_p_br_contiguous, LD1RQH_z_p_bi_u16, 16) \
  V(qw, 2, S, LD1RQW_z_p_br_contiguous, LD1RQW_z_p_bi_u32, 16) \
  V(qd, 3, D, LD1RQD_z_p_br_contiguous, LD1RQD_z_p_bi_u64, 16) \
  V(ob, 0, B, 0xa4200000, 0xa4202000, 32)                      \
  V(oh, 1, H, 0xa4a00000, 0xa4a02000, 32)                      \
  V(ow, 2, S, 0xa5200000, 0xa5202000, 32)                      \
  V(od, 3, D, 0xa5a00000, 0xa5a02000, 32)

#define VIXL_DEFINE_ASM_FUNC(FN, SH, SZ, SCA, IMM, BYTES)                    \
  void Assembler::ld1r##FN(const ZRegister& zt,                              \
                           const PRegisterZ& pg,                             \
                           const SVEMemOperand& addr) {                      \
    VIXL_ASSERT((BYTES == 16) ||                                             \
                ((BYTES == 32) && (CPUHas(CPUFeatures::kSVEF64MM))));        \
    VIXL_ASSERT(addr.IsScalarPlusImmediate() || addr.IsEquivalentToLSL(SH)); \
    VIXL_ASSERT(zt.IsLaneSize##SZ());                                        \
    SVELd1St1ScaImmHelper(zt, pg, addr, SCA, IMM, BYTES);                    \
  }
VIXL_SVE_LD1R_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC
#undef VIXL_SVE_LD1R_LIST

#define VIXL_DEFINE_LDFF1(MSZ, LANE_SIZE)                                  \
  void Assembler::ldff1##MSZ(const ZRegister& zt,                          \
                             const PRegisterZ& pg,                         \
                             const SVEMemOperand& addr) {                  \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                                \
    SVELdff1Helper(k##LANE_SIZE##RegSizeInBytesLog2, zt, pg, addr, false); \
  }
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_LDFF1)

#define VIXL_DEFINE_LDFF1S(MSZ, LANE_SIZE)                                \
  void Assembler::ldff1s##MSZ(const ZRegister& zt,                        \
                              const PRegisterZ& pg,                       \
                              const SVEMemOperand& addr) {                \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                               \
    SVELdff1Helper(k##LANE_SIZE##RegSizeInBytesLog2, zt, pg, addr, true); \
  }
VIXL_SVE_LOAD_STORE_SIGNED_VARIANT_LIST(VIXL_DEFINE_LDFF1S)

void Assembler::ldnf1b(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(0,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ false,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1d(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(3,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ false,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1h(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(1,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ false,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1sb(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(0,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ true,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1sh(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(1,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ true,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1sw(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(2,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ true,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnf1w(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(addr.IsPlainRegister() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));

  SVELdSt1Helper(2,
                 zt,
                 pg,
                 addr,
                 /* is_signed = */ false,
                 SVEContiguousNonFaultLoad_ScalarPlusImmFixed);
}

void Assembler::ldnt1b(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(0)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVELd1VecScaHelper(zt, pg, addr, 0, /* is_signed = */ false);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          LDNT1B_z_p_br_contiguous,
                          LDNT1B_z_p_bi_contiguous);
  }
}

void Assembler::ldnt1d(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(3)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVELd1VecScaHelper(zt, pg, addr, 3, /* is_signed = */ false);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          LDNT1D_z_p_br_contiguous,
                          LDNT1D_z_p_bi_contiguous);
  }
}

void Assembler::ldnt1h(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(1)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVELd1VecScaHelper(zt, pg, addr, 1, /* is_signed = */ false);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          LDNT1H_z_p_br_contiguous,
                          LDNT1H_z_p_bi_contiguous);
  }
}

void Assembler::ldnt1w(const ZRegister& zt,
                       const PRegisterZ& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(2)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVELd1VecScaHelper(zt, pg, addr, 2, /* is_signed = */ false);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          LDNT1W_z_p_br_contiguous,
                          LDNT1W_z_p_bi_contiguous);
  }
}

void Assembler::ldnt1sb(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2));
  SVELd1VecScaHelper(zt, pg, addr, 0, /* is_signed = */ true);
}

void Assembler::ldnt1sh(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2));
  SVELd1VecScaHelper(zt, pg, addr, 1, /* is_signed = */ true);
}

void Assembler::ldnt1sw(const ZRegister& zt,
                        const PRegisterZ& pg,
                        const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2));
  SVELd1VecScaHelper(zt, pg, addr, 2, /* is_signed = */ true);
}

Instr Assembler::SVEMemOperandHelper(unsigned msize_in_bytes_log2,
                                     int num_regs,
                                     const SVEMemOperand& addr,
                                     bool is_load) {
  VIXL_ASSERT((num_regs >= 1) && (num_regs <= 4));

  Instr op = 0xfffffff;
  if (addr.IsScalarPlusImmediate()) {
    VIXL_ASSERT((addr.GetImmediateOffset() == 0) || addr.IsMulVl());
    int64_t imm = addr.GetImmediateOffset();
    VIXL_ASSERT((imm % num_regs) == 0);
    op = RnSP(addr.GetScalarBase()) | ImmField<19, 16>(imm / num_regs);

  } else if (addr.IsScalarPlusScalar()) {
    VIXL_ASSERT(addr.GetScalarOffset().IsZero() ||
                addr.IsEquivalentToLSL(msize_in_bytes_log2));
    op = RnSP(addr.GetScalarBase()) | Rm(addr.GetScalarOffset());

  } else if (addr.IsVectorPlusImmediate()) {
    ZRegister zn = addr.GetVectorBase();
    uint64_t imm = addr.GetImmediateOffset();
    VIXL_ASSERT(num_regs == 1);
    VIXL_ASSERT(zn.IsLaneSizeS() || zn.IsLaneSizeD());
    VIXL_ASSERT(IsMultiple(imm, (1 << msize_in_bytes_log2)));
    op = Rn(zn) | ImmUnsignedField<20, 16>(imm >> msize_in_bytes_log2);
  } else if (addr.IsVectorPlusScalar()) {
    VIXL_ASSERT(addr.GetOffsetModifier() == NO_SVE_OFFSET_MODIFIER);
    VIXL_ASSERT(addr.GetShiftAmount() == 0);
    ZRegister zn = addr.GetVectorBase();
    VIXL_ASSERT(zn.IsLaneSizeS() || zn.IsLaneSizeD());
    Register xm = addr.GetScalarOffset();
    op = Rn(zn) | Rm(xm);
  } else if (addr.IsScalarPlusVector()) {
    // We have to support several different addressing modes. Some instructions
    // support a subset of these, but the SVEMemOperand encoding is consistent.
    Register xn = addr.GetScalarBase();
    ZRegister zm = addr.GetVectorOffset();
    SVEOffsetModifier mod = addr.GetOffsetModifier();
    Instr modifier_bit = 1 << (is_load ? 22 : 14);
    Instr xs = (mod == SVE_SXTW) ? modifier_bit : 0;
    VIXL_ASSERT(num_regs == 1);

    if (mod == SVE_LSL) {
      // 64-bit scaled offset:            [<Xn|SP>, <Zm>.D, LSL #<shift>]
      VIXL_ASSERT(zm.IsLaneSizeD());
      VIXL_ASSERT(addr.GetShiftAmount() == msize_in_bytes_log2);
    } else if (mod == NO_SVE_OFFSET_MODIFIER) {
      // 64-bit unscaled offset:          [<Xn|SP>, <Zm>.D]
      VIXL_ASSERT(zm.IsLaneSizeD());
      VIXL_ASSERT(addr.GetShiftAmount() == 0);
    } else {
      // 32-bit scaled offset:            [<Xn|SP>, <Zm>.S, <mod> #<shift>]
      // 32-bit unscaled offset:          [<Xn|SP>, <Zm>.S, <mod>]
      // 32-bit unpacked scaled offset:   [<Xn|SP>, <Zm>.D, <mod> #<shift>]
      // 32-bit unpacked unscaled offset: [<Xn|SP>, <Zm>.D, <mod>]
      VIXL_ASSERT(zm.IsLaneSizeS() || zm.IsLaneSizeD());
      VIXL_ASSERT((mod == SVE_SXTW) || (mod == SVE_UXTW));
      VIXL_ASSERT((addr.GetShiftAmount() == 0) ||
                  (addr.GetShiftAmount() == msize_in_bytes_log2));
    }

    // The form itself is encoded in the instruction opcode.
    op = RnSP(xn) | Rm(zm) | xs;
  } else {
    VIXL_UNIMPLEMENTED();
  }

  return op;
}

// SVEMemStore.

void Assembler::SVESt1Helper(unsigned msize_in_bytes_log2,
                             const ZRegister& zt,
                             const PRegister& pg,
                             const SVEMemOperand& addr) {
  if (addr.IsScalarPlusScalar()) {
    // Rm must not be xzr.
    VIXL_ASSERT(!addr.GetScalarOffset().IsZero());
  }

  if (addr.IsScatterGather()) {
    bool is_load = false;
    bool is_signed = false;
    bool is_ff = false;
    SVEScatterGatherHelper(msize_in_bytes_log2,
                           zt,
                           pg,
                           addr,
                           is_load,
                           is_signed,
                           is_ff);
    return;
  }

  Instr op;
  if (addr.IsScalarPlusImmediate()) {
    op = SVEContiguousStore_ScalarPlusImmFixed;
  } else if (addr.IsScalarPlusScalar()) {
    op = SVEContiguousStore_ScalarPlusScalarFixed;
  } else {
    VIXL_UNIMPLEMENTED();
    op = 0xffffffff;
  }
  SVELdSt1Helper(msize_in_bytes_log2, zt, pg, addr, false, op);
}

void Assembler::SVESt234Helper(int num_regs,
                               const ZRegister& zt1,
                               const PRegister& pg,
                               const SVEMemOperand& addr) {
  if (addr.IsScalarPlusScalar()) {
    // Rm must not be xzr.
    VIXL_ASSERT(!addr.GetScalarOffset().IsZero());
  }

  Instr op;
  if (addr.IsScalarPlusImmediate()) {
    op = SVEStoreMultipleStructures_ScalarPlusImmFixed;
  } else if (addr.IsScalarPlusScalar()) {
    op = SVEStoreMultipleStructures_ScalarPlusScalarFixed;
  } else {
    // These instructions don't support any other addressing modes.
    VIXL_ABORT();
  }
  SVELdSt234Helper(num_regs, zt1, pg, addr, op);
}

#define VIXL_DEFINE_ST1(MSZ, LANE_SIZE)                           \
  void Assembler::st1##MSZ(const ZRegister& zt,                   \
                           const PRegister& pg,                   \
                           const SVEMemOperand& addr) {           \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));                       \
    SVESt1Helper(k##LANE_SIZE##RegSizeInBytesLog2, zt, pg, addr); \
  }
#define VIXL_DEFINE_ST2(MSZ, LANE_SIZE)                 \
  void Assembler::st2##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const PRegister& pg,         \
                           const SVEMemOperand& addr) { \
    USE(zt2);                                           \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2));              \
    VIXL_ASSERT(AreSameFormat(zt1, zt2));               \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVESt234Helper(2, zt1, pg, addr);                   \
  }
#define VIXL_DEFINE_ST3(MSZ, LANE_SIZE)                 \
  void Assembler::st3##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const ZRegister& zt3,        \
                           const PRegister& pg,         \
                           const SVEMemOperand& addr) { \
    USE(zt2, zt3);                                      \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2, zt3));         \
    VIXL_ASSERT(AreSameFormat(zt1, zt2, zt3));          \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVESt234Helper(3, zt1, pg, addr);                   \
  }
#define VIXL_DEFINE_ST4(MSZ, LANE_SIZE)                 \
  void Assembler::st4##MSZ(const ZRegister& zt1,        \
                           const ZRegister& zt2,        \
                           const ZRegister& zt3,        \
                           const ZRegister& zt4,        \
                           const PRegister& pg,         \
                           const SVEMemOperand& addr) { \
    USE(zt2, zt3, zt4);                                 \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));             \
    VIXL_ASSERT(AreConsecutive(zt1, zt2, zt3, zt4));    \
    VIXL_ASSERT(AreSameFormat(zt1, zt2, zt3, zt4));     \
    VIXL_ASSERT(zt1.IsLaneSize##LANE_SIZE());           \
    SVESt234Helper(4, zt1, pg, addr);                   \
  }

VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_ST1)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_ST2)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_ST3)
VIXL_SVE_LOAD_STORE_VARIANT_LIST(VIXL_DEFINE_ST4)

void Assembler::stnt1b(const ZRegister& zt,
                       const PRegister& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(0)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVESt1VecScaHelper(zt, pg, addr, 0);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          STNT1B_z_p_br_contiguous,
                          STNT1B_z_p_bi_contiguous);
  }
}

void Assembler::stnt1d(const ZRegister& zt,
                       const PRegister& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(3)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVESt1VecScaHelper(zt, pg, addr, 3);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          STNT1D_z_p_br_contiguous,
                          STNT1D_z_p_bi_contiguous);
  }
}

void Assembler::stnt1h(const ZRegister& zt,
                       const PRegister& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(1)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVESt1VecScaHelper(zt, pg, addr, 1);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          STNT1H_z_p_br_contiguous,
                          STNT1H_z_p_bi_contiguous);
  }
}

void Assembler::stnt1w(const ZRegister& zt,
                       const PRegister& pg,
                       const SVEMemOperand& addr) {
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() && addr.IsMulVl()) ||
              (addr.IsScalarPlusScalar() && addr.IsEquivalentToLSL(2)) ||
              (addr.IsVectorPlusScalar() && CPUHas(CPUFeatures::kSVE2)));
  if (addr.IsVectorPlusScalar()) {
    SVESt1VecScaHelper(zt, pg, addr, 2);
  } else {
    SVELd1St1ScaImmHelper(zt,
                          pg,
                          addr,
                          STNT1W_z_p_br_contiguous,
                          STNT1W_z_p_bi_contiguous);
  }
}

void Assembler::str(const CPURegister& rt, const SVEMemOperand& addr) {
  // STR <Pt/Zt>, [<Xn|SP>{, #<imm>, MUL VL}]

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rt.IsPRegister() || rt.IsZRegister());
  VIXL_ASSERT(addr.IsPlainScalar() ||
              (addr.IsScalarPlusImmediate() &&
               (addr.GetOffsetModifier() == SVE_MUL_VL)));
  int64_t imm9 = addr.GetImmediateOffset();
  VIXL_ASSERT(IsInt9(imm9));
  Instr imm9l = ExtractUnsignedBitfield32(2, 0, imm9) << 10;
  Instr imm9h = ExtractUnsignedBitfield32(8, 3, imm9) << 16;

  Instr op = STR_z_bi;
  if (rt.IsPRegister()) {
    op = STR_p_bi;
  }
  Emit(op | Rt(rt) | RnSP(addr.GetScalarBase()) | imm9h | imm9l);
}

// SVEMulIndex.

void Assembler::sdot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));

  Instr op = 0xffffffff;
  switch (zda.GetLaneSizeInBits()) {
    case kSRegSize:
      VIXL_ASSERT(IsUint2(index));
      op = SDOT_z_zzzi_s | Rx<18, 16>(zm) | (index << 19) | Rd(zda) | Rn(zn);
      break;
    case kDRegSize:
      VIXL_ASSERT(IsUint1(index));
      op = SDOT_z_zzzi_d | Rx<19, 16>(zm) | (index << 20) | Rd(zda) | Rn(zn);
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  Emit(op);
}

void Assembler::udot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));

  Instr op = 0xffffffff;
  switch (zda.GetLaneSizeInBits()) {
    case kSRegSize:
      VIXL_ASSERT(IsUint2(index));
      op = UDOT_z_zzzi_s | Rx<18, 16>(zm) | (index << 19) | Rd(zda) | Rn(zn);
      break;
    case kDRegSize:
      VIXL_ASSERT(IsUint1(index));
      op = UDOT_z_zzzi_d | Rx<19, 16>(zm) | (index << 20) | Rd(zda) | Rn(zn);
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  Emit(op);
}

// SVEPartitionBreak.

void Assembler::brka(const PRegisterWithLaneSize& pd,
                     const PRegister& pg,
                     const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pg.IsMerging() || pg.IsZeroing());
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());

  Instr m = pg.IsMerging() ? 0x00000010 : 0x00000000;
  Emit(BRKA_p_p_p | Pd(pd) | Pg<13, 10>(pg) | m | Pn(pn));
}

void Assembler::brkas(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());

  Emit(BRKAS_p_p_p_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn));
}

void Assembler::brkb(const PRegisterWithLaneSize& pd,
                     const PRegister& pg,
                     const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pg.IsMerging() || pg.IsZeroing());
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());

  Instr m = pg.IsMerging() ? 0x00000010 : 0x00000000;
  Emit(BRKB_p_p_p | Pd(pd) | Pg<13, 10>(pg) | m | Pn(pn));
}

void Assembler::brkbs(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());

  Emit(BRKBS_p_p_p_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn));
}

void Assembler::brkn(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  USE(pm);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());
  VIXL_ASSERT(pd.Is(pm));

  Emit(BRKN_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn));
}

void Assembler::brkns(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn,
                      const PRegisterWithLaneSize& pm) {
  USE(pm);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeB() && pn.IsLaneSizeB());
  VIXL_ASSERT(pd.Is(pm));

  Emit(BRKNS_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn));
}

// SVEPermutePredicate.

void Assembler::punpkhi(const PRegisterWithLaneSize& pd,
                        const PRegisterWithLaneSize& pn) {
  // PUNPKHI <Pd>.H, <Pn>.B
  //  0000 0101 0011 0001 0100 000. ...0 ....
  //  H<16> = 1 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeH());
  VIXL_ASSERT(pn.IsLaneSizeB());

  Emit(PUNPKHI_p_p | Pd(pd) | Pn(pn));
}

void Assembler::punpklo(const PRegisterWithLaneSize& pd,
                        const PRegisterWithLaneSize& pn) {
  // PUNPKLO <Pd>.H, <Pn>.B
  //  0000 0101 0011 0000 0100 000. ...0 ....
  //  H<16> = 0 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.IsLaneSizeH());
  VIXL_ASSERT(pn.IsLaneSizeB());

  Emit(PUNPKLO_p_p | Pd(pd) | Pn(pn));
}

void Assembler::rev(const PRegisterWithLaneSize& pd,
                    const PRegisterWithLaneSize& pn) {
  // REV <Pd>.<T>, <Pn>.<T>
  //  0000 0101 ..11 0100 0100 000. ...0 ....
  //  size<23:22> | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn));

  Emit(REV_p_p | SVESize(pd) | Pd(pd) | Rx<8, 5>(pn));
}

void Assembler::trn1(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // TRN1 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0101 000. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 10 | H<10> = 0 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(TRN1_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

void Assembler::trn2(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // TRN2 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0101 010. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 10 | H<10> = 1 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(TRN2_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

void Assembler::uzp1(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // UZP1 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0100 100. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 01 | H<10> = 0 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(UZP1_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

void Assembler::uzp2(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // UZP2 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0100 110. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 01 | H<10> = 1 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(UZP2_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

void Assembler::zip1(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // ZIP1 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0100 000. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 00 | H<10> = 0 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(ZIP1_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

void Assembler::zip2(const PRegisterWithLaneSize& pd,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  // ZIP2 <Pd>.<T>, <Pn>.<T>, <Pm>.<T>
  //  0000 0101 ..10 .... 0100 010. ...0 ....
  //  size<23:22> | Pm<19:16> | opc<12:11> = 00 | H<10> = 1 | Pn<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(pd, pn, pm));

  Emit(ZIP2_p_pp | SVESize(pd) | Pd(pd) | Pn(pn) | Pm(pm));
}

// SVEPermuteVectorExtract.

void Assembler::ext(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    unsigned offset) {
  // EXT <Zdn>.B, <Zdn>.B, <Zm>.B, #<imm>
  //  0000 0101 001. .... 000. .... .... ....
  //  imm8h<20:16> | imm8l<12:10> | Zm<9:5> | Zdn<4:0>

  // EXT <Zd>.B, { <Zn1>.B, <Zn2>.B }, #<imm>
  //  0000 0101 011. .... 000. .... .... ....
  //  imm8h<20:16> | imm8l<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(IsUint8(offset));

  int imm8h = ExtractUnsignedBitfield32(7, 3, offset);
  int imm8l = ExtractUnsignedBitfield32(2, 0, offset);

  Instr op;
  if (zd.Is(zn)) {
    // Destructive form.
    op = EXT_z_zi_des | Rn(zm);
  } else {
    // Constructive form (requires SVE2).
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2) && AreConsecutive(zn, zm));
    op = 0x05600000 | Rn(zn);
  }

  Emit(op | Rd(zd) | ImmUnsignedField<20, 16>(imm8h) |
       ImmUnsignedField<12, 10>(imm8l));
}

// SVEPermuteVectorInterleaving.

void Assembler::trn1(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // TRN1 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0111 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 100 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(TRN1_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::trn2(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // TRN2 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0111 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 101 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(TRN2_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uzp1(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UZP1 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0110 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 010 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(UZP1_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uzp2(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UZP2 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0110 11.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 011 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(UZP2_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::zip1(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // ZIP1 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0110 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 000 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(ZIP1_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::zip2(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // ZIP2 <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0110 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<12:10> = 001 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(ZIP2_z_zz | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

// SVEPermuteVectorPredicated.

void Assembler::clasta(const Register& rd,
                       const PRegister& pg,
                       const Register& rn,
                       const ZRegister& zm) {
  // CLASTA <R><dn>, <Pg>, <R><dn>, <Zm>.<T>
  //  0000 0101 ..11 0000 101. .... .... ....
  //  size<23:22> | B<16> = 0 | Pg<12:10> | Zm<9:5> | Rdn<4:0>

  USE(rn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rd.Is(rn));

  Emit(CLASTA_r_p_z | SVESize(zm) | Rd(rd) | PgLow8(pg) | Rn(zm));
}

void Assembler::clasta(const VRegister& vd,
                       const PRegister& pg,
                       const VRegister& vn,
                       const ZRegister& zm) {
  // CLASTA <V><dn>, <Pg>, <V><dn>, <Zm>.<T>
  //  0000 0101 ..10 1010 100. .... .... ....
  //  size<23:22> | B<16> = 0 | Pg<12:10> | Zm<9:5> | Vdn<4:0>

  USE(vn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.Is(vn));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(AreSameLaneSize(vd, zm));

  Emit(CLASTA_v_p_z | SVESize(zm) | Rd(vd) | PgLow8(pg) | Rn(zm));
}

void Assembler::clasta(const ZRegister& zd,
                       const PRegister& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // CLASTA <Zdn>.<T>, <Pg>, <Zdn>.<T>, <Zm>.<T>
  //  0000 0101 ..10 1000 100. .... .... ....
  //  size<23:22> | B<16> = 0 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(CLASTA_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::clastb(const Register& rd,
                       const PRegister& pg,
                       const Register& rn,
                       const ZRegister& zm) {
  // CLASTB <R><dn>, <Pg>, <R><dn>, <Zm>.<T>
  //  0000 0101 ..11 0001 101. .... .... ....
  //  size<23:22> | B<16> = 1 | Pg<12:10> | Zm<9:5> | Rdn<4:0>

  USE(rn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(rd.Is(rn));

  Emit(CLASTB_r_p_z | SVESize(zm) | Rd(rd) | PgLow8(pg) | Rn(zm));
}

void Assembler::clastb(const VRegister& vd,
                       const PRegister& pg,
                       const VRegister& vn,
                       const ZRegister& zm) {
  // CLASTB <V><dn>, <Pg>, <V><dn>, <Zm>.<T>
  //  0000 0101 ..10 1011 100. .... .... ....
  //  size<23:22> | B<16> = 1 | Pg<12:10> | Zm<9:5> | Vdn<4:0>

  USE(vn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.Is(vn));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(AreSameLaneSize(vd, zm));

  Emit(CLASTB_v_p_z | SVESize(zm) | Rd(vd) | PgLow8(pg) | Rn(zm));
}

void Assembler::clastb(const ZRegister& zd,
                       const PRegister& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // CLASTB <Zdn>.<T>, <Pg>, <Zdn>.<T>, <Zm>.<T>
  //  0000 0101 ..10 1001 100. .... .... ....
  //  size<23:22> | B<16> = 1 | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(CLASTB_z_p_zz | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::compact(const ZRegister& zd,
                        const PRegister& pg,
                        const ZRegister& zn) {
  // COMPACT <Zd>.<T>, <Pg>, <Zn>.<T>
  //  0000 0101 1.10 0001 100. .... .... ....
  //  sz<22> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT((zd.GetLaneSizeInBits() == kSRegSize) ||
              (zd.GetLaneSizeInBits() == kDRegSize));

  Instr sz = (zd.GetLaneSizeInBits() == kDRegSize) ? (1 << 22) : 0;
  Emit(COMPACT_z_p_z | sz | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::cpy(const ZRegister& zd,
                    const PRegisterM& pg,
                    const Register& rn) {
  // CPY <Zd>.<T>, <Pg>/M, <R><n|SP>
  //  0000 0101 ..10 1000 101. .... .... ....
  //  size<23:22> | Pg<12:10> | Rn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(static_cast<unsigned>(rn.GetSizeInBits()) >=
              zd.GetLaneSizeInBits());

  Emit(CPY_z_p_r | SVESize(zd) | Rd(zd) | PgLow8(pg) | RnSP(rn));
}

void Assembler::cpy(const ZRegister& zd,
                    const PRegisterM& pg,
                    const VRegister& vn) {
  // CPY <Zd>.<T>, <Pg>/M, <V><n>
  //  0000 0101 ..10 0000 100. .... .... ....
  //  size<23:22> | Pg<12:10> | Vn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vn.IsScalar());
  VIXL_ASSERT(static_cast<unsigned>(vn.GetSizeInBits()) ==
              zd.GetLaneSizeInBits());

  Emit(CPY_z_p_v | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(vn));
}

void Assembler::lasta(const Register& rd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // LASTA <R><d>, <Pg>, <Zn>.<T>
  //  0000 0101 ..10 0000 101. .... .... ....
  //  size<23:22> | B<16> = 0 | Pg<12:10> | Zn<9:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LASTA_r_p_z | SVESize(zn) | Rd(rd) | PgLow8(pg) | Rn(zn));
}

void Assembler::lasta(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // LASTA <V><d>, <Pg>, <Zn>.<T>
  //  0000 0101 ..10 0010 100. .... .... ....
  //  size<23:22> | B<16> = 0 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(LASTA_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::lastb(const Register& rd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // LASTB <R><d>, <Pg>, <Zn>.<T>
  //  0000 0101 ..10 0001 101. .... .... ....
  //  size<23:22> | B<16> = 1 | Pg<12:10> | Zn<9:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(LASTB_r_p_z | SVESize(zn) | Rd(rd) | PgLow8(pg) | Rn(zn));
}

void Assembler::lastb(const VRegister& vd,
                      const PRegister& pg,
                      const ZRegister& zn) {
  // LASTB <V><d>, <Pg>, <Zn>.<T>
  //  0000 0101 ..10 0011 100. .... .... ....
  //  size<23:22> | B<16> = 1 | Pg<12:10> | Zn<9:5> | Vd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vd.IsScalar());

  Emit(LASTB_v_p_z | SVESize(zn) | Rd(vd) | PgLow8(pg) | Rn(zn));
}

void Assembler::rbit(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // RBIT <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0101 ..10 0111 100. .... .... ....
  //  size<23:22> | opc<17:16> = 11 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(RBIT_z_p_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::revb(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // REVB <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0101 ..10 0100 100. .... .... ....
  //  size<23:22> | opc<17:16> = 00 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.IsLaneSizeH() || zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(REVB_z_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::revh(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // REVH <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0000 0101 ..10 0101 100. .... .... ....
  //  size<23:22> | opc<17:16> = 01 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(REVH_z_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::revw(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn) {
  // REVW <Zd>.D, <Pg>/M, <Zn>.D
  //  0000 0101 ..10 0110 100. .... .... ....
  //  size<23:22> | opc<17:16> = 10 | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(REVW_z_z | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::splice(const ZRegister& zd,
                       const PRegister& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  if (zd.Aliases(zn)) {
    // SPLICE <Zdn>.<T>, <Pg>, <Zdn>.<T>, <Zm>.<T>
    //  0000 0101 ..10 1100 100. .... .... ....
    //  size<23:22> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

    USE(zn);
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
    VIXL_ASSERT(zd.Is(zn));

    Emit(SPLICE_z_p_zz_des | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
  } else {
    splice_con(zd, pg, zn, zm);
  }
}

void Assembler::splice_con(const ZRegister& zd,
                           const PRegister& pg,
                           const ZRegister& zn1,
                           const ZRegister& zn2) {
  // SPLICE <Zd>.<T>, <Pg>, { <Zn1>.<T>, <Zn2>.<T> }
  //  0000 0101 ..10 1101 100. .... .... ....
  //  size<23:22> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  USE(zn2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreConsecutive(zn1, zn2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn1, zn2));

  Emit(0x052d8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn1));
}

// SVEPermuteVectorUnpredicated.

void Assembler::dup(const ZRegister& zd, const Register& xn) {
  // DUP <Zd>.<T>, <R><n|SP>
  //  0000 0101 ..10 0000 0011 10.. .... ....
  //  size<23:22> | Rn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(DUP_z_r | SVESize(zd) | Rd(zd) | RnSP(xn));
}

void Assembler::dup(const ZRegister& zd, const ZRegister& zn, unsigned index) {
  // DUP <Zd>.<T>, <Zn>.<T>[<imm>]
  //  0000 0101 ..1. .... 0010 00.. .... ....
  //  imm2<23:22> | tsz<20:16> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(zd, zn));
  VIXL_ASSERT((index * zd.GetLaneSizeInBits()) < 512);
  int n = zd.GetLaneSizeInBytesLog2();
  unsigned imm_7 = (index << (n + 1)) | (1 << n);
  VIXL_ASSERT(IsUint7(imm_7));
  unsigned imm_2 = ExtractUnsignedBitfield32(6, 5, imm_7);
  unsigned tsz_5 = ExtractUnsignedBitfield32(4, 0, imm_7);

  Emit(DUP_z_zi | ImmUnsignedField<23, 22>(imm_2) |
       ImmUnsignedField<20, 16>(tsz_5) | Rd(zd) | Rn(zn));
}

void Assembler::insr(const ZRegister& zdn, const Register& rm) {
  // INSR <Zdn>.<T>, <R><m>
  //  0000 0101 ..10 0100 0011 10.. .... ....
  //  size<23:22> | Rm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(INSR_z_r | SVESize(zdn) | Rd(zdn) | Rn(rm));
}

void Assembler::insr(const ZRegister& zdn, const VRegister& vm) {
  // INSR <Zdn>.<T>, <V><m>
  //  0000 0101 ..11 0100 0011 10.. .... ....
  //  size<23:22> | Vm<9:5> | Zdn<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(vm.IsScalar());

  Emit(INSR_z_v | SVESize(zdn) | Rd(zdn) | Rn(vm));
}

void Assembler::rev(const ZRegister& zd, const ZRegister& zn) {
  // REV <Zd>.<T>, <Zn>.<T>
  //  0000 0101 ..11 1000 0011 10.. .... ....
  //  size<23:22> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(zd, zn));

  Emit(REV_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::sunpkhi(const ZRegister& zd, const ZRegister& zn) {
  // SUNPKHI <Zd>.<T>, <Zn>.<Tb>
  //  0000 0101 ..11 0001 0011 10.. .... ....
  //  size<23:22> | U<17> = 0 | H<16> = 1 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(!zd.IsLaneSizeB());

  Emit(SUNPKHI_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::sunpklo(const ZRegister& zd, const ZRegister& zn) {
  // SUNPKLO <Zd>.<T>, <Zn>.<Tb>
  //  0000 0101 ..11 0000 0011 10.. .... ....
  //  size<23:22> | U<17> = 0 | H<16> = 0 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(!zd.IsLaneSizeB());

  Emit(SUNPKLO_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::tbl(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // TBL <Zd>.<T>, { <Zn>.<T> }, <Zm>.<T>
  //  0000 0101 ..1. .... 0011 00.. .... ....
  //  size<23:22> | Zm<20:16> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(zd, zn, zm));

  Emit(TBL_z_zz_1 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uunpkhi(const ZRegister& zd, const ZRegister& zn) {
  // UUNPKHI <Zd>.<T>, <Zn>.<Tb>
  //  0000 0101 ..11 0011 0011 10.. .... ....
  //  size<23:22> | U<17> = 1 | H<16> = 1 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(!zd.IsLaneSizeB());

  Emit(UUNPKHI_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

void Assembler::uunpklo(const ZRegister& zd, const ZRegister& zn) {
  // UUNPKLO <Zd>.<T>, <Zn>.<Tb>
  //  0000 0101 ..11 0010 0011 10.. .... ....
  //  size<23:22> | U<17> = 1 | H<16> = 0 | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(!zd.IsLaneSizeB());

  Emit(UUNPKLO_z_z | SVESize(zd) | Rd(zd) | Rn(zn));
}

// SVEPredicateCount.

void Assembler::cntp(const Register& xd,
                     const PRegister& pg,
                     const PRegisterWithLaneSize& pn) {
  // CNTP <Xd>, <Pg>, <Pn>.<T>
  //  0010 0101 ..10 0000 10.. ..0. .... ....
  //  size<23:22> | opc<18:16> = 000 | Pg<13:10> | o2<9> = 0 | Pn<8:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX());
  VIXL_ASSERT(pg.IsUnqualified());
  if (pg.HasLaneSize()) VIXL_ASSERT(AreSameFormat(pg, pn));

  Emit(CNTP_r_p_p | SVESize(pn) | Rd(xd) | Pg<13, 10>(pg) | Pn(pn));
}

// SVEPredicateLogicalOp.
void Assembler::and_(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(AND_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::ands(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(ANDS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::bic(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(BIC_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::bics(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(BICS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::eor(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(EOR_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::eors(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(EORS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::nand(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(NAND_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::nands(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn,
                      const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(NANDS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::nor(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(NOR_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::nors(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(NORS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::orn(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(ORN_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::orns(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(ORNS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::orr(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(ORR_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::orrs(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn,
                     const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameFormat(pd, pn, pm));
  VIXL_ASSERT(pd.IsLaneSizeB());
  Emit(ORRS_p_p_pp_z | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::sel(const PRegisterWithLaneSize& pd,
                    const PRegister& pg,
                    const PRegisterWithLaneSize& pn,
                    const PRegisterWithLaneSize& pm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  Emit(SEL_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

// SVEPredicateMisc.

void Assembler::pfalse(const PRegisterWithLaneSize& pd) {
  // PFALSE <Pd>.B
  //  0010 0101 0001 1000 1110 0100 0000 ....
  //  op<23> = 0 | S<22> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  // Ignore the lane size, since it makes no difference to the operation.

  Emit(PFALSE_p | Pd(pd));
}

void Assembler::pfirst(const PRegisterWithLaneSize& pd,
                       const PRegister& pg,
                       const PRegisterWithLaneSize& pn) {
  // PFIRST <Pdn>.B, <Pg>, <Pdn>.B
  //  0010 0101 0101 1000 1100 000. ...0 ....
  //  op<23> = 0 | S<22> = 1 | Pg<8:5> | Pdn<3:0>

  USE(pn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.Is(pn));
  VIXL_ASSERT(pd.IsLaneSizeB());

  Emit(PFIRST_p_p_p | Pd(pd) | Pg<8, 5>(pg));
}

void Assembler::pnext(const PRegisterWithLaneSize& pd,
                      const PRegister& pg,
                      const PRegisterWithLaneSize& pn) {
  // PNEXT <Pdn>.<T>, <Pg>, <Pdn>.<T>
  //  0010 0101 ..01 1001 1100 010. ...0 ....
  //  size<23:22> | Pg<8:5> | Pdn<3:0>

  USE(pn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pd.Is(pn));

  Emit(PNEXT_p_p_p | SVESize(pd) | Pd(pd) | Pg<8, 5>(pg));
}

void Assembler::ptest(const PRegister& pg, const PRegisterWithLaneSize& pn) {
  // PTEST <Pg>, <Pn>.B
  //  0010 0101 0101 0000 11.. ..0. ...0 0000
  //  op<23> = 0 | S<22> = 1 | Pg<13:10> | Pn<8:5> | opc2<3:0> = 0000

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(pn.IsLaneSizeB());

  Emit(PTEST_p_p | Pg<13, 10>(pg) | Rx<8, 5>(pn));
}

void Assembler::ptrue(const PRegisterWithLaneSize& pd, int pattern) {
  // PTRUE <Pd>.<T>{, <pattern>}
  //  0010 0101 ..01 1000 1110 00.. ...0 ....
  //  size<23:22> | S<16> = 0 | pattern<9:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(PTRUE_p_s | SVESize(pd) | Pd(pd) | ImmSVEPredicateConstraint(pattern));
}

void Assembler::ptrues(const PRegisterWithLaneSize& pd, int pattern) {
  // PTRUES <Pd>.<T>{, <pattern>}
  //  0010 0101 ..01 1001 1110 00.. ...0 ....
  //  size<23:22> | S<16> = 1 | pattern<9:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(PTRUES_p_s | SVESize(pd) | Pd(pd) | ImmSVEPredicateConstraint(pattern));
}

void Assembler::rdffr(const PRegisterWithLaneSize& pd) {
  // RDFFR <Pd>.B
  //  0010 0101 0001 1001 1111 0000 0000 ....
  //  op<23> = 0 | S<22> = 0 | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(RDFFR_p_f | Pd(pd));
}

void Assembler::rdffr(const PRegisterWithLaneSize& pd, const PRegisterZ& pg) {
  // RDFFR <Pd>.B, <Pg>/Z
  //  0010 0101 0001 1000 1111 000. ...0 ....
  //  op<23> = 0 | S<22> = 0 | Pg<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(RDFFR_p_p_f | Pd(pd) | Pg<8, 5>(pg));
}

void Assembler::rdffrs(const PRegisterWithLaneSize& pd, const PRegisterZ& pg) {
  // RDFFRS <Pd>.B, <Pg>/Z
  //  0010 0101 0101 1000 1111 000. ...0 ....
  //  op<23> = 0 | S<22> = 1 | Pg<8:5> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(RDFFRS_p_p_f | Pd(pd) | Pg<8, 5>(pg));
}

// SVEPropagateBreak.

void Assembler::brkpa(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn,
                      const PRegisterWithLaneSize& pm) {
  // BRKPA <Pd>.B, <Pg>/Z, <Pn>.B, <Pm>.B
  //  0010 0101 0000 .... 11.. ..0. ...0 ....
  //  op<23> = 0 | S<22> = 0 | Pm<19:16> | Pg<13:10> | Pn<8:5> | B<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(BRKPA_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::brkpas(const PRegisterWithLaneSize& pd,
                       const PRegisterZ& pg,
                       const PRegisterWithLaneSize& pn,
                       const PRegisterWithLaneSize& pm) {
  // BRKPAS <Pd>.B, <Pg>/Z, <Pn>.B, <Pm>.B
  //  0010 0101 0100 .... 11.. ..0. ...0 ....
  //  op<23> = 0 | S<22> = 1 | Pm<19:16> | Pg<13:10> | Pn<8:5> | B<4> = 0 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(BRKPAS_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::brkpb(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const PRegisterWithLaneSize& pn,
                      const PRegisterWithLaneSize& pm) {
  // BRKPB <Pd>.B, <Pg>/Z, <Pn>.B, <Pm>.B
  //  0010 0101 0000 .... 11.. ..0. ...1 ....
  //  op<23> = 0 | S<22> = 0 | Pm<19:16> | Pg<13:10> | Pn<8:5> | B<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(BRKPB_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

void Assembler::brkpbs(const PRegisterWithLaneSize& pd,
                       const PRegisterZ& pg,
                       const PRegisterWithLaneSize& pn,
                       const PRegisterWithLaneSize& pm) {
  // BRKPBS <Pd>.B, <Pg>/Z, <Pn>.B, <Pm>.B
  //  0010 0101 0100 .... 11.. ..0. ...1 ....
  //  op<23> = 0 | S<22> = 1 | Pm<19:16> | Pg<13:10> | Pn<8:5> | B<4> = 1 |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(BRKPBS_p_p_pp | Pd(pd) | Pg<13, 10>(pg) | Pn(pn) | Pm(pm));
}

// SVEStackFrameAdjustment.

void Assembler::addpl(const Register& xd, const Register& xn, int imm6) {
  // ADDPL <Xd|SP>, <Xn|SP>, #<imm>
  //  0000 0100 011. .... 0101 0... .... ....
  //  op<22> = 1 | Rn<20:16> | imm6<10:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX());
  VIXL_ASSERT(xn.IsX());

  Emit(ADDPL_r_ri | RdSP(xd) | RmSP(xn) | ImmField<10, 5>(imm6));
}

void Assembler::addvl(const Register& xd, const Register& xn, int imm6) {
  // ADDVL <Xd|SP>, <Xn|SP>, #<imm>
  //  0000 0100 001. .... 0101 0... .... ....
  //  op<22> = 0 | Rn<20:16> | imm6<10:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX());
  VIXL_ASSERT(xn.IsX());

  Emit(ADDVL_r_ri | RdSP(xd) | RmSP(xn) | ImmField<10, 5>(imm6));
}

// SVEStackFrameSize.

void Assembler::rdvl(const Register& xd, int imm6) {
  // RDVL <Xd>, #<imm>
  //  0000 0100 1011 1111 0101 0... .... ....
  //  op<22> = 0 | opc2<20:16> = 11111 | imm6<10:5> | Rd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(xd.IsX());

  Emit(RDVL_r_i | Rd(xd) | ImmField<10, 5>(imm6));
}

// SVEVectorSelect.

void Assembler::sel(const ZRegister& zd,
                    const PRegister& pg,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(SEL_z_p_zz | SVESize(zd) | Rd(zd) | Pg<13, 10>(pg) | Rn(zn) | Rm(zm));
}

// SVEWriteFFR.

void Assembler::setffr() {
  // SETFFR
  //  0010 0101 0010 1100 1001 0000 0000 0000
  //  opc<23:22> = 00

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(SETFFR_f);
}

void Assembler::wrffr(const PRegisterWithLaneSize& pn) {
  // WRFFR <Pn>.B
  //  0010 0101 0010 1000 1001 000. ...0 0000
  //  opc<23:22> = 00 | Pn<8:5>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));

  Emit(WRFFR_f_p | Rx<8, 5>(pn));
}

// Aliases.

void Assembler::bic(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  and_(zd, zn, ~imm);
}

void Assembler::eon(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  eor(zd, zn, ~imm);
}

void Assembler::orn(const ZRegister& zd, const ZRegister& zn, uint64_t imm) {
  orr(zd, zn, ~imm);
}


void Assembler::fmov(const ZRegister& zd, const PRegisterM& pg, double imm) {
  if (IsPositiveZero(imm)) {
    cpy(zd, pg, 0);
  } else {
    fcpy(zd, pg, imm);
  }
}

void Assembler::fmov(const ZRegister& zd, double imm) {
  if (IsPositiveZero(imm)) {
    dup(zd, 0);
  } else {
    fdup(zd, imm);
  }
}

void Assembler::mov(const PRegister& pd, const PRegister& pn) {
  // If the inputs carry a lane size, they must match.
  VIXL_ASSERT((!pd.HasLaneSize() && !pn.HasLaneSize()) ||
              AreSameLaneSize(pd, pn));
  orr(pd.VnB(), pn.Zeroing(), pn.VnB(), pn.VnB());
}

void Assembler::mov(const PRegisterWithLaneSize& pd,
                    const PRegisterM& pg,
                    const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  sel(pd, pg, pn, pd);
}

void Assembler::mov(const PRegisterWithLaneSize& pd,
                    const PRegisterZ& pg,
                    const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  and_(pd, pg, pn, pn);
}

void Assembler::mov(const ZRegister& zd,
                    const PRegister& pg,
                    int imm8,
                    int shift) {
  VIXL_ASSERT(pg.IsMerging() || pg.IsZeroing());
  cpy(zd, pg, imm8, shift);
}

void Assembler::mov(const ZRegister& zd, const Register& xn) { dup(zd, xn); }

void Assembler::mov(const ZRegister& zd, const VRegister& vn) {
  VIXL_ASSERT(vn.IsScalar());
  VIXL_ASSERT(AreSameLaneSize(zd, vn));
  dup(zd, vn.Z().WithSameLaneSizeAs(vn), 0);
}

void Assembler::mov(const ZRegister& zd, const ZRegister& zn) {
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  orr(zd.VnD(), zn.VnD(), zn.VnD());
}

void Assembler::mov(const ZRegister& zd, const ZRegister& zn, unsigned index) {
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  dup(zd, zn, index);
}

void Assembler::mov(const ZRegister& zd,
                    const PRegisterM& pg,
                    const Register& rn) {
  cpy(zd, pg, rn);
}

void Assembler::mov(const ZRegister& zd,
                    const PRegisterM& pg,
                    const VRegister& vn) {
  VIXL_ASSERT(vn.IsScalar());
  VIXL_ASSERT(AreSameLaneSize(zd, vn));
  cpy(zd, pg, vn);
}

void Assembler::mov(const ZRegister& zd,
                    const PRegisterM& pg,
                    const ZRegister& zn) {
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  sel(zd, pg, zn, zd);
}

void Assembler::mov(const ZRegister& zd, uint64_t imm) {
  // Mov is an alias of dupm for certain values of imm. Whilst this matters in
  // the disassembler, for the assembler, we don't distinguish between the
  // two mnemonics, and simply call dupm.
  dupm(zd, imm);
}

void Assembler::mov(const ZRegister& zd, int imm8, int shift) {
  dup(zd, imm8, shift);
}

void Assembler::movs(const PRegister& pd, const PRegister& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  orrs(pd.VnB(), pn.Zeroing(), pn.VnB(), pn.VnB());
}

void Assembler::movs(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  ands(pd, pg, pn, pn);
}

void Assembler::not_(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  eor(pd, pg, pn, pg.VnB());
}

void Assembler::nots(const PRegisterWithLaneSize& pd,
                     const PRegisterZ& pg,
                     const PRegisterWithLaneSize& pn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  eors(pd, pg, pn, pg.VnB());
}

// SVE2

void Assembler::adclb(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // ADCLB <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 0.0. .... 1101 00.. .... ....
  //  size<23:22> | Zm<20:16> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());

  Instr sz = zda.IsLaneSizeD() ? (1 << 22) : 0;
  Emit(0x4500d000 | sz | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::adclt(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // ADCLT <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 0.0. .... 1101 01.. .... ....
  //  size<23:22> | Zm<20:16> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());

  Instr sz = zda.IsLaneSizeD() ? (1 << 22) : 0;
  Emit(0x4500d400 | sz | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::addhnb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // ADDHNB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0110 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45206000 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::addhnt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // ADDHNT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0110 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45206400 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::addp(const ZRegister& zd,
                     const PRegisterM& pg,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // ADDP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0001 101. .... .... ....
  //  size<23:22> | opc<18:17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4411a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::bcax(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     const ZRegister& zk) {
  // BCAX <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 011. .... 0011 10.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04603800 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::bdep(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // BDEP <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1011 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEBitPerm));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4500b400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::bext(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // BEXT <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1011 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEBitPerm));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4500b000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::bgrp(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // BGRP <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1011 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEBitPerm));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4500b800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::bsl(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    const ZRegister& zk) {
  // BSL <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 001. .... 0011 11.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04203c00 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::bsl1n(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      const ZRegister& zk) {
  // BSL1N <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 011. .... 0011 11.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04603c00 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::bsl2n(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      const ZRegister& zk) {
  // BSL2N <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 101. .... 0011 11.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04a03c00 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::cadd(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int rot) {
  // CADD <Zdn>.<T>, <Zdn>.<T>, <Zm>.<T>, <const>
  //  0100 0101 ..00 0000 1101 1... .... ....
  //  size<23:22> | op<16> | rot<10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT((rot == 90) || (rot == 270));

  Instr rotate_bit = (rot == 90) ? 0 : (1 << 10);
  Emit(0x4500d800 | rotate_bit | SVESize(zd) | Rd(zd) | Rn(zm));
}

void Assembler::cdot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index,
                     int rot) {
  // CDOT <Zda>.D, <Zn>.H, <Zm>.H[<imm>], <const>
  //  0100 0100 111. .... 0100 .... .... ....
  //  size<23:22> | opc<20:16> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));
  VIXL_ASSERT(index >= 0);

  Instr zm_and_idx = 0;
  if (zm.IsLaneSizeB()) {
    // Zm<18:16> | i2<20:19>
    VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 3));
    zm_and_idx = (index << 19) | Rx<18, 16>(zm);
  } else {
    // Zm<19:16> | i1<20>
    VIXL_ASSERT(zm.IsLaneSizeH());
    VIXL_ASSERT((zm.GetCode() <= 15) && (index <= 1));
    zm_and_idx = (index << 20) | Rx<19, 16>(zm);
  }

  Instr rotate_bits = (rot / 90) << 10;
  Emit(0x44a04000 | zm_and_idx | rotate_bits | SVESize(zda) | Rd(zda) | Rn(zn));
}

void Assembler::cdot(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int rot) {
  // CDOT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>, <const>
  //  0100 0100 ..0. .... 0001 .... .... ....
  //  size<23:22> | Zm<20:16> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 4));

  Instr rotate_bits = (rot / 90) << 10;
  Emit(0x44001000 | rotate_bits | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::cmla(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int index,
                     int rot) {
  // CMLA <Zda>.H, <Zn>.H, <Zm>.H[<imm>], <const>
  //  0100 0100 101. .... 0110 .... .... ....
  //  size<23:22> | opc<20:16> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 10;
  Emit(0x44a06000 | SVEMulComplexIndexHelper(zm, index) | rotate_bit | Rd(zda) |
       Rn(zn));
}

void Assembler::cmla(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     int rot) {
  // CMLA <Zda>.<T>, <Zn>.<T>, <Zm>.<T>, <const>
  //  0100 0100 ..0. .... 0010 .... .... ....
  //  size<23:22> | Zm<20:16> | op<12> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 10;
  Emit(0x44002000 | rotate_bit | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::eor3(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     const ZRegister& zk) {
  // EOR3 <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 001. .... 0011 10.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04203800 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::eorbt(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // EORBT <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1001 00.. .... ....
  //  size<23:22> | Zm<20:16> | tb<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x45009000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::eortb(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // EORTB <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1001 01.. .... ....
  //  size<23:22> | Zm<20:16> | tb<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x45009400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::faddp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FADDP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0100 ..01 0000 100. .... .... ....
  //  size<23:22> | opc<18:16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x64108000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fcvtlt(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // FCVTLT <Zd>.S, <Pg>/M, <Zn>.H
  //  0110 0100 1000 1001 101. .... .... ....
  //  opc<23:22> | opc2<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Instr op;
  if (zd.IsLaneSizeD() && zn.IsLaneSizeS()) {
    op = 0x64cba000;
  } else {
    VIXL_ASSERT(zd.IsLaneSizeS() && zn.IsLaneSizeH());
    op = 0x6489a000;
  }

  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcvtnt(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // FCVTNT <Zd>.S, <Pg>/M, <Zn>.D
  //  0110 0100 1100 1010 101. .... .... ....
  //  opc<23:22> | opc2<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Instr op;
  if (zd.IsLaneSizeS() && zn.IsLaneSizeD()) {
    op = 0x64caa000;
  } else {
    VIXL_ASSERT(zd.IsLaneSizeH() && zn.IsLaneSizeS());
    op = 0x6488a000;
  }
  Emit(op | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcvtx(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  // FCVTX <Zd>.S, <Pg>/M, <Zn>.D
  //  0110 0101 0000 1010 101. .... .... ....
  //  opc<23:22> | opc2<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() && zn.IsLaneSizeD());

  Emit(0x650aa000 | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fcvtxnt(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn) {
  // FCVTXNT <Zd>.S, <Pg>/M, <Zn>.D
  //  0110 0100 0000 1010 101. .... .... ....
  //  opc<23:22> | opc2<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0x640aa000 | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::flogb(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  // FLOGB <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0110 0101 0001 1..0 101. .... .... ....
  //  opc<23:22> | opc2<18:17> | U<16> | Pg<12:10> | Zn<9:5> | Zd<4:0> | size<>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(!zd.IsLaneSizeB());

  // Size field is encoded in bits <18:17> rather than <23:22>.
  Instr size = SVESize(zd) >> 5;
  Emit(0x6518a000 | size | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::fmaxnmp(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // FMAXNMP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0100 ..01 0100 100. .... .... ....
  //  size<23:22> | opc<18:16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x64148000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmaxp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FMAXP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0100 ..01 0110 100. .... .... ....
  //  size<23:22> | opc<18:16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x64168000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fminnmp(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // FMINNMP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0100 ..01 0101 100. .... .... ....
  //  size<23:22> | opc<18:16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x64158000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fminp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // FMINP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0110 0100 ..01 0111 100. .... .... ....
  //  size<23:22> | opc<18:16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x64178000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::fmlalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMLALB <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1000 00.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());

  Emit(0x64a08000 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::fmlalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm,
                       int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());
  VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
  Instr zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                     (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);

  Emit(0x64a04000 | Rd(zda) | Rn(zn) | zm_and_idx);
}

void Assembler::fmlalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMLALT <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1000 01.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());

  Emit(0x64a08400 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::fmlalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm,
                       int index) {
  // FMLALT <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1000 01.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());
  VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
  Instr zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                     (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);

  Emit(0x64a04400 | Rd(zda) | Rn(zn) | zm_and_idx);
}

void Assembler::fmlslb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMLSLB <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1010 00.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());

  Emit(0x64a0a000 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::fmlslb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm,
                       int index) {
  // FMLSLB <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1010 00.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());
  VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
  Instr zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                     (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);

  Emit(0x64a06000 | Rd(zda) | Rn(zn) | zm_and_idx);
}

void Assembler::fmlslt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // FMLSLT <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1010 01.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());

  Emit(0x64a0a400 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::fmlslt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm,
                       int index) {
  // FMLSLT <Zda>.S, <Zn>.H, <Zm>.H
  //  0110 0100 101. .... 1010 01.. .... ....
  //  o2<22> | Zm<20:16> | op<13> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeH() && zm.IsLaneSizeH());
  VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
  Instr zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                     (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);

  Emit(0x64a06400 | Rd(zda) | Rn(zn) | zm_and_idx);
}

void Assembler::histcnt(const ZRegister& zd,
                        const PRegisterZ& pg,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // HISTCNT <Zd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..1. .... 110. .... .... ....
  //  size<23:22> | Zm<20:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeD());

  Emit(0x4520c000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::histseg(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // HISTSEG <Zd>.B, <Zn>.B, <Zm>.B
  //  0100 0101 ..1. .... 1010 00.. .... ....
  //  size<23:22> | Zm<20:16> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT(zd.IsLaneSizeB());

  Emit(0x4520a000 | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::match(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // MATCH <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..1. .... 100. .... ...0 ....
  //  size<23:22> | Zm<20:16> | Pg<12:10> | Zn<9:5> | op<4> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(pd, zn, zm));
  VIXL_ASSERT(zm.IsLaneSizeB() || zm.IsLaneSizeH());

  Emit(0x45208000 | SVESize(zm) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::mla(const ZRegister& zda,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    int index) {
  // MLA <Zda>.D, <Zn>.D, <Zm>.D[<imm>]
  //  0100 0100 111. .... 0000 10.. .... ....
  //  size<23:22> | opc<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Instr synthesised_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           0x44200800,
                                           0x44a00800,
                                           0x44e00800);

  Emit(synthesised_op | Rd(zda) | Rn(zn));
}

void Assembler::mls(const ZRegister& zda,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    int index) {
  // MLS <Zda>.D, <Zn>.D, <Zm>.D[<imm>]
  //  0100 0100 111. .... 0000 11.. .... ....
  //  size<23:22> | opc<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Instr synthesised_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           0x44200c00,
                                           0x44a00c00,
                                           0x44e00c00);

  Emit(synthesised_op | Rd(zda) | Rn(zn));
}

void Assembler::mul(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    int index) {
  // MUL <Zd>.D, <Zn>.D, <Zm>.D[<imm>]
  //  0100 0100 111. .... 1111 10.. .... ....
  //  size<23:22> | opc<20:16> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Instr synthesised_op = SVEMulIndexHelper(zd.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           0x4420f800,
                                           0x44a0f800,
                                           0x44e0f800);

  Emit(synthesised_op | Rd(zd) | Rn(zn));
}

void Assembler::mul(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // MUL <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0110 00.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x04206000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::nbsl(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm,
                     const ZRegister& zk) {
  // NBSL <Zdn>.D, <Zdn>.D, <Zm>.D, <Zk>.D
  //  0000 0100 111. .... 0011 11.. .... ....
  //  opc<23:22> | Zm<20:16> | o2<10> | Zk<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm, zk));
  VIXL_ASSERT(zd.IsLaneSizeD());

  Emit(0x04e03c00 | Rd(zd) | Rm(zm) | Rn(zk));
}

void Assembler::nmatch(const PRegisterWithLaneSize& pd,
                       const PRegisterZ& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // NMATCH <Pd>.<T>, <Pg>/Z, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..1. .... 100. .... ...1 ....
  //  size<23:22> | Zm<20:16> | Pg<12:10> | Zn<9:5> | op<4> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(pd, zn, zm));
  VIXL_ASSERT(zm.IsLaneSizeB() || zm.IsLaneSizeH());

  Emit(0x45208010 | SVESize(zm) | Pd(pd) | PgLow8(pg) | Rn(zn) | Rm(zm));
}

void Assembler::pmul(const ZRegister& zd,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // PMUL <Zd>.B, <Zn>.B, <Zm>.B
  //  0000 0100 001. .... 0110 01.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0x04206400 | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::pmullb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // PMULLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0110 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEPmull128) || !zd.IsLaneSizeQ());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeS());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);
  Instr size = zd.IsLaneSizeQ() ? 0 : SVESize(zd);

  Emit(0x45006800 | size | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::pmullt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // PMULLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0110 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEPmull128) || !zd.IsLaneSizeQ());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeS());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);
  Instr size = zd.IsLaneSizeQ() ? 0 : SVESize(zd);

  Emit(0x45006c00 | size | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::raddhnb(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // RADDHNB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0110 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45206800 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::raddhnt(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // RADDHNT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0110 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45206c00 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

#define VIXL_SVE_SHR_LIST(V) \
  V(rshrnb, 0x45201800)      \
  V(rshrnt, 0x45201c00)      \
  V(shrnb, 0x45201000)       \
  V(shrnt, 0x45201400)       \
  V(sqrshrnb, 0x45202800)    \
  V(sqrshrnt, 0x45202c00)    \
  V(sqrshrunb, 0x45200800)   \
  V(sqrshrunt, 0x45200c00)   \
  V(sqshrnb, 0x45202000)     \
  V(sqshrnt, 0x45202400)     \
  V(sqshrunb, 0x45200000)    \
  V(sqshrunt, 0x45200400)    \
  V(uqrshrnb, 0x45203800)    \
  V(uqrshrnt, 0x45203c00)    \
  V(uqshrnb, 0x45203000)     \
  V(uqshrnt, 0x45203400)

#define VIXL_DEFINE_ASM_FUNC(MNE, X)                                         \
  void Assembler::MNE(const ZRegister& zd, const ZRegister& zn, int shift) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));                                 \
    VIXL_ASSERT(!zd.IsLaneSizeD() && !zd.IsLaneSizeQ());                     \
    VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));   \
    Instr encoded_imm =                                                      \
        EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());         \
    SVEBitwiseShiftImmediate(zd, zn, encoded_imm, X);                        \
  }
VIXL_SVE_SHR_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

void Assembler::rsubhnb(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // RSUBHNB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0111 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45207800 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::rsubhnt(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // RSUBHNT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0111 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45207c00 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::saba(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // SABA <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1111 10.. .... ....
  //  size<23:22> | Zm<20:16> | U<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(0x4500f800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sabalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SABALB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1100 00.. .... ....
  //  size<23:22> | Zm<20:16> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4500c000 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sabalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SABALT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1100 01.. .... ....
  //  size<23:22> | Zm<20:16> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4500c400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sabdlb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SABDLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0011 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45003000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sabdlt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SABDLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0011 01.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45003400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sadalp(const ZRegister& zda,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // SADALP <Zda>.<T>, <Pg>/M, <Zn>.<Tb>
  //  0100 0100 ..00 0100 101. .... .... ....
  //  size<23:22> | U<16> | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4404a000 | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn));
}

void Assembler::saddlb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SADDLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0000 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45000000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::saddlbt(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // SADDLBT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1000 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | tb<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.IsLaneSizeD() || zd.IsLaneSizeH() || zd.IsLaneSizeS());

  Emit(0x45008000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::saddlt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SADDLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0000 01.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45000400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::saddwb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SADDWB <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0100 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45004000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::saddwt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SADDWT <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0100 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45004400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sbclb(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SBCLB <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 1.0. .... 1101 00.. .... ....
  //  size<23:22> | Zm<20:16> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());

  Instr sz = zda.IsLaneSizeD() ? (1 << 22) : 0;
  Emit(0x4580d000 | sz | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sbclt(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SBCLT <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 1.0. .... 1101 01.. .... ....
  //  size<23:22> | Zm<20:16> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());

  Instr sz = zda.IsLaneSizeD() ? (1 << 22) : 0;
  Emit(0x4580d400 | sz | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::shadd(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SHADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0000 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44108000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::shsub(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SHSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0010 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44128000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::shsubr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SHSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0110 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44168000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sli(const ZRegister& zd, const ZRegister& zn, int shift) {
  // SLI <Zd>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1111 01.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | op<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500f400);
}

void Assembler::smaxp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SMAXP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0100 101. .... .... ....
  //  size<23:22> | opc<18:17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4414a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sminp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SMINP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0110 101. .... .... ....
  //  size<23:22> | opc<18:17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4416a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

#define VIXL_SVE_MULL_INDEX_LIST(V) \
  V(smullb, 0x44a0c000)             \
  V(smullt, 0x44a0c400)             \
  V(umullb, 0x44a0d000)             \
  V(umullt, 0x44a0d400)             \
  V(smlalb, 0x44a08000)             \
  V(smlalt, 0x44a08400)             \
  V(smlslb, 0x44a0a000)             \
  V(smlslt, 0x44a0a400)             \
  V(umlalb, 0x44a09000)             \
  V(umlalt, 0x44a09400)             \
  V(umlslb, 0x44a0b000)             \
  V(umlslt, 0x44a0b400)             \
  V(sqdmullb, 0x44a0e000)           \
  V(sqdmullt, 0x44a0e400)

#define VIXL_DEFINE_ASM_FUNC(MNE, OP)                                     \
  void Assembler::MNE(const ZRegister& zda,                               \
                      const ZRegister& zn,                                \
                      const ZRegister& zm,                                \
                      int index) {                                        \
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));                              \
    VIXL_ASSERT(AreSameLaneSize(zn, zm));                                 \
    VIXL_ASSERT(zda.IsLaneSizeD() || zda.IsLaneSizeS());                  \
    VIXL_ASSERT(zda.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2); \
    Instr zm_with_index = SVEMulLongIndexHelper(zm, index);               \
    Emit(OP | SVESize(zda) | Rd(zda) | Rn(zn) | zm_with_index);           \
  }
VIXL_SVE_MULL_INDEX_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FuNC

void Assembler::smlalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMLALB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0100 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44004000 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::smlalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMLALT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0100 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44004400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::smlslb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMLSLB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0101 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44005000 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::smlslt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMLSLT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0101 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44005400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::smulh(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SMULH <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0110 10.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x04206800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::smullb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMULLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0111 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45007000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::smullt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SMULLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0111 01.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45007400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqabs(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  // SQABS <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0100 0100 ..00 1000 101. .... .... ....
  //  size<23:22> | Q<19> | opc<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(0x4408a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sqadd(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SQADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1000 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44188000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqcadd(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm,
                       int rot) {
  // SQCADD <Zdn>.<T>, <Zdn>.<T>, <Zm>.<T>, <const>
  //  0100 0101 ..00 0001 1101 1... .... ....
  //  size<23:22> | op<16> | rot<10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));
  VIXL_ASSERT((rot == 90) || (rot == 270));

  Instr rotate_bit = (rot == 90) ? 0 : (1 << 10);
  Emit(0x4501d800 | rotate_bit | SVESize(zd) | Rd(zd) | Rn(zm));
}

// This prototype maps to 2 instruction encodings:
//  sqdmlalb_z_zzzi_d
//  sqdmlalb_z_zzzi_s
void Assembler::sqdmlalb(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  // SQDMLALB <Zda>.D, <Zn>.S, <Zm>.S[<imm>]
  //  0100 0100 111. .... 0010 .0.. .... ....
  //  size<23:22> | opc<20:16> | S<12> | il<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(index >= 0);

  Instr zm_and_idx = 0;
  if (zm.IsLaneSizeH()) {
    // Zm<18:16> | i3h<20:19> | i3l<11>
    VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
    zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                 (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);
  } else {
    // Zm<19:16> | i2h<20> | i2l<11>
    VIXL_ASSERT(zm.IsLaneSizeS());
    VIXL_ASSERT((zm.GetCode() <= 15) && (index <= 3));
    zm_and_idx = (ExtractBit(index, 1) << 20) | (ExtractBit(index, 0) << 11) |
                 Rx<19, 16>(zm);
  }

  Emit(0x44202000 | zm_and_idx | SVESize(zda) | Rd(zda) | Rn(zn));
}

void Assembler::sqdmlalb(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMLALB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0110 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44006000 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sqdmlalbt(const ZRegister& zda,
                          const ZRegister& zn,
                          const ZRegister& zm) {
  // SQDMLALBT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0000 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44000800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  sqdmlalt_z_zzzi_d
//  sqdmlalt_z_zzzi_s
void Assembler::sqdmlalt(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  // SQDMLALT <Zda>.D, <Zn>.S, <Zm>.S[<imm>]
  //  0100 0100 111. .... 0010 .1.. .... ....
  //  size<23:22> | opc<20:16> | S<12> | il<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(index >= 0);

  Instr zm_and_idx = 0;
  if (zm.IsLaneSizeH()) {
    // Zm<18:16> | i3h<20:19> | i3l<11>
    VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
    zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                 (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);
  } else {
    // Zm<19:16> | i2h<20> | i2l<11>
    VIXL_ASSERT(zm.IsLaneSizeS());
    VIXL_ASSERT((zm.GetCode() <= 15) && (index <= 3));
    zm_and_idx = (ExtractBit(index, 1) << 20) | (ExtractBit(index, 0) << 11) |
                 Rx<19, 16>(zm);
  }

  Emit(0x44202400 | zm_and_idx | SVESize(zda) | Rd(zda) | Rn(zn));
}

void Assembler::sqdmlalt(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMLALT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0110 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44006400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  sqdmlslb_z_zzzi_d
//  sqdmlslb_z_zzzi_s
void Assembler::sqdmlslb(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  // SQDMLSLB <Zda>.D, <Zn>.S, <Zm>.S[<imm>]
  //  0100 0100 111. .... 0011 .0.. .... ....
  //  size<23:22> | opc<20:16> | S<12> | il<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(index >= 0);

  Instr zm_and_idx = 0;
  if (zm.IsLaneSizeH()) {
    // Zm<18:16> | i3h<20:19> | i3l<11>
    VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
    zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                 (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);
  } else {
    // Zm<19:16> | i2h<20> | i2l<11>
    VIXL_ASSERT(zm.IsLaneSizeS());
    VIXL_ASSERT((zm.GetCode() <= 15) && (index <= 3));
    zm_and_idx = (ExtractBit(index, 1) << 20) | (ExtractBit(index, 0) << 11) |
                 Rx<19, 16>(zm);
  }

  Emit(0x44203000 | zm_and_idx | SVESize(zda) | Rd(zda) | Rn(zn));
}

void Assembler::sqdmlslb(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMLSLB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0110 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44006800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sqdmlslbt(const ZRegister& zda,
                          const ZRegister& zn,
                          const ZRegister& zm) {
  // SQDMLSLBT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0000 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44000c00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// This prototype maps to 2 instruction encodings:
//  sqdmlslt_z_zzzi_d
//  sqdmlslt_z_zzzi_s
void Assembler::sqdmlslt(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  // SQDMLSLT <Zda>.D, <Zn>.S, <Zm>.S[<imm>]
  //  0100 0100 111. .... 0011 .1.. .... ....
  //  size<23:22> | opc<20:16> | S<12> | il<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.IsLaneSizeS() || zda.IsLaneSizeD());
  VIXL_ASSERT(index >= 0);

  Instr zm_and_idx = 0;
  if (zm.IsLaneSizeH()) {
    // Zm<18:16> | i3h<20:19> | i3l<11>
    VIXL_ASSERT((zm.GetCode() <= 7) && (index <= 7));
    zm_and_idx = (ExtractUnsignedBitfield32(2, 1, index) << 19) |
                 (ExtractBit(index, 0) << 11) | Rx<18, 16>(zm);
  } else {
    // Zm<19:16> | i2h<20> | i2l<11>
    VIXL_ASSERT(zm.IsLaneSizeS());
    VIXL_ASSERT((zm.GetCode() <= 15) && (index <= 3));
    zm_and_idx = (ExtractBit(index, 1) << 20) | (ExtractBit(index, 0) << 11) |
                 Rx<19, 16>(zm);
  }

  Emit(0x44203400 | zm_and_idx | SVESize(zda) | Rd(zda) | Rn(zn));
}

void Assembler::sqdmlslt(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMLSLT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0110 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x44006c00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sqdmulh(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm,
                        int index) {
  // SQDMULH <Zd>.D, <Zn>.D, <Zm>.D[<imm>]
  //  0100 0100 111. .... 1111 00.. .... ....
  //  size<23:22> | opc<20:16> | R<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Instr synthesised_op = SVEMulIndexHelper(zd.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           0x4420f000,
                                           0x44a0f000,
                                           0x44e0f000);

  Emit(synthesised_op | Rd(zd) | Rn(zn));
}

void Assembler::sqdmulh(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // SQDMULH <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0111 00.. .... ....
  //  size<23:22> | Zm<20:16> | R<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x04207000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqdmullb(const ZRegister& zd,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMULLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0110 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45006000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqdmullt(const ZRegister& zd,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQDMULLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0110 01.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45006400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqneg(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn) {
  // SQNEG <Zd>.<T>, <Pg>/M, <Zn>.<T>
  //  0100 0100 ..00 1001 101. .... .... ....
  //  size<23:22> | Q<19> | opc<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));

  Emit(0x4409a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::sqrdcmlah(const ZRegister& zda,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int index,
                          int rot) {
  // SQRDCMLAH <Zda>.H, <Zn>.H, <Zm>.H[<imm>], <const>
  //  0100 0100 101. .... 0111 .... .... ....
  //  size<23:22> | opc<20:16> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 10;
  Emit(0x44a07000 | SVEMulComplexIndexHelper(zm, index) | rotate_bit | Rd(zda) |
       Rn(zn));
}

void Assembler::sqrdcmlah(const ZRegister& zda,
                          const ZRegister& zn,
                          const ZRegister& zm,
                          int rot) {
  // SQRDCMLAH <Zda>.<T>, <Zn>.<T>, <Zm>.<T>, <const>
  //  0100 0100 ..0. .... 0011 .... .... ....
  //  size<23:22> | Zm<20:16> | op<12> | rot<11:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));
  VIXL_ASSERT((rot == 0) || (rot == 90) || (rot == 180) || (rot == 270));

  Instr rotate_bit = (rot / 90) << 10;
  Emit(0x44003000 | rotate_bit | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// This prototype maps to 3 instruction encodings:
//  sqrdmlah_z_zzzi_d
//  sqrdmlah_z_zzzi_h
//  sqrdmlah_z_zzzi_s
void Assembler::sqrdmlah(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Instr op_h = 0x44201000;
  Instr op_s = op_h | (1 << 23);
  Instr op_d = op_h | (3 << 22);
  // The encoding of opcode, index, Zm, and size are synthesized in this
  // variable.
  Instr synthesized_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           op_h,
                                           op_s,
                                           op_d);

  Emit(synthesized_op | Rd(zda) | Rn(zn));
}

void Assembler::sqrdmlah(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQRDMLAH <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0100 ..0. .... 0111 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(0x44007000 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

// This prototype maps to 3 instruction encodings:
//  sqrdmlsh_z_zzzi_d
//  sqrdmlsh_z_zzzi_h
//  sqrdmlsh_z_zzzi_s
void Assembler::sqrdmlsh(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Instr op_h = 0x44201400;
  Instr op_s = op_h | (1 << 23);
  Instr op_d = op_h | (3 << 22);
  // The encoding of opcode, index, Zm, and size are synthesized in this
  // variable.
  Instr synthesized_op = SVEMulIndexHelper(zda.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           op_h,
                                           op_s,
                                           op_d);

  Emit(synthesized_op | Rd(zda) | Rn(zn));
}

void Assembler::sqrdmlsh(const ZRegister& zda,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQRDMLSH <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0100 ..0. .... 0111 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(0x44007400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::sqrdmulh(const ZRegister& zd,
                         const ZRegister& zn,
                         const ZRegister& zm,
                         int index) {
  // SQRDMULH <Zd>.D, <Zn>.D, <Zm>.D[<imm>]
  //  0100 0100 111. .... 1111 01.. .... ....
  //  size<23:22> | opc<20:16> | R<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Instr synthesised_op = SVEMulIndexHelper(zd.GetLaneSizeInBytesLog2(),
                                           zm,
                                           index,
                                           0x4420f400,
                                           0x44a0f400,
                                           0x44e0f400);

  Emit(synthesised_op | Rd(zd) | Rn(zn));
}

void Assembler::sqrdmulh(const ZRegister& zd,
                         const ZRegister& zn,
                         const ZRegister& zm) {
  // SQRDMULH <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0111 01.. .... ....
  //  size<23:22> | Zm<20:16> | R<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x04207400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::sqrshl(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SQRSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1010 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440a8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqrshlr(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // SQRSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1110 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440e8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      int shift) {
  // SQSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0110 100. .... .... ....
  //  tszh<23:22> | opc<19:18> | L<17> | U<16> | Pg<12:10> | tszl<9:8> |
  //  imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, 0x04068000);
}

void Assembler::sqshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SQSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1000 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44088000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqshlr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SQSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1100 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440c8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqshlu(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       int shift) {
  // SQSHLU <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 1111 100. .... .... ....
  //  tszh<23:22> | opc<19:18> | L<17> | U<16> | Pg<12:10> | tszl<9:8> |
  //  imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, 0x040f8000);
}

void Assembler::sqsub(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SQSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1010 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441a8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqsubr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SQSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1110 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441e8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sqxtnb(const ZRegister& zd, const ZRegister& zn) {
  // SQXTNB <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0100 00.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45204000);
}

void Assembler::sqxtnt(const ZRegister& zd, const ZRegister& zn) {
  // SQXTNT <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0100 01.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45204400);
}

void Assembler::sqxtunb(const ZRegister& zd, const ZRegister& zn) {
  // SQXTUNB <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0101 00.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45205000);
}

void Assembler::sqxtunt(const ZRegister& zd, const ZRegister& zn) {
  // SQXTUNT <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0101 01.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45205400);
}

void Assembler::srhadd(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SRHADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0100 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44148000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::sri(const ZRegister& zd, const ZRegister& zn, int shift) {
  // SRI <Zd>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1111 00.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | op<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500f000);
}

void Assembler::srshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // SRSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 0010 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44028000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::srshlr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SRSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 0110 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44068000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::srshr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      int shift) {
  // SRSHR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 1100 100. .... .... ....
  //  tszh<23:22> | opc<19:18> | L<17> | U<16> | Pg<12:10> | tszl<9:8> |
  //  imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, 0x040c8000);
}

void Assembler::srsra(const ZRegister& zda, const ZRegister& zn, int shift) {
  // SRSRA <Zda>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1110 10.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | R<11> | U<10> | Zn<9:5> |
  //  Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zda.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zda, zn, encoded_imm, 0x4500e800);
}

void Assembler::sshllb(const ZRegister& zd, const ZRegister& zn, int shift) {
  // SSHLLB <Zd>.<T>, <Zn>.<Tb>, #<const>
  //  0100 0101 0.0. .... 1010 00.. .... ....
  //  tszh<22> | tszl<20:19> | imm3<18:16> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zn.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500a000);
}

void Assembler::sshllt(const ZRegister& zd, const ZRegister& zn, int shift) {
  // SSHLLT <Zd>.<T>, <Zn>.<Tb>, #<const>
  //  0100 0101 0.0. .... 1010 01.. .... ....
  //  tszh<22> | tszl<20:19> | imm3<18:16> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zn.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500a400);
}

void Assembler::ssra(const ZRegister& zda, const ZRegister& zn, int shift) {
  // SSRA <Zda>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1110 00.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | R<11> | U<10> | Zn<9:5> |
  //  Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zda.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zda, zn, encoded_imm, 0x4500e000);
}

void Assembler::ssublb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SSUBLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0001 00.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45001000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ssublbt(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // SSUBLBT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1000 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | tb<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.IsLaneSizeD() || zd.IsLaneSizeH() || zd.IsLaneSizeS());

  Emit(0x45008800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ssublt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SSUBLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0001 01.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45001400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ssubltb(const ZRegister& zd,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // SSUBLTB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1000 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<11> | tb<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.IsLaneSizeD() || zd.IsLaneSizeH() || zd.IsLaneSizeS());

  Emit(0x45008c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ssubwb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SSUBWB <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0101 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45005000 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::ssubwt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SSUBWT <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0101 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45005400 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

#if 0
// This prototype maps to 2 instruction encodings:
//  stnt1b_z_p_ar_d_64_unscaled
//  stnt1b_z_p_ar_s_x32_unscaled
void Assembler::stnt1b(const ZRegister& zt, const PRegister& pg, const ZRegister& zn, const Register& rm) {
  // STNT1B { <Zt>.D }, <Pg>, [<Zn>.D{, <Xm>}]
  //  1110 0100 000. .... 001. .... .... ....
  //  msz<24:23> | Rm<20:16> | Pg<12:10> | Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0xe4002000 | Rt(zt) | PgLow8(pg) | Rn(zn) | Rm(rm));
}

void Assembler::stnt1d(const ZRegister& zt, const PRegister& pg, const ZRegister& zn, const Register& rm) {
  // STNT1D { <Zt>.D }, <Pg>, [<Zn>.D{, <Xm>}]
  //  1110 0101 100. .... 001. .... .... ....
  //  msz<24:23> | Rm<20:16> | Pg<12:10> | Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0xe5802000 | Rt(zt) | PgLow8(pg) | Rn(zn) | Rm(rm));
}

// This prototype maps to 2 instruction encodings:
//  stnt1h_z_p_ar_d_64_unscaled
//  stnt1h_z_p_ar_s_x32_unscaled
void Assembler::stnt1h(const ZRegister& zt, const PRegister& pg, const ZRegister& zn, const Register& rm) {
  // STNT1H { <Zt>.D }, <Pg>, [<Zn>.D{, <Xm>}]
  //  1110 0100 100. .... 001. .... .... ....
  //  msz<24:23> | Rm<20:16> | Pg<12:10> | Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0xe4802000 | Rt(zt) | PgLow8(pg) | Rn(zn) | Rm(rm));
}

// This prototype maps to 2 instruction encodings:
//  stnt1w_z_p_ar_d_64_unscaled
//  stnt1w_z_p_ar_s_x32_unscaled
void Assembler::stnt1w(const ZRegister& zt, const PRegister& pg, const ZRegister& zn, const Register& rm) {
  // STNT1W { <Zt>.D }, <Pg>, [<Zn>.D{, <Xm>}]
  //  1110 0101 000. .... 001. .... .... ....
  //  msz<24:23> | Rm<20:16> | Pg<12:10> | Zn<9:5> | Zt<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));

  Emit(0xe5002000 | Rt(zt) | PgLow8(pg) | Rn(zn) | Rm(rm));
}
#endif

void Assembler::subhnb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SUBHNB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0111 00.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45207000 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::subhnt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SUBHNT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..1. .... 0111 01.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | R<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() == (zd.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zn.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45207400 | SVESize(zn) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::suqadd(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // SUQADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1100 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441c8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::tbl(const ZRegister& zd,
                    const ZRegister& zn1,
                    const ZRegister& zn2,
                    const ZRegister& zm) {
  // TBL <Zd>.<T>, { <Zn1>.<T>, <Zn2>.<T> }, <Zm>.<T>
  //  0000 0101 ..1. .... 0010 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<10> | Zn<9:5> | Zd<4:0>

  USE(zn2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreConsecutive(zn1, zn2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn1, zn2, zm));

  Emit(0x05202800 | SVESize(zd) | Rd(zd) | Rn(zn1) | Rn(zn2) | Rm(zm));
}

void Assembler::tbx(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm) {
  // TBX <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0101 ..1. .... 0010 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x05202c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uaba(const ZRegister& zda,
                     const ZRegister& zn,
                     const ZRegister& zm) {
  // UABA <Zda>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0100 0101 ..0. .... 1111 11.. .... ....
  //  size<23:22> | Zm<20:16> | U<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(0x4500fc00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::uabalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UABALB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1100 10.. .... ....
  //  size<23:22> | Zm<20:16> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4500c800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::uabalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UABALT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 1100 11.. .... ....
  //  size<23:22> | Zm<20:16> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4500cc00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::uabdlb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UABDLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0011 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45003800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uabdlt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UABDLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0011 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45003c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uadalp(const ZRegister& zda,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // UADALP <Zda>.<T>, <Pg>/M, <Zn>.<Tb>
  //  0100 0100 ..00 0101 101. .... .... ....
  //  size<23:22> | U<16> | Pg<12:10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x4405a000 | SVESize(zda) | Rd(zda) | PgLow8(pg) | Rn(zn));
}

void Assembler::uaddlb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UADDLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0000 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45000800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uaddlt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UADDLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0000 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45000c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uaddwb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UADDWB <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0100 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45004800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uaddwt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UADDWT <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0100 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45004c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uhadd(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UHADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0001 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44118000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uhsub(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UHSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0011 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44138000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uhsubr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UHSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0111 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44178000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::umaxp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UMAXP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0101 101. .... .... ....
  //  size<23:22> | opc<18:17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4415a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uminp(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UMINP <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0111 101. .... .... ....
  //  size<23:22> | opc<18:17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x4417a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::umlalb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMLALB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0100 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44004800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::umlalt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMLALT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0100 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44004c00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::umlslb(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMLSLB <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0101 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44005800 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::umlslt(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMLSLT <Zda>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0100 ..0. .... 0101 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zda.IsLaneSizeB());
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zda.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Emit(0x44005c00 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::umulh(const ZRegister& zd,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UMULH <Zd>.<T>, <Zn>.<T>, <Zm>.<T>
  //  0000 0100 ..1. .... 0110 11.. .... ....
  //  size<23:22> | Zm<20:16> | opc<11:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x04206c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::umullb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMULLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0111 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45007800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::umullt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UMULLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0111 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(!zd.IsLaneSizeB() && !zd.IsLaneSizeQ());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == zn.GetLaneSizeInBytes() * 2);

  Emit(0x45007c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::uqadd(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UQADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1001 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44198000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqrshl(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UQRSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1011 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440b8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqrshlr(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn,
                        const ZRegister& zm) {
  // UQRSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1111 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440f8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      int shift) {
  // UQSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 0111 100. .... .... ....
  //  tszh<23:22> | opc<19:18> | L<17> | U<16> | Pg<12:10> | tszl<9:8> |
  //  imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, 0x04078000);
}

void Assembler::uqshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UQSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1001 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44098000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqshlr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UQSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 1101 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x440d8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqsub(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // UQSUB <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1011 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441b8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqsubr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // UQSUBR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1111 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441f8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::uqxtnb(const ZRegister& zd, const ZRegister& zn) {
  // UQXTNB <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0100 10.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45204800);
}

void Assembler::uqxtnt(const ZRegister& zd, const ZRegister& zn) {
  // UQXTNT <Zd>.<T>, <Zn>.<Tb>
  //  0100 0101 0.1. .000 0100 11.. .... ....
  //  tszh<22> | tszl<20:19> | opc<12:11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() || zd.IsLaneSizeH() || zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() / 2));

  // XTN instructions look like immediate shifts with zero shift distance.
  Instr size = EncodeSVEShiftLeftImmediate(0, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, size, 0x45204c00);
}

void Assembler::urecpe(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn) {
  // URECPE <Zd>.S, <Pg>/M, <Zn>.S
  //  0100 0100 ..00 0000 101. .... .... ....
  //  size<23:22> | Q<19> | opc<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() && zn.IsLaneSizeS());

  Emit(0x4400a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::urhadd(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // URHADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 0101 100. .... .... ....
  //  size<23:22> | R<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44158000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::urshl(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  // URSHL <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 0011 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44038000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::urshlr(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // URSHLR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..00 0111 100. .... .... ....
  //  size<23:22> | Q<19> | R<18> | N<17> | U<16> | Pg<12:10> | Zm<9:5> |
  //  Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x44078000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::urshr(const ZRegister& zd,
                      const PRegisterM& pg,
                      const ZRegister& zn,
                      int shift) {
  // URSHR <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, #<const>
  //  0000 0100 ..00 1101 100. .... .... ....
  //  tszh<23:22> | opc<19:18> | L<17> | U<16> | Pg<12:10> | tszl<9:8> |
  //  imm3<7:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediatePred(zd, pg, encoded_imm, 0x040d8000);
}

void Assembler::ursqrte(const ZRegister& zd,
                        const PRegisterM& pg,
                        const ZRegister& zn) {
  // URSQRTE <Zd>.S, <Pg>/M, <Zn>.S
  //  0100 0100 ..00 0001 101. .... .... ....
  //  size<23:22> | Q<19> | opc<17:16> | Pg<12:10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.IsLaneSizeS() && zn.IsLaneSizeS());

  Emit(0x4401a000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zn));
}

void Assembler::ursra(const ZRegister& zda, const ZRegister& zn, int shift) {
  // URSRA <Zda>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1110 11.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | R<11> | U<10> | Zn<9:5> |
  //  Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zda.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zda, zn, encoded_imm, 0x4500ec00);
}

void Assembler::ushllb(const ZRegister& zd, const ZRegister& zn, int shift) {
  // USHLLB <Zd>.<T>, <Zn>.<Tb>, #<const>
  //  0100 0101 0.0. .... 1010 10.. .... ....
  //  tszh<22> | tszl<20:19> | imm3<18:16> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zn.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500a800);
}

void Assembler::ushllt(const ZRegister& zd, const ZRegister& zn, int shift) {
  // USHLLT <Zd>.<T>, <Zn>.<Tb>, #<const>
  //  0100 0101 0.0. .... 1010 11.. .... ....
  //  tszh<22> | tszl<20:19> | imm3<18:16> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(!zd.IsLaneSizeB());
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));

  Instr encoded_imm =
      EncodeSVEShiftLeftImmediate(shift, zn.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zn, encoded_imm, 0x4500ac00);
}

void Assembler::usqadd(const ZRegister& zd,
                       const PRegisterM& pg,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // USQADD <Zdn>.<T>, <Pg>/M, <Zdn>.<T>, <Zm>.<T>
  //  0100 0100 ..01 1101 100. .... .... ....
  //  size<23:22> | op<18> | S<17> | U<16> | Pg<12:10> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zn, zm));

  Emit(0x441d8000 | SVESize(zd) | Rd(zd) | PgLow8(pg) | Rn(zm));
}

void Assembler::usra(const ZRegister& zda, const ZRegister& zn, int shift) {
  // USRA <Zda>.<T>, <Zn>.<T>, #<const>
  //  0100 0101 ..0. .... 1110 01.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | R<11> | U<10> | Zn<9:5> |
  //  Zda<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zda, zn));
  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zda.GetLaneSizeInBits());

  SVEBitwiseShiftImmediate(zda, zn, encoded_imm, 0x4500e400);
}

void Assembler::usublb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // USUBLB <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0001 10.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45001800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::usublt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // USUBLT <Zd>.<T>, <Zn>.<Tb>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0001 11.. .... ....
  //  size<23:22> | Zm<20:16> | op<13> | S<12> | U<11> | T<10> | Zn<9:5> |
  //  Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zn, zm));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zn.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45001c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::usubwb(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // USUBWB <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0101 10.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45005800 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::usubwt(const ZRegister& zd,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  // USUBWT <Zd>.<T>, <Zn>.<T>, <Zm>.<Tb>
  //  0100 0101 ..0. .... 0101 11.. .... ....
  //  size<23:22> | Zm<20:16> | S<12> | U<11> | T<10> | Zn<9:5> | Zd<4:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameLaneSize(zd, zn));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() == (zm.GetLaneSizeInBytes() * 2));
  VIXL_ASSERT(zd.GetLaneSizeInBytes() != kBRegSizeInBytes);

  Emit(0x45005c00 | SVESize(zd) | Rd(zd) | Rn(zn) | Rm(zm));
}

void Assembler::whilege(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILEGE <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 00.. ...0 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> | lt<10> | Rn<9:5> | eq<4> |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(0x25200000 | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilegt(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILEGT <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 00.. ...1 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> | lt<10> | Rn<9:5> | eq<4> |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(0x25200010 | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilehi(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILEHI <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 10.. ...1 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> | lt<10> | Rn<9:5> | eq<4> |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(0x25200810 | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilehs(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILEHS <Pd>.<T>, <R><n>, <R><m>
  //  0010 0101 ..1. .... 000. 10.. ...0 ....
  //  size<23:22> | Rm<20:16> | sf<12> | U<11> | lt<10> | Rn<9:5> | eq<4> |
  //  Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(AreSameSizeAndType(rn, rm));
  const Instr sf = rn.Is64Bits() ? 0x00001000 : 0x00000000;

  Emit(0x25200800 | SVESize(pd) | sf | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilerw(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILERW <Pd>.<T>, <Xn>, <Xm>
  //  0010 0101 ..1. .... 0011 00.. ...1 ....
  //  size<23:22> | Rm<20:16> | Rn<9:5> | rw<4> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(rn.IsX() && rm.IsX());

  Emit(0x25203010 | SVESize(pd) | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::whilewr(const PRegisterWithLaneSize& pd,
                        const Register& rn,
                        const Register& rm) {
  // WHILEWR <Pd>.<T>, <Xn>, <Xm>
  //  0010 0101 ..1. .... 0011 00.. ...0 ....
  //  size<23:22> | Rm<20:16> | Rn<9:5> | rw<4> | Pd<3:0>

  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(rn.IsX() && rm.IsX());

  Emit(0x25203000 | SVESize(pd) | Pd(pd) | Rn(rn) | Rm(rm));
}

void Assembler::xar(const ZRegister& zd,
                    const ZRegister& zn,
                    const ZRegister& zm,
                    int shift) {
  // XAR <Zdn>.<T>, <Zdn>.<T>, <Zm>.<T>, #<const>
  //  0000 0100 ..1. .... 0011 01.. .... ....
  //  tszh<23:22> | tszl<20:19> | imm3<18:16> | Zm<9:5> | Zdn<4:0>

  USE(zn);
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE2));
  VIXL_ASSERT(zd.Is(zn));
  VIXL_ASSERT(AreSameLaneSize(zd, zm));

  Instr encoded_imm =
      EncodeSVEShiftRightImmediate(shift, zd.GetLaneSizeInBits());
  SVEBitwiseShiftImmediate(zd, zm, encoded_imm, 0x04203400);
}

void Assembler::fmmla(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT((CPUHas(CPUFeatures::kSVEF32MM) && zda.IsLaneSizeS()) ||
              (CPUHas(CPUFeatures::kSVEF64MM) && zda.IsLaneSizeD()));
  VIXL_ASSERT(AreSameLaneSize(zda, zn, zm));

  Emit(0x6420e400 | SVESize(zda) | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::smmla(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());

  Emit(0x45009800 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::usmmla(const ZRegister& zda,
                       const ZRegister& zn,
                       const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());

  Emit(0x45809800 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::ummla(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());

  Emit(0x45c09800 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::usdot(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());

  Emit(0x44807800 | Rd(zda) | Rn(zn) | Rm(zm));
}

void Assembler::usdot(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());
  VIXL_ASSERT(zm.GetCode() <= 7);
  VIXL_ASSERT(IsUint2(index));

  Emit(0x44a01800 | Rx<18, 16>(zm) | (index << 19) | Rd(zda) | Rn(zn));
}

void Assembler::sudot(const ZRegister& zda,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSVEI8MM));
  VIXL_ASSERT(zda.IsLaneSizeS());
  VIXL_ASSERT(zn.IsLaneSizeB() && zm.IsLaneSizeB());
  VIXL_ASSERT(zm.GetCode() <= 7);
  VIXL_ASSERT(IsUint2(index));

  Emit(0x44a01c00 | Rx<18, 16>(zm) | (index << 19) | Rd(zda) | Rn(zn));
}

}  // namespace aarch64
}  // namespace vixl
