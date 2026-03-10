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

#include "registers-aarch64.h"

#include <sstream>
#include <string>

namespace vixl {
namespace aarch64 {

std::string CPURegister::GetArchitecturalName() const {
  std::ostringstream name;
  if (IsZRegister()) {
    name << 'z' << GetCode();
    if (HasLaneSize()) {
      name << '.' << GetLaneSizeSymbol();
    }
  } else if (IsPRegister()) {
    name << 'p' << GetCode();
    if (HasLaneSize()) {
      name << '.' << GetLaneSizeSymbol();
    }
    switch (qualifiers_) {
      case kNoQualifiers:
        break;
      case kMerging:
        name << "/m";
        break;
      case kZeroing:
        name << "/z";
        break;
    }
  } else {
    VIXL_UNIMPLEMENTED();
  }
  return name.str();
}

unsigned CPURegister::GetMaxCodeFor(CPURegister::RegisterBank bank) {
  switch (bank) {
    case kNoRegisterBank:
      return 0;
    case kRRegisterBank:
      return Register::GetMaxCode();
    case kVRegisterBank:
#ifdef VIXL_HAS_CONSTEXPR
      VIXL_STATIC_ASSERT(VRegister::GetMaxCode() == ZRegister::GetMaxCode());
#else
      VIXL_ASSERT(VRegister::GetMaxCode() == ZRegister::GetMaxCode());
#endif
      return VRegister::GetMaxCode();
    case kPRegisterBank:
      return PRegister::GetMaxCode();
  }
  VIXL_UNREACHABLE();
  return 0;
}

bool CPURegister::IsValidRegister() const {
  return ((code_ < kNumberOfRegisters) || (code_ == kSPRegInternalCode)) &&
         (bank_ == kRRegisterBank) &&
         ((size_ == kEncodedWRegSize) || (size_ == kEncodedXRegSize)) &&
         (qualifiers_ == kNoQualifiers) && (lane_size_ == size_);
}

bool CPURegister::IsValidVRegister() const {
  VIXL_STATIC_ASSERT(kEncodedBRegSize < kEncodedQRegSize);
  return (code_ < kNumberOfVRegisters) && (bank_ == kVRegisterBank) &&
         ((size_ >= kEncodedBRegSize) && (size_ <= kEncodedQRegSize)) &&
         (qualifiers_ == kNoQualifiers) &&
         (lane_size_ != kEncodedUnknownSize) && (lane_size_ <= size_);
}

bool CPURegister::IsValidFPRegister() const {
  return IsValidVRegister() && IsFPRegister();
}

bool CPURegister::IsValidZRegister() const {
  VIXL_STATIC_ASSERT(kEncodedBRegSize < kEncodedQRegSize);
  // Z registers are valid with or without a lane size, so we don't need to
  // check lane_size_.
  return (code_ < kNumberOfZRegisters) && (bank_ == kVRegisterBank) &&
         (size_ == kEncodedUnknownSize) && (qualifiers_ == kNoQualifiers);
}

bool CPURegister::IsValidPRegister() const {
  VIXL_STATIC_ASSERT(kEncodedBRegSize < kEncodedQRegSize);
  // P registers are valid with or without a lane size, so we don't need to
  // check lane_size_.
  return (code_ < kNumberOfPRegisters) && (bank_ == kPRegisterBank) &&
         (size_ == kEncodedUnknownSize) &&
         ((qualifiers_ == kNoQualifiers) || (qualifiers_ == kMerging) ||
          (qualifiers_ == kZeroing));
}

bool CPURegister::IsValid() const {
  return IsValidRegister() || IsValidVRegister() || IsValidZRegister() ||
         IsValidPRegister();
}

// Most coercions simply invoke the necessary constructor.
#define VIXL_CPUREG_COERCION_LIST(U) \
  U(Register, W, R)                  \
  U(Register, X, R)                  \
  U(VRegister, B, V)                 \
  U(VRegister, H, V)                 \
  U(VRegister, S, V)                 \
  U(VRegister, D, V)                 \
  U(VRegister, Q, V)                 \
  U(VRegister, V, V)                 \
  U(ZRegister, Z, V)                 \
  U(PRegister, P, P)
#define VIXL_DEFINE_CPUREG_COERCION(RET_TYPE, CTOR_TYPE, BANK) \
  RET_TYPE CPURegister::CTOR_TYPE() const {                    \
    VIXL_ASSERT(GetBank() == k##BANK##RegisterBank);           \
    return CTOR_TYPE##Register(GetCode());                     \
  }
VIXL_CPUREG_COERCION_LIST(VIXL_DEFINE_CPUREG_COERCION)
#undef VIXL_CPUREG_COERCION_LIST
#undef VIXL_DEFINE_CPUREG_COERCION

// NEON lane-format coercions always return VRegisters.
#define VIXL_CPUREG_NEON_COERCION_LIST(V) \
  V(8, B)                                 \
  V(16, B)                                \
  V(2, H)                                 \
  V(4, H)                                 \
  V(8, H)                                 \
  V(2, S)                                 \
  V(4, S)                                 \
  V(1, D)                                 \
  V(2, D)                                 \
  V(1, Q)
#define VIXL_DEFINE_CPUREG_NEON_COERCION(LANES, LANE_TYPE)             \
  VRegister VRegister::V##LANES##LANE_TYPE() const {                   \
    VIXL_ASSERT(IsVRegister());                                        \
    return VRegister(GetCode(), LANES * k##LANE_TYPE##RegSize, LANES); \
  }
VIXL_CPUREG_NEON_COERCION_LIST(VIXL_DEFINE_CPUREG_NEON_COERCION)
#undef VIXL_CPUREG_NEON_COERCION_LIST
#undef VIXL_DEFINE_CPUREG_NEON_COERCION

// Semantic type coercion for sdot and udot.
// TODO: Use the qualifiers_ field to distinguish this from ::S().
VRegister VRegister::S4B() const {
  VIXL_ASSERT(IsVRegister());
  return SRegister(GetCode());
}

bool AreAliased(const CPURegister& reg1,
                const CPURegister& reg2,
                const CPURegister& reg3,
                const CPURegister& reg4,
                const CPURegister& reg5,
                const CPURegister& reg6,
                const CPURegister& reg7,
                const CPURegister& reg8) {
  int number_of_valid_regs = 0;
  int number_of_valid_vregs = 0;
  int number_of_valid_pregs = 0;

  RegList unique_regs = 0;
  RegList unique_vregs = 0;
  RegList unique_pregs = 0;

  const CPURegister regs[] = {reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8};

  for (size_t i = 0; i < ArrayLength(regs); i++) {
    switch (regs[i].GetBank()) {
      case CPURegister::kRRegisterBank:
        number_of_valid_regs++;
        unique_regs |= regs[i].GetBit();
        break;
      case CPURegister::kVRegisterBank:
        number_of_valid_vregs++;
        unique_vregs |= regs[i].GetBit();
        break;
      case CPURegister::kPRegisterBank:
        number_of_valid_pregs++;
        unique_pregs |= regs[i].GetBit();
        break;
      case CPURegister::kNoRegisterBank:
        VIXL_ASSERT(regs[i].IsNone());
        break;
    }
  }

  int number_of_unique_regs = CountSetBits(unique_regs);
  int number_of_unique_vregs = CountSetBits(unique_vregs);
  int number_of_unique_pregs = CountSetBits(unique_pregs);

  VIXL_ASSERT(number_of_valid_regs >= number_of_unique_regs);
  VIXL_ASSERT(number_of_valid_vregs >= number_of_unique_vregs);
  VIXL_ASSERT(number_of_valid_pregs >= number_of_unique_pregs);

  return (number_of_valid_regs != number_of_unique_regs) ||
         (number_of_valid_vregs != number_of_unique_vregs) ||
         (number_of_valid_pregs != number_of_unique_pregs);
}

bool AreSameSizeAndType(const CPURegister& reg1,
                        const CPURegister& reg2,
                        const CPURegister& reg3,
                        const CPURegister& reg4,
                        const CPURegister& reg5,
                        const CPURegister& reg6,
                        const CPURegister& reg7,
                        const CPURegister& reg8) {
  VIXL_ASSERT(reg1.IsValid());
  bool match = true;
  match &= !reg2.IsValid() || reg2.IsSameSizeAndType(reg1);
  match &= !reg3.IsValid() || reg3.IsSameSizeAndType(reg1);
  match &= !reg4.IsValid() || reg4.IsSameSizeAndType(reg1);
  match &= !reg5.IsValid() || reg5.IsSameSizeAndType(reg1);
  match &= !reg6.IsValid() || reg6.IsSameSizeAndType(reg1);
  match &= !reg7.IsValid() || reg7.IsSameSizeAndType(reg1);
  match &= !reg8.IsValid() || reg8.IsSameSizeAndType(reg1);
  return match;
}

bool AreEven(const CPURegister& reg1,
             const CPURegister& reg2,
             const CPURegister& reg3,
             const CPURegister& reg4,
             const CPURegister& reg5,
             const CPURegister& reg6,
             const CPURegister& reg7,
             const CPURegister& reg8) {
  VIXL_ASSERT(reg1.IsValid());
  bool even = (reg1.GetCode() % 2) == 0;
  even &= !reg2.IsValid() || ((reg2.GetCode() % 2) == 0);
  even &= !reg3.IsValid() || ((reg3.GetCode() % 2) == 0);
  even &= !reg4.IsValid() || ((reg4.GetCode() % 2) == 0);
  even &= !reg5.IsValid() || ((reg5.GetCode() % 2) == 0);
  even &= !reg6.IsValid() || ((reg6.GetCode() % 2) == 0);
  even &= !reg7.IsValid() || ((reg7.GetCode() % 2) == 0);
  even &= !reg8.IsValid() || ((reg8.GetCode() % 2) == 0);
  return even;
}

bool AreConsecutive(const CPURegister& reg1,
                    const CPURegister& reg2,
                    const CPURegister& reg3,
                    const CPURegister& reg4) {
  VIXL_ASSERT(reg1.IsValid());

  if (!reg2.IsValid()) {
    return true;
  } else if (reg2.GetCode() !=
             ((reg1.GetCode() + 1) % (reg1.GetMaxCode() + 1))) {
    return false;
  }

  if (!reg3.IsValid()) {
    return true;
  } else if (reg3.GetCode() !=
             ((reg2.GetCode() + 1) % (reg1.GetMaxCode() + 1))) {
    return false;
  }

  if (!reg4.IsValid()) {
    return true;
  } else if (reg4.GetCode() !=
             ((reg3.GetCode() + 1) % (reg1.GetMaxCode() + 1))) {
    return false;
  }

  return true;
}

bool AreSameFormat(const CPURegister& reg1,
                   const CPURegister& reg2,
                   const CPURegister& reg3,
                   const CPURegister& reg4) {
  VIXL_ASSERT(reg1.IsValid());
  bool match = true;
  match &= !reg2.IsValid() || reg2.IsSameFormat(reg1);
  match &= !reg3.IsValid() || reg3.IsSameFormat(reg1);
  match &= !reg4.IsValid() || reg4.IsSameFormat(reg1);
  return match;
}

bool AreSameLaneSize(const CPURegister& reg1,
                     const CPURegister& reg2,
                     const CPURegister& reg3,
                     const CPURegister& reg4) {
  VIXL_ASSERT(reg1.IsValid());
  bool match = true;
  match &=
      !reg2.IsValid() || (reg2.GetLaneSizeInBits() == reg1.GetLaneSizeInBits());
  match &=
      !reg3.IsValid() || (reg3.GetLaneSizeInBits() == reg1.GetLaneSizeInBits());
  match &=
      !reg4.IsValid() || (reg4.GetLaneSizeInBits() == reg1.GetLaneSizeInBits());
  return match;
}
}  // namespace aarch64
}  // namespace vixl
