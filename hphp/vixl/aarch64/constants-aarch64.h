// Copyright 2015, VIXL authors
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

#ifndef VIXL_AARCH64_CONSTANTS_AARCH64_H_
#define VIXL_AARCH64_CONSTANTS_AARCH64_H_

#include "../globals-vixl.h"

namespace vixl {
namespace aarch64 {

const unsigned kNumberOfRegisters = 32;
const unsigned kNumberOfVRegisters = 32;
const unsigned kNumberOfZRegisters = kNumberOfVRegisters;
const unsigned kNumberOfPRegisters = 16;
// Callee saved registers are x21-x30(lr).
const int kNumberOfCalleeSavedRegisters = 10;
const int kFirstCalleeSavedRegisterIndex = 21;
// Callee saved FP registers are d8-d15. Note that the high parts of v8-v15 are
// still caller-saved.
const int kNumberOfCalleeSavedFPRegisters = 8;
const int kFirstCalleeSavedFPRegisterIndex = 8;
// All predicated instructions accept at least p0-p7 as the governing predicate.
const unsigned kNumberOfGoverningPRegisters = 8;

// clang-format off
#define AARCH64_P_REGISTER_CODE_LIST(R)                                        \
  R(0)  R(1)  R(2)  R(3)  R(4)  R(5)  R(6)  R(7)                               \
  R(8)  R(9)  R(10) R(11) R(12) R(13) R(14) R(15)

#define AARCH64_REGISTER_CODE_LIST(R)                                          \
  R(0)  R(1)  R(2)  R(3)  R(4)  R(5)  R(6)  R(7)                               \
  R(8)  R(9)  R(10) R(11) R(12) R(13) R(14) R(15)                              \
  R(16) R(17) R(18) R(19) R(20) R(21) R(22) R(23)                              \
  R(24) R(25) R(26) R(27) R(28) R(29) R(30) R(31)

// SVE loads and stores use "w" instead of "s" for word-sized accesses, so the
// mapping from the load/store variant to constants like k*RegSize is irregular.
#define VIXL_SVE_LOAD_STORE_VARIANT_LIST(V) \
  V(b, B)                            \
  V(h, H)                            \
  V(w, S)                            \
  V(d, D)

// Sign-extending loads don't have double-word variants.
#define VIXL_SVE_LOAD_STORE_SIGNED_VARIANT_LIST(V) \
  V(b, B)                            \
  V(h, H)                            \
  V(w, S)

#define INSTRUCTION_FIELDS_LIST(V_)                                          \
/* Register fields */                                                        \
V_(Rd, 4, 0, ExtractBits)         /* Destination register.                */ \
V_(Rn, 9, 5, ExtractBits)         /* First source register.               */ \
V_(Rm, 20, 16, ExtractBits)       /* Second source register.              */ \
V_(RmLow16, 19, 16, ExtractBits)  /* Second source register (code 0-15).  */ \
V_(Ra, 14, 10, ExtractBits)       /* Third source register.               */ \
V_(Rt, 4, 0, ExtractBits)         /* Load/store register.                 */ \
V_(Rt2, 14, 10, ExtractBits)      /* Load/store second register.          */ \
V_(Rs, 20, 16, ExtractBits)       /* Exclusive access status.             */ \
V_(Pt, 3, 0, ExtractBits)         /* Load/store register (p0-p7).         */ \
V_(Pd, 3, 0, ExtractBits)         /* SVE destination predicate register.  */ \
V_(Pn, 8, 5, ExtractBits)         /* SVE first source predicate register. */ \
V_(Pm, 19, 16, ExtractBits)       /* SVE second source predicate register.*/ \
V_(PgLow8, 12, 10, ExtractBits)   /* Governing predicate (p0-p7).         */ \
                                                                             \
/* Common bits */                                                            \
V_(SixtyFourBits, 31, 31, ExtractBits)                                       \
V_(FlagsUpdate, 29, 29, ExtractBits)                                         \
                                                                             \
/* PC relative addressing */                                                 \
V_(ImmPCRelHi, 23, 5, ExtractSignedBits)                                     \
V_(ImmPCRelLo, 30, 29, ExtractBits)                                          \
                                                                             \
/* Add/subtract/logical shift register */                                    \
V_(ShiftDP, 23, 22, ExtractBits)                                             \
V_(ImmDPShift, 15, 10, ExtractBits)                                          \
                                                                             \
/* Add/subtract immediate */                                                 \
V_(ImmAddSub, 21, 10, ExtractBits)                                           \
V_(ImmAddSubShift, 22, 22, ExtractBits)                                      \
                                                                             \
/* Add/substract extend */                                                   \
V_(ImmExtendShift, 12, 10, ExtractBits)                                      \
V_(ExtendMode, 15, 13, ExtractBits)                                          \
                                                                             \
/* Move wide */                                                              \
V_(ImmMoveWide, 20, 5, ExtractBits)                                          \
V_(ShiftMoveWide, 22, 21, ExtractBits)                                       \
                                                                             \
/* Logical immediate, bitfield and extract */                                \
V_(BitN, 22, 22, ExtractBits)                                                \
V_(ImmRotate, 21, 16, ExtractBits)                                           \
V_(ImmSetBits, 15, 10, ExtractBits)                                          \
V_(ImmR, 21, 16, ExtractBits)                                                \
V_(ImmS, 15, 10, ExtractBits)                                                \
                                                                             \
/* Test and branch immediate */                                              \
V_(ImmTestBranch, 18, 5, ExtractSignedBits)                                  \
V_(ImmTestBranchBit40, 23, 19, ExtractBits)                                  \
V_(ImmTestBranchBit5, 31, 31, ExtractBits)                                   \
                                                                             \
/* Conditionals */                                                           \
V_(Condition, 15, 12, ExtractBits)                                           \
V_(ConditionBranch, 3, 0, ExtractBits)                                       \
V_(Nzcv, 3, 0, ExtractBits)                                                  \
V_(ImmCondCmp, 20, 16, ExtractBits)                                          \
V_(ImmCondBranch, 23, 5, ExtractSignedBits)                                  \
                                                                             \
/* Floating point */                                                         \
V_(FPType, 23, 22, ExtractBits)                                              \
V_(ImmFP, 20, 13, ExtractBits)                                               \
V_(FPScale, 15, 10, ExtractBits)                                             \
                                                                             \
/* Load Store */                                                             \
V_(ImmLS, 20, 12, ExtractSignedBits)                                         \
V_(ImmLSUnsigned, 21, 10, ExtractBits)                                       \
V_(ImmLSPair, 21, 15, ExtractSignedBits)                                     \
V_(ImmShiftLS, 12, 12, ExtractBits)                                          \
V_(LSOpc, 23, 22, ExtractBits)                                               \
V_(LSVector, 26, 26, ExtractBits)                                            \
V_(LSSize, 31, 30, ExtractBits)                                              \
V_(ImmPrefetchOperation, 4, 0, ExtractBits)                                  \
V_(PrefetchHint, 4, 3, ExtractBits)                                          \
V_(PrefetchTarget, 2, 1, ExtractBits)                                        \
V_(PrefetchStream, 0, 0, ExtractBits)                                        \
V_(ImmLSPACHi, 22, 22, ExtractSignedBits)                                    \
V_(ImmLSPACLo, 20, 12, ExtractBits)                                          \
                                                                             \
/* Other immediates */                                                       \
V_(ImmUncondBranch, 25, 0, ExtractSignedBits)                                \
V_(ImmCmpBranch, 23, 5, ExtractSignedBits)                                   \
V_(ImmLLiteral, 23, 5, ExtractSignedBits)                                    \
V_(ImmException, 20, 5, ExtractBits)                                         \
V_(ImmHint, 11, 5, ExtractBits)                                              \
V_(ImmBarrierDomain, 11, 10, ExtractBits)                                    \
V_(ImmBarrierType, 9, 8, ExtractBits)                                        \
V_(ImmUdf, 15, 0, ExtractBits)                                               \
                                                                             \
/* System (MRS, MSR, SYS) */                                                 \
V_(ImmSystemRegister, 20, 5, ExtractBits)                                    \
V_(SysO0, 19, 19, ExtractBits)                                               \
V_(SysOp, 18, 5, ExtractBits)                                                \
V_(SysOp0, 20, 19, ExtractBits)                                              \
V_(SysOp1, 18, 16, ExtractBits)                                              \
V_(SysOp2, 7, 5, ExtractBits)                                                \
V_(CRn, 15, 12, ExtractBits)                                                 \
V_(CRm, 11, 8, ExtractBits)                                                  \
V_(ImmRMIFRotation, 20, 15, ExtractBits)                                     \
                                                                             \
/* Load-/store-exclusive */                                                  \
V_(LdStXLoad, 22, 22, ExtractBits)                                           \
V_(LdStXNotExclusive, 23, 23, ExtractBits)                                   \
V_(LdStXAcquireRelease, 15, 15, ExtractBits)                                 \
V_(LdStXSizeLog2, 31, 30, ExtractBits)                                       \
V_(LdStXPair, 21, 21, ExtractBits)                                           \
                                                                             \
/* NEON generic fields */                                                    \
V_(NEONQ, 30, 30, ExtractBits)                                               \
V_(NEONSize, 23, 22, ExtractBits)                                            \
V_(NEONLSSize, 11, 10, ExtractBits)                                          \
V_(NEONS, 12, 12, ExtractBits)                                               \
V_(NEONL, 21, 21, ExtractBits)                                               \
V_(NEONM, 20, 20, ExtractBits)                                               \
V_(NEONH, 11, 11, ExtractBits)                                               \
V_(ImmNEONExt, 14, 11, ExtractBits)                                          \
V_(ImmNEON5, 20, 16, ExtractBits)                                            \
V_(ImmNEON4, 14, 11, ExtractBits)                                            \
                                                                             \
/* NEON extra fields */                                                      \
V_(ImmRotFcadd, 12, 12, ExtractBits)                                         \
V_(ImmRotFcmlaVec, 12, 11, ExtractBits)                                      \
V_(ImmRotFcmlaSca, 14, 13, ExtractBits)                                      \
                                                                             \
/* NEON Modified Immediate fields */                                         \
V_(ImmNEONabc, 18, 16, ExtractBits)                                          \
V_(ImmNEONdefgh, 9, 5, ExtractBits)                                          \
V_(NEONModImmOp, 29, 29, ExtractBits)                                        \
V_(NEONCmode, 15, 12, ExtractBits)                                           \
                                                                             \
/* NEON Shift Immediate fields */                                            \
V_(ImmNEONImmhImmb, 22, 16, ExtractBits)                                     \
V_(ImmNEONImmh, 22, 19, ExtractBits)                                         \
V_(ImmNEONImmb, 18, 16, ExtractBits)                                         \
                                                                             \
/* SVE generic fields */                                                     \
V_(SVESize, 23, 22, ExtractBits)                                             \
V_(ImmSVEVLScale, 10, 5, ExtractSignedBits)                                  \
V_(ImmSVEIntWideSigned, 12, 5, ExtractSignedBits)                            \
V_(ImmSVEIntWideUnsigned, 12, 5, ExtractBits)                                \
V_(ImmSVEPredicateConstraint, 9, 5, ExtractBits)                             \
                                                                             \
/* SVE Bitwise Immediate bitfield */                                         \
V_(SVEBitN, 17, 17, ExtractBits)                                             \
V_(SVEImmRotate, 16, 11, ExtractBits)                                        \
V_(SVEImmSetBits, 10, 5, ExtractBits)                                        \
                                                                             \
V_(SVEImmPrefetchOperation, 3, 0, ExtractBits)                               \
V_(SVEPrefetchHint, 3, 3, ExtractBits)

// clang-format on

#define SYSTEM_REGISTER_FIELDS_LIST(V_, M_) \
  /* NZCV */                                \
  V_(Flags, 31, 28, ExtractBits)            \
  V_(N, 31, 31, ExtractBits)                \
  V_(Z, 30, 30, ExtractBits)                \
  V_(C, 29, 29, ExtractBits)                \
  V_(V, 28, 28, ExtractBits)                \
  M_(NZCV, Flags_mask)                      \
  /* FPCR */                                \
  V_(AHP, 26, 26, ExtractBits)              \
  V_(DN, 25, 25, ExtractBits)               \
  V_(FZ, 24, 24, ExtractBits)               \
  V_(RMode, 23, 22, ExtractBits)            \
  M_(FPCR, AHP_mask | DN_mask | FZ_mask | RMode_mask)

// Fields offsets.
#define DECLARE_FIELDS_OFFSETS(Name, HighBit, LowBit, X) \
  const int Name##_offset = LowBit;                      \
  const int Name##_width = HighBit - LowBit + 1;         \
  const uint32_t Name##_mask = ((1 << Name##_width) - 1) << LowBit;
#define NOTHING(A, B)
INSTRUCTION_FIELDS_LIST(DECLARE_FIELDS_OFFSETS)
SYSTEM_REGISTER_FIELDS_LIST(DECLARE_FIELDS_OFFSETS, NOTHING)
#undef NOTHING
#undef DECLARE_FIELDS_BITS

// ImmPCRel is a compound field (not present in INSTRUCTION_FIELDS_LIST), formed
// from ImmPCRelLo and ImmPCRelHi.
const int ImmPCRel_mask = ImmPCRelLo_mask | ImmPCRelHi_mask;

// Disable `clang-format` for the `enum`s below. We care about the manual
// formatting that `clang-format` would destroy.
// clang-format off

// Condition codes.
enum Condition {
  eq = 0,   // Z set            Equal.
  ne = 1,   // Z clear          Not equal.
  cs = 2,   // C set            Carry set.
  cc = 3,   // C clear          Carry clear.
  mi = 4,   // N set            Negative.
  pl = 5,   // N clear          Positive or zero.
  vs = 6,   // V set            Overflow.
  vc = 7,   // V clear          No overflow.
  hi = 8,   // C set, Z clear   Unsigned higher.
  ls = 9,   // C clear or Z set Unsigned lower or same.
  ge = 10,  // N == V           Greater or equal.
  lt = 11,  // N != V           Less than.
  gt = 12,  // Z clear, N == V  Greater than.
  le = 13,  // Z set or N != V  Less then or equal
  al = 14,  //                  Always.
  nv = 15,  // Behaves as always/al.

  // Aliases.
  hs = cs,  // C set            Unsigned higher or same.
  lo = cc,  // C clear          Unsigned lower.

  // Floating-point additional condition code.
  uo,       // Unordered comparison.

  // SVE predicate condition aliases.
  sve_none  = eq,  // No active elements were true.
  sve_any   = ne,  // An active element was true.
  sve_nlast = cs,  // The last element was not true.
  sve_last  = cc,  // The last element was true.
  sve_first = mi,  // The first element was true.
  sve_nfrst = pl,  // The first element was not true.
  sve_pmore = hi,  // An active element was true but not the last element.
  sve_plast = ls,  // The last active element was true or no active elements were true.
  sve_tcont = ge,  // CTERM termination condition not deleted.
  sve_tstop = lt   // CTERM termination condition deleted.
};

inline Condition InvertCondition(Condition cond) {
  // Conditions al and nv behave identically, as "always true". They can't be
  // inverted, because there is no "always false" condition.
  VIXL_ASSERT((cond != al) && (cond != nv));
  return static_cast<Condition>(cond ^ 1);
}

enum FPTrapFlags {
  EnableTrap   = 1,
  DisableTrap = 0
};

enum FlagsUpdate {
  SetFlags   = 1,
  LeaveFlags = 0
};

enum StatusFlags {
  NoFlag    = 0,

  // Derive the flag combinations from the system register bit descriptions.
  NFlag     = N_mask,
  ZFlag     = Z_mask,
  CFlag     = C_mask,
  VFlag     = V_mask,
  NZFlag    = NFlag | ZFlag,
  NCFlag    = NFlag | CFlag,
  NVFlag    = NFlag | VFlag,
  ZCFlag    = ZFlag | CFlag,
  ZVFlag    = ZFlag | VFlag,
  CVFlag    = CFlag | VFlag,
  NZCFlag   = NFlag | ZFlag | CFlag,
  NZVFlag   = NFlag | ZFlag | VFlag,
  NCVFlag   = NFlag | CFlag | VFlag,
  ZCVFlag   = ZFlag | CFlag | VFlag,
  NZCVFlag  = NFlag | ZFlag | CFlag | VFlag,

  // Floating-point comparison results.
  FPEqualFlag       = ZCFlag,
  FPLessThanFlag    = NFlag,
  FPGreaterThanFlag = CFlag,
  FPUnorderedFlag   = CVFlag,

  // SVE condition flags.
  SVEFirstFlag   = NFlag,
  SVENoneFlag    = ZFlag,
  SVENotLastFlag = CFlag
};

enum Shift {
  NO_SHIFT = -1,
  LSL = 0x0,
  LSR = 0x1,
  ASR = 0x2,
  ROR = 0x3,
  MSL = 0x4
};

enum Extend {
  NO_EXTEND = -1,
  UXTB      = 0,
  UXTH      = 1,
  UXTW      = 2,
  UXTX      = 3,
  SXTB      = 4,
  SXTH      = 5,
  SXTW      = 6,
  SXTX      = 7
};

enum SVEOffsetModifier {
  NO_SVE_OFFSET_MODIFIER,
  // Multiply (each element of) the offset by either the vector or predicate
  // length, according to the context.
  SVE_MUL_VL,
  // Shift or extend modifiers (as in `Shift` or `Extend`).
  SVE_LSL,
  SVE_UXTW,
  SVE_SXTW
};

enum SystemHint {
  NOP    = 0,
  YIELD  = 1,
  WFE    = 2,
  WFI    = 3,
  SEV    = 4,
  SEVL   = 5,
  ESB    = 16,
  CSDB   = 20,
  BTI    = 32,
  BTI_c  = 34,
  BTI_j  = 36,
  BTI_jc = 38,
  CHKFEAT = 40
};

enum BranchTargetIdentifier {
  EmitBTI_none = NOP,
  EmitBTI = BTI,
  EmitBTI_c = BTI_c,
  EmitBTI_j = BTI_j,
  EmitBTI_jc = BTI_jc,

  // These correspond to the values of the CRm:op2 fields in the equivalent HINT
  // instruction.
  EmitPACIASP = 25,
  EmitPACIBSP = 27
};

enum BarrierDomain {
  OuterShareable = 0,
  NonShareable   = 1,
  InnerShareable = 2,
  FullSystem     = 3
};

enum BarrierType {
  BarrierOther  = 0,
  BarrierReads  = 1,
  BarrierWrites = 2,
  BarrierAll    = 3
};

enum PrefetchOperation {
  PLDL1KEEP = 0x00,
  PLDL1STRM = 0x01,
  PLDL2KEEP = 0x02,
  PLDL2STRM = 0x03,
  PLDL3KEEP = 0x04,
  PLDL3STRM = 0x05,

  PrfUnallocated06 = 0x06,
  PrfUnallocated07 = 0x07,

  PLIL1KEEP = 0x08,
  PLIL1STRM = 0x09,
  PLIL2KEEP = 0x0a,
  PLIL2STRM = 0x0b,
  PLIL3KEEP = 0x0c,
  PLIL3STRM = 0x0d,

  PrfUnallocated0e = 0x0e,
  PrfUnallocated0f = 0x0f,

  PSTL1KEEP = 0x10,
  PSTL1STRM = 0x11,
  PSTL2KEEP = 0x12,
  PSTL2STRM = 0x13,
  PSTL3KEEP = 0x14,
  PSTL3STRM = 0x15,

  PrfUnallocated16 = 0x16,
  PrfUnallocated17 = 0x17,
  PrfUnallocated18 = 0x18,
  PrfUnallocated19 = 0x19,
  PrfUnallocated1a = 0x1a,
  PrfUnallocated1b = 0x1b,
  PrfUnallocated1c = 0x1c,
  PrfUnallocated1d = 0x1d,
  PrfUnallocated1e = 0x1e,
  PrfUnallocated1f = 0x1f,
};

constexpr bool IsNamedPrefetchOperation(int op) {
  return ((op >= PLDL1KEEP) && (op <= PLDL3STRM)) ||
      ((op >= PLIL1KEEP) && (op <= PLIL3STRM)) ||
      ((op >= PSTL1KEEP) && (op <= PSTL3STRM));
}

enum BType {
  // Set when executing any instruction on a guarded page, except those cases
  // listed below.
  DefaultBType = 0,

  // Set when an indirect branch is taken from an unguarded page to a guarded
  // page, or from a guarded page to ip0 or ip1 (x16 or x17), eg "br ip0".
  BranchFromUnguardedOrToIP = 1,

  // Set when an indirect branch and link (call) is taken, eg. "blr x0".
  BranchAndLink = 2,

  // Set when an indirect branch is taken from a guarded page to a register
  // that is not ip0 or ip1 (x16 or x17), eg, "br x0".
  BranchFromGuardedNotToIP = 3
};

template<int op0, int op1, int crn, int crm, int op2>
class SystemRegisterEncoder {
 public:
  static const uint32_t value =
      ((op0 << SysO0_offset) |
       (op1 << SysOp1_offset) |
       (crn << CRn_offset) |
       (crm << CRm_offset) |
       (op2 << SysOp2_offset)) >> ImmSystemRegister_offset;
};

// System/special register names.
// This information is not encoded as one field but as the concatenation of
// multiple fields (Op0, Op1, Crn, Crm, Op2).
enum SystemRegister {
  NZCV = SystemRegisterEncoder<3, 3, 4, 2, 0>::value,
  FPCR = SystemRegisterEncoder<3, 3, 4, 4, 0>::value,
  RNDR = SystemRegisterEncoder<3, 3, 2, 4, 0>::value,    // Random number.
  RNDRRS = SystemRegisterEncoder<3, 3, 2, 4, 1>::value,  // Reseeded random number.
  DCZID_EL0 = SystemRegisterEncoder<3, 3, 0, 0, 7>::value
};

template<int op1, int crn, int crm, int op2>
class CacheOpEncoder {
 public:
  static const uint32_t value =
      ((op1 << SysOp1_offset) |
       (crn << CRn_offset) |
       (crm << CRm_offset) |
       (op2 << SysOp2_offset)) >> SysOp_offset;
};

enum InstructionCacheOp {
  IVAU = CacheOpEncoder<3, 7, 5, 1>::value
};

enum DataCacheOp {
  CVAC = CacheOpEncoder<3, 7, 10, 1>::value,
  CVAU = CacheOpEncoder<3, 7, 11, 1>::value,
  CVAP = CacheOpEncoder<3, 7, 12, 1>::value,
  CVADP = CacheOpEncoder<3, 7, 13, 1>::value,
  CIVAC = CacheOpEncoder<3, 7, 14, 1>::value,
  ZVA = CacheOpEncoder<3, 7, 4, 1>::value,
  GVA = CacheOpEncoder<3, 7, 4, 3>::value,
  GZVA = CacheOpEncoder<3, 7, 4, 4>::value,
  CGVAC = CacheOpEncoder<3, 7, 10, 3>::value,
  CGDVAC = CacheOpEncoder<3, 7, 10, 5>::value,
  CGVAP = CacheOpEncoder<3, 7, 12, 3>::value,
  CGDVAP = CacheOpEncoder<3, 7, 12, 5>::value,
  CIGVAC = CacheOpEncoder<3, 7, 14, 3>::value,
  CIGDVAC = CacheOpEncoder<3, 7, 14, 5>::value
};

enum GCSOp {
  GCSPUSHM = CacheOpEncoder<3, 7, 7, 0>::value,
  GCSPOPM = CacheOpEncoder<3, 7, 7, 1>::value,
  GCSSS1 = CacheOpEncoder<3, 7, 7, 2>::value,
  GCSSS2 = CacheOpEncoder<3, 7, 7, 3>::value
};

// Some SVE instructions support a predicate constraint pattern. This is
// interpreted as a VL-dependent value, and is typically used to initialise
// predicates, or to otherwise limit the number of processed elements.
enum SVEPredicateConstraint {
  // Select 2^N elements, for the largest possible N.
  SVE_POW2 = 0x0,
  // Each VL<N> selects exactly N elements if possible, or zero if N is greater
  // than the number of elements. Note that the encoding values for VL<N> are
  // not linearly related to N.
  SVE_VL1 = 0x1,
  SVE_VL2 = 0x2,
  SVE_VL3 = 0x3,
  SVE_VL4 = 0x4,
  SVE_VL5 = 0x5,
  SVE_VL6 = 0x6,
  SVE_VL7 = 0x7,
  SVE_VL8 = 0x8,
  SVE_VL16 = 0x9,
  SVE_VL32 = 0xa,
  SVE_VL64 = 0xb,
  SVE_VL128 = 0xc,
  SVE_VL256 = 0xd,
  // Each MUL<N> selects the largest multiple of N elements that the vector
  // length supports. Note that for D-sized lanes, this can be zero.
  SVE_MUL4 = 0x1d,
  SVE_MUL3 = 0x1e,
  // Select all elements.
  SVE_ALL = 0x1f
};

// Instruction enumerations.
//
// These are the masks that define a class of instructions, and the list of
// instructions within each class. Each enumeration has a Fixed, FMask and
// Mask value.
//
// Fixed: The fixed bits in this instruction class.
// FMask: The mask used to extract the fixed bits in the class.
// Mask:  The mask used to identify the instructions within a class.
//
// The enumerations can be used like this:
//
// VIXL_ASSERT(instr->Mask(PCRelAddressingFMask) == PCRelAddressingFixed);
// switch(instr->Mask(PCRelAddressingMask)) {
//   case ADR:  Format("adr 'Xd, 'AddrPCRelByte"); break;
//   case ADRP: Format("adrp 'Xd, 'AddrPCRelPage"); break;
//   default:   printf("Unknown instruction\n");
// }


// Generic fields.
enum GenericInstrField {
  SixtyFourBits        = 0x80000000,
  ThirtyTwoBits        = 0x00000000,

  FPTypeMask           = 0x00C00000,
  FP16                 = 0x00C00000,
  FP32                 = 0x00000000,
  FP64                 = 0x00400000
};

enum NEONFormatField {
  NEONFormatFieldMask   = 0x40C00000,
  NEON_Q                = 0x40000000,
  NEON_8B               = 0x00000000,
  NEON_16B              = NEON_8B | NEON_Q,
  NEON_4H               = 0x00400000,
  NEON_8H               = NEON_4H | NEON_Q,
  NEON_2S               = 0x00800000,
  NEON_4S               = NEON_2S | NEON_Q,
  NEON_1D               = 0x00C00000,
  NEON_2D               = 0x00C00000 | NEON_Q
};

enum NEONFPFormatField {
  NEONFPFormatFieldMask = 0x40400000,
  NEON_FP_4H            = FP16,
  NEON_FP_2S            = FP32,
  NEON_FP_8H            = FP16 | NEON_Q,
  NEON_FP_4S            = FP32 | NEON_Q,
  NEON_FP_2D            = FP64 | NEON_Q
};

enum NEONLSFormatField {
  NEONLSFormatFieldMask = 0x40000C00,
  LS_NEON_8B            = 0x00000000,
  LS_NEON_16B           = LS_NEON_8B | NEON_Q,
  LS_NEON_4H            = 0x00000400,
  LS_NEON_8H            = LS_NEON_4H | NEON_Q,
  LS_NEON_2S            = 0x00000800,
  LS_NEON_4S            = LS_NEON_2S | NEON_Q,
  LS_NEON_1D            = 0x00000C00,
  LS_NEON_2D            = LS_NEON_1D | NEON_Q
};

enum NEONScalarFormatField {
  NEONScalarFormatFieldMask = 0x00C00000,
  NEONScalar                = 0x10000000,
  NEON_B                    = 0x00000000,
  NEON_H                    = 0x00400000,
  NEON_S                    = 0x00800000,
  NEON_D                    = 0x00C00000
};

enum SVESizeField {
  SVESizeFieldMask = 0x00C00000,
  SVE_B            = 0x00000000,
  SVE_H            = 0x00400000,
  SVE_S            = 0x00800000,
  SVE_D            = 0x00C00000
};

// PC relative addressing.
enum PCRelAddressingOp {
  PCRelAddressingFixed = 0x10000000,
  PCRelAddressingFMask = 0x1F000000,
  PCRelAddressingMask  = 0x9F000000,
  ADR                  = PCRelAddressingFixed | 0x00000000,
  ADRP                 = PCRelAddressingFixed | 0x80000000
};

// Add/sub (immediate, shifted and extended.)
const int kSFOffset = 31;
enum AddSubOp {
  AddSubOpMask      = 0x60000000,
  AddSubSetFlagsBit = 0x20000000,
  ADD               = 0x00000000,
  ADDS              = ADD | AddSubSetFlagsBit,
  SUB               = 0x40000000,
  SUBS              = SUB | AddSubSetFlagsBit
};

#define ADD_SUB_OP_LIST(V)  \
  V(ADD),                   \
  V(ADDS),                  \
  V(SUB),                   \
  V(SUBS)

enum AddSubImmediateOp {
  AddSubImmediateFixed = 0x11000000,
  AddSubImmediateFMask = 0x1F800000,
  AddSubImmediateMask  = 0xFF800000,
  #define ADD_SUB_IMMEDIATE(A)           \
  A##_w_imm = AddSubImmediateFixed | A,  \
  A##_x_imm = AddSubImmediateFixed | A | SixtyFourBits
  ADD_SUB_OP_LIST(ADD_SUB_IMMEDIATE)
  #undef ADD_SUB_IMMEDIATE
};

enum AddSubShiftedOp {
  AddSubShiftedFixed   = 0x0B000000,
  AddSubShiftedFMask   = 0x1F200000,
  AddSubShiftedMask    = 0xFF200000,
  #define ADD_SUB_SHIFTED(A)             \
  A##_w_shift = AddSubShiftedFixed | A,  \
  A##_x_shift = AddSubShiftedFixed | A | SixtyFourBits
  ADD_SUB_OP_LIST(ADD_SUB_SHIFTED)
  #undef ADD_SUB_SHIFTED
};

enum AddSubExtendedOp {
  AddSubExtendedFixed  = 0x0B200000,
  AddSubExtendedFMask  = 0x1F200000,
  AddSubExtendedMask   = 0xFFE00000,
  #define ADD_SUB_EXTENDED(A)           \
  A##_w_ext = AddSubExtendedFixed | A,  \
  A##_x_ext = AddSubExtendedFixed | A | SixtyFourBits
  ADD_SUB_OP_LIST(ADD_SUB_EXTENDED)
  #undef ADD_SUB_EXTENDED
};

// Add/sub with carry.
enum AddSubWithCarryOp {
  AddSubWithCarryFixed = 0x1A000000,
  AddSubWithCarryFMask = 0x1FE00000,
  AddSubWithCarryMask  = 0xFFE0FC00,
  ADC_w                = AddSubWithCarryFixed | ADD,
  ADC_x                = AddSubWithCarryFixed | ADD | SixtyFourBits,
  ADC                  = ADC_w,
  ADCS_w               = AddSubWithCarryFixed | ADDS,
  ADCS_x               = AddSubWithCarryFixed | ADDS | SixtyFourBits,
  SBC_w                = AddSubWithCarryFixed | SUB,
  SBC_x                = AddSubWithCarryFixed | SUB | SixtyFourBits,
  SBC                  = SBC_w,
  SBCS_w               = AddSubWithCarryFixed | SUBS,
  SBCS_x               = AddSubWithCarryFixed | SUBS | SixtyFourBits
};

// Rotate right into flags.
enum RotateRightIntoFlagsOp {
  RotateRightIntoFlagsFixed = 0x1A000400,
  RotateRightIntoFlagsFMask = 0x1FE07C00,
  RotateRightIntoFlagsMask  = 0xFFE07C10,
  RMIF                      = RotateRightIntoFlagsFixed | 0xA0000000
};

// Evaluate into flags.
enum EvaluateIntoFlagsOp {
  EvaluateIntoFlagsFixed = 0x1A000800,
  EvaluateIntoFlagsFMask = 0x1FE03C00,
  EvaluateIntoFlagsMask  = 0xFFE07C1F,
  SETF8                  = EvaluateIntoFlagsFixed | 0x2000000D,
  SETF16                 = EvaluateIntoFlagsFixed | 0x2000400D
};


// Logical (immediate and shifted register).
enum LogicalOp {
  LogicalOpMask = 0x60200000,
  NOT   = 0x00200000,
  AND   = 0x00000000,
  BIC   = AND | NOT,
  ORR   = 0x20000000,
  ORN   = ORR | NOT,
  EOR   = 0x40000000,
  EON   = EOR | NOT,
  ANDS  = 0x60000000,
  BICS  = ANDS | NOT
};

// Logical immediate.
enum LogicalImmediateOp {
  LogicalImmediateFixed = 0x12000000,
  LogicalImmediateFMask = 0x1F800000,
  LogicalImmediateMask  = 0xFF800000,
  AND_w_imm   = LogicalImmediateFixed | AND,
  AND_x_imm   = LogicalImmediateFixed | AND | SixtyFourBits,
  ORR_w_imm   = LogicalImmediateFixed | ORR,
  ORR_x_imm   = LogicalImmediateFixed | ORR | SixtyFourBits,
  EOR_w_imm   = LogicalImmediateFixed | EOR,
  EOR_x_imm   = LogicalImmediateFixed | EOR | SixtyFourBits,
  ANDS_w_imm  = LogicalImmediateFixed | ANDS,
  ANDS_x_imm  = LogicalImmediateFixed | ANDS | SixtyFourBits
};

// Logical shifted register.
enum LogicalShiftedOp {
  LogicalShiftedFixed = 0x0A000000,
  LogicalShiftedFMask = 0x1F000000,
  LogicalShiftedMask  = 0xFF200000,
  AND_w               = LogicalShiftedFixed | AND,
  AND_x               = LogicalShiftedFixed | AND | SixtyFourBits,
  AND_shift           = AND_w,
  BIC_w               = LogicalShiftedFixed | BIC,
  BIC_x               = LogicalShiftedFixed | BIC | SixtyFourBits,
  BIC_shift           = BIC_w,
  ORR_w               = LogicalShiftedFixed | ORR,
  ORR_x               = LogicalShiftedFixed | ORR | SixtyFourBits,
  ORR_shift           = ORR_w,
  ORN_w               = LogicalShiftedFixed | ORN,
  ORN_x               = LogicalShiftedFixed | ORN | SixtyFourBits,
  ORN_shift           = ORN_w,
  EOR_w               = LogicalShiftedFixed | EOR,
  EOR_x               = LogicalShiftedFixed | EOR | SixtyFourBits,
  EOR_shift           = EOR_w,
  EON_w               = LogicalShiftedFixed | EON,
  EON_x               = LogicalShiftedFixed | EON | SixtyFourBits,
  EON_shift           = EON_w,
  ANDS_w              = LogicalShiftedFixed | ANDS,
  ANDS_x              = LogicalShiftedFixed | ANDS | SixtyFourBits,
  ANDS_shift          = ANDS_w,
  BICS_w              = LogicalShiftedFixed | BICS,
  BICS_x              = LogicalShiftedFixed | BICS | SixtyFourBits,
  BICS_shift          = BICS_w
};

// Move wide immediate.
enum MoveWideImmediateOp {
  MoveWideImmediateFixed = 0x12800000,
  MoveWideImmediateFMask = 0x1F800000,
  MoveWideImmediateMask  = 0xFF800000,
  MOVN                   = 0x00000000,
  MOVZ                   = 0x40000000,
  MOVK                   = 0x60000000,
  MOVN_w                 = MoveWideImmediateFixed | MOVN,
  MOVN_x                 = MoveWideImmediateFixed | MOVN | SixtyFourBits,
  MOVZ_w                 = MoveWideImmediateFixed | MOVZ,
  MOVZ_x                 = MoveWideImmediateFixed | MOVZ | SixtyFourBits,
  MOVK_w                 = MoveWideImmediateFixed | MOVK,
  MOVK_x                 = MoveWideImmediateFixed | MOVK | SixtyFourBits
};

// Bitfield.
const int kBitfieldNOffset = 22;
enum BitfieldOp {
  BitfieldFixed = 0x13000000,
  BitfieldFMask = 0x1F800000,
  BitfieldMask  = 0xFF800000,
  SBFM_w        = BitfieldFixed | 0x00000000,
  SBFM_x        = BitfieldFixed | 0x80000000,
  SBFM          = SBFM_w,
  BFM_w         = BitfieldFixed | 0x20000000,
  BFM_x         = BitfieldFixed | 0xA0000000,
  BFM           = BFM_w,
  UBFM_w        = BitfieldFixed | 0x40000000,
  UBFM_x        = BitfieldFixed | 0xC0000000,
  UBFM          = UBFM_w
  // Bitfield N field.
};

// Extract.
enum ExtractOp {
  ExtractFixed = 0x13800000,
  ExtractFMask = 0x1F800000,
  ExtractMask  = 0xFFA00000,
  EXTR_w       = ExtractFixed | 0x00000000,
  EXTR_x       = ExtractFixed | 0x80000000,
  EXTR         = EXTR_w
};

// Unconditional branch.
enum UnconditionalBranchOp {
  UnconditionalBranchFixed = 0x14000000,
  UnconditionalBranchFMask = 0x7C000000,
  UnconditionalBranchMask  = 0xFC000000,
  B                        = UnconditionalBranchFixed | 0x00000000,
  BL                       = UnconditionalBranchFixed | 0x80000000
};

// Unconditional branch to register.
enum UnconditionalBranchToRegisterOp {
  UnconditionalBranchToRegisterFixed = 0xD6000000,
  UnconditionalBranchToRegisterFMask = 0xFE000000,
  UnconditionalBranchToRegisterMask  = 0xFFFFFC00,
  BR      = UnconditionalBranchToRegisterFixed | 0x001F0000,
  BLR     = UnconditionalBranchToRegisterFixed | 0x003F0000,
  RET     = UnconditionalBranchToRegisterFixed | 0x005F0000,

