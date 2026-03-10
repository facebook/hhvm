// Copyright 2016, VIXL authors
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

#include "operands-aarch64.h"

namespace vixl {
namespace aarch64 {

// CPURegList utilities.
CPURegister CPURegList::PopLowestIndex(RegList mask) {
  RegList list = list_ & mask;
  if (list == 0) return NoCPUReg;
  int index = CountTrailingZeros(list);
  VIXL_ASSERT(((static_cast<RegList>(1) << index) & list) != 0);
  Remove(index);
  return CPURegister(index, size_, type_);
}


CPURegister CPURegList::PopHighestIndex(RegList mask) {
  RegList list = list_ & mask;
  if (list == 0) return NoCPUReg;
  int index = CountLeadingZeros(list);
  index = kRegListSizeInBits - 1 - index;
  VIXL_ASSERT(((static_cast<RegList>(1) << index) & list) != 0);
  Remove(index);
  return CPURegister(index, size_, type_);
}


bool CPURegList::IsValid() const {
  if (type_ == CPURegister::kNoRegister) {
    // We can't use IsEmpty here because that asserts IsValid().
    return list_ == 0;
  } else {
    bool is_valid = true;
    // Try to create a CPURegister for each element in the list.
    for (int i = 0; i < kRegListSizeInBits; i++) {
      if (((list_ >> i) & 1) != 0) {
        is_valid &= CPURegister(i, size_, type_).IsValid();
      }
    }
    return is_valid;
  }
}


void CPURegList::RemoveCalleeSaved() {
  if (GetType() == CPURegister::kRegister) {
    Remove(GetCalleeSaved(GetRegisterSizeInBits()));
  } else if (GetType() == CPURegister::kVRegister) {
    Remove(GetCalleeSavedV(GetRegisterSizeInBits()));
  } else {
    VIXL_ASSERT(GetType() == CPURegister::kNoRegister);
    VIXL_ASSERT(IsEmpty());
    // The list must already be empty, so do nothing.
  }
}


CPURegList CPURegList::Union(const CPURegList& list_1,
                             const CPURegList& list_2,
                             const CPURegList& list_3) {
  return Union(list_1, Union(list_2, list_3));
}


CPURegList CPURegList::Union(const CPURegList& list_1,
                             const CPURegList& list_2,
                             const CPURegList& list_3,
                             const CPURegList& list_4) {
  return Union(Union(list_1, list_2), Union(list_3, list_4));
}


CPURegList CPURegList::Intersection(const CPURegList& list_1,
                                    const CPURegList& list_2,
                                    const CPURegList& list_3) {
  return Intersection(list_1, Intersection(list_2, list_3));
}


CPURegList CPURegList::Intersection(const CPURegList& list_1,
                                    const CPURegList& list_2,
                                    const CPURegList& list_3,
                                    const CPURegList& list_4) {
  return Intersection(Intersection(list_1, list_2),
                      Intersection(list_3, list_4));
}


CPURegList CPURegList::GetCalleeSaved(unsigned size) {
  return CPURegList(CPURegister::kRegister, size, 19, 29);
}


CPURegList CPURegList::GetCalleeSavedV(unsigned size) {
  return CPURegList(CPURegister::kVRegister, size, 8, 15);
}


CPURegList CPURegList::GetCallerSaved(unsigned size) {
  // Registers x0-x18 and lr (x30) are caller-saved.
  CPURegList list = CPURegList(CPURegister::kRegister, size, 0, 18);
  // Do not use lr directly to avoid initialisation order fiasco bugs for users.
  list.Combine(Register(30, kXRegSize));
  return list;
}


CPURegList CPURegList::GetCallerSavedV(unsigned size) {
  // Registers d0-d7 and d16-d31 are caller-saved.
  CPURegList list = CPURegList(CPURegister::kVRegister, size, 0, 7);
  list.Combine(CPURegList(CPURegister::kVRegister, size, 16, 31));
  return list;
}


const CPURegList kCalleeSaved = CPURegList::GetCalleeSaved();
const CPURegList kCalleeSavedV = CPURegList::GetCalleeSavedV();
const CPURegList kCallerSaved = CPURegList::GetCallerSaved();
const CPURegList kCallerSavedV = CPURegList::GetCallerSavedV();

// Operand.
Operand::Operand(int64_t immediate)
    : immediate_(immediate),
      reg_(NoReg),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND),
      shift_amount_(0) {}

Operand::Operand(IntegerOperand immediate)
    : immediate_(immediate.AsIntN(64)),
      reg_(NoReg),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND),
      shift_amount_(0) {}

Operand::Operand(Register reg, Shift shift, unsigned shift_amount)
    : reg_(reg),
      shift_(shift),
      extend_(NO_EXTEND),
      shift_amount_(shift_amount) {
  VIXL_ASSERT(shift != MSL);
  VIXL_ASSERT(reg.Is64Bits() || (shift_amount < kWRegSize));
  VIXL_ASSERT(reg.Is32Bits() || (shift_amount < kXRegSize));
  VIXL_ASSERT(!reg.IsSP());
}


