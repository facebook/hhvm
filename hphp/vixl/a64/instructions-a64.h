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

#ifndef VIXL_A64_INSTRUCTIONS_A64_H_
#define VIXL_A64_INSTRUCTIONS_A64_H_

#include "hphp/vixl/globals.h"
#include "hphp/vixl/utils.h"
#include "hphp/vixl/a64/constants-a64.h"

namespace vixl {
// ISA constants. --------------------------------------------------------------

typedef uint32_t Instr;
const unsigned kInstructionSize = 4;
const unsigned kInstructionSizeLog2 = 2;
const unsigned kLiteralEntrySize = 4;
const unsigned kLiteralEntrySizeLog2 = 2;
const unsigned kMaxLoadLiteralRange = 1 * MBytes;

const unsigned kWRegSize = 32;
const unsigned kWRegSizeLog2 = 5;
const unsigned kWRegSizeInBytes = kWRegSize / 8;
const unsigned kXRegSize = 64;
const unsigned kXRegSizeLog2 = 6;
const unsigned kXRegSizeInBytes = kXRegSize / 8;
const unsigned kSRegSize = 32;
const unsigned kSRegSizeLog2 = 5;
const unsigned kSRegSizeInBytes = kSRegSize / 8;
const unsigned kDRegSize = 64;
const unsigned kDRegSizeLog2 = 6;
const unsigned kDRegSizeInBytes = kDRegSize / 8;
const int64_t kWRegMask = 0x00000000ffffffffL;
const int64_t kXRegMask = 0xffffffffffffffffL;
const int64_t kSRegMask = 0x00000000ffffffffL;
const int64_t kDRegMask = 0xffffffffffffffffL;
const int64_t kXSignMask = 0x1L << 63;
const int64_t kWSignMask = 0x1L << 31;
const int64_t kByteMask = 0xffL;
const int64_t kHalfWordMask = 0xffffL;
const int64_t kWordMask = 0xffffffffL;
const uint64_t kXMaxUInt = 0xffffffffffffffffUL;
const uint64_t kWMaxUInt = 0xffffffffUL;
const int64_t kXMaxInt = 0x7fffffffffffffffL;
const int64_t kXMinInt = 0x8000000000000000L;
const int32_t kWMaxInt = 0x7fffffff;
const int32_t kWMinInt = 0x80000000;
const unsigned kLinkRegCode = 30;
// Register 31 in hardware is the stack pointer. But the zero register is also
// encoded as register 31. Within vixl, we use SPRegInternalCode to distinguish
// the stack pointer (to avoid accidentally emitting encodings where the
// programmer means one and gets the other). It is converted to SPRegCode by the
// assembler.
const unsigned kZeroRegCode = 31;
const unsigned kSPRegCode = 31;
const unsigned kSPRegInternalCode = 32;

// AArch64 floating-point specifics. These match IEEE-754.
const unsigned kDoubleMantissaBits = 52;
const unsigned kDoubleExponentBits = 11;
const unsigned kFloatMantissaBits = 23;
const unsigned kFloatExponentBits = 8;

const float kFP32PositiveInfinity = rawbits_to_float(0x7f800000);
const float kFP32NegativeInfinity = rawbits_to_float(0xff800000);
const double kFP64PositiveInfinity = rawbits_to_double(0x7ff0000000000000UL);
const double kFP64NegativeInfinity = rawbits_to_double(0xfff0000000000000UL);

// This value is a signalling NaN as both a double and as a float (taking the
// least-significant word).
const double kFP64SignallingNaN = rawbits_to_double(0x7ff000007f800001);
const float kFP32SignallingNaN = rawbits_to_float(0x7f800001);

// A similar value, but as a quiet NaN.
const double kFP64QuietNaN = rawbits_to_double(0x7ff800007fc00001);
const float kFP32QuietNaN = rawbits_to_float(0x7fc00001);

enum LSDataSize {
  LSByte        = 0,
  LSHalfword    = 1,
  LSWord        = 2,
  LSDoubleWord  = 3
};

LSDataSize CalcLSPairDataSize(LoadStorePairOp op);

enum ImmBranchType {
  UnknownBranchType = 0,
  CondBranchType    = 1,
  UncondBranchType  = 2,
  CompareBranchType = 3,
  TestBranchType    = 4
};

enum AddrMode {
  Offset,
  PreIndex,
  PostIndex
};

enum FPRounding {
  // The first four values are encodable directly by FPCR<RMode>.
  FPTieEven = 0x0,
  FPPositiveInfinity = 0x1,
  FPNegativeInfinity = 0x2,
  FPZero = 0x3,