  BRAAZ  = UnconditionalBranchToRegisterFixed | 0x001F0800,
  BRABZ  = UnconditionalBranchToRegisterFixed | 0x001F0C00,
  BLRAAZ = UnconditionalBranchToRegisterFixed | 0x003F0800,
  BLRABZ = UnconditionalBranchToRegisterFixed | 0x003F0C00,
  RETAA  = UnconditionalBranchToRegisterFixed | 0x005F0800,
  RETAB  = UnconditionalBranchToRegisterFixed | 0x005F0C00,
  BRAA   = UnconditionalBranchToRegisterFixed | 0x011F0800,
  BRAB   = UnconditionalBranchToRegisterFixed | 0x011F0C00,
  BLRAA  = UnconditionalBranchToRegisterFixed | 0x013F0800,
  BLRAB  = UnconditionalBranchToRegisterFixed | 0x013F0C00
};

// Compare and branch.
enum CompareBranchOp {
  CompareBranchFixed = 0x34000000,
  CompareBranchFMask = 0x7E000000,
  CompareBranchMask  = 0xFF000000,
  CBZ_w              = CompareBranchFixed | 0x00000000,
  CBZ_x              = CompareBranchFixed | 0x80000000,
  CBZ                = CBZ_w,
  CBNZ_w             = CompareBranchFixed | 0x01000000,
  CBNZ_x             = CompareBranchFixed | 0x81000000,
  CBNZ               = CBNZ_w
};

// Test and branch.
enum TestBranchOp {
  TestBranchFixed = 0x36000000,
  TestBranchFMask = 0x7E000000,
  TestBranchMask  = 0x7F000000,
  TBZ             = TestBranchFixed | 0x00000000,
  TBNZ            = TestBranchFixed | 0x01000000
};

// Conditional branch.
enum ConditionalBranchOp {
  ConditionalBranchFixed = 0x54000000,
  ConditionalBranchFMask = 0xFE000000,
  ConditionalBranchMask  = 0xFF000010,
  B_cond                 = ConditionalBranchFixed | 0x00000000
};

// System.
// System instruction encoding is complicated because some instructions use op
// and CR fields to encode parameters. To handle this cleanly, the system
// instructions are split into more than one enum.

enum SystemOp {
  SystemFixed = 0xD5000000,
  SystemFMask = 0xFFC00000
};

enum SystemSysRegOp {
  SystemSysRegFixed = 0xD5100000,
  SystemSysRegFMask = 0xFFD00000,
  SystemSysRegMask  = 0xFFF00000,
  MRS               = SystemSysRegFixed | 0x00200000,
  MSR               = SystemSysRegFixed | 0x00000000
};

enum SystemPStateOp {
  SystemPStateFixed = 0xD5004000,
  SystemPStateFMask = 0xFFF8F000,
  SystemPStateMask  = 0xFFFFF0FF,
  CFINV             = SystemPStateFixed | 0x0000001F,
  XAFLAG            = SystemPStateFixed | 0x0000003F,
  AXFLAG            = SystemPStateFixed | 0x0000005F
};

enum SystemHintOp {
  SystemHintFixed = 0xD503201F,
  SystemHintFMask = 0xFFFFF01F,
  SystemHintMask  = 0xFFFFF01F,
  HINT            = SystemHintFixed | 0x00000000
};

enum SystemSysOp {
  SystemSysFixed  = 0xD5080000,
  SystemSysFMask  = 0xFFF80000,
  SystemSysMask   = 0xFFF80000,
  SYS             = SystemSysFixed | 0x00000000,
  SYSL            = SystemSysFixed | 0x00200000
};

// Exception.
enum ExceptionOp {
  ExceptionFixed = 0xD4000000,
  ExceptionFMask = 0xFF000000,
  ExceptionMask  = 0xFFE0001F,
  HLT            = ExceptionFixed | 0x00400000,
  BRK            = ExceptionFixed | 0x00200000,
  SVC            = ExceptionFixed | 0x00000001,
  HVC            = ExceptionFixed | 0x00000002,
  SMC            = ExceptionFixed | 0x00000003,
  DCPS1          = ExceptionFixed | 0x00A00001,
  DCPS2          = ExceptionFixed | 0x00A00002,
  DCPS3          = ExceptionFixed | 0x00A00003
};

enum MemBarrierOp {
  MemBarrierFixed = 0xD503309F,
  MemBarrierFMask = 0xFFFFF09F,
  MemBarrierMask  = 0xFFFFF0FF,
  DSB             = MemBarrierFixed | 0x00000000,
  DMB             = MemBarrierFixed | 0x00000020,
  ISB             = MemBarrierFixed | 0x00000040
};

enum SystemExclusiveMonitorOp {
  SystemExclusiveMonitorFixed = 0xD503305F,
  SystemExclusiveMonitorFMask = 0xFFFFF0FF,
  SystemExclusiveMonitorMask  = 0xFFFFF0FF,
  CLREX                       = SystemExclusiveMonitorFixed
};

enum SystemPAuthOp {
  SystemPAuthFixed = 0xD503211F,
  SystemPAuthFMask = 0xFFFFFD1F,
  SystemPAuthMask  = 0xFFFFFFFF,
  PACIA1716 = SystemPAuthFixed | 0x00000100,
  PACIB1716 = SystemPAuthFixed | 0x00000140,
  AUTIA1716 = SystemPAuthFixed | 0x00000180,
  AUTIB1716 = SystemPAuthFixed | 0x000001C0,
  PACIAZ    = SystemPAuthFixed | 0x00000300,
  PACIASP   = SystemPAuthFixed | 0x00000320,
  PACIBZ    = SystemPAuthFixed | 0x00000340,
  PACIBSP   = SystemPAuthFixed | 0x00000360,
  AUTIAZ    = SystemPAuthFixed | 0x00000380,
  AUTIASP   = SystemPAuthFixed | 0x000003A0,
  AUTIBZ    = SystemPAuthFixed | 0x000003C0,
  AUTIBSP   = SystemPAuthFixed | 0x000003E0,

  // XPACLRI has the same fixed mask as System Hints and needs to be handled
  // differently.
  XPACLRI   = 0xD50320FF
};

// Any load or store.
enum LoadStoreAnyOp {
  LoadStoreAnyFMask = 0x0a000000,
  LoadStoreAnyFixed = 0x08000000
};

// Any load pair or store pair.
enum LoadStorePairAnyOp {
  LoadStorePairAnyFMask = 0x3a000000,
  LoadStorePairAnyFixed = 0x28000000
};

#define LOAD_STORE_PAIR_OP_LIST(V)  \
  V(STP, w,   0x00000000),          \
  V(LDP, w,   0x00400000),          \
  V(LDPSW, x, 0x40400000),          \
  V(STP, x,   0x80000000),          \
  V(LDP, x,   0x80400000),          \
  V(STP, s,   0x04000000),          \
  V(LDP, s,   0x04400000),          \
  V(STP, d,   0x44000000),          \
  V(LDP, d,   0x44400000),          \
  V(STP, q,   0x84000000),          \
  V(LDP, q,   0x84400000)

// Load/store pair (post, pre and offset.)
enum LoadStorePairOp {
  LoadStorePairMask = 0xC4400000,
  LoadStorePairLBit = 1 << 22,
  #define LOAD_STORE_PAIR(A, B, C) \
  A##_##B = C
  LOAD_STORE_PAIR_OP_LIST(LOAD_STORE_PAIR)
  #undef LOAD_STORE_PAIR
};

enum LoadStorePairPostIndexOp {
  LoadStorePairPostIndexFixed = 0x28800000,
  LoadStorePairPostIndexFMask = 0x3B800000,
  LoadStorePairPostIndexMask  = 0xFFC00000,
  #define LOAD_STORE_PAIR_POST_INDEX(A, B, C)  \
  A##_##B##_post = LoadStorePairPostIndexFixed | A##_##B
  LOAD_STORE_PAIR_OP_LIST(LOAD_STORE_PAIR_POST_INDEX)
  #undef LOAD_STORE_PAIR_POST_INDEX
};

enum LoadStorePairPreIndexOp {
  LoadStorePairPreIndexFixed = 0x29800000,
  LoadStorePairPreIndexFMask = 0x3B800000,
  LoadStorePairPreIndexMask  = 0xFFC00000,
  #define LOAD_STORE_PAIR_PRE_INDEX(A, B, C)  \
  A##_##B##_pre = LoadStorePairPreIndexFixed | A##_##B
  LOAD_STORE_PAIR_OP_LIST(LOAD_STORE_PAIR_PRE_INDEX)
  #undef LOAD_STORE_PAIR_PRE_INDEX
};

enum LoadStorePairOffsetOp {
  LoadStorePairOffsetFixed = 0x29000000,
  LoadStorePairOffsetFMask = 0x3B800000,
  LoadStorePairOffsetMask  = 0xFFC00000,
  #define LOAD_STORE_PAIR_OFFSET(A, B, C)  \
  A##_##B##_off = LoadStorePairOffsetFixed | A##_##B
  LOAD_STORE_PAIR_OP_LIST(LOAD_STORE_PAIR_OFFSET)
  #undef LOAD_STORE_PAIR_OFFSET
};

enum LoadStorePairNonTemporalOp {
  LoadStorePairNonTemporalFixed = 0x28000000,
  LoadStorePairNonTemporalFMask = 0x3B800000,
  LoadStorePairNonTemporalMask  = 0xFFC00000,
  LoadStorePairNonTemporalLBit = 1 << 22,
  STNP_w = LoadStorePairNonTemporalFixed | STP_w,
  LDNP_w = LoadStorePairNonTemporalFixed | LDP_w,
  STNP_x = LoadStorePairNonTemporalFixed | STP_x,
  LDNP_x = LoadStorePairNonTemporalFixed | LDP_x,
  STNP_s = LoadStorePairNonTemporalFixed | STP_s,
  LDNP_s = LoadStorePairNonTemporalFixed | LDP_s,
  STNP_d = LoadStorePairNonTemporalFixed | STP_d,
  LDNP_d = LoadStorePairNonTemporalFixed | LDP_d,
  STNP_q = LoadStorePairNonTemporalFixed | STP_q,
  LDNP_q = LoadStorePairNonTemporalFixed | LDP_q
};

// Load with pointer authentication.
enum LoadStorePACOp {
  LoadStorePACFixed  = 0xF8200400,
  LoadStorePACFMask  = 0xFF200400,
  LoadStorePACMask   = 0xFFA00C00,
  LoadStorePACPreBit = 0x00000800,
  LDRAA     = LoadStorePACFixed | 0x00000000,
  LDRAA_pre = LoadStorePACPreBit | LDRAA,
  LDRAB     = LoadStorePACFixed | 0x00800000,
  LDRAB_pre = LoadStorePACPreBit | LDRAB
};

// Load literal.
enum LoadLiteralOp {
  LoadLiteralFixed = 0x18000000,
  LoadLiteralFMask = 0x3B000000,
  LoadLiteralMask  = 0xFF000000,
  LDR_w_lit        = LoadLiteralFixed | 0x00000000,
  LDR_x_lit        = LoadLiteralFixed | 0x40000000,
  LDRSW_x_lit      = LoadLiteralFixed | 0x80000000,
  PRFM_lit         = LoadLiteralFixed | 0xC0000000,
  LDR_s_lit        = LoadLiteralFixed | 0x04000000,
  LDR_d_lit        = LoadLiteralFixed | 0x44000000,
  LDR_q_lit        = LoadLiteralFixed | 0x84000000
};

#define LOAD_STORE_OP_LIST(V)     \
  V(ST, RB, w,  0x00000000),  \
  V(ST, RH, w,  0x40000000),  \
  V(ST, R, w,   0x80000000),  \
  V(ST, R, x,   0xC0000000),  \
  V(LD, RB, w,  0x00400000),  \
  V(LD, RH, w,  0x40400000),  \
  V(LD, R, w,   0x80400000),  \
  V(LD, R, x,   0xC0400000),  \
  V(LD, RSB, x, 0x00800000),  \
  V(LD, RSH, x, 0x40800000),  \
  V(LD, RSW, x, 0x80800000),  \
  V(LD, RSB, w, 0x00C00000),  \
  V(LD, RSH, w, 0x40C00000),  \
  V(ST, R, b,   0x04000000),  \
  V(ST, R, h,   0x44000000),  \
  V(ST, R, s,   0x84000000),  \
  V(ST, R, d,   0xC4000000),  \
  V(ST, R, q,   0x04800000),  \
  V(LD, R, b,   0x04400000),  \
  V(LD, R, h,   0x44400000),  \
  V(LD, R, s,   0x84400000),  \
  V(LD, R, d,   0xC4400000),  \
  V(LD, R, q,   0x04C00000)

// Load/store (post, pre, offset and unsigned.)
enum LoadStoreOp {
  LoadStoreMask = 0xC4C00000,
  LoadStoreVMask = 0x04000000,
  #define LOAD_STORE(A, B, C, D)  \
  A##B##_##C = D
  LOAD_STORE_OP_LIST(LOAD_STORE),
  #undef LOAD_STORE
  PRFM = 0xC0800000
};

// Load/store unscaled offset.
enum LoadStoreUnscaledOffsetOp {
  LoadStoreUnscaledOffsetFixed = 0x38000000,
  LoadStoreUnscaledOffsetFMask = 0x3B200C00,
  LoadStoreUnscaledOffsetMask  = 0xFFE00C00,
  PRFUM                        = LoadStoreUnscaledOffsetFixed | PRFM,
  #define LOAD_STORE_UNSCALED(A, B, C, D)  \
  A##U##B##_##C = LoadStoreUnscaledOffsetFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_UNSCALED)
  #undef LOAD_STORE_UNSCALED
};

// Load/store post index.
enum LoadStorePostIndex {
  LoadStorePostIndexFixed = 0x38000400,
  LoadStorePostIndexFMask = 0x3B200C00,
  LoadStorePostIndexMask  = 0xFFE00C00,
  #define LOAD_STORE_POST_INDEX(A, B, C, D)  \
  A##B##_##C##_post = LoadStorePostIndexFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_POST_INDEX)
  #undef LOAD_STORE_POST_INDEX
};

// Load/store pre index.
enum LoadStorePreIndex {
  LoadStorePreIndexFixed = 0x38000C00,
  LoadStorePreIndexFMask = 0x3B200C00,
  LoadStorePreIndexMask  = 0xFFE00C00,
  #define LOAD_STORE_PRE_INDEX(A, B, C, D)  \
  A##B##_##C##_pre = LoadStorePreIndexFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_PRE_INDEX)
  #undef LOAD_STORE_PRE_INDEX
};

// Load/store unsigned offset.
enum LoadStoreUnsignedOffset {
  LoadStoreUnsignedOffsetFixed = 0x39000000,
  LoadStoreUnsignedOffsetFMask = 0x3B000000,
  LoadStoreUnsignedOffsetMask  = 0xFFC00000,
  PRFM_unsigned                = LoadStoreUnsignedOffsetFixed | PRFM,
  #define LOAD_STORE_UNSIGNED_OFFSET(A, B, C, D) \
  A##B##_##C##_unsigned = LoadStoreUnsignedOffsetFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_UNSIGNED_OFFSET)
  #undef LOAD_STORE_UNSIGNED_OFFSET
};

// Load/store register offset.
enum LoadStoreRegisterOffset {
  LoadStoreRegisterOffsetFixed = 0x38200800,
  LoadStoreRegisterOffsetFMask = 0x3B200C00,
  LoadStoreRegisterOffsetMask  = 0xFFE00C00,
  PRFM_reg                     = LoadStoreRegisterOffsetFixed | PRFM,
  #define LOAD_STORE_REGISTER_OFFSET(A, B, C, D) \
  A##B##_##C##_reg = LoadStoreRegisterOffsetFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_REGISTER_OFFSET)
  #undef LOAD_STORE_REGISTER_OFFSET
};

enum LoadStoreExclusive {
  LoadStoreExclusiveFixed = 0x08000000,
  LoadStoreExclusiveFMask = 0x3F000000,
  LoadStoreExclusiveMask  = 0xFFE08000,
  STXRB_w  = LoadStoreExclusiveFixed | 0x00000000,
  STXRH_w  = LoadStoreExclusiveFixed | 0x40000000,
  STXR_w   = LoadStoreExclusiveFixed | 0x80000000,
  STXR_x   = LoadStoreExclusiveFixed | 0xC0000000,
  LDXRB_w  = LoadStoreExclusiveFixed | 0x00400000,
  LDXRH_w  = LoadStoreExclusiveFixed | 0x40400000,
  LDXR_w   = LoadStoreExclusiveFixed | 0x80400000,
  LDXR_x   = LoadStoreExclusiveFixed | 0xC0400000,
  STXP_w   = LoadStoreExclusiveFixed | 0x80200000,
  STXP_x   = LoadStoreExclusiveFixed | 0xC0200000,
  LDXP_w   = LoadStoreExclusiveFixed | 0x80600000,
  LDXP_x   = LoadStoreExclusiveFixed | 0xC0600000,
  STLXRB_w = LoadStoreExclusiveFixed | 0x00008000,
  STLXRH_w = LoadStoreExclusiveFixed | 0x40008000,
  STLXR_w  = LoadStoreExclusiveFixed | 0x80008000,
  STLXR_x  = LoadStoreExclusiveFixed | 0xC0008000,
  LDAXRB_w = LoadStoreExclusiveFixed | 0x00408000,
  LDAXRH_w = LoadStoreExclusiveFixed | 0x40408000,
  LDAXR_w  = LoadStoreExclusiveFixed | 0x80408000,
  LDAXR_x  = LoadStoreExclusiveFixed | 0xC0408000,
  STLXP_w  = LoadStoreExclusiveFixed | 0x80208000,
  STLXP_x  = LoadStoreExclusiveFixed | 0xC0208000,
  LDAXP_w  = LoadStoreExclusiveFixed | 0x80608000,
  LDAXP_x  = LoadStoreExclusiveFixed | 0xC0608000,
  STLRB_w  = LoadStoreExclusiveFixed | 0x00808000,
  STLRH_w  = LoadStoreExclusiveFixed | 0x40808000,
  STLR_w   = LoadStoreExclusiveFixed | 0x80808000,
  STLR_x   = LoadStoreExclusiveFixed | 0xC0808000,
  LDARB_w  = LoadStoreExclusiveFixed | 0x00C08000,
  LDARH_w  = LoadStoreExclusiveFixed | 0x40C08000,
  LDAR_w   = LoadStoreExclusiveFixed | 0x80C08000,
  LDAR_x   = LoadStoreExclusiveFixed | 0xC0C08000,

  // v8.1 Load/store LORegion ops
  STLLRB   = LoadStoreExclusiveFixed | 0x00800000,
  LDLARB   = LoadStoreExclusiveFixed | 0x00C00000,
  STLLRH   = LoadStoreExclusiveFixed | 0x40800000,
  LDLARH   = LoadStoreExclusiveFixed | 0x40C00000,
  STLLR_w  = LoadStoreExclusiveFixed | 0x80800000,
  LDLAR_w  = LoadStoreExclusiveFixed | 0x80C00000,
  STLLR_x  = LoadStoreExclusiveFixed | 0xC0800000,
  LDLAR_x  = LoadStoreExclusiveFixed | 0xC0C00000,

  // v8.1 Load/store exclusive ops
  LSEBit_l  = 0x00400000,
  LSEBit_o0 = 0x00008000,
  LSEBit_sz = 0x40000000,
  CASFixed  = LoadStoreExclusiveFixed | 0x80A00000,
  CASBFixed = LoadStoreExclusiveFixed | 0x00A00000,
  CASHFixed = LoadStoreExclusiveFixed | 0x40A00000,
  CASPFixed = LoadStoreExclusiveFixed | 0x00200000,
  CAS_w    = CASFixed,
  CAS_x    = CASFixed | LSEBit_sz,
  CASA_w   = CASFixed | LSEBit_l,
  CASA_x   = CASFixed | LSEBit_l | LSEBit_sz,
  CASL_w   = CASFixed | LSEBit_o0,
  CASL_x   = CASFixed | LSEBit_o0 | LSEBit_sz,
  CASAL_w  = CASFixed | LSEBit_l | LSEBit_o0,
  CASAL_x  = CASFixed | LSEBit_l | LSEBit_o0 | LSEBit_sz,
  CASB     = CASBFixed,
  CASAB    = CASBFixed | LSEBit_l,
  CASLB    = CASBFixed | LSEBit_o0,
  CASALB   = CASBFixed | LSEBit_l | LSEBit_o0,
  CASH     = CASHFixed,
  CASAH    = CASHFixed | LSEBit_l,
  CASLH    = CASHFixed | LSEBit_o0,
  CASALH   = CASHFixed | LSEBit_l | LSEBit_o0,
  CASP_w   = CASPFixed,
  CASP_x   = CASPFixed | LSEBit_sz,
  CASPA_w  = CASPFixed | LSEBit_l,
  CASPA_x  = CASPFixed | LSEBit_l | LSEBit_sz,
  CASPL_w  = CASPFixed | LSEBit_o0,
  CASPL_x  = CASPFixed | LSEBit_o0 | LSEBit_sz,
  CASPAL_w = CASPFixed | LSEBit_l | LSEBit_o0,
  CASPAL_x = CASPFixed | LSEBit_l | LSEBit_o0 | LSEBit_sz
};

// Load/store RCpc unscaled offset.
enum LoadStoreRCpcUnscaledOffsetOp {
  LoadStoreRCpcUnscaledOffsetFixed = 0x19000000,
  LoadStoreRCpcUnscaledOffsetFMask = 0x3F200C00,
  LoadStoreRCpcUnscaledOffsetMask  = 0xFFE00C00,
  STLURB     = LoadStoreRCpcUnscaledOffsetFixed | 0x00000000,
  LDAPURB    = LoadStoreRCpcUnscaledOffsetFixed | 0x00400000,
  LDAPURSB_x = LoadStoreRCpcUnscaledOffsetFixed | 0x00800000,
  LDAPURSB_w = LoadStoreRCpcUnscaledOffsetFixed | 0x00C00000,
  STLURH     = LoadStoreRCpcUnscaledOffsetFixed | 0x40000000,
  LDAPURH    = LoadStoreRCpcUnscaledOffsetFixed | 0x40400000,
  LDAPURSH_x = LoadStoreRCpcUnscaledOffsetFixed | 0x40800000,
  LDAPURSH_w = LoadStoreRCpcUnscaledOffsetFixed | 0x40C00000,
  STLUR_w    = LoadStoreRCpcUnscaledOffsetFixed | 0x80000000,
  LDAPUR_w   = LoadStoreRCpcUnscaledOffsetFixed | 0x80400000,
  LDAPURSW   = LoadStoreRCpcUnscaledOffsetFixed | 0x80800000,
  STLUR_x    = LoadStoreRCpcUnscaledOffsetFixed | 0xC0000000,
  LDAPUR_x   = LoadStoreRCpcUnscaledOffsetFixed | 0xC0400000
};

#define ATOMIC_MEMORY_SIMPLE_OPC_LIST(V) \
  V(LDADD, 0x00000000),                  \
  V(LDCLR, 0x00001000),                  \
  V(LDEOR, 0x00002000),                  \
  V(LDSET, 0x00003000),                  \
  V(LDSMAX, 0x00004000),                 \
  V(LDSMIN, 0x00005000),                 \
  V(LDUMAX, 0x00006000),                 \
  V(LDUMIN, 0x00007000)

// Atomic memory.
enum AtomicMemoryOp {
  AtomicMemoryFixed = 0x38200000,
  AtomicMemoryFMask = 0x3B200C00,
  AtomicMemoryMask = 0xFFE0FC00,
  SWPB = AtomicMemoryFixed | 0x00008000,
  SWPAB = AtomicMemoryFixed | 0x00808000,
  SWPLB = AtomicMemoryFixed | 0x00408000,
  SWPALB = AtomicMemoryFixed | 0x00C08000,
  SWPH = AtomicMemoryFixed | 0x40008000,
  SWPAH = AtomicMemoryFixed | 0x40808000,
  SWPLH = AtomicMemoryFixed | 0x40408000,
  SWPALH = AtomicMemoryFixed | 0x40C08000,
  SWP_w = AtomicMemoryFixed | 0x80008000,
  SWPA_w = AtomicMemoryFixed | 0x80808000,
  SWPL_w = AtomicMemoryFixed | 0x80408000,
  SWPAL_w = AtomicMemoryFixed | 0x80C08000,
  SWP_x = AtomicMemoryFixed | 0xC0008000,
  SWPA_x = AtomicMemoryFixed | 0xC0808000,
  SWPL_x = AtomicMemoryFixed | 0xC0408000,
  SWPAL_x = AtomicMemoryFixed | 0xC0C08000,
  LDAPRB = AtomicMemoryFixed | 0x0080C000,
  LDAPRH = AtomicMemoryFixed | 0x4080C000,
  LDAPR_w = AtomicMemoryFixed | 0x8080C000,
  LDAPR_x = AtomicMemoryFixed | 0xC080C000,

  AtomicMemorySimpleFMask = 0x3B208C00,
  AtomicMemorySimpleOpMask = 0x00007000,
#define ATOMIC_MEMORY_SIMPLE(N, OP)              \
  N##Op = OP,                                    \
  N##B = AtomicMemoryFixed | OP,                 \
  N##AB = AtomicMemoryFixed | OP | 0x00800000,   \
  N##LB = AtomicMemoryFixed | OP | 0x00400000,   \
  N##ALB = AtomicMemoryFixed | OP | 0x00C00000,  \
  N##H = AtomicMemoryFixed | OP | 0x40000000,    \
  N##AH = AtomicMemoryFixed | OP | 0x40800000,   \
  N##LH = AtomicMemoryFixed | OP | 0x40400000,   \
  N##ALH = AtomicMemoryFixed | OP | 0x40C00000,  \
  N##_w = AtomicMemoryFixed | OP | 0x80000000,   \
  N##A_w = AtomicMemoryFixed | OP | 0x80800000,  \
  N##L_w = AtomicMemoryFixed | OP | 0x80400000,  \
  N##AL_w = AtomicMemoryFixed | OP | 0x80C00000, \
  N##_x = AtomicMemoryFixed | OP | 0xC0000000,   \
  N##A_x = AtomicMemoryFixed | OP | 0xC0800000,  \
  N##L_x = AtomicMemoryFixed | OP | 0xC0400000,  \
  N##AL_x = AtomicMemoryFixed | OP | 0xC0C00000

