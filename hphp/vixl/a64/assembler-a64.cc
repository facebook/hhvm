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


#include "hphp/vixl/a64/assembler-a64.h"

#include <cmath>

namespace vixl {

// CPURegList utilities.
CPURegister CPURegList::PopLowestIndex() {
  if (IsEmpty()) {
    return NoCPUReg;
  }
  int index = CountTrailingZeros(list_, kRegListSizeInBits);
  assert((1 << index) & list_);
  Remove(index);
  return CPURegister(index, size_, type_);
}


CPURegister CPURegList::PopHighestIndex() {
  assert(IsValid());
  if (IsEmpty()) {
    return NoCPUReg;
  }
  int index = CountLeadingZeros(list_, kRegListSizeInBits);
  index = kRegListSizeInBits - 1 - index;
  assert((1 << index) & list_);
  Remove(index);
  return CPURegister(index, size_, type_);
}


bool CPURegList::IsValid() const {
  if ((type_ == CPURegister::kRegister) ||
      (type_ == CPURegister::kFPRegister)) {
    bool is_valid = true;
    // Try to create a CPURegister for each element in the list.
    for (int i = 0; i < kRegListSizeInBits; i++) {
      if (((list_ >> i) & 1) != 0) {
        is_valid &= CPURegister(i, size_, type_).IsValid();
      }
    }
    return is_valid;
  } else {
    return false;
  }
}


void CPURegList::RemoveCalleeSaved() {
  if (type() == CPURegister::kRegister) {
    Remove(GetCalleeSaved(RegisterSizeInBits()));
  } else if (type() == CPURegister::kFPRegister) {
    Remove(GetCalleeSavedFP(RegisterSizeInBits()));
  }
}


CPURegList CPURegList::GetCalleeSaved(unsigned size) {
  return CPURegList(CPURegister::kRegister, size, 19, 29);
}


CPURegList CPURegList::GetCalleeSavedFP(unsigned size) {
  return CPURegList(CPURegister::kFPRegister, size, 8, 15);
}


CPURegList CPURegList::GetCallerSaved(unsigned size) {
  // Registers x0-x18 and lr (x30) are caller-saved.
  CPURegList list = CPURegList(CPURegister::kRegister, size, 0, 18);
  list.Combine(lr);
  return list;
}


CPURegList CPURegList::GetCallerSavedFP(unsigned size) {
  // Registers d0-d7 and d16-d31 are caller-saved.
  CPURegList list = CPURegList(CPURegister::kFPRegister, size, 0, 7);
  list.Combine(CPURegList(CPURegister::kFPRegister, size, 16, 31));
  return list;
}


const CPURegList kCalleeSaved = CPURegList::GetCalleeSaved();
const CPURegList kCalleeSavedFP = CPURegList::GetCalleeSavedFP();
const CPURegList kCallerSaved = CPURegList::GetCallerSaved();
const CPURegList kCallerSavedFP = CPURegList::GetCallerSavedFP();


// Registers.
#define WREG(n) w##n,
const Register Register::wregisters[] = {
REGISTER_CODE_LIST(WREG)
};
#undef WREG

#define XREG(n) x##n,
const Register Register::xregisters[] = {
REGISTER_CODE_LIST(XREG)
};
#undef XREG

#define SREG(n) s##n,
const FPRegister FPRegister::sregisters[] = {
REGISTER_CODE_LIST(SREG)
};
#undef SREG

#define DREG(n) d##n,
const FPRegister FPRegister::dregisters[] = {
REGISTER_CODE_LIST(DREG)
};
#undef DREG


MemOperand Register::operator[](const ptrdiff_t offset) const {
  return MemOperand { *this, offset };
}

MemOperand Register::operator[](const Register& offset) const {
  return MemOperand { *this, offset };
}


const Register& Register::WRegFromCode(unsigned code) {
  // This function returns the zero register when code = 31. The stack pointer
  // can not be returned.
  assert(code < kNumberOfRegisters);
  return wregisters[code];
}


const Register& Register::XRegFromCode(unsigned code) {
  // This function returns the zero register when code = 31. The stack pointer
  // can not be returned.
  assert(code < kNumberOfRegisters);
  return xregisters[code];
}


const FPRegister& FPRegister::SRegFromCode(unsigned code) {
  assert(code < kNumberOfFPRegisters);
  return sregisters[code];
}


const FPRegister& FPRegister::DRegFromCode(unsigned code) {
  assert(code < kNumberOfFPRegisters);
  return dregisters[code];
}


const Register& CPURegister::W() const {
  assert(IsValidRegister());
  assert(Is64Bits());
  return Register::WRegFromCode(code_);
}


const Register& CPURegister::X() const {
  assert(IsValidRegister());
  assert(Is32Bits());
  return Register::XRegFromCode(code_);
}


const FPRegister& CPURegister::S() const {
  assert(IsValidFPRegister());
  assert(Is64Bits());
  return FPRegister::SRegFromCode(code_);
}


const FPRegister& CPURegister::D() const {
  assert(IsValidFPRegister());
  assert(Is32Bits());
  return FPRegister::DRegFromCode(code_);
}


// Operand.
Operand::Operand(int64_t immediate)
    : immediate_(immediate),
      reg_(NoReg),
      shift_(NO_SHIFT),
      extend_(NO_EXTEND),
      shift_amount_(0) {}


Operand::Operand(Register reg, Shift shift, unsigned shift_amount)
    : reg_(reg),
      shift_(shift),
      extend_(NO_EXTEND),
      shift_amount_(shift_amount) {
  assert(reg.Is64Bits() || (shift_amount < kWRegSize));
  assert(reg.Is32Bits() || (shift_amount < kXRegSize));
  assert(!reg.IsSP());
}


Operand::Operand(Register reg, Extend extend, unsigned shift_amount)
    : reg_(reg),
      shift_(NO_SHIFT),
      extend_(extend),
      shift_amount_(shift_amount) {
  assert(reg.IsValid());
  assert(shift_amount <= 4);
  assert(!reg.IsSP());
}


bool Operand::IsImmediate() const {
  return reg_.Is(NoReg);
}


bool Operand::IsShiftedRegister() const {
  return reg_.IsValid() && (shift_ != NO_SHIFT);
}


bool Operand::IsExtendedRegister() const {
  return reg_.IsValid() && (extend_ != NO_EXTEND);
}


Operand Operand::ToExtendedRegister() const {
  assert(IsShiftedRegister());
  assert((shift_ == LSL) && (shift_amount_ <= 4));
  return Operand(reg_, reg_.Is64Bits() ? UXTX : UXTW, shift_amount_);
}


// MemOperand
MemOperand::MemOperand(Register base, ptrdiff_t offset, AddrMode addrmode)
  : base_(base), regoffset_(NoReg), offset_(offset), addrmode_(addrmode) {
  assert(base.Is64Bits() && !base.IsZero());
}


MemOperand::MemOperand(Register base,
                       Register regoffset,
                       Extend extend,
                       unsigned shift_amount)
  : base_(base), regoffset_(regoffset), offset_(0), addrmode_(Offset),
    shift_(NO_SHIFT), extend_(extend), shift_amount_(shift_amount) {
  assert(base.Is64Bits() && !base.IsZero());
  assert(!regoffset.IsSP());
  assert((extend == UXTW) || (extend == SXTW) || (extend == SXTX));
}


MemOperand::MemOperand(Register base,
                       Register regoffset,
                       Shift shift,
                       unsigned shift_amount)
  : base_(base), regoffset_(regoffset), offset_(0), addrmode_(Offset),
    shift_(shift), extend_(NO_EXTEND), shift_amount_(shift_amount) {
  assert(base.Is64Bits() && !base.IsZero());
  assert(!regoffset.IsSP());
  assert(shift == LSL);
}


MemOperand::MemOperand(Register base, const Operand& offset, AddrMode addrmode)
  : base_(base), regoffset_(NoReg), addrmode_(addrmode) {
  assert(base.Is64Bits() && !base.IsZero());

  if (offset.IsImmediate()) {
    offset_ = offset.immediate();
  } else if (offset.IsShiftedRegister()) {
    assert(addrmode == Offset);

    regoffset_ = offset.reg();
    shift_= offset.shift();
    shift_amount_ = offset.shift_amount();

    extend_ = NO_EXTEND;
    offset_ = 0;

    // These assertions match those in the shifted-register constructor.
    assert(!regoffset_.IsSP());
    assert(shift_ == LSL);
  } else {
    assert(offset.IsExtendedRegister());
    assert(addrmode == Offset);

    regoffset_ = offset.reg();
    extend_ = offset.extend();
    shift_amount_ = offset.shift_amount();

    shift_= NO_SHIFT;
    offset_ = 0;

    // These assertions match those in the extended-register constructor.
    assert(!regoffset_.IsSP());
    assert((extend_ == UXTW) || (extend_ == SXTW) || (extend_ == SXTX));
  }
}


bool MemOperand::IsImmediateOffset() const {
  return (addrmode_ == Offset) && regoffset_.Is(NoReg);
}


bool MemOperand::IsRegisterOffset() const {
  return (addrmode_ == Offset) && !regoffset_.Is(NoReg);
}


bool MemOperand::IsPreIndex() const {
  return addrmode_ == PreIndex;
}


bool MemOperand::IsPostIndex() const {
  return addrmode_ == PostIndex;
}


// Assembler
Assembler::Assembler(HPHP::CodeBlock& cb)
    : cb_(cb), literal_pool_monitor_(0) {
  // Assert that this is an LP64 system.
  assert(sizeof(int) == sizeof(int32_t));     // NOLINT(runtime/sizeof)
  assert(sizeof(long) == sizeof(int64_t));    // NOLINT(runtime/int)
  assert(sizeof(void *) == sizeof(int64_t));  // NOLINT(runtime/sizeof)
  assert(sizeof(1) == sizeof(int32_t));       // NOLINT(runtime/sizeof)
  assert(sizeof(1L) == sizeof(int64_t));      // NOLINT(runtime/sizeof)
}


Assembler::~Assembler() {
  FinalizeCode();
  assert(finalized_ || (cb_.used() == 0));
  assert(literals_.empty());
}


void Assembler::Reset() {
#ifdef DEBUG
  assert(literal_pool_monitor_ == 0);
  cb_.zero();
  finalized_ = false;
#endif
  cb_.clear();
  literals_.clear();
  next_literal_pool_check_ = cb_.frontier() + kLiteralPoolCheckInterval;
}


void Assembler::FinalizeCode() {
  if (!literals_.empty()) {
    EmitLiteralPool();
  }
#ifdef DEBUG
  finalized_ = true;
#endif
}


void Assembler::bind(Label* label) {
  label->is_bound_ = true;
  label->target_ = cb_.frontier();
  while (label->IsLinked()) {
    // Get the address of the following instruction in the chain.
    auto link = Instruction::Cast(label->link_);
    Instruction* next_link = link->ImmPCOffsetTarget();
    // Update the instruction target.
    link->SetImmPCOffsetTarget(Instruction::Cast(label->target_));
    // Update the label's link.
    // If the offset of the branch we just updated was 0 (kEndOfChain) we are
    // done.
    label->link_ = (link != next_link
                    ? reinterpret_cast<HPHP::CodeAddress>(next_link)
                    : nullptr);
  }
}


int Assembler::UpdateAndGetByteOffsetTo(Label* label) {
  int offset;
  if (label->IsBound()) {
    offset = label->target() - cb_.frontier();
  } else if (label->IsLinked()) {
    offset = label->link() - cb_.frontier();
  } else {
    offset = Label::kEndOfChain;
  }
  label->set_link(cb_.frontier());
  return offset;
}


// Code generation.
void Assembler::br(const Register& xn) {
  assert(xn.Is64Bits());
  Emit(BR | Rn(xn));
}


void Assembler::blr(const Register& xn) {
  assert(xn.Is64Bits());
  Emit(BLR | Rn(xn));
}


void Assembler::ret(const Register& xn) {
  assert(xn.Is64Bits());
  Emit(RET | Rn(xn));
}


void Assembler::b(int imm26) {
  Emit(B | ImmUncondBranch(imm26));
}


void Assembler::b(int imm19, Condition cond) {
  Emit(B_cond | ImmCondBranch(imm19) | cond);
}


void Assembler::b(Label* label) {
  b(UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::b(Label* label, Condition cond) {
  b(UpdateAndGetInstructionOffsetTo(label), cond);
}


void Assembler::bl(int imm26) {
  Emit(BL | ImmUncondBranch(imm26));
}


void Assembler::bl(Label* label) {
  bl(UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::cbz(const Register& rt,
                    int imm19) {
  Emit(SF(rt) | CBZ | ImmCmpBranch(imm19) | Rt(rt));
}


void Assembler::cbz(const Register& rt,
                    Label* label) {
  cbz(rt, UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::cbnz(const Register& rt,
                     int imm19) {
  Emit(SF(rt) | CBNZ | ImmCmpBranch(imm19) | Rt(rt));
}


void Assembler::cbnz(const Register& rt,
                     Label* label) {
  cbnz(rt, UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::tbz(const Register& rt,
                    unsigned bit_pos,
                    int imm14) {
  assert(rt.Is64Bits());
  Emit(TBZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
}


void Assembler::tbz(const Register& rt,
                    unsigned bit_pos,
                    Label* label) {
  tbz(rt, bit_pos, UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::tbnz(const Register& rt,
                     unsigned bit_pos,
                     int imm14) {
  assert(rt.Is64Bits());
  Emit(TBNZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
}


void Assembler::tbnz(const Register& rt,
                     unsigned bit_pos,
                     Label* label) {
  tbnz(rt, bit_pos, UpdateAndGetInstructionOffsetTo(label));
}


void Assembler::adr(const Register& rd, int imm21) {
  assert(rd.Is64Bits());
  Emit(ADR | ImmPCRelAddress(imm21) | Rd(rd));
}


void Assembler::adr(const Register& rd, Label* label) {
  adr(rd, UpdateAndGetByteOffsetTo(label));
}


void Assembler::add(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    FlagsUpdate S) {
  AddSub(rd, rn, operand, S, ADD);
}


void Assembler::cmn(const Register& rn,
                    const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rn);
  add(zr, rn, operand, SetFlags);
}


void Assembler::sub(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    FlagsUpdate S) {
  AddSub(rd, rn, operand, S, SUB);
}


void Assembler::cmp(const Register& rn, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rn);
  sub(zr, rn, operand, SetFlags);
}


void Assembler::neg(const Register& rd, const Operand& operand, FlagsUpdate S) {
  Register zr = AppropriateZeroRegFor(rd);
  sub(rd, zr, operand, S);
}


void Assembler::adc(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    FlagsUpdate S) {
  AddSubWithCarry(rd, rn, operand, S, ADC);
}


void Assembler::sbc(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    FlagsUpdate S) {
  AddSubWithCarry(rd, rn, operand, S, SBC);
}


void Assembler::ngc(const Register& rd, const Operand& operand, FlagsUpdate S) {
  Register zr = AppropriateZeroRegFor(rd);
  sbc(rd, zr, operand, S);
}


// Logical instructions.
void Assembler::and_(const Register& rd,
                     const Register& rn,
                     const Operand& operand,
                     FlagsUpdate S) {
  Logical(rd, rn, operand, (S == SetFlags) ? ANDS : AND);
}


void Assembler::tst(const Register& rn,
                    const Operand& operand) {
  and_(AppropriateZeroRegFor(rn), rn, operand, SetFlags);
}


void Assembler::bic(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    FlagsUpdate S) {
  Logical(rd, rn, operand, (S == SetFlags) ? BICS : BIC);
}


void Assembler::orr(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  Logical(rd, rn, operand, ORR);
}


void Assembler::orn(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  Logical(rd, rn, operand, ORN);
}


void Assembler::eor(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  Logical(rd, rn, operand, EOR);
}


void Assembler::eon(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  Logical(rd, rn, operand, EON);
}


void Assembler::lslv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | LSLV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::lsrv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | LSRV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::asrv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | ASRV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::rorv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | RORV | Rm(rm) | Rn(rn) | Rd(rd));
}


// Bitfield operations.
void Assembler::bfm(const Register& rd,
                     const Register& rn,
                     unsigned immr,
                     unsigned imms) {
  assert(rd.size() == rn.size());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | BFM | N |
       ImmR(immr, rd.size()) | ImmS(imms, rd.size()) | Rn(rn) | Rd(rd));
}


void Assembler::sbfm(const Register& rd,
                     const Register& rn,
                     unsigned immr,
                     unsigned imms) {
  assert(rd.size() == rn.size());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | SBFM | N |
       ImmR(immr, rd.size()) | ImmS(imms, rd.size()) | Rn(rn) | Rd(rd));
}


void Assembler::ubfm(const Register& rd,
                     const Register& rn,
                     unsigned immr,
                     unsigned imms) {
  assert(rd.size() == rn.size());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | UBFM | N |
       ImmR(immr, rd.size()) | ImmS(imms, rd.size()) | Rn(rn) | Rd(rd));
}


void Assembler::extr(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     unsigned lsb) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | EXTR | N | Rm(rm) | ImmS(lsb, rd.size()) | Rn(rn) | Rd(rd));
}


void Assembler::csel(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     Condition cond) {
  ConditionalSelect(rd, rn, rm, cond, CSEL);
}


void Assembler::csinc(const Register& rd,
                      const Register& rn,
                      const Register& rm,
                      Condition cond) {
  ConditionalSelect(rd, rn, rm, cond, CSINC);
}


void Assembler::csinv(const Register& rd,
                      const Register& rn,
                      const Register& rm,
                      Condition cond) {
  ConditionalSelect(rd, rn, rm, cond, CSINV);
}


void Assembler::csneg(const Register& rd,
                      const Register& rn,
                      const Register& rm,
                      Condition cond) {
  ConditionalSelect(rd, rn, rm, cond, CSNEG);
}


void Assembler::cset(const Register &rd, Condition cond) {
  assert((cond != al) && (cond != nv));
  Register zr = AppropriateZeroRegFor(rd);
  csinc(rd, zr, zr, InvertCondition(cond));
}


void Assembler::csetm(const Register &rd, Condition cond) {
  assert((cond != al) && (cond != nv));
  Register zr = AppropriateZeroRegFor(rd);
  csinv(rd, zr, zr, InvertCondition(cond));
}


void Assembler::cinc(const Register &rd, const Register &rn, Condition cond) {
  assert((cond != al) && (cond != nv));
  csinc(rd, rn, rn, InvertCondition(cond));
}


void Assembler::cinv(const Register &rd, const Register &rn, Condition cond) {
  assert((cond != al) && (cond != nv));
  csinv(rd, rn, rn, InvertCondition(cond));
}


void Assembler::cneg(const Register &rd, const Register &rn, Condition cond) {
  assert((cond != al) && (cond != nv));
  csneg(rd, rn, rn, InvertCondition(cond));
}


void Assembler::ConditionalSelect(const Register& rd,
                                  const Register& rn,
                                  const Register& rm,
                                  Condition cond,
                                  ConditionalSelectOp op) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | op | Rm(rm) | Cond(cond) | Rn(rn) | Rd(rd));
}


void Assembler::ccmn(const Register& rn,
                     const Operand& operand,
                     StatusFlags nzcv,
                     Condition cond) {
  ConditionalCompare(rn, operand, nzcv, cond, CCMN);
}


void Assembler::ccmp(const Register& rn,
                     const Operand& operand,
                     StatusFlags nzcv,
                     Condition cond) {
  ConditionalCompare(rn, operand, nzcv, cond, CCMP);
}


void Assembler::DataProcessing3Source(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     const Register& ra,
                     DataProcessing3SourceOp op) {
  Emit(SF(rd) | op | Rm(rm) | Ra(ra) | Rn(rn) | Rd(rd));
}


void Assembler::mul(const Register& rd,
                    const Register& rn,
                    const Register& rm) {
  assert(AreSameSizeAndType(rd, rn, rm));
  DataProcessing3Source(rd, rn, rm, AppropriateZeroRegFor(rd), MADD);
}


void Assembler::madd(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     const Register& ra) {
  DataProcessing3Source(rd, rn, rm, ra, MADD);
}


void Assembler::mneg(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(AreSameSizeAndType(rd, rn, rm));
  DataProcessing3Source(rd, rn, rm, AppropriateZeroRegFor(rd), MSUB);
}


void Assembler::msub(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     const Register& ra) {
  DataProcessing3Source(rd, rn, rm, ra, MSUB);
}


void Assembler::umaddl(const Register& rd,
                       const Register& rn,
                       const Register& rm,
                       const Register& ra) {
  assert(rd.Is64Bits() && ra.Is64Bits());
  assert(rn.Is32Bits() && rm.Is32Bits());
  DataProcessing3Source(rd, rn, rm, ra, UMADDL_x);
}


void Assembler::smaddl(const Register& rd,
                       const Register& rn,
                       const Register& rm,
                       const Register& ra) {
  assert(rd.Is64Bits() && ra.Is64Bits());
  assert(rn.Is32Bits() && rm.Is32Bits());
  DataProcessing3Source(rd, rn, rm, ra, SMADDL_x);
}


void Assembler::umsubl(const Register& rd,
                       const Register& rn,
                       const Register& rm,
                       const Register& ra) {
  assert(rd.Is64Bits() && ra.Is64Bits());
  assert(rn.Is32Bits() && rm.Is32Bits());
  DataProcessing3Source(rd, rn, rm, ra, UMSUBL_x);
}


void Assembler::smsubl(const Register& rd,
                       const Register& rn,
                       const Register& rm,
                       const Register& ra) {
  assert(rd.Is64Bits() && ra.Is64Bits());
  assert(rn.Is32Bits() && rm.Is32Bits());
  DataProcessing3Source(rd, rn, rm, ra, SMSUBL_x);
}


void Assembler::smull(const Register& rd,
                      const Register& rn,
                      const Register& rm) {
  assert(rd.Is64Bits());
  assert(rn.Is32Bits() && rm.Is32Bits());
  DataProcessing3Source(rd, rn, rm, xzr, SMADDL_x);
}


void Assembler::sdiv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | SDIV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::smulh(const Register& xd,
                      const Register& xn,
                      const Register& xm) {
  assert(xd.Is64Bits() && xn.Is64Bits() && xm.Is64Bits());
  DataProcessing3Source(xd, xn, xm, xzr, SMULH_x);
}

void Assembler::udiv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  assert(rd.size() == rn.size());
  assert(rd.size() == rm.size());
  Emit(SF(rd) | UDIV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::rbit(const Register& rd,
                     const Register& rn) {
  DataProcessing1Source(rd, rn, RBIT);
}


void Assembler::rev16(const Register& rd,
                      const Register& rn) {
  DataProcessing1Source(rd, rn, REV16);
}


void Assembler::rev32(const Register& rd,
                      const Register& rn) {
  assert(rd.Is64Bits());
  DataProcessing1Source(rd, rn, REV);
}


void Assembler::rev(const Register& rd,
                    const Register& rn) {
  DataProcessing1Source(rd, rn, rd.Is64Bits() ? REV_x : REV_w);
}


void Assembler::clz(const Register& rd,
                    const Register& rn) {
  DataProcessing1Source(rd, rn, CLZ);
}


void Assembler::cls(const Register& rd,
                    const Register& rn) {
  DataProcessing1Source(rd, rn, CLS);
}


void Assembler::ldp(const CPURegister& rt,
                    const CPURegister& rt2,
                    const MemOperand& src) {
  LoadStorePair(rt, rt2, src, LoadPairOpFor(rt, rt2));
}


void Assembler::stp(const CPURegister& rt,
                    const CPURegister& rt2,
                    const MemOperand& dst) {
  LoadStorePair(rt, rt2, dst, StorePairOpFor(rt, rt2));
}


void Assembler::ldpsw(const Register& rt,
                      const Register& rt2,
                      const MemOperand& src) {
  assert(rt.Is64Bits());
  LoadStorePair(rt, rt2, src, LDPSW_x);
}


void Assembler::LoadStorePair(const CPURegister& rt,
                              const CPURegister& rt2,
                              const MemOperand& addr,
                              LoadStorePairOp op) {
  // 'rt' and 'rt2' can only be aliased for stores.
  assert(((op & LoadStorePairLBit) == 0) || !rt.Is(rt2));
  assert(AreSameSizeAndType(rt, rt2));

  Instr memop = op | Rt(rt) | Rt2(rt2) | RnSP(addr.base()) |
                ImmLSPair(addr.offset(), CalcLSPairDataSize(op));

  Instr addrmodeop;
  if (addr.IsImmediateOffset()) {
    addrmodeop = LoadStorePairOffsetFixed;
  } else {
    assert(addr.offset() != 0);
    if (addr.IsPreIndex()) {
      addrmodeop = LoadStorePairPreIndexFixed;
    } else {
      assert(addr.IsPostIndex());
      addrmodeop = LoadStorePairPostIndexFixed;
    }
  }
  Emit(addrmodeop | memop);
}


void Assembler::ldnp(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& src) {
  LoadStorePairNonTemporal(rt, rt2, src,
                           LoadPairNonTemporalOpFor(rt, rt2));
}


void Assembler::stnp(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& dst) {
  LoadStorePairNonTemporal(rt, rt2, dst,
                           StorePairNonTemporalOpFor(rt, rt2));
}


void Assembler::LoadStorePairNonTemporal(const CPURegister& rt,
                                         const CPURegister& rt2,
                                         const MemOperand& addr,
                                         LoadStorePairNonTemporalOp op) {
  assert(!rt.Is(rt2));
  assert(AreSameSizeAndType(rt, rt2));
  assert(addr.IsImmediateOffset());

  LSDataSize size = CalcLSPairDataSize(
    static_cast<LoadStorePairOp>(op & LoadStorePairMask));
  Emit(op | Rt(rt) | Rt2(rt2) | RnSP(addr.base()) |
       ImmLSPair(addr.offset(), size));
}


// Memory instructions.
void Assembler::ldrb(const Register& rt, const MemOperand& src) {
  LoadStore(rt, src, LDRB_w);
}


void Assembler::strb(const Register& rt, const MemOperand& dst) {
  LoadStore(rt, dst, STRB_w);
}


void Assembler::ldrsb(const Register& rt, const MemOperand& src) {
  LoadStore(rt, src, rt.Is64Bits() ? LDRSB_x : LDRSB_w);
}


void Assembler::ldrh(const Register& rt, const MemOperand& src) {
  LoadStore(rt, src, LDRH_w);
}


void Assembler::strh(const Register& rt, const MemOperand& dst) {
  LoadStore(rt, dst, STRH_w);
}


void Assembler::ldrsh(const Register& rt, const MemOperand& src) {
  LoadStore(rt, src, rt.Is64Bits() ? LDRSH_x : LDRSH_w);
}


void Assembler::ldr(const CPURegister& rt, const MemOperand& src) {
  LoadStore(rt, src, LoadOpFor(rt));
}


void Assembler::str(const CPURegister& rt, const MemOperand& src) {
  LoadStore(rt, src, StoreOpFor(rt));
}


void Assembler::ldr(const Register& rt, Label* label) {
  assert(rt.Is64Bits());
  Emit(LDR_x_lit
       | ImmLLiteral(UpdateAndGetInstructionOffsetTo(label))
       | Rt(rt));
}


void Assembler::ldrsw(const Register& rt, const MemOperand& src) {
  assert(rt.Is64Bits());
  LoadStore(rt, src, LDRSW_x);
}


void Assembler::ldr(const Register& rt, uint64_t imm) {
  LoadLiteral(rt, imm, rt.Is64Bits() ? LDR_x_lit : LDR_w_lit);
}


void Assembler::ldr(const FPRegister& ft, double imm) {
  uint64_t rawbits = 0;
  LoadLiteralOp op;

  if (ft.Is64Bits()) {
    rawbits = double_to_rawbits(imm);
    op = LDR_d_lit;
  } else {
    assert(ft.Is32Bits());
    float float_imm = static_cast<float>(imm);
    rawbits = float_to_rawbits(float_imm);
    op = LDR_s_lit;
  }

  LoadLiteral(ft, rawbits, op);
}


void Assembler::mov(const Register& rd, const Register& rm) {
  // Moves involving the stack pointer are encoded as add immediate with
  // second operand of zero. Otherwise, orr with first operand zr is
  // used.
  if (rd.IsSP() || rm.IsSP()) {
    add(rd, rm, 0);
  } else {
    orr(rd, AppropriateZeroRegFor(rd), rm);
  }
}


void Assembler::mvn(const Register& rd, const Operand& operand) {
  orn(rd, AppropriateZeroRegFor(rd), operand);
}


void Assembler::mrs(const Register& rt, SystemRegister sysreg) {
  assert(rt.Is64Bits());
  Emit(MRS | ImmSystemRegister(sysreg) | Rt(rt));
}


void Assembler::msr(SystemRegister sysreg, const Register& rt) {
  assert(rt.Is64Bits());
  Emit(MSR | Rt(rt) | ImmSystemRegister(sysreg));
}


void Assembler::hint(SystemHint code) {
  Emit(HINT | ImmHint(code) | Rt(xzr));
}


void Assembler::fmov(FPRegister fd, double imm) {
  if (fd.Is64Bits() && IsImmFP64(imm)) {
    Emit(FMOV_d_imm | Rd(fd) | ImmFP64(imm));
  } else if (fd.Is32Bits() && IsImmFP32(imm)) {
    Emit(FMOV_s_imm | Rd(fd) | ImmFP32(static_cast<float>(imm)));
  } else if ((imm == 0.0) && (copysign(1.0, imm) == 1.0)) {
    Register zr = AppropriateZeroRegFor(fd);
    fmov(fd, zr);
  } else {
    ldr(fd, imm);
  }
}


void Assembler::fmov(Register rd, FPRegister fn) {
  assert(rd.size() == fn.size());
  FPIntegerConvertOp op = rd.Is32Bits() ? FMOV_ws : FMOV_xd;
  Emit(op | Rd(rd) | Rn(fn));
}


void Assembler::fmov(FPRegister fd, Register rn) {
  assert(fd.size() == rn.size());
  FPIntegerConvertOp op = fd.Is32Bits() ? FMOV_sw : FMOV_dx;
  Emit(op | Rd(fd) | Rn(rn));
}


void Assembler::fmov(FPRegister fd, FPRegister fn) {
  assert(fd.size() == fn.size());
  Emit(FPType(fd) | FMOV | Rd(fd) | Rn(fn));
}


void Assembler::fadd(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FADD);
}


void Assembler::fsub(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FSUB);
}


void Assembler::fmul(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FMUL);
}


