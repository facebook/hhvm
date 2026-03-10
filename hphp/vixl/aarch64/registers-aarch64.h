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

#ifndef VIXL_AARCH64_REGISTERS_AARCH64_H_
#define VIXL_AARCH64_REGISTERS_AARCH64_H_

#include <string>

#include "instructions-aarch64.h"

namespace vixl {
namespace aarch64 {

// An integer type capable of representing a homogeneous, non-overlapping set of
// registers as a bitmask of their codes.
typedef uint64_t RegList;
static const int kRegListSizeInBits = sizeof(RegList) * 8;

class Register;
class WRegister;
class XRegister;

class VRegister;
class BRegister;
class HRegister;
class SRegister;
class DRegister;
class QRegister;

class ZRegister;

class PRegister;
class PRegisterWithLaneSize;
class PRegisterM;
class PRegisterZ;

// A container for any single register supported by the processor. Selected
// qualifications are also supported. Basic registers can be constructed
// directly as CPURegister objects. Other variants should be constructed as one
// of the derived classes.
//
// CPURegister aims to support any getter that would also be available to more
// specialised register types. However, using the equivalent functions on the
// specialised register types can avoid run-time checks, and should therefore be
// preferred where run-time polymorphism isn't required.
//
// Type-specific modifiers are typically implemented only on the derived
// classes.
//
// The encoding is such that CPURegister objects are cheap to pass by value.
class CPURegister {
 public:
  enum RegisterBank : uint8_t {
    kNoRegisterBank = 0,
    kRRegisterBank,
    kVRegisterBank,
    kPRegisterBank
  };
  enum RegisterType {
    kNoRegister,
    kRegister,
    kVRegister,
    kZRegister,
    kPRegister
  };

  static const unsigned kUnknownSize = 0;

  VIXL_CONSTEXPR CPURegister()
      : code_(0),
        bank_(kNoRegisterBank),
        size_(kEncodedUnknownSize),
        qualifiers_(kNoQualifiers),
        lane_size_(kEncodedUnknownSize) {}

  CPURegister(int code, int size_in_bits, RegisterType type)
      : code_(code),
        bank_(GetBankFor(type)),
        size_(EncodeSizeInBits(size_in_bits)),
        qualifiers_(kNoQualifiers),
        lane_size_(EncodeSizeInBits(size_in_bits)) {
    VIXL_ASSERT(IsValid());
  }

  // Basic accessors.

  // TODO: Make this return 'int'.
  unsigned GetCode() const { return code_; }

  RegisterBank GetBank() const { return bank_; }

  // For scalar registers, the lane size matches the register size, and is
  // always known.
  bool HasSize() const { return size_ != kEncodedUnknownSize; }
  bool HasLaneSize() const { return lane_size_ != kEncodedUnknownSize; }

  RegList GetBit() const {
    if (IsNone()) return 0;
    VIXL_ASSERT(code_ < kRegListSizeInBits);
    return static_cast<RegList>(1) << code_;
  }

  // Return the architectural name for this register.
  // TODO: This is temporary. Ultimately, we should move the
  // Simulator::*RegNameForCode helpers out of the simulator, and provide an
  // independent way to obtain the name of a register.
  std::string GetArchitecturalName() const;

  // Return the highest valid register code for this type, to allow generic
  // loops to be written. This excludes kSPRegInternalCode, since it is not
  // contiguous, and sp usually requires special handling anyway.
  unsigned GetMaxCode() const { return GetMaxCodeFor(GetBank()); }

  // Registers without a known size report kUnknownSize.
  int GetSizeInBits() const { return DecodeSizeInBits(size_); }
  int GetSizeInBytes() const { return DecodeSizeInBytes(size_); }
  // TODO: Make these return 'int'.
  unsigned GetLaneSizeInBits() const { return DecodeSizeInBits(lane_size_); }
  unsigned GetLaneSizeInBytes() const { return DecodeSizeInBytes(lane_size_); }
  unsigned GetLaneSizeInBytesLog2() const {
    VIXL_ASSERT(HasLaneSize());
    return DecodeSizeInBytesLog2(lane_size_);
  }

