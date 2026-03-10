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

#ifndef VIXL_AARCH64_INSTRUCTIONS_AARCH64_H_
#define VIXL_AARCH64_INSTRUCTIONS_AARCH64_H_

#include "../globals-vixl.h"
#include "../utils-vixl.h"

#include "constants-aarch64.h"

namespace vixl {
namespace aarch64 {
// ISA constants. --------------------------------------------------------------

typedef uint32_t Instr;
const unsigned kInstructionSize = 4;
const unsigned kInstructionSizeLog2 = 2;
const unsigned kLiteralEntrySize = 4;
const unsigned kLiteralEntrySizeLog2 = 2;
const unsigned kMaxLoadLiteralRange = 1 * MBytes;

// This is the nominal page size (as used by the adrp instruction); the actual
// size of the memory pages allocated by the kernel is likely to differ.
const unsigned kPageSize = 4 * KBytes;
const unsigned kPageSizeLog2 = 12;

const unsigned kBRegSize = 8;
const unsigned kBRegSizeLog2 = 3;
const unsigned kBRegSizeInBytes = kBRegSize / 8;
const unsigned kBRegSizeInBytesLog2 = kBRegSizeLog2 - 3;
const unsigned kHRegSize = 16;
const unsigned kHRegSizeLog2 = 4;
const unsigned kHRegSizeInBytes = kHRegSize / 8;
const unsigned kHRegSizeInBytesLog2 = kHRegSizeLog2 - 3;
const unsigned kWRegSize = 32;
const unsigned kWRegSizeLog2 = 5;
const unsigned kWRegSizeInBytes = kWRegSize / 8;
const unsigned kWRegSizeInBytesLog2 = kWRegSizeLog2 - 3;
const unsigned kXRegSize = 64;
const unsigned kXRegSizeLog2 = 6;
const unsigned kXRegSizeInBytes = kXRegSize / 8;
const unsigned kXRegSizeInBytesLog2 = kXRegSizeLog2 - 3;
const unsigned kSRegSize = 32;
const unsigned kSRegSizeLog2 = 5;
const unsigned kSRegSizeInBytes = kSRegSize / 8;
const unsigned kSRegSizeInBytesLog2 = kSRegSizeLog2 - 3;
const unsigned kDRegSize = 64;
const unsigned kDRegSizeLog2 = 6;
const unsigned kDRegSizeInBytes = kDRegSize / 8;
const unsigned kDRegSizeInBytesLog2 = kDRegSizeLog2 - 3;
const unsigned kQRegSize = 128;
const unsigned kQRegSizeLog2 = 7;
const unsigned kQRegSizeInBytes = kQRegSize / 8;
const unsigned kQRegSizeInBytesLog2 = kQRegSizeLog2 - 3;
const uint64_t kWRegMask = UINT64_C(0xffffffff);
const uint64_t kXRegMask = UINT64_C(0xffffffffffffffff);
const uint64_t kHRegMask = UINT64_C(0xffff);
const uint64_t kSRegMask = UINT64_C(0xffffffff);
const uint64_t kDRegMask = UINT64_C(0xffffffffffffffff);
const uint64_t kHSignMask = UINT64_C(0x8000);
const uint64_t kSSignMask = UINT64_C(0x80000000);
const uint64_t kDSignMask = UINT64_C(0x8000000000000000);
const uint64_t kWSignMask = UINT64_C(0x80000000);
const uint64_t kXSignMask = UINT64_C(0x8000000000000000);
const uint64_t kByteMask = UINT64_C(0xff);
const uint64_t kHalfWordMask = UINT64_C(0xffff);
const uint64_t kWordMask = UINT64_C(0xffffffff);
const uint64_t kXMaxUInt = UINT64_C(0xffffffffffffffff);
const uint64_t kWMaxUInt = UINT64_C(0xffffffff);
const uint64_t kHMaxUInt = UINT64_C(0xffff);
// Define k*MinInt with "-k*MaxInt - 1", because the hexadecimal representation
// (e.g. "INT32_C(0x80000000)") has implementation-defined behaviour.
const int64_t kXMaxInt = INT64_C(0x7fffffffffffffff);
const int64_t kXMinInt = -kXMaxInt - 1;
const int32_t kWMaxInt = INT32_C(0x7fffffff);
const int32_t kWMinInt = -kWMaxInt - 1;
const int16_t kHMaxInt = INT16_C(0x7fff);
const int16_t kHMinInt = -kHMaxInt - 1;
const unsigned kFpRegCode = 29;
const unsigned kLinkRegCode = 30;
const unsigned kSpRegCode = 31;
const unsigned kZeroRegCode = 31;
const unsigned kSPRegInternalCode = 63;
const unsigned kRegCodeMask = 0x1f;

const unsigned kAtomicAccessGranule = 16;

const unsigned kAddressTagOffset = 56;
const unsigned kAddressTagWidth = 8;
const uint64_t kAddressTagMask = ((UINT64_C(1) << kAddressTagWidth) - 1)
                                 << kAddressTagOffset;
VIXL_STATIC_ASSERT(kAddressTagMask == UINT64_C(0xff00000000000000));

const uint64_t kTTBRMask = UINT64_C(1) << 55;

// We can't define a static kZRegSize because the size depends on the
// implementation. However, it is sometimes useful to know the minimum and
// maximum possible sizes.
const unsigned kZRegMinSize = 128;
const unsigned kZRegMinSizeLog2 = 7;
const unsigned kZRegMinSizeInBytes = kZRegMinSize / 8;
const unsigned kZRegMinSizeInBytesLog2 = kZRegMinSizeLog2 - 3;
const unsigned kZRegMaxSize = 2048;
const unsigned kZRegMaxSizeLog2 = 11;
const unsigned kZRegMaxSizeInBytes = kZRegMaxSize / 8;
const unsigned kZRegMaxSizeInBytesLog2 = kZRegMaxSizeLog2 - 3;

// The P register size depends on the Z register size.
const unsigned kZRegBitsPerPRegBit = kBitsPerByte;
const unsigned kZRegBitsPerPRegBitLog2 = 3;
const unsigned kPRegMinSize = kZRegMinSize / kZRegBitsPerPRegBit;
const unsigned kPRegMinSizeLog2 = kZRegMinSizeLog2 - 3;
const unsigned kPRegMinSizeInBytes = kPRegMinSize / 8;
const unsigned kPRegMinSizeInBytesLog2 = kPRegMinSizeLog2 - 3;
const unsigned kPRegMaxSize = kZRegMaxSize / kZRegBitsPerPRegBit;
const unsigned kPRegMaxSizeLog2 = kZRegMaxSizeLog2 - 3;
const unsigned kPRegMaxSizeInBytes = kPRegMaxSize / 8;
const unsigned kPRegMaxSizeInBytesLog2 = kPRegMaxSizeLog2 - 3;

const unsigned kMTETagGranuleInBytes = 16;
const unsigned kMTETagGranuleInBytesLog2 = 4;
const unsigned kMTETagWidth = 4;

// Make these moved float constants backwards compatible
// with explicit vixl::aarch64:: namespace references.
using vixl::kDoubleExponentBits;
using vixl::kDoubleMantissaBits;
using vixl::kFloat16ExponentBits;
using vixl::kFloat16MantissaBits;
using vixl::kFloatExponentBits;
using vixl::kFloatMantissaBits;

using vixl::kFP16NegativeInfinity;
using vixl::kFP16PositiveInfinity;
using vixl::kFP32NegativeInfinity;
using vixl::kFP32PositiveInfinity;
using vixl::kFP64NegativeInfinity;
using vixl::kFP64PositiveInfinity;

using vixl::kFP16DefaultNaN;
using vixl::kFP32DefaultNaN;
using vixl::kFP64DefaultNaN;

unsigned CalcLSDataSize(LoadStoreOp op);
unsigned CalcLSPairDataSize(LoadStorePairOp op);

enum ImmBranchType {
  UnknownBranchType = 0,
  CondBranchType = 1,
  UncondBranchType = 2,
  CompareBranchType = 3,
  TestBranchType = 4
};

enum AddrMode { Offset, PreIndex, PostIndex };

enum Reg31Mode { Reg31IsStackPointer, Reg31IsZeroRegister };

enum VectorFormat {
  kFormatUndefined = 0xffffffff,
  kFormat8B = NEON_8B,
  kFormat16B = NEON_16B,
  kFormat4H = NEON_4H,
  kFormat8H = NEON_8H,
  kFormat2S = NEON_2S,
  kFormat4S = NEON_4S,
  kFormat1D = NEON_1D,
  kFormat2D = NEON_2D,