  ATOMIC_MEMORY_SIMPLE_OPC_LIST(ATOMIC_MEMORY_SIMPLE)
#undef ATOMIC_MEMORY_SIMPLE
};

// Conditional compare.
enum ConditionalCompareOp {
  ConditionalCompareMask = 0x60000000,
  CCMN                   = 0x20000000,
  CCMP                   = 0x60000000
};

// Conditional compare register.
enum ConditionalCompareRegisterOp {
  ConditionalCompareRegisterFixed = 0x1A400000,
  ConditionalCompareRegisterFMask = 0x1FE00800,
  ConditionalCompareRegisterMask  = 0xFFE00C10,
  CCMN_w = ConditionalCompareRegisterFixed | CCMN,
  CCMN_x = ConditionalCompareRegisterFixed | SixtyFourBits | CCMN,
  CCMP_w = ConditionalCompareRegisterFixed | CCMP,
  CCMP_x = ConditionalCompareRegisterFixed | SixtyFourBits | CCMP
};

// Conditional compare immediate.
enum ConditionalCompareImmediateOp {
  ConditionalCompareImmediateFixed = 0x1A400800,
  ConditionalCompareImmediateFMask = 0x1FE00800,
  ConditionalCompareImmediateMask  = 0xFFE00C10,
  CCMN_w_imm = ConditionalCompareImmediateFixed | CCMN,
  CCMN_x_imm = ConditionalCompareImmediateFixed | SixtyFourBits | CCMN,
  CCMP_w_imm = ConditionalCompareImmediateFixed | CCMP,
  CCMP_x_imm = ConditionalCompareImmediateFixed | SixtyFourBits | CCMP
};

// Conditional select.
enum ConditionalSelectOp {
  ConditionalSelectFixed = 0x1A800000,
  ConditionalSelectFMask = 0x1FE00000,
  ConditionalSelectMask  = 0xFFE00C00,
  CSEL_w                 = ConditionalSelectFixed | 0x00000000,
  CSEL_x                 = ConditionalSelectFixed | 0x80000000,
  CSEL                   = CSEL_w,
  CSINC_w                = ConditionalSelectFixed | 0x00000400,
  CSINC_x                = ConditionalSelectFixed | 0x80000400,
  CSINC                  = CSINC_w,
  CSINV_w                = ConditionalSelectFixed | 0x40000000,
  CSINV_x                = ConditionalSelectFixed | 0xC0000000,
  CSINV                  = CSINV_w,
  CSNEG_w                = ConditionalSelectFixed | 0x40000400,
  CSNEG_x                = ConditionalSelectFixed | 0xC0000400,
  CSNEG                  = CSNEG_w
};

// Data processing 1 source.
enum DataProcessing1SourceOp {
  DataProcessing1SourceFixed = 0x5AC00000,
  DataProcessing1SourceFMask = 0x5FE00000,
  DataProcessing1SourceMask  = 0xFFFFFC00,
  RBIT    = DataProcessing1SourceFixed | 0x00000000,
  RBIT_w  = RBIT,
  RBIT_x  = RBIT | SixtyFourBits,
  REV16   = DataProcessing1SourceFixed | 0x00000400,
  REV16_w = REV16,
  REV16_x = REV16 | SixtyFourBits,
  REV     = DataProcessing1SourceFixed | 0x00000800,
  REV_w   = REV,
  REV32_x = REV | SixtyFourBits,
  REV_x   = DataProcessing1SourceFixed | SixtyFourBits | 0x00000C00,
  CLZ     = DataProcessing1SourceFixed | 0x00001000,
  CLZ_w   = CLZ,
  CLZ_x   = CLZ | SixtyFourBits,
  CLS     = DataProcessing1SourceFixed | 0x00001400,
  CLS_w   = CLS,
  CLS_x   = CLS | SixtyFourBits,

  // Pointer authentication instructions in Armv8.3.
  PACIA  = DataProcessing1SourceFixed | 0x80010000,
  PACIB  = DataProcessing1SourceFixed | 0x80010400,
  PACDA  = DataProcessing1SourceFixed | 0x80010800,
  PACDB  = DataProcessing1SourceFixed | 0x80010C00,
  AUTIA  = DataProcessing1SourceFixed | 0x80011000,
  AUTIB  = DataProcessing1SourceFixed | 0x80011400,
  AUTDA  = DataProcessing1SourceFixed | 0x80011800,
  AUTDB  = DataProcessing1SourceFixed | 0x80011C00,
  PACIZA = DataProcessing1SourceFixed | 0x80012000,
  PACIZB = DataProcessing1SourceFixed | 0x80012400,
  PACDZA = DataProcessing1SourceFixed | 0x80012800,
  PACDZB = DataProcessing1SourceFixed | 0x80012C00,
  AUTIZA = DataProcessing1SourceFixed | 0x80013000,
  AUTIZB = DataProcessing1SourceFixed | 0x80013400,
  AUTDZA = DataProcessing1SourceFixed | 0x80013800,
  AUTDZB = DataProcessing1SourceFixed | 0x80013C00,
  XPACI  = DataProcessing1SourceFixed | 0x80014000,
  XPACD  = DataProcessing1SourceFixed | 0x80014400
};

// Data processing 2 source.
enum DataProcessing2SourceOp {
  DataProcessing2SourceFixed = 0x1AC00000,
  DataProcessing2SourceFMask = 0x5FE00000,
  DataProcessing2SourceMask  = 0xFFE0FC00,
  UDIV_w  = DataProcessing2SourceFixed | 0x00000800,
  UDIV_x  = DataProcessing2SourceFixed | 0x80000800,
  UDIV    = UDIV_w,
  SDIV_w  = DataProcessing2SourceFixed | 0x00000C00,
  SDIV_x  = DataProcessing2SourceFixed | 0x80000C00,
  SDIV    = SDIV_w,
  LSLV_w  = DataProcessing2SourceFixed | 0x00002000,
  LSLV_x  = DataProcessing2SourceFixed | 0x80002000,
  LSLV    = LSLV_w,
  LSRV_w  = DataProcessing2SourceFixed | 0x00002400,
  LSRV_x  = DataProcessing2SourceFixed | 0x80002400,
  LSRV    = LSRV_w,
  ASRV_w  = DataProcessing2SourceFixed | 0x00002800,
  ASRV_x  = DataProcessing2SourceFixed | 0x80002800,
  ASRV    = ASRV_w,
  RORV_w  = DataProcessing2SourceFixed | 0x00002C00,
  RORV_x  = DataProcessing2SourceFixed | 0x80002C00,
  RORV    = RORV_w,
  PACGA   = DataProcessing2SourceFixed | SixtyFourBits | 0x00003000,
  CRC32B  = DataProcessing2SourceFixed | 0x00004000,
  CRC32H  = DataProcessing2SourceFixed | 0x00004400,
  CRC32W  = DataProcessing2SourceFixed | 0x00004800,
  CRC32X  = DataProcessing2SourceFixed | SixtyFourBits | 0x00004C00,
  CRC32CB = DataProcessing2SourceFixed | 0x00005000,
  CRC32CH = DataProcessing2SourceFixed | 0x00005400,
  CRC32CW = DataProcessing2SourceFixed | 0x00005800,
  CRC32CX = DataProcessing2SourceFixed | SixtyFourBits | 0x00005C00
};

// Data processing 3 source.
enum DataProcessing3SourceOp {
  DataProcessing3SourceFixed = 0x1B000000,
  DataProcessing3SourceFMask = 0x1F000000,
  DataProcessing3SourceMask  = 0xFFE08000,
  MADD_w                     = DataProcessing3SourceFixed | 0x00000000,
  MADD_x                     = DataProcessing3SourceFixed | 0x80000000,
  MADD                       = MADD_w,
  MSUB_w                     = DataProcessing3SourceFixed | 0x00008000,
  MSUB_x                     = DataProcessing3SourceFixed | 0x80008000,
  MSUB                       = MSUB_w,
  SMADDL_x                   = DataProcessing3SourceFixed | 0x80200000,
  SMSUBL_x                   = DataProcessing3SourceFixed | 0x80208000,
  SMULH_x                    = DataProcessing3SourceFixed | 0x80400000,
  UMADDL_x                   = DataProcessing3SourceFixed | 0x80A00000,
  UMSUBL_x                   = DataProcessing3SourceFixed | 0x80A08000,
  UMULH_x                    = DataProcessing3SourceFixed | 0x80C00000
};

// Floating point compare.
enum FPCompareOp {
  FPCompareFixed = 0x1E202000,
  FPCompareFMask = 0x5F203C00,
  FPCompareMask  = 0xFFE0FC1F,
  FCMP_h         = FPCompareFixed | FP16 | 0x00000000,
  FCMP_s         = FPCompareFixed | 0x00000000,
  FCMP_d         = FPCompareFixed | FP64 | 0x00000000,
  FCMP           = FCMP_s,
  FCMP_h_zero    = FPCompareFixed | FP16 | 0x00000008,
  FCMP_s_zero    = FPCompareFixed | 0x00000008,
  FCMP_d_zero    = FPCompareFixed | FP64 | 0x00000008,
  FCMP_zero      = FCMP_s_zero,
  FCMPE_h        = FPCompareFixed | FP16 | 0x00000010,
  FCMPE_s        = FPCompareFixed | 0x00000010,
  FCMPE_d        = FPCompareFixed | FP64 | 0x00000010,
  FCMPE          = FCMPE_s,
  FCMPE_h_zero   = FPCompareFixed | FP16 | 0x00000018,
  FCMPE_s_zero   = FPCompareFixed | 0x00000018,
  FCMPE_d_zero   = FPCompareFixed | FP64 | 0x00000018,
  FCMPE_zero     = FCMPE_s_zero
};

// Floating point conditional compare.
enum FPConditionalCompareOp {
  FPConditionalCompareFixed = 0x1E200400,
  FPConditionalCompareFMask = 0x5F200C00,
  FPConditionalCompareMask  = 0xFFE00C10,
  FCCMP_h                   = FPConditionalCompareFixed | FP16 | 0x00000000,
  FCCMP_s                   = FPConditionalCompareFixed | 0x00000000,
  FCCMP_d                   = FPConditionalCompareFixed | FP64 | 0x00000000,
  FCCMP                     = FCCMP_s,
  FCCMPE_h                  = FPConditionalCompareFixed | FP16 | 0x00000010,
  FCCMPE_s                  = FPConditionalCompareFixed | 0x00000010,
  FCCMPE_d                  = FPConditionalCompareFixed | FP64 | 0x00000010,
  FCCMPE                    = FCCMPE_s
};

// Floating point conditional select.
enum FPConditionalSelectOp {
  FPConditionalSelectFixed = 0x1E200C00,
  FPConditionalSelectFMask = 0x5F200C00,
  FPConditionalSelectMask  = 0xFFE00C00,
  FCSEL_h                  = FPConditionalSelectFixed | FP16 | 0x00000000,
  FCSEL_s                  = FPConditionalSelectFixed | 0x00000000,
  FCSEL_d                  = FPConditionalSelectFixed | FP64 | 0x00000000,
  FCSEL                    = FCSEL_s
};

// Floating point immediate.
enum FPImmediateOp {
  FPImmediateFixed = 0x1E201000,
  FPImmediateFMask = 0x5F201C00,
  FPImmediateMask  = 0xFFE01C00,
  FMOV_h_imm       = FPImmediateFixed | FP16 | 0x00000000,
  FMOV_s_imm       = FPImmediateFixed | 0x00000000,
  FMOV_d_imm       = FPImmediateFixed | FP64 | 0x00000000
};

// Floating point data processing 1 source.
enum FPDataProcessing1SourceOp {
  FPDataProcessing1SourceFixed = 0x1E204000,
  FPDataProcessing1SourceFMask = 0x5F207C00,
  FPDataProcessing1SourceMask  = 0xFFFFFC00,
  FMOV_h   = FPDataProcessing1SourceFixed | FP16 | 0x00000000,
  FMOV_s   = FPDataProcessing1SourceFixed | 0x00000000,
  FMOV_d   = FPDataProcessing1SourceFixed | FP64 | 0x00000000,
  FMOV     = FMOV_s,
  FABS_h   = FPDataProcessing1SourceFixed | FP16 | 0x00008000,
  FABS_s   = FPDataProcessing1SourceFixed | 0x00008000,
  FABS_d   = FPDataProcessing1SourceFixed | FP64 | 0x00008000,
  FABS     = FABS_s,
  FNEG_h   = FPDataProcessing1SourceFixed | FP16 | 0x00010000,
  FNEG_s   = FPDataProcessing1SourceFixed | 0x00010000,
  FNEG_d   = FPDataProcessing1SourceFixed | FP64 | 0x00010000,
  FNEG     = FNEG_s,
  FSQRT_h  = FPDataProcessing1SourceFixed | FP16 | 0x00018000,
  FSQRT_s  = FPDataProcessing1SourceFixed | 0x00018000,
  FSQRT_d  = FPDataProcessing1SourceFixed | FP64 | 0x00018000,
  FSQRT    = FSQRT_s,
  FCVT_ds  = FPDataProcessing1SourceFixed | 0x00028000,
  FCVT_sd  = FPDataProcessing1SourceFixed | FP64 | 0x00020000,
  FCVT_hs  = FPDataProcessing1SourceFixed | 0x00038000,
  FCVT_hd  = FPDataProcessing1SourceFixed | FP64 | 0x00038000,
  FCVT_sh  = FPDataProcessing1SourceFixed | 0x00C20000,
  FCVT_dh  = FPDataProcessing1SourceFixed | 0x00C28000,
  FRINT32X_s = FPDataProcessing1SourceFixed | 0x00088000,
  FRINT32X_d = FPDataProcessing1SourceFixed | FP64 | 0x00088000,
  FRINT32X = FRINT32X_s,
  FRINT32Z_s = FPDataProcessing1SourceFixed | 0x00080000,
  FRINT32Z_d = FPDataProcessing1SourceFixed | FP64 | 0x00080000,
  FRINT32Z = FRINT32Z_s,
  FRINT64X_s = FPDataProcessing1SourceFixed | 0x00098000,
  FRINT64X_d = FPDataProcessing1SourceFixed | FP64 | 0x00098000,
  FRINT64X = FRINT64X_s,
  FRINT64Z_s = FPDataProcessing1SourceFixed | 0x00090000,
  FRINT64Z_d = FPDataProcessing1SourceFixed | FP64 | 0x00090000,
  FRINT64Z = FRINT64Z_s,
  FRINTN_h = FPDataProcessing1SourceFixed | FP16 | 0x00040000,
  FRINTN_s = FPDataProcessing1SourceFixed | 0x00040000,
  FRINTN_d = FPDataProcessing1SourceFixed | FP64 | 0x00040000,
  FRINTN   = FRINTN_s,
  FRINTP_h = FPDataProcessing1SourceFixed | FP16 | 0x00048000,
  FRINTP_s = FPDataProcessing1SourceFixed | 0x00048000,
  FRINTP_d = FPDataProcessing1SourceFixed | FP64 | 0x00048000,
  FRINTP   = FRINTP_s,
  FRINTM_h = FPDataProcessing1SourceFixed | FP16 | 0x00050000,
  FRINTM_s = FPDataProcessing1SourceFixed | 0x00050000,
  FRINTM_d = FPDataProcessing1SourceFixed | FP64 | 0x00050000,
  FRINTM   = FRINTM_s,
  FRINTZ_h = FPDataProcessing1SourceFixed | FP16 | 0x00058000,
  FRINTZ_s = FPDataProcessing1SourceFixed | 0x00058000,
  FRINTZ_d = FPDataProcessing1SourceFixed | FP64 | 0x00058000,
  FRINTZ   = FRINTZ_s,
  FRINTA_h = FPDataProcessing1SourceFixed | FP16 | 0x00060000,
  FRINTA_s = FPDataProcessing1SourceFixed | 0x00060000,
  FRINTA_d = FPDataProcessing1SourceFixed | FP64 | 0x00060000,
  FRINTA   = FRINTA_s,
  FRINTX_h = FPDataProcessing1SourceFixed | FP16 | 0x00070000,
  FRINTX_s = FPDataProcessing1SourceFixed | 0x00070000,
  FRINTX_d = FPDataProcessing1SourceFixed | FP64 | 0x00070000,
  FRINTX   = FRINTX_s,
  FRINTI_h = FPDataProcessing1SourceFixed | FP16 | 0x00078000,
  FRINTI_s = FPDataProcessing1SourceFixed | 0x00078000,
  FRINTI_d = FPDataProcessing1SourceFixed | FP64 | 0x00078000,
  FRINTI   = FRINTI_s
};

// Floating point data processing 2 source.
enum FPDataProcessing2SourceOp {
  FPDataProcessing2SourceFixed = 0x1E200800,
  FPDataProcessing2SourceFMask = 0x5F200C00,
  FPDataProcessing2SourceMask  = 0xFFE0FC00,
  FMUL     = FPDataProcessing2SourceFixed | 0x00000000,
  FMUL_h   = FMUL | FP16,
  FMUL_s   = FMUL,
  FMUL_d   = FMUL | FP64,
  FDIV     = FPDataProcessing2SourceFixed | 0x00001000,
  FDIV_h   = FDIV | FP16,
  FDIV_s   = FDIV,
  FDIV_d   = FDIV | FP64,
  FADD     = FPDataProcessing2SourceFixed | 0x00002000,
  FADD_h   = FADD | FP16,
  FADD_s   = FADD,
  FADD_d   = FADD | FP64,
  FSUB     = FPDataProcessing2SourceFixed | 0x00003000,
  FSUB_h   = FSUB | FP16,
  FSUB_s   = FSUB,
  FSUB_d   = FSUB | FP64,
  FMAX     = FPDataProcessing2SourceFixed | 0x00004000,
  FMAX_h   = FMAX | FP16,
  FMAX_s   = FMAX,
  FMAX_d   = FMAX | FP64,
  FMIN     = FPDataProcessing2SourceFixed | 0x00005000,
  FMIN_h   = FMIN | FP16,
  FMIN_s   = FMIN,
  FMIN_d   = FMIN | FP64,
  FMAXNM   = FPDataProcessing2SourceFixed | 0x00006000,
  FMAXNM_h = FMAXNM | FP16,
  FMAXNM_s = FMAXNM,
  FMAXNM_d = FMAXNM | FP64,
  FMINNM   = FPDataProcessing2SourceFixed | 0x00007000,
  FMINNM_h = FMINNM | FP16,
  FMINNM_s = FMINNM,
  FMINNM_d = FMINNM | FP64,
  FNMUL    = FPDataProcessing2SourceFixed | 0x00008000,
  FNMUL_h  = FNMUL | FP16,
  FNMUL_s  = FNMUL,
  FNMUL_d  = FNMUL | FP64
};

// Floating point data processing 3 source.
enum FPDataProcessing3SourceOp {
  FPDataProcessing3SourceFixed = 0x1F000000,
  FPDataProcessing3SourceFMask = 0x5F000000,
  FPDataProcessing3SourceMask  = 0xFFE08000,
  FMADD_h                      = FPDataProcessing3SourceFixed | 0x00C00000,
  FMSUB_h                      = FPDataProcessing3SourceFixed | 0x00C08000,
  FNMADD_h                     = FPDataProcessing3SourceFixed | 0x00E00000,
  FNMSUB_h                     = FPDataProcessing3SourceFixed | 0x00E08000,
  FMADD_s                      = FPDataProcessing3SourceFixed | 0x00000000,
  FMSUB_s                      = FPDataProcessing3SourceFixed | 0x00008000,
  FNMADD_s                     = FPDataProcessing3SourceFixed | 0x00200000,
  FNMSUB_s                     = FPDataProcessing3SourceFixed | 0x00208000,
  FMADD_d                      = FPDataProcessing3SourceFixed | 0x00400000,
  FMSUB_d                      = FPDataProcessing3SourceFixed | 0x00408000,
  FNMADD_d                     = FPDataProcessing3SourceFixed | 0x00600000,
  FNMSUB_d                     = FPDataProcessing3SourceFixed | 0x00608000
};

// Conversion between floating point and integer.
enum FPIntegerConvertOp {
  FPIntegerConvertFixed = 0x1E200000,
  FPIntegerConvertFMask = 0x5F20FC00,
  FPIntegerConvertMask  = 0xFFFFFC00,
  FCVTNS    = FPIntegerConvertFixed | 0x00000000,
  FCVTNS_wh = FCVTNS | FP16,
  FCVTNS_xh = FCVTNS | SixtyFourBits | FP16,
  FCVTNS_ws = FCVTNS,
  FCVTNS_xs = FCVTNS | SixtyFourBits,
  FCVTNS_wd = FCVTNS | FP64,
  FCVTNS_xd = FCVTNS | SixtyFourBits | FP64,
  FCVTNU    = FPIntegerConvertFixed | 0x00010000,
  FCVTNU_wh = FCVTNU | FP16,
  FCVTNU_xh = FCVTNU | SixtyFourBits | FP16,
  FCVTNU_ws = FCVTNU,
  FCVTNU_xs = FCVTNU | SixtyFourBits,
  FCVTNU_wd = FCVTNU | FP64,
  FCVTNU_xd = FCVTNU | SixtyFourBits | FP64,
  FCVTPS    = FPIntegerConvertFixed | 0x00080000,
  FCVTPS_wh = FCVTPS | FP16,
  FCVTPS_xh = FCVTPS | SixtyFourBits | FP16,
  FCVTPS_ws = FCVTPS,
  FCVTPS_xs = FCVTPS | SixtyFourBits,
  FCVTPS_wd = FCVTPS | FP64,
  FCVTPS_xd = FCVTPS | SixtyFourBits | FP64,
  FCVTPU    = FPIntegerConvertFixed | 0x00090000,
  FCVTPU_wh = FCVTPU | FP16,
  FCVTPU_xh = FCVTPU | SixtyFourBits | FP16,
  FCVTPU_ws = FCVTPU,
  FCVTPU_xs = FCVTPU | SixtyFourBits,
  FCVTPU_wd = FCVTPU | FP64,
  FCVTPU_xd = FCVTPU | SixtyFourBits | FP64,
  FCVTMS    = FPIntegerConvertFixed | 0x00100000,
  FCVTMS_wh = FCVTMS | FP16,
  FCVTMS_xh = FCVTMS | SixtyFourBits | FP16,
  FCVTMS_ws = FCVTMS,
  FCVTMS_xs = FCVTMS | SixtyFourBits,
  FCVTMS_wd = FCVTMS | FP64,
  FCVTMS_xd = FCVTMS | SixtyFourBits | FP64,
  FCVTMU    = FPIntegerConvertFixed | 0x00110000,
  FCVTMU_wh = FCVTMU | FP16,
  FCVTMU_xh = FCVTMU | SixtyFourBits | FP16,
  FCVTMU_ws = FCVTMU,
  FCVTMU_xs = FCVTMU | SixtyFourBits,
  FCVTMU_wd = FCVTMU | FP64,
  FCVTMU_xd = FCVTMU | SixtyFourBits | FP64,
  FCVTZS    = FPIntegerConvertFixed | 0x00180000,
  FCVTZS_wh = FCVTZS | FP16,
  FCVTZS_xh = FCVTZS | SixtyFourBits | FP16,
  FCVTZS_ws = FCVTZS,
  FCVTZS_xs = FCVTZS | SixtyFourBits,
  FCVTZS_wd = FCVTZS | FP64,
  FCVTZS_xd = FCVTZS | SixtyFourBits | FP64,
  FCVTZU    = FPIntegerConvertFixed | 0x00190000,
  FCVTZU_wh = FCVTZU | FP16,
  FCVTZU_xh = FCVTZU | SixtyFourBits | FP16,
  FCVTZU_ws = FCVTZU,
  FCVTZU_xs = FCVTZU | SixtyFourBits,
  FCVTZU_wd = FCVTZU | FP64,
  FCVTZU_xd = FCVTZU | SixtyFourBits | FP64,
  SCVTF     = FPIntegerConvertFixed | 0x00020000,
  SCVTF_hw  = SCVTF | FP16,
  SCVTF_hx  = SCVTF | SixtyFourBits | FP16,
  SCVTF_sw  = SCVTF,
  SCVTF_sx  = SCVTF | SixtyFourBits,
  SCVTF_dw  = SCVTF | FP64,
  SCVTF_dx  = SCVTF | SixtyFourBits | FP64,
  UCVTF     = FPIntegerConvertFixed | 0x00030000,
  UCVTF_hw  = UCVTF | FP16,
  UCVTF_hx  = UCVTF | SixtyFourBits | FP16,
  UCVTF_sw  = UCVTF,
  UCVTF_sx  = UCVTF | SixtyFourBits,
  UCVTF_dw  = UCVTF | FP64,
  UCVTF_dx  = UCVTF | SixtyFourBits | FP64,
  FCVTAS    = FPIntegerConvertFixed | 0x00040000,
  FCVTAS_wh = FCVTAS | FP16,
  FCVTAS_xh = FCVTAS | SixtyFourBits | FP16,
  FCVTAS_ws = FCVTAS,
  FCVTAS_xs = FCVTAS | SixtyFourBits,
  FCVTAS_wd = FCVTAS | FP64,
  FCVTAS_xd = FCVTAS | SixtyFourBits | FP64,
  FCVTAU    = FPIntegerConvertFixed | 0x00050000,
  FCVTAU_wh = FCVTAU | FP16,
  FCVTAU_xh = FCVTAU | SixtyFourBits | FP16,
  FCVTAU_ws = FCVTAU,
  FCVTAU_xs = FCVTAU | SixtyFourBits,
  FCVTAU_wd = FCVTAU | FP64,
  FCVTAU_xd = FCVTAU | SixtyFourBits | FP64,
  FMOV_wh   = FPIntegerConvertFixed | 0x00060000 | FP16,
  FMOV_hw   = FPIntegerConvertFixed | 0x00070000 | FP16,
  FMOV_xh   = FMOV_wh | SixtyFourBits,
  FMOV_hx   = FMOV_hw | SixtyFourBits,
  FMOV_ws   = FPIntegerConvertFixed | 0x00060000,
  FMOV_sw   = FPIntegerConvertFixed | 0x00070000,
  FMOV_xd   = FMOV_ws | SixtyFourBits | FP64,
  FMOV_dx   = FMOV_sw | SixtyFourBits | FP64,
  FMOV_d1_x = FPIntegerConvertFixed | SixtyFourBits | 0x008F0000,
  FMOV_x_d1 = FPIntegerConvertFixed | SixtyFourBits | 0x008E0000,
  FJCVTZS   = FPIntegerConvertFixed | FP64 | 0x001E0000
};

// Conversion between fixed point and floating point.
enum FPFixedPointConvertOp {
  FPFixedPointConvertFixed = 0x1E000000,
  FPFixedPointConvertFMask = 0x5F200000,
  FPFixedPointConvertMask  = 0xFFFF0000,
  FCVTZS_fixed    = FPFixedPointConvertFixed | 0x00180000,
  FCVTZS_wh_fixed = FCVTZS_fixed | FP16,
  FCVTZS_xh_fixed = FCVTZS_fixed | SixtyFourBits | FP16,
  FCVTZS_ws_fixed = FCVTZS_fixed,
  FCVTZS_xs_fixed = FCVTZS_fixed | SixtyFourBits,
  FCVTZS_wd_fixed = FCVTZS_fixed | FP64,
  FCVTZS_xd_fixed = FCVTZS_fixed | SixtyFourBits | FP64,
  FCVTZU_fixed    = FPFixedPointConvertFixed | 0x00190000,
  FCVTZU_wh_fixed = FCVTZU_fixed | FP16,
  FCVTZU_xh_fixed = FCVTZU_fixed | SixtyFourBits | FP16,
  FCVTZU_ws_fixed = FCVTZU_fixed,
  FCVTZU_xs_fixed = FCVTZU_fixed | SixtyFourBits,
  FCVTZU_wd_fixed = FCVTZU_fixed | FP64,
  FCVTZU_xd_fixed = FCVTZU_fixed | SixtyFourBits | FP64,
  SCVTF_fixed     = FPFixedPointConvertFixed | 0x00020000,
  SCVTF_hw_fixed  = SCVTF_fixed | FP16,
  SCVTF_hx_fixed  = SCVTF_fixed | SixtyFourBits | FP16,
  SCVTF_sw_fixed  = SCVTF_fixed,
  SCVTF_sx_fixed  = SCVTF_fixed | SixtyFourBits,
  SCVTF_dw_fixed  = SCVTF_fixed | FP64,
  SCVTF_dx_fixed  = SCVTF_fixed | SixtyFourBits | FP64,
  UCVTF_fixed     = FPFixedPointConvertFixed | 0x00030000,
  UCVTF_hw_fixed  = UCVTF_fixed | FP16,
  UCVTF_hx_fixed  = UCVTF_fixed | SixtyFourBits | FP16,
  UCVTF_sw_fixed  = UCVTF_fixed,
  UCVTF_sx_fixed  = UCVTF_fixed | SixtyFourBits,
  UCVTF_dw_fixed  = UCVTF_fixed | FP64,
  UCVTF_dx_fixed  = UCVTF_fixed | SixtyFourBits | FP64
};

// Crypto - two register SHA.
enum Crypto2RegSHAOp {
  Crypto2RegSHAFixed = 0x5E280800,
  Crypto2RegSHAFMask = 0xFF3E0C00
};

// Crypto - three register SHA.
enum Crypto3RegSHAOp {
  Crypto3RegSHAFixed = 0x5E000000,
  Crypto3RegSHAFMask = 0xFF208C00
};

// Crypto - AES.
enum CryptoAESOp {
  CryptoAESFixed = 0x4E280800,
  CryptoAESFMask = 0xFF3E0C00
};

// NEON instructions with two register operands.
enum NEON2RegMiscOp {
  NEON2RegMiscFixed = 0x0E200800,
  NEON2RegMiscFMask = 0x9F3E0C00,
  NEON2RegMiscMask  = 0xBF3FFC00,
  NEON2RegMiscUBit  = 0x20000000,
  NEON_REV64     = NEON2RegMiscFixed | 0x00000000,
  NEON_REV32     = NEON2RegMiscFixed | 0x20000000,
  NEON_REV16     = NEON2RegMiscFixed | 0x00001000,
  NEON_SADDLP    = NEON2RegMiscFixed | 0x00002000,
  NEON_UADDLP    = NEON_SADDLP | NEON2RegMiscUBit,
  NEON_SUQADD    = NEON2RegMiscFixed | 0x00003000,
  NEON_USQADD    = NEON_SUQADD | NEON2RegMiscUBit,
  NEON_CLS       = NEON2RegMiscFixed | 0x00004000,
  NEON_CLZ       = NEON2RegMiscFixed | 0x20004000,
  NEON_CNT       = NEON2RegMiscFixed | 0x00005000,
  NEON_RBIT_NOT  = NEON2RegMiscFixed | 0x20005000,
  NEON_SADALP    = NEON2RegMiscFixed | 0x00006000,
  NEON_UADALP    = NEON_SADALP | NEON2RegMiscUBit,
  NEON_SQABS     = NEON2RegMiscFixed | 0x00007000,
  NEON_SQNEG     = NEON2RegMiscFixed | 0x20007000,
  NEON_CMGT_zero = NEON2RegMiscFixed | 0x00008000,
  NEON_CMGE_zero = NEON2RegMiscFixed | 0x20008000,
  NEON_CMEQ_zero = NEON2RegMiscFixed | 0x00009000,
  NEON_CMLE_zero = NEON2RegMiscFixed | 0x20009000,
  NEON_CMLT_zero = NEON2RegMiscFixed | 0x0000A000,
  NEON_ABS       = NEON2RegMiscFixed | 0x0000B000,
  NEON_NEG       = NEON2RegMiscFixed | 0x2000B000,
  NEON_XTN       = NEON2RegMiscFixed | 0x00012000,
  NEON_SQXTUN    = NEON2RegMiscFixed | 0x20012000,
  NEON_SHLL      = NEON2RegMiscFixed | 0x20013000,
  NEON_SQXTN     = NEON2RegMiscFixed | 0x00014000,
  NEON_UQXTN     = NEON_SQXTN | NEON2RegMiscUBit,

  NEON2RegMiscOpcode = 0x0001F000,
  NEON_RBIT_NOT_opcode = NEON_RBIT_NOT & NEON2RegMiscOpcode,
  NEON_NEG_opcode = NEON_NEG & NEON2RegMiscOpcode,
  NEON_XTN_opcode = NEON_XTN & NEON2RegMiscOpcode,
  NEON_UQXTN_opcode = NEON_UQXTN & NEON2RegMiscOpcode,