  int GetLanes() const {
    if (HasSize() && HasLaneSize()) {
      // Take advantage of the size encoding to calculate this efficiently.
      VIXL_STATIC_ASSERT(kEncodedHRegSize == (kEncodedBRegSize + 1));
      VIXL_STATIC_ASSERT(kEncodedSRegSize == (kEncodedHRegSize + 1));
      VIXL_STATIC_ASSERT(kEncodedDRegSize == (kEncodedSRegSize + 1));
      VIXL_STATIC_ASSERT(kEncodedQRegSize == (kEncodedDRegSize + 1));
      int log2_delta = static_cast<int>(size_) - static_cast<int>(lane_size_);
      VIXL_ASSERT(log2_delta >= 0);
      return 1 << log2_delta;
    }
    return kUnknownSize;
  }

  bool Is8Bits() const { return size_ == kEncodedBRegSize; }
  bool Is16Bits() const { return size_ == kEncodedHRegSize; }
  bool Is32Bits() const { return size_ == kEncodedSRegSize; }
  bool Is64Bits() const { return size_ == kEncodedDRegSize; }
  bool Is128Bits() const { return size_ == kEncodedQRegSize; }

  bool IsLaneSizeB() const { return lane_size_ == kEncodedBRegSize; }
  bool IsLaneSizeH() const { return lane_size_ == kEncodedHRegSize; }
  bool IsLaneSizeS() const { return lane_size_ == kEncodedSRegSize; }
  bool IsLaneSizeD() const { return lane_size_ == kEncodedDRegSize; }
  bool IsLaneSizeQ() const { return lane_size_ == kEncodedQRegSize; }

  // If Is<Foo>Register(), then it is valid to convert the CPURegister to some
  // <Foo>Register<Bar> type.
  //
  //  If...                              ... then it is safe to construct ...
  //      r.IsRegister()                       -> Register(r)
  //      r.IsVRegister()                      -> VRegister(r)
  //      r.IsZRegister()                      -> ZRegister(r)
  //      r.IsPRegister()                      -> PRegister(r)
  //
  //      r.IsPRegister() && HasLaneSize()     -> PRegisterWithLaneSize(r)
  //      r.IsPRegister() && IsMerging()       -> PRegisterM(r)
  //      r.IsPRegister() && IsZeroing()       -> PRegisterZ(r)
  bool IsRegister() const { return GetType() == kRegister; }
  bool IsVRegister() const { return GetType() == kVRegister; }
  bool IsZRegister() const { return GetType() == kZRegister; }
  bool IsPRegister() const { return GetType() == kPRegister; }

  bool IsNone() const { return GetType() == kNoRegister; }

  // `GetType() == kNoRegister` implies IsNone(), and vice-versa.
  // `GetType() == k<Foo>Register` implies Is<Foo>Register(), and vice-versa.
  RegisterType GetType() const {
    switch (bank_) {
      case kNoRegisterBank:
        return kNoRegister;
      case kRRegisterBank:
        return kRegister;
      case kVRegisterBank:
        return HasSize() ? kVRegister : kZRegister;
      case kPRegisterBank:
        return kPRegister;
    }
    VIXL_UNREACHABLE();
    return kNoRegister;
  }

  // IsFPRegister() is true for scalar FP types (and therefore implies
  // IsVRegister()). There is no corresponding FPRegister type.
  bool IsFPRegister() const { return Is1H() || Is1S() || Is1D(); }

  // TODO: These are stricter forms of the helpers above. We should make the
  // basic helpers strict, and remove these.
  bool IsValidRegister() const;
  bool IsValidVRegister() const;
  bool IsValidFPRegister() const;
  bool IsValidZRegister() const;
  bool IsValidPRegister() const;

  bool IsValid() const;
  bool IsValidOrNone() const { return IsNone() || IsValid(); }

  bool IsVector() const { return HasLaneSize() && (size_ != lane_size_); }
  bool IsScalar() const { return HasLaneSize() && (size_ == lane_size_); }

  bool IsSameType(const CPURegister& other) const {
    return GetType() == other.GetType();
  }

  bool IsSameBank(const CPURegister& other) const {
    return GetBank() == other.GetBank();
  }

  // Two registers with unknown size are considered to have the same size if
  // they also have the same type. For example, all Z registers have the same
  // size, even though we don't know what that is.
  bool IsSameSizeAndType(const CPURegister& other) const {
    return IsSameType(other) && (size_ == other.size_);
  }