void Assembler::fmsub(const FPRegister& fd,
                      const FPRegister& fn,
                      const FPRegister& fm,
                      const FPRegister& fa) {
  FPDataProcessing3Source(fd, fn, fm, fa, fd.Is32Bits() ? FMSUB_s : FMSUB_d);
}


void Assembler::fdiv(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FDIV);
}


void Assembler::fmax(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FMAX);
}


void Assembler::fmin(const FPRegister& fd,
                     const FPRegister& fn,
                     const FPRegister& fm) {
  FPDataProcessing2Source(fd, fn, fm, FMIN);
}


void Assembler::fabs(const FPRegister& fd,
                     const FPRegister& fn) {
  assert(fd.SizeInBits() == fn.SizeInBits());
  FPDataProcessing1Source(fd, fn, FABS);
}


void Assembler::fneg(const FPRegister& fd,
                     const FPRegister& fn) {
  assert(fd.SizeInBits() == fn.SizeInBits());
  FPDataProcessing1Source(fd, fn, FNEG);
}


void Assembler::fsqrt(const FPRegister& fd,
                      const FPRegister& fn) {
  assert(fd.SizeInBits() == fn.SizeInBits());
  FPDataProcessing1Source(fd, fn, FSQRT);
}


void Assembler::frintn(const FPRegister& fd,
                       const FPRegister& fn) {
  assert(fd.SizeInBits() == fn.SizeInBits());
  FPDataProcessing1Source(fd, fn, FRINTN);
}