  // These instructions use only one bit of the size field. The other bit is
  // used to distinguish between instructions.
  NEON2RegMiscFPMask = NEON2RegMiscMask | 0x00800000,
  NEON_FABS   = NEON2RegMiscFixed | 0x0080F000,
  NEON_FNEG   = NEON2RegMiscFixed | 0x2080F000,
  NEON_FCVTN  = NEON2RegMiscFixed | 0x00016000,
  NEON_FCVTXN = NEON2RegMiscFixed | 0x20016000,
  NEON_FCVTL  = NEON2RegMiscFixed | 0x00017000,
  NEON_FRINT32X = NEON2RegMiscFixed | 0x2001E000,
  NEON_FRINT32Z = NEON2RegMiscFixed | 0x0001E000,
  NEON_FRINT64X = NEON2RegMiscFixed | 0x2001F000,
  NEON_FRINT64Z = NEON2RegMiscFixed | 0x0001F000,
  NEON_FRINTN = NEON2RegMiscFixed | 0x00018000,
  NEON_FRINTA = NEON2RegMiscFixed | 0x20018000,
  NEON_FRINTP = NEON2RegMiscFixed | 0x00818000,
  NEON_FRINTM = NEON2RegMiscFixed | 0x00019000,
  NEON_FRINTX = NEON2RegMiscFixed | 0x20019000,
  NEON_FRINTZ = NEON2RegMiscFixed | 0x00819000,
  NEON_FRINTI = NEON2RegMiscFixed | 0x20819000,
  NEON_FCVTNS = NEON2RegMiscFixed | 0x0001A000,
  NEON_FCVTNU = NEON_FCVTNS | NEON2RegMiscUBit,
  NEON_FCVTPS = NEON2RegMiscFixed | 0x0081A000,
  NEON_FCVTPU = NEON_FCVTPS | NEON2RegMiscUBit,
  NEON_FCVTMS = NEON2RegMiscFixed | 0x0001B000,
  NEON_FCVTMU = NEON_FCVTMS | NEON2RegMiscUBit,
  NEON_FCVTZS = NEON2RegMiscFixed | 0x0081B000,
  NEON_FCVTZU = NEON_FCVTZS | NEON2RegMiscUBit,
  NEON_FCVTAS = NEON2RegMiscFixed | 0x0001C000,
  NEON_FCVTAU = NEON_FCVTAS | NEON2RegMiscUBit,
  NEON_FSQRT  = NEON2RegMiscFixed | 0x2081F000,
  NEON_SCVTF  = NEON2RegMiscFixed | 0x0001D000,
  NEON_UCVTF  = NEON_SCVTF | NEON2RegMiscUBit,
  NEON_URSQRTE = NEON2RegMiscFixed | 0x2081C000,
  NEON_URECPE  = NEON2RegMiscFixed | 0x0081C000,
  NEON_FRSQRTE = NEON2RegMiscFixed | 0x2081D000,
  NEON_FRECPE  = NEON2RegMiscFixed | 0x0081D000,
  NEON_FCMGT_zero = NEON2RegMiscFixed | 0x0080C000,
  NEON_FCMGE_zero = NEON2RegMiscFixed | 0x2080C000,
  NEON_FCMEQ_zero = NEON2RegMiscFixed | 0x0080D000,
  NEON_FCMLE_zero = NEON2RegMiscFixed | 0x2080D000,
  NEON_FCMLT_zero = NEON2RegMiscFixed | 0x0080E000,

  NEON_FCVTL_opcode = NEON_FCVTL & NEON2RegMiscOpcode,
  NEON_FCVTN_opcode = NEON_FCVTN & NEON2RegMiscOpcode
};

// NEON instructions with two register operands (FP16).
enum NEON2RegMiscFP16Op {
  NEON2RegMiscFP16Fixed = 0x0E780800,
  NEON2RegMiscFP16FMask = 0x9F7E0C00,
  NEON2RegMiscFP16Mask  = 0xBFFFFC00,
  NEON_FRINTN_H     = NEON2RegMiscFP16Fixed | 0x00018000,
  NEON_FRINTM_H     = NEON2RegMiscFP16Fixed | 0x00019000,
  NEON_FCVTNS_H     = NEON2RegMiscFP16Fixed | 0x0001A000,
  NEON_FCVTMS_H     = NEON2RegMiscFP16Fixed | 0x0001B000,
  NEON_FCVTAS_H     = NEON2RegMiscFP16Fixed | 0x0001C000,
  NEON_SCVTF_H      = NEON2RegMiscFP16Fixed | 0x0001D000,
  NEON_FCMGT_H_zero = NEON2RegMiscFP16Fixed | 0x0080C000,
  NEON_FCMEQ_H_zero = NEON2RegMiscFP16Fixed | 0x0080D000,
  NEON_FCMLT_H_zero = NEON2RegMiscFP16Fixed | 0x0080E000,
  NEON_FABS_H       = NEON2RegMiscFP16Fixed | 0x0080F000,
  NEON_FRINTP_H     = NEON2RegMiscFP16Fixed | 0x00818000,
  NEON_FRINTZ_H     = NEON2RegMiscFP16Fixed | 0x00819000,
  NEON_FCVTPS_H     = NEON2RegMiscFP16Fixed | 0x0081A000,
  NEON_FCVTZS_H     = NEON2RegMiscFP16Fixed | 0x0081B000,
  NEON_FRECPE_H     = NEON2RegMiscFP16Fixed | 0x0081D000,
  NEON_FRINTA_H     = NEON2RegMiscFP16Fixed | 0x20018000,
  NEON_FRINTX_H     = NEON2RegMiscFP16Fixed | 0x20019000,
  NEON_FCVTNU_H     = NEON2RegMiscFP16Fixed | 0x2001A000,
  NEON_FCVTMU_H     = NEON2RegMiscFP16Fixed | 0x2001B000,
  NEON_FCVTAU_H     = NEON2RegMiscFP16Fixed | 0x2001C000,
  NEON_UCVTF_H      = NEON2RegMiscFP16Fixed | 0x2001D000,
  NEON_FCMGE_H_zero = NEON2RegMiscFP16Fixed | 0x2080C000,
  NEON_FCMLE_H_zero = NEON2RegMiscFP16Fixed | 0x2080D000,
  NEON_FNEG_H       = NEON2RegMiscFP16Fixed | 0x2080F000,
  NEON_FRINTI_H     = NEON2RegMiscFP16Fixed | 0x20819000,
  NEON_FCVTPU_H     = NEON2RegMiscFP16Fixed | 0x2081A000,
  NEON_FCVTZU_H     = NEON2RegMiscFP16Fixed | 0x2081B000,
  NEON_FRSQRTE_H    = NEON2RegMiscFP16Fixed | 0x2081D000,
  NEON_FSQRT_H      = NEON2RegMiscFP16Fixed | 0x2081F000
};

// NEON instructions with three same-type operands.
enum NEON3SameOp {
  NEON3SameFixed = 0x0E200400,
  NEON3SameFMask = 0x9F200400,
  NEON3SameMask =  0xBF20FC00,
  NEON3SameUBit =  0x20000000,
  NEON_ADD    = NEON3SameFixed | 0x00008000,
  NEON_ADDP   = NEON3SameFixed | 0x0000B800,
  NEON_SHADD  = NEON3SameFixed | 0x00000000,
  NEON_SHSUB  = NEON3SameFixed | 0x00002000,
  NEON_SRHADD = NEON3SameFixed | 0x00001000,
  NEON_CMEQ   = NEON3SameFixed | NEON3SameUBit | 0x00008800,
  NEON_CMGE   = NEON3SameFixed | 0x00003800,
  NEON_CMGT   = NEON3SameFixed | 0x00003000,
  NEON_CMHI   = NEON3SameFixed | NEON3SameUBit | NEON_CMGT,
  NEON_CMHS   = NEON3SameFixed | NEON3SameUBit | NEON_CMGE,
  NEON_CMTST  = NEON3SameFixed | 0x00008800,
  NEON_MLA    = NEON3SameFixed | 0x00009000,
  NEON_MLS    = NEON3SameFixed | 0x20009000,
  NEON_MUL    = NEON3SameFixed | 0x00009800,
  NEON_PMUL   = NEON3SameFixed | 0x20009800,
  NEON_SRSHL  = NEON3SameFixed | 0x00005000,
  NEON_SQSHL  = NEON3SameFixed | 0x00004800,
  NEON_SQRSHL = NEON3SameFixed | 0x00005800,
  NEON_SSHL   = NEON3SameFixed | 0x00004000,
  NEON_SMAX   = NEON3SameFixed | 0x00006000,
  NEON_SMAXP  = NEON3SameFixed | 0x0000A000,
  NEON_SMIN   = NEON3SameFixed | 0x00006800,
  NEON_SMINP  = NEON3SameFixed | 0x0000A800,
  NEON_SABD   = NEON3SameFixed | 0x00007000,
  NEON_SABA   = NEON3SameFixed | 0x00007800,
  NEON_UABD   = NEON3SameFixed | NEON3SameUBit | NEON_SABD,
  NEON_UABA   = NEON3SameFixed | NEON3SameUBit | NEON_SABA,
  NEON_SQADD  = NEON3SameFixed | 0x00000800,
  NEON_SQSUB  = NEON3SameFixed | 0x00002800,
  NEON_SUB    = NEON3SameFixed | NEON3SameUBit | 0x00008000,
  NEON_UHADD  = NEON3SameFixed | NEON3SameUBit | NEON_SHADD,
  NEON_UHSUB  = NEON3SameFixed | NEON3SameUBit | NEON_SHSUB,
  NEON_URHADD = NEON3SameFixed | NEON3SameUBit | NEON_SRHADD,
  NEON_UMAX   = NEON3SameFixed | NEON3SameUBit | NEON_SMAX,
  NEON_UMAXP  = NEON3SameFixed | NEON3SameUBit | NEON_SMAXP,
  NEON_UMIN   = NEON3SameFixed | NEON3SameUBit | NEON_SMIN,
  NEON_UMINP  = NEON3SameFixed | NEON3SameUBit | NEON_SMINP,
  NEON_URSHL  = NEON3SameFixed | NEON3SameUBit | NEON_SRSHL,
  NEON_UQADD  = NEON3SameFixed | NEON3SameUBit | NEON_SQADD,
  NEON_UQRSHL = NEON3SameFixed | NEON3SameUBit | NEON_SQRSHL,
  NEON_UQSHL  = NEON3SameFixed | NEON3SameUBit | NEON_SQSHL,
  NEON_UQSUB  = NEON3SameFixed | NEON3SameUBit | NEON_SQSUB,
  NEON_USHL   = NEON3SameFixed | NEON3SameUBit | NEON_SSHL,
  NEON_SQDMULH  = NEON3SameFixed | 0x0000B000,
  NEON_SQRDMULH = NEON3SameFixed | 0x2000B000,

  // NEON floating point instructions with three same-type operands.
  NEON3SameFPFixed = NEON3SameFixed | 0x0000C000,
  NEON3SameFPFMask = NEON3SameFMask | 0x0000C000,
  NEON3SameFPMask = NEON3SameMask | 0x00800000,
  NEON_FADD    = NEON3SameFixed | 0x0000D000,
  NEON_FSUB    = NEON3SameFixed | 0x0080D000,
  NEON_FMUL    = NEON3SameFixed | 0x2000D800,
  NEON_FDIV    = NEON3SameFixed | 0x2000F800,
  NEON_FMAX    = NEON3SameFixed | 0x0000F000,
  NEON_FMAXNM  = NEON3SameFixed | 0x0000C000,
  NEON_FMAXP   = NEON3SameFixed | 0x2000F000,
  NEON_FMAXNMP = NEON3SameFixed | 0x2000C000,
  NEON_FMIN    = NEON3SameFixed | 0x0080F000,
  NEON_FMINNM  = NEON3SameFixed | 0x0080C000,
  NEON_FMINP   = NEON3SameFixed | 0x2080F000,
  NEON_FMINNMP = NEON3SameFixed | 0x2080C000,
  NEON_FMLA    = NEON3SameFixed | 0x0000C800,
  NEON_FMLS    = NEON3SameFixed | 0x0080C800,
  NEON_FMULX   = NEON3SameFixed | 0x0000D800,
  NEON_FRECPS  = NEON3SameFixed | 0x0000F800,
  NEON_FRSQRTS = NEON3SameFixed | 0x0080F800,
  NEON_FABD    = NEON3SameFixed | 0x2080D000,
  NEON_FADDP   = NEON3SameFixed | 0x2000D000,
  NEON_FCMEQ   = NEON3SameFixed | 0x0000E000,
  NEON_FCMGE   = NEON3SameFixed | 0x2000E000,
  NEON_FCMGT   = NEON3SameFixed | 0x2080E000,
  NEON_FACGE   = NEON3SameFixed | 0x2000E800,
  NEON_FACGT   = NEON3SameFixed | 0x2080E800,

  // NEON logical instructions with three same-type operands.
  NEON3SameLogicalFixed = NEON3SameFixed | 0x00001800,
  NEON3SameLogicalFMask = NEON3SameFMask | 0x0000F800,
  NEON3SameLogicalMask = 0xBFE0FC00,
  NEON3SameLogicalFormatMask = NEON_Q,
  NEON_AND = NEON3SameLogicalFixed | 0x00000000,
  NEON_ORR = NEON3SameLogicalFixed | 0x00A00000,
  NEON_ORN = NEON3SameLogicalFixed | 0x00C00000,
  NEON_EOR = NEON3SameLogicalFixed | 0x20000000,
  NEON_BIC = NEON3SameLogicalFixed | 0x00400000,
  NEON_BIF = NEON3SameLogicalFixed | 0x20C00000,
  NEON_BIT = NEON3SameLogicalFixed | 0x20800000,
  NEON_BSL = NEON3SameLogicalFixed | 0x20400000,

  // FHM (FMLAL-like) instructions have an oddball encoding scheme under 3Same.
  NEON3SameFHMMask = 0xBFE0FC00,                // U  size  opcode
  NEON_FMLAL   = NEON3SameFixed | 0x0000E800,   // 0    00   11101
  NEON_FMLAL2  = NEON3SameFixed | 0x2000C800,   // 1    00   11001
  NEON_FMLSL   = NEON3SameFixed | 0x0080E800,   // 0    10   11101
  NEON_FMLSL2  = NEON3SameFixed | 0x2080C800    // 1    10   11001
};


enum NEON3SameFP16 {
  NEON3SameFP16Fixed = 0x0E400400,
  NEON3SameFP16FMask = 0x9F60C400,
  NEON3SameFP16Mask =  0xBFE0FC00,
  NEON_FMAXNM_H  = NEON3SameFP16Fixed | 0x00000000,
  NEON_FMLA_H    = NEON3SameFP16Fixed | 0x00000800,
  NEON_FADD_H    = NEON3SameFP16Fixed | 0x00001000,
  NEON_FMULX_H   = NEON3SameFP16Fixed | 0x00001800,
  NEON_FCMEQ_H   = NEON3SameFP16Fixed | 0x00002000,
  NEON_FMAX_H    = NEON3SameFP16Fixed | 0x00003000,
  NEON_FRECPS_H  = NEON3SameFP16Fixed | 0x00003800,
  NEON_FMINNM_H  = NEON3SameFP16Fixed | 0x00800000,
  NEON_FMLS_H    = NEON3SameFP16Fixed | 0x00800800,
  NEON_FSUB_H    = NEON3SameFP16Fixed | 0x00801000,
  NEON_FMIN_H    = NEON3SameFP16Fixed | 0x00803000,
  NEON_FRSQRTS_H = NEON3SameFP16Fixed | 0x00803800,
  NEON_FMAXNMP_H = NEON3SameFP16Fixed | 0x20000000,
  NEON_FADDP_H   = NEON3SameFP16Fixed | 0x20001000,
  NEON_FMUL_H    = NEON3SameFP16Fixed | 0x20001800,
  NEON_FCMGE_H   = NEON3SameFP16Fixed | 0x20002000,
  NEON_FACGE_H   = NEON3SameFP16Fixed | 0x20002800,
  NEON_FMAXP_H   = NEON3SameFP16Fixed | 0x20003000,
  NEON_FDIV_H    = NEON3SameFP16Fixed | 0x20003800,
  NEON_FMINNMP_H = NEON3SameFP16Fixed | 0x20800000,
  NEON_FABD_H    = NEON3SameFP16Fixed | 0x20801000,
  NEON_FCMGT_H   = NEON3SameFP16Fixed | 0x20802000,
  NEON_FACGT_H   = NEON3SameFP16Fixed | 0x20802800,
  NEON_FMINP_H   = NEON3SameFP16Fixed | 0x20803000
};


// 'Extra' NEON instructions with three same-type operands.
enum NEON3SameExtraOp {
  NEON3SameExtraFixed = 0x0E008400,
  NEON3SameExtraUBit = 0x20000000,
  NEON3SameExtraFMask = 0x9E208400,
  NEON3SameExtraMask = 0xBE20FC00,
  NEON_SQRDMLAH = NEON3SameExtraFixed | NEON3SameExtraUBit,
  NEON_SQRDMLSH = NEON3SameExtraFixed | NEON3SameExtraUBit | 0x00000800,
  NEON_SDOT = NEON3SameExtraFixed | 0x00001000,
  NEON_UDOT = NEON3SameExtraFixed | NEON3SameExtraUBit | 0x00001000,

  /* v8.3 Complex Numbers */
  NEON3SameExtraFCFixed = 0x2E00C400,
  NEON3SameExtraFCFMask = 0xBF20C400,
  // FCMLA fixes opcode<3:2>, and uses opcode<1:0> to encode <rotate>.
  NEON3SameExtraFCMLAMask = NEON3SameExtraFCFMask | 0x00006000,
  NEON_FCMLA = NEON3SameExtraFCFixed,
  // FCADD fixes opcode<3:2, 0>, and uses opcode<1> to encode <rotate>.
  NEON3SameExtraFCADDMask = NEON3SameExtraFCFMask | 0x00006800,
  NEON_FCADD = NEON3SameExtraFCFixed | 0x00002000
  // Other encodings under NEON3SameExtraFCFMask are UNALLOCATED.
};

// NEON instructions with three different-type operands.
enum NEON3DifferentOp {
  NEON3DifferentFixed = 0x0E200000,
  NEON3DifferentFMask = 0x9F200C00,
  NEON3DifferentMask  = 0xFF20FC00,
  NEON_ADDHN    = NEON3DifferentFixed | 0x00004000,
  NEON_ADDHN2   = NEON_ADDHN | NEON_Q,
  NEON_PMULL    = NEON3DifferentFixed | 0x0000E000,
  NEON_PMULL2   = NEON_PMULL | NEON_Q,
  NEON_RADDHN   = NEON3DifferentFixed | 0x20004000,
  NEON_RADDHN2  = NEON_RADDHN | NEON_Q,
  NEON_RSUBHN   = NEON3DifferentFixed | 0x20006000,
  NEON_RSUBHN2  = NEON_RSUBHN | NEON_Q,
  NEON_SABAL    = NEON3DifferentFixed | 0x00005000,
  NEON_SABAL2   = NEON_SABAL | NEON_Q,
  NEON_SABDL    = NEON3DifferentFixed | 0x00007000,
  NEON_SABDL2   = NEON_SABDL | NEON_Q,
  NEON_SADDL    = NEON3DifferentFixed | 0x00000000,
  NEON_SADDL2   = NEON_SADDL | NEON_Q,
  NEON_SADDW    = NEON3DifferentFixed | 0x00001000,
  NEON_SADDW2   = NEON_SADDW | NEON_Q,
  NEON_SMLAL    = NEON3DifferentFixed | 0x00008000,
  NEON_SMLAL2   = NEON_SMLAL | NEON_Q,
  NEON_SMLSL    = NEON3DifferentFixed | 0x0000A000,
  NEON_SMLSL2   = NEON_SMLSL | NEON_Q,
  NEON_SMULL    = NEON3DifferentFixed | 0x0000C000,
  NEON_SMULL2   = NEON_SMULL | NEON_Q,
  NEON_SSUBL    = NEON3DifferentFixed | 0x00002000,
  NEON_SSUBL2   = NEON_SSUBL | NEON_Q,
  NEON_SSUBW    = NEON3DifferentFixed | 0x00003000,
  NEON_SSUBW2   = NEON_SSUBW | NEON_Q,
  NEON_SQDMLAL  = NEON3DifferentFixed | 0x00009000,
  NEON_SQDMLAL2 = NEON_SQDMLAL | NEON_Q,
  NEON_SQDMLSL  = NEON3DifferentFixed | 0x0000B000,
  NEON_SQDMLSL2 = NEON_SQDMLSL | NEON_Q,
  NEON_SQDMULL  = NEON3DifferentFixed | 0x0000D000,
  NEON_SQDMULL2 = NEON_SQDMULL | NEON_Q,
  NEON_SUBHN    = NEON3DifferentFixed | 0x00006000,
  NEON_SUBHN2   = NEON_SUBHN | NEON_Q,
  NEON_UABAL    = NEON_SABAL | NEON3SameUBit,
  NEON_UABAL2   = NEON_UABAL | NEON_Q,
  NEON_UABDL    = NEON_SABDL | NEON3SameUBit,
  NEON_UABDL2   = NEON_UABDL | NEON_Q,
  NEON_UADDL    = NEON_SADDL | NEON3SameUBit,
  NEON_UADDL2   = NEON_UADDL | NEON_Q,
  NEON_UADDW    = NEON_SADDW | NEON3SameUBit,
  NEON_UADDW2   = NEON_UADDW | NEON_Q,
  NEON_UMLAL    = NEON_SMLAL | NEON3SameUBit,
  NEON_UMLAL2   = NEON_UMLAL | NEON_Q,
  NEON_UMLSL    = NEON_SMLSL | NEON3SameUBit,
  NEON_UMLSL2   = NEON_UMLSL | NEON_Q,
  NEON_UMULL    = NEON_SMULL | NEON3SameUBit,
  NEON_UMULL2   = NEON_UMULL | NEON_Q,
  NEON_USUBL    = NEON_SSUBL | NEON3SameUBit,
  NEON_USUBL2   = NEON_USUBL | NEON_Q,
  NEON_USUBW    = NEON_SSUBW | NEON3SameUBit,
  NEON_USUBW2   = NEON_USUBW | NEON_Q
};

// NEON instructions operating across vectors.
enum NEONAcrossLanesOp {
  NEONAcrossLanesFixed = 0x0E300800,
  NEONAcrossLanesFMask = 0x9F3E0C00,
  NEONAcrossLanesMask  = 0xBF3FFC00,
  NEON_ADDV   = NEONAcrossLanesFixed | 0x0001B000,
  NEON_SADDLV = NEONAcrossLanesFixed | 0x00003000,
  NEON_UADDLV = NEONAcrossLanesFixed | 0x20003000,
  NEON_SMAXV  = NEONAcrossLanesFixed | 0x0000A000,
  NEON_SMINV  = NEONAcrossLanesFixed | 0x0001A000,
  NEON_UMAXV  = NEONAcrossLanesFixed | 0x2000A000,
  NEON_UMINV  = NEONAcrossLanesFixed | 0x2001A000,

  NEONAcrossLanesFP16Fixed = NEONAcrossLanesFixed | 0x0000C000,
  NEONAcrossLanesFP16FMask = NEONAcrossLanesFMask | 0x2000C000,
  NEONAcrossLanesFP16Mask  = NEONAcrossLanesMask  | 0x20800000,
  NEON_FMAXNMV_H = NEONAcrossLanesFP16Fixed | 0x00000000,
  NEON_FMAXV_H   = NEONAcrossLanesFP16Fixed | 0x00003000,
  NEON_FMINNMV_H = NEONAcrossLanesFP16Fixed | 0x00800000,
  NEON_FMINV_H   = NEONAcrossLanesFP16Fixed | 0x00803000,

  // NEON floating point across instructions.
  NEONAcrossLanesFPFixed = NEONAcrossLanesFixed | 0x2000C000,
  NEONAcrossLanesFPFMask = NEONAcrossLanesFMask | 0x2000C000,
  NEONAcrossLanesFPMask  = NEONAcrossLanesMask  | 0x20800000,

  NEON_FMAXV   = NEONAcrossLanesFPFixed | 0x2000F000,
  NEON_FMINV   = NEONAcrossLanesFPFixed | 0x2080F000,
  NEON_FMAXNMV = NEONAcrossLanesFPFixed | 0x2000C000,
  NEON_FMINNMV = NEONAcrossLanesFPFixed | 0x2080C000
};

// NEON instructions with indexed element operand.
enum NEONByIndexedElementOp {
  NEONByIndexedElementFixed = 0x0F000000,
  NEONByIndexedElementFMask = 0x9F000400,
  NEONByIndexedElementMask  = 0xBF00F400,
  NEON_MUL_byelement   = NEONByIndexedElementFixed | 0x00008000,
  NEON_MLA_byelement   = NEONByIndexedElementFixed | 0x20000000,
  NEON_MLS_byelement   = NEONByIndexedElementFixed | 0x20004000,
  NEON_SMULL_byelement = NEONByIndexedElementFixed | 0x0000A000,
  NEON_SMLAL_byelement = NEONByIndexedElementFixed | 0x00002000,
  NEON_SMLSL_byelement = NEONByIndexedElementFixed | 0x00006000,
  NEON_UMULL_byelement = NEONByIndexedElementFixed | 0x2000A000,
  NEON_UMLAL_byelement = NEONByIndexedElementFixed | 0x20002000,
  NEON_UMLSL_byelement = NEONByIndexedElementFixed | 0x20006000,
  NEON_SQDMULL_byelement = NEONByIndexedElementFixed | 0x0000B000,
  NEON_SQDMLAL_byelement = NEONByIndexedElementFixed | 0x00003000,
  NEON_SQDMLSL_byelement = NEONByIndexedElementFixed | 0x00007000,
  NEON_SQDMULH_byelement  = NEONByIndexedElementFixed | 0x0000C000,
  NEON_SQRDMULH_byelement = NEONByIndexedElementFixed | 0x0000D000,
  NEON_SDOT_byelement = NEONByIndexedElementFixed | 0x0000E000,
  NEON_SQRDMLAH_byelement = NEONByIndexedElementFixed | 0x2000D000,
  NEON_UDOT_byelement = NEONByIndexedElementFixed | 0x2000E000,
  NEON_SQRDMLSH_byelement = NEONByIndexedElementFixed | 0x2000F000,

  NEON_FMLA_H_byelement   = NEONByIndexedElementFixed | 0x00001000,
  NEON_FMLS_H_byelement   = NEONByIndexedElementFixed | 0x00005000,
  NEON_FMUL_H_byelement   = NEONByIndexedElementFixed | 0x00009000,
  NEON_FMULX_H_byelement  = NEONByIndexedElementFixed | 0x20009000,

  // Floating point instructions.
  NEONByIndexedElementFPFixed = NEONByIndexedElementFixed | 0x00800000,
  NEONByIndexedElementFPMask = NEONByIndexedElementMask | 0x00800000,
  NEON_FMLA_byelement  = NEONByIndexedElementFPFixed | 0x00001000,
  NEON_FMLS_byelement  = NEONByIndexedElementFPFixed | 0x00005000,
  NEON_FMUL_byelement  = NEONByIndexedElementFPFixed | 0x00009000,
  NEON_FMULX_byelement = NEONByIndexedElementFPFixed | 0x20009000,

  // FMLAL-like instructions.
  // For all cases: U = x, size = 10, opcode = xx00
  NEONByIndexedElementFPLongFixed = NEONByIndexedElementFixed | 0x00800000,
  NEONByIndexedElementFPLongFMask = NEONByIndexedElementFMask | 0x00C03000,
  NEONByIndexedElementFPLongMask = 0xBFC0F400,
  NEON_FMLAL_H_byelement  = NEONByIndexedElementFixed | 0x00800000,
  NEON_FMLAL2_H_byelement = NEONByIndexedElementFixed | 0x20808000,
  NEON_FMLSL_H_byelement  = NEONByIndexedElementFixed | 0x00804000,
  NEON_FMLSL2_H_byelement = NEONByIndexedElementFixed | 0x2080C000,