  bool IsSameFormat(const CPURegister& other) const {
    return IsSameSizeAndType(other) && (lane_size_ == other.lane_size_);
  }

  // Note that NoReg aliases itself, so that 'Is' implies 'Aliases'.
  bool Aliases(const CPURegister& other) const {
    return IsSameBank(other) && (code_ == other.code_);
  }

  bool Is(const CPURegister& other) const {
    if (IsRegister() || IsVRegister()) {
      // For core (W, X) and FP/NEON registers, we only consider the code, size
      // and type. This is legacy behaviour.
      // TODO: We should probably check every field for all registers.
      return Aliases(other) && (size_ == other.size_);
    } else {
      // For Z and P registers, we require all fields to match exactly.
      VIXL_ASSERT(IsNone() || IsZRegister() || IsPRegister());
      return (code_ == other.code_) && (bank_ == other.bank_) &&
             (size_ == other.size_) && (qualifiers_ == other.qualifiers_) &&
             (lane_size_ == other.lane_size_);
    }
  }

  // Conversions to specific register types. The result is a register that
  // aliases the original CPURegister. That is, the original register bank
  // (`GetBank()`) is checked and the code (`GetCode()`) preserved, but all
  // other properties are ignored.
  //
  // Typical usage:
  //
  //     if (reg.GetBank() == kVRegisterBank) {
  //       DRegister d = reg.D();
  //       ...
  //     }
  //
  // These could all return types with compile-time guarantees (like XRegister),
  // but this breaks backwards-compatibility quite severely, particularly with
  // code like `cond ? reg.W() : reg.X()`, which would have indeterminate type.

  // Core registers, like "w0".
  Register W() const;
  Register X() const;
  // FP/NEON registers, like "b0".
  VRegister B() const;
  VRegister H() const;
  VRegister S() const;
  VRegister D() const;
  VRegister Q() const;
  VRegister V() const;
  // SVE registers, like "z0".
  ZRegister Z() const;
  PRegister P() const;

  // Utilities for kRegister types.

  bool IsZero() const { return IsRegister() && (code_ == kZeroRegCode); }
  bool IsSP() const { return IsRegister() && (code_ == kSPRegInternalCode); }
  bool IsW() const { return IsRegister() && Is32Bits(); }
  bool IsX() const { return IsRegister() && Is64Bits(); }

  // Utilities for FP/NEON kVRegister types.

  // These helpers ensure that the size and type of the register are as
  // described. They do not consider the number of lanes that make up a vector.
  // So, for example, Is8B() implies IsD(), and Is1D() implies IsD, but IsD()
  // does not imply Is1D() or Is8B().
  // Check the number of lanes, ie. the format of the vector, using methods such
  // as Is8B(), Is1D(), etc.
  bool IsB() const { return IsVRegister() && Is8Bits(); }
  bool IsH() const { return IsVRegister() && Is16Bits(); }
  bool IsS() const { return IsVRegister() && Is32Bits(); }
  bool IsD() const { return IsVRegister() && Is64Bits(); }
  bool IsQ() const { return IsVRegister() && Is128Bits(); }

  // As above, but also check that the register has exactly one lane. For
  // example, reg.Is1D() implies DRegister(reg).IsValid(), but reg.IsD() does
  // not.
  bool Is1B() const { return IsB() && IsScalar(); }
  bool Is1H() const { return IsH() && IsScalar(); }
  bool Is1S() const { return IsS() && IsScalar(); }
  bool Is1D() const { return IsD() && IsScalar(); }
  bool Is1Q() const { return IsQ() && IsScalar(); }

  // Check the specific NEON format.
  bool Is8B() const { return IsD() && IsLaneSizeB(); }
  bool Is16B() const { return IsQ() && IsLaneSizeB(); }
  bool Is2H() const { return IsS() && IsLaneSizeH(); }
  bool Is4H() const { return IsD() && IsLaneSizeH(); }
  bool Is8H() const { return IsQ() && IsLaneSizeH(); }
  bool Is2S() const { return IsD() && IsLaneSizeS(); }
  bool Is4S() const { return IsQ() && IsLaneSizeS(); }
  bool Is2D() const { return IsQ() && IsLaneSizeD(); }