void Assembler::frintz(const FPRegister& fd,
                       const FPRegister& fn) {
  assert(fd.SizeInBits() == fn.SizeInBits());
  FPDataProcessing1Source(fd, fn, FRINTZ);
}


void Assembler::fcmp(const FPRegister& fn,
                     const FPRegister& fm) {
  assert(fn.size() == fm.size());
  Emit(FPType(fn) | FCMP | Rm(fm) | Rn(fn));
}


void Assembler::fcmp(const FPRegister& fn,
                     double value) {
  USE(value);
  // Although the fcmp instruction can strictly only take an immediate value of
  // +0.0, we don't need to check for -0.0 because the sign of 0.0 doesn't
  // affect the result of the comparison.
  assert(value == 0.0);
  Emit(FPType(fn) | FCMP_zero | Rn(fn));
}


void Assembler::fccmp(const FPRegister& fn,
                      const FPRegister& fm,
                      StatusFlags nzcv,
                      Condition cond) {
  assert(fn.size() == fm.size());
  Emit(FPType(fn) | FCCMP | Rm(fm) | Cond(cond) | Rn(fn) | Nzcv(nzcv));
}


void Assembler::fcsel(const FPRegister& fd,
                      const FPRegister& fn,
                      const FPRegister& fm,
                      Condition cond) {
  assert(fd.size() == fn.size());
  assert(fd.size() == fm.size());
  Emit(FPType(fd) | FCSEL | Rm(fm) | Cond(cond) | Rn(fn) | Rd(fd));
}


