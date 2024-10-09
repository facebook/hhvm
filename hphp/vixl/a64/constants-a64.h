// Copyright 2013, ARM Limited
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

#ifndef VIXL_A64_CONSTANTS_A64_H_
#define VIXL_A64_CONSTANTS_A64_H_

namespace vixl {

constexpr unsigned kNumberOfRegisters = 32;
constexpr unsigned kNumberOfFPRegisters = 32;
// Callee saved registers are x19-x28.
constexpr int kNumberOfCalleeSavedRegisters = 10;
constexpr int kFirstCalleeSavedRegisterIndex = 19;
// Callee saved FP registers are d8-d15.
constexpr int kNumberOfCalleeSavedFPRegisters = 8;
constexpr int kFirstCalleeSavedFPRegisterIndex = 8;

const int kWRegSizeInBits = 32;

constexpr byte kSimulatorStackJunk = 0x8b;

#define REGISTER_CODE_LIST(R)                                                  \
R(0)  R(1)  R(2)  R(3)  R(4)  R(5)  R(6)  R(7)                                 \
R(8)  R(9)  R(10) R(11) R(12) R(13) R(14) R(15)                                \
R(16) R(17) R(18) R(19) R(20) R(21) R(22) R(23)                                \
R(24) R(25) R(26) R(27) R(28) R(29) R(30) R(31)

#define INSTRUCTION_FIELDS_LIST(V_)                                            \
/* Register fields */                                                          \
V_(Rd, 4, 0, Bits)                        /* Destination register.     */      \
V_(Rn, 9, 5, Bits)                        /* First source register.    */      \
V_(Rm, 20, 16, Bits)                      /* Second source register.   */      \
V_(Ra, 14, 10, Bits)                      /* Third source register.    */      \
V_(Rt, 4, 0, Bits)                        /* Load/store register.      */      \
V_(Rt2, 14, 10, Bits)                     /* Load/store 2nd register   */      \
V_(Rs, 20, 16, Bits)                      /* Exclusive access status.  */      \
V_(PrefetchMode, 4, 0, Bits)                                                   \
                                                                               \
/* Common bits */                                                              \
V_(SixtyFourBits, 31, 31, Bits)                                                \
V_(FlagsUpdate, 29, 29, Bits)                                                  \
                                                                               \
/* PC relative addressing */                                                   \
V_(ImmPCRelHi, 23, 5, SignedBits)                                              \
V_(ImmPCRelLo, 30, 29, Bits)                                                   \
                                                                               \
/* Add/subtract/logical shift register */                                      \
V_(ShiftDP, 23, 22, Bits)                                                      \
V_(ImmDPShift, 15, 10, Bits)                                                   \
                                                                               \
/* Add/subtract immediate */                                                   \
V_(ImmAddSub, 21, 10, Bits)                                                    \
V_(ShiftAddSub, 23, 22, Bits)                                                  \
                                                                               \
/* Add/substract extend */                                                     \
V_(ImmExtendShift, 12, 10, Bits)                                               \
V_(ExtendMode, 15, 13, Bits)                                                   \
                                                                               \
/* Move wide */                                                                \
V_(ImmMoveWide, 20, 5, Bits)                                                   \
V_(ShiftMoveWide, 22, 21, Bits)                                                \
                                                                               \
/* Logical immediate, bitfield and extract */                                  \
V_(BitN, 22, 22, Bits)                                                         \
V_(ImmRotate, 21, 16, Bits)                                                    \
V_(ImmSetBits, 15, 10, Bits)                                                   \
V_(ImmR, 21, 16, Bits)                                                         \
V_(ImmS, 15, 10, Bits)                                                         \
                                                                               \
/* Test and branch immediate */                                                \
V_(ImmTestBranch, 18, 5, SignedBits)                                           \
V_(ImmTestBranchBit40, 23, 19, Bits)                                           \
V_(ImmTestBranchBit5, 31, 31, Bits)                                            \
                                                                               \
/* Conditionals */                                                             \
V_(Condition, 15, 12, Bits)                                                    \
V_(ConditionBranch, 3, 0, Bits)                                                \
V_(Nzcv, 3, 0, Bits)                                                           \
V_(ImmCondCmp, 20, 16, Bits)                                                   \
V_(ImmCondBranch, 23, 5, SignedBits)                                           \
                                                                               \
/* Floating point */                                                           \
V_(FPType, 23, 22, Bits)                                                       \
V_(ImmFP, 20, 13, Bits)                                                        \
V_(FPScale, 15, 10, Bits)                                                      \
                                                                               \
/* Load Store */                                                               \
V_(ImmLS, 20, 12, SignedBits)                                                  \
V_(ImmLSUnsigned, 21, 10, Bits)                                                \
V_(ImmLSPair, 21, 15, SignedBits)                                              \
V_(SizeLS, 31, 30, Bits)                                                       \
V_(ImmShiftLS, 12, 12, Bits)                                                   \
                                                                               \
/* Other immediates */                                                         \
V_(ImmUncondBranch, 25, 0, SignedBits)                                         \
V_(ImmCmpBranch, 23, 5, SignedBits)                                            \
V_(ImmLLiteral, 23, 5, SignedBits)                                             \
V_(ImmException, 20, 5, Bits)                                                  \
V_(ImmHint, 11, 5, Bits)                                                       \
                                                                               \
/* System (MRS, MSR) */                                                        \
V_(ImmSystemRegister, 19, 5, Bits)                                             \
V_(SysO0, 19, 19, Bits)                                                        \
V_(SysOp1, 18, 16, Bits)                                                       \
V_(SysOp2, 7, 5, Bits)                                                         \
V_(CRn, 15, 12, Bits)                                                          \
V_(CRm, 11, 8, Bits)                                                           \
                                                                               \
/* Large System Extension */                                                   \
V_(Ar, 23, 22, Bits)                                                           \
V_(Opc, 14, 12, Bits)                                                          \



#define SYSTEM_REGISTER_FIELDS_LIST(V_, M_)                                    \
/* NZCV */                                                                     \
V_(Flags, 31, 28, Bits)                                                        \
V_(N, 31, 31, Bits)                                                            \
V_(Z, 30, 30, Bits)                                                            \
V_(C, 29, 29, Bits)                                                            \
V_(V, 28, 28, Bits)                                                            \
M_(NZCV, Flags_mask)                                                           \
                                                                               \
/* FPCR */                                                                     \
V_(AHP, 26, 26, Bits)                                                          \
V_(DN, 25, 25, Bits)                                                           \
V_(FZ, 24, 24, Bits)                                                           \
V_(RMode, 23, 22, Bits)                                                        \
M_(FPCR, AHP_mask | DN_mask | FZ_mask | RMode_mask)


// Fields offsets.
#define DECLARE_FIELDS_OFFSETS(Name, HighBit, LowBit, X)                       \
const int Name##_offset = LowBit;                                              \
const int Name##_width = HighBit - LowBit + 1;                                 \
const uint32_t Name##_mask = ((1 << Name##_width) - 1) << LowBit;
#define NOTHING(A, B)
INSTRUCTION_FIELDS_LIST(DECLARE_FIELDS_OFFSETS)
SYSTEM_REGISTER_FIELDS_LIST(DECLARE_FIELDS_OFFSETS, NOTHING)
#undef NOTHING
#undef DECLARE_FIELDS_BITS

// ImmPCRel is a compound field (not present in INSTRUCTION_FIELDS_LIST), formed
// from ImmPCRelLo and ImmPCRelHi.
const int ImmPCRel_mask = ImmPCRelLo_mask | ImmPCRelHi_mask;

// Condition codes.
enum Condition {
  eq = 0,
  ne = 1,
  hs = 2,
  lo = 3,
  mi = 4,
  pl = 5,
  vs = 6,
  vc = 7,
  hi = 8,
  ls = 9,
  ge = 10,
  lt = 11,
  gt = 12,
  le = 13,
  al = 14,
  nv = 15  // Behaves as always/al.
};

inline Condition InvertCondition(Condition cond) {
  // Conditions al and nv behave identically, as "always true". They can't be
  // inverted, because there is no "always false" condition.
  assert((cond != al) && (cond != nv));
  return static_cast<Condition>(cond ^ 1);
}

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
  FPUnorderedFlag   = CVFlag
};