  // A semantic alias for sdot and udot (indexed and by element) instructions.
  // The current CPURegister implementation cannot not tell this from Is1S(),
  // but it might do later.
  // TODO: Do this with the qualifiers_ field.
  bool Is1S4B() const { return Is1S(); }

  // Utilities for SVE registers.

  bool IsUnqualified() const { return qualifiers_ == kNoQualifiers; }
  bool IsMerging() const { return IsPRegister() && (qualifiers_ == kMerging); }
  bool IsZeroing() const { return IsPRegister() && (qualifiers_ == kZeroing); }

  // SVE types have unknown sizes, but within known bounds.

  int GetMaxSizeInBytes() const {
    switch (GetType()) {
      case kZRegister:
        return kZRegMaxSizeInBytes;
      case kPRegister:
        return kPRegMaxSizeInBytes;
      default:
        VIXL_ASSERT(HasSize());
        return GetSizeInBits();
    }
  }

  int GetMinSizeInBytes() const {
    switch (GetType()) {
      case kZRegister:
        return kZRegMinSizeInBytes;
      case kPRegister:
        return kPRegMinSizeInBytes;
      default:
        VIXL_ASSERT(HasSize());
        return GetSizeInBits();
    }
  }

  int GetMaxSizeInBits() const { return GetMaxSizeInBytes() * kBitsPerByte; }
  int GetMinSizeInBits() const { return GetMinSizeInBytes() * kBitsPerByte; }

  static RegisterBank GetBankFor(RegisterType type) {
    switch (type) {
      case kNoRegister:
        return kNoRegisterBank;
      case kRegister:
        return kRRegisterBank;
      case kVRegister:
      case kZRegister:
        return kVRegisterBank;
      case kPRegister:
        return kPRegisterBank;
    }
    VIXL_UNREACHABLE();
    return kNoRegisterBank;
  }

  static unsigned GetMaxCodeFor(CPURegister::RegisterType type) {
    return GetMaxCodeFor(GetBankFor(type));
  }

 protected:
  enum EncodedSize : uint8_t {
    // Ensure that kUnknownSize (and therefore kNoRegister) is encoded as zero.
    kEncodedUnknownSize = 0,

    // The implementation assumes that the remaining sizes are encoded as
    // `log2(size) + c`, so the following names must remain in sequence.
    kEncodedBRegSize,
    kEncodedHRegSize,
    kEncodedSRegSize,
    kEncodedDRegSize,
    kEncodedQRegSize,

    kEncodedWRegSize = kEncodedSRegSize,
    kEncodedXRegSize = kEncodedDRegSize
  };
  VIXL_STATIC_ASSERT(kSRegSize == kWRegSize);
  VIXL_STATIC_ASSERT(kDRegSize == kXRegSize);

  char GetLaneSizeSymbol() const {
    switch (lane_size_) {
      case kEncodedBRegSize:
        return 'B';
      case kEncodedHRegSize:
        return 'H';
      case kEncodedSRegSize:
        return 'S';
      case kEncodedDRegSize:
        return 'D';
      case kEncodedQRegSize:
        return 'Q';
      case kEncodedUnknownSize:
        break;
    }
    VIXL_UNREACHABLE();
    return '?';
  }

  static EncodedSize EncodeSizeInBits(int size_in_bits) {
    switch (size_in_bits) {
      case kUnknownSize:
        return kEncodedUnknownSize;
      case kBRegSize:
        return kEncodedBRegSize;
      case kHRegSize:
        return kEncodedHRegSize;
      case kSRegSize:
        return kEncodedSRegSize;
      case kDRegSize:
        return kEncodedDRegSize;
      case kQRegSize:
        return kEncodedQRegSize;
    }
    VIXL_UNREACHABLE();
    return kEncodedUnknownSize;
  }

  static int DecodeSizeInBytesLog2(EncodedSize encoded_size) {
    switch (encoded_size) {
      case kEncodedUnknownSize:
        // Log2 of B-sized lane in bytes is 0, so we can't just return 0 here.
        VIXL_UNREACHABLE();
        return -1;
      case kEncodedBRegSize:
        return kBRegSizeInBytesLog2;
      case kEncodedHRegSize:
        return kHRegSizeInBytesLog2;
      case kEncodedSRegSize:
        return kSRegSizeInBytesLog2;
      case kEncodedDRegSize:
        return kDRegSizeInBytesLog2;
      case kEncodedQRegSize:
        return kQRegSizeInBytesLog2;
    }
    VIXL_UNREACHABLE();
    return kUnknownSize;
  }