void Assembler::FPConvertToInt(const Register& rd,
                               const FPRegister& fn,
                               FPIntegerConvertOp op) {
  Emit(SF(rd) | FPType(fn) | op | Rn(fn) | Rd(rd));
}


void Assembler::fcvt(const FPRegister& fd,
                     const FPRegister& fn) {
  if (fd.Is64Bits()) {
    // Convert float to double.
    assert(fn.Is32Bits());
    FPDataProcessing1Source(fd, fn, FCVT_ds);
  } else {
    // Convert double to float.
    assert(fn.Is64Bits());
    FPDataProcessing1Source(fd, fn, FCVT_sd);
  }
}


void Assembler::fcvtmu(const Register& rd, const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTMU);
}


void Assembler::fcvtms(const Register& rd, const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTMS);
}


void Assembler::fcvtnu(const Register& rd,
                       const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTNU);
}


void Assembler::fcvtns(const Register& rd,
                       const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTNS);
}


void Assembler::fcvtzu(const Register& rd,
                       const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTZU);
}


void Assembler::fcvtzs(const Register& rd,
                       const FPRegister& fn) {
  FPConvertToInt(rd, fn, FCVTZS);
}


void Assembler::scvtf(const FPRegister& fd,
                      const Register& rn,
                      unsigned fbits) {
  if (fbits == 0) {
    Emit(SF(rn) | FPType(fd) | SCVTF | Rn(rn) | Rd(fd));
  } else {
    Emit(SF(rn) | FPType(fd) | SCVTF_fixed | FPScale(64 - fbits) | Rn(rn) |
         Rd(fd));
  }
}