  // The final rounding mode is only available when explicitly specified by the
  // instruction (such as with fcvta). It cannot be set in FPCR.
  FPTieAway
};

enum Reg31Mode {
  Reg31IsStackPointer,
  Reg31IsZeroRegister
};

// Instructions. ---------------------------------------------------------------

class Instruction {
 public:
  inline Instr InstructionBits() const {
    return *(reinterpret_cast<const Instr*>(this));
  }

  inline void SetInstructionBits(Instr new_instr) {
    *(reinterpret_cast<Instr*>(this)) = new_instr;
  }

  inline int Bit(int pos) const {
    return (InstructionBits() >> pos) & 1;
  }

  inline uint32_t Bits(int msb, int lsb) const {
    return unsigned_bitextract_32(msb, lsb, InstructionBits());
  }

  inline int32_t SignedBits(int msb, int lsb) const {
    int32_t bits = *(reinterpret_cast<const int32_t*>(this));
    return signed_bitextract_32(msb, lsb, bits);
  }

  inline Instr Mask(uint32_t mask) const {
    return InstructionBits() & mask;
  }

  #define DEFINE_GETTER(Name, HighBit, LowBit, Func)             \
  inline int64_t Name() const { return Func(HighBit, LowBit); }
  INSTRUCTION_FIELDS_LIST(DEFINE_GETTER)
  #undef DEFINE_GETTER

  // ImmPCRel is a compound field (not present in INSTRUCTION_FIELDS_LIST),
  // formed from ImmPCRelLo and ImmPCRelHi.
  int ImmPCRel() const {
    int const offset = ((ImmPCRelHi() << ImmPCRelLo_width) | ImmPCRelLo());
    int const width = ImmPCRelLo_width + ImmPCRelHi_width;
    return signed_bitextract_32(width-1, 0, offset);
  }

  uint64_t ImmLogical();
  float ImmFP32();
  double ImmFP64();

  inline LSDataSize SizeLSPair() const {
    return CalcLSPairDataSize(
             static_cast<LoadStorePairOp>(Mask(LoadStorePairMask)));
  }

  // Helpers.
  inline bool IsCondBranchImm() const {
    return Mask(ConditionalBranchFMask) == ConditionalBranchFixed;
  }

  inline bool IsUncondBranchImm() const {
    return Mask(UnconditionalBranchFMask) == UnconditionalBranchFixed;
  }

  inline bool IsCompareBranch() const {
    return Mask(CompareBranchFMask) == CompareBranchFixed;
  }

  inline bool IsTestBranch() const {
    return Mask(TestBranchFMask) == TestBranchFixed;
  }

  inline bool IsPCRelAddressing() const {
    return Mask(PCRelAddressingFMask) == PCRelAddressingFixed;
  }

  inline bool IsLogicalImmediate() const {
    return Mask(LogicalImmediateFMask) == LogicalImmediateFixed;
  }

  inline bool IsAddSubImmediate() const {
    return Mask(AddSubImmediateFMask) == AddSubImmediateFixed;
  }

  inline bool IsAddSubExtended() const {
    return Mask(AddSubExtendedFMask) == AddSubExtendedFixed;
  }

  inline bool IsLoadOrStore() const {
    return Mask(LoadStoreAnyFMask) == LoadStoreAnyFixed;
  }