  static int DecodeSizeInBytes(EncodedSize encoded_size) {
    if (encoded_size == kEncodedUnknownSize) {
      return kUnknownSize;
    }
    return 1 << DecodeSizeInBytesLog2(encoded_size);
  }

  static int DecodeSizeInBits(EncodedSize encoded_size) {
    VIXL_STATIC_ASSERT(kUnknownSize == 0);
    return DecodeSizeInBytes(encoded_size) * kBitsPerByte;
  }

  static unsigned GetMaxCodeFor(CPURegister::RegisterBank bank);

  enum Qualifiers : uint8_t {
    kNoQualifiers = 0,
    // Used by P registers.
    kMerging,
    kZeroing
  };

  // An unchecked constructor, for use by derived classes.
  CPURegister(int code,
              EncodedSize size,
              RegisterBank bank,
              EncodedSize lane_size,
              Qualifiers qualifiers = kNoQualifiers)
      : code_(code),
        bank_(bank),
        size_(size),
        qualifiers_(qualifiers),
        lane_size_(lane_size) {}

  // TODO: Check that access to these fields is reasonably efficient.
  uint8_t code_;
  RegisterBank bank_;
  EncodedSize size_;
  Qualifiers qualifiers_;
  EncodedSize lane_size_;
};
// Ensure that CPURegisters can fit in a single (64-bit) register. This is a
// proxy for being "cheap to pass by value", which is hard to check directly.
VIXL_STATIC_ASSERT(sizeof(CPURegister) <= sizeof(uint64_t));

// TODO: Add constexpr constructors.
#define VIXL_DECLARE_REGISTER_COMMON(NAME, REGISTER_TYPE, PARENT_TYPE) \
  VIXL_CONSTEXPR NAME() : PARENT_TYPE() {}                             \
                                                                       \
  explicit NAME(CPURegister other) : PARENT_TYPE(other) {              \
    VIXL_ASSERT(IsValid());                                            \
  }                                                                    \
                                                                       \
  VIXL_CONSTEXPR static unsigned GetMaxCode() {                        \
    return kNumberOf##REGISTER_TYPE##s - 1;                            \
  }

// Any W or X register, including the zero register and the stack pointer.
class Register : public CPURegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(Register, Register, CPURegister)

  Register(int code, int size_in_bits)
      : CPURegister(code, size_in_bits, kRegister) {
    VIXL_ASSERT(IsValidRegister());
  }

  bool IsValid() const { return IsValidRegister(); }
};

// Any FP or NEON V register, including vector (V.<T>) and scalar forms
// (B, H, S, D, Q).
class VRegister : public CPURegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(VRegister, VRegister, CPURegister)

  // For historical reasons, VRegister(0) returns v0.1Q (or equivalently, q0).
  explicit VRegister(int code, int size_in_bits = kQRegSize, int lanes = 1)
      : CPURegister(code,
                    EncodeSizeInBits(size_in_bits),
                    kVRegisterBank,
                    EncodeLaneSizeInBits(size_in_bits, lanes)) {
    VIXL_ASSERT(IsValidVRegister());
  }

  VRegister(int code, VectorFormat format)
      : CPURegister(code,
                    EncodeSizeInBits(RegisterSizeInBitsFromFormat(format)),
                    kVRegisterBank,
                    EncodeSizeInBits(LaneSizeInBitsFromFormat(format)),
                    kNoQualifiers) {
    VIXL_ASSERT(IsValid());
  }

  VRegister V8B() const;
  VRegister V16B() const;
  VRegister V2H() const;
  VRegister V4H() const;
  VRegister V8H() const;
  VRegister V2S() const;
  VRegister V4S() const;
  VRegister V1D() const;
  VRegister V2D() const;
  VRegister V1Q() const;
  VRegister S4B() const;

  bool IsValid() const { return IsValidVRegister(); }

 protected:
  static EncodedSize EncodeLaneSizeInBits(int size_in_bits, int lanes) {
    VIXL_ASSERT(lanes >= 1);
    VIXL_ASSERT((size_in_bits % lanes) == 0);
    return EncodeSizeInBits(size_in_bits / lanes);
  }
};