void Assembler::ucvtf(const FPRegister& fd,
                      const Register& rn,
                      unsigned fbits) {
  if (fbits == 0) {
    Emit(SF(rn) | FPType(fd) | UCVTF | Rn(rn) | Rd(fd));
  } else {
    Emit(SF(rn) | FPType(fd) | UCVTF_fixed | FPScale(64 - fbits) | Rn(rn) |
         Rd(fd));
  }
}


// Note:
// Below, a difference in case for the same letter indicates a
// negated bit.
// If b is 1, then B is 0.
Instr Assembler::ImmFP32(float imm) {
  assert(IsImmFP32(imm));
  // bits: aBbb.bbbc.defg.h000.0000.0000.0000.0000
  uint32_t bits = float_to_rawbits(imm);
  // bit7: a000.0000
  uint32_t bit7 = ((bits >> 31) & 0x1) << 7;
  // bit6: 0b00.0000
  uint32_t bit6 = ((bits >> 29) & 0x1) << 6;
  // bit5_to_0: 00cd.efgh
  uint32_t bit5_to_0 = (bits >> 19) & 0x3f;

  return (bit7 | bit6 | bit5_to_0) << ImmFP_offset;
}


Instr Assembler::ImmFP64(double imm) {
  assert(IsImmFP64(imm));
  // bits: aBbb.bbbb.bbcd.efgh.0000.0000.0000.0000
  //       0000.0000.0000.0000.0000.0000.0000.0000
  uint64_t bits = double_to_rawbits(imm);
  // bit7: a000.0000
  uint32_t bit7 = ((bits >> 63) & 0x1) << 7;
  // bit6: 0b00.0000
  uint32_t bit6 = ((bits >> 61) & 0x1) << 6;
  // bit5_to_0: 00cd.efgh
  uint32_t bit5_to_0 = (bits >> 48) & 0x3f;

  return (bit7 | bit6 | bit5_to_0) << ImmFP_offset;
}


// Code generation helpers.
void Assembler::MoveWide(const Register& rd,
                         uint64_t imm,
                         int shift,
                         MoveWideImmediateOp mov_op) {
  if (shift >= 0) {
    // Explicit shift specified.
    assert((shift == 0) || (shift == 16) || (shift == 32) || (shift == 48));
    assert(rd.Is64Bits() || (shift == 0) || (shift == 16));
    shift /= 16;
  } else {
    // Calculate a new immediate and shift combination to encode the immediate
    // argument.
    shift = 0;
    if ((imm & ~0xffffUL) == 0) {
      // Nothing to do.
    } else if ((imm & ~(0xffffUL << 16)) == 0) {
      imm >>= 16;
      shift = 1;
    } else if ((imm & ~(0xffffUL << 32)) == 0) {
      assert(rd.Is64Bits());
      imm >>= 32;
      shift = 2;
    } else if ((imm & ~(0xffffUL << 48)) == 0) {
      assert(rd.Is64Bits());
      imm >>= 48;
      shift = 3;
    }
  }

  assert(is_uint16(imm));

  Emit(SF(rd) | MoveWideImmediateFixed | mov_op |
       Rd(rd) | ImmMoveWide(imm) | ShiftMoveWide(shift));
}