  // Scalar formats. We add the scalar bit to distinguish between scalar and
  // vector enumerations; the bit is always set in the encoding of scalar ops
  // and always clear for vector ops. Although kFormatD and kFormat1D appear
  // to be the same, their meaning is subtly different. The first is a scalar
  // operation, the second a vector operation that only affects one lane.
  kFormatB = NEON_B | NEONScalar,
  kFormatH = NEON_H | NEONScalar,
  kFormatS = NEON_S | NEONScalar,
  kFormatD = NEON_D | NEONScalar,

  // An artificial value, used to distinguish from NEON format category.
  kFormatSVE = 0x0000fffd,
  // Artificial values. Q and O lane sizes aren't encoded in the usual size
  // field.
  kFormatSVEQ = 0x00080000,
  kFormatSVEO = 0x00040000,

  // Vector element width of SVE register with the unknown lane count since
  // the vector length is implementation dependent.
  kFormatVnB = SVE_B | kFormatSVE,
  kFormatVnH = SVE_H | kFormatSVE,
  kFormatVnS = SVE_S | kFormatSVE,
  kFormatVnD = SVE_D | kFormatSVE,
  kFormatVnQ = kFormatSVEQ | kFormatSVE,
  kFormatVnO = kFormatSVEO | kFormatSVE,

  // Artificial values, used by simulator trace tests and a few oddball
  // instructions (such as FMLAL).
  kFormat2H = 0xfffffffe,
  kFormat1Q = 0xfffffffd
};

// Instructions. ---------------------------------------------------------------

class Instruction {
 public:
  Instr GetInstructionBits() const {
    return *(reinterpret_cast<const Instr*>(this));
  }
  VIXL_DEPRECATED("GetInstructionBits", Instr InstructionBits() const) {
    return GetInstructionBits();
  }

  void SetInstructionBits(Instr new_instr) {
    *(reinterpret_cast<Instr*>(this)) = new_instr;
  }

  int ExtractBit(int pos) const { return (GetInstructionBits() >> pos) & 1; }
  VIXL_DEPRECATED("ExtractBit", int Bit(int pos) const) {
    return ExtractBit(pos);
  }

  uint32_t ExtractBits(int msb, int lsb) const {
    return ExtractUnsignedBitfield32(msb, lsb, GetInstructionBits());
  }
  VIXL_DEPRECATED("ExtractBits", uint32_t Bits(int msb, int lsb) const) {
    return ExtractBits(msb, lsb);
  }