// Any SVE Z register, with or without a lane size specifier.
class ZRegister : public CPURegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(ZRegister, ZRegister, CPURegister)

  explicit ZRegister(int code, int lane_size_in_bits = kUnknownSize)
      : CPURegister(code,
                    kEncodedUnknownSize,
                    kVRegisterBank,
                    EncodeSizeInBits(lane_size_in_bits)) {
    VIXL_ASSERT(IsValid());
  }

  ZRegister(int code, VectorFormat format)
      : CPURegister(code,
                    kEncodedUnknownSize,
                    kVRegisterBank,
                    EncodeSizeInBits(LaneSizeInBitsFromFormat(format)),
                    kNoQualifiers) {
    VIXL_ASSERT(IsValid());
  }

  // Return a Z register with a known lane size (like "z0.B").
  ZRegister VnB() const { return ZRegister(GetCode(), kBRegSize); }
  ZRegister VnH() const { return ZRegister(GetCode(), kHRegSize); }
  ZRegister VnS() const { return ZRegister(GetCode(), kSRegSize); }
  ZRegister VnD() const { return ZRegister(GetCode(), kDRegSize); }
  ZRegister VnQ() const { return ZRegister(GetCode(), kQRegSize); }

  template <typename T>
  ZRegister WithLaneSize(T format) const {
    return ZRegister(GetCode(), format);
  }

  ZRegister WithSameLaneSizeAs(const CPURegister& other) const {
    VIXL_ASSERT(other.HasLaneSize());
    return this->WithLaneSize(other.GetLaneSizeInBits());
  }

  bool IsValid() const { return IsValidZRegister(); }
};

// Any SVE P register, with or without a qualifier or lane size specifier.
class PRegister : public CPURegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(PRegister, PRegister, CPURegister)

  explicit PRegister(int code) : CPURegister(code, kUnknownSize, kPRegister) {
    VIXL_ASSERT(IsValid());
  }

  bool IsValid() const {
    return IsValidPRegister() && !HasLaneSize() && IsUnqualified();
  }

  // Return a P register with a known lane size (like "p0.B").
  PRegisterWithLaneSize VnB() const;
  PRegisterWithLaneSize VnH() const;
  PRegisterWithLaneSize VnS() const;
  PRegisterWithLaneSize VnD() const;

  template <typename T>
  PRegisterWithLaneSize WithLaneSize(T format) const;

  PRegisterWithLaneSize WithSameLaneSizeAs(const CPURegister& other) const;

  // SVE predicates are specified (in normal assembly) with a "/z" (zeroing) or
  // "/m" (merging) suffix. These methods are VIXL's equivalents.
  PRegisterZ Zeroing() const;
  PRegisterM Merging() const;

 protected:
  // Unchecked constructors, for use by derived classes.
  PRegister(int code, EncodedSize encoded_lane_size)
      : CPURegister(code,
                    kEncodedUnknownSize,
                    kPRegisterBank,
                    encoded_lane_size,
                    kNoQualifiers) {}

  PRegister(int code, Qualifiers qualifiers)
      : CPURegister(code,
                    kEncodedUnknownSize,
                    kPRegisterBank,
                    kEncodedUnknownSize,
                    qualifiers) {}
};

// Any SVE P register with a known lane size (like "p0.B").
class PRegisterWithLaneSize : public PRegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(PRegisterWithLaneSize, PRegister, PRegister)

  PRegisterWithLaneSize(int code, int lane_size_in_bits)
      : PRegister(code, EncodeSizeInBits(lane_size_in_bits)) {
    VIXL_ASSERT(IsValid());
  }

  PRegisterWithLaneSize(int code, VectorFormat format)
      : PRegister(code, EncodeSizeInBits(LaneSizeInBitsFromFormat(format))) {
    VIXL_ASSERT(IsValid());
  }

  bool IsValid() const {
    return IsValidPRegister() && HasLaneSize() && IsUnqualified();
  }

  // Overload lane size accessors so we can assert `HasLaneSize()`. This allows
  // tools such as clang-tidy to prove that the result of GetLaneSize* is
  // non-zero.

  // TODO: Make these return 'int'.
  unsigned GetLaneSizeInBits() const {
    VIXL_ASSERT(HasLaneSize());
    return PRegister::GetLaneSizeInBits();
  }

  unsigned GetLaneSizeInBytes() const {
    VIXL_ASSERT(HasLaneSize());
    return PRegister::GetLaneSizeInBytes();
  }
};