void Assembler::AddSub(const Register& rd,
                       const Register& rn,
                       const Operand& operand,
                       FlagsUpdate S,
                       AddSubOp op) {
  assert(rd.size() == rn.size());
  if (operand.IsImmediate()) {
    int64_t immediate = operand.immediate();
    assert(IsImmAddSub(immediate));
    Instr dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
    Emit(SF(rd) | AddSubImmediateFixed | op | Flags(S) |
         ImmAddSub(immediate) | dest_reg | RnSP(rn));
  } else if (operand.IsShiftedRegister()) {
    assert(operand.reg().size() == rd.size());
    assert(operand.shift() != ROR);

    // For instructions of the form:
    //   add/sub   wsp, <Wn>, <Wm> [, LSL #0-3 ]
    //   add/sub   <Wd>, wsp, <Wm> [, LSL #0-3 ]
    //   add/sub   wsp, wsp, <Wm> [, LSL #0-3 ]
    //   adds/subs <Wd>, wsp, <Wm> [, LSL #0-3 ]
    // or their 64-bit register equivalents, convert the operand from shifted to
    // extended register mode, and emit an add/sub extended instruction.
    if (rn.IsSP() || rd.IsSP()) {
      assert(!(rd.IsSP() && (S == SetFlags)));
      DataProcExtendedRegister(rd, rn, operand.ToExtendedRegister(), S,
                               AddSubExtendedFixed | op);
    } else {
      DataProcShiftedRegister(rd, rn, operand, S, AddSubShiftedFixed | op);
    }
  } else {
    assert(operand.IsExtendedRegister());
    DataProcExtendedRegister(rd, rn, operand, S, AddSubExtendedFixed | op);
  }
}


void Assembler::AddSubWithCarry(const Register& rd,
                                const Register& rn,
                                const Operand& operand,
                                FlagsUpdate S,
                                AddSubWithCarryOp op) {
  assert(rd.size() == rn.size());
  assert(rd.size() == operand.reg().size());
  assert(operand.IsShiftedRegister() && (operand.shift_amount() == 0));
  Emit(SF(rd) | op | Flags(S) | Rm(operand.reg()) | Rn(rn) | Rd(rd));
}


void Assembler::hlt(int code) {
  assert(is_uint16(code));
  Emit(HLT | ImmException(code));
}


void Assembler::brk(int code) {
  assert(is_uint16(code));
  Emit(BRK | ImmException(code));
}


void Assembler::Logical(const Register& rd,
                        const Register& rn,
                        const Operand& operand,
                        LogicalOp op) {
  assert(rd.size() == rn.size());
  if (operand.IsImmediate()) {
    int64_t immediate = operand.immediate();
    unsigned reg_size = rd.size();

    assert(immediate != 0);
    assert(immediate != -1);
    assert(rd.Is64Bits() || is_uint32(immediate));

    // If the operation is NOT, invert the operation and immediate.
    if ((op & NOT) == NOT) {
      op = static_cast<LogicalOp>(op & ~NOT);
      immediate = rd.Is64Bits() ? ~immediate : (~immediate & kWRegMask);
    }

    unsigned n, imm_s, imm_r;
    if (IsImmLogical(immediate, reg_size, &n, &imm_s, &imm_r)) {
      // Immediate can be encoded in the instruction.
      LogicalImmediate(rd, rn, n, imm_s, imm_r, op);
    } else {
      // This case is handled in the macro assembler.
      not_reached();
    }
  } else {
    assert(operand.IsShiftedRegister());
    assert(operand.reg().size() == rd.size());
    Instr dp_op = static_cast<Instr>(op | LogicalShiftedFixed);
    DataProcShiftedRegister(rd, rn, operand, LeaveFlags, dp_op);
  }
}


void Assembler::LogicalImmediate(const Register& rd,
                                 const Register& rn,
                                 unsigned n,
                                 unsigned imm_s,
                                 unsigned imm_r,
                                 LogicalOp op) {
  unsigned reg_size = rd.size();
  Instr dest_reg = (op == ANDS) ? Rd(rd) : RdSP(rd);
  Emit(SF(rd) | LogicalImmediateFixed | op | BitN(n, reg_size) |
       ImmSetBits(imm_s, reg_size) | ImmRotate(imm_r, reg_size) | dest_reg |
       Rn(rn));
}


void Assembler::ConditionalCompare(const Register& rn,
                                   const Operand& operand,
                                   StatusFlags nzcv,
                                   Condition cond,
                                   ConditionalCompareOp op) {
  Instr ccmpop;
  if (operand.IsImmediate()) {
    int64_t immediate = operand.immediate();
    assert(IsImmConditionalCompare(immediate));
    ccmpop = ConditionalCompareImmediateFixed | op | ImmCondCmp(immediate);
  } else {
    assert(operand.IsShiftedRegister() && (operand.shift_amount() == 0));
    ccmpop = ConditionalCompareRegisterFixed | op | Rm(operand.reg());
  }
  Emit(SF(rn) | ccmpop | Cond(cond) | Rn(rn) | Nzcv(nzcv));
}


void Assembler::DataProcessing1Source(const Register& rd,
                                      const Register& rn,
                                      DataProcessing1SourceOp op) {
  assert(rd.size() == rn.size());
  Emit(SF(rn) | op | Rn(rn) | Rd(rd));
}


void Assembler::FPDataProcessing1Source(const FPRegister& fd,
                                        const FPRegister& fn,
                                        FPDataProcessing1SourceOp op) {
  Emit(FPType(fn) | op | Rn(fn) | Rd(fd));
}


void Assembler::FPDataProcessing2Source(const FPRegister& fd,
                                        const FPRegister& fn,
                                        const FPRegister& fm,
                                        FPDataProcessing2SourceOp op) {
  assert(fd.size() == fn.size());
  assert(fd.size() == fm.size());
  Emit(FPType(fd) | op | Rm(fm) | Rn(fn) | Rd(fd));
}


void Assembler::FPDataProcessing3Source(const FPRegister& fd,
                                        const FPRegister& fn,
                                        const FPRegister& fm,
                                        const FPRegister& fa,
                                        FPDataProcessing3SourceOp op) {
  assert(AreSameSizeAndType(fd, fn, fm, fa));
  Emit(FPType(fd) | op | Rm(fm) | Rn(fn) | Rd(fd) | Ra(fa));
}


void Assembler::EmitShift(const Register& rd,
                          const Register& rn,
                          Shift shift,
                          unsigned shift_amount) {
  switch (shift) {
    case LSL:
      lsl(rd, rn, shift_amount);
      break;
    case LSR:
      lsr(rd, rn, shift_amount);
      break;
    case ASR:
      asr(rd, rn, shift_amount);
      break;
    case ROR:
      ror(rd, rn, shift_amount);
      break;
    default:
      not_reached();
  }
}


void Assembler::EmitExtendShift(const Register& rd,
                                const Register& rn,
                                Extend extend,
                                unsigned left_shift) {
  assert(rd.size() >= rn.size());
  unsigned reg_size = rd.size();
  // Use the correct size of register.
  Register rn_ = Register(rn.code(), rd.size());
  // Bits extracted are high_bit:0.
  unsigned high_bit = (8 << (extend & 0x3)) - 1;
  // Number of bits left in the result that are not introduced by the shift.
  unsigned non_shift_bits = (reg_size - left_shift) & (reg_size - 1);

  if ((non_shift_bits > high_bit) || (non_shift_bits == 0)) {
    switch (extend) {
      case UXTB:
      case UXTH:
      case UXTW: ubfm(rd, rn_, non_shift_bits, high_bit); break;
      case SXTB:
      case SXTH:
      case SXTW: sbfm(rd, rn_, non_shift_bits, high_bit); break;
      case UXTX:
      case SXTX: {
        assert(rn.size() == kXRegSize);
        // Nothing to extend. Just shift.
        lsl(rd, rn_, left_shift);
        break;
      }
      default: not_reached();
    }
  } else {
    // No need to extend as the extended bits would be shifted away.
    lsl(rd, rn_, left_shift);
  }
}


void Assembler::DataProcShiftedRegister(const Register& rd,
                                        const Register& rn,
                                        const Operand& operand,
                                        FlagsUpdate S,
                                        Instr op) {
  assert(operand.IsShiftedRegister());
  assert(rn.Is64Bits() || (rn.Is32Bits() && is_uint5(operand.shift_amount())));
  Emit(SF(rd) | op | Flags(S) |
       ShiftDP(operand.shift()) | ImmDPShift(operand.shift_amount()) |
       Rm(operand.reg()) | Rn(rn) | Rd(rd));
}