  // Complex instruction(s).
  // This is necessary because the 'rot' encoding moves into the
  // NEONByIndex..Mask space.
  NEONByIndexedElementFPComplexMask = 0xBF009400,
  NEON_FCMLA_byelement = NEONByIndexedElementFixed | 0x20001000
};

// NEON register copy.
enum NEONCopyOp {
  NEONCopyFixed = 0x0E000400,
  NEONCopyFMask = 0x9FE08400,
  NEONCopyMask  = 0x3FE08400,
  NEONCopyInsElementMask = NEONCopyMask | 0x40000000,
  NEONCopyInsGeneralMask = NEONCopyMask | 0x40007800,
  NEONCopyDupElementMask = NEONCopyMask | 0x20007800,
  NEONCopyDupGeneralMask = NEONCopyDupElementMask,
  NEONCopyUmovMask       = NEONCopyMask | 0x20007800,
  NEONCopySmovMask       = NEONCopyMask | 0x20007800,
  NEON_INS_ELEMENT       = NEONCopyFixed | 0x60000000,
  NEON_INS_GENERAL       = NEONCopyFixed | 0x40001800,
  NEON_DUP_ELEMENT       = NEONCopyFixed | 0x00000000,
  NEON_DUP_GENERAL       = NEONCopyFixed | 0x00000800,
  NEON_SMOV              = NEONCopyFixed | 0x00002800,
  NEON_UMOV              = NEONCopyFixed | 0x00003800
};

// NEON extract.
enum NEONExtractOp {
  NEONExtractFixed = 0x2E000000,
  NEONExtractFMask = 0xBF208400,
  NEONExtractMask =  0xBFE08400,
  NEON_EXT = NEONExtractFixed | 0x00000000
};

enum NEONLoadStoreMultiOp {
  NEONLoadStoreMultiL    = 0x00400000,
  NEONLoadStoreMulti1_1v = 0x00007000,
  NEONLoadStoreMulti1_2v = 0x0000A000,
  NEONLoadStoreMulti1_3v = 0x00006000,
  NEONLoadStoreMulti1_4v = 0x00002000,
  NEONLoadStoreMulti2    = 0x00008000,
  NEONLoadStoreMulti3    = 0x00004000,
  NEONLoadStoreMulti4    = 0x00000000
};

// NEON load/store multiple structures.
enum NEONLoadStoreMultiStructOp {
  NEONLoadStoreMultiStructFixed = 0x0C000000,
  NEONLoadStoreMultiStructFMask = 0xBFBF0000,
  NEONLoadStoreMultiStructMask  = 0xBFFFF000,
  NEONLoadStoreMultiStructStore = NEONLoadStoreMultiStructFixed,
  NEONLoadStoreMultiStructLoad  = NEONLoadStoreMultiStructFixed |
                                  NEONLoadStoreMultiL,
  NEON_LD1_1v = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti1_1v,
  NEON_LD1_2v = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti1_2v,
  NEON_LD1_3v = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti1_3v,
  NEON_LD1_4v = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti1_4v,
  NEON_LD2    = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti2,
  NEON_LD3    = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti3,
  NEON_LD4    = NEONLoadStoreMultiStructLoad | NEONLoadStoreMulti4,
  NEON_ST1_1v = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti1_1v,
  NEON_ST1_2v = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti1_2v,
  NEON_ST1_3v = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti1_3v,
  NEON_ST1_4v = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti1_4v,
  NEON_ST2    = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti2,
  NEON_ST3    = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti3,
  NEON_ST4    = NEONLoadStoreMultiStructStore | NEONLoadStoreMulti4
};

// NEON load/store multiple structures with post-index addressing.
enum NEONLoadStoreMultiStructPostIndexOp {
  NEONLoadStoreMultiStructPostIndexFixed = 0x0C800000,
  NEONLoadStoreMultiStructPostIndexFMask = 0xBFA00000,
  NEONLoadStoreMultiStructPostIndexMask  = 0xBFE0F000,
  NEONLoadStoreMultiStructPostIndex = 0x00800000,
  NEON_LD1_1v_post = NEON_LD1_1v | NEONLoadStoreMultiStructPostIndex,
  NEON_LD1_2v_post = NEON_LD1_2v | NEONLoadStoreMultiStructPostIndex,
  NEON_LD1_3v_post = NEON_LD1_3v | NEONLoadStoreMultiStructPostIndex,
  NEON_LD1_4v_post = NEON_LD1_4v | NEONLoadStoreMultiStructPostIndex,
  NEON_LD2_post = NEON_LD2 | NEONLoadStoreMultiStructPostIndex,
  NEON_LD3_post = NEON_LD3 | NEONLoadStoreMultiStructPostIndex,
  NEON_LD4_post = NEON_LD4 | NEONLoadStoreMultiStructPostIndex,
  NEON_ST1_1v_post = NEON_ST1_1v | NEONLoadStoreMultiStructPostIndex,
  NEON_ST1_2v_post = NEON_ST1_2v | NEONLoadStoreMultiStructPostIndex,
  NEON_ST1_3v_post = NEON_ST1_3v | NEONLoadStoreMultiStructPostIndex,
  NEON_ST1_4v_post = NEON_ST1_4v | NEONLoadStoreMultiStructPostIndex,
  NEON_ST2_post = NEON_ST2 | NEONLoadStoreMultiStructPostIndex,
  NEON_ST3_post = NEON_ST3 | NEONLoadStoreMultiStructPostIndex,
  NEON_ST4_post = NEON_ST4 | NEONLoadStoreMultiStructPostIndex
};

enum NEONLoadStoreSingleOp {
  NEONLoadStoreSingle1        = 0x00000000,
  NEONLoadStoreSingle2        = 0x00200000,
  NEONLoadStoreSingle3        = 0x00002000,
  NEONLoadStoreSingle4        = 0x00202000,
  NEONLoadStoreSingleL        = 0x00400000,
  NEONLoadStoreSingle_b       = 0x00000000,
  NEONLoadStoreSingle_h       = 0x00004000,
  NEONLoadStoreSingle_s       = 0x00008000,
  NEONLoadStoreSingle_d       = 0x00008400,
  NEONLoadStoreSingleAllLanes = 0x0000C000,
  NEONLoadStoreSingleLenMask  = 0x00202000
};

// NEON load/store single structure.
enum NEONLoadStoreSingleStructOp {
  NEONLoadStoreSingleStructFixed = 0x0D000000,
  NEONLoadStoreSingleStructFMask = 0xBF9F0000,
  NEONLoadStoreSingleStructMask  = 0xBFFFE000,
  NEONLoadStoreSingleStructStore = NEONLoadStoreSingleStructFixed,
  NEONLoadStoreSingleStructLoad  = NEONLoadStoreSingleStructFixed |
                                   NEONLoadStoreSingleL,
  NEONLoadStoreSingleStructLoad1 = NEONLoadStoreSingle1 |
                                   NEONLoadStoreSingleStructLoad,
  NEONLoadStoreSingleStructLoad2 = NEONLoadStoreSingle2 |
                                   NEONLoadStoreSingleStructLoad,
  NEONLoadStoreSingleStructLoad3 = NEONLoadStoreSingle3 |
                                   NEONLoadStoreSingleStructLoad,
  NEONLoadStoreSingleStructLoad4 = NEONLoadStoreSingle4 |
                                   NEONLoadStoreSingleStructLoad,
  NEONLoadStoreSingleStructStore1 = NEONLoadStoreSingle1 |
                                    NEONLoadStoreSingleStructFixed,
  NEONLoadStoreSingleStructStore2 = NEONLoadStoreSingle2 |
                                    NEONLoadStoreSingleStructFixed,
  NEONLoadStoreSingleStructStore3 = NEONLoadStoreSingle3 |
                                    NEONLoadStoreSingleStructFixed,
  NEONLoadStoreSingleStructStore4 = NEONLoadStoreSingle4 |
                                    NEONLoadStoreSingleStructFixed,
  NEON_LD1_b = NEONLoadStoreSingleStructLoad1 | NEONLoadStoreSingle_b,
  NEON_LD1_h = NEONLoadStoreSingleStructLoad1 | NEONLoadStoreSingle_h,
  NEON_LD1_s = NEONLoadStoreSingleStructLoad1 | NEONLoadStoreSingle_s,
  NEON_LD1_d = NEONLoadStoreSingleStructLoad1 | NEONLoadStoreSingle_d,
  NEON_LD1R  = NEONLoadStoreSingleStructLoad1 | NEONLoadStoreSingleAllLanes,
  NEON_ST1_b = NEONLoadStoreSingleStructStore1 | NEONLoadStoreSingle_b,
  NEON_ST1_h = NEONLoadStoreSingleStructStore1 | NEONLoadStoreSingle_h,
  NEON_ST1_s = NEONLoadStoreSingleStructStore1 | NEONLoadStoreSingle_s,
  NEON_ST1_d = NEONLoadStoreSingleStructStore1 | NEONLoadStoreSingle_d,

  NEON_LD2_b = NEONLoadStoreSingleStructLoad2 | NEONLoadStoreSingle_b,
  NEON_LD2_h = NEONLoadStoreSingleStructLoad2 | NEONLoadStoreSingle_h,
  NEON_LD2_s = NEONLoadStoreSingleStructLoad2 | NEONLoadStoreSingle_s,
  NEON_LD2_d = NEONLoadStoreSingleStructLoad2 | NEONLoadStoreSingle_d,
  NEON_LD2R  = NEONLoadStoreSingleStructLoad2 | NEONLoadStoreSingleAllLanes,
  NEON_ST2_b = NEONLoadStoreSingleStructStore2 | NEONLoadStoreSingle_b,
  NEON_ST2_h = NEONLoadStoreSingleStructStore2 | NEONLoadStoreSingle_h,
  NEON_ST2_s = NEONLoadStoreSingleStructStore2 | NEONLoadStoreSingle_s,
  NEON_ST2_d = NEONLoadStoreSingleStructStore2 | NEONLoadStoreSingle_d,

  NEON_LD3_b = NEONLoadStoreSingleStructLoad3 | NEONLoadStoreSingle_b,
  NEON_LD3_h = NEONLoadStoreSingleStructLoad3 | NEONLoadStoreSingle_h,
  NEON_LD3_s = NEONLoadStoreSingleStructLoad3 | NEONLoadStoreSingle_s,
  NEON_LD3_d = NEONLoadStoreSingleStructLoad3 | NEONLoadStoreSingle_d,
  NEON_LD3R  = NEONLoadStoreSingleStructLoad3 | NEONLoadStoreSingleAllLanes,
  NEON_ST3_b = NEONLoadStoreSingleStructStore3 | NEONLoadStoreSingle_b,
  NEON_ST3_h = NEONLoadStoreSingleStructStore3 | NEONLoadStoreSingle_h,
  NEON_ST3_s = NEONLoadStoreSingleStructStore3 | NEONLoadStoreSingle_s,
  NEON_ST3_d = NEONLoadStoreSingleStructStore3 | NEONLoadStoreSingle_d,

  NEON_LD4_b = NEONLoadStoreSingleStructLoad4 | NEONLoadStoreSingle_b,
  NEON_LD4_h = NEONLoadStoreSingleStructLoad4 | NEONLoadStoreSingle_h,
  NEON_LD4_s = NEONLoadStoreSingleStructLoad4 | NEONLoadStoreSingle_s,
  NEON_LD4_d = NEONLoadStoreSingleStructLoad4 | NEONLoadStoreSingle_d,
  NEON_LD4R  = NEONLoadStoreSingleStructLoad4 | NEONLoadStoreSingleAllLanes,
  NEON_ST4_b = NEONLoadStoreSingleStructStore4 | NEONLoadStoreSingle_b,
  NEON_ST4_h = NEONLoadStoreSingleStructStore4 | NEONLoadStoreSingle_h,
  NEON_ST4_s = NEONLoadStoreSingleStructStore4 | NEONLoadStoreSingle_s,
  NEON_ST4_d = NEONLoadStoreSingleStructStore4 | NEONLoadStoreSingle_d
};

// NEON load/store single structure with post-index addressing.
enum NEONLoadStoreSingleStructPostIndexOp {
  NEONLoadStoreSingleStructPostIndexFixed = 0x0D800000,
  NEONLoadStoreSingleStructPostIndexFMask = 0xBF800000,
  NEONLoadStoreSingleStructPostIndexMask  = 0xBFE0E000,
  NEONLoadStoreSingleStructPostIndex =      0x00800000,
  NEON_LD1_b_post = NEON_LD1_b | NEONLoadStoreSingleStructPostIndex,
  NEON_LD1_h_post = NEON_LD1_h | NEONLoadStoreSingleStructPostIndex,
  NEON_LD1_s_post = NEON_LD1_s | NEONLoadStoreSingleStructPostIndex,
  NEON_LD1_d_post = NEON_LD1_d | NEONLoadStoreSingleStructPostIndex,
  NEON_LD1R_post  = NEON_LD1R | NEONLoadStoreSingleStructPostIndex,
  NEON_ST1_b_post = NEON_ST1_b | NEONLoadStoreSingleStructPostIndex,
  NEON_ST1_h_post = NEON_ST1_h | NEONLoadStoreSingleStructPostIndex,
  NEON_ST1_s_post = NEON_ST1_s | NEONLoadStoreSingleStructPostIndex,
  NEON_ST1_d_post = NEON_ST1_d | NEONLoadStoreSingleStructPostIndex,

  NEON_LD2_b_post = NEON_LD2_b | NEONLoadStoreSingleStructPostIndex,
  NEON_LD2_h_post = NEON_LD2_h | NEONLoadStoreSingleStructPostIndex,
  NEON_LD2_s_post = NEON_LD2_s | NEONLoadStoreSingleStructPostIndex,
  NEON_LD2_d_post = NEON_LD2_d | NEONLoadStoreSingleStructPostIndex,
  NEON_LD2R_post  = NEON_LD2R | NEONLoadStoreSingleStructPostIndex,
  NEON_ST2_b_post = NEON_ST2_b | NEONLoadStoreSingleStructPostIndex,
  NEON_ST2_h_post = NEON_ST2_h | NEONLoadStoreSingleStructPostIndex,
  NEON_ST2_s_post = NEON_ST2_s | NEONLoadStoreSingleStructPostIndex,
  NEON_ST2_d_post = NEON_ST2_d | NEONLoadStoreSingleStructPostIndex,

  NEON_LD3_b_post = NEON_LD3_b | NEONLoadStoreSingleStructPostIndex,
  NEON_LD3_h_post = NEON_LD3_h | NEONLoadStoreSingleStructPostIndex,
  NEON_LD3_s_post = NEON_LD3_s | NEONLoadStoreSingleStructPostIndex,
  NEON_LD3_d_post = NEON_LD3_d | NEONLoadStoreSingleStructPostIndex,
  NEON_LD3R_post  = NEON_LD3R | NEONLoadStoreSingleStructPostIndex,
  NEON_ST3_b_post = NEON_ST3_b | NEONLoadStoreSingleStructPostIndex,
  NEON_ST3_h_post = NEON_ST3_h | NEONLoadStoreSingleStructPostIndex,
  NEON_ST3_s_post = NEON_ST3_s | NEONLoadStoreSingleStructPostIndex,
  NEON_ST3_d_post = NEON_ST3_d | NEONLoadStoreSingleStructPostIndex,

  NEON_LD4_b_post = NEON_LD4_b | NEONLoadStoreSingleStructPostIndex,
  NEON_LD4_h_post = NEON_LD4_h | NEONLoadStoreSingleStructPostIndex,
  NEON_LD4_s_post = NEON_LD4_s | NEONLoadStoreSingleStructPostIndex,
  NEON_LD4_d_post = NEON_LD4_d | NEONLoadStoreSingleStructPostIndex,
  NEON_LD4R_post  = NEON_LD4R | NEONLoadStoreSingleStructPostIndex,
  NEON_ST4_b_post = NEON_ST4_b | NEONLoadStoreSingleStructPostIndex,
  NEON_ST4_h_post = NEON_ST4_h | NEONLoadStoreSingleStructPostIndex,
  NEON_ST4_s_post = NEON_ST4_s | NEONLoadStoreSingleStructPostIndex,
  NEON_ST4_d_post = NEON_ST4_d | NEONLoadStoreSingleStructPostIndex
};

// NEON modified immediate.
enum NEONModifiedImmediateOp {
  NEONModifiedImmediateFixed = 0x0F000400,
  NEONModifiedImmediateFMask = 0x9FF80400,
  NEONModifiedImmediateOpBit = 0x20000000,
  NEONModifiedImmediate_FMOV = NEONModifiedImmediateFixed | 0x00000800,
  NEONModifiedImmediate_MOVI = NEONModifiedImmediateFixed | 0x00000000,
  NEONModifiedImmediate_MVNI = NEONModifiedImmediateFixed | 0x20000000,
  NEONModifiedImmediate_ORR  = NEONModifiedImmediateFixed | 0x00001000,
  NEONModifiedImmediate_BIC  = NEONModifiedImmediateFixed | 0x20001000
};

// NEON shift immediate.
enum NEONShiftImmediateOp {
  NEONShiftImmediateFixed = 0x0F000400,
  NEONShiftImmediateFMask = 0x9F800400,
  NEONShiftImmediateMask  = 0xBF80FC00,
  NEONShiftImmediateUBit  = 0x20000000,
  NEON_SHL      = NEONShiftImmediateFixed | 0x00005000,
  NEON_SSHLL    = NEONShiftImmediateFixed | 0x0000A000,
  NEON_USHLL    = NEONShiftImmediateFixed | 0x2000A000,
  NEON_SLI      = NEONShiftImmediateFixed | 0x20005000,
  NEON_SRI      = NEONShiftImmediateFixed | 0x20004000,
  NEON_SHRN     = NEONShiftImmediateFixed | 0x00008000,
  NEON_RSHRN    = NEONShiftImmediateFixed | 0x00008800,
  NEON_UQSHRN   = NEONShiftImmediateFixed | 0x20009000,
  NEON_UQRSHRN  = NEONShiftImmediateFixed | 0x20009800,
  NEON_SQSHRN   = NEONShiftImmediateFixed | 0x00009000,
  NEON_SQRSHRN  = NEONShiftImmediateFixed | 0x00009800,
  NEON_SQSHRUN  = NEONShiftImmediateFixed | 0x20008000,
  NEON_SQRSHRUN = NEONShiftImmediateFixed | 0x20008800,
  NEON_SSHR     = NEONShiftImmediateFixed | 0x00000000,
  NEON_SRSHR    = NEONShiftImmediateFixed | 0x00002000,
  NEON_USHR     = NEONShiftImmediateFixed | 0x20000000,
  NEON_URSHR    = NEONShiftImmediateFixed | 0x20002000,
  NEON_SSRA     = NEONShiftImmediateFixed | 0x00001000,
  NEON_SRSRA    = NEONShiftImmediateFixed | 0x00003000,
  NEON_USRA     = NEONShiftImmediateFixed | 0x20001000,
  NEON_URSRA    = NEONShiftImmediateFixed | 0x20003000,
  NEON_SQSHLU   = NEONShiftImmediateFixed | 0x20006000,
  NEON_SCVTF_imm = NEONShiftImmediateFixed | 0x0000E000,
  NEON_UCVTF_imm = NEONShiftImmediateFixed | 0x2000E000,
  NEON_FCVTZS_imm = NEONShiftImmediateFixed | 0x0000F800,
  NEON_FCVTZU_imm = NEONShiftImmediateFixed | 0x2000F800,
  NEON_SQSHL_imm = NEONShiftImmediateFixed | 0x00007000,
  NEON_UQSHL_imm = NEONShiftImmediateFixed | 0x20007000
};

// NEON table.
enum NEONTableOp {
  NEONTableFixed = 0x0E000000,
  NEONTableFMask = 0xBF208C00,
  NEONTableExt   = 0x00001000,
  NEONTableMask  = 0xBF20FC00,
  NEON_TBL_1v    = NEONTableFixed | 0x00000000,
  NEON_TBL_2v    = NEONTableFixed | 0x00002000,
  NEON_TBL_3v    = NEONTableFixed | 0x00004000,
  NEON_TBL_4v    = NEONTableFixed | 0x00006000,
  NEON_TBX_1v    = NEON_TBL_1v | NEONTableExt,
  NEON_TBX_2v    = NEON_TBL_2v | NEONTableExt,
  NEON_TBX_3v    = NEON_TBL_3v | NEONTableExt,
  NEON_TBX_4v    = NEON_TBL_4v | NEONTableExt
};

// NEON perm.
enum NEONPermOp {
  NEONPermFixed = 0x0E000800,
  NEONPermFMask = 0xBF208C00,
  NEONPermMask  = 0x3F20FC00,
  NEON_UZP1 = NEONPermFixed | 0x00001000,
  NEON_TRN1 = NEONPermFixed | 0x00002000,
  NEON_ZIP1 = NEONPermFixed | 0x00003000,
  NEON_UZP2 = NEONPermFixed | 0x00005000,
  NEON_TRN2 = NEONPermFixed | 0x00006000,
  NEON_ZIP2 = NEONPermFixed | 0x00007000
};

// NEON scalar instructions with two register operands.
enum NEONScalar2RegMiscOp {
  NEONScalar2RegMiscFixed = 0x5E200800,
  NEONScalar2RegMiscFMask = 0xDF3E0C00,
  NEONScalar2RegMiscMask = NEON_Q | NEONScalar | NEON2RegMiscMask,
  NEON_CMGT_zero_scalar = NEON_Q | NEONScalar | NEON_CMGT_zero,
  NEON_CMEQ_zero_scalar = NEON_Q | NEONScalar | NEON_CMEQ_zero,
  NEON_CMLT_zero_scalar = NEON_Q | NEONScalar | NEON_CMLT_zero,
  NEON_CMGE_zero_scalar = NEON_Q | NEONScalar | NEON_CMGE_zero,
  NEON_CMLE_zero_scalar = NEON_Q | NEONScalar | NEON_CMLE_zero,
  NEON_ABS_scalar       = NEON_Q | NEONScalar | NEON_ABS,
  NEON_SQABS_scalar     = NEON_Q | NEONScalar | NEON_SQABS,
  NEON_NEG_scalar       = NEON_Q | NEONScalar | NEON_NEG,
  NEON_SQNEG_scalar     = NEON_Q | NEONScalar | NEON_SQNEG,
  NEON_SQXTN_scalar     = NEON_Q | NEONScalar | NEON_SQXTN,
  NEON_UQXTN_scalar     = NEON_Q | NEONScalar | NEON_UQXTN,
  NEON_SQXTUN_scalar    = NEON_Q | NEONScalar | NEON_SQXTUN,
  NEON_SUQADD_scalar    = NEON_Q | NEONScalar | NEON_SUQADD,
  NEON_USQADD_scalar    = NEON_Q | NEONScalar | NEON_USQADD,

  NEONScalar2RegMiscOpcode = NEON2RegMiscOpcode,
  NEON_NEG_scalar_opcode = NEON_NEG_scalar & NEONScalar2RegMiscOpcode,

  NEONScalar2RegMiscFPMask  = NEONScalar2RegMiscMask | 0x00800000,
  NEON_FRSQRTE_scalar    = NEON_Q | NEONScalar | NEON_FRSQRTE,
  NEON_FRECPE_scalar     = NEON_Q | NEONScalar | NEON_FRECPE,
  NEON_SCVTF_scalar      = NEON_Q | NEONScalar | NEON_SCVTF,
  NEON_UCVTF_scalar      = NEON_Q | NEONScalar | NEON_UCVTF,
  NEON_FCMGT_zero_scalar = NEON_Q | NEONScalar | NEON_FCMGT_zero,
  NEON_FCMEQ_zero_scalar = NEON_Q | NEONScalar | NEON_FCMEQ_zero,
  NEON_FCMLT_zero_scalar = NEON_Q | NEONScalar | NEON_FCMLT_zero,
  NEON_FCMGE_zero_scalar = NEON_Q | NEONScalar | NEON_FCMGE_zero,
  NEON_FCMLE_zero_scalar = NEON_Q | NEONScalar | NEON_FCMLE_zero,
  NEON_FRECPX_scalar     = NEONScalar2RegMiscFixed | 0x0081F000,
  NEON_FCVTNS_scalar     = NEON_Q | NEONScalar | NEON_FCVTNS,
  NEON_FCVTNU_scalar     = NEON_Q | NEONScalar | NEON_FCVTNU,
  NEON_FCVTPS_scalar     = NEON_Q | NEONScalar | NEON_FCVTPS,
  NEON_FCVTPU_scalar     = NEON_Q | NEONScalar | NEON_FCVTPU,
  NEON_FCVTMS_scalar     = NEON_Q | NEONScalar | NEON_FCVTMS,
  NEON_FCVTMU_scalar     = NEON_Q | NEONScalar | NEON_FCVTMU,
  NEON_FCVTZS_scalar     = NEON_Q | NEONScalar | NEON_FCVTZS,
  NEON_FCVTZU_scalar     = NEON_Q | NEONScalar | NEON_FCVTZU,
  NEON_FCVTAS_scalar     = NEON_Q | NEONScalar | NEON_FCVTAS,
  NEON_FCVTAU_scalar     = NEON_Q | NEONScalar | NEON_FCVTAU,
  NEON_FCVTXN_scalar     = NEON_Q | NEONScalar | NEON_FCVTXN
};

// NEON instructions with two register operands (FP16).
enum NEONScalar2RegMiscFP16Op {
  NEONScalar2RegMiscFP16Fixed = 0x5E780800,
  NEONScalar2RegMiscFP16FMask = 0xDF7E0C00,
  NEONScalar2RegMiscFP16Mask  = 0xFFFFFC00,
  NEON_FCVTNS_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTNS_H,
  NEON_FCVTMS_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTMS_H,
  NEON_FCVTAS_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTAS_H,
  NEON_SCVTF_H_scalar      = NEON_Q | NEONScalar | NEON_SCVTF_H,
  NEON_FCMGT_H_zero_scalar = NEON_Q | NEONScalar | NEON_FCMGT_H_zero,
  NEON_FCMEQ_H_zero_scalar = NEON_Q | NEONScalar | NEON_FCMEQ_H_zero,
  NEON_FCMLT_H_zero_scalar = NEON_Q | NEONScalar | NEON_FCMLT_H_zero,
  NEON_FCVTPS_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTPS_H,
  NEON_FCVTZS_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTZS_H,
  NEON_FRECPE_H_scalar     = NEON_Q | NEONScalar | NEON_FRECPE_H,
  NEON_FRECPX_H_scalar     = NEONScalar2RegMiscFP16Fixed | 0x0081F000,
  NEON_FCVTNU_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTNU_H,
  NEON_FCVTMU_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTMU_H,
  NEON_FCVTAU_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTAU_H,
  NEON_UCVTF_H_scalar      = NEON_Q | NEONScalar | NEON_UCVTF_H,
  NEON_FCMGE_H_zero_scalar = NEON_Q | NEONScalar | NEON_FCMGE_H_zero,
  NEON_FCMLE_H_zero_scalar = NEON_Q | NEONScalar | NEON_FCMLE_H_zero,
  NEON_FCVTPU_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTPU_H,
  NEON_FCVTZU_H_scalar     = NEON_Q | NEONScalar | NEON_FCVTZU_H,
  NEON_FRSQRTE_H_scalar    = NEON_Q | NEONScalar | NEON_FRSQRTE_H
};

// NEON scalar instructions with three same-type operands.
enum NEONScalar3SameOp {
  NEONScalar3SameFixed = 0x5E200400,
  NEONScalar3SameFMask = 0xDF200400,
  NEONScalar3SameMask  = 0xFF20FC00,
  NEON_ADD_scalar    = NEON_Q | NEONScalar | NEON_ADD,
  NEON_CMEQ_scalar   = NEON_Q | NEONScalar | NEON_CMEQ,
  NEON_CMGE_scalar   = NEON_Q | NEONScalar | NEON_CMGE,
  NEON_CMGT_scalar   = NEON_Q | NEONScalar | NEON_CMGT,
  NEON_CMHI_scalar   = NEON_Q | NEONScalar | NEON_CMHI,
  NEON_CMHS_scalar   = NEON_Q | NEONScalar | NEON_CMHS,
  NEON_CMTST_scalar  = NEON_Q | NEONScalar | NEON_CMTST,
  NEON_SUB_scalar    = NEON_Q | NEONScalar | NEON_SUB,
  NEON_UQADD_scalar  = NEON_Q | NEONScalar | NEON_UQADD,
  NEON_SQADD_scalar  = NEON_Q | NEONScalar | NEON_SQADD,
  NEON_UQSUB_scalar  = NEON_Q | NEONScalar | NEON_UQSUB,
  NEON_SQSUB_scalar  = NEON_Q | NEONScalar | NEON_SQSUB,
  NEON_USHL_scalar   = NEON_Q | NEONScalar | NEON_USHL,
  NEON_SSHL_scalar   = NEON_Q | NEONScalar | NEON_SSHL,
  NEON_UQSHL_scalar  = NEON_Q | NEONScalar | NEON_UQSHL,
  NEON_SQSHL_scalar  = NEON_Q | NEONScalar | NEON_SQSHL,
  NEON_URSHL_scalar  = NEON_Q | NEONScalar | NEON_URSHL,
  NEON_SRSHL_scalar  = NEON_Q | NEONScalar | NEON_SRSHL,
  NEON_UQRSHL_scalar = NEON_Q | NEONScalar | NEON_UQRSHL,
  NEON_SQRSHL_scalar = NEON_Q | NEONScalar | NEON_SQRSHL,
  NEON_SQDMULH_scalar = NEON_Q | NEONScalar | NEON_SQDMULH,
  NEON_SQRDMULH_scalar = NEON_Q | NEONScalar | NEON_SQRDMULH,

  // NEON floating point scalar instructions with three same-type operands.
  NEONScalar3SameFPFixed = NEONScalar3SameFixed | 0x0000C000,
  NEONScalar3SameFPFMask = NEONScalar3SameFMask | 0x0000C000,
  NEONScalar3SameFPMask  = NEONScalar3SameMask | 0x00800000,
  NEON_FACGE_scalar   = NEON_Q | NEONScalar | NEON_FACGE,
  NEON_FACGT_scalar   = NEON_Q | NEONScalar | NEON_FACGT,
  NEON_FCMEQ_scalar   = NEON_Q | NEONScalar | NEON_FCMEQ,
  NEON_FCMGE_scalar   = NEON_Q | NEONScalar | NEON_FCMGE,
  NEON_FCMGT_scalar   = NEON_Q | NEONScalar | NEON_FCMGT,
  NEON_FMULX_scalar   = NEON_Q | NEONScalar | NEON_FMULX,
  NEON_FRECPS_scalar  = NEON_Q | NEONScalar | NEON_FRECPS,
  NEON_FRSQRTS_scalar = NEON_Q | NEONScalar | NEON_FRSQRTS,
  NEON_FABD_scalar    = NEON_Q | NEONScalar | NEON_FABD
};

// NEON scalar FP16 instructions with three same-type operands.
enum NEONScalar3SameFP16Op {
  NEONScalar3SameFP16Fixed = 0x5E400400,
  NEONScalar3SameFP16FMask = 0xDF60C400,
  NEONScalar3SameFP16Mask  = 0xFFE0FC00,
  NEON_FABD_H_scalar    = NEON_Q | NEONScalar | NEON_FABD_H,
  NEON_FMULX_H_scalar   = NEON_Q | NEONScalar | NEON_FMULX_H,
  NEON_FCMEQ_H_scalar   = NEON_Q | NEONScalar | NEON_FCMEQ_H,
  NEON_FCMGE_H_scalar   = NEON_Q | NEONScalar | NEON_FCMGE_H,
  NEON_FCMGT_H_scalar   = NEON_Q | NEONScalar | NEON_FCMGT_H,
  NEON_FACGE_H_scalar   = NEON_Q | NEONScalar | NEON_FACGE_H,
  NEON_FACGT_H_scalar   = NEON_Q | NEONScalar | NEON_FACGT_H,
  NEON_FRECPS_H_scalar  = NEON_Q | NEONScalar | NEON_FRECPS_H,
  NEON_FRSQRTS_H_scalar = NEON_Q | NEONScalar | NEON_FRSQRTS_H
};

// 'Extra' NEON scalar instructions with three same-type operands.
enum NEONScalar3SameExtraOp {
  NEONScalar3SameExtraFixed = 0x5E008400,
  NEONScalar3SameExtraFMask = 0xDF208400,
  NEONScalar3SameExtraMask = 0xFF20FC00,
  NEON_SQRDMLAH_scalar = NEON_Q | NEONScalar | NEON_SQRDMLAH,
  NEON_SQRDMLSH_scalar = NEON_Q | NEONScalar | NEON_SQRDMLSH
};

// NEON scalar instructions with three different-type operands.
enum NEONScalar3DiffOp {
  NEONScalar3DiffFixed = 0x5E200000,
  NEONScalar3DiffFMask = 0xDF200C00,
  NEONScalar3DiffMask  = NEON_Q | NEONScalar | NEON3DifferentMask,
  NEON_SQDMLAL_scalar  = NEON_Q | NEONScalar | NEON_SQDMLAL,
  NEON_SQDMLSL_scalar  = NEON_Q | NEONScalar | NEON_SQDMLSL,
  NEON_SQDMULL_scalar  = NEON_Q | NEONScalar | NEON_SQDMULL
};

// NEON scalar instructions with indexed element operand.
enum NEONScalarByIndexedElementOp {
  NEONScalarByIndexedElementFixed = 0x5F000000,
  NEONScalarByIndexedElementFMask = 0xDF000400,
  NEONScalarByIndexedElementMask  = 0xFF00F400,
  NEON_SQDMLAL_byelement_scalar  = NEON_Q | NEONScalar | NEON_SQDMLAL_byelement,
  NEON_SQDMLSL_byelement_scalar  = NEON_Q | NEONScalar | NEON_SQDMLSL_byelement,
  NEON_SQDMULL_byelement_scalar  = NEON_Q | NEONScalar | NEON_SQDMULL_byelement,
  NEON_SQDMULH_byelement_scalar  = NEON_Q | NEONScalar | NEON_SQDMULH_byelement,
  NEON_SQRDMULH_byelement_scalar
    = NEON_Q | NEONScalar | NEON_SQRDMULH_byelement,
  NEON_SQRDMLAH_byelement_scalar
    = NEON_Q | NEONScalar | NEON_SQRDMLAH_byelement,
  NEON_SQRDMLSH_byelement_scalar
    = NEON_Q | NEONScalar | NEON_SQRDMLSH_byelement,
  NEON_FMLA_H_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMLA_H_byelement,
  NEON_FMLS_H_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMLS_H_byelement,
  NEON_FMUL_H_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMUL_H_byelement,
  NEON_FMULX_H_byelement_scalar = NEON_Q | NEONScalar | NEON_FMULX_H_byelement,