// Any SVE P register with the zeroing qualifier (like "p0/z").
class PRegisterZ : public PRegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(PRegisterZ, PRegister, PRegister)

  explicit PRegisterZ(int code) : PRegister(code, kZeroing) {
    VIXL_ASSERT(IsValid());
  }

  bool IsValid() const {
    return IsValidPRegister() && !HasLaneSize() && IsZeroing();
  }
};

// Any SVE P register with the merging qualifier (like "p0/m").
class PRegisterM : public PRegister {
 public:
  VIXL_DECLARE_REGISTER_COMMON(PRegisterM, PRegister, PRegister)

  explicit PRegisterM(int code) : PRegister(code, kMerging) {
    VIXL_ASSERT(IsValid());
  }

  bool IsValid() const {
    return IsValidPRegister() && !HasLaneSize() && IsMerging();
  }
};

inline PRegisterWithLaneSize PRegister::VnB() const {
  return PRegisterWithLaneSize(GetCode(), kBRegSize);
}
inline PRegisterWithLaneSize PRegister::VnH() const {
  return PRegisterWithLaneSize(GetCode(), kHRegSize);
}
inline PRegisterWithLaneSize PRegister::VnS() const {
  return PRegisterWithLaneSize(GetCode(), kSRegSize);
}
inline PRegisterWithLaneSize PRegister::VnD() const {
  return PRegisterWithLaneSize(GetCode(), kDRegSize);
}

template <typename T>
inline PRegisterWithLaneSize PRegister::WithLaneSize(T format) const {
  return PRegisterWithLaneSize(GetCode(), format);
}

inline PRegisterWithLaneSize PRegister::WithSameLaneSizeAs(
    const CPURegister& other) const {
  VIXL_ASSERT(other.HasLaneSize());
  return this->WithLaneSize(other.GetLaneSizeInBits());
}

inline PRegisterZ PRegister::Zeroing() const { return PRegisterZ(GetCode()); }
inline PRegisterM PRegister::Merging() const { return PRegisterM(GetCode()); }

#define VIXL_REGISTER_WITH_SIZE_LIST(V) \
  V(WRegister, kWRegSize, Register)     \
  V(XRegister, kXRegSize, Register)     \
  V(QRegister, kQRegSize, VRegister)    \
  V(DRegister, kDRegSize, VRegister)    \
  V(SRegister, kSRegSize, VRegister)    \
  V(HRegister, kHRegSize, VRegister)    \
  V(BRegister, kBRegSize, VRegister)

#define VIXL_DEFINE_REGISTER_WITH_SIZE(NAME, SIZE, PARENT)           \
  class NAME : public PARENT {                                       \
   public:                                                           \
    VIXL_CONSTEXPR NAME() : PARENT() {}                              \
    explicit NAME(int code) : PARENT(code, SIZE) {}                  \
                                                                     \
    explicit NAME(PARENT other) : PARENT(other) {                    \
      VIXL_ASSERT(GetSizeInBits() == SIZE);                          \
    }                                                                \
                                                                     \
    PARENT As##PARENT() const { return *this; }                      \
                                                                     \
    VIXL_CONSTEXPR int GetSizeInBits() const { return SIZE; }        \
                                                                     \
    bool IsValid() const {                                           \
      return PARENT::IsValid() && (PARENT::GetSizeInBits() == SIZE); \
    }                                                                \
  };

VIXL_REGISTER_WITH_SIZE_LIST(VIXL_DEFINE_REGISTER_WITH_SIZE)

// No*Reg is used to provide default values for unused arguments, error cases
// and so on. Note that these (and the default constructors) all compare equal
// (using the Is() method).
const Register NoReg;
const VRegister NoVReg;
const CPURegister NoCPUReg;
const ZRegister NoZReg;