  // Compress bit extraction operation from Hacker's Delight.
  // https://github.com/hcs0/Hackers-Delight/blob/master/compress.c.txt
  uint32_t Compress(uint32_t mask) const {
    uint32_t mk, mp, mv, t;
    uint32_t x = GetInstructionBits() & mask;  // Clear irrelevant bits.
    mk = ~mask << 1;                           // We will count 0's to right.
    for (int i = 0; i < 5; i++) {
      mp = mk ^ (mk << 1);  // Parallel suffix.
      mp = mp ^ (mp << 2);
      mp = mp ^ (mp << 4);
      mp = mp ^ (mp << 8);
      mp = mp ^ (mp << 16);
      mv = mp & mask;                         // Bits to move.
      mask = (mask ^ mv) | (mv >> (1 << i));  // Compress mask.
      t = x & mv;
      x = (x ^ t) | (t >> (1 << i));  // Compress x.
      mk = mk & ~mp;
    }
    return x;
  }

  template <uint32_t M>
  uint32_t ExtractBits() const {
    return Compress(M);
  }

  uint32_t ExtractBitsAbsent() const {
    VIXL_UNREACHABLE();
    return 0;
  }

  template <uint32_t M, uint32_t V>
  uint32_t IsMaskedValue() const {
    return (Mask(M) == V) ? 1 : 0;
  }

  uint32_t IsMaskedValueAbsent() const {
    VIXL_UNREACHABLE();
    return 0;
  }

  int32_t ExtractSignedBits(int msb, int lsb) const {
    int32_t bits = *(reinterpret_cast<const int32_t*>(this));
    return ExtractSignedBitfield32(msb, lsb, bits);
  }
  VIXL_DEPRECATED("ExtractSignedBits",
                  int32_t SignedBits(int msb, int lsb) const) {
    return ExtractSignedBits(msb, lsb);
  }