enum Shift {
  NO_SHIFT = -1,
  LSL = 0x0,
  LSR = 0x1,
  ASR = 0x2,
  ROR = 0x3
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

enum SystemHint {
  NOP   = 0,
  YIELD = 1,
  WFE   = 2,
  WFI   = 3,
  SEV   = 4,
  SEVL  = 5
};

// System/special register names.
// This information is not encoded as one field but as the concatenation of
// multiple fields (Op0<0>, Op1, Crn, Crm, Op2).
enum SystemRegister {
  NZCV = ((0x1 << SysO0_offset) |
          (0x3 << SysOp1_offset) |
          (0x4 << CRn_offset) |
          (0x2 << CRm_offset) |
          (0x0 << SysOp2_offset)) >> ImmSystemRegister_offset,
  FPCR = ((0x1 << SysO0_offset) |
          (0x3 << SysOp1_offset) |
          (0x4 << CRn_offset) |
          (0x4 << CRm_offset) |
          (0x0 << SysOp2_offset)) >> ImmSystemRegister_offset,
  FPSR = ((0x1 << SysO0_offset) |
          (0x3 << SysOp1_offset) |
          (0x4 << CRn_offset) |
          (0x4 << CRm_offset) |
          (0x1 << SysOp2_offset)) >> ImmSystemRegister_offset,
  TPIDR_EL0 = ((0x1 << SysO0_offset) |
               (0x3 << SysOp1_offset) |
               (0xd << CRn_offset) |
               (0x0 << CRm_offset) |
               (0x2 << SysOp2_offset)) >> ImmSystemRegister_offset,
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
// assert(instr->Mask(PCRelAddressingFMask) == PCRelAddressingFixed);
// switch(instr->Mask(PCRelAddressingMask)) {
//   case ADR:  Format("adr 'Xd, 'AddrPCRelByte"); break;
//   case ADRP: Format("adrp 'Xd, 'AddrPCRelPage"); break;
//   default:   printf("Unknown instruction\n");
// }


// Generic fields.
enum GenericInstrField {
  SixtyFourBits        = 0x80000000,
  ThirtyTwoBits        = 0x00000000,
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
  AddSubImmediateFMask = 0x1F000000,
  AddSubImmediateMask  = 0xFF000000,
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
  UnconditionalBranchToRegisterMask  = 0xFFFFFC1F,
  BR      = UnconditionalBranchToRegisterFixed | 0x001F0000,
  BLR     = UnconditionalBranchToRegisterFixed | 0x003F0000,
  RET     = UnconditionalBranchToRegisterFixed | 0x005F0000
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

enum SystemHintOp {
  SystemHintFixed = 0xD503201F,
  SystemHintFMask = 0xFFFFF01F,
  SystemHintMask  = 0xFFFFF01F,
  HINT            = SystemHintFixed | 0x00000000
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

// Any load or store.
enum LoadStoreAnyOp {
  LoadStoreAnyFMask = 0x0a000000,
  LoadStoreAnyFixed = 0x08000000
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
  V(LDP, d,   0x44400000)

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
  STNP_w = LoadStorePairNonTemporalFixed | STP_w,
  LDNP_w = LoadStorePairNonTemporalFixed | LDP_w,
  STNP_x = LoadStorePairNonTemporalFixed | STP_x,
  LDNP_x = LoadStorePairNonTemporalFixed | LDP_x,
  STNP_s = LoadStorePairNonTemporalFixed | STP_s,
  LDNP_s = LoadStorePairNonTemporalFixed | LDP_s,
  STNP_d = LoadStorePairNonTemporalFixed | STP_d,
  LDNP_d = LoadStorePairNonTemporalFixed | LDP_d
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
  LDR_d_lit        = LoadLiteralFixed | 0x44000000
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
  V(ST, R, s,   0x84000000),  \
  V(ST, R, d,   0xC4000000),  \
  V(LD, R, s,   0x84400000),  \
  V(LD, R, d,   0xC4400000)


// Load/store unscaled offset.
enum LoadStoreUnscaledOffsetOp {
  LoadStoreUnscaledOffsetFixed = 0x38000000,
  LoadStoreUnscaledOffsetFMask = 0x3B200C00,
  LoadStoreUnscaledOffsetMask  = 0xFFE00C00,
  #define LOAD_STORE_UNSCALED(A, B, C, D)  \
  A##U##B##_##C = LoadStoreUnscaledOffsetFixed | D
  LOAD_STORE_OP_LIST(LOAD_STORE_UNSCALED)
  #undef LOAD_STORE_UNSCALED
};

// Load/store (post, pre, offset and unsigned.)
enum LoadStoreOp {
  LoadStoreOpMask   = 0xC4C00000,
  #define LOAD_STORE(A, B, C, D)  \
  A##B##_##C = D
  LOAD_STORE_OP_LIST(LOAD_STORE),
  #undef LOAD_STORE
  PRFM = 0xC0800000
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

// Load/store excusively
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
  LDAR_x   = LoadStoreExclusiveFixed | 0xC0C08000
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
  CLS_x   = CLS | SixtyFourBits
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
  FCMP_s         = FPCompareFixed | 0x00000000,
  FCMP_d         = FPCompareFixed | FP64 | 0x00000000,
  FCMP           = FCMP_s,
  FCMP_s_zero    = FPCompareFixed | 0x00000008,
  FCMP_d_zero    = FPCompareFixed | FP64 | 0x00000008,
  FCMP_zero      = FCMP_s_zero,
  FCMPE_s        = FPCompareFixed | 0x00000010,
  FCMPE_d        = FPCompareFixed | FP64 | 0x00000010,
  FCMPE_s_zero   = FPCompareFixed | 0x00000018,
  FCMPE_d_zero   = FPCompareFixed | FP64 | 0x00000018
};

// Floating point conditional compare.
enum FPConditionalCompareOp {
  FPConditionalCompareFixed = 0x1E200400,
  FPConditionalCompareFMask = 0x5F200C00,
  FPConditionalCompareMask  = 0xFFE00C10,
  FCCMP_s                   = FPConditionalCompareFixed | 0x00000000,
  FCCMP_d                   = FPConditionalCompareFixed | FP64 | 0x00000000,
  FCCMP                     = FCCMP_s,
  FCCMPE_s                  = FPConditionalCompareFixed | 0x00000010,
  FCCMPE_d                  = FPConditionalCompareFixed | FP64 | 0x00000010,
  FCCMPE                    = FCCMPE_s
};

// Floating point conditional select.
enum FPConditionalSelectOp {
  FPConditionalSelectFixed = 0x1E200C00,
  FPConditionalSelectFMask = 0x5F200C00,
  FPConditionalSelectMask  = 0xFFE00C00,
  FCSEL_s                  = FPConditionalSelectFixed | 0x00000000,
  FCSEL_d                  = FPConditionalSelectFixed | FP64 | 0x00000000,
  FCSEL                    = FCSEL_s
};

// Floating point immediate.
enum FPImmediateOp {
  FPImmediateFixed = 0x1E201000,
  FPImmediateFMask = 0x5F201C00,
  FPImmediateMask  = 0xFFE01C00,
  FMOV_s_imm       = FPImmediateFixed | 0x00000000,
  FMOV_d_imm       = FPImmediateFixed | FP64 | 0x00000000
};

// Floating point data processing 1 source.
enum FPDataProcessing1SourceOp {
  FPDataProcessing1SourceFixed = 0x1E204000,
  FPDataProcessing1SourceFMask = 0x5F207C00,
  FPDataProcessing1SourceMask  = 0xFFFFFC00,
  FMOV_s   = FPDataProcessing1SourceFixed | 0x00000000,
  FMOV_d   = FPDataProcessing1SourceFixed | FP64 | 0x00000000,
  FMOV     = FMOV_s,
  FABS_s   = FPDataProcessing1SourceFixed | 0x00008000,
  FABS_d   = FPDataProcessing1SourceFixed | FP64 | 0x00008000,
  FABS     = FABS_s,
  FNEG_s   = FPDataProcessing1SourceFixed | 0x00010000,
  FNEG_d   = FPDataProcessing1SourceFixed | FP64 | 0x00010000,
  FNEG     = FNEG_s,
  FSQRT_s  = FPDataProcessing1SourceFixed | 0x00018000,
  FSQRT_d  = FPDataProcessing1SourceFixed | FP64 | 0x00018000,
  FSQRT    = FSQRT_s,
  FCVT_ds  = FPDataProcessing1SourceFixed | 0x00028000,
  FCVT_sd  = FPDataProcessing1SourceFixed | FP64 | 0x00020000,
  FRINTN_s = FPDataProcessing1SourceFixed | 0x00040000,
  FRINTN_d = FPDataProcessing1SourceFixed | FP64 | 0x00040000,
  FRINTN   = FRINTN_s,
  FRINTP_s = FPDataProcessing1SourceFixed | 0x00048000,
  FRINTP_d = FPDataProcessing1SourceFixed | FP64 | 0x00048000,
  FRINTP   = FRINTP_s,
  FRINTM_s = FPDataProcessing1SourceFixed | 0x00050000,
  FRINTM_d = FPDataProcessing1SourceFixed | FP64 | 0x00050000,
  FRINTM   = FRINTM_s,
  FRINTZ_s = FPDataProcessing1SourceFixed | 0x00058000,
  FRINTZ_d = FPDataProcessing1SourceFixed | FP64 | 0x00058000,
  FRINTZ   = FRINTZ_s,
  FRINTA_s = FPDataProcessing1SourceFixed | 0x00060000,
  FRINTA_d = FPDataProcessing1SourceFixed | FP64 | 0x00060000,
  FRINTX_s = FPDataProcessing1SourceFixed | 0x00070000,
  FRINTX_d = FPDataProcessing1SourceFixed | FP64 | 0x00070000,
  FRINTI_s = FPDataProcessing1SourceFixed | 0x00078000,
  FRINTI_d = FPDataProcessing1SourceFixed | FP64 | 0x00078000
};

// Floating point data processing 2 source.
enum FPDataProcessing2SourceOp {
  FPDataProcessing2SourceFixed = 0x1E200800,
  FPDataProcessing2SourceFMask = 0x5F200C00,
  FPDataProcessing2SourceMask  = 0xFFE0FC00,
  FMUL     = FPDataProcessing2SourceFixed | 0x00000000,
  FMUL_s   = FMUL,
  FMUL_d   = FMUL | FP64,
  FDIV     = FPDataProcessing2SourceFixed | 0x00001000,
  FDIV_s   = FDIV,
  FDIV_d   = FDIV | FP64,
  FADD     = FPDataProcessing2SourceFixed | 0x00002000,
  FADD_s   = FADD,
  FADD_d   = FADD | FP64,
  FSUB     = FPDataProcessing2SourceFixed | 0x00003000,
  FSUB_s   = FSUB,
  FSUB_d   = FSUB | FP64,
  FMAX     = FPDataProcessing2SourceFixed | 0x00004000,
  FMAX_s   = FMAX,
  FMAX_d   = FMAX | FP64,
  FMIN     = FPDataProcessing2SourceFixed | 0x00005000,
  FMIN_s   = FMIN,
  FMIN_d   = FMIN | FP64,
  FMAXNM   = FPDataProcessing2SourceFixed | 0x00006000,
  FMAXNM_s = FMAXNM,
  FMAXNM_d = FMAXNM | FP64,
  FMINNM   = FPDataProcessing2SourceFixed | 0x00007000,
  FMINNM_s = FMINNM,
  FMINNM_d = FMINNM | FP64,
  FNMUL    = FPDataProcessing2SourceFixed | 0x00008000,
  FNMUL_s  = FNMUL,
  FNMUL_d  = FNMUL | FP64
};

// Floating point data processing 3 source.
enum FPDataProcessing3SourceOp {
  FPDataProcessing3SourceFixed = 0x1F000000,
  FPDataProcessing3SourceFMask = 0x5F000000,
  FPDataProcessing3SourceMask  = 0xFFE08000,
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
  FCVTNS_ws = FCVTNS,
  FCVTNS_xs = FCVTNS | SixtyFourBits,
  FCVTNS_wd = FCVTNS | FP64,
  FCVTNS_xd = FCVTNS | SixtyFourBits | FP64,
  FCVTNU    = FPIntegerConvertFixed | 0x00010000,
  FCVTNU_ws = FCVTNU,
  FCVTNU_xs = FCVTNU | SixtyFourBits,
  FCVTNU_wd = FCVTNU | FP64,
  FCVTNU_xd = FCVTNU | SixtyFourBits | FP64,
  FCVTPS    = FPIntegerConvertFixed | 0x00080000,
  FCVTPS_ws = FCVTPS,
  FCVTPS_xs = FCVTPS | SixtyFourBits,
  FCVTPS_wd = FCVTPS | FP64,
  FCVTPS_xd = FCVTPS | SixtyFourBits | FP64,
  FCVTPU    = FPIntegerConvertFixed | 0x00090000,
  FCVTPU_ws = FCVTPU,
  FCVTPU_xs = FCVTPU | SixtyFourBits,
  FCVTPU_wd = FCVTPU | FP64,
  FCVTPU_xd = FCVTPU | SixtyFourBits | FP64,
  FCVTMS    = FPIntegerConvertFixed | 0x00100000,
  FCVTMS_ws = FCVTMS,
  FCVTMS_xs = FCVTMS | SixtyFourBits,
  FCVTMS_wd = FCVTMS | FP64,
  FCVTMS_xd = FCVTMS | SixtyFourBits | FP64,
  FCVTMU    = FPIntegerConvertFixed | 0x00110000,
  FCVTMU_ws = FCVTMU,
  FCVTMU_xs = FCVTMU | SixtyFourBits,
  FCVTMU_wd = FCVTMU | FP64,
  FCVTMU_xd = FCVTMU | SixtyFourBits | FP64,
  FCVTZS    = FPIntegerConvertFixed | 0x00180000,
  FCVTZS_ws = FCVTZS,
  FCVTZS_xs = FCVTZS | SixtyFourBits,
  FCVTZS_wd = FCVTZS | FP64,
  FCVTZS_xd = FCVTZS | SixtyFourBits | FP64,
  FCVTZU    = FPIntegerConvertFixed | 0x00190000,
  FCVTZU_ws = FCVTZU,
  FCVTZU_xs = FCVTZU | SixtyFourBits,
  FCVTZU_wd = FCVTZU | FP64,
  FCVTZU_xd = FCVTZU | SixtyFourBits | FP64,
  SCVTF     = FPIntegerConvertFixed | 0x00020000,
  SCVTF_sw  = SCVTF,
  SCVTF_sx  = SCVTF | SixtyFourBits,
  SCVTF_dw  = SCVTF | FP64,
  SCVTF_dx  = SCVTF | SixtyFourBits | FP64,
  UCVTF     = FPIntegerConvertFixed | 0x00030000,
  UCVTF_sw  = UCVTF,
  UCVTF_sx  = UCVTF | SixtyFourBits,
  UCVTF_dw  = UCVTF | FP64,
  UCVTF_dx  = UCVTF | SixtyFourBits | FP64,
  FCVTAS    = FPIntegerConvertFixed | 0x00040000,
  FCVTAS_ws = FCVTAS,
  FCVTAS_xs = FCVTAS | SixtyFourBits,
  FCVTAS_wd = FCVTAS | FP64,
  FCVTAS_xd = FCVTAS | SixtyFourBits | FP64,
  FCVTAU    = FPIntegerConvertFixed | 0x00050000,
  FCVTAU_ws = FCVTAU,
  FCVTAU_xs = FCVTAU | SixtyFourBits,
  FCVTAU_wd = FCVTAU | FP64,
  FCVTAU_xd = FCVTAU | SixtyFourBits | FP64,
  FMOV_ws   = FPIntegerConvertFixed | 0x00060000,
  FMOV_sw   = FPIntegerConvertFixed | 0x00070000,
  FMOV_xd   = FMOV_ws | SixtyFourBits | FP64,
  FMOV_dx   = FMOV_sw | SixtyFourBits | FP64,
  FMOV_d1_x = FPIntegerConvertFixed | SixtyFourBits | 0x008F0000,
  FMOV_x_d1 = FPIntegerConvertFixed | SixtyFourBits | 0x008E0000
};

// Conversion between fixed point and floating point.
enum FPFixedPointConvertOp {
  FPFixedPointConvertFixed = 0x1E000000,
  FPFixedPointConvertFMask = 0x5F200000,
  FPFixedPointConvertMask  = 0xFFFF0000,
  FCVTZS_fixed    = FPFixedPointConvertFixed | 0x00180000,
  FCVTZS_ws_fixed = FCVTZS_fixed,
  FCVTZS_xs_fixed = FCVTZS_fixed | SixtyFourBits,
  FCVTZS_wd_fixed = FCVTZS_fixed | FP64,
  FCVTZS_xd_fixed = FCVTZS_fixed | SixtyFourBits | FP64,
  FCVTZU_fixed    = FPFixedPointConvertFixed | 0x00190000,
  FCVTZU_ws_fixed = FCVTZU_fixed,
  FCVTZU_xs_fixed = FCVTZU_fixed | SixtyFourBits,
  FCVTZU_wd_fixed = FCVTZU_fixed | FP64,
  FCVTZU_xd_fixed = FCVTZU_fixed | SixtyFourBits | FP64,
  SCVTF_fixed     = FPFixedPointConvertFixed | 0x00020000,
  SCVTF_sw_fixed  = SCVTF_fixed,
  SCVTF_sx_fixed  = SCVTF_fixed | SixtyFourBits,
  SCVTF_dw_fixed  = SCVTF_fixed | FP64,
  SCVTF_dx_fixed  = SCVTF_fixed | SixtyFourBits | FP64,
  UCVTF_fixed     = FPFixedPointConvertFixed | 0x00030000,
  UCVTF_sw_fixed  = UCVTF_fixed,
  UCVTF_sx_fixed  = UCVTF_fixed | SixtyFourBits,
  UCVTF_dw_fixed  = UCVTF_fixed | FP64,
  UCVTF_dx_fixed  = UCVTF_fixed | SixtyFourBits | FP64
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
  NEON_BSL = NEON3SameLogicalFixed | 0x20400000
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
  NEONLoadStoreSingleStructPostIndex = 0x00800000,
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

enum LSELoadOp {
  LSELoadOPMask = 0x00007000,
  LSE_LDADD  = 0x00000000,
  LSE_LDBIC  = 0x00001000,
  LSE_LDEOR  = 0x00002000,
  LSE_LDORR  = 0x00003000,
  LSE_LDSMAX = 0x00004000,
  LSE_LDSMIN = 0x00005000,
  LSE_LDUMAX = 0x00006000,
  LSE_LDUMIN = 0x00007000
};

enum LSEAquireRelease {
  LSEAquireReleaseN  = 0x00000000,
  LSEAquireReleaseA  = 0x00800000,
  LSEAquireReleaseL  = 0x00400000,
  LSEAquireReleaseAL = 0x00C00000
};

enum LSESize {
  LSESizeB = 0x00000000,
  LSESizeH = 0x40000000,
  LSESizeW = 0x80000000,
  LSESizeX = 0xc0000000
};

#define LSELD_OP_LIST(V) \
  V(ADD),                \
  V(BIC),                \
  V(EOR),                \
  V(ORR),                \
  V(SMAX),               \
  V(SMIN),               \
  V(UMAX),               \
  V(UMIN)

enum LSELd {
  LseLdOpFixed = 0x38200000,
  LseLdOpFMask = 0x3F208C00,
  #define LSELD(A)       \
  LSELD_##A##_b   = LseLdOpFixed | LSESizeB | LSEAquireReleaseN  | LSE_LD##A, \
  LSELD_##A##_ab  = LseLdOpFixed | LSESizeB | LSEAquireReleaseA  | LSE_LD##A, \
  LSELD_##A##_lb  = LseLdOpFixed | LSESizeB | LSEAquireReleaseL  | LSE_LD##A, \
  LSELD_##A##_alb = LseLdOpFixed | LSESizeB | LSEAquireReleaseAL | LSE_LD##A, \
  LSELD_##A##_h   = LseLdOpFixed | LSESizeH | LSEAquireReleaseN  | LSE_LD##A, \
  LSELD_##A##_ah  = LseLdOpFixed | LSESizeH | LSEAquireReleaseA  | LSE_LD##A, \
  LSELD_##A##_lh  = LseLdOpFixed | LSESizeH | LSEAquireReleaseL  | LSE_LD##A, \
  LSELD_##A##_alh = LseLdOpFixed | LSESizeH | LSEAquireReleaseAL | LSE_LD##A, \
  LSELD_##A##_w   = LseLdOpFixed | LSESizeW | LSEAquireReleaseN  | LSE_LD##A, \
  LSELD_##A##_aw  = LseLdOpFixed | LSESizeW | LSEAquireReleaseA  | LSE_LD##A, \
  LSELD_##A##_lw  = LseLdOpFixed | LSESizeW | LSEAquireReleaseL  | LSE_LD##A, \
  LSELD_##A##_alw = LseLdOpFixed | LSESizeW | LSEAquireReleaseAL | LSE_LD##A, \
  LSELD_##A##_x   = LseLdOpFixed | LSESizeX | LSEAquireReleaseN  | LSE_LD##A, \
  LSELD_##A##_ax  = LseLdOpFixed | LSESizeX | LSEAquireReleaseA  | LSE_LD##A, \
  LSELD_##A##_lx  = LseLdOpFixed | LSESizeX | LSEAquireReleaseL  | LSE_LD##A, \
  LSELD_##A##_alx = LseLdOpFixed | LSESizeX | LSEAquireReleaseAL | LSE_LD##A
  LSELD_OP_LIST(LSELD)
  #undef LSELD
};

// name from Architecture Reference Manual
enum SIMDScalarThreeSameOp {
  SQADDop    = 0x00000800,
  UQADDop    = 0x20000800,
  SQSUBop    = 0x00001800,
  UQSUBop    = 0x20002800,
  CMGTop     = 0x00003000,
  CMHIop     = 0x20003000,
  CMGEop     = 0x00003800,
  CMHSop     = 0x20003800,
  SSHLop     = 0x00004000,
  USHLop     = 0x20004000,
  SQSHLop    = 0x00004800,
  UQSHLop    = 0x20004800,
  SRSHLop    = 0x00005000,
  URSHLop    = 0x20005000,
  SQRSHLop   = 0x00005800,
  UQRSHLop   = 0x20005800,
  STSADDop   = 0x00008000,
  STSSUBop   = 0x20008000,
  CMEQop     = 0x20008800,
  CMTSTop    = 0x00008800,
  SQDMULHop  = 0x0000B000,
  SQRDMULHop = 0x2000B000,
  FABDop     = 0x2080D000,
  FMULXop    = 0x0000D800,
  FCMEQop    = 0x0000E000,
  FCMGEop    = 0x2000E000,
  FCMGTop    = 0x2080E000,
  FACGEop    = 0x2000E800,
  FACGTop    = 0x2080E800,
};

enum SIMDScalarThreeSameSize {
  SIMDScalarThreeSameDbl = 0x00400000
};

enum SIMDScalarThreeSame {
  SIMDScalarThreeSameFixed = 0x5E200400,
  SIMDScalarThreeSameFMask = 0xDF200C00,
  SIMDScalarThreeSameMask  = 0x2000F800,
  SQADD    = SIMDScalarThreeSameFixed | SQADDop,
  UQADD    = SIMDScalarThreeSameFixed | UQADDop,
  SQSUB    = SIMDScalarThreeSameFixed | SQSUBop,
  UQSUB    = SIMDScalarThreeSameFixed | UQSUBop,
  CMGT     = SIMDScalarThreeSameFixed | CMGTop,
  CMHI     = SIMDScalarThreeSameFixed | CMHIop,
  CMGE     = SIMDScalarThreeSameFixed | CMGEop,
  CMHS     = SIMDScalarThreeSameFixed | CMHSop,
  SSHL     = SIMDScalarThreeSameFixed | SSHLop,
  USHL     = SIMDScalarThreeSameFixed | USHLop,
  SQSHL    = SIMDScalarThreeSameFixed | SQSHLop,
  UQSHL    = SIMDScalarThreeSameFixed | UQSHLop,
  SRSHL    = SIMDScalarThreeSameFixed | SRSHLop,
  URSHL    = SIMDScalarThreeSameFixed | URSHLop,
  SQRSHL   = SIMDScalarThreeSameFixed | SQRSHLop,
  UQRSHL   = SIMDScalarThreeSameFixed | UQRSHLop,
  STSADD   = SIMDScalarThreeSameFixed | STSADDop,
  STSSUB   = SIMDScalarThreeSameFixed | STSSUBop,
  CMEQ     = SIMDScalarThreeSameFixed | CMEQop,
  CMTST    = SIMDScalarThreeSameFixed | CMTSTop,
  SQDMULH  = SIMDScalarThreeSameFixed | SQDMULHop,
  SQRDMULH = SIMDScalarThreeSameFixed | SQRDMULHop,
  FABD     = SIMDScalarThreeSameFixed | FABDop,
  FABD_s   = SIMDScalarThreeSameFixed | FABDop,
  FABD_d   = SIMDScalarThreeSameFixed | FABDop | SIMDScalarThreeSameDbl,
  FMULX    = SIMDScalarThreeSameFixed | FMULXop,
  FCMEQ    = SIMDScalarThreeSameFixed | FCMEQop,
  FCMEQ_s  = SIMDScalarThreeSameFixed | FCMEQop,
  FCMEQ_d  = SIMDScalarThreeSameFixed | FCMEQop | SIMDScalarThreeSameDbl,
  FCMGE    = SIMDScalarThreeSameFixed | FCMGEop,
  FCMGE_s  = SIMDScalarThreeSameFixed | FCMGEop,
  FCMGE_d  = SIMDScalarThreeSameFixed | FCMGEop | SIMDScalarThreeSameDbl,
  FCMGT    = SIMDScalarThreeSameFixed | FCMGTop,
  FCMGT_s  = SIMDScalarThreeSameFixed | FCMGTop,
  FCMGT_d  = SIMDScalarThreeSameFixed | FCMGTop | SIMDScalarThreeSameDbl,
  FACGE    = SIMDScalarThreeSameFixed | FACGEop,
  FACGE_s  = SIMDScalarThreeSameFixed | FACGEop,
  FACGE_d  = SIMDScalarThreeSameFixed | FACGEop | SIMDScalarThreeSameDbl,
  FACGT    = SIMDScalarThreeSameFixed | FACGTop,
  FACGT_s  = SIMDScalarThreeSameFixed | FACGTop,
  FACGT_d  = SIMDScalarThreeSameFixed | FACGTop | SIMDScalarThreeSameDbl
};

enum SIMDScalarTwoMiscOp {
  SUQADDop = 0x00803000,
  USQADDop = 0x20803000,
  SQABSop  = 0x00807000,
  SQNEGop  = 0x20807000,
  CMGEZop  = 0x20008000,
  CMGTZop  = 0x00808000,
  CMEQZop  = 0x00809000,
  CMLEZop  = 0x20809000,
  CMLTZop  = 0x0080A000,
  STMABSop = 0x0080B000,
  STMNEGop = 0x2080B000,
  FCMGEZop = 0x2080C000,
  FCMGTZop = 0x0080C000,
  FCMEQZop = 0x0080D000,
  FCMLEZop = 0x2080D000,
  FCMLTZop = 0x0080E000,
  SQXTUNop = 0x20812000,
  FCVTXNop = 0x20016000,
  FCVTNSop = 0x0001A000,
  FCVTNUop = 0x2001A000,
  FCVTPSop = 0x0081A000,
  FCVTPUop = 0x2081A000,
  FCVTMSop = 0x0001B000,
  FCVTMUop = 0x2001B000,
  FCVTZSop = 0x0081B000,
  FCVTZUop = 0x2081B000,
  FCVTASop = 0x0001C000,
  FCVTAUop = 0x2001C000,
  FRECPEop = 0x0081D000,
  FRSQRTEop= 0x2081D000,
  UCVTFop  = 0x2001D000,
  FRECPXop = 0x0081F000,
  STMUQXTNop = 0x20814000,
  STMSCVTFop = 0x0001D000
};

enum SIMDScalarTwoMiscSize {
  SIMDScalarTwoMiscDbl = 0x00400000
};

enum SIMDScalarTwoMisc {
  SIMDScalarTwoMiscFixed = 0x5E200800,
  SIMDScalarTwoMiscFMask = 0xDF3E0C00,
  SIMDScalarTwoMiscMask  = 0x2081F000,
  SUQADD       = SIMDScalarTwoMiscFixed | SUQADDop,
  SUQADD_s     = SIMDScalarTwoMiscFixed | SUQADDop,
  SUQADD_d     = SIMDScalarTwoMiscFixed | SUQADDop | SIMDScalarTwoMiscDbl,
  USQADD       = SIMDScalarTwoMiscFixed | USQADDop,
  USQADD_s     = SIMDScalarTwoMiscFixed | USQADDop,
  USQADD_d     = SIMDScalarTwoMiscFixed | USQADDop | SIMDScalarTwoMiscDbl,
  SQABS        = SIMDScalarTwoMiscFixed | SQABSop,
  SQABS_s      = SIMDScalarTwoMiscFixed | SQABSop,
  SQABS_d      = SIMDScalarTwoMiscFixed | SQABSop  | SIMDScalarTwoMiscDbl,
  SQNEG        = SIMDScalarTwoMiscFixed | SQNEGop,
  SQNEG_s      = SIMDScalarTwoMiscFixed | SQNEGop,
  SQNEG_d      = SIMDScalarTwoMiscFixed | SQNEGop  | SIMDScalarTwoMiscDbl,
  CMGEZ        = SIMDScalarTwoMiscFixed | CMGEZop  | SIMDScalarTwoMiscDbl,
  CMGEZ_d      = SIMDScalarTwoMiscFixed | CMGEZop  | SIMDScalarTwoMiscDbl,
  CMGTZ        = SIMDScalarTwoMiscFixed | CMGTZop  | SIMDScalarTwoMiscDbl,
  CMGTZ_d      = SIMDScalarTwoMiscFixed | CMGTZop  | SIMDScalarTwoMiscDbl,
  CMEQZ        = SIMDScalarTwoMiscFixed | CMEQZop  | SIMDScalarTwoMiscDbl,
  CMEQZ_d      = SIMDScalarTwoMiscFixed | CMEQZop  | SIMDScalarTwoMiscDbl,
  CMLEZ        = SIMDScalarTwoMiscFixed | CMLEZop  | SIMDScalarTwoMiscDbl,
  CMLEZ_d      = SIMDScalarTwoMiscFixed | CMLEZop  | SIMDScalarTwoMiscDbl,
  CMLTZ        = SIMDScalarTwoMiscFixed | CMLTZop  | SIMDScalarTwoMiscDbl,
  CMLTZ_d      = SIMDScalarTwoMiscFixed | CMLTZop  | SIMDScalarTwoMiscDbl,
  STMABS       = SIMDScalarTwoMiscFixed | STMABSop | SIMDScalarTwoMiscDbl,
  STMABS_d     = SIMDScalarTwoMiscFixed | STMABSop | SIMDScalarTwoMiscDbl,
  STMNEG       = SIMDScalarTwoMiscFixed | STMNEGop | SIMDScalarTwoMiscDbl,
  STMNEG_d     = SIMDScalarTwoMiscFixed | STMNEGop | SIMDScalarTwoMiscDbl,
  FCMGEZ       = SIMDScalarTwoMiscFixed | FCMGEZop,
  FCMGEZ_s     = SIMDScalarTwoMiscFixed | FCMGEZop,
  FCMGEZ_d     = SIMDScalarTwoMiscFixed | FCMGEZop | SIMDScalarTwoMiscDbl,
  FCMGTZ       = SIMDScalarTwoMiscFixed | FCMGTZop,
  FCMGTZ_s     = SIMDScalarTwoMiscFixed | FCMGTZop,
  FCMGTZ_d     = SIMDScalarTwoMiscFixed | FCMGTZop | SIMDScalarTwoMiscDbl,
  FCMEQZ       = SIMDScalarTwoMiscFixed | FCMEQZop,
  FCMEQZ_s     = SIMDScalarTwoMiscFixed | FCMEQZop,
  FCMEQZ_d     = SIMDScalarTwoMiscFixed | FCMEQZop | SIMDScalarTwoMiscDbl,
  FCMLEZ       = SIMDScalarTwoMiscFixed | FCMLEZop,
  FCMLEZ_s     = SIMDScalarTwoMiscFixed | FCMLEZop,
  FCMLEZ_d     = SIMDScalarTwoMiscFixed | FCMLEZop | SIMDScalarTwoMiscDbl,
  FCMLTZ       = SIMDScalarTwoMiscFixed | FCMLTZop,
  FCMLTZ_s     = SIMDScalarTwoMiscFixed | FCMLTZop,
  FCMLTZ_d     = SIMDScalarTwoMiscFixed | FCMLTZop | SIMDScalarTwoMiscDbl,
  SQXTUN       = SIMDScalarTwoMiscFixed | SQXTUNop,
  STMUQXTN     = SIMDScalarTwoMiscFixed | STMUQXTNop,
  FCVTXN       = SIMDScalarTwoMiscFixed | FCVTXNop,
  FCVTXN_s     = SIMDScalarTwoMiscFixed | FCVTXNop,
  STMFCVTNS    = SIMDScalarTwoMiscFixed | FCVTNSop,
  STMFCVTNS_s  = SIMDScalarTwoMiscFixed | FCVTNSop,
  STMFCVTNS_d  = SIMDScalarTwoMiscFixed | FCVTNSop | SIMDScalarTwoMiscDbl,
  STMFCVTNU    = SIMDScalarTwoMiscFixed | FCVTNUop,
  STMFCVTNU_s  = SIMDScalarTwoMiscFixed | FCVTNUop,
  STMFCVTNU_d  = SIMDScalarTwoMiscFixed | FCVTNUop | SIMDScalarTwoMiscDbl,
  STMFCVTPS    = SIMDScalarTwoMiscFixed | FCVTPSop,
  STMFCVTPS_s  = SIMDScalarTwoMiscFixed | FCVTPSop,
  STMFCVTPS_d  = SIMDScalarTwoMiscFixed | FCVTPSop | SIMDScalarTwoMiscDbl,
  STMFCVTPU    = SIMDScalarTwoMiscFixed | FCVTPUop,
  STMFCVTPU_s  = SIMDScalarTwoMiscFixed | FCVTPUop,
  STMFCVTPU_d  = SIMDScalarTwoMiscFixed | FCVTPUop | SIMDScalarTwoMiscDbl,
  STMFCVTMS    = SIMDScalarTwoMiscFixed | FCVTMSop,
  STMFCVTMS_s  = SIMDScalarTwoMiscFixed | FCVTMSop,
  STMFCVTMS_d  = SIMDScalarTwoMiscFixed | FCVTMSop | SIMDScalarTwoMiscDbl,
  STMFCVTMU    = SIMDScalarTwoMiscFixed | FCVTMUop,
  STMFCVTMU_s  = SIMDScalarTwoMiscFixed | FCVTMUop,
  STMFCVTMU_d  = SIMDScalarTwoMiscFixed | FCVTMUop | SIMDScalarTwoMiscDbl,
  STMFCVTZS    = SIMDScalarTwoMiscFixed | FCVTZSop,
  STMFCVTZS_s  = SIMDScalarTwoMiscFixed | FCVTZSop,
  STMFCVTZS_d  = SIMDScalarTwoMiscFixed | FCVTZSop | SIMDScalarTwoMiscDbl,
  STMFCVTZU    = SIMDScalarTwoMiscFixed | FCVTZUop,
  STMFCVTZU_s  = SIMDScalarTwoMiscFixed | FCVTZUop,
  STMFCVTZU_d  = SIMDScalarTwoMiscFixed | FCVTZUop | SIMDScalarTwoMiscDbl,
  STMFCVTAS    = SIMDScalarTwoMiscFixed | FCVTASop,
  STMFCVTAS_s  = SIMDScalarTwoMiscFixed | FCVTASop,
  STMFCVTAS_d  = SIMDScalarTwoMiscFixed | FCVTASop | SIMDScalarTwoMiscDbl,
  STMFCVTAU    = SIMDScalarTwoMiscFixed | FCVTAUop,
  STMFCVTAU_s  = SIMDScalarTwoMiscFixed | FCVTAUop,
  STMFCVTAU_d  = SIMDScalarTwoMiscFixed | FCVTAUop | SIMDScalarTwoMiscDbl,
  FRECPE       = SIMDScalarTwoMiscFixed | FRECPEop,
  FRECPE_s     = SIMDScalarTwoMiscFixed | FRECPEop,
  FRECPE_d     = SIMDScalarTwoMiscFixed | FRECPEop | SIMDScalarTwoMiscDbl,
  FRSQRTE      = SIMDScalarTwoMiscFixed | FRSQRTEop,
  FRSQRTE_s    = SIMDScalarTwoMiscFixed | FRSQRTEop,
  FRSQRTE_d    = SIMDScalarTwoMiscFixed | FRSQRTEop | SIMDScalarTwoMiscDbl,
  STMSCVTF     = SIMDScalarTwoMiscFixed | STMSCVTFop,
  STMSCVTF_s   = SIMDScalarTwoMiscFixed | STMSCVTFop,
  STMSCVTF_d   = SIMDScalarTwoMiscFixed | STMSCVTFop | SIMDScalarTwoMiscDbl,
  STMUCVTF     = SIMDScalarTwoMiscFixed | UCVTFop,
  STMUCVTF_s   = SIMDScalarTwoMiscFixed | UCVTFop,
  STMUCVTF_d   = SIMDScalarTwoMiscFixed | UCVTFop | SIMDScalarTwoMiscDbl,
  FRECPX       = SIMDScalarTwoMiscFixed | FRECPXop,
  FRECPX_s     = SIMDScalarTwoMiscFixed | FRECPXop,
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

}  // namespace vixl

#endif  // VIXL_A64_CONSTANTS_A64_H_