Operand::Operand(Register reg, Extend extend, unsigned shift_amount)
    : reg_(reg),
      shift_(NO_SHIFT),
      extend_(extend),
      shift_amount_(shift_amount) {
  VIXL_ASSERT(reg.IsValid());
  VIXL_ASSERT(shift_amount <= 4);
  VIXL_ASSERT(!reg.IsSP());

  // Extend modes SXTX and UXTX require a 64-bit register.
  VIXL_ASSERT(reg.Is64Bits() || ((extend != SXTX) && (extend != UXTX)));
}


bool Operand::IsImmediate() const { return reg_.Is(NoReg); }


bool Operand::IsPlainRegister() const {
  return reg_.IsValid() &&
         (((shift_ == NO_SHIFT) && (extend_ == NO_EXTEND)) ||
          // No-op shifts.
          ((shift_ != NO_SHIFT) && (shift_amount_ == 0)) ||
          // No-op extend operations.
          // We can't include [US]XTW here without knowing more about the
          // context; they are only no-ops for 32-bit operations.
          //
          // For example, this operand could be replaced with w1:
          //   __ Add(w0, w0, Operand(w1, UXTW));
          // However, no plain register can replace it in this context:
          //   __ Add(x0, x0, Operand(w1, UXTW));
          (((extend_ == UXTX) || (extend_ == SXTX)) && (shift_amount_ == 0)));
}


bool Operand::IsShiftedRegister() const {
  return reg_.IsValid() && (shift_ != NO_SHIFT);
}


bool Operand::IsExtendedRegister() const {
  return reg_.IsValid() && (extend_ != NO_EXTEND);
}


bool Operand::IsZero() const {
  if (IsImmediate()) {
    return GetImmediate() == 0;
  } else {
    return GetRegister().IsZero();
  }
}


Operand Operand::ToExtendedRegister() const {
  VIXL_ASSERT(IsShiftedRegister());
  VIXL_ASSERT((shift_ == LSL) && (shift_amount_ <= 4));
  return Operand(reg_, reg_.Is64Bits() ? UXTX : UXTW, shift_amount_);
}


// MemOperand
MemOperand::MemOperand()
    : base_(NoReg),
      regoffset_(NoReg),
      offset_(0),
      addrmode_(Offset),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND) {}


MemOperand::MemOperand(Register base, int64_t offset, AddrMode addrmode)
    : base_(base),
      regoffset_(NoReg),
      offset_(offset),
      addrmode_(addrmode),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND),
      shift_amount_(0) {
  VIXL_ASSERT(base.Is64Bits() && !base.IsZero());
}


MemOperand::MemOperand(Register base,
                       Register regoffset,
                       Extend extend,
                       unsigned shift_amount)
    : base_(base),
      regoffset_(regoffset),
      offset_(0),
      addrmode_(Offset),
      shift_(NO_SHIFT),
      extend_(extend),
      shift_amount_(shift_amount) {
  VIXL_ASSERT(base.Is64Bits() && !base.IsZero());
  VIXL_ASSERT(!regoffset.IsSP());
  VIXL_ASSERT((extend == UXTW) || (extend == SXTW) || (extend == SXTX));

  // SXTX extend mode requires a 64-bit offset register.
  VIXL_ASSERT(regoffset.Is64Bits() || (extend != SXTX));
}


MemOperand::MemOperand(Register base,
                       Register regoffset,
                       Shift shift,
                       unsigned shift_amount)
    : base_(base),
      regoffset_(regoffset),
      offset_(0),
      addrmode_(Offset),
      shift_(shift),
      extend_(NO_EXTEND),
      shift_amount_(shift_amount) {
  VIXL_ASSERT(base.Is64Bits() && !base.IsZero());
  VIXL_ASSERT(regoffset.Is64Bits() && !regoffset.IsSP());
  VIXL_ASSERT(shift == LSL);
}


MemOperand::MemOperand(Register base, const Operand& offset, AddrMode addrmode)
    : base_(base),
      regoffset_(NoReg),
      addrmode_(addrmode),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND),
      shift_amount_(0) {
  VIXL_ASSERT(base.Is64Bits() && !base.IsZero());

  if (offset.IsImmediate()) {
    offset_ = offset.GetImmediate();
  } else if (offset.IsShiftedRegister()) {
    VIXL_ASSERT((addrmode == Offset) || (addrmode == PostIndex));

    regoffset_ = offset.GetRegister();
    shift_ = offset.GetShift();
    shift_amount_ = offset.GetShiftAmount();

    extend_ = NO_EXTEND;
    offset_ = 0;

    // These assertions match those in the shifted-register constructor.
    VIXL_ASSERT(regoffset_.Is64Bits() && !regoffset_.IsSP());
    VIXL_ASSERT(shift_ == LSL);
  } else {
    VIXL_ASSERT(offset.IsExtendedRegister());
    VIXL_ASSERT(addrmode == Offset);

    regoffset_ = offset.GetRegister();
    extend_ = offset.GetExtend();
    shift_amount_ = offset.GetShiftAmount();

    shift_ = NO_SHIFT;
    offset_ = 0;

    // These assertions match those in the extended-register constructor.
    VIXL_ASSERT(!regoffset_.IsSP());
    VIXL_ASSERT((extend_ == UXTW) || (extend_ == SXTW) || (extend_ == SXTX));
    VIXL_ASSERT((regoffset_.Is64Bits() || (extend_ != SXTX)));
  }
}