  inline bool IsMovn() const {
    return (Mask(MoveWideImmediateMask) == MOVN_x) ||
           (Mask(MoveWideImmediateMask) == MOVN_w);
  }

  // Indicate whether Rd can be the stack pointer or the zero register. This
  // does not check that the instruction actually has an Rd field.
  inline Reg31Mode RdMode() const {
    // The following instructions use sp or wsp as Rd:
    //  Add/sub (immediate) when not setting the flags.
    //  Add/sub (extended) when not setting the flags.
    //  Logical (immediate) when not setting the flags.
    // Otherwise, r31 is the zero register.
    if (IsAddSubImmediate() || IsAddSubExtended()) {
      if (Mask(AddSubSetFlagsBit)) {
        return Reg31IsZeroRegister;
      } else {
        return Reg31IsStackPointer;
      }
    }
    if (IsLogicalImmediate()) {
      // Of the logical (immediate) instructions, only ANDS (and its aliases)
      // can set the flags. The others can all write into sp.
      // Note that some logical operations are not available to
      // immediate-operand instructions, so we have to combine two masks here.
      if (Mask(LogicalImmediateMask & LogicalOpMask) == ANDS) {
        return Reg31IsZeroRegister;
      } else {
        return Reg31IsStackPointer;
      }
    }
    return Reg31IsZeroRegister;
  }

  // Indicate whether Rn can be the stack pointer or the zero register. This
  // does not check that the instruction actually has an Rn field.
  inline Reg31Mode RnMode() const {
    // The following instructions use sp or wsp as Rn:
    //  All loads and stores.
    //  Add/sub (immediate).
    //  Add/sub (extended).
    // Otherwise, r31 is the zero register.
    if (IsLoadOrStore() || IsAddSubImmediate() || IsAddSubExtended()) {
      return Reg31IsStackPointer;
    }
    return Reg31IsZeroRegister;
  }

  inline ImmBranchType BranchType() const {
    if (IsCondBranchImm()) {
      return CondBranchType;
    } else if (IsUncondBranchImm()) {
      return UncondBranchType;
    } else if (IsCompareBranch()) {
      return CompareBranchType;
    } else if (IsTestBranch()) {
      return TestBranchType;
    } else {
      return UnknownBranchType;
    }
  }

  // Find the target of this instruction. 'this' may be a branch or a
  // PC-relative addressing instruction.
  Instruction* ImmPCOffsetTarget();

  // Patch a PC-relative offset to refer to 'target'. 'this' may be a branch or
  // a PC-relative addressing instruction.
  void SetImmPCOffsetTarget(Instruction* target);
  // Patch a literal load instruction to load from 'source'.
  void SetImmLLiteral(Instruction* source);

  inline uint8_t* LiteralAddress() {
    int offset = ImmLLiteral() << kLiteralEntrySizeLog2;
    return reinterpret_cast<uint8_t*>(this) + offset;
  }

  inline uint32_t Literal32() {
    uint32_t literal;
    memcpy(&literal, LiteralAddress(), sizeof(literal));

    return literal;
  }

  inline uint64_t Literal64() {
    uint64_t literal;
    memcpy(&literal, LiteralAddress(), sizeof(literal));

    return literal;
  }

  inline float LiteralFP32() {
    return rawbits_to_float(Literal32());
  }

  inline double LiteralFP64() {
    return rawbits_to_double(Literal64());
  }

  inline Instruction* NextInstruction() {
    return this + kInstructionSize;
  }

  inline Instruction* InstructionAtOffset(int64_t offset) {
    assert(IsWordAligned(this + offset));
    return this + offset;
  }

  template<typename T> static inline Instruction* Cast(T src) {
    return reinterpret_cast<Instruction*>(src);
  }

 private:
  inline int ImmBranch() const;

  void SetPCRelImmTarget(Instruction* target);
  void SetPCRelLoadStoreTarget(Instruction* target);
  void SetBranchImmTarget(Instruction* target);
};
}  // namespace vixl

#endif  // VIXL_A64_INSTRUCTIONS_A64_H_