void Assembler::DataProcExtendedRegister(const Register& rd,
                                         const Register& rn,
                                         const Operand& operand,
                                         FlagsUpdate S,
                                         Instr op) {
  Instr dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
  Emit(SF(rd) | op | Flags(S) | Rm(operand.reg()) |
       ExtendMode(operand.extend()) | ImmExtendShift(operand.shift_amount()) |
       dest_reg | RnSP(rn));
}


bool Assembler::IsImmAddSub(int64_t immediate) {
  return is_uint12(immediate) ||
         (is_uint12(immediate >> 12) && ((immediate & 0xfff) == 0));
}

void Assembler::LoadStore(const CPURegister& rt,
                          const MemOperand& addr,
                          LoadStoreOp op) {
  Instr memop = op | Rt(rt) | RnSP(addr.base());
  ptrdiff_t offset = addr.offset();

  if (addr.IsImmediateOffset()) {
    LSDataSize size = CalcLSDataSize(op);
    if (IsImmLSScaled(offset, size)) {
      // Use the scaled addressing mode.
      Emit(LoadStoreUnsignedOffsetFixed | memop |
           ImmLSUnsigned(offset >> size));
    } else if (IsImmLSUnscaled(offset)) {
      // Use the unscaled addressing mode.
      Emit(LoadStoreUnscaledOffsetFixed | memop | ImmLS(offset));
    } else {
      // This case is handled in the macro assembler.
      not_reached();
    }
  } else if (addr.IsRegisterOffset()) {
    Extend ext = addr.extend();
    Shift shift = addr.shift();
    unsigned shift_amount = addr.shift_amount();

    // LSL is encoded in the option field as UXTX.
    if (shift == LSL) {
      ext = UXTX;
    }

    // Shifts are encoded in one bit, indicating a left shift by the memory
    // access size.
    assert((shift_amount == 0) ||
           (shift_amount == static_cast<unsigned>(CalcLSDataSize(op))));
    Emit(LoadStoreRegisterOffsetFixed | memop | Rm(addr.regoffset()) |
         ExtendMode(ext) | ImmShiftLS((shift_amount > 0) ? 1 : 0));
  } else {
    if (IsImmLSUnscaled(offset)) {
      if (addr.IsPreIndex()) {
        Emit(LoadStorePreIndexFixed | memop | ImmLS(offset));
      } else {
        assert(addr.IsPostIndex());
        Emit(LoadStorePostIndexFixed | memop | ImmLS(offset));
      }
    } else {
      // This case is handled in the macro assembler.
      not_reached();
    }
  }
}


bool Assembler::IsImmLSUnscaled(ptrdiff_t offset) {
  return is_int9(offset);
}


bool Assembler::IsImmLSScaled(ptrdiff_t offset, LSDataSize size) {
  bool offset_is_size_multiple = (((offset >> size) << size) == offset);
  return offset_is_size_multiple && is_uint12(offset >> size);
}


void Assembler::LoadLiteral(const CPURegister& rt,
                            uint64_t imm,
                            LoadLiteralOp op) {
  assert(is_int32(imm) || is_uint32(imm) || (rt.Is64Bits()));

  BlockLiteralPoolScope scope(this);
  RecordLiteral(imm, rt.SizeInBytes());
  Emit(op | ImmLLiteral(0) | Rt(rt));
}


// Test if a given value can be encoded in the immediate field of a logical
// instruction.
// If it can be encoded, the function returns true, and values pointed to by n,
// imm_s and imm_r are updated with immediates encoded in the format required
// by the corresponding fields in the logical instruction.
// If it can not be encoded, the function returns false, and the values pointed
// to by n, imm_s and imm_r are undefined.
bool Assembler::IsImmLogical(uint64_t value,
                             unsigned width,
                             unsigned* n,
                             unsigned* imm_s,
                             unsigned* imm_r) {
  assert((n != nullptr) && (imm_s != nullptr) && (imm_r != nullptr));
  assert((width == kWRegSize) || (width == kXRegSize));

  // Logical immediates are encoded using parameters n, imm_s and imm_r using
  // the following table:
  //
  //  N   imms    immr    size        S             R
  //  1  ssssss  rrrrrr    64    UInt(ssssss)  UInt(rrrrrr)
  //  0  0sssss  xrrrrr    32    UInt(sssss)   UInt(rrrrr)
  //  0  10ssss  xxrrrr    16    UInt(ssss)    UInt(rrrr)
  //  0  110sss  xxxrrr     8    UInt(sss)     UInt(rrr)
  //  0  1110ss  xxxxrr     4    UInt(ss)      UInt(rr)
  //  0  11110s  xxxxxr     2    UInt(s)       UInt(r)
  // (s bits must not be all set)
  //
  // A pattern is constructed of size bits, where the least significant S+1
  // bits are set. The pattern is rotated right by R, and repeated across a
  // 32 or 64-bit value, depending on destination register width.
  //
  // To test if an arbitrary immediate can be encoded using this scheme, an
  // iterative algorithm is used.
  //
  // TODO: This code does not consider using X/W register overlap to support
  // 64-bit immediates where the top 32-bits are zero, and the bottom 32-bits
  // are an encodable logical immediate.

  // 1. If the value has all set or all clear bits, it can't be encoded.
  if ((value == 0) || (value == 0xffffffffffffffffUL) ||
      ((width == kWRegSize) && (value == 0xffffffff))) {
    return false;
  }

  unsigned lead_zero = CountLeadingZeros(value, width);
  unsigned lead_one = CountLeadingZeros(~value, width);
  unsigned trail_zero = CountTrailingZeros(value, width);
  unsigned trail_one = CountTrailingZeros(~value, width);
  unsigned set_bits = CountSetBits(value, width);

  // The fixed bits in the immediate s field.
  // If width == 64 (X reg), start at 0xFFFFFF80.
  // If width == 32 (W reg), start at 0xFFFFFFC0, as the iteration for 64-bit
  // widths won't be executed.
  int imm_s_fixed = (width == kXRegSize) ? -128 : -64;
  int imm_s_mask = 0x3F;

  for (;;) {
    // 2. If the value is two bits wide, it can be encoded.
    if (width == 2) {
      *n = 0;
      *imm_s = 0x3C;
      *imm_r = (value & 3) - 1;
      return true;
    }

    *n = (width == 64) ? 1 : 0;
    *imm_s = ((imm_s_fixed | (set_bits - 1)) & imm_s_mask);
    if ((lead_zero + set_bits) == width) {
      *imm_r = 0;
    } else {
      *imm_r = (lead_zero > 0) ? (width - trail_zero) : lead_one;
    }

    // 3. If the sum of leading zeros, trailing zeros and set bits is equal to
    //    the bit width of the value, it can be encoded.
    if (lead_zero + trail_zero + set_bits == width) {
      return true;
    }

    // 4. If the sum of leading ones, trailing ones and unset bits in the
    //    value is equal to the bit width of the value, it can be encoded.
    if (lead_one + trail_one + (width - set_bits) == width) {
      return true;
    }

    // 5. If the most-significant half of the bitwise value is equal to the
    //    least-significant half, return to step 2 using the least-significant
    //    half of the value.
    uint64_t mask = (1UL << (width >> 1)) - 1;
    if ((value & mask) == ((value >> (width >> 1)) & mask)) {
      width >>= 1;
      set_bits >>= 1;
      imm_s_fixed >>= 1;
      continue;
    }

    // 6. Otherwise, the value can't be encoded.
    return false;
  }
}

bool Assembler::IsImmConditionalCompare(int64_t immediate) {
  return is_uint5(immediate);
}