bool MemOperand::IsPlainRegister() const {
  return IsImmediateOffset() && (GetOffset() == 0);
}


bool MemOperand::IsEquivalentToPlainRegister() const {
  if (regoffset_.Is(NoReg)) {
    // Immediate offset, pre-index or post-index.
    return GetOffset() == 0;
  } else if (GetRegisterOffset().IsZero()) {
    // Zero register offset, pre-index or post-index.
    // We can ignore shift and extend options because they all result in zero.
    return true;
  }
  return false;
}


bool MemOperand::IsImmediateOffset() const {
  return (addrmode_ == Offset) && regoffset_.Is(NoReg);
}


bool MemOperand::IsRegisterOffset() const {
  return (addrmode_ == Offset) && !regoffset_.Is(NoReg);
}

bool MemOperand::IsPreIndex() const { return addrmode_ == PreIndex; }
bool MemOperand::IsPostIndex() const { return addrmode_ == PostIndex; }

bool MemOperand::IsImmediatePreIndex() const {
  return IsPreIndex() && regoffset_.Is(NoReg);
}

bool MemOperand::IsImmediatePostIndex() const {
  return IsPostIndex() && regoffset_.Is(NoReg);
}

void MemOperand::AddOffset(int64_t offset) {
  VIXL_ASSERT(IsImmediateOffset());
  offset_ += offset;
}


bool SVEMemOperand::IsValid() const {
#ifdef VIXL_DEBUG
  {
    // It should not be possible for an SVEMemOperand to match multiple types.
    int count = 0;
    if (IsScalarPlusImmediate()) count++;
    if (IsScalarPlusScalar()) count++;
    if (IsScalarPlusVector()) count++;
    if (IsVectorPlusImmediate()) count++;
    if (IsVectorPlusScalar()) count++;
    if (IsVectorPlusVector()) count++;
    VIXL_ASSERT(count <= 1);
  }
#endif

  // We can't have a register _and_ an immediate offset.
  if ((offset_ != 0) && (!regoffset_.IsNone())) return false;

  if (shift_amount_ != 0) {
    // Only shift and extend modifiers can take a shift amount.
    switch (mod_) {
      case NO_SVE_OFFSET_MODIFIER:
      case SVE_MUL_VL:
        return false;
      case SVE_LSL:
      case SVE_UXTW:
      case SVE_SXTW:
        // Fall through.
        break;
    }
  }

  return IsScalarPlusImmediate() || IsScalarPlusScalar() ||
         IsScalarPlusVector() || IsVectorPlusImmediate() ||
         IsVectorPlusScalar() || IsVectorPlusVector();
}


bool SVEMemOperand::IsEquivalentToScalar() const {
  if (IsScalarPlusImmediate()) {
    return GetImmediateOffset() == 0;
  }
  if (IsScalarPlusScalar()) {
    // We can ignore the shift because it will still result in zero.
    return GetScalarOffset().IsZero();
  }
  // Forms involving vectors are never equivalent to a single scalar.
  return false;
}

bool SVEMemOperand::IsPlainRegister() const {
  if (IsScalarPlusImmediate()) {
    return GetImmediateOffset() == 0;
  }
  return false;
}

GenericOperand::GenericOperand(const CPURegister& reg)
    : cpu_register_(reg), mem_op_size_(0) {
  if (reg.IsQ()) {
    VIXL_ASSERT(reg.GetSizeInBits() > static_cast<int>(kXRegSize));
    // Support for Q registers is not implemented yet.
    VIXL_UNIMPLEMENTED();
  }
}


GenericOperand::GenericOperand(const MemOperand& mem_op, size_t mem_op_size)
    : cpu_register_(NoReg), mem_op_(mem_op), mem_op_size_(mem_op_size) {
  if (mem_op_size_ > kXRegSizeInBytes) {
    // We only support generic operands up to the size of X registers.
    VIXL_UNIMPLEMENTED();
  }
}

bool GenericOperand::Equals(const GenericOperand& other) const {
  if (!IsValid() || !other.IsValid()) {
    // Two invalid generic operands are considered equal.
    return !IsValid() && !other.IsValid();
  }
  if (IsCPURegister() && other.IsCPURegister()) {
    return GetCPURegister().Is(other.GetCPURegister());
  } else if (IsMemOperand() && other.IsMemOperand()) {
    return GetMemOperand().Equals(other.GetMemOperand()) &&
           (GetMemOperandSizeInBytes() == other.GetMemOperandSizeInBytes());
  }
  return false;
}
}  // namespace aarch64
}  // namespace vixl