  // Floating point instructions.
  NEONScalarByIndexedElementFPFixed
    = NEONScalarByIndexedElementFixed | 0x00800000,
  NEONScalarByIndexedElementFPMask
    = NEONScalarByIndexedElementMask | 0x00800000,
  NEON_FMLA_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMLA_byelement,
  NEON_FMLS_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMLS_byelement,
  NEON_FMUL_byelement_scalar  = NEON_Q | NEONScalar | NEON_FMUL_byelement,
  NEON_FMULX_byelement_scalar = NEON_Q | NEONScalar | NEON_FMULX_byelement
};

// NEON scalar register copy.
enum NEONScalarCopyOp {
  NEONScalarCopyFixed = 0x5E000400,
  NEONScalarCopyFMask = 0xDFE08400,
  NEONScalarCopyMask  = 0xFFE0FC00,
  NEON_DUP_ELEMENT_scalar = NEON_Q | NEONScalar | NEON_DUP_ELEMENT
};

// NEON scalar pairwise instructions.
enum NEONScalarPairwiseOp {
  NEONScalarPairwiseFixed = 0x5E300800,
  NEONScalarPairwiseFMask = 0xDF3E0C00,
  NEONScalarPairwiseMask  = 0xFFB1F800,
  NEON_ADDP_scalar      = NEONScalarPairwiseFixed | 0x0081B000,
  NEON_FMAXNMP_h_scalar = NEONScalarPairwiseFixed | 0x0000C000,
  NEON_FADDP_h_scalar   = NEONScalarPairwiseFixed | 0x0000D000,
  NEON_FMAXP_h_scalar   = NEONScalarPairwiseFixed | 0x0000F000,
  NEON_FMINNMP_h_scalar = NEONScalarPairwiseFixed | 0x0080C000,
  NEON_FMINP_h_scalar   = NEONScalarPairwiseFixed | 0x0080F000,
  NEON_FMAXNMP_scalar   = NEONScalarPairwiseFixed | 0x2000C000,
  NEON_FMINNMP_scalar   = NEONScalarPairwiseFixed | 0x2080C000,
  NEON_FADDP_scalar     = NEONScalarPairwiseFixed | 0x2000D000,
  NEON_FMAXP_scalar     = NEONScalarPairwiseFixed | 0x2000F000,
  NEON_FMINP_scalar     = NEONScalarPairwiseFixed | 0x2080F000
};

// NEON scalar shift immediate.
enum NEONScalarShiftImmediateOp {
  NEONScalarShiftImmediateFixed = 0x5F000400,
  NEONScalarShiftImmediateFMask = 0xDF800400,
  NEONScalarShiftImmediateMask  = 0xFF80FC00,
  NEON_SHL_scalar  =       NEON_Q | NEONScalar | NEON_SHL,
  NEON_SLI_scalar  =       NEON_Q | NEONScalar | NEON_SLI,
  NEON_SRI_scalar  =       NEON_Q | NEONScalar | NEON_SRI,
  NEON_SSHR_scalar =       NEON_Q | NEONScalar | NEON_SSHR,
  NEON_USHR_scalar =       NEON_Q | NEONScalar | NEON_USHR,
  NEON_SRSHR_scalar =      NEON_Q | NEONScalar | NEON_SRSHR,
  NEON_URSHR_scalar =      NEON_Q | NEONScalar | NEON_URSHR,
  NEON_SSRA_scalar =       NEON_Q | NEONScalar | NEON_SSRA,
  NEON_USRA_scalar =       NEON_Q | NEONScalar | NEON_USRA,
  NEON_SRSRA_scalar =      NEON_Q | NEONScalar | NEON_SRSRA,
  NEON_URSRA_scalar =      NEON_Q | NEONScalar | NEON_URSRA,
  NEON_UQSHRN_scalar =     NEON_Q | NEONScalar | NEON_UQSHRN,
  NEON_UQRSHRN_scalar =    NEON_Q | NEONScalar | NEON_UQRSHRN,
  NEON_SQSHRN_scalar =     NEON_Q | NEONScalar | NEON_SQSHRN,
  NEON_SQRSHRN_scalar =    NEON_Q | NEONScalar | NEON_SQRSHRN,
  NEON_SQSHRUN_scalar =    NEON_Q | NEONScalar | NEON_SQSHRUN,
  NEON_SQRSHRUN_scalar =   NEON_Q | NEONScalar | NEON_SQRSHRUN,
  NEON_SQSHLU_scalar =     NEON_Q | NEONScalar | NEON_SQSHLU,
  NEON_SQSHL_imm_scalar  = NEON_Q | NEONScalar | NEON_SQSHL_imm,
  NEON_UQSHL_imm_scalar  = NEON_Q | NEONScalar | NEON_UQSHL_imm,
  NEON_SCVTF_imm_scalar =  NEON_Q | NEONScalar | NEON_SCVTF_imm,
  NEON_UCVTF_imm_scalar =  NEON_Q | NEONScalar | NEON_UCVTF_imm,
  NEON_FCVTZS_imm_scalar = NEON_Q | NEONScalar | NEON_FCVTZS_imm,
  NEON_FCVTZU_imm_scalar = NEON_Q | NEONScalar | NEON_FCVTZU_imm
};

enum SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsOp {
  SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed = 0x84A00000,
  SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFMask = 0xFFA08000,
  SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsMask = 0xFFA0E000,
  LD1SH_z_p_bz_s_x32_scaled = SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed,
  LDFF1SH_z_p_bz_s_x32_scaled = SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed | 0x00002000,
  LD1H_z_p_bz_s_x32_scaled = SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed | 0x00004000,
  LDFF1H_z_p_bz_s_x32_scaled = SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsetsFixed | 0x00006000
};

enum SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsOp {
  SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsFixed = 0x85200000,
  SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsFMask = 0xFFA08000,
  SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsMask = 0xFFA0E000,
  LD1W_z_p_bz_s_x32_scaled = SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsFixed | 0x00004000,
  LDFF1W_z_p_bz_s_x32_scaled = SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsetsFixed | 0x00006000
};

enum SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsOp {
  SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed = 0x84000000,
  SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFMask = 0xFE208000,
  SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsMask = 0xFFA0E000,
  LD1SB_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed,
  LDFF1SB_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00002000,
  LD1B_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00004000,
  LDFF1B_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00006000,
  LD1SH_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00800000,
  LDFF1SH_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00802000,
  LD1H_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00804000,
  LDFF1H_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x00806000,
  LD1W_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x01004000,
  LDFF1W_z_p_bz_s_x32_unscaled = SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsetsFixed | 0x01006000
};

enum SVE32BitGatherLoad_VectorPlusImmOp {
  SVE32BitGatherLoad_VectorPlusImmFixed = 0x84208000,
  SVE32BitGatherLoad_VectorPlusImmFMask = 0xFE608000,
  SVE32BitGatherLoad_VectorPlusImmMask = 0xFFE0E000,
  LD1SB_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed,
  LDFF1SB_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00002000,
  LD1B_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00004000,
  LDFF1B_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00006000,
  LD1SH_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00800000,
  LDFF1SH_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00802000,
  LD1H_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00804000,
  LDFF1H_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x00806000,
  LD1W_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x01004000,
  LDFF1W_z_p_ai_s = SVE32BitGatherLoad_VectorPlusImmFixed | 0x01006000
};

enum SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsOp {
  SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFixed = 0x84200000,
  SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFMask = 0xFFA08010,
  SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsMask = 0xFFA0E010,
  PRFB_i_p_bz_s_x32_scaled = SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFixed,
  PRFH_i_p_bz_s_x32_scaled = SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFixed | 0x00002000,
  PRFW_i_p_bz_s_x32_scaled = SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFixed | 0x00004000,
  PRFD_i_p_bz_s_x32_scaled = SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsFixed | 0x00006000
};

enum SVE32BitGatherPrefetch_VectorPlusImmOp {
  SVE32BitGatherPrefetch_VectorPlusImmFixed = 0x8400E000,
  SVE32BitGatherPrefetch_VectorPlusImmFMask = 0xFE60E010,
  SVE32BitGatherPrefetch_VectorPlusImmMask = 0xFFE0E010,
  PRFB_i_p_ai_s = SVE32BitGatherPrefetch_VectorPlusImmFixed,
  PRFH_i_p_ai_s = SVE32BitGatherPrefetch_VectorPlusImmFixed | 0x00800000,
  PRFW_i_p_ai_s = SVE32BitGatherPrefetch_VectorPlusImmFixed | 0x01000000,
  PRFD_i_p_ai_s = SVE32BitGatherPrefetch_VectorPlusImmFixed | 0x01800000
};

enum SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsOp {
  SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFixed = 0xE4608000,
  SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFMask = 0xFE60A000,
  SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsMask = 0xFFE0A000,
  ST1H_z_p_bz_s_x32_scaled = SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_s_x32_scaled = SVE32BitScatterStore_ScalarPlus32BitScaledOffsetsFixed | 0x01000000
};

enum SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsOp {
  SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFixed = 0xE4408000,
  SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFMask = 0xFE60A000,
  SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsMask = 0xFFE0A000,
  ST1B_z_p_bz_s_x32_unscaled = SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFixed,
  ST1H_z_p_bz_s_x32_unscaled = SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_s_x32_unscaled = SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsetsFixed | 0x01000000
};

enum SVE32BitScatterStore_VectorPlusImmOp {
  SVE32BitScatterStore_VectorPlusImmFixed = 0xE460A000,
  SVE32BitScatterStore_VectorPlusImmFMask = 0xFE60E000,
  SVE32BitScatterStore_VectorPlusImmMask = 0xFFE0E000,
  ST1B_z_p_ai_s = SVE32BitScatterStore_VectorPlusImmFixed,
  ST1H_z_p_ai_s = SVE32BitScatterStore_VectorPlusImmFixed | 0x00800000,
  ST1W_z_p_ai_s = SVE32BitScatterStore_VectorPlusImmFixed | 0x01000000
};

enum SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsOp {
  SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed = 0xC4200000,
  SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFMask = 0xFE208000,
  SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsMask = 0xFFA0E000,
  LD1SH_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x00800000,
  LDFF1SH_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x00802000,
  LD1H_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x00804000,
  LDFF1H_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x00806000,
  LD1SW_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01000000,
  LDFF1SW_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01002000,
  LD1W_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01004000,
  LDFF1W_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01006000,
  LD1D_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01804000,
  LDFF1D_z_p_bz_d_x32_scaled = SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsetsFixed | 0x01806000
};

enum SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsOp {
  SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed = 0xC4608000,
  SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFMask = 0xFE608000,
  SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsMask = 0xFFE0E000,
  LD1SH_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x00800000,
  LDFF1SH_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x00802000,
  LD1H_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x00804000,
  LDFF1H_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x00806000,
  LD1SW_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01000000,
  LDFF1SW_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01002000,
  LD1W_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01004000,
  LDFF1W_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01006000,
  LD1D_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01804000,
  LDFF1D_z_p_bz_d_64_scaled = SVE64BitGatherLoad_ScalarPlus64BitScaledOffsetsFixed | 0x01806000
};

enum SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsOp {
  SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed = 0xC4408000,
  SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFMask = 0xFE608000,
  SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsMask = 0xFFE0E000,
  LD1SB_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed,
  LDFF1SB_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00002000,
  LD1B_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00004000,
  LDFF1B_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00006000,
  LD1SH_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00800000,
  LDFF1SH_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00802000,
  LD1H_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00804000,
  LDFF1H_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x00806000,
  LD1SW_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01000000,
  LDFF1SW_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01002000,
  LD1W_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01004000,
  LDFF1W_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01006000,
  LD1D_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01804000,
  LDFF1D_z_p_bz_d_64_unscaled = SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsetsFixed | 0x01806000
};

enum SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsOp {
  SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed = 0xC4000000,
  SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFMask = 0xFE208000,
  SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsMask = 0xFFA0E000,
  LD1SB_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed,
  LDFF1SB_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00002000,
  LD1B_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00004000,
  LDFF1B_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00006000,
  LD1SH_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00800000,
  LDFF1SH_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00802000,
  LD1H_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00804000,
  LDFF1H_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00806000,
  LD1SW_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01000000,
  LDFF1SW_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01002000,
  LD1W_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01004000,
  LDFF1W_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01006000,
  LD1D_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01804000,
  LDFF1D_z_p_bz_d_x32_unscaled = SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01806000
};

enum SVE64BitGatherLoad_VectorPlusImmOp {
  SVE64BitGatherLoad_VectorPlusImmFixed = 0xC4208000,
  SVE64BitGatherLoad_VectorPlusImmFMask = 0xFE608000,
  SVE64BitGatherLoad_VectorPlusImmMask = 0xFFE0E000,
  LD1SB_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed,
  LDFF1SB_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00002000,
  LD1B_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00004000,
  LDFF1B_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00006000,
  LD1SH_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00800000,
  LDFF1SH_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00802000,
  LD1H_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00804000,
  LDFF1H_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x00806000,
  LD1SW_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01000000,
  LDFF1SW_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01002000,
  LD1W_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01004000,
  LDFF1W_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01006000,
  LD1D_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01804000,
  LDFF1D_z_p_ai_d = SVE64BitGatherLoad_VectorPlusImmFixed | 0x01806000
};

enum SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsOp {
  SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFixed = 0xC4608000,
  SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFMask = 0xFFE08010,
  SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsMask = 0xFFE0E010,
  PRFB_i_p_bz_d_64_scaled = SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFixed,
  PRFH_i_p_bz_d_64_scaled = SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFixed | 0x00002000,
  PRFW_i_p_bz_d_64_scaled = SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFixed | 0x00004000,
  PRFD_i_p_bz_d_64_scaled = SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsetsFixed | 0x00006000
};

enum SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsOp {
  SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFixed = 0xC4200000,
  SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFMask = 0xFFA08010,
  SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsMask = 0xFFA0E010,
  PRFB_i_p_bz_d_x32_scaled = SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFixed,
  PRFH_i_p_bz_d_x32_scaled = SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x00002000,
  PRFW_i_p_bz_d_x32_scaled = SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x00004000,
  PRFD_i_p_bz_d_x32_scaled = SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x00006000
};

enum SVE64BitGatherPrefetch_VectorPlusImmOp {
  SVE64BitGatherPrefetch_VectorPlusImmFixed = 0xC400E000,
  SVE64BitGatherPrefetch_VectorPlusImmFMask = 0xFE60E010,
  SVE64BitGatherPrefetch_VectorPlusImmMask = 0xFFE0E010,
  PRFB_i_p_ai_d = SVE64BitGatherPrefetch_VectorPlusImmFixed,
  PRFH_i_p_ai_d = SVE64BitGatherPrefetch_VectorPlusImmFixed | 0x00800000,
  PRFW_i_p_ai_d = SVE64BitGatherPrefetch_VectorPlusImmFixed | 0x01000000,
  PRFD_i_p_ai_d = SVE64BitGatherPrefetch_VectorPlusImmFixed | 0x01800000
};

enum SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsOp {
  SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFixed = 0xE420A000,
  SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFMask = 0xFE60E000,
  SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsMask = 0xFFE0E000,
  ST1H_z_p_bz_d_64_scaled = SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_d_64_scaled = SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFixed | 0x01000000,
  ST1D_z_p_bz_d_64_scaled = SVE64BitScatterStore_ScalarPlus64BitScaledOffsetsFixed | 0x01800000
};

enum SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsOp {
  SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed = 0xE400A000,
  SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFMask = 0xFE60E000,
  SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsMask = 0xFFE0E000,
  ST1B_z_p_bz_d_64_unscaled = SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed,
  ST1H_z_p_bz_d_64_unscaled = SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_d_64_unscaled = SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed | 0x01000000,
  ST1D_z_p_bz_d_64_unscaled = SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsetsFixed | 0x01800000
};

enum SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsOp {
  SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFixed = 0xE4208000,
  SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFMask = 0xFE60A000,
  SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsMask = 0xFFE0A000,
  ST1H_z_p_bz_d_x32_scaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_d_x32_scaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x01000000,
  ST1D_z_p_bz_d_x32_scaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsetsFixed | 0x01800000
};

enum SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsOp {
  SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed = 0xE4008000,
  SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFMask = 0xFE60A000,
  SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsMask = 0xFFE0A000,
  ST1B_z_p_bz_d_x32_unscaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed,
  ST1H_z_p_bz_d_x32_unscaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x00800000,
  ST1W_z_p_bz_d_x32_unscaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01000000,
  ST1D_z_p_bz_d_x32_unscaled = SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsetsFixed | 0x01800000
};

enum SVE64BitScatterStore_VectorPlusImmOp {
  SVE64BitScatterStore_VectorPlusImmFixed = 0xE440A000,
  SVE64BitScatterStore_VectorPlusImmFMask = 0xFE60E000,
  SVE64BitScatterStore_VectorPlusImmMask = 0xFFE0E000,
  ST1B_z_p_ai_d = SVE64BitScatterStore_VectorPlusImmFixed,
  ST1H_z_p_ai_d = SVE64BitScatterStore_VectorPlusImmFixed | 0x00800000,
  ST1W_z_p_ai_d = SVE64BitScatterStore_VectorPlusImmFixed | 0x01000000,
  ST1D_z_p_ai_d = SVE64BitScatterStore_VectorPlusImmFixed | 0x01800000
};

enum SVEAddressGenerationOp {
  SVEAddressGenerationFixed = 0x0420A000,
  SVEAddressGenerationFMask = 0xFF20F000,
  SVEAddressGenerationMask = 0xFFE0F000,
  ADR_z_az_d_s32_scaled = SVEAddressGenerationFixed,
  ADR_z_az_d_u32_scaled = SVEAddressGenerationFixed | 0x00400000,
  ADR_z_az_s_same_scaled = SVEAddressGenerationFixed | 0x00800000,
  ADR_z_az_d_same_scaled = SVEAddressGenerationFixed | 0x00C00000
};

enum SVEBitwiseLogicalUnpredicatedOp {
  SVEBitwiseLogicalUnpredicatedFixed = 0x04202000,
  SVEBitwiseLogicalUnpredicatedFMask = 0xFF20E000,
  SVEBitwiseLogicalUnpredicatedMask = 0xFFE0FC00,
  AND_z_zz = SVEBitwiseLogicalUnpredicatedFixed | 0x00001000,
  ORR_z_zz = SVEBitwiseLogicalUnpredicatedFixed | 0x00401000,
  EOR_z_zz = SVEBitwiseLogicalUnpredicatedFixed | 0x00801000,
  BIC_z_zz = SVEBitwiseLogicalUnpredicatedFixed | 0x00C01000
};

enum SVEBitwiseLogicalWithImm_UnpredicatedOp {
  SVEBitwiseLogicalWithImm_UnpredicatedFixed = 0x05000000,
  SVEBitwiseLogicalWithImm_UnpredicatedFMask = 0xFF3C0000,
  SVEBitwiseLogicalWithImm_UnpredicatedMask = 0xFFFC0000,
  ORR_z_zi = SVEBitwiseLogicalWithImm_UnpredicatedFixed,
  EOR_z_zi = SVEBitwiseLogicalWithImm_UnpredicatedFixed | 0x00400000,
  AND_z_zi = SVEBitwiseLogicalWithImm_UnpredicatedFixed | 0x00800000
};

enum SVEBitwiseLogical_PredicatedOp {
  SVEBitwiseLogical_PredicatedFixed = 0x04180000,
  SVEBitwiseLogical_PredicatedFMask = 0xFF38E000,
  SVEBitwiseLogical_PredicatedMask = 0xFF3FE000,
  ORR_z_p_zz = SVEBitwiseLogical_PredicatedFixed,
  EOR_z_p_zz = SVEBitwiseLogical_PredicatedFixed | 0x00010000,
  AND_z_p_zz = SVEBitwiseLogical_PredicatedFixed | 0x00020000,
  BIC_z_p_zz = SVEBitwiseLogical_PredicatedFixed | 0x00030000
};

enum SVEBitwiseShiftByImm_PredicatedOp {
  SVEBitwiseShiftByImm_PredicatedFixed = 0x04008000,
  SVEBitwiseShiftByImm_PredicatedFMask = 0xFF30E000,
  SVEBitwiseShiftByImm_PredicatedMask = 0xFF3FE000,
  ASR_z_p_zi = SVEBitwiseShiftByImm_PredicatedFixed,
  LSR_z_p_zi = SVEBitwiseShiftByImm_PredicatedFixed | 0x00010000,
  LSL_z_p_zi = SVEBitwiseShiftByImm_PredicatedFixed | 0x00030000,
  ASRD_z_p_zi = SVEBitwiseShiftByImm_PredicatedFixed | 0x00040000
};

enum SVEBitwiseShiftByVector_PredicatedOp {
  SVEBitwiseShiftByVector_PredicatedFixed = 0x04108000,
  SVEBitwiseShiftByVector_PredicatedFMask = 0xFF38E000,
  SVEBitwiseShiftByVector_PredicatedMask = 0xFF3FE000,
  ASR_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed,
  LSR_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed | 0x00010000,
  LSL_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed | 0x00030000,
  ASRR_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed | 0x00040000,
  LSRR_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed | 0x00050000,
  LSLR_z_p_zz = SVEBitwiseShiftByVector_PredicatedFixed | 0x00070000
};

enum SVEBitwiseShiftByWideElements_PredicatedOp {
  SVEBitwiseShiftByWideElements_PredicatedFixed = 0x04188000,
  SVEBitwiseShiftByWideElements_PredicatedFMask = 0xFF38E000,
  SVEBitwiseShiftByWideElements_PredicatedMask = 0xFF3FE000,
  ASR_z_p_zw = SVEBitwiseShiftByWideElements_PredicatedFixed,
  LSR_z_p_zw = SVEBitwiseShiftByWideElements_PredicatedFixed | 0x00010000,
  LSL_z_p_zw = SVEBitwiseShiftByWideElements_PredicatedFixed | 0x00030000
};

enum SVEBitwiseShiftUnpredicatedOp {
  SVEBitwiseShiftUnpredicatedFixed = 0x04208000,
  SVEBitwiseShiftUnpredicatedFMask = 0xFF20E000,
  SVEBitwiseShiftUnpredicatedMask = 0xFF20FC00,
  ASR_z_zw = SVEBitwiseShiftUnpredicatedFixed,
  LSR_z_zw = SVEBitwiseShiftUnpredicatedFixed | 0x00000400,
  LSL_z_zw = SVEBitwiseShiftUnpredicatedFixed | 0x00000C00,
  ASR_z_zi = SVEBitwiseShiftUnpredicatedFixed | 0x00001000,
  LSR_z_zi = SVEBitwiseShiftUnpredicatedFixed | 0x00001400,
  LSL_z_zi = SVEBitwiseShiftUnpredicatedFixed | 0x00001C00
};

enum SVEBroadcastBitmaskImmOp {
  SVEBroadcastBitmaskImmFixed = 0x05C00000,
  SVEBroadcastBitmaskImmFMask = 0xFFFC0000,
  SVEBroadcastBitmaskImmMask = 0xFFFC0000,
  DUPM_z_i = SVEBroadcastBitmaskImmFixed
};

enum SVEBroadcastFPImm_UnpredicatedOp {
  SVEBroadcastFPImm_UnpredicatedFixed = 0x2539C000,
  SVEBroadcastFPImm_UnpredicatedFMask = 0xFF39C000,
  SVEBroadcastFPImm_UnpredicatedMask = 0xFF3FE000,
  FDUP_z_i = SVEBroadcastFPImm_UnpredicatedFixed
};

enum SVEBroadcastGeneralRegisterOp {
  SVEBroadcastGeneralRegisterFixed = 0x05203800,
  SVEBroadcastGeneralRegisterFMask = 0xFF3FFC00,
  SVEBroadcastGeneralRegisterMask = 0xFF3FFC00,
  DUP_z_r = SVEBroadcastGeneralRegisterFixed
};

enum SVEBroadcastIndexElementOp {
  SVEBroadcastIndexElementFixed = 0x05202000,
  SVEBroadcastIndexElementFMask = 0xFF20FC00,
  SVEBroadcastIndexElementMask = 0xFF20FC00,
  DUP_z_zi = SVEBroadcastIndexElementFixed
};

enum SVEBroadcastIntImm_UnpredicatedOp {
  SVEBroadcastIntImm_UnpredicatedFixed = 0x2538C000,
  SVEBroadcastIntImm_UnpredicatedFMask = 0xFF39C000,
  SVEBroadcastIntImm_UnpredicatedMask = 0xFF3FC000,
  DUP_z_i = SVEBroadcastIntImm_UnpredicatedFixed
};

enum SVECompressActiveElementsOp {
  SVECompressActiveElementsFixed = 0x05A18000,
  SVECompressActiveElementsFMask = 0xFFBFE000,
  SVECompressActiveElementsMask = 0xFFBFE000,
  COMPACT_z_p_z = SVECompressActiveElementsFixed
};

enum SVEConditionallyBroadcastElementToVectorOp {
  SVEConditionallyBroadcastElementToVectorFixed = 0x05288000,
  SVEConditionallyBroadcastElementToVectorFMask = 0xFF3EE000,
  SVEConditionallyBroadcastElementToVectorMask = 0xFF3FE000,
  CLASTA_z_p_zz = SVEConditionallyBroadcastElementToVectorFixed,
  CLASTB_z_p_zz = SVEConditionallyBroadcastElementToVectorFixed | 0x00010000
};

enum SVEConditionallyExtractElementToGeneralRegisterOp {
  SVEConditionallyExtractElementToGeneralRegisterFixed = 0x0530A000,
  SVEConditionallyExtractElementToGeneralRegisterFMask = 0xFF3EE000,
  SVEConditionallyExtractElementToGeneralRegisterMask = 0xFF3FE000,
  CLASTA_r_p_z = SVEConditionallyExtractElementToGeneralRegisterFixed,
  CLASTB_r_p_z = SVEConditionallyExtractElementToGeneralRegisterFixed | 0x00010000
};

enum SVEConditionallyExtractElementToSIMDFPScalarOp {
  SVEConditionallyExtractElementToSIMDFPScalarFixed = 0x052A8000,
  SVEConditionallyExtractElementToSIMDFPScalarFMask = 0xFF3EE000,
  SVEConditionallyExtractElementToSIMDFPScalarMask = 0xFF3FE000,
  CLASTA_v_p_z = SVEConditionallyExtractElementToSIMDFPScalarFixed,
  CLASTB_v_p_z = SVEConditionallyExtractElementToSIMDFPScalarFixed | 0x00010000
};

enum SVEConditionallyTerminateScalarsOp {
  SVEConditionallyTerminateScalarsFixed = 0x25202000,
  SVEConditionallyTerminateScalarsFMask = 0xFF20FC0F,
  SVEConditionallyTerminateScalarsMask = 0xFFA0FC1F,
  CTERMEQ_rr = SVEConditionallyTerminateScalarsFixed | 0x00800000,
  CTERMNE_rr = SVEConditionallyTerminateScalarsFixed | 0x00800010
};

enum SVEConstructivePrefix_UnpredicatedOp {
  SVEConstructivePrefix_UnpredicatedFixed = 0x0420BC00,
  SVEConstructivePrefix_UnpredicatedFMask = 0xFF20FC00,
  SVEConstructivePrefix_UnpredicatedMask = 0xFFFFFC00,
  MOVPRFX_z_z = SVEConstructivePrefix_UnpredicatedFixed
};

enum SVEContiguousFirstFaultLoad_ScalarPlusScalarOp {
  SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed = 0xA4006000,
  SVEContiguousFirstFaultLoad_ScalarPlusScalarFMask = 0xFE00E000,
  SVEContiguousFirstFaultLoad_ScalarPlusScalarMask = 0xFFE0E000,
  LDFF1B_z_p_br_u8 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed,
  LDFF1B_z_p_br_u16 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00200000,
  LDFF1B_z_p_br_u32 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00400000,
  LDFF1B_z_p_br_u64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00600000,
  LDFF1SW_z_p_br_s64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00800000,
  LDFF1H_z_p_br_u16 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00A00000,
  LDFF1H_z_p_br_u32 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00C00000,
  LDFF1H_z_p_br_u64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x00E00000,
  LDFF1SH_z_p_br_s64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01000000,
  LDFF1SH_z_p_br_s32 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01200000,
  LDFF1W_z_p_br_u32 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01400000,
  LDFF1W_z_p_br_u64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01600000,
  LDFF1SB_z_p_br_s64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01800000,
  LDFF1SB_z_p_br_s32 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01A00000,
  LDFF1SB_z_p_br_s16 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01C00000,
  LDFF1D_z_p_br_u64 = SVEContiguousFirstFaultLoad_ScalarPlusScalarFixed | 0x01E00000
};

enum SVEContiguousLoad_ScalarPlusImmOp {
  SVEContiguousLoad_ScalarPlusImmFixed = 0xA400A000,
  SVEContiguousLoad_ScalarPlusImmFMask = 0xFE10E000,
  SVEContiguousLoad_ScalarPlusImmMask = 0xFFF0E000,
  LD1B_z_p_bi_u8 = SVEContiguousLoad_ScalarPlusImmFixed,
  LD1B_z_p_bi_u16 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00200000,
  LD1B_z_p_bi_u32 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00400000,
  LD1B_z_p_bi_u64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00600000,
  LD1SW_z_p_bi_s64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00800000,
  LD1H_z_p_bi_u16 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00A00000,
  LD1H_z_p_bi_u32 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00C00000,
  LD1H_z_p_bi_u64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x00E00000,
  LD1SH_z_p_bi_s64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01000000,
  LD1SH_z_p_bi_s32 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01200000,
  LD1W_z_p_bi_u32 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01400000,
  LD1W_z_p_bi_u64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01600000,
  LD1SB_z_p_bi_s64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01800000,
  LD1SB_z_p_bi_s32 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01A00000,
  LD1SB_z_p_bi_s16 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01C00000,
  LD1D_z_p_bi_u64 = SVEContiguousLoad_ScalarPlusImmFixed | 0x01E00000
};

enum SVEContiguousLoad_ScalarPlusScalarOp {
  SVEContiguousLoad_ScalarPlusScalarFixed = 0xA4004000,
  SVEContiguousLoad_ScalarPlusScalarFMask = 0xFE00E000,
  SVEContiguousLoad_ScalarPlusScalarMask = 0xFFE0E000,
  LD1B_z_p_br_u8 = SVEContiguousLoad_ScalarPlusScalarFixed,
  LD1B_z_p_br_u16 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00200000,
  LD1B_z_p_br_u32 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00400000,
  LD1B_z_p_br_u64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00600000,
  LD1SW_z_p_br_s64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00800000,
  LD1H_z_p_br_u16 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00A00000,
  LD1H_z_p_br_u32 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00C00000,
  LD1H_z_p_br_u64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x00E00000,
  LD1SH_z_p_br_s64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01000000,
  LD1SH_z_p_br_s32 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01200000,
  LD1W_z_p_br_u32 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01400000,
  LD1W_z_p_br_u64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01600000,
  LD1SB_z_p_br_s64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01800000,
  LD1SB_z_p_br_s32 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01A00000,
  LD1SB_z_p_br_s16 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01C00000,
  LD1D_z_p_br_u64 = SVEContiguousLoad_ScalarPlusScalarFixed | 0x01E00000
};

enum SVEContiguousNonFaultLoad_ScalarPlusImmOp {
  SVEContiguousNonFaultLoad_ScalarPlusImmFixed = 0xA410A000,
  SVEContiguousNonFaultLoad_ScalarPlusImmFMask = 0xFE10E000,
  SVEContiguousNonFaultLoad_ScalarPlusImmMask = 0xFFF0E000,
  LDNF1B_z_p_bi_u8 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed,
  LDNF1B_z_p_bi_u16 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00200000,
  LDNF1B_z_p_bi_u32 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00400000,
  LDNF1B_z_p_bi_u64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00600000,
  LDNF1SW_z_p_bi_s64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00800000,
  LDNF1H_z_p_bi_u16 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00A00000,
  LDNF1H_z_p_bi_u32 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00C00000,
  LDNF1H_z_p_bi_u64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x00E00000,
  LDNF1SH_z_p_bi_s64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01000000,
  LDNF1SH_z_p_bi_s32 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01200000,
  LDNF1W_z_p_bi_u32 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01400000,
  LDNF1W_z_p_bi_u64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01600000,
  LDNF1SB_z_p_bi_s64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01800000,
  LDNF1SB_z_p_bi_s32 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01A00000,
  LDNF1SB_z_p_bi_s16 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01C00000,
  LDNF1D_z_p_bi_u64 = SVEContiguousNonFaultLoad_ScalarPlusImmFixed | 0x01E00000
};

enum SVEContiguousNonTemporalLoad_ScalarPlusImmOp {
  SVEContiguousNonTemporalLoad_ScalarPlusImmFixed = 0xA400E000,
  SVEContiguousNonTemporalLoad_ScalarPlusImmFMask = 0xFE70E000,
  SVEContiguousNonTemporalLoad_ScalarPlusImmMask = 0xFFF0E000,
  LDNT1B_z_p_bi_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusImmFixed,
  LDNT1H_z_p_bi_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusImmFixed | 0x00800000,
  LDNT1W_z_p_bi_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusImmFixed | 0x01000000,
  LDNT1D_z_p_bi_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusImmFixed | 0x01800000
};

enum SVEContiguousNonTemporalLoad_ScalarPlusScalarOp {
  SVEContiguousNonTemporalLoad_ScalarPlusScalarFixed = 0xA400C000,
  SVEContiguousNonTemporalLoad_ScalarPlusScalarFMask = 0xFE60E000,
  SVEContiguousNonTemporalLoad_ScalarPlusScalarMask = 0xFFE0E000,
  LDNT1B_z_p_br_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusScalarFixed,
  LDNT1H_z_p_br_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusScalarFixed | 0x00800000,
  LDNT1W_z_p_br_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusScalarFixed | 0x01000000,
  LDNT1D_z_p_br_contiguous = SVEContiguousNonTemporalLoad_ScalarPlusScalarFixed | 0x01800000
};

enum SVEContiguousNonTemporalStore_ScalarPlusImmOp {
  SVEContiguousNonTemporalStore_ScalarPlusImmFixed = 0xE410E000,
  SVEContiguousNonTemporalStore_ScalarPlusImmFMask = 0xFE70E000,
  SVEContiguousNonTemporalStore_ScalarPlusImmMask = 0xFFF0E000,
  STNT1B_z_p_bi_contiguous = SVEContiguousNonTemporalStore_ScalarPlusImmFixed,
  STNT1H_z_p_bi_contiguous = SVEContiguousNonTemporalStore_ScalarPlusImmFixed | 0x00800000,
  STNT1W_z_p_bi_contiguous = SVEContiguousNonTemporalStore_ScalarPlusImmFixed | 0x01000000,
  STNT1D_z_p_bi_contiguous = SVEContiguousNonTemporalStore_ScalarPlusImmFixed | 0x01800000
};

enum SVEContiguousNonTemporalStore_ScalarPlusScalarOp {
  SVEContiguousNonTemporalStore_ScalarPlusScalarFixed = 0xE4006000,
  SVEContiguousNonTemporalStore_ScalarPlusScalarFMask = 0xFE60E000,
  SVEContiguousNonTemporalStore_ScalarPlusScalarMask = 0xFFE0E000,
  STNT1B_z_p_br_contiguous = SVEContiguousNonTemporalStore_ScalarPlusScalarFixed,
  STNT1H_z_p_br_contiguous = SVEContiguousNonTemporalStore_ScalarPlusScalarFixed | 0x00800000,
  STNT1W_z_p_br_contiguous = SVEContiguousNonTemporalStore_ScalarPlusScalarFixed | 0x01000000,
  STNT1D_z_p_br_contiguous = SVEContiguousNonTemporalStore_ScalarPlusScalarFixed | 0x01800000
};

enum SVEContiguousPrefetch_ScalarPlusImmOp {
  SVEContiguousPrefetch_ScalarPlusImmFixed = 0x85C00000,
  SVEContiguousPrefetch_ScalarPlusImmFMask = 0xFFC08010,
  SVEContiguousPrefetch_ScalarPlusImmMask = 0xFFC0E010,
  PRFB_i_p_bi_s = SVEContiguousPrefetch_ScalarPlusImmFixed,
  PRFH_i_p_bi_s = SVEContiguousPrefetch_ScalarPlusImmFixed | 0x00002000,
  PRFW_i_p_bi_s = SVEContiguousPrefetch_ScalarPlusImmFixed | 0x00004000,
  PRFD_i_p_bi_s = SVEContiguousPrefetch_ScalarPlusImmFixed | 0x00006000
};

enum SVEContiguousPrefetch_ScalarPlusScalarOp {
  SVEContiguousPrefetch_ScalarPlusScalarFixed = 0x8400C000,
  SVEContiguousPrefetch_ScalarPlusScalarFMask = 0xFE60E010,
  SVEContiguousPrefetch_ScalarPlusScalarMask = 0xFFE0E010,
  PRFB_i_p_br_s = SVEContiguousPrefetch_ScalarPlusScalarFixed,
  PRFH_i_p_br_s = SVEContiguousPrefetch_ScalarPlusScalarFixed | 0x00800000,
  PRFW_i_p_br_s = SVEContiguousPrefetch_ScalarPlusScalarFixed | 0x01000000,
  PRFD_i_p_br_s = SVEContiguousPrefetch_ScalarPlusScalarFixed | 0x01800000
};

enum SVEContiguousStore_ScalarPlusImmOp {
  SVEContiguousStore_ScalarPlusImmFixed = 0xE400E000,
  SVEContiguousStore_ScalarPlusImmFMask = 0xFE10E000,
  SVEContiguousStore_ScalarPlusImmMask = 0xFF90E000,
  ST1B_z_p_bi = SVEContiguousStore_ScalarPlusImmFixed,
  ST1H_z_p_bi = SVEContiguousStore_ScalarPlusImmFixed | 0x00800000,
  ST1W_z_p_bi = SVEContiguousStore_ScalarPlusImmFixed | 0x01000000,
  ST1D_z_p_bi = SVEContiguousStore_ScalarPlusImmFixed | 0x01800000
};

enum SVEContiguousStore_ScalarPlusScalarOp {
  SVEContiguousStore_ScalarPlusScalarFixed = 0xE4004000,
  SVEContiguousStore_ScalarPlusScalarFMask = 0xFE00E000,
  SVEContiguousStore_ScalarPlusScalarMask = 0xFF80E000,
  ST1B_z_p_br = SVEContiguousStore_ScalarPlusScalarFixed,
  ST1H_z_p_br = SVEContiguousStore_ScalarPlusScalarFixed | 0x00800000,
  ST1W_z_p_br = SVEContiguousStore_ScalarPlusScalarFixed | 0x01000000,
  ST1D_z_p_br = SVEContiguousStore_ScalarPlusScalarFixed | 0x01800000
};

enum SVECopyFPImm_PredicatedOp {
  SVECopyFPImm_PredicatedFixed = 0x0510C000,
  SVECopyFPImm_PredicatedFMask = 0xFF30E000,
  SVECopyFPImm_PredicatedMask = 0xFF30E000,
  FCPY_z_p_i = SVECopyFPImm_PredicatedFixed
};

enum SVECopyGeneralRegisterToVector_PredicatedOp {
  SVECopyGeneralRegisterToVector_PredicatedFixed = 0x0528A000,
  SVECopyGeneralRegisterToVector_PredicatedFMask = 0xFF3FE000,
  SVECopyGeneralRegisterToVector_PredicatedMask = 0xFF3FE000,
  CPY_z_p_r = SVECopyGeneralRegisterToVector_PredicatedFixed
};

enum SVECopyIntImm_PredicatedOp {
  SVECopyIntImm_PredicatedFixed = 0x05100000,
  SVECopyIntImm_PredicatedFMask = 0xFF308000,
  SVECopyIntImm_PredicatedMask = 0xFF308000,
  CPY_z_p_i = SVECopyIntImm_PredicatedFixed
};

enum SVECopySIMDFPScalarRegisterToVector_PredicatedOp {
  SVECopySIMDFPScalarRegisterToVector_PredicatedFixed = 0x05208000,
  SVECopySIMDFPScalarRegisterToVector_PredicatedFMask = 0xFF3FE000,
  SVECopySIMDFPScalarRegisterToVector_PredicatedMask = 0xFF3FE000,
  CPY_z_p_v = SVECopySIMDFPScalarRegisterToVector_PredicatedFixed
};

enum SVEElementCountOp {
  SVEElementCountFixed = 0x0420E000,
  SVEElementCountFMask = 0xFF30F800,
  SVEElementCountMask = 0xFFF0FC00,
  CNTB_r_s = SVEElementCountFixed,
  CNTH_r_s = SVEElementCountFixed | 0x00400000,
  CNTW_r_s = SVEElementCountFixed | 0x00800000,
  CNTD_r_s = SVEElementCountFixed | 0x00C00000
};

enum SVEExtractElementToGeneralRegisterOp {
  SVEExtractElementToGeneralRegisterFixed = 0x0520A000,
  SVEExtractElementToGeneralRegisterFMask = 0xFF3EE000,
  SVEExtractElementToGeneralRegisterMask = 0xFF3FE000,
  LASTA_r_p_z = SVEExtractElementToGeneralRegisterFixed,
  LASTB_r_p_z = SVEExtractElementToGeneralRegisterFixed | 0x00010000
};

enum SVEExtractElementToSIMDFPScalarRegisterOp {
  SVEExtractElementToSIMDFPScalarRegisterFixed = 0x05228000,
  SVEExtractElementToSIMDFPScalarRegisterFMask = 0xFF3EE000,
  SVEExtractElementToSIMDFPScalarRegisterMask = 0xFF3FE000,
  LASTA_v_p_z = SVEExtractElementToSIMDFPScalarRegisterFixed,
  LASTB_v_p_z = SVEExtractElementToSIMDFPScalarRegisterFixed | 0x00010000
};

enum SVEFFRInitialiseOp {
  SVEFFRInitialiseFixed = 0x252C9000,
  SVEFFRInitialiseFMask = 0xFF3FFFFF,
  SVEFFRInitialiseMask = 0xFFFFFFFF,
  SETFFR_f = SVEFFRInitialiseFixed
};

enum SVEFFRWriteFromPredicateOp {
  SVEFFRWriteFromPredicateFixed = 0x25289000,
  SVEFFRWriteFromPredicateFMask = 0xFF3FFE1F,
  SVEFFRWriteFromPredicateMask = 0xFFFFFE1F,
  WRFFR_f_p = SVEFFRWriteFromPredicateFixed
};

enum SVEFPAccumulatingReductionOp {
  SVEFPAccumulatingReductionFixed = 0x65182000,
  SVEFPAccumulatingReductionFMask = 0xFF38E000,
  SVEFPAccumulatingReductionMask = 0xFF3FE000,
  FADDA_v_p_z = SVEFPAccumulatingReductionFixed
};

enum SVEFPArithmeticUnpredicatedOp {
  SVEFPArithmeticUnpredicatedFixed = 0x65000000,
  SVEFPArithmeticUnpredicatedFMask = 0xFF20E000,
  SVEFPArithmeticUnpredicatedMask = 0xFF20FC00,
  FADD_z_zz = SVEFPArithmeticUnpredicatedFixed,
  FSUB_z_zz = SVEFPArithmeticUnpredicatedFixed | 0x00000400,
  FMUL_z_zz = SVEFPArithmeticUnpredicatedFixed | 0x00000800,
  FTSMUL_z_zz = SVEFPArithmeticUnpredicatedFixed | 0x00000C00,
  FRECPS_z_zz = SVEFPArithmeticUnpredicatedFixed | 0x00001800,
  FRSQRTS_z_zz = SVEFPArithmeticUnpredicatedFixed | 0x00001C00
};

enum SVEFPArithmeticWithImm_PredicatedOp {
  SVEFPArithmeticWithImm_PredicatedFixed = 0x65188000,
  SVEFPArithmeticWithImm_PredicatedFMask = 0xFF38E3C0,
  SVEFPArithmeticWithImm_PredicatedMask = 0xFF3FE3C0,
  FADD_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed,
  FSUB_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00010000,
  FMUL_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00020000,
  FSUBR_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00030000,
  FMAXNM_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00040000,
  FMINNM_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00050000,
  FMAX_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00060000,
  FMIN_z_p_zs = SVEFPArithmeticWithImm_PredicatedFixed | 0x00070000
};

enum SVEFPArithmetic_PredicatedOp {
  SVEFPArithmetic_PredicatedFixed = 0x65008000,
  SVEFPArithmetic_PredicatedFMask = 0xFF30E000,
  SVEFPArithmetic_PredicatedMask = 0xFF3FE000,
  FADD_z_p_zz = SVEFPArithmetic_PredicatedFixed,
  FSUB_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00010000,
  FMUL_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00020000,
  FSUBR_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00030000,
  FMAXNM_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00040000,
  FMINNM_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00050000,
  FMAX_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00060000,
  FMIN_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00070000,
  FABD_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00080000,
  FSCALE_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x00090000,
  FMULX_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x000A0000,
  FDIVR_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x000C0000,
  FDIV_z_p_zz = SVEFPArithmetic_PredicatedFixed | 0x000D0000
};

enum SVEFPCompareVectorsOp {
  SVEFPCompareVectorsFixed = 0x65004000,
  SVEFPCompareVectorsFMask = 0xFF204000,
  SVEFPCompareVectorsMask = 0xFF20E010,
  FCMGE_p_p_zz = SVEFPCompareVectorsFixed,
  FCMGT_p_p_zz = SVEFPCompareVectorsFixed | 0x00000010,
  FCMEQ_p_p_zz = SVEFPCompareVectorsFixed | 0x00002000,
  FCMNE_p_p_zz = SVEFPCompareVectorsFixed | 0x00002010,
  FCMUO_p_p_zz = SVEFPCompareVectorsFixed | 0x00008000,
  FACGE_p_p_zz = SVEFPCompareVectorsFixed | 0x00008010,
  FACGT_p_p_zz = SVEFPCompareVectorsFixed | 0x0000A010
};

enum SVEFPCompareWithZeroOp {
  SVEFPCompareWithZeroFixed = 0x65102000,
  SVEFPCompareWithZeroFMask = 0xFF38E000,
  SVEFPCompareWithZeroMask = 0xFF3FE010,
  FCMGE_p_p_z0 = SVEFPCompareWithZeroFixed,
  FCMGT_p_p_z0 = SVEFPCompareWithZeroFixed | 0x00000010,
  FCMLT_p_p_z0 = SVEFPCompareWithZeroFixed | 0x00010000,
  FCMLE_p_p_z0 = SVEFPCompareWithZeroFixed | 0x00010010,
  FCMEQ_p_p_z0 = SVEFPCompareWithZeroFixed | 0x00020000,
  FCMNE_p_p_z0 = SVEFPCompareWithZeroFixed | 0x00030000
};

enum SVEFPComplexAdditionOp {
  SVEFPComplexAdditionFixed = 0x64008000,
  SVEFPComplexAdditionFMask = 0xFF3EE000,
  SVEFPComplexAdditionMask = 0xFF3EE000,
  FCADD_z_p_zz = SVEFPComplexAdditionFixed
};

enum SVEFPComplexMulAddOp {
  SVEFPComplexMulAddFixed = 0x64000000,
  SVEFPComplexMulAddFMask = 0xFF208000,
  SVEFPComplexMulAddMask = 0xFF208000,
  FCMLA_z_p_zzz = SVEFPComplexMulAddFixed
};

enum SVEFPComplexMulAddIndexOp {
  SVEFPComplexMulAddIndexFixed = 0x64201000,
  SVEFPComplexMulAddIndexFMask = 0xFF20F000,
  SVEFPComplexMulAddIndexMask = 0xFFE0F000,
  FCMLA_z_zzzi_h = SVEFPComplexMulAddIndexFixed | 0x00800000,
  FCMLA_z_zzzi_s = SVEFPComplexMulAddIndexFixed | 0x00C00000
};

enum SVEFPConvertPrecisionOp {
  SVEFPConvertPrecisionFixed = 0x6508A000,
  SVEFPConvertPrecisionFMask = 0xFF3CE000,
  SVEFPConvertPrecisionMask = 0xFFFFE000,
  FCVT_z_p_z_s2h = SVEFPConvertPrecisionFixed | 0x00800000,
  FCVT_z_p_z_h2s = SVEFPConvertPrecisionFixed | 0x00810000,
  FCVT_z_p_z_d2h = SVEFPConvertPrecisionFixed | 0x00C00000,
  FCVT_z_p_z_h2d = SVEFPConvertPrecisionFixed | 0x00C10000,
  FCVT_z_p_z_d2s = SVEFPConvertPrecisionFixed | 0x00C20000,
  FCVT_z_p_z_s2d = SVEFPConvertPrecisionFixed | 0x00C30000
};

enum SVEFPConvertToIntOp {
  SVEFPConvertToIntFixed = 0x6518A000,
  SVEFPConvertToIntFMask = 0xFF38E000,
  SVEFPConvertToIntMask = 0xFFFFE000,
  FCVTZS_z_p_z_fp162h = SVEFPConvertToIntFixed | 0x00420000,
  FCVTZU_z_p_z_fp162h = SVEFPConvertToIntFixed | 0x00430000,
  FCVTZS_z_p_z_fp162w = SVEFPConvertToIntFixed | 0x00440000,
  FCVTZU_z_p_z_fp162w = SVEFPConvertToIntFixed | 0x00450000,
  FCVTZS_z_p_z_fp162x = SVEFPConvertToIntFixed | 0x00460000,
  FCVTZU_z_p_z_fp162x = SVEFPConvertToIntFixed | 0x00470000,
  FCVTZS_z_p_z_s2w = SVEFPConvertToIntFixed | 0x00840000,
  FCVTZU_z_p_z_s2w = SVEFPConvertToIntFixed | 0x00850000,
  FCVTZS_z_p_z_d2w = SVEFPConvertToIntFixed | 0x00C00000,
  FCVTZU_z_p_z_d2w = SVEFPConvertToIntFixed | 0x00C10000,
  FCVTZS_z_p_z_s2x = SVEFPConvertToIntFixed | 0x00C40000,
  FCVTZU_z_p_z_s2x = SVEFPConvertToIntFixed | 0x00C50000,
  FCVTZS_z_p_z_d2x = SVEFPConvertToIntFixed | 0x00C60000,
  FCVTZU_z_p_z_d2x = SVEFPConvertToIntFixed | 0x00C70000
};

enum SVEFPExponentialAcceleratorOp {
  SVEFPExponentialAcceleratorFixed = 0x0420B800,
  SVEFPExponentialAcceleratorFMask = 0xFF20FC00,
  SVEFPExponentialAcceleratorMask = 0xFF3FFC00,
  FEXPA_z_z = SVEFPExponentialAcceleratorFixed
};

enum SVEFPFastReductionOp {
  SVEFPFastReductionFixed = 0x65002000,
  SVEFPFastReductionFMask = 0xFF38E000,
  SVEFPFastReductionMask = 0xFF3FE000,
  FADDV_v_p_z = SVEFPFastReductionFixed,
  FMAXNMV_v_p_z = SVEFPFastReductionFixed | 0x00040000,
  FMINNMV_v_p_z = SVEFPFastReductionFixed | 0x00050000,
  FMAXV_v_p_z = SVEFPFastReductionFixed | 0x00060000,
  FMINV_v_p_z = SVEFPFastReductionFixed | 0x00070000
};

enum SVEFPMulAddOp {
  SVEFPMulAddFixed = 0x65200000,
  SVEFPMulAddFMask = 0xFF200000,
  SVEFPMulAddMask = 0xFF20E000,
  FMLA_z_p_zzz = SVEFPMulAddFixed,
  FMLS_z_p_zzz = SVEFPMulAddFixed | 0x00002000,
  FNMLA_z_p_zzz = SVEFPMulAddFixed | 0x00004000,
  FNMLS_z_p_zzz = SVEFPMulAddFixed | 0x00006000,
  FMAD_z_p_zzz = SVEFPMulAddFixed | 0x00008000,
  FMSB_z_p_zzz = SVEFPMulAddFixed | 0x0000A000,
  FNMAD_z_p_zzz = SVEFPMulAddFixed | 0x0000C000,
  FNMSB_z_p_zzz = SVEFPMulAddFixed | 0x0000E000
};

enum SVEFPMulAddIndexOp {
  SVEFPMulAddIndexFixed = 0x64200000,
  SVEFPMulAddIndexFMask = 0xFF20F800,
  SVEFPMulAddIndexMask = 0xFFE0FC00,
  FMLA_z_zzzi_h = SVEFPMulAddIndexFixed,
  FMLA_z_zzzi_h_i3h = FMLA_z_zzzi_h | 0x00400000,
  FMLS_z_zzzi_h = SVEFPMulAddIndexFixed | 0x00000400,
  FMLS_z_zzzi_h_i3h = FMLS_z_zzzi_h | 0x00400000,
  FMLA_z_zzzi_s = SVEFPMulAddIndexFixed | 0x00800000,
  FMLS_z_zzzi_s = SVEFPMulAddIndexFixed | 0x00800400,
  FMLA_z_zzzi_d = SVEFPMulAddIndexFixed | 0x00C00000,
  FMLS_z_zzzi_d = SVEFPMulAddIndexFixed | 0x00C00400
};

enum SVEFPMulIndexOp {
  SVEFPMulIndexFixed = 0x64202000,
  SVEFPMulIndexFMask = 0xFF20FC00,
  SVEFPMulIndexMask = 0xFFE0FC00,
  FMUL_z_zzi_h = SVEFPMulIndexFixed,
  FMUL_z_zzi_h_i3h = FMUL_z_zzi_h | 0x00400000,
  FMUL_z_zzi_s = SVEFPMulIndexFixed | 0x00800000,
  FMUL_z_zzi_d = SVEFPMulIndexFixed | 0x00C00000
};

enum SVEFPRoundToIntegralValueOp {
  SVEFPRoundToIntegralValueFixed = 0x6500A000,
  SVEFPRoundToIntegralValueFMask = 0xFF38E000,
  SVEFPRoundToIntegralValueMask = 0xFF3FE000,
  FRINTN_z_p_z = SVEFPRoundToIntegralValueFixed,
  FRINTP_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00010000,
  FRINTM_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00020000,
  FRINTZ_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00030000,
  FRINTA_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00040000,
  FRINTX_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00060000,
  FRINTI_z_p_z = SVEFPRoundToIntegralValueFixed | 0x00070000
};

enum SVEFPTrigMulAddCoefficientOp {
  SVEFPTrigMulAddCoefficientFixed = 0x65108000,
  SVEFPTrigMulAddCoefficientFMask = 0xFF38FC00,
  SVEFPTrigMulAddCoefficientMask = 0xFF38FC00,
  FTMAD_z_zzi = SVEFPTrigMulAddCoefficientFixed
};

enum SVEFPTrigSelectCoefficientOp {
  SVEFPTrigSelectCoefficientFixed = 0x0420B000,
  SVEFPTrigSelectCoefficientFMask = 0xFF20F800,
  SVEFPTrigSelectCoefficientMask = 0xFF20FC00,
  FTSSEL_z_zz = SVEFPTrigSelectCoefficientFixed
};

enum SVEFPUnaryOpOp {
  SVEFPUnaryOpFixed = 0x650CA000,
  SVEFPUnaryOpFMask = 0xFF3CE000,
  SVEFPUnaryOpMask = 0xFF3FE000,
  FRECPX_z_p_z = SVEFPUnaryOpFixed,
  FSQRT_z_p_z = SVEFPUnaryOpFixed | 0x00010000
};

enum SVEFPUnaryOpUnpredicatedOp {
  SVEFPUnaryOpUnpredicatedFixed = 0x65083000,
  SVEFPUnaryOpUnpredicatedFMask = 0xFF38F000,
  SVEFPUnaryOpUnpredicatedMask = 0xFF3FFC00,
  FRECPE_z_z = SVEFPUnaryOpUnpredicatedFixed | 0x00060000,
  FRSQRTE_z_z = SVEFPUnaryOpUnpredicatedFixed | 0x00070000
};

enum SVEIncDecByPredicateCountOp {
  SVEIncDecByPredicateCountFixed = 0x25288000,
  SVEIncDecByPredicateCountFMask = 0xFF38F000,
  SVEIncDecByPredicateCountMask = 0xFF3FFE00,
  SQINCP_z_p_z = SVEIncDecByPredicateCountFixed,
  SQINCP_r_p_r_sx = SVEIncDecByPredicateCountFixed | 0x00000800,
  SQINCP_r_p_r_x = SVEIncDecByPredicateCountFixed | 0x00000C00,
  UQINCP_z_p_z = SVEIncDecByPredicateCountFixed | 0x00010000,
  UQINCP_r_p_r_uw = SVEIncDecByPredicateCountFixed | 0x00010800,
  UQINCP_r_p_r_x = SVEIncDecByPredicateCountFixed | 0x00010C00,
  SQDECP_z_p_z = SVEIncDecByPredicateCountFixed | 0x00020000,
  SQDECP_r_p_r_sx = SVEIncDecByPredicateCountFixed | 0x00020800,
  SQDECP_r_p_r_x = SVEIncDecByPredicateCountFixed | 0x00020C00,
  UQDECP_z_p_z = SVEIncDecByPredicateCountFixed | 0x00030000,
  UQDECP_r_p_r_uw = SVEIncDecByPredicateCountFixed | 0x00030800,
  UQDECP_r_p_r_x = SVEIncDecByPredicateCountFixed | 0x00030C00,
  INCP_z_p_z = SVEIncDecByPredicateCountFixed | 0x00040000,
  INCP_r_p_r = SVEIncDecByPredicateCountFixed | 0x00040800,
  DECP_z_p_z = SVEIncDecByPredicateCountFixed | 0x00050000,
  DECP_r_p_r = SVEIncDecByPredicateCountFixed | 0x00050800
};

enum SVEIncDecRegisterByElementCountOp {
  SVEIncDecRegisterByElementCountFixed = 0x0430E000,
  SVEIncDecRegisterByElementCountFMask = 0xFF30F800,
  SVEIncDecRegisterByElementCountMask = 0xFFF0FC00,
  INCB_r_rs = SVEIncDecRegisterByElementCountFixed,
  DECB_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00000400,
  INCH_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00400000,
  DECH_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00400400,
  INCW_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00800000,
  DECW_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00800400,
  INCD_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00C00000,
  DECD_r_rs = SVEIncDecRegisterByElementCountFixed | 0x00C00400
};

enum SVEIncDecVectorByElementCountOp {
  SVEIncDecVectorByElementCountFixed = 0x0430C000,
  SVEIncDecVectorByElementCountFMask = 0xFF30F800,
  SVEIncDecVectorByElementCountMask = 0xFFF0FC00,
  INCH_z_zs = SVEIncDecVectorByElementCountFixed | 0x00400000,
  DECH_z_zs = SVEIncDecVectorByElementCountFixed | 0x00400400,
  INCW_z_zs = SVEIncDecVectorByElementCountFixed | 0x00800000,
  DECW_z_zs = SVEIncDecVectorByElementCountFixed | 0x00800400,
  INCD_z_zs = SVEIncDecVectorByElementCountFixed | 0x00C00000,
  DECD_z_zs = SVEIncDecVectorByElementCountFixed | 0x00C00400
};

enum SVEIndexGenerationOp {
  SVEIndexGenerationFixed = 0x04204000,
  SVEIndexGenerationFMask = 0xFF20F000,
  SVEIndexGenerationMask = 0xFF20FC00,
  INDEX_z_ii = SVEIndexGenerationFixed,
  INDEX_z_ri = SVEIndexGenerationFixed | 0x00000400,
  INDEX_z_ir = SVEIndexGenerationFixed | 0x00000800,
  INDEX_z_rr = SVEIndexGenerationFixed | 0x00000C00
};

enum SVEInsertGeneralRegisterOp {
  SVEInsertGeneralRegisterFixed = 0x05243800,
  SVEInsertGeneralRegisterFMask = 0xFF3FFC00,
  SVEInsertGeneralRegisterMask = 0xFF3FFC00,
  INSR_z_r = SVEInsertGeneralRegisterFixed
};

enum SVEInsertSIMDFPScalarRegisterOp {
  SVEInsertSIMDFPScalarRegisterFixed = 0x05343800,
  SVEInsertSIMDFPScalarRegisterFMask = 0xFF3FFC00,
  SVEInsertSIMDFPScalarRegisterMask = 0xFF3FFC00,
  INSR_z_v = SVEInsertSIMDFPScalarRegisterFixed
};

enum SVEIntAddSubtractImm_UnpredicatedOp {
  SVEIntAddSubtractImm_UnpredicatedFixed = 0x2520C000,
  SVEIntAddSubtractImm_UnpredicatedFMask = 0xFF38C000,
  SVEIntAddSubtractImm_UnpredicatedMask = 0xFF3FC000,
  ADD_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed,
  SUB_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00010000,
  SUBR_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00030000,
  SQADD_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00040000,
  UQADD_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00050000,
  SQSUB_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00060000,
  UQSUB_z_zi = SVEIntAddSubtractImm_UnpredicatedFixed | 0x00070000
};

enum SVEIntAddSubtractVectors_PredicatedOp {
  SVEIntAddSubtractVectors_PredicatedFixed = 0x04000000,
  SVEIntAddSubtractVectors_PredicatedFMask = 0xFF38E000,
  SVEIntAddSubtractVectors_PredicatedMask = 0xFF3FE000,
  ADD_z_p_zz = SVEIntAddSubtractVectors_PredicatedFixed,
  SUB_z_p_zz = SVEIntAddSubtractVectors_PredicatedFixed | 0x00010000,
  SUBR_z_p_zz = SVEIntAddSubtractVectors_PredicatedFixed | 0x00030000
};

enum SVEIntArithmeticUnpredicatedOp {
  SVEIntArithmeticUnpredicatedFixed = 0x04200000,
  SVEIntArithmeticUnpredicatedFMask = 0xFF20E000,
  SVEIntArithmeticUnpredicatedMask = 0xFF20FC00,
  ADD_z_zz = SVEIntArithmeticUnpredicatedFixed,
  SUB_z_zz = SVEIntArithmeticUnpredicatedFixed | 0x00000400,
  SQADD_z_zz = SVEIntArithmeticUnpredicatedFixed | 0x00001000,
  UQADD_z_zz = SVEIntArithmeticUnpredicatedFixed | 0x00001400,
  SQSUB_z_zz = SVEIntArithmeticUnpredicatedFixed | 0x00001800,
  UQSUB_z_zz = SVEIntArithmeticUnpredicatedFixed | 0x00001C00
};

enum SVEIntCompareScalarCountAndLimitOp {
  SVEIntCompareScalarCountAndLimitFixed = 0x25200000,
  SVEIntCompareScalarCountAndLimitFMask = 0xFF20E000,
  SVEIntCompareScalarCountAndLimitMask = 0xFF20EC10,
  WHILELT_p_p_rr = SVEIntCompareScalarCountAndLimitFixed | 0x00000400,
  WHILELE_p_p_rr = SVEIntCompareScalarCountAndLimitFixed | 0x00000410,
  WHILELO_p_p_rr = SVEIntCompareScalarCountAndLimitFixed | 0x00000C00,
  WHILELS_p_p_rr = SVEIntCompareScalarCountAndLimitFixed | 0x00000C10
};

enum SVEIntCompareSignedImmOp {
  SVEIntCompareSignedImmFixed = 0x25000000,
  SVEIntCompareSignedImmFMask = 0xFF204000,
  SVEIntCompareSignedImmMask = 0xFF20E010,
  CMPGE_p_p_zi = SVEIntCompareSignedImmFixed,
  CMPGT_p_p_zi = SVEIntCompareSignedImmFixed | 0x00000010,
  CMPLT_p_p_zi = SVEIntCompareSignedImmFixed | 0x00002000,
  CMPLE_p_p_zi = SVEIntCompareSignedImmFixed | 0x00002010,
  CMPEQ_p_p_zi = SVEIntCompareSignedImmFixed | 0x00008000,
  CMPNE_p_p_zi = SVEIntCompareSignedImmFixed | 0x00008010
};

enum SVEIntCompareUnsignedImmOp {
  SVEIntCompareUnsignedImmFixed = 0x24200000,
  SVEIntCompareUnsignedImmFMask = 0xFF200000,
  SVEIntCompareUnsignedImmMask = 0xFF202010,
  CMPHS_p_p_zi = SVEIntCompareUnsignedImmFixed,
  CMPHI_p_p_zi = SVEIntCompareUnsignedImmFixed | 0x00000010,
  CMPLO_p_p_zi = SVEIntCompareUnsignedImmFixed | 0x00002000,
  CMPLS_p_p_zi = SVEIntCompareUnsignedImmFixed | 0x00002010
};

enum SVEIntCompareVectorsOp {
  SVEIntCompareVectorsFixed = 0x24000000,
  SVEIntCompareVectorsFMask = 0xFF200000,
  SVEIntCompareVectorsMask = 0xFF20E010,
  CMPHS_p_p_zz = SVEIntCompareVectorsFixed,
  CMPHI_p_p_zz = SVEIntCompareVectorsFixed | 0x00000010,
  CMPEQ_p_p_zw = SVEIntCompareVectorsFixed | 0x00002000,
  CMPNE_p_p_zw = SVEIntCompareVectorsFixed | 0x00002010,
  CMPGE_p_p_zw = SVEIntCompareVectorsFixed | 0x00004000,
  CMPGT_p_p_zw = SVEIntCompareVectorsFixed | 0x00004010,
  CMPLT_p_p_zw = SVEIntCompareVectorsFixed | 0x00006000,
  CMPLE_p_p_zw = SVEIntCompareVectorsFixed | 0x00006010,
  CMPGE_p_p_zz = SVEIntCompareVectorsFixed | 0x00008000,
  CMPGT_p_p_zz = SVEIntCompareVectorsFixed | 0x00008010,
  CMPEQ_p_p_zz = SVEIntCompareVectorsFixed | 0x0000A000,
  CMPNE_p_p_zz = SVEIntCompareVectorsFixed | 0x0000A010,
  CMPHS_p_p_zw = SVEIntCompareVectorsFixed | 0x0000C000,
  CMPHI_p_p_zw = SVEIntCompareVectorsFixed | 0x0000C010,
  CMPLO_p_p_zw = SVEIntCompareVectorsFixed | 0x0000E000,
  CMPLS_p_p_zw = SVEIntCompareVectorsFixed | 0x0000E010
};

enum SVEIntConvertToFPOp {
  SVEIntConvertToFPFixed = 0x6510A000,
  SVEIntConvertToFPFMask = 0xFF38E000,
  SVEIntConvertToFPMask = 0xFFFFE000,
  SCVTF_z_p_z_h2fp16 = SVEIntConvertToFPFixed | 0x00420000,
  UCVTF_z_p_z_h2fp16 = SVEIntConvertToFPFixed | 0x00430000,
  SCVTF_z_p_z_w2fp16 = SVEIntConvertToFPFixed | 0x00440000,
  UCVTF_z_p_z_w2fp16 = SVEIntConvertToFPFixed | 0x00450000,
  SCVTF_z_p_z_x2fp16 = SVEIntConvertToFPFixed | 0x00460000,
  UCVTF_z_p_z_x2fp16 = SVEIntConvertToFPFixed | 0x00470000,
  SCVTF_z_p_z_w2s = SVEIntConvertToFPFixed | 0x00840000,
  UCVTF_z_p_z_w2s = SVEIntConvertToFPFixed | 0x00850000,
  SCVTF_z_p_z_w2d = SVEIntConvertToFPFixed | 0x00C00000,
  UCVTF_z_p_z_w2d = SVEIntConvertToFPFixed | 0x00C10000,
  SCVTF_z_p_z_x2s = SVEIntConvertToFPFixed | 0x00C40000,
  UCVTF_z_p_z_x2s = SVEIntConvertToFPFixed | 0x00C50000,
  SCVTF_z_p_z_x2d = SVEIntConvertToFPFixed | 0x00C60000,
  UCVTF_z_p_z_x2d = SVEIntConvertToFPFixed | 0x00C70000
};

enum SVEIntDivideVectors_PredicatedOp {
  SVEIntDivideVectors_PredicatedFixed = 0x04140000,
  SVEIntDivideVectors_PredicatedFMask = 0xFF3CE000,
  SVEIntDivideVectors_PredicatedMask = 0xFF3FE000,
  SDIV_z_p_zz = SVEIntDivideVectors_PredicatedFixed,
  UDIV_z_p_zz = SVEIntDivideVectors_PredicatedFixed | 0x00010000,
  SDIVR_z_p_zz = SVEIntDivideVectors_PredicatedFixed | 0x00020000,
  UDIVR_z_p_zz = SVEIntDivideVectors_PredicatedFixed | 0x00030000
};

enum SVEIntMinMaxDifference_PredicatedOp {
  SVEIntMinMaxDifference_PredicatedFixed = 0x04080000,
  SVEIntMinMaxDifference_PredicatedFMask = 0xFF38E000,
  SVEIntMinMaxDifference_PredicatedMask = 0xFF3FE000,
  SMAX_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed,
  UMAX_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed | 0x00010000,
  SMIN_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed | 0x00020000,
  UMIN_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed | 0x00030000,
  SABD_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed | 0x00040000,
  UABD_z_p_zz = SVEIntMinMaxDifference_PredicatedFixed | 0x00050000
};

enum SVEIntMinMaxImm_UnpredicatedOp {
  SVEIntMinMaxImm_UnpredicatedFixed = 0x2528C000,
  SVEIntMinMaxImm_UnpredicatedFMask = 0xFF38C000,
  SVEIntMinMaxImm_UnpredicatedMask = 0xFF3FE000,
  SMAX_z_zi = SVEIntMinMaxImm_UnpredicatedFixed,
  UMAX_z_zi = SVEIntMinMaxImm_UnpredicatedFixed | 0x00010000,
  SMIN_z_zi = SVEIntMinMaxImm_UnpredicatedFixed | 0x00020000,
  UMIN_z_zi = SVEIntMinMaxImm_UnpredicatedFixed | 0x00030000
};

enum SVEIntMulAddPredicatedOp {
  SVEIntMulAddPredicatedFixed = 0x04004000,
  SVEIntMulAddPredicatedFMask = 0xFF204000,
  SVEIntMulAddPredicatedMask = 0xFF20E000,
  MLA_z_p_zzz = SVEIntMulAddPredicatedFixed,
  MLS_z_p_zzz = SVEIntMulAddPredicatedFixed | 0x00002000,
  MAD_z_p_zzz = SVEIntMulAddPredicatedFixed | 0x00008000,
  MSB_z_p_zzz = SVEIntMulAddPredicatedFixed | 0x0000A000
};

enum SVEIntMulAddUnpredicatedOp {
  SVEIntMulAddUnpredicatedFixed = 0x44000000,
  SVEIntMulAddUnpredicatedFMask = 0xFF208000,
  SVEIntMulAddUnpredicatedMask = 0xFF20FC00,
  SDOT_z_zzz = SVEIntMulAddUnpredicatedFixed,
  UDOT_z_zzz = SVEIntMulAddUnpredicatedFixed | 0x00000400
};

enum SVEIntMulImm_UnpredicatedOp {
  SVEIntMulImm_UnpredicatedFixed = 0x2530C000,
  SVEIntMulImm_UnpredicatedFMask = 0xFF38C000,
  SVEIntMulImm_UnpredicatedMask = 0xFF3FE000,
  MUL_z_zi = SVEIntMulImm_UnpredicatedFixed
};

enum SVEIntMulVectors_PredicatedOp {
  SVEIntMulVectors_PredicatedFixed = 0x04100000,
  SVEIntMulVectors_PredicatedFMask = 0xFF3CE000,
  SVEIntMulVectors_PredicatedMask = 0xFF3FE000,
  MUL_z_p_zz = SVEIntMulVectors_PredicatedFixed,
  SMULH_z_p_zz = SVEIntMulVectors_PredicatedFixed | 0x00020000,
  UMULH_z_p_zz = SVEIntMulVectors_PredicatedFixed | 0x00030000
};

enum SVEMovprfxOp {
  SVEMovprfxFixed = 0x04002000,
  SVEMovprfxFMask = 0xFF20E000,
  SVEMovprfxMask = 0xFF3EE000,
  MOVPRFX_z_p_z = SVEMovprfxFixed | 0x00100000
};

enum SVEIntReductionOp {
  SVEIntReductionFixed = 0x04002000,
  SVEIntReductionFMask = 0xFF20E000,
  SVEIntReductionMask = 0xFF3FE000,
  SADDV_r_p_z = SVEIntReductionFixed,
  UADDV_r_p_z = SVEIntReductionFixed | 0x00010000,
  SMAXV_r_p_z = SVEIntReductionFixed | 0x00080000,
  UMAXV_r_p_z = SVEIntReductionFixed | 0x00090000,
  SMINV_r_p_z = SVEIntReductionFixed | 0x000A0000,
  UMINV_r_p_z = SVEIntReductionFixed | 0x000B0000
};

enum SVEIntReductionLogicalOp {
  SVEIntReductionLogicalFixed = 0x04182000,
  SVEIntReductionLogicalFMask = 0xFF38E000,
  SVEIntReductionLogicalMask = 0xFF3FE000,
  ORV_r_p_z = SVEIntReductionLogicalFixed | 0x00180000,
  EORV_r_p_z = SVEIntReductionLogicalFixed | 0x00190000,
  ANDV_r_p_z = SVEIntReductionLogicalFixed | 0x001A0000
};

enum SVEIntUnaryArithmeticPredicatedOp {
  SVEIntUnaryArithmeticPredicatedFixed = 0x0400A000,
  SVEIntUnaryArithmeticPredicatedFMask = 0xFF20E000,
  SVEIntUnaryArithmeticPredicatedMask = 0xFF3FE000,
  SXTB_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00100000,
  UXTB_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00110000,
  SXTH_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00120000,
  UXTH_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00130000,
  SXTW_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00140000,
  UXTW_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00150000,
  ABS_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00160000,
  NEG_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00170000,
  CLS_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00180000,
  CLZ_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x00190000,
  CNT_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x001A0000,
  CNOT_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x001B0000,
  FABS_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x001C0000,
  FNEG_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x001D0000,
  NOT_z_p_z = SVEIntUnaryArithmeticPredicatedFixed | 0x001E0000
};

enum SVELoadAndBroadcastElementOp {
  SVELoadAndBroadcastElementFixed = 0x84408000,
  SVELoadAndBroadcastElementFMask = 0xFE408000,
  SVELoadAndBroadcastElementMask = 0xFFC0E000,
  LD1RB_z_p_bi_u8 = SVELoadAndBroadcastElementFixed,
  LD1RB_z_p_bi_u16 = SVELoadAndBroadcastElementFixed | 0x00002000,
  LD1RB_z_p_bi_u32 = SVELoadAndBroadcastElementFixed | 0x00004000,
  LD1RB_z_p_bi_u64 = SVELoadAndBroadcastElementFixed | 0x00006000,
  LD1RSW_z_p_bi_s64 = SVELoadAndBroadcastElementFixed | 0x00800000,
  LD1RH_z_p_bi_u16 = SVELoadAndBroadcastElementFixed | 0x00802000,
  LD1RH_z_p_bi_u32 = SVELoadAndBroadcastElementFixed | 0x00804000,
  LD1RH_z_p_bi_u64 = SVELoadAndBroadcastElementFixed | 0x00806000,
  LD1RSH_z_p_bi_s64 = SVELoadAndBroadcastElementFixed | 0x01000000,
  LD1RSH_z_p_bi_s32 = SVELoadAndBroadcastElementFixed | 0x01002000,
  LD1RW_z_p_bi_u32 = SVELoadAndBroadcastElementFixed | 0x01004000,
  LD1RW_z_p_bi_u64 = SVELoadAndBroadcastElementFixed | 0x01006000,
  LD1RSB_z_p_bi_s64 = SVELoadAndBroadcastElementFixed | 0x01800000,
  LD1RSB_z_p_bi_s32 = SVELoadAndBroadcastElementFixed | 0x01802000,
  LD1RSB_z_p_bi_s16 = SVELoadAndBroadcastElementFixed | 0x01804000,
  LD1RD_z_p_bi_u64 = SVELoadAndBroadcastElementFixed | 0x01806000
};

enum SVELoadAndBroadcastQuadword_ScalarPlusImmOp {
  SVELoadAndBroadcastQuadword_ScalarPlusImmFixed = 0xA4002000,
  SVELoadAndBroadcastQuadword_ScalarPlusImmFMask = 0xFE10E000,
  SVELoadAndBroadcastQuadword_ScalarPlusImmMask = 0xFFF0E000,
  LD1RQB_z_p_bi_u8 = SVELoadAndBroadcastQuadword_ScalarPlusImmFixed,
  LD1RQH_z_p_bi_u16 = SVELoadAndBroadcastQuadword_ScalarPlusImmFixed | 0x00800000,
  LD1RQW_z_p_bi_u32 = SVELoadAndBroadcastQuadword_ScalarPlusImmFixed | 0x01000000,
  LD1RQD_z_p_bi_u64 = SVELoadAndBroadcastQuadword_ScalarPlusImmFixed | 0x01800000
};

enum SVELoadAndBroadcastQuadword_ScalarPlusScalarOp {
  SVELoadAndBroadcastQuadword_ScalarPlusScalarFixed = 0xA4000000,
  SVELoadAndBroadcastQuadword_ScalarPlusScalarFMask = 0xFE00E000,
  SVELoadAndBroadcastQuadword_ScalarPlusScalarMask = 0xFFE0E000,
  LD1RQB_z_p_br_contiguous = SVELoadAndBroadcastQuadword_ScalarPlusScalarFixed,
  LD1RQH_z_p_br_contiguous = SVELoadAndBroadcastQuadword_ScalarPlusScalarFixed | 0x00800000,
  LD1RQW_z_p_br_contiguous = SVELoadAndBroadcastQuadword_ScalarPlusScalarFixed | 0x01000000,
  LD1RQD_z_p_br_contiguous = SVELoadAndBroadcastQuadword_ScalarPlusScalarFixed | 0x01800000
};

enum SVELoadMultipleStructures_ScalarPlusImmOp {
  SVELoadMultipleStructures_ScalarPlusImmFixed = 0xA400E000,
  SVELoadMultipleStructures_ScalarPlusImmFMask = 0xFE10E000,
  SVELoadMultipleStructures_ScalarPlusImmMask = 0xFFF0E000,
  LD2B_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00200000,
  LD3B_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00400000,
  LD4B_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00600000,
  LD2H_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00A00000,
  LD3H_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00C00000,
  LD4H_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x00E00000,
  LD2W_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01200000,
  LD3W_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01400000,
  LD4W_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01600000,
  LD2D_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01A00000,
  LD3D_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01C00000,
  LD4D_z_p_bi_contiguous = SVELoadMultipleStructures_ScalarPlusImmFixed | 0x01E00000
};

enum SVELoadMultipleStructures_ScalarPlusScalarOp {
  SVELoadMultipleStructures_ScalarPlusScalarFixed = 0xA400C000,
  SVELoadMultipleStructures_ScalarPlusScalarFMask = 0xFE00E000,
  SVELoadMultipleStructures_ScalarPlusScalarMask = 0xFFE0E000,
  LD2B_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00200000,
  LD3B_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00400000,
  LD4B_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00600000,
  LD2H_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00A00000,
  LD3H_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00C00000,
  LD4H_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x00E00000,
  LD2W_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01200000,
  LD3W_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01400000,
  LD4W_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01600000,
  LD2D_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01A00000,
  LD3D_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01C00000,
  LD4D_z_p_br_contiguous = SVELoadMultipleStructures_ScalarPlusScalarFixed | 0x01E00000
};

enum SVELoadPredicateRegisterOp {
  SVELoadPredicateRegisterFixed = 0x85800000,
  SVELoadPredicateRegisterFMask = 0xFFC0E010,
  SVELoadPredicateRegisterMask = 0xFFC0E010,
  LDR_p_bi = SVELoadPredicateRegisterFixed
};

enum SVELoadVectorRegisterOp {
  SVELoadVectorRegisterFixed = 0x85804000,
  SVELoadVectorRegisterFMask = 0xFFC0E000,
  SVELoadVectorRegisterMask = 0xFFC0E000,
  LDR_z_bi = SVELoadVectorRegisterFixed
};

enum SVEMulIndexOp {
  SVEMulIndexFixed = 0x44200000,
  SVEMulIndexFMask = 0xFF200000,
  SVEMulIndexMask = 0xFFE0FC00,
  SDOT_z_zzzi_s = SVEMulIndexFixed | 0x00800000,
  UDOT_z_zzzi_s = SVEMulIndexFixed | 0x00800400,
  SDOT_z_zzzi_d = SVEMulIndexFixed | 0x00C00000,
  UDOT_z_zzzi_d = SVEMulIndexFixed | 0x00C00400
};

enum SVEPartitionBreakConditionOp {
  SVEPartitionBreakConditionFixed = 0x25104000,
  SVEPartitionBreakConditionFMask = 0xFF3FC200,
  SVEPartitionBreakConditionMask = 0xFFFFC200,
  BRKA_p_p_p = SVEPartitionBreakConditionFixed,
  BRKAS_p_p_p_z = SVEPartitionBreakConditionFixed | 0x00400000,
  BRKB_p_p_p = SVEPartitionBreakConditionFixed | 0x00800000,
  BRKBS_p_p_p_z = SVEPartitionBreakConditionFixed | 0x00C00000
};

enum SVEPermutePredicateElementsOp {
  SVEPermutePredicateElementsFixed = 0x05204000,
  SVEPermutePredicateElementsFMask = 0xFF30E210,
  SVEPermutePredicateElementsMask = 0xFF30FE10,
  ZIP1_p_pp = SVEPermutePredicateElementsFixed,
  ZIP2_p_pp = SVEPermutePredicateElementsFixed | 0x00000400,
  UZP1_p_pp = SVEPermutePredicateElementsFixed | 0x00000800,
  UZP2_p_pp = SVEPermutePredicateElementsFixed | 0x00000C00,
  TRN1_p_pp = SVEPermutePredicateElementsFixed | 0x00001000,
  TRN2_p_pp = SVEPermutePredicateElementsFixed | 0x00001400
};

enum SVEPermuteVectorExtractOp {
  SVEPermuteVectorExtractFixed = 0x05200000,
  SVEPermuteVectorExtractFMask = 0xFF20E000,
  SVEPermuteVectorExtractMask = 0xFFE0E000,
  EXT_z_zi_des = SVEPermuteVectorExtractFixed
};

enum SVEPermuteVectorInterleavingOp {
  SVEPermuteVectorInterleavingFixed = 0x05206000,
  SVEPermuteVectorInterleavingFMask = 0xFF20E000,
  SVEPermuteVectorInterleavingMask = 0xFF20FC00,
  ZIP1_z_zz = SVEPermuteVectorInterleavingFixed,
  ZIP2_z_zz = SVEPermuteVectorInterleavingFixed | 0x00000400,
  UZP1_z_zz = SVEPermuteVectorInterleavingFixed | 0x00000800,
  UZP2_z_zz = SVEPermuteVectorInterleavingFixed | 0x00000C00,
  TRN1_z_zz = SVEPermuteVectorInterleavingFixed | 0x00001000,
  TRN2_z_zz = SVEPermuteVectorInterleavingFixed | 0x00001400
};

enum SVEPredicateCountOp {
  SVEPredicateCountFixed = 0x25208000,
  SVEPredicateCountFMask = 0xFF38C000,
  SVEPredicateCountMask = 0xFF3FC200,
  CNTP_r_p_p = SVEPredicateCountFixed
};

enum SVEPredicateFirstActiveOp {
  SVEPredicateFirstActiveFixed = 0x2518C000,
  SVEPredicateFirstActiveFMask = 0xFF3FFE10,
  SVEPredicateFirstActiveMask = 0xFFFFFE10,
  PFIRST_p_p_p = SVEPredicateFirstActiveFixed | 0x00400000
};

enum SVEPredicateInitializeOp {
  SVEPredicateInitializeFixed = 0x2518E000,
  SVEPredicateInitializeFMask = 0xFF3EFC10,
  SVEPredicateInitializeMask = 0xFF3FFC10,
  SVEPredicateInitializeSetFlagsBit = 0x00010000,
  PTRUE_p_s = SVEPredicateInitializeFixed | 0x00000000,
  PTRUES_p_s = SVEPredicateInitializeFixed | SVEPredicateInitializeSetFlagsBit
};

enum SVEPredicateLogicalOp {
  SVEPredicateLogicalFixed = 0x25004000,
  SVEPredicateLogicalFMask = 0xFF30C000,
  SVEPredicateLogicalMask = 0xFFF0C210,
  SVEPredicateLogicalSetFlagsBit = 0x00400000,
  AND_p_p_pp_z = SVEPredicateLogicalFixed,
  ANDS_p_p_pp_z = AND_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  BIC_p_p_pp_z = SVEPredicateLogicalFixed | 0x00000010,
  BICS_p_p_pp_z = BIC_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  EOR_p_p_pp_z = SVEPredicateLogicalFixed | 0x00000200,
  EORS_p_p_pp_z = EOR_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  ORR_p_p_pp_z = SVEPredicateLogicalFixed | 0x00800000,
  ORRS_p_p_pp_z = ORR_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  ORN_p_p_pp_z = SVEPredicateLogicalFixed | 0x00800010,
  ORNS_p_p_pp_z = ORN_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  NAND_p_p_pp_z = SVEPredicateLogicalFixed | 0x00800210,
  NANDS_p_p_pp_z = NAND_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  NOR_p_p_pp_z = SVEPredicateLogicalFixed | 0x00800200,
  NORS_p_p_pp_z = NOR_p_p_pp_z | SVEPredicateLogicalSetFlagsBit,
  SEL_p_p_pp = SVEPredicateLogicalFixed | 0x00000210
};

enum SVEPredicateNextActiveOp {
  SVEPredicateNextActiveFixed = 0x2519C400,
  SVEPredicateNextActiveFMask = 0xFF3FFE10,
  SVEPredicateNextActiveMask = 0xFF3FFE10,
  PNEXT_p_p_p = SVEPredicateNextActiveFixed
};

enum SVEPredicateReadFromFFR_PredicatedOp {
  SVEPredicateReadFromFFR_PredicatedFixed = 0x2518F000,
  SVEPredicateReadFromFFR_PredicatedFMask = 0xFF3FFE10,
  SVEPredicateReadFromFFR_PredicatedMask = 0xFFFFFE10,
  RDFFR_p_p_f = SVEPredicateReadFromFFR_PredicatedFixed,
  RDFFRS_p_p_f = SVEPredicateReadFromFFR_PredicatedFixed | 0x00400000
};

enum SVEPredicateReadFromFFR_UnpredicatedOp {
  SVEPredicateReadFromFFR_UnpredicatedFixed = 0x2519F000,
  SVEPredicateReadFromFFR_UnpredicatedFMask = 0xFF3FFFF0,
  SVEPredicateReadFromFFR_UnpredicatedMask = 0xFFFFFFF0,
  RDFFR_p_f = SVEPredicateReadFromFFR_UnpredicatedFixed
};

enum SVEPredicateTestOp {
  SVEPredicateTestFixed = 0x2510C000,
  SVEPredicateTestFMask = 0xFF3FC210,
  SVEPredicateTestMask = 0xFFFFC21F,
  PTEST_p_p = SVEPredicateTestFixed | 0x00400000
};

enum SVEPredicateZeroOp {
  SVEPredicateZeroFixed = 0x2518E400,
  SVEPredicateZeroFMask = 0xFF3FFFF0,
  SVEPredicateZeroMask = 0xFFFFFFF0,
  PFALSE_p = SVEPredicateZeroFixed
};

enum SVEPropagateBreakOp {
  SVEPropagateBreakFixed = 0x2500C000,
  SVEPropagateBreakFMask = 0xFF30C000,
  SVEPropagateBreakMask = 0xFFF0C210,
  BRKPA_p_p_pp = SVEPropagateBreakFixed,
  BRKPB_p_p_pp = SVEPropagateBreakFixed | 0x00000010,
  BRKPAS_p_p_pp = SVEPropagateBreakFixed | 0x00400000,
  BRKPBS_p_p_pp = SVEPropagateBreakFixed | 0x00400010
};

enum SVEPropagateBreakToNextPartitionOp {
  SVEPropagateBreakToNextPartitionFixed = 0x25184000,
  SVEPropagateBreakToNextPartitionFMask = 0xFFBFC210,
  SVEPropagateBreakToNextPartitionMask = 0xFFFFC210,
  BRKN_p_p_pp = SVEPropagateBreakToNextPartitionFixed,
  BRKNS_p_p_pp = SVEPropagateBreakToNextPartitionFixed | 0x00400000
};

enum SVEReversePredicateElementsOp {
  SVEReversePredicateElementsFixed = 0x05344000,
  SVEReversePredicateElementsFMask = 0xFF3FFE10,
  SVEReversePredicateElementsMask = 0xFF3FFE10,
  REV_p_p = SVEReversePredicateElementsFixed
};

enum SVEReverseVectorElementsOp {
  SVEReverseVectorElementsFixed = 0x05383800,
  SVEReverseVectorElementsFMask = 0xFF3FFC00,
  SVEReverseVectorElementsMask = 0xFF3FFC00,
  REV_z_z = SVEReverseVectorElementsFixed
};

enum SVEReverseWithinElementsOp {
  SVEReverseWithinElementsFixed = 0x05248000,
  SVEReverseWithinElementsFMask = 0xFF3CE000,
  SVEReverseWithinElementsMask = 0xFF3FE000,
  REVB_z_z = SVEReverseWithinElementsFixed,
  REVH_z_z = SVEReverseWithinElementsFixed | 0x00010000,
  REVW_z_z = SVEReverseWithinElementsFixed | 0x00020000,
  RBIT_z_p_z = SVEReverseWithinElementsFixed | 0x00030000
};

enum SVESaturatingIncDecRegisterByElementCountOp {
  SVESaturatingIncDecRegisterByElementCountFixed = 0x0420F000,
  SVESaturatingIncDecRegisterByElementCountFMask = 0xFF20F000,
  SVESaturatingIncDecRegisterByElementCountMask = 0xFFF0FC00,
  SQINCB_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed,
  UQINCB_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00000400,
  SQDECB_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00000800,
  UQDECB_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00000C00,
  SQINCB_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00100000,
  UQINCB_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00100400,
  SQDECB_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00100800,
  UQDECB_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00100C00,
  SQINCH_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00400000,
  UQINCH_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00400400,
  SQDECH_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00400800,
  UQDECH_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00400C00,
  SQINCH_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00500000,
  UQINCH_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00500400,
  SQDECH_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00500800,
  UQDECH_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00500C00,
  SQINCW_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00800000,
  UQINCW_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00800400,
  SQDECW_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00800800,
  UQDECW_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00800C00,
  SQINCW_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00900000,
  UQINCW_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00900400,
  SQDECW_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00900800,
  UQDECW_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00900C00,
  SQINCD_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00C00000,
  UQINCD_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00C00400,
  SQDECD_r_rs_sx = SVESaturatingIncDecRegisterByElementCountFixed | 0x00C00800,
  UQDECD_r_rs_uw = SVESaturatingIncDecRegisterByElementCountFixed | 0x00C00C00,
  SQINCD_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00D00000,
  UQINCD_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00D00400,
  SQDECD_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00D00800,
  UQDECD_r_rs_x = SVESaturatingIncDecRegisterByElementCountFixed | 0x00D00C00
};

enum SVESaturatingIncDecVectorByElementCountOp {
  SVESaturatingIncDecVectorByElementCountFixed = 0x0420C000,
  SVESaturatingIncDecVectorByElementCountFMask = 0xFF30F000,
  SVESaturatingIncDecVectorByElementCountMask = 0xFFF0FC00,
  SQINCH_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00400000,
  UQINCH_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00400400,
  SQDECH_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00400800,
  UQDECH_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00400C00,
  SQINCW_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00800000,
  UQINCW_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00800400,
  SQDECW_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00800800,
  UQDECW_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00800C00,
  SQINCD_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00C00000,
  UQINCD_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00C00400,
  SQDECD_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00C00800,
  UQDECD_z_zs = SVESaturatingIncDecVectorByElementCountFixed | 0x00C00C00
};

enum SVEStackFrameAdjustmentOp {
  SVEStackFrameAdjustmentFixed = 0x04205000,
  SVEStackFrameAdjustmentFMask = 0xFFA0F800,
  SVEStackFrameAdjustmentMask = 0xFFE0F800,
  ADDVL_r_ri = SVEStackFrameAdjustmentFixed,
  ADDPL_r_ri = SVEStackFrameAdjustmentFixed | 0x00400000
};

enum SVEStackFrameSizeOp {
  SVEStackFrameSizeFixed = 0x04BF5000,
  SVEStackFrameSizeFMask = 0xFFFFF800,
  SVEStackFrameSizeMask = 0xFFFFF800,
  RDVL_r_i = SVEStackFrameSizeFixed
};

enum SVEStoreMultipleStructures_ScalarPlusImmOp {
  SVEStoreMultipleStructures_ScalarPlusImmFixed = 0xE410E000,
  SVEStoreMultipleStructures_ScalarPlusImmFMask = 0xFE10E000,
  SVEStoreMultipleStructures_ScalarPlusImmMask = 0xFFF0E000,
  ST2B_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00200000,
  ST3B_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00400000,
  ST4B_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00600000,
  ST2H_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00A00000,
  ST3H_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00C00000,
  ST4H_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x00E00000,
  ST2W_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01200000,
  ST3W_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01400000,
  ST4W_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01600000,
  ST2D_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01A00000,
  ST3D_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01C00000,
  ST4D_z_p_bi_contiguous = SVEStoreMultipleStructures_ScalarPlusImmFixed | 0x01E00000
};

enum SVEStoreMultipleStructures_ScalarPlusScalarOp {
  SVEStoreMultipleStructures_ScalarPlusScalarFixed = 0xE4006000,
  SVEStoreMultipleStructures_ScalarPlusScalarFMask = 0xFE00E000,
  SVEStoreMultipleStructures_ScalarPlusScalarMask = 0xFFE0E000,
  ST2B_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00200000,
  ST3B_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00400000,
  ST4B_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00600000,
  ST2H_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00A00000,
  ST3H_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00C00000,
  ST4H_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x00E00000,
  ST2W_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01200000,
  ST3W_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01400000,
  ST4W_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01600000,
  ST2D_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01A00000,
  ST3D_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01C00000,
  ST4D_z_p_br_contiguous = SVEStoreMultipleStructures_ScalarPlusScalarFixed | 0x01E00000
};

enum SVEStorePredicateRegisterOp {
  SVEStorePredicateRegisterFixed = 0xE5800000,
  SVEStorePredicateRegisterFMask = 0xFFC0E010,
  SVEStorePredicateRegisterMask = 0xFFC0E010,
  STR_p_bi = SVEStorePredicateRegisterFixed
};

enum SVEStoreVectorRegisterOp {
  SVEStoreVectorRegisterFixed = 0xE5804000,
  SVEStoreVectorRegisterFMask = 0xFFC0E000,
  SVEStoreVectorRegisterMask = 0xFFC0E000,
  STR_z_bi = SVEStoreVectorRegisterFixed
};

enum SVETableLookupOp {
  SVETableLookupFixed = 0x05203000,
  SVETableLookupFMask = 0xFF20FC00,
  SVETableLookupMask = 0xFF20FC00,
  TBL_z_zz_1 = SVETableLookupFixed
};

enum SVEUnpackPredicateElementsOp {
  SVEUnpackPredicateElementsFixed = 0x05304000,
  SVEUnpackPredicateElementsFMask = 0xFFFEFE10,
  SVEUnpackPredicateElementsMask = 0xFFFFFE10,
  PUNPKLO_p_p = SVEUnpackPredicateElementsFixed,
  PUNPKHI_p_p = SVEUnpackPredicateElementsFixed | 0x00010000
};

enum SVEUnpackVectorElementsOp {
  SVEUnpackVectorElementsFixed = 0x05303800,
  SVEUnpackVectorElementsFMask = 0xFF3CFC00,
  SVEUnpackVectorElementsMask = 0xFF3FFC00,
  SUNPKLO_z_z = SVEUnpackVectorElementsFixed,
  SUNPKHI_z_z = SVEUnpackVectorElementsFixed | 0x00010000,
  UUNPKLO_z_z = SVEUnpackVectorElementsFixed | 0x00020000,
  UUNPKHI_z_z = SVEUnpackVectorElementsFixed | 0x00030000
};

enum SVEVectorSelectOp {
  SVEVectorSelectFixed = 0x0520C000,
  SVEVectorSelectFMask = 0xFF20C000,
  SVEVectorSelectMask = 0xFF20C000,
  SEL_z_p_zz = SVEVectorSelectFixed
};

enum SVEVectorSpliceOp {
  SVEVectorSpliceFixed = 0x052C8000,
  SVEVectorSpliceFMask = 0xFF3FE000,
  SVEVectorSpliceMask = 0xFF3FE000,
  SPLICE_z_p_zz_des = SVEVectorSpliceFixed
};

enum ReservedOp {
  ReservedFixed = 0x00000000,
  ReservedFMask = 0x1E000000,
  ReservedMask = 0xFFFF0000,
  UDF = ReservedFixed | 0x00000000
};

// Unimplemented and unallocated instructions. These are defined to make fixed
// bit assertion easier.
enum UnimplementedOp {
  UnimplementedFixed = 0x00000000,
  UnimplementedFMask = 0x00000000
};

enum UnallocatedOp {
  UnallocatedFixed = 0x00000000,
  UnallocatedFMask = 0x00000000
};

// Re-enable `clang-format` after the `enum`s.
// clang-format on

}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_AARCH64_CONSTANTS_AARCH64_H_