bool Assembler::IsImmFP32(float imm) {
  // Valid values will have the form:
  // aBbb.bbbc.defg.h000.0000.0000.0000.0000
  uint32_t bits = float_to_rawbits(imm);
  // bits[19..0] are cleared.
  if ((bits & 0x7ffff) != 0) {
    return false;
  }

  // bits[29..25] are all set or all cleared.
  uint32_t b_pattern = (bits >> 16) & 0x3e00;
  if (b_pattern != 0 && b_pattern != 0x3e00) {
    return false;
  }

  // bit[30] and bit[29] are opposite.
  if (((bits ^ (bits << 1)) & 0x40000000) == 0) {
    return false;
  }

  return true;
}


bool Assembler::IsImmFP64(double imm) {
  // Valid values will have the form:
  // aBbb.bbbb.bbcd.efgh.0000.0000.0000.0000
  // 0000.0000.0000.0000.0000.0000.0000.0000
  uint64_t bits = double_to_rawbits(imm);
  // bits[47..0] are cleared.
  if ((bits & 0xffffffffffffL) != 0) {
    return false;
  }

  // bits[61..54] are all set or all cleared.
  uint32_t b_pattern = (bits >> 48) & 0x3fc0;
  if (b_pattern != 0 && b_pattern != 0x3fc0) {
    return false;
  }

  // bit[62] and bit[61] are opposite.
  if (((bits ^ (bits << 1)) & 0x4000000000000000L) == 0) {
    return false;
  }

  return true;
}


LoadStoreOp Assembler::LoadOpFor(const CPURegister& rt) {
  assert(rt.IsValid());
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? LDR_x : LDR_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? LDR_d : LDR_s;
  }
}


LoadStorePairOp Assembler::LoadPairOpFor(const CPURegister& rt,
    const CPURegister& rt2) {
  assert(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? LDP_x : LDP_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? LDP_d : LDP_s;
  }
}


LoadStoreOp Assembler::StoreOpFor(const CPURegister& rt) {
  assert(rt.IsValid());
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STR_x : STR_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? STR_d : STR_s;
  }
}


LoadStorePairOp Assembler::StorePairOpFor(const CPURegister& rt,
    const CPURegister& rt2) {
  assert(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STP_x : STP_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? STP_d : STP_s;
  }
}


LoadStorePairNonTemporalOp Assembler::LoadPairNonTemporalOpFor(
    const CPURegister& rt, const CPURegister& rt2) {
  assert(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? LDNP_x : LDNP_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? LDNP_d : LDNP_s;
  }
}


LoadStorePairNonTemporalOp Assembler::StorePairNonTemporalOpFor(
    const CPURegister& rt, const CPURegister& rt2) {
  assert(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STNP_x : STNP_w;
  } else {
    assert(rt.IsFPRegister());
    return rt.Is64Bits() ? STNP_d : STNP_s;
  }
}


void Assembler::RecordLiteral(int64_t imm, unsigned size) {
  literals_.push_front(new Literal(cb_.frontier(), imm, size));
}


// Check if a literal pool should be emitted. Currently a literal is emitted
// when:
//  * the distance to the first literal load handled by this pool is greater
//    than the recommended distance and the literal pool can be emitted without
//    generating a jump over it.
//  * the distance to the first literal load handled by this pool is greater
//    than twice the recommended distance.
// TODO: refine this heuristic using real world data.
void Assembler::CheckLiteralPool(LiteralPoolEmitOption option) {
  if (IsLiteralPoolBlocked()) {
    // Literal pool emission is forbidden, no point in doing further checks.
    return;
  }

  if (literals_.empty()) {
    // No literal pool to emit.
    next_literal_pool_check_ += kLiteralPoolCheckInterval;
    return;
  }

  intptr_t distance = cb_.frontier() - literals_.back()->pc_;
  if ((distance < kRecommendedLiteralPoolRange) ||
      ((option == JumpRequired) &&
       (distance < (2 * kRecommendedLiteralPoolRange)))) {
    // We prefer not to have to jump over the literal pool.
    next_literal_pool_check_ += kLiteralPoolCheckInterval;
    return;
  }

  EmitLiteralPool(option);
}


void Assembler::EmitLiteralPool(LiteralPoolEmitOption option) {
  // Prevent recursive calls while emitting the literal pool.
  BlockLiteralPoolScope scope(this);

  Label marker;
  Label start_of_pool;
  Label end_of_pool;

  if (option == JumpRequired) {
    b(&end_of_pool);
  }

  // Leave space for a literal pool marker. This is populated later, once the
  // size of the pool is known.
  bind(&marker);
  nop();

  // Now populate the literal pool.
  bind(&start_of_pool);
  std::list<Literal*>::iterator it;
  for (it = literals_.begin(); it != literals_.end(); it++) {
    // Update the load-literal instruction to point to this pool entry.
    auto load_literal = Instruction::Cast((*it)->pc_);
    load_literal->SetImmLLiteral(Instruction::Cast(cb_.frontier()));
    // Copy the data into the pool.
    uint64_t value= (*it)->value_;
    unsigned size = (*it)->size_;
    assert((size == kXRegSizeInBytes) || (size == kWRegSizeInBytes));
    assert(cb_.canEmit(size));
    cb_.bytes(size, reinterpret_cast<const uint8_t*>(&value));
    delete *it;
  }
  literals_.clear();
  bind(&end_of_pool);

  // The pool size should always be a multiple of four bytes because that is the
  // scaling applied by the LDR(literal) instruction, even for X-register loads.
  assert((SizeOfCodeGeneratedSince(&start_of_pool) % 4) == 0);
  uint64_t pool_size = SizeOfCodeGeneratedSince(&start_of_pool) / 4;

  // Literal pool marker indicating the size in words of the literal pool.
  // We use a literal load to the zero register, the offset indicating the
  // size in words. This instruction can encode a large enough offset to span
  // the entire pool at its maximum size.
  Instr marker_instruction = LDR_x_lit | ImmLLiteral(pool_size) | Rt(xzr);
  memcpy(marker.target(), &marker_instruction, kInstructionSize);

  next_literal_pool_check_ = cb_.frontier() + kLiteralPoolCheckInterval;
}


// Return the size in bytes, required by the literal pool entries. This does
// not include any marker or branch over the literal pool itself.
size_t Assembler::LiteralPoolSize() {
  size_t size = 0;

  std::list<Literal*>::iterator it;
  for (it = literals_.begin(); it != literals_.end(); it++) {
    size += (*it)->size_;
  }

  return size;
}


bool AreAliased(const CPURegister& reg1, const CPURegister& reg2,
                const CPURegister& reg3, const CPURegister& reg4,
                const CPURegister& reg5, const CPURegister& reg6,
                const CPURegister& reg7, const CPURegister& reg8) {
  int number_of_valid_regs = 0;
  int number_of_valid_fpregs = 0;

  RegList unique_regs = 0;
  RegList unique_fpregs = 0;

  const CPURegister regs[] = {reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8};

  for (unsigned i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
    if (regs[i].IsRegister()) {
      number_of_valid_regs++;
      unique_regs |= regs[i].Bit();
    } else if (regs[i].IsFPRegister()) {
      number_of_valid_fpregs++;
      unique_fpregs |= regs[i].Bit();
    } else {
      assert(!regs[i].IsValid());
    }
  }

  int number_of_unique_regs =
    CountSetBits(unique_regs, sizeof(unique_regs) * 8);
  int number_of_unique_fpregs =
    CountSetBits(unique_fpregs, sizeof(unique_fpregs) * 8);

  assert(number_of_valid_regs >= number_of_unique_regs);
  assert(number_of_valid_fpregs >= number_of_unique_fpregs);

  return (number_of_valid_regs != number_of_unique_regs) ||
         (number_of_valid_fpregs != number_of_unique_fpregs);
}


bool AreSameSizeAndType(const CPURegister& reg1, const CPURegister& reg2,
                        const CPURegister& reg3, const CPURegister& reg4,
                        const CPURegister& reg5, const CPURegister& reg6,
                        const CPURegister& reg7, const CPURegister& reg8) {
  assert(reg1.IsValid());
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


}  // namespace vixl