  Instr Mask(uint32_t mask) const {
    VIXL_ASSERT(mask != 0);
    return GetInstructionBits() & mask;
  }

#define DEFINE_GETTER(Name, HighBit, LowBit, Func)                  \
  int32_t Get##Name() const { return this->Func(HighBit, LowBit); } \
  VIXL_DEPRECATED("Get" #Name, int32_t Name() const) { return Get##Name(); }
  INSTRUCTION_FIELDS_LIST(DEFINE_GETTER)
#undef DEFINE_GETTER

  template <int msb, int lsb>
  int32_t GetRx() const {
    // We don't have any register fields wider than five bits, so the result
    // will always fit into an int32_t.
    VIXL_ASSERT((msb - lsb + 1) <= 5);
    return this->ExtractBits(msb, lsb);
  }

  VectorFormat GetSVEVectorFormat(int field_lsb = 22) const {
    VIXL_ASSERT((field_lsb >= 0) && (field_lsb <= 30));
    uint32_t instr = ExtractUnsignedBitfield32(field_lsb + 1,
                                               field_lsb,
                                               GetInstructionBits())
                     << 22;
    switch (instr & SVESizeFieldMask) {
      case SVE_B:
        return kFormatVnB;
      case SVE_H:
        return kFormatVnH;
      case SVE_S:
        return kFormatVnS;
      case SVE_D:
        return kFormatVnD;
    }
    VIXL_UNREACHABLE();
    return kFormatUndefined;
  }

  // ImmPCRel is a compound field (not present in INSTRUCTION_FIELDS_LIST),
  // formed from ImmPCRelLo and ImmPCRelHi.
  int GetImmPCRel() const {
    uint32_t hi = static_cast<uint32_t>(GetImmPCRelHi());
    uint32_t lo = GetImmPCRelLo();
    uint32_t offset = (hi << ImmPCRelLo_width) | lo;
    int width = ImmPCRelLo_width + ImmPCRelHi_width;
    return ExtractSignedBitfield32(width - 1, 0, offset);
  }
  VIXL_DEPRECATED("GetImmPCRel", int ImmPCRel() const) { return GetImmPCRel(); }

  // ImmLSPAC is a compound field (not present in INSTRUCTION_FIELDS_LIST),
  // formed from ImmLSPACLo and ImmLSPACHi.
  int GetImmLSPAC() const {
    uint32_t hi = static_cast<uint32_t>(GetImmLSPACHi());
    uint32_t lo = GetImmLSPACLo();
    uint32_t offset = (hi << ImmLSPACLo_width) | lo;
    int width = ImmLSPACLo_width + ImmLSPACHi_width;
    return ExtractSignedBitfield32(width - 1, 0, offset) << 3;
  }

  uint64_t GetImmLogical() const;
  VIXL_DEPRECATED("GetImmLogical", uint64_t ImmLogical() const) {
    return GetImmLogical();
  }
  uint64_t GetSVEImmLogical() const;
  int GetSVEBitwiseImmLaneSizeInBytesLog2() const;
  uint64_t DecodeImmBitMask(int32_t n,
                            int32_t imm_s,
                            int32_t imm_r,
                            int32_t size) const;

  std::pair<int, int> GetSVEPermuteIndexAndLaneSizeLog2() const;

  std::pair<int, int> GetNEONMulRmAndIndex() const;
  std::pair<int, int> GetSVEMulZmAndIndex() const;
  std::pair<int, int> GetSVEMulLongZmAndIndex() const;

  std::pair<int, int> GetSVEImmShiftAndLaneSizeLog2(bool is_predicated) const;

  int GetSVEExtractImmediate() const;

  int GetSVEMsizeFromDtype(bool is_signed, int dtype_h_lsb = 23) const;

  int GetSVEEsizeFromDtype(bool is_signed, int dtype_l_lsb = 21) const;


  unsigned GetImmNEONabcdefgh() const;
  VIXL_DEPRECATED("GetImmNEONabcdefgh", unsigned ImmNEONabcdefgh() const) {
    return GetImmNEONabcdefgh();
  }

  Float16 GetImmFP16() const;

  float GetImmFP32() const;
  VIXL_DEPRECATED("GetImmFP32", float ImmFP32() const) { return GetImmFP32(); }

  double GetImmFP64() const;
  VIXL_DEPRECATED("GetImmFP64", double ImmFP64() const) { return GetImmFP64(); }

  Float16 GetImmNEONFP16() const;

  float GetImmNEONFP32() const;
  VIXL_DEPRECATED("GetImmNEONFP32", float ImmNEONFP32() const) {
    return GetImmNEONFP32();
  }

  double GetImmNEONFP64() const;
  VIXL_DEPRECATED("GetImmNEONFP64", double ImmNEONFP64() const) {
    return GetImmNEONFP64();
  }

  Float16 GetSVEImmFP16() const { return Imm8ToFloat16(ExtractBits(12, 5)); }

  float GetSVEImmFP32() const { return Imm8ToFP32(ExtractBits(12, 5)); }

  double GetSVEImmFP64() const { return Imm8ToFP64(ExtractBits(12, 5)); }

  static Float16 Imm8ToFloat16(uint32_t imm8);
  static float Imm8ToFP32(uint32_t imm8);
  static double Imm8ToFP64(uint32_t imm8);

  unsigned GetSizeLS() const {
    return CalcLSDataSize(static_cast<LoadStoreOp>(Mask(LoadStoreMask)));
  }
  VIXL_DEPRECATED("GetSizeLS", unsigned SizeLS() const) { return GetSizeLS(); }

  unsigned GetSizeLSPair() const {
    return CalcLSPairDataSize(
        static_cast<LoadStorePairOp>(Mask(LoadStorePairMask)));
  }
  VIXL_DEPRECATED("GetSizeLSPair", unsigned SizeLSPair() const) {
    return GetSizeLSPair();
  }

  int GetNEONLSIndex(int access_size_shift) const {
    int64_t q = GetNEONQ();
    int64_t s = GetNEONS();
    int64_t size = GetNEONLSSize();
    int64_t index = (q << 3) | (s << 2) | size;
    return static_cast<int>(index >> access_size_shift);
  }
  VIXL_DEPRECATED("GetNEONLSIndex",
                  int NEONLSIndex(int access_size_shift) const) {
    return GetNEONLSIndex(access_size_shift);
  }

  // Helpers.
  bool IsCondBranchImm() const {
    return Mask(ConditionalBranchFMask) == ConditionalBranchFixed;
  }

  bool IsUncondBranchImm() const {
    return Mask(UnconditionalBranchFMask) == UnconditionalBranchFixed;
  }

  bool IsCompareBranch() const {
    return Mask(CompareBranchFMask) == CompareBranchFixed;
  }

  bool IsTestBranch() const { return Mask(TestBranchFMask) == TestBranchFixed; }

  bool IsImmBranch() const { return GetBranchType() != UnknownBranchType; }

  bool IsPCRelAddressing() const {
    return Mask(PCRelAddressingFMask) == PCRelAddressingFixed;
  }

  bool IsLogicalImmediate() const {
    return Mask(LogicalImmediateFMask) == LogicalImmediateFixed;
  }

  bool IsAddSubImmediate() const {
    return Mask(AddSubImmediateFMask) == AddSubImmediateFixed;
  }

  bool IsAddSubExtended() const {
    return Mask(AddSubExtendedFMask) == AddSubExtendedFixed;
  }

  bool IsLoadOrStore() const {
    return Mask(LoadStoreAnyFMask) == LoadStoreAnyFixed;
  }

  // True if `this` is valid immediately after the provided movprfx instruction.
  bool CanTakeSVEMovprfx(uint32_t form_hash, Instruction const* movprfx) const;
  bool CanTakeSVEMovprfx(const char* form, Instruction const* movprfx) const;

  bool IsLoad() const;
  bool IsStore() const;

  bool IsLoadLiteral() const {
    // This includes PRFM_lit.
    return Mask(LoadLiteralFMask) == LoadLiteralFixed;
  }

  bool IsMovn() const {
    return (Mask(MoveWideImmediateMask) == MOVN_x) ||
           (Mask(MoveWideImmediateMask) == MOVN_w);
  }

  bool IsException() const { return Mask(ExceptionFMask) == ExceptionFixed; }

  bool IsPAuth() const { return Mask(SystemPAuthFMask) == SystemPAuthFixed; }

  bool IsBti() const {
    if (Mask(SystemHintFMask) == SystemHintFixed) {
      int imm_hint = GetImmHint();
      switch (imm_hint) {
        case BTI:
        case BTI_c:
        case BTI_j:
        case BTI_jc:
          return true;
      }
    }
    return false;
  }

  bool IsMOPSPrologueOf(const Instruction* instr, uint32_t mops_type) const {
    VIXL_ASSERT((mops_type == "set"_h) || (mops_type == "setg"_h) ||
                (mops_type == "cpy"_h));
    const int op_lsb = (mops_type == "cpy"_h) ? 22 : 14;
    return GetInstructionBits() == instr->Mask(~(0x3U << op_lsb));
  }

  bool IsMOPSMainOf(const Instruction* instr, uint32_t mops_type) const {
    VIXL_ASSERT((mops_type == "set"_h) || (mops_type == "setg"_h) ||
                (mops_type == "cpy"_h));
    const int op_lsb = (mops_type == "cpy"_h) ? 22 : 14;
    return GetInstructionBits() ==
           (instr->Mask(~(0x3U << op_lsb)) | (0x1 << op_lsb));
  }

  bool IsMOPSEpilogueOf(const Instruction* instr, uint32_t mops_type) const {
    VIXL_ASSERT((mops_type == "set"_h) || (mops_type == "setg"_h) ||
                (mops_type == "cpy"_h));
    const int op_lsb = (mops_type == "cpy"_h) ? 22 : 14;
    return GetInstructionBits() ==
           (instr->Mask(~(0x3U << op_lsb)) | (0x2 << op_lsb));
  }

  template <uint32_t mops_type>
  bool IsConsistentMOPSTriplet() const {
    VIXL_STATIC_ASSERT((mops_type == "set"_h) || (mops_type == "setg"_h) ||
                       (mops_type == "cpy"_h));

    int64_t isize = static_cast<int64_t>(kInstructionSize);
    const Instruction* prev2 = GetInstructionAtOffset(-2 * isize);
    const Instruction* prev1 = GetInstructionAtOffset(-1 * isize);
    const Instruction* next1 = GetInstructionAtOffset(1 * isize);
    const Instruction* next2 = GetInstructionAtOffset(2 * isize);

    // Use the encoding of the current instruction to determine the expected
    // adjacent instructions. NB. this doesn't check if the nearby instructions
    // are MOPS-type, but checks that they form a consistent triplet if they
    // are. For example, 'mov x0, #0; mov x0, #512; mov x0, #1024' is a
    // consistent triplet, but they are not MOPS instructions.
    const int op_lsb = (mops_type == "cpy"_h) ? 22 : 14;
    const uint32_t kMOPSOpfield = 0x3 << op_lsb;
    const uint32_t kMOPSPrologue = 0;
    const uint32_t kMOPSMain = 0x1 << op_lsb;
    const uint32_t kMOPSEpilogue = 0x2 << op_lsb;
    switch (Mask(kMOPSOpfield)) {
      case kMOPSPrologue:
        return next1->IsMOPSMainOf(this, mops_type) &&
               next2->IsMOPSEpilogueOf(this, mops_type);
      case kMOPSMain:
        return prev1->IsMOPSPrologueOf(this, mops_type) &&
               next1->IsMOPSEpilogueOf(this, mops_type);
      case kMOPSEpilogue:
        return prev2->IsMOPSPrologueOf(this, mops_type) &&
               prev1->IsMOPSMainOf(this, mops_type);
      default:
        VIXL_ABORT_WITH_MSG("Undefined MOPS operation\n");
    }
  }

  static int GetImmBranchRangeBitwidth(ImmBranchType branch_type);
  VIXL_DEPRECATED(
      "GetImmBranchRangeBitwidth",
      static int ImmBranchRangeBitwidth(ImmBranchType branch_type)) {
    return GetImmBranchRangeBitwidth(branch_type);
  }

  static int32_t GetImmBranchForwardRange(ImmBranchType branch_type);
  VIXL_DEPRECATED(
      "GetImmBranchForwardRange",
      static int32_t ImmBranchForwardRange(ImmBranchType branch_type)) {
    return GetImmBranchForwardRange(branch_type);
  }

  static bool IsValidImmPCOffset(ImmBranchType branch_type, int64_t offset);

  // Indicate whether Rd can be the stack pointer or the zero register. This
  // does not check that the instruction actually has an Rd field.
  Reg31Mode GetRdMode() const {
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
  VIXL_DEPRECATED("GetRdMode", Reg31Mode RdMode() const) { return GetRdMode(); }

  // Indicate whether Rn can be the stack pointer or the zero register. This
  // does not check that the instruction actually has an Rn field.
  Reg31Mode GetRnMode() const {
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
  VIXL_DEPRECATED("GetRnMode", Reg31Mode RnMode() const) { return GetRnMode(); }

  ImmBranchType GetBranchType() const {
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
  VIXL_DEPRECATED("GetBranchType", ImmBranchType BranchType() const) {
    return GetBranchType();
  }

  // Find the target of this instruction. 'this' may be a branch or a
  // PC-relative addressing instruction.
  const Instruction* GetImmPCOffsetTarget() const;
  VIXL_DEPRECATED("GetImmPCOffsetTarget",
                  const Instruction* ImmPCOffsetTarget() const) {
    return GetImmPCOffsetTarget();
  }

  // Patch a PC-relative offset to refer to 'target'. 'this' may be a branch or
  // a PC-relative addressing instruction.
  void SetImmPCOffsetTarget(const Instruction* target);
  // Patch a literal load instruction to load from 'source'.
  void SetImmLLiteral(const Instruction* source);

  // The range of a load literal instruction, expressed as 'instr +- range'.
  // The range is actually the 'positive' range; the branch instruction can
  // target [instr - range - kInstructionSize, instr + range].
  static const int kLoadLiteralImmBitwidth = 19;
  static const int kLoadLiteralRange =
      (1 << kLoadLiteralImmBitwidth) / 2 - kInstructionSize;

  // Calculate the address of a literal referred to by a load-literal
  // instruction, and return it as the specified type.
  //
  // The literal itself is safely mutable only if the backing buffer is safely
  // mutable.
  template <typename T>
  T GetLiteralAddress() const {
    uint64_t base_raw = reinterpret_cast<uint64_t>(this);
    int64_t offset = GetImmLLiteral() * static_cast<int>(kLiteralEntrySize);
    uint64_t address_raw = base_raw + offset;

    // Cast the address using a C-style cast. A reinterpret_cast would be
    // appropriate, but it can't cast one integral type to another.
    T address = (T)(address_raw);

    // Assert that the address can be represented by the specified type.
    VIXL_ASSERT((uint64_t)(address) == address_raw);

    return address;
  }
  template <typename T>
  VIXL_DEPRECATED("GetLiteralAddress", T LiteralAddress() const) {
    return GetLiteralAddress<T>();
  }

  uint32_t GetLiteral32() const {
    uint32_t literal;
    memcpy(&literal, GetLiteralAddress<const void*>(), sizeof(literal));
    return literal;
  }
  VIXL_DEPRECATED("GetLiteral32", uint32_t Literal32() const) {
    return GetLiteral32();
  }

  uint64_t GetLiteral64() const {
    uint64_t literal;
    memcpy(&literal, GetLiteralAddress<const void*>(), sizeof(literal));
    return literal;
  }
  VIXL_DEPRECATED("GetLiteral64", uint64_t Literal64() const) {
    return GetLiteral64();
  }

  float GetLiteralFP32() const { return RawbitsToFloat(GetLiteral32()); }
  VIXL_DEPRECATED("GetLiteralFP32", float LiteralFP32() const) {
    return GetLiteralFP32();
  }

  double GetLiteralFP64() const { return RawbitsToDouble(GetLiteral64()); }
  VIXL_DEPRECATED("GetLiteralFP64", double LiteralFP64() const) {
    return GetLiteralFP64();
  }

  Instruction* GetNextInstruction() { return this + kInstructionSize; }
  const Instruction* GetNextInstruction() const {
    return this + kInstructionSize;
  }
  VIXL_DEPRECATED("GetNextInstruction",
                  const Instruction* NextInstruction() const) {
    return GetNextInstruction();
  }

  const Instruction* GetInstructionAtOffset(int64_t offset) const {
    VIXL_ASSERT(IsWordAligned(this + offset));
    return this + offset;
  }
  VIXL_DEPRECATED("GetInstructionAtOffset",
                  const Instruction* InstructionAtOffset(int64_t offset)
                      const) {
    return GetInstructionAtOffset(offset);
  }

  template <typename T>
  static Instruction* Cast(T src) {
    return reinterpret_cast<Instruction*>(src);
  }

  template <typename T>
  static const Instruction* CastConst(T src) {
    return reinterpret_cast<const Instruction*>(src);
  }

 private:
  int GetImmBranch() const;

  void SetPCRelImmTarget(const Instruction* target);
  void SetBranchImmTarget(const Instruction* target);
};


// Functions for handling NEON and SVE vector format information.

const int kMaxLanesPerVector = 16;

VectorFormat VectorFormatHalfWidth(VectorFormat vform);
VectorFormat VectorFormatDoubleWidth(VectorFormat vform);
VectorFormat VectorFormatDoubleLanes(VectorFormat vform);
VectorFormat VectorFormatHalfLanes(VectorFormat vform);
VectorFormat ScalarFormatFromLaneSize(int lane_size_in_bits);
VectorFormat VectorFormatHalfWidthDoubleLanes(VectorFormat vform);
VectorFormat VectorFormatFillQ(VectorFormat vform);
VectorFormat ScalarFormatFromFormat(VectorFormat vform);
VectorFormat SVEFormatFromLaneSizeInBits(int lane_size_in_bits);
VectorFormat SVEFormatFromLaneSizeInBytes(int lane_size_in_bytes);
VectorFormat SVEFormatFromLaneSizeInBytesLog2(int lane_size_in_bytes_log_2);
unsigned RegisterSizeInBitsFromFormat(VectorFormat vform);
unsigned RegisterSizeInBytesFromFormat(VectorFormat vform);
bool IsSVEFormat(VectorFormat vform);
// TODO: Make the return types of these functions consistent.
unsigned LaneSizeInBitsFromFormat(VectorFormat vform);
int LaneSizeInBytesFromFormat(VectorFormat vform);
int LaneSizeInBytesLog2FromFormat(VectorFormat vform);
int LaneCountFromFormat(VectorFormat vform);
int MaxLaneCountFromFormat(VectorFormat vform);
bool IsVectorFormat(VectorFormat vform);
int64_t MaxIntFromFormat(VectorFormat vform);
int64_t MinIntFromFormat(VectorFormat vform);
uint64_t MaxUintFromFormat(VectorFormat vform);


// clang-format off
enum NEONFormat {
  NF_UNDEF = 0,
  NF_8B    = 1,
  NF_16B   = 2,
  NF_4H    = 3,
  NF_8H    = 4,
  NF_2S    = 5,
  NF_4S    = 6,
  NF_1D    = 7,
  NF_2D    = 8,
  NF_B     = 9,
  NF_H     = 10,
  NF_S     = 11,
  NF_D     = 12
};
// clang-format on

static const unsigned kNEONFormatMaxBits = 6;

struct NEONFormatMap {
  // The bit positions in the instruction to consider.
  uint8_t bits[kNEONFormatMaxBits];

  // Mapping from concatenated bits to format.
  NEONFormat map[1 << kNEONFormatMaxBits];
};

class NEONFormatDecoder {
 public:
  enum SubstitutionMode { kPlaceholder, kFormat };

  // Construct a format decoder with increasingly specific format maps for each
  // substitution. If no format map is specified, the default is the integer
  // format map.
  explicit NEONFormatDecoder(const Instruction* instr) {
    instrbits_ = instr->GetInstructionBits();
    SetFormatMaps(IntegerFormatMap());
  }
  NEONFormatDecoder(const Instruction* instr, const NEONFormatMap* format) {
    instrbits_ = instr->GetInstructionBits();
    SetFormatMaps(format);
  }
  NEONFormatDecoder(const Instruction* instr,
                    const NEONFormatMap* format0,
                    const NEONFormatMap* format1) {
    instrbits_ = instr->GetInstructionBits();
    SetFormatMaps(format0, format1);
  }
  NEONFormatDecoder(const Instruction* instr,
                    const NEONFormatMap* format0,
                    const NEONFormatMap* format1,
                    const NEONFormatMap* format2) {
    instrbits_ = instr->GetInstructionBits();
    SetFormatMaps(format0, format1, format2);
  }

  // Set the format mapping for all or individual substitutions.
  void SetFormatMaps(const NEONFormatMap* format0,
                     const NEONFormatMap* format1 = NULL,
                     const NEONFormatMap* format2 = NULL,
                     const NEONFormatMap* format3 = NULL) {
    VIXL_ASSERT(format0 != NULL);
    formats_[0] = format0;
    formats_[1] = (format1 == NULL) ? formats_[0] : format1;
    formats_[2] = (format2 == NULL) ? formats_[1] : format2;
    formats_[3] = (format3 == NULL) ? formats_[2] : format3;
  }
  void SetFormatMap(unsigned index, const NEONFormatMap* format) {
    VIXL_ASSERT(index <= ArrayLength(formats_));
    VIXL_ASSERT(format != NULL);
    formats_[index] = format;
  }

  // Substitute %s in the input string with the placeholder string for each
  // register, ie. "'B", "'H", etc.
  const char* SubstitutePlaceholders(const char* string) {
    return Substitute(string, kPlaceholder, kPlaceholder, kPlaceholder);
  }

  // Substitute %s in the input string with a new string based on the
  // substitution mode.
  const char* Substitute(const char* string,
                         SubstitutionMode mode0 = kFormat,
                         SubstitutionMode mode1 = kFormat,
                         SubstitutionMode mode2 = kFormat,
                         SubstitutionMode mode3 = kFormat) {
    const char* subst0 = GetSubstitute(0, mode0);
    const char* subst1 = GetSubstitute(1, mode1);
    const char* subst2 = GetSubstitute(2, mode2);
    const char* subst3 = GetSubstitute(3, mode3);

    if ((subst0 == NULL) || (subst1 == NULL) || (subst2 == NULL) ||
        (subst3 == NULL)) {
      return NULL;
    }

    snprintf(form_buffer_,
             sizeof(form_buffer_),
             string,
             subst0,
             subst1,
             subst2,
             subst3);
    return form_buffer_;
  }

  // Append a "2" to a mnemonic string based on the state of the Q bit.
  const char* Mnemonic(const char* mnemonic) {
    if ((mnemonic != NULL) && (instrbits_ & NEON_Q) != 0) {
      snprintf(mne_buffer_, sizeof(mne_buffer_), "%s2", mnemonic);
      return mne_buffer_;
    }
    return mnemonic;
  }

  VectorFormat GetVectorFormat(int format_index = 0) {
    return GetVectorFormat(formats_[format_index]);
  }

  VectorFormat GetVectorFormat(const NEONFormatMap* format_map) {
    static const VectorFormat vform[] = {kFormatUndefined,
                                         kFormat8B,
                                         kFormat16B,
                                         kFormat4H,
                                         kFormat8H,
                                         kFormat2S,
                                         kFormat4S,
                                         kFormat1D,
                                         kFormat2D,
                                         kFormatB,
                                         kFormatH,
                                         kFormatS,
                                         kFormatD};
    VIXL_ASSERT(GetNEONFormat(format_map) < ArrayLength(vform));
    return vform[GetNEONFormat(format_map)];
  }

  // Built in mappings for common cases.

  // The integer format map uses three bits (Q, size<1:0>) to encode the
  // "standard" set of NEON integer vector formats.
  static const NEONFormatMap* IntegerFormatMap() {
    static const NEONFormatMap map =
        {{23, 22, 30},
         {NF_8B, NF_16B, NF_4H, NF_8H, NF_2S, NF_4S, NF_UNDEF, NF_2D}};
    return &map;
  }

  // The long integer format map uses two bits (size<1:0>) to encode the
  // long set of NEON integer vector formats. These are used in narrow, wide
  // and long operations.
  static const NEONFormatMap* LongIntegerFormatMap() {
    static const NEONFormatMap map = {{23, 22}, {NF_8H, NF_4S, NF_2D}};
    return &map;
  }

  // The FP format map uses two bits (Q, size<0>) to encode the NEON FP vector
  // formats: NF_2S, NF_4S, NF_2D.
  static const NEONFormatMap* FPFormatMap() {
    // The FP format map assumes two bits (Q, size<0>) are used to encode the
    // NEON FP vector formats: NF_2S, NF_4S, NF_2D.
    static const NEONFormatMap map = {{22, 30},
                                      {NF_2S, NF_4S, NF_UNDEF, NF_2D}};
    return &map;
  }

  // The FP16 format map uses one bit (Q) to encode the NEON vector format:
  // NF_4H, NF_8H.
  static const NEONFormatMap* FP16FormatMap() {
    static const NEONFormatMap map = {{30}, {NF_4H, NF_8H}};
    return &map;
  }

  // The load/store format map uses three bits (Q, 11, 10) to encode the
  // set of NEON vector formats.
  static const NEONFormatMap* LoadStoreFormatMap() {
    static const NEONFormatMap map =
        {{11, 10, 30},
         {NF_8B, NF_16B, NF_4H, NF_8H, NF_2S, NF_4S, NF_1D, NF_2D}};
    return &map;
  }

  // The logical format map uses one bit (Q) to encode the NEON vector format:
  // NF_8B, NF_16B.
  static const NEONFormatMap* LogicalFormatMap() {
    static const NEONFormatMap map = {{30}, {NF_8B, NF_16B}};
    return &map;
  }

  // The triangular format map uses between two and five bits to encode the NEON
  // vector format:
  // xxx10->8B, xxx11->16B, xx100->4H, xx101->8H
  // x1000->2S, x1001->4S,  10001->2D, all others undefined.
  static const NEONFormatMap* TriangularFormatMap() {
    static const NEONFormatMap map =
        {{19, 18, 17, 16, 30},
         {NF_UNDEF, NF_UNDEF, NF_8B, NF_16B, NF_4H, NF_8H, NF_8B, NF_16B,
          NF_2S,    NF_4S,    NF_8B, NF_16B, NF_4H, NF_8H, NF_8B, NF_16B,
          NF_UNDEF, NF_2D,    NF_8B, NF_16B, NF_4H, NF_8H, NF_8B, NF_16B,
          NF_2S,    NF_4S,    NF_8B, NF_16B, NF_4H, NF_8H, NF_8B, NF_16B}};
    return &map;
  }

  // The shift immediate map uses between two and five bits to encode the NEON
  // vector format:
  // 00010->8B, 00011->16B, 001x0->4H, 001x1->8H,
  // 01xx0->2S, 01xx1->4S, 1xxx1->2D, all others undefined.
  static const NEONFormatMap* ShiftImmFormatMap() {
    static const NEONFormatMap map = {{22, 21, 20, 19, 30},
                                      {NF_UNDEF, NF_UNDEF, NF_8B,    NF_16B,
                                       NF_4H,    NF_8H,    NF_4H,    NF_8H,
                                       NF_2S,    NF_4S,    NF_2S,    NF_4S,
                                       NF_2S,    NF_4S,    NF_2S,    NF_4S,
                                       NF_UNDEF, NF_2D,    NF_UNDEF, NF_2D,
                                       NF_UNDEF, NF_2D,    NF_UNDEF, NF_2D,
                                       NF_UNDEF, NF_2D,    NF_UNDEF, NF_2D,
                                       NF_UNDEF, NF_2D,    NF_UNDEF, NF_2D}};
    return &map;
  }

  // The shift long/narrow immediate map uses between two and four bits to
  // encode the NEON vector format:
  // 0001->8H, 001x->4S, 01xx->2D, all others undefined.
  static const NEONFormatMap* ShiftLongNarrowImmFormatMap() {
    static const NEONFormatMap map =
        {{22, 21, 20, 19},
         {NF_UNDEF, NF_8H, NF_4S, NF_4S, NF_2D, NF_2D, NF_2D, NF_2D}};
    return &map;
  }

  // The scalar format map uses two bits (size<1:0>) to encode the NEON scalar
  // formats: NF_B, NF_H, NF_S, NF_D.
  static const NEONFormatMap* ScalarFormatMap() {
    static const NEONFormatMap map = {{23, 22}, {NF_B, NF_H, NF_S, NF_D}};
    return &map;
  }

  // The long scalar format map uses two bits (size<1:0>) to encode the longer
  // NEON scalar formats: NF_H, NF_S, NF_D.
  static const NEONFormatMap* LongScalarFormatMap() {
    static const NEONFormatMap map = {{23, 22}, {NF_H, NF_S, NF_D}};
    return &map;
  }

  // The FP scalar format map assumes one bit (size<0>) is used to encode the
  // NEON FP scalar formats: NF_S, NF_D.
  static const NEONFormatMap* FPScalarFormatMap() {
    static const NEONFormatMap map = {{22}, {NF_S, NF_D}};
    return &map;
  }

  // The FP scalar pairwise format map assumes two bits (U, size<0>) are used to
  // encode the NEON FP scalar formats: NF_H, NF_S, NF_D.
  static const NEONFormatMap* FPScalarPairwiseFormatMap() {
    static const NEONFormatMap map = {{29, 22}, {NF_H, NF_UNDEF, NF_S, NF_D}};
    return &map;
  }

  // The triangular scalar format map uses between one and four bits to encode
  // the NEON FP scalar formats:
  // xxx1->B, xx10->H, x100->S, 1000->D, all others undefined.
  static const NEONFormatMap* TriangularScalarFormatMap() {
    static const NEONFormatMap map = {{19, 18, 17, 16},
                                      {NF_UNDEF,
                                       NF_B,
                                       NF_H,
                                       NF_B,
                                       NF_S,
                                       NF_B,
                                       NF_H,
                                       NF_B,
                                       NF_D,
                                       NF_B,
                                       NF_H,
                                       NF_B,
                                       NF_S,
                                       NF_B,
                                       NF_H,
                                       NF_B}};
    return &map;
  }

 private:
  // Get a pointer to a string that represents the format or placeholder for
  // the specified substitution index, based on the format map and instruction.
  const char* GetSubstitute(int index, SubstitutionMode mode) {
    if (mode == kFormat) {
      return NEONFormatAsString(GetNEONFormat(formats_[index]));
    }
    VIXL_ASSERT(mode == kPlaceholder);
    return NEONFormatAsPlaceholder(GetNEONFormat(formats_[index]));
  }

  // Get the NEONFormat enumerated value for bits obtained from the
  // instruction based on the specified format mapping.
  NEONFormat GetNEONFormat(const NEONFormatMap* format_map) {
    return format_map->map[PickBits(format_map->bits)];
  }

  // Convert a NEONFormat into a string.
  static const char* NEONFormatAsString(NEONFormat format) {
    // clang-format off
    static const char* formats[] = {
      NULL,
      "8b", "16b", "4h", "8h", "2s", "4s", "1d", "2d",
      "b", "h", "s", "d"
    };
    // clang-format on
    VIXL_ASSERT(format < ArrayLength(formats));
    return formats[format];
  }

  // Convert a NEONFormat into a register placeholder string.
  static const char* NEONFormatAsPlaceholder(NEONFormat format) {
    VIXL_ASSERT((format == NF_B) || (format == NF_H) || (format == NF_S) ||
                (format == NF_D) || (format == NF_UNDEF));
    // clang-format off
    static const char* formats[] = {
      NULL,
      NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL,
      "'B", "'H", "'S", "'D"
    };
    // clang-format on
    return formats[format];
  }

  // Select bits from instrbits_ defined by the bits array, concatenate them,
  // and return the value.
  uint8_t PickBits(const uint8_t bits[]) {
    uint8_t result = 0;
    for (unsigned b = 0; b < kNEONFormatMaxBits; b++) {
      if (bits[b] == 0) break;
      result <<= 1;
      result |= ((instrbits_ & (1 << bits[b])) == 0) ? 0 : 1;
    }
    return result;
  }

  Instr instrbits_;
  const NEONFormatMap* formats_[4];
  char form_buffer_[64];
  char mne_buffer_[16];
};
}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_AARCH64_INSTRUCTIONS_AARCH64_H_