// TODO: Ideally, these would use specialised register types (like XRegister and
// so on). However, doing so throws up template overloading problems elsewhere.
#define VIXL_DEFINE_REGISTERS(N)       \
  const Register w##N = WRegister(N);  \
  const Register x##N = XRegister(N);  \
  const VRegister b##N = BRegister(N); \
  const VRegister h##N = HRegister(N); \
  const VRegister s##N = SRegister(N); \
  const VRegister d##N = DRegister(N); \
  const VRegister q##N = QRegister(N); \
  const VRegister v##N(N);             \
  const ZRegister z##N(N);
AARCH64_REGISTER_CODE_LIST(VIXL_DEFINE_REGISTERS)
#undef VIXL_DEFINE_REGISTERS

#define VIXL_DEFINE_P_REGISTERS(N) const PRegister p##N(N);
AARCH64_P_REGISTER_CODE_LIST(VIXL_DEFINE_P_REGISTERS)
#undef VIXL_DEFINE_P_REGISTERS

// VIXL represents 'sp' with a unique code, to tell it apart from 'xzr'.
const Register wsp = WRegister(kSPRegInternalCode);
const Register sp = XRegister(kSPRegInternalCode);

// Standard aliases.
const Register ip0 = x16;
const Register ip1 = x17;
const Register lr = x30;
const Register xzr = x31;
const Register wzr = w31;

// AreAliased returns true if any of the named registers overlap. Arguments
// set to NoReg are ignored. The system stack pointer may be specified.
bool AreAliased(const CPURegister& reg1,
                const CPURegister& reg2,
                const CPURegister& reg3 = NoReg,
                const CPURegister& reg4 = NoReg,
                const CPURegister& reg5 = NoReg,
                const CPURegister& reg6 = NoReg,
                const CPURegister& reg7 = NoReg,
                const CPURegister& reg8 = NoReg);

// AreSameSizeAndType returns true if all of the specified registers have the
// same size, and are of the same type. The system stack pointer may be
// specified. Arguments set to NoReg are ignored, as are any subsequent
// arguments. At least one argument (reg1) must be valid (not NoCPUReg).
bool AreSameSizeAndType(const CPURegister& reg1,
                        const CPURegister& reg2,
                        const CPURegister& reg3 = NoCPUReg,
                        const CPURegister& reg4 = NoCPUReg,
                        const CPURegister& reg5 = NoCPUReg,
                        const CPURegister& reg6 = NoCPUReg,
                        const CPURegister& reg7 = NoCPUReg,
                        const CPURegister& reg8 = NoCPUReg);

// AreEven returns true if all of the specified registers have even register
// indices. Arguments set to NoReg are ignored, as are any subsequent
// arguments. At least one argument (reg1) must be valid (not NoCPUReg).
bool AreEven(const CPURegister& reg1,
             const CPURegister& reg2,
             const CPURegister& reg3 = NoReg,
             const CPURegister& reg4 = NoReg,
             const CPURegister& reg5 = NoReg,
             const CPURegister& reg6 = NoReg,
             const CPURegister& reg7 = NoReg,
             const CPURegister& reg8 = NoReg);

// AreConsecutive returns true if all of the specified registers are
// consecutive in the register file. Arguments set to NoReg are ignored, as are
// any subsequent arguments. At least one argument (reg1) must be valid
// (not NoCPUReg).
bool AreConsecutive(const CPURegister& reg1,
                    const CPURegister& reg2,
                    const CPURegister& reg3 = NoCPUReg,
                    const CPURegister& reg4 = NoCPUReg);

// AreSameFormat returns true if all of the specified registers have the same
// vector format. Arguments set to NoReg are ignored, as are any subsequent
// arguments. At least one argument (reg1) must be valid (not NoVReg).
bool AreSameFormat(const CPURegister& reg1,
                   const CPURegister& reg2,
                   const CPURegister& reg3 = NoCPUReg,
                   const CPURegister& reg4 = NoCPUReg);

// AreSameLaneSize returns true if all of the specified registers have the same
// element lane size, B, H, S or D. It doesn't compare the type of registers.
// Arguments set to NoReg are ignored, as are any subsequent arguments.
// At least one argument (reg1) must be valid (not NoVReg).
// TODO: Remove this, and replace its uses with AreSameFormat.
bool AreSameLaneSize(const CPURegister& reg1,
                     const CPURegister& reg2,
                     const CPURegister& reg3 = NoCPUReg,
                     const CPURegister& reg4 = NoCPUReg);
}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_AARCH64_REGISTERS_AARCH64_H_
