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


#include "assembler-aarch64.h"

#include <cmath>

#include "macro-assembler-aarch64.h"

namespace vixl {
namespace aarch64 {

RawLiteral::RawLiteral(size_t size,
                       LiteralPool* literal_pool,
                       DeletionPolicy deletion_policy)
    : size_(size),
      offset_(0),
      low64_(0),
      high64_(0),
      literal_pool_(literal_pool),
      deletion_policy_(deletion_policy) {
  VIXL_ASSERT((deletion_policy == kManuallyDeleted) || (literal_pool_ != NULL));
  if (deletion_policy == kDeletedOnPoolDestruction) {
    literal_pool_->DeleteOnDestruction(this);
  }
}


void Assembler::Reset() { GetBuffer()->Reset(); }


void Assembler::bind(Label* label) {
  BindToOffset(label, GetBuffer()->GetCursorOffset());
}


void Assembler::BindToOffset(Label* label, ptrdiff_t offset) {
  VIXL_ASSERT((offset >= 0) && (offset <= GetBuffer()->GetCursorOffset()));
  VIXL_ASSERT(offset % kInstructionSize == 0);

  label->Bind(offset);

  for (Label::LabelLinksIterator it(label); !it.Done(); it.Advance()) {
    Instruction* link =
        GetBuffer()->GetOffsetAddress<Instruction*>(*it.Current());
    link->SetImmPCOffsetTarget(GetLabelAddress<Instruction*>(label));
  }
  label->ClearAllLinks();
}


// A common implementation for the LinkAndGet<Type>OffsetTo helpers.
//
// The offset is calculated by aligning the PC and label addresses down to a
// multiple of 1 << element_shift, then calculating the (scaled) offset between
// them. This matches the semantics of adrp, for example.
template <int element_shift>
ptrdiff_t Assembler::LinkAndGetOffsetTo(Label* label) {
  VIXL_STATIC_ASSERT(element_shift < (sizeof(ptrdiff_t) * 8));

  if (label->IsBound()) {
    uintptr_t pc_offset = GetCursorAddress<uintptr_t>() >> element_shift;
    uintptr_t label_offset = GetLabelAddress<uintptr_t>(label) >> element_shift;
    return label_offset - pc_offset;
  } else {
    label->AddLink(GetBuffer()->GetCursorOffset());
    return 0;
  }
}


ptrdiff_t Assembler::LinkAndGetByteOffsetTo(Label* label) {
  return LinkAndGetOffsetTo<0>(label);
}


ptrdiff_t Assembler::LinkAndGetInstructionOffsetTo(Label* label) {
  return LinkAndGetOffsetTo<kInstructionSizeLog2>(label);
}


ptrdiff_t Assembler::LinkAndGetPageOffsetTo(Label* label) {
  return LinkAndGetOffsetTo<kPageSizeLog2>(label);
}


void Assembler::place(RawLiteral* literal) {
  VIXL_ASSERT(!literal->IsPlaced());

  // Patch instructions using this literal.
  if (literal->IsUsed()) {
    Instruction* target = GetCursorAddress<Instruction*>();
    ptrdiff_t offset = literal->GetLastUse();
    bool done;
    do {
      Instruction* ldr = GetBuffer()->GetOffsetAddress<Instruction*>(offset);
      VIXL_ASSERT(ldr->IsLoadLiteral());

      ptrdiff_t imm19 = ldr->GetImmLLiteral();
      VIXL_ASSERT(imm19 <= 0);
      done = (imm19 == 0);
      offset += imm19 * kLiteralEntrySize;

      ldr->SetImmLLiteral(target);
    } while (!done);
  }

  // "bind" the literal.
  literal->SetOffset(GetCursorOffset());
  // Copy the data into the pool.
  switch (literal->GetSize()) {
    case kSRegSizeInBytes:
      dc32(literal->GetRawValue32());
      break;
    case kDRegSizeInBytes:
      dc64(literal->GetRawValue64());
      break;
    default:
      VIXL_ASSERT(literal->GetSize() == kQRegSizeInBytes);
      dc64(literal->GetRawValue128Low64());
      dc64(literal->GetRawValue128High64());
  }

  literal->literal_pool_ = NULL;
}


ptrdiff_t Assembler::LinkAndGetWordOffsetTo(RawLiteral* literal) {
  VIXL_ASSERT(IsWordAligned(GetCursorOffset()));

  bool register_first_use =
      (literal->GetLiteralPool() != NULL) && !literal->IsUsed();

  if (literal->IsPlaced()) {
    // The literal is "behind", the offset will be negative.
    VIXL_ASSERT((literal->GetOffset() - GetCursorOffset()) <= 0);
    return (literal->GetOffset() - GetCursorOffset()) >> kLiteralEntrySizeLog2;
  }

  ptrdiff_t offset = 0;
  // Link all uses together.
  if (literal->IsUsed()) {
    offset =
        (literal->GetLastUse() - GetCursorOffset()) >> kLiteralEntrySizeLog2;
  }
  literal->SetLastUse(GetCursorOffset());

  if (register_first_use) {
    literal->GetLiteralPool()->AddEntry(literal);
  }

  return offset;
}


// Code generation.
void Assembler::br(const Register& xn) {
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BR | Rn(xn));
}


void Assembler::blr(const Register& xn) {
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BLR | Rn(xn));
}


void Assembler::ret(const Register& xn) {
  VIXL_ASSERT(xn.Is64Bits());
  Emit(RET | Rn(xn));
}


void Assembler::braaz(const Register& xn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BRAAZ | Rn(xn) | Rd_mask);
}

void Assembler::brabz(const Register& xn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BRABZ | Rn(xn) | Rd_mask);
}

void Assembler::blraaz(const Register& xn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BLRAAZ | Rn(xn) | Rd_mask);
}

void Assembler::blrabz(const Register& xn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits());
  Emit(BLRABZ | Rn(xn) | Rd_mask);
}

void Assembler::retaa() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(RETAA | Rn_mask | Rd_mask);
}

void Assembler::retab() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(RETAB | Rn_mask | Rd_mask);
}

// The Arm ARM names the register Xm but encodes it in the Xd bitfield.
void Assembler::braa(const Register& xn, const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits() && xm.Is64Bits());
  Emit(BRAA | Rn(xn) | RdSP(xm));
}

void Assembler::brab(const Register& xn, const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits() && xm.Is64Bits());
  Emit(BRAB | Rn(xn) | RdSP(xm));
}

void Assembler::blraa(const Register& xn, const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits() && xm.Is64Bits());
  Emit(BLRAA | Rn(xn) | RdSP(xm));
}

void Assembler::blrab(const Register& xn, const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xn.Is64Bits() && xm.Is64Bits());
  Emit(BLRAB | Rn(xn) | RdSP(xm));
}


void Assembler::b(int64_t imm26) { Emit(B | ImmUncondBranch(imm26)); }


void Assembler::b(int64_t imm19, Condition cond) {
  Emit(B_cond | ImmCondBranch(imm19) | cond);
}


void Assembler::b(Label* label) {
  int64_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(UncondBranchType, offset));
  b(static_cast<int>(offset));
}


void Assembler::b(Label* label, Condition cond) {
  int64_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(CondBranchType, offset));
  b(static_cast<int>(offset), cond);
}


void Assembler::bl(int64_t imm26) { Emit(BL | ImmUncondBranch(imm26)); }


void Assembler::bl(Label* label) {
  int64_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(UncondBranchType, offset));
  bl(static_cast<int>(offset));
}


void Assembler::cbz(const Register& rt, int64_t imm19) {
  Emit(SF(rt) | CBZ | ImmCmpBranch(imm19) | Rt(rt));
}


void Assembler::cbz(const Register& rt, Label* label) {
  int64_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(CompareBranchType, offset));
  cbz(rt, static_cast<int>(offset));
}


void Assembler::cbnz(const Register& rt, int64_t imm19) {
  Emit(SF(rt) | CBNZ | ImmCmpBranch(imm19) | Rt(rt));
}


void Assembler::cbnz(const Register& rt, Label* label) {
  int64_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(CompareBranchType, offset));
  cbnz(rt, static_cast<int>(offset));
}


void Assembler::NEONTable(const VRegister& vd,
                          const VRegister& vn,
                          const VRegister& vm,
                          NEONTableOp op) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.Is16B() || vd.Is8B());
  VIXL_ASSERT(vn.Is16B());
  VIXL_ASSERT(AreSameFormat(vd, vm));
  Emit(op | (vd.IsQ() ? NEON_Q : 0) | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::tbl(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONTable(vd, vn, vm, NEON_TBL_1v);
}


void Assembler::tbl(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vm) {
  USE(vn2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2));
  VIXL_ASSERT(AreConsecutive(vn, vn2));
  NEONTable(vd, vn, vm, NEON_TBL_2v);
}


void Assembler::tbl(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vn3,
                    const VRegister& vm) {
  USE(vn2, vn3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2, vn3));
  VIXL_ASSERT(AreConsecutive(vn, vn2, vn3));
  NEONTable(vd, vn, vm, NEON_TBL_3v);
}


void Assembler::tbl(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vn3,
                    const VRegister& vn4,
                    const VRegister& vm) {
  USE(vn2, vn3, vn4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2, vn3, vn4));
  VIXL_ASSERT(AreConsecutive(vn, vn2, vn3, vn4));
  NEONTable(vd, vn, vm, NEON_TBL_4v);
}


void Assembler::tbx(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONTable(vd, vn, vm, NEON_TBX_1v);
}


void Assembler::tbx(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vm) {
  USE(vn2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2));
  VIXL_ASSERT(AreConsecutive(vn, vn2));
  NEONTable(vd, vn, vm, NEON_TBX_2v);
}


void Assembler::tbx(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vn3,
                    const VRegister& vm) {
  USE(vn2, vn3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2, vn3));
  VIXL_ASSERT(AreConsecutive(vn, vn2, vn3));
  NEONTable(vd, vn, vm, NEON_TBX_3v);
}


void Assembler::tbx(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vn2,
                    const VRegister& vn3,
                    const VRegister& vn4,
                    const VRegister& vm) {
  USE(vn2, vn3, vn4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vn2, vn3, vn4));
  VIXL_ASSERT(AreConsecutive(vn, vn2, vn3, vn4));
  NEONTable(vd, vn, vm, NEON_TBX_4v);
}


void Assembler::tbz(const Register& rt, unsigned bit_pos, int64_t imm14) {
  VIXL_ASSERT(rt.Is64Bits() || (rt.Is32Bits() && (bit_pos < kWRegSize)));
  Emit(TBZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
}


void Assembler::tbz(const Register& rt, unsigned bit_pos, Label* label) {
  ptrdiff_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(TestBranchType, offset));
  tbz(rt, bit_pos, static_cast<int>(offset));
}


void Assembler::tbnz(const Register& rt, unsigned bit_pos, int64_t imm14) {
  VIXL_ASSERT(rt.Is64Bits() || (rt.Is32Bits() && (bit_pos < kWRegSize)));
  Emit(TBNZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
}


void Assembler::tbnz(const Register& rt, unsigned bit_pos, Label* label) {
  ptrdiff_t offset = LinkAndGetInstructionOffsetTo(label);
  VIXL_ASSERT(Instruction::IsValidImmPCOffset(TestBranchType, offset));
  tbnz(rt, bit_pos, static_cast<int>(offset));
}


void Assembler::adr(const Register& xd, int64_t imm21) {
  VIXL_ASSERT(xd.Is64Bits());
  Emit(ADR | ImmPCRelAddress(imm21) | Rd(xd));
}


void Assembler::adr(const Register& xd, Label* label) {
  adr(xd, static_cast<int>(LinkAndGetByteOffsetTo(label)));
}


void Assembler::adrp(const Register& xd, int64_t imm21) {
  VIXL_ASSERT(xd.Is64Bits());
  Emit(ADRP | ImmPCRelAddress(imm21) | Rd(xd));
}


void Assembler::adrp(const Register& xd, Label* label) {
  VIXL_ASSERT(AllowPageOffsetDependentCode());
  adrp(xd, static_cast<int>(LinkAndGetPageOffsetTo(label)));
}


void Assembler::add(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  AddSub(rd, rn, operand, LeaveFlags, ADD);
}


void Assembler::adds(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  AddSub(rd, rn, operand, SetFlags, ADD);
}


void Assembler::cmn(const Register& rn, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rn);
  adds(zr, rn, operand);
}


void Assembler::sub(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  AddSub(rd, rn, operand, LeaveFlags, SUB);
}


void Assembler::subs(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  AddSub(rd, rn, operand, SetFlags, SUB);
}


void Assembler::cmp(const Register& rn, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rn);
  subs(zr, rn, operand);
}


void Assembler::neg(const Register& rd, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rd);
  sub(rd, zr, operand);
}


void Assembler::negs(const Register& rd, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rd);
  subs(rd, zr, operand);
}


void Assembler::adc(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  AddSubWithCarry(rd, rn, operand, LeaveFlags, ADC);
}


void Assembler::adcs(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  AddSubWithCarry(rd, rn, operand, SetFlags, ADC);
}


void Assembler::sbc(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  AddSubWithCarry(rd, rn, operand, LeaveFlags, SBC);
}


void Assembler::sbcs(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  AddSubWithCarry(rd, rn, operand, SetFlags, SBC);
}


void Assembler::rmif(const Register& xn, unsigned rotation, StatusFlags flags) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFlagM));
  VIXL_ASSERT(xn.Is64Bits());
  Emit(RMIF | Rn(xn) | ImmRMIFRotation(rotation) | Nzcv(flags));
}


void Assembler::setf8(const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFlagM));
  Emit(SETF8 | Rn(rn));
}


void Assembler::setf16(const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFlagM));
  Emit(SETF16 | Rn(rn));
}


void Assembler::ngc(const Register& rd, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rd);
  sbc(rd, zr, operand);
}


void Assembler::ngcs(const Register& rd, const Operand& operand) {
  Register zr = AppropriateZeroRegFor(rd);
  sbcs(rd, zr, operand);
}


// Logical instructions.
void Assembler::and_(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  Logical(rd, rn, operand, AND);
}


void Assembler::ands(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  Logical(rd, rn, operand, ANDS);
}


void Assembler::tst(const Register& rn, const Operand& operand) {
  ands(AppropriateZeroRegFor(rn), rn, operand);
}


void Assembler::bic(const Register& rd,
                    const Register& rn,
                    const Operand& operand) {
  Logical(rd, rn, operand, BIC);
}


void Assembler::bics(const Register& rd,
                     const Register& rn,
                     const Operand& operand) {
  Logical(rd, rn, operand, BICS);
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
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | LSLV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::lsrv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | LSRV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::asrv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | ASRV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::rorv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | RORV | Rm(rm) | Rn(rn) | Rd(rd));
}


// Bitfield operations.
void Assembler::bfm(const Register& rd,
                    const Register& rn,
                    unsigned immr,
                    unsigned imms) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | BFM | N | ImmR(immr, rd.GetSizeInBits()) |
       ImmS(imms, rn.GetSizeInBits()) | Rn(rn) | Rd(rd));
}


void Assembler::sbfm(const Register& rd,
                     const Register& rn,
                     unsigned immr,
                     unsigned imms) {
  VIXL_ASSERT(rd.Is64Bits() || rn.Is32Bits());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | SBFM | N | ImmR(immr, rd.GetSizeInBits()) |
       ImmS(imms, rn.GetSizeInBits()) | Rn(rn) | Rd(rd));
}


void Assembler::ubfm(const Register& rd,
                     const Register& rn,
                     unsigned immr,
                     unsigned imms) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | UBFM | N | ImmR(immr, rd.GetSizeInBits()) |
       ImmS(imms, rn.GetSizeInBits()) | Rn(rn) | Rd(rd));
}


void Assembler::extr(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     unsigned lsb) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Instr N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
  Emit(SF(rd) | EXTR | N | Rm(rm) | ImmS(lsb, rn.GetSizeInBits()) | Rn(rn) |
       Rd(rd));
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


void Assembler::cset(const Register& rd, Condition cond) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  Register zr = AppropriateZeroRegFor(rd);
  csinc(rd, zr, zr, InvertCondition(cond));
}


void Assembler::csetm(const Register& rd, Condition cond) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  Register zr = AppropriateZeroRegFor(rd);
  csinv(rd, zr, zr, InvertCondition(cond));
}


void Assembler::cinc(const Register& rd, const Register& rn, Condition cond) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  csinc(rd, rn, rn, InvertCondition(cond));
}


void Assembler::cinv(const Register& rd, const Register& rn, Condition cond) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  csinv(rd, rn, rn, InvertCondition(cond));
}


void Assembler::cneg(const Register& rd, const Register& rn, Condition cond) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  csneg(rd, rn, rn, InvertCondition(cond));
}


void Assembler::ConditionalSelect(const Register& rd,
                                  const Register& rn,
                                  const Register& rm,
                                  Condition cond,
                                  ConditionalSelectOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
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


void Assembler::crc32b(const Register& wd,
                       const Register& wn,
                       const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32B | Rn(wn) | Rd(wd));
}


void Assembler::crc32h(const Register& wd,
                       const Register& wn,
                       const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32H | Rn(wn) | Rd(wd));
}


void Assembler::crc32w(const Register& wd,
                       const Register& wn,
                       const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32W | Rn(wn) | Rd(wd));
}


void Assembler::crc32x(const Register& wd,
                       const Register& wn,
                       const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && xm.Is64Bits());
  Emit(SF(xm) | Rm(xm) | CRC32X | Rn(wn) | Rd(wd));
}


void Assembler::crc32cb(const Register& wd,
                        const Register& wn,
                        const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32CB | Rn(wn) | Rd(wd));
}


void Assembler::crc32ch(const Register& wd,
                        const Register& wn,
                        const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32CH | Rn(wn) | Rd(wd));
}


void Assembler::crc32cw(const Register& wd,
                        const Register& wn,
                        const Register& wm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && wm.Is32Bits());
  Emit(SF(wm) | Rm(wm) | CRC32CW | Rn(wn) | Rd(wd));
}


void Assembler::crc32cx(const Register& wd,
                        const Register& wn,
                        const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCRC32));
  VIXL_ASSERT(wd.Is32Bits() && wn.Is32Bits() && xm.Is64Bits());
  Emit(SF(xm) | Rm(xm) | CRC32CX | Rn(wn) | Rd(wd));
}


void Assembler::mul(const Register& rd,
                    const Register& rn,
                    const Register& rm) {
  VIXL_ASSERT(AreSameSizeAndType(rd, rn, rm));
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
  VIXL_ASSERT(AreSameSizeAndType(rd, rn, rm));
  DataProcessing3Source(rd, rn, rm, AppropriateZeroRegFor(rd), MSUB);
}


void Assembler::msub(const Register& rd,
                     const Register& rn,
                     const Register& rm,
                     const Register& ra) {
  DataProcessing3Source(rd, rn, rm, ra, MSUB);
}


void Assembler::umaddl(const Register& xd,
                       const Register& wn,
                       const Register& wm,
                       const Register& xa) {
  VIXL_ASSERT(xd.Is64Bits() && xa.Is64Bits());
  VIXL_ASSERT(wn.Is32Bits() && wm.Is32Bits());
  DataProcessing3Source(xd, wn, wm, xa, UMADDL_x);
}


void Assembler::smaddl(const Register& xd,
                       const Register& wn,
                       const Register& wm,
                       const Register& xa) {
  VIXL_ASSERT(xd.Is64Bits() && xa.Is64Bits());
  VIXL_ASSERT(wn.Is32Bits() && wm.Is32Bits());
  DataProcessing3Source(xd, wn, wm, xa, SMADDL_x);
}


void Assembler::umsubl(const Register& xd,
                       const Register& wn,
                       const Register& wm,
                       const Register& xa) {
  VIXL_ASSERT(xd.Is64Bits() && xa.Is64Bits());
  VIXL_ASSERT(wn.Is32Bits() && wm.Is32Bits());
  DataProcessing3Source(xd, wn, wm, xa, UMSUBL_x);
}


void Assembler::smsubl(const Register& xd,
                       const Register& wn,
                       const Register& wm,
                       const Register& xa) {
  VIXL_ASSERT(xd.Is64Bits() && xa.Is64Bits());
  VIXL_ASSERT(wn.Is32Bits() && wm.Is32Bits());
  DataProcessing3Source(xd, wn, wm, xa, SMSUBL_x);
}


void Assembler::smull(const Register& xd,
                      const Register& wn,
                      const Register& wm) {
  VIXL_ASSERT(xd.Is64Bits());
  VIXL_ASSERT(wn.Is32Bits() && wm.Is32Bits());
  DataProcessing3Source(xd, wn, wm, xzr, SMADDL_x);
}


void Assembler::sdiv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | SDIV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::smulh(const Register& xd,
                      const Register& xn,
                      const Register& xm) {
  VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits() && xm.Is64Bits());
  DataProcessing3Source(xd, xn, xm, xzr, SMULH_x);
}


void Assembler::umulh(const Register& xd,
                      const Register& xn,
                      const Register& xm) {
  VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits() && xm.Is64Bits());
  DataProcessing3Source(xd, xn, xm, xzr, UMULH_x);
}


void Assembler::udiv(const Register& rd,
                     const Register& rn,
                     const Register& rm) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == rm.GetSizeInBits());
  Emit(SF(rd) | UDIV | Rm(rm) | Rn(rn) | Rd(rd));
}


void Assembler::rbit(const Register& rd, const Register& rn) {
  DataProcessing1Source(rd, rn, RBIT);
}


void Assembler::rev16(const Register& rd, const Register& rn) {
  DataProcessing1Source(rd, rn, REV16);
}


void Assembler::rev32(const Register& xd, const Register& xn) {
  VIXL_ASSERT(xd.Is64Bits());
  DataProcessing1Source(xd, xn, REV);
}


void Assembler::rev(const Register& rd, const Register& rn) {
  DataProcessing1Source(rd, rn, rd.Is64Bits() ? REV_x : REV_w);
}


void Assembler::clz(const Register& rd, const Register& rn) {
  DataProcessing1Source(rd, rn, CLZ);
}


void Assembler::cls(const Register& rd, const Register& rn) {
  DataProcessing1Source(rd, rn, CLS);
}

#define PAUTH_VARIATIONS(V) \
  V(paci, PACI)             \
  V(pacd, PACD)             \
  V(auti, AUTI)             \
  V(autd, AUTD)

#define VIXL_DEFINE_ASM_FUNC(PRE, OP)                              \
  void Assembler::PRE##a(const Register& xd, const Register& xn) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));                      \
    VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits());                   \
    Emit(SF(xd) | OP##A | Rd(xd) | RnSP(xn));                      \
  }                                                                \
                                                                   \
  void Assembler::PRE##za(const Register& xd) {                    \
    VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));                      \
    VIXL_ASSERT(xd.Is64Bits());                                    \
    Emit(SF(xd) | OP##ZA | Rd(xd) | Rn(xzr));                      \
  }                                                                \
                                                                   \
  void Assembler::PRE##b(const Register& xd, const Register& xn) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));                      \
    VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits());                   \
    Emit(SF(xd) | OP##B | Rd(xd) | RnSP(xn));                      \
  }                                                                \
                                                                   \
  void Assembler::PRE##zb(const Register& xd) {                    \
    VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));                      \
    VIXL_ASSERT(xd.Is64Bits());                                    \
    Emit(SF(xd) | OP##ZB | Rd(xd) | Rn(xzr));                      \
  }

PAUTH_VARIATIONS(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

void Assembler::pacga(const Register& xd,
                      const Register& xn,
                      const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth, CPUFeatures::kPAuthGeneric));
  VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits() && xm.Is64Bits());
  Emit(SF(xd) | PACGA | Rd(xd) | Rn(xn) | RmSP(xm));
}

void Assembler::xpaci(const Register& xd) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xd.Is64Bits());
  Emit(SF(xd) | XPACI | Rd(xd) | Rn(xzr));
}

void Assembler::xpacd(const Register& xd) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  VIXL_ASSERT(xd.Is64Bits());
  Emit(SF(xd) | XPACD | Rd(xd) | Rn(xzr));
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


void Assembler::ldpsw(const Register& xt,
                      const Register& xt2,
                      const MemOperand& src) {
  VIXL_ASSERT(xt.Is64Bits() && xt2.Is64Bits());
  LoadStorePair(xt, xt2, src, LDPSW_x);
}


void Assembler::LoadStorePair(const CPURegister& rt,
                              const CPURegister& rt2,
                              const MemOperand& addr,
                              LoadStorePairOp op) {
  VIXL_ASSERT(CPUHas(rt, rt2));

  // 'rt' and 'rt2' can only be aliased for stores.
  VIXL_ASSERT(((op & LoadStorePairLBit) == 0) || !rt.Is(rt2));
  VIXL_ASSERT(AreSameSizeAndType(rt, rt2));
  VIXL_ASSERT(IsImmLSPair(addr.GetOffset(), CalcLSPairDataSize(op)));

  int offset = static_cast<int>(addr.GetOffset());
  Instr memop = op | Rt(rt) | Rt2(rt2) | RnSP(addr.GetBaseRegister()) |
                ImmLSPair(offset, CalcLSPairDataSize(op));

  Instr addrmodeop;
  if (addr.IsImmediateOffset()) {
    addrmodeop = LoadStorePairOffsetFixed;
  } else {
    if (addr.IsImmediatePreIndex()) {
      addrmodeop = LoadStorePairPreIndexFixed;
    } else {
      VIXL_ASSERT(addr.IsImmediatePostIndex());
      addrmodeop = LoadStorePairPostIndexFixed;
    }
  }

  Instr emitop = addrmodeop | memop;

  // Only X registers may be specified for ldpsw.
  VIXL_ASSERT(((emitop & LoadStorePairMask) != LDPSW_x) || rt.IsX());

  Emit(emitop);
}


void Assembler::ldnp(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& src) {
  LoadStorePairNonTemporal(rt, rt2, src, LoadPairNonTemporalOpFor(rt, rt2));
}


void Assembler::stnp(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& dst) {
  LoadStorePairNonTemporal(rt, rt2, dst, StorePairNonTemporalOpFor(rt, rt2));
}


void Assembler::LoadStorePairNonTemporal(const CPURegister& rt,
                                         const CPURegister& rt2,
                                         const MemOperand& addr,
                                         LoadStorePairNonTemporalOp op) {
  VIXL_ASSERT(CPUHas(rt, rt2));

  VIXL_ASSERT(!rt.Is(rt2));
  VIXL_ASSERT(AreSameSizeAndType(rt, rt2));
  VIXL_ASSERT(addr.IsImmediateOffset());

  unsigned size =
      CalcLSPairDataSize(static_cast<LoadStorePairOp>(op & LoadStorePairMask));
  VIXL_ASSERT(IsImmLSPair(addr.GetOffset(), size));
  int offset = static_cast<int>(addr.GetOffset());
  Emit(op | Rt(rt) | Rt2(rt2) | RnSP(addr.GetBaseRegister()) |
       ImmLSPair(offset, size));
}


// Memory instructions.
void Assembler::ldrb(const Register& rt,
                     const MemOperand& src,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, src, LDRB_w, option);
}


void Assembler::strb(const Register& rt,
                     const MemOperand& dst,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, dst, STRB_w, option);
}


void Assembler::ldrsb(const Register& rt,
                      const MemOperand& src,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, src, rt.Is64Bits() ? LDRSB_x : LDRSB_w, option);
}


void Assembler::ldrh(const Register& rt,
                     const MemOperand& src,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, src, LDRH_w, option);
}


void Assembler::strh(const Register& rt,
                     const MemOperand& dst,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, dst, STRH_w, option);
}


void Assembler::ldrsh(const Register& rt,
                      const MemOperand& src,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, src, rt.Is64Bits() ? LDRSH_x : LDRSH_w, option);
}


void Assembler::ldr(const CPURegister& rt,
                    const MemOperand& src,
                    LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, src, LoadOpFor(rt), option);
}


void Assembler::str(const CPURegister& rt,
                    const MemOperand& dst,
                    LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(rt, dst, StoreOpFor(rt), option);
}


void Assembler::ldrsw(const Register& xt,
                      const MemOperand& src,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  LoadStore(xt, src, LDRSW_x, option);
}


void Assembler::ldurb(const Register& rt,
                      const MemOperand& src,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, src, LDRB_w, option);
}


void Assembler::sturb(const Register& rt,
                      const MemOperand& dst,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, dst, STRB_w, option);
}


void Assembler::ldursb(const Register& rt,
                       const MemOperand& src,
                       LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, src, rt.Is64Bits() ? LDRSB_x : LDRSB_w, option);
}


void Assembler::ldurh(const Register& rt,
                      const MemOperand& src,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, src, LDRH_w, option);
}


void Assembler::sturh(const Register& rt,
                      const MemOperand& dst,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, dst, STRH_w, option);
}


void Assembler::ldursh(const Register& rt,
                       const MemOperand& src,
                       LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, src, rt.Is64Bits() ? LDRSH_x : LDRSH_w, option);
}


void Assembler::ldur(const CPURegister& rt,
                     const MemOperand& src,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, src, LoadOpFor(rt), option);
}


void Assembler::stur(const CPURegister& rt,
                     const MemOperand& dst,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(rt, dst, StoreOpFor(rt), option);
}


void Assembler::ldursw(const Register& xt,
                       const MemOperand& src,
                       LoadStoreScalingOption option) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  LoadStore(xt, src, LDRSW_x, option);
}


void Assembler::ldraa(const Register& xt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  LoadStorePAC(xt, src, LDRAA);
}


void Assembler::ldrab(const Register& xt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  LoadStorePAC(xt, src, LDRAB);
}


void Assembler::ldrsw(const Register& xt, RawLiteral* literal) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(literal->GetSize() == kWRegSizeInBytes);
  ldrsw(xt, static_cast<int>(LinkAndGetWordOffsetTo(literal)));
}


void Assembler::ldr(const CPURegister& rt, RawLiteral* literal) {
  VIXL_ASSERT(CPUHas(rt));
  VIXL_ASSERT(literal->GetSize() == static_cast<size_t>(rt.GetSizeInBytes()));
  ldr(rt, static_cast<int>(LinkAndGetWordOffsetTo(literal)));
}


void Assembler::ldrsw(const Register& rt, int64_t imm19) {
  Emit(LDRSW_x_lit | ImmLLiteral(imm19) | Rt(rt));
}


void Assembler::ldr(const CPURegister& rt, int64_t imm19) {
  VIXL_ASSERT(CPUHas(rt));
  LoadLiteralOp op = LoadLiteralOpFor(rt);
  Emit(op | ImmLLiteral(imm19) | Rt(rt));
}


void Assembler::prfm(int op, int64_t imm19) {
  Emit(PRFM_lit | ImmPrefetchOperation(op) | ImmLLiteral(imm19));
}

void Assembler::prfm(PrefetchOperation op, int64_t imm19) {
  // Passing unnamed values in 'op' is undefined behaviour in C++.
  VIXL_ASSERT(IsNamedPrefetchOperation(op));
  prfm(static_cast<int>(op), imm19);
}


// Exclusive-access instructions.
void Assembler::stxrb(const Register& rs,
                      const Register& rt,
                      const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STXRB_w | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stxrh(const Register& rs,
                      const Register& rt,
                      const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STXRH_w | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stxr(const Register& rs,
                     const Register& rt,
                     const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STXR_x : STXR_w;
  Emit(op | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::ldxrb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDXRB_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldxrh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDXRH_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldxr(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDXR_x : LDXR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::stxp(const Register& rs,
                     const Register& rt,
                     const Register& rt2,
                     const MemOperand& dst) {
  VIXL_ASSERT(rt.GetSizeInBits() == rt2.GetSizeInBits());
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STXP_x : STXP_w;
  Emit(op | Rs(rs) | Rt(rt) | Rt2(rt2) | RnSP(dst.GetBaseRegister()));
}


void Assembler::ldxp(const Register& rt,
                     const Register& rt2,
                     const MemOperand& src) {
  VIXL_ASSERT(rt.GetSizeInBits() == rt2.GetSizeInBits());
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDXP_x : LDXP_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2(rt2) | RnSP(src.GetBaseRegister()));
}


void Assembler::stlxrb(const Register& rs,
                       const Register& rt,
                       const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLXRB_w | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stlxrh(const Register& rs,
                       const Register& rt,
                       const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLXRH_w | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stlxr(const Register& rs,
                      const Register& rt,
                      const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STLXR_x : STLXR_w;
  Emit(op | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::ldaxrb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDAXRB_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldaxrh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDAXRH_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldaxr(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDAXR_x : LDAXR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::stlxp(const Register& rs,
                      const Register& rt,
                      const Register& rt2,
                      const MemOperand& dst) {
  VIXL_ASSERT(rt.GetSizeInBits() == rt2.GetSizeInBits());
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STLXP_x : STLXP_w;
  Emit(op | Rs(rs) | Rt(rt) | Rt2(rt2) | RnSP(dst.GetBaseRegister()));
}


void Assembler::ldaxp(const Register& rt,
                      const Register& rt2,
                      const MemOperand& src) {
  VIXL_ASSERT(rt.GetSizeInBits() == rt2.GetSizeInBits());
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDAXP_x : LDAXP_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2(rt2) | RnSP(src.GetBaseRegister()));
}


void Assembler::stlrb(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLRB_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}

void Assembler::stlurb(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(dst.IsImmediateOffset() && IsImmLSUnscaled(dst.GetOffset()));

  Instr base = RnSP(dst.GetBaseRegister());
  int64_t offset = dst.GetOffset();
  Emit(STLURB | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}


void Assembler::stlrh(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLRH_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}

void Assembler::stlurh(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(dst.IsImmediateOffset() && IsImmLSUnscaled(dst.GetOffset()));

  Instr base = RnSP(dst.GetBaseRegister());
  int64_t offset = dst.GetOffset();
  Emit(STLURH | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}


void Assembler::stlr(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STLR_x : STLR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}

void Assembler::stlur(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(dst.IsImmediateOffset() && IsImmLSUnscaled(dst.GetOffset()));

  Instr base = RnSP(dst.GetBaseRegister());
  int64_t offset = dst.GetOffset();
  Instr op = rt.Is64Bits() ? STLUR_x : STLUR_w;
  Emit(op | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}


void Assembler::ldarb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDARB_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldarh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDARH_w | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldar(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDAR_x : LDAR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::stllrb(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLLRB | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stllrh(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  Emit(STLLRH | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::stllr(const Register& rt, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(dst.IsImmediateOffset() && (dst.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? STLLR_x : STLLR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(dst.GetBaseRegister()));
}


void Assembler::ldlarb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDLARB | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldlarh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  Emit(LDLARH | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


void Assembler::ldlar(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kLORegions));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  LoadStoreExclusive op = rt.Is64Bits() ? LDLAR_x : LDLAR_w;
  Emit(op | Rs_mask | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister()));
}


// clang-format off
#define COMPARE_AND_SWAP_W_X_LIST(V) \
  V(cas,   CAS)                      \
  V(casa,  CASA)                     \
  V(casl,  CASL)                     \
  V(casal, CASAL)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                     \
  void Assembler::FN(const Register& rs,                                 \
                     const Register& rt,                                 \
                     const MemOperand& src) {                            \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                          \
    VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));      \
    VIXL_ASSERT(AreSameFormat(rs, rt));                                  \
    LoadStoreExclusive op = rt.Is64Bits() ? OP##_x : OP##_w;             \
    Emit(op | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister())); \
  }
COMPARE_AND_SWAP_W_X_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// clang-format off
#define COMPARE_AND_SWAP_W_LIST(V) \
  V(casb,   CASB)                  \
  V(casab,  CASAB)                 \
  V(caslb,  CASLB)                 \
  V(casalb, CASALB)                \
  V(cash,   CASH)                  \
  V(casah,  CASAH)                 \
  V(caslh,  CASLH)                 \
  V(casalh, CASALH)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                     \
  void Assembler::FN(const Register& rs,                                 \
                     const Register& rt,                                 \
                     const MemOperand& src) {                            \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                          \
    VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));      \
    Emit(OP | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister())); \
  }
COMPARE_AND_SWAP_W_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define COMPARE_AND_SWAP_PAIR_LIST(V) \
  V(casp,   CASP)                     \
  V(caspa,  CASPA)                    \
  V(caspl,  CASPL)                    \
  V(caspal, CASPAL)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                     \
  void Assembler::FN(const Register& rs,                                 \
                     const Register& rs1,                                \
                     const Register& rt,                                 \
                     const Register& rt1,                                \
                     const MemOperand& src) {                            \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                          \
    USE(rs1, rt1);                                                       \
    VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));      \
    VIXL_ASSERT(AreEven(rs, rt));                                        \
    VIXL_ASSERT(AreConsecutive(rs, rs1));                                \
    VIXL_ASSERT(AreConsecutive(rt, rt1));                                \
    VIXL_ASSERT(AreSameFormat(rs, rs1, rt, rt1));                        \
    LoadStoreExclusive op = rt.Is64Bits() ? OP##_x : OP##_w;             \
    Emit(op | Rs(rs) | Rt(rt) | Rt2_mask | RnSP(src.GetBaseRegister())); \
  }
COMPARE_AND_SWAP_PAIR_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// These macros generate all the variations of the atomic memory operations,
// e.g. ldadd, ldadda, ldaddb, staddl, etc.
// For a full list of the methods with comments, see the assembler header file.

// clang-format off
#define ATOMIC_MEMORY_SIMPLE_OPERATION_LIST(V, DEF) \
  V(DEF, add,  LDADD)                               \
  V(DEF, clr,  LDCLR)                               \
  V(DEF, eor,  LDEOR)                               \
  V(DEF, set,  LDSET)                               \
  V(DEF, smax, LDSMAX)                              \
  V(DEF, smin, LDSMIN)                              \
  V(DEF, umax, LDUMAX)                              \
  V(DEF, umin, LDUMIN)

#define ATOMIC_MEMORY_STORE_MODES(V, NAME, OP) \
  V(NAME,     OP##_x,   OP##_w)                \
  V(NAME##l,  OP##L_x,  OP##L_w)               \
  V(NAME##b,  OP##B,    OP##B)                 \
  V(NAME##lb, OP##LB,   OP##LB)                \
  V(NAME##h,  OP##H,    OP##H)                 \
  V(NAME##lh, OP##LH,   OP##LH)

#define ATOMIC_MEMORY_LOAD_MODES(V, NAME, OP) \
  ATOMIC_MEMORY_STORE_MODES(V, NAME, OP)      \
  V(NAME##a,   OP##A_x,  OP##A_w)             \
  V(NAME##al,  OP##AL_x, OP##AL_w)            \
  V(NAME##ab,  OP##AB,   OP##AB)              \
  V(NAME##alb, OP##ALB,  OP##ALB)             \
  V(NAME##ah,  OP##AH,   OP##AH)              \
  V(NAME##alh, OP##ALH,  OP##ALH)
// clang-format on

#define DEFINE_ASM_LOAD_FUNC(FN, OP_X, OP_W)                        \
  void Assembler::ld##FN(const Register& rs,                        \
                         const Register& rt,                        \
                         const MemOperand& src) {                   \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                     \
    VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0)); \
    AtomicMemoryOp op = rt.Is64Bits() ? OP_X : OP_W;                \
    Emit(op | Rs(rs) | Rt(rt) | RnSP(src.GetBaseRegister()));       \
  }
#define DEFINE_ASM_STORE_FUNC(FN, OP_X, OP_W)                         \
  void Assembler::st##FN(const Register& rs, const MemOperand& src) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                       \
    ld##FN(rs, AppropriateZeroRegFor(rs), src);                       \
  }

ATOMIC_MEMORY_SIMPLE_OPERATION_LIST(ATOMIC_MEMORY_LOAD_MODES,
                                    DEFINE_ASM_LOAD_FUNC)
ATOMIC_MEMORY_SIMPLE_OPERATION_LIST(ATOMIC_MEMORY_STORE_MODES,
                                    DEFINE_ASM_STORE_FUNC)

#define DEFINE_ASM_SWP_FUNC(FN, OP_X, OP_W)                         \
  void Assembler::FN(const Register& rs,                            \
                     const Register& rt,                            \
                     const MemOperand& src) {                       \
    VIXL_ASSERT(CPUHas(CPUFeatures::kAtomics));                     \
    VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0)); \
    AtomicMemoryOp op = rt.Is64Bits() ? OP_X : OP_W;                \
    Emit(op | Rs(rs) | Rt(rt) | RnSP(src.GetBaseRegister()));       \
  }

ATOMIC_MEMORY_LOAD_MODES(DEFINE_ASM_SWP_FUNC, swp, SWP)

#undef DEFINE_ASM_LOAD_FUNC
#undef DEFINE_ASM_STORE_FUNC
#undef DEFINE_ASM_SWP_FUNC


void Assembler::ldaprb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  AtomicMemoryOp op = LDAPRB;
  Emit(op | Rs(xzr) | Rt(rt) | RnSP(src.GetBaseRegister()));
}

void Assembler::ldapurb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  Emit(LDAPURB | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::ldapursb(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  Instr op = rt.Is64Bits() ? LDAPURSB_x : LDAPURSB_w;
  Emit(op | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::ldaprh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  AtomicMemoryOp op = LDAPRH;
  Emit(op | Rs(xzr) | Rt(rt) | RnSP(src.GetBaseRegister()));
}

void Assembler::ldapurh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  Emit(LDAPURH | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::ldapursh(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  LoadStoreRCpcUnscaledOffsetOp op = rt.Is64Bits() ? LDAPURSH_x : LDAPURSH_w;
  Emit(op | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::ldapr(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc));
  VIXL_ASSERT(src.IsImmediateOffset() && (src.GetOffset() == 0));
  AtomicMemoryOp op = rt.Is64Bits() ? LDAPR_x : LDAPR_w;
  Emit(op | Rs(xzr) | Rt(rt) | RnSP(src.GetBaseRegister()));
}

void Assembler::ldapur(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  LoadStoreRCpcUnscaledOffsetOp op = rt.Is64Bits() ? LDAPUR_x : LDAPUR_w;
  Emit(op | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::ldapursw(const Register& rt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm));
  VIXL_ASSERT(rt.Is64Bits());
  VIXL_ASSERT(src.IsImmediateOffset() && IsImmLSUnscaled(src.GetOffset()));

  Instr base = RnSP(src.GetBaseRegister());
  int64_t offset = src.GetOffset();
  Emit(LDAPURSW | Rt(rt) | base | ImmLS(static_cast<int>(offset)));
}

void Assembler::prfm(int op,
                     const MemOperand& address,
                     LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireUnscaledOffset);
  VIXL_ASSERT(option != PreferUnscaledOffset);
  Prefetch(op, address, option);
}

void Assembler::prfm(PrefetchOperation op,
                     const MemOperand& address,
                     LoadStoreScalingOption option) {
  // Passing unnamed values in 'op' is undefined behaviour in C++.
  VIXL_ASSERT(IsNamedPrefetchOperation(op));
  prfm(static_cast<int>(op), address, option);
}


void Assembler::prfum(int op,
                      const MemOperand& address,
                      LoadStoreScalingOption option) {
  VIXL_ASSERT(option != RequireScaledOffset);
  VIXL_ASSERT(option != PreferScaledOffset);
  Prefetch(op, address, option);
}

void Assembler::prfum(PrefetchOperation op,
                      const MemOperand& address,
                      LoadStoreScalingOption option) {
  // Passing unnamed values in 'op' is undefined behaviour in C++.
  VIXL_ASSERT(IsNamedPrefetchOperation(op));
  prfum(static_cast<int>(op), address, option);
}


void Assembler::prfm(int op, RawLiteral* literal) {
  prfm(op, static_cast<int>(LinkAndGetWordOffsetTo(literal)));
}

void Assembler::prfm(PrefetchOperation op, RawLiteral* literal) {
  // Passing unnamed values in 'op' is undefined behaviour in C++.
  VIXL_ASSERT(IsNamedPrefetchOperation(op));
  prfm(static_cast<int>(op), literal);
}


void Assembler::sys(int op1, int crn, int crm, int op2, const Register& xt) {
  VIXL_ASSERT(xt.Is64Bits());
  Emit(SYS | ImmSysOp1(op1) | CRn(crn) | CRm(crm) | ImmSysOp2(op2) | Rt(xt));
}


void Assembler::sys(int op, const Register& xt) {
  VIXL_ASSERT(xt.Is64Bits());
  Emit(SYS | SysOp(op) | Rt(xt));
}


void Assembler::sysl(int op, const Register& xt) {
  VIXL_ASSERT(xt.Is64Bits());
  Emit(SYSL | SysOp(op) | Rt(xt));
}


void Assembler::dc(DataCacheOp op, const Register& rt) {
  if (op == CVAP) VIXL_ASSERT(CPUHas(CPUFeatures::kDCPoP));
  if (op == CVADP) VIXL_ASSERT(CPUHas(CPUFeatures::kDCCVADP));
  sys(op, rt);
}


void Assembler::ic(InstructionCacheOp op, const Register& rt) {
  VIXL_ASSERT(op == IVAU);
  sys(op, rt);
}

void Assembler::gcspushm(const Register& rt) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kGCS));
  sys(GCSPUSHM, rt);
}

void Assembler::gcspopm(const Register& rt) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kGCS));
  sysl(GCSPOPM, rt);
}


void Assembler::gcsss1(const Register& rt) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kGCS));
  sys(GCSSS1, rt);
}


void Assembler::gcsss2(const Register& rt) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kGCS));
  sysl(GCSSS2, rt);
}


void Assembler::chkfeat(const Register& rd) {
  VIXL_ASSERT(rd.Is(x16));
  USE(rd);
  hint(CHKFEAT);
}


void Assembler::hint(SystemHint code) { hint(static_cast<int>(code)); }


void Assembler::hint(int imm7) {
  VIXL_ASSERT(IsUint7(imm7));
  Emit(HINT | ImmHint(imm7) | Rt(xzr));
}


// MTE.

void Assembler::addg(const Register& xd,
                     const Register& xn,
                     int offset,
                     int tag_offset) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  VIXL_ASSERT(IsMultiple(offset, kMTETagGranuleInBytes));

  Emit(0x91800000 | RdSP(xd) | RnSP(xn) |
       ImmUnsignedField<21, 16>(offset / kMTETagGranuleInBytes) |
       ImmUnsignedField<13, 10>(tag_offset));
}

void Assembler::gmi(const Register& xd,
                    const Register& xn,
                    const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));

  Emit(0x9ac01400 | Rd(xd) | RnSP(xn) | Rm(xm));
}

void Assembler::irg(const Register& xd,
                    const Register& xn,
                    const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));

  Emit(0x9ac01000 | RdSP(xd) | RnSP(xn) | Rm(xm));
}

void Assembler::ldg(const Register& xt, const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  VIXL_ASSERT(addr.IsImmediateOffset());
  int offset = static_cast<int>(addr.GetOffset());
  VIXL_ASSERT(IsMultiple(offset, kMTETagGranuleInBytes));

  Emit(0xd9600000 | Rt(xt) | RnSP(addr.GetBaseRegister()) |
       ImmField<20, 12>(offset / static_cast<int>(kMTETagGranuleInBytes)));
}

void Assembler::StoreTagHelper(const Register& xt,
                               const MemOperand& addr,
                               Instr op) {
  int offset = static_cast<int>(addr.GetOffset());
  VIXL_ASSERT(IsMultiple(offset, kMTETagGranuleInBytes));

  Instr addr_mode;
  if (addr.IsImmediateOffset()) {
    addr_mode = 2;
  } else if (addr.IsImmediatePreIndex()) {
    addr_mode = 3;
  } else {
    VIXL_ASSERT(addr.IsImmediatePostIndex());
    addr_mode = 1;
  }

  Emit(op | RdSP(xt) | RnSP(addr.GetBaseRegister()) | (addr_mode << 10) |
       ImmField<20, 12>(offset / static_cast<int>(kMTETagGranuleInBytes)));
}

void Assembler::st2g(const Register& xt, const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  StoreTagHelper(xt, addr, 0xd9a00000);
}

void Assembler::stg(const Register& xt, const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  StoreTagHelper(xt, addr, 0xd9200000);
}

void Assembler::stgp(const Register& xt1,
                     const Register& xt2,
                     const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  int offset = static_cast<int>(addr.GetOffset());
  VIXL_ASSERT(IsMultiple(offset, kMTETagGranuleInBytes));

  Instr addr_mode;
  if (addr.IsImmediateOffset()) {
    addr_mode = 2;
  } else if (addr.IsImmediatePreIndex()) {
    addr_mode = 3;
  } else {
    VIXL_ASSERT(addr.IsImmediatePostIndex());
    addr_mode = 1;
  }

  Emit(0x68000000 | RnSP(addr.GetBaseRegister()) | (addr_mode << 23) |
       ImmField<21, 15>(offset / static_cast<int>(kMTETagGranuleInBytes)) |
       Rt2(xt2) | Rt(xt1));
}

void Assembler::stz2g(const Register& xt, const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  StoreTagHelper(xt, addr, 0xd9e00000);
}

void Assembler::stzg(const Register& xt, const MemOperand& addr) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  StoreTagHelper(xt, addr, 0xd9600000);
}

void Assembler::subg(const Register& xd,
                     const Register& xn,
                     int offset,
                     int tag_offset) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));
  VIXL_ASSERT(IsMultiple(offset, kMTETagGranuleInBytes));

  Emit(0xd1800000 | RdSP(xd) | RnSP(xn) |
       ImmUnsignedField<21, 16>(offset / kMTETagGranuleInBytes) |
       ImmUnsignedField<13, 10>(tag_offset));
}

void Assembler::subp(const Register& xd,
                     const Register& xn,
                     const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));

  Emit(0x9ac00000 | Rd(xd) | RnSP(xn) | RmSP(xm));
}

void Assembler::subps(const Register& xd,
                      const Register& xn,
                      const Register& xm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMTE));

  Emit(0xbac00000 | Rd(xd) | RnSP(xn) | RmSP(xm));
}

void Assembler::cpye(const Register& rd,
                     const Register& rs,
                     const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d800400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyen(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d80c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyern(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d808400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyewn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d804400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfe(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19800400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfen(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1980c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfern(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19808400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfewn(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19804400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfm(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19400400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfmn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1940c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfmrn(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19408400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfmwn(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19404400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfp(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19000400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfpn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1900c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfprn(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19008400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyfpwn(const Register& rd,
                        const Register& rs,
                        const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x19004400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpym(const Register& rd,
                     const Register& rs,
                     const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d400400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpymn(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d40c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpymrn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d408400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpymwn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d404400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyp(const Register& rd,
                     const Register& rs,
                     const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d000400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpypn(const Register& rd,
                      const Register& rs,
                      const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d00c400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpyprn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d008400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::cpypwn(const Register& rd,
                       const Register& rs,
                       const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero() && !rs.IsZero());

  Emit(0x1d004400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::sete(const Register& rd,
                     const Register& rn,
                     const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c08400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::seten(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c0a400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setge(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc08400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setgen(const Register& rd,
                       const Register& rn,
                       const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc0a400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setgm(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc04400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setgmn(const Register& rd,
                       const Register& rn,
                       const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc06400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setgp(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc00400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setgpn(const Register& rd,
                       const Register& rn,
                       const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x1dc02400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setm(const Register& rd,
                     const Register& rn,
                     const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c04400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setmn(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c06400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setp(const Register& rd,
                     const Register& rn,
                     const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c00400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::setpn(const Register& rd,
                      const Register& rn,
                      const Register& rs) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kMOPS));
  VIXL_ASSERT(!AreAliased(rd, rn, rs));
  VIXL_ASSERT(!rd.IsZero() && !rn.IsZero());

  Emit(0x19c02400 | Rd(rd) | Rn(rn) | Rs(rs));
}

void Assembler::abs(const Register& rd, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCSSC));
  VIXL_ASSERT(rd.IsSameSizeAndType(rn));

  Emit(0x5ac02000 | SF(rd) | Rd(rd) | Rn(rn));
}

void Assembler::cnt(const Register& rd, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCSSC));
  VIXL_ASSERT(rd.IsSameSizeAndType(rn));

  Emit(0x5ac01c00 | SF(rd) | Rd(rd) | Rn(rn));
}

void Assembler::ctz(const Register& rd, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kCSSC));
  VIXL_ASSERT(rd.IsSameSizeAndType(rn));

  Emit(0x5ac01800 | SF(rd) | Rd(rd) | Rn(rn));
}

#define MINMAX(V)                        \
  V(smax, 0x11c00000, 0x1ac06000, true)  \
  V(smin, 0x11c80000, 0x1ac06800, true)  \
  V(umax, 0x11c40000, 0x1ac06400, false) \
  V(umin, 0x11cc0000, 0x1ac06c00, false)

#define VIXL_DEFINE_ASM_FUNC(FN, IMMOP, REGOP, SIGNED)                     \
  void Assembler::FN(const Register& rd,                                   \
                     const Register& rn,                                   \
                     const Operand& op) {                                  \
    VIXL_ASSERT(rd.IsSameSizeAndType(rn));                                 \
    Instr i = SF(rd) | Rd(rd) | Rn(rn);                                    \
    if (op.IsImmediate()) {                                                \
      int64_t imm = op.GetImmediate();                                     \
      i |= SIGNED ? ImmField<17, 10>(imm) : ImmUnsignedField<17, 10>(imm); \
      Emit(IMMOP | i);                                                     \
    } else {                                                               \
      VIXL_ASSERT(op.IsPlainRegister());                                   \
      VIXL_ASSERT(op.GetRegister().IsSameSizeAndType(rd));                 \
      Emit(REGOP | i | Rm(op.GetRegister()));                              \
    }                                                                      \
  }
MINMAX(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// NEON structure loads and stores.
Instr Assembler::LoadStoreStructAddrModeField(const MemOperand& addr) {
  Instr addr_field = RnSP(addr.GetBaseRegister());

  if (addr.IsPostIndex()) {
    VIXL_STATIC_ASSERT(NEONLoadStoreMultiStructPostIndex ==
                       static_cast<NEONLoadStoreMultiStructPostIndexOp>(
                           NEONLoadStoreSingleStructPostIndex));

    addr_field |= NEONLoadStoreMultiStructPostIndex;
    if (addr.GetOffset() == 0) {
      addr_field |= RmNot31(addr.GetRegisterOffset());
    } else {
      // The immediate post index addressing mode is indicated by rm = 31.
      // The immediate is implied by the number of vector registers used.
      addr_field |= (0x1f << Rm_offset);
    }
  } else {
    VIXL_ASSERT(addr.IsImmediateOffset() && (addr.GetOffset() == 0));
  }
  return addr_field;
}

void Assembler::LoadStoreStructVerify(const VRegister& vt,
                                      const MemOperand& addr,
                                      Instr op) {
#ifdef VIXL_DEBUG
  // Assert that addressing mode is either offset (with immediate 0), post
  // index by immediate of the size of the register list, or post index by a
  // value in a core register.
  VIXL_ASSERT(vt.HasSize() && vt.HasLaneSize());
  if (addr.IsImmediateOffset()) {
    VIXL_ASSERT(addr.GetOffset() == 0);
  } else {
    int offset = vt.GetSizeInBytes();
    switch (op) {
      case NEON_LD1_1v:
      case NEON_ST1_1v:
        offset *= 1;
        break;
      case NEONLoadStoreSingleStructLoad1:
      case NEONLoadStoreSingleStructStore1:
      case NEON_LD1R:
        offset = (offset / vt.GetLanes()) * 1;
        break;

      case NEON_LD1_2v:
      case NEON_ST1_2v:
      case NEON_LD2:
      case NEON_ST2:
        offset *= 2;
        break;
      case NEONLoadStoreSingleStructLoad2:
      case NEONLoadStoreSingleStructStore2:
      case NEON_LD2R:
        offset = (offset / vt.GetLanes()) * 2;
        break;

      case NEON_LD1_3v:
      case NEON_ST1_3v:
      case NEON_LD3:
      case NEON_ST3:
        offset *= 3;
        break;
      case NEONLoadStoreSingleStructLoad3:
      case NEONLoadStoreSingleStructStore3:
      case NEON_LD3R:
        offset = (offset / vt.GetLanes()) * 3;
        break;

      case NEON_LD1_4v:
      case NEON_ST1_4v:
      case NEON_LD4:
      case NEON_ST4:
        offset *= 4;
        break;
      case NEONLoadStoreSingleStructLoad4:
      case NEONLoadStoreSingleStructStore4:
      case NEON_LD4R:
        offset = (offset / vt.GetLanes()) * 4;
        break;
      default:
        VIXL_UNREACHABLE();
    }
    VIXL_ASSERT(!addr.GetRegisterOffset().Is(NoReg) ||
                addr.GetOffset() == offset);
  }
#else
  USE(vt, addr, op);
#endif
}

void Assembler::LoadStoreStruct(const VRegister& vt,
                                const MemOperand& addr,
                                NEONLoadStoreMultiStructOp op) {
  LoadStoreStructVerify(vt, addr, op);
  VIXL_ASSERT(vt.IsVector() || vt.Is1D());
  Emit(op | LoadStoreStructAddrModeField(addr) | LSVFormat(vt) | Rt(vt));
}


void Assembler::LoadStoreStructSingleAllLanes(const VRegister& vt,
                                              const MemOperand& addr,
                                              NEONLoadStoreSingleStructOp op) {
  LoadStoreStructVerify(vt, addr, op);
  Emit(op | LoadStoreStructAddrModeField(addr) | LSVFormat(vt) | Rt(vt));
}


void Assembler::ld1(const VRegister& vt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  LoadStoreStruct(vt, src, NEON_LD1_1v);
}


void Assembler::ld1(const VRegister& vt,
                    const VRegister& vt2,
                    const MemOperand& src) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStruct(vt, src, NEON_LD1_2v);
}


void Assembler::ld1(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const MemOperand& src) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStruct(vt, src, NEON_LD1_3v);
}


void Assembler::ld1(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    const MemOperand& src) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStruct(vt, src, NEON_LD1_4v);
}


void Assembler::ld2(const VRegister& vt,
                    const VRegister& vt2,
                    const MemOperand& src) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStruct(vt, src, NEON_LD2);
}


void Assembler::ld2(const VRegister& vt,
                    const VRegister& vt2,
                    int lane,
                    const MemOperand& src) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStructSingle(vt, lane, src, NEONLoadStoreSingleStructLoad2);
}


void Assembler::ld2r(const VRegister& vt,
                     const VRegister& vt2,
                     const MemOperand& src) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStructSingleAllLanes(vt, src, NEON_LD2R);
}


void Assembler::ld3(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const MemOperand& src) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStruct(vt, src, NEON_LD3);
}


void Assembler::ld3(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    int lane,
                    const MemOperand& src) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStructSingle(vt, lane, src, NEONLoadStoreSingleStructLoad3);
}


void Assembler::ld3r(const VRegister& vt,
                     const VRegister& vt2,
                     const VRegister& vt3,
                     const MemOperand& src) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStructSingleAllLanes(vt, src, NEON_LD3R);
}


void Assembler::ld4(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    const MemOperand& src) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStruct(vt, src, NEON_LD4);
}


void Assembler::ld4(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    int lane,
                    const MemOperand& src) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStructSingle(vt, lane, src, NEONLoadStoreSingleStructLoad4);
}


void Assembler::ld4r(const VRegister& vt,
                     const VRegister& vt2,
                     const VRegister& vt3,
                     const VRegister& vt4,
                     const MemOperand& src) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStructSingleAllLanes(vt, src, NEON_LD4R);
}


void Assembler::st1(const VRegister& vt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  LoadStoreStruct(vt, src, NEON_ST1_1v);
}


void Assembler::st1(const VRegister& vt,
                    const VRegister& vt2,
                    const MemOperand& src) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStruct(vt, src, NEON_ST1_2v);
}


void Assembler::st1(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const MemOperand& src) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStruct(vt, src, NEON_ST1_3v);
}


void Assembler::st1(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    const MemOperand& src) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStruct(vt, src, NEON_ST1_4v);
}


void Assembler::st2(const VRegister& vt,
                    const VRegister& vt2,
                    const MemOperand& dst) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStruct(vt, dst, NEON_ST2);
}


void Assembler::st2(const VRegister& vt,
                    const VRegister& vt2,
                    int lane,
                    const MemOperand& dst) {
  USE(vt2);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2));
  VIXL_ASSERT(AreConsecutive(vt, vt2));
  LoadStoreStructSingle(vt, lane, dst, NEONLoadStoreSingleStructStore2);
}


void Assembler::st3(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const MemOperand& dst) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStruct(vt, dst, NEON_ST3);
}


void Assembler::st3(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    int lane,
                    const MemOperand& dst) {
  USE(vt2, vt3);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3));
  LoadStoreStructSingle(vt, lane, dst, NEONLoadStoreSingleStructStore3);
}


void Assembler::st4(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    const MemOperand& dst) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStruct(vt, dst, NEON_ST4);
}


void Assembler::st4(const VRegister& vt,
                    const VRegister& vt2,
                    const VRegister& vt3,
                    const VRegister& vt4,
                    int lane,
                    const MemOperand& dst) {
  USE(vt2, vt3, vt4);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vt, vt2, vt3, vt4));
  VIXL_ASSERT(AreConsecutive(vt, vt2, vt3, vt4));
  LoadStoreStructSingle(vt, lane, dst, NEONLoadStoreSingleStructStore4);
}


void Assembler::LoadStoreStructSingle(const VRegister& vt,
                                      uint32_t lane,
                                      const MemOperand& addr,
                                      NEONLoadStoreSingleStructOp op) {
  LoadStoreStructVerify(vt, addr, op);

  // We support vt arguments of the form vt.VxT() or vt.T(), where x is the
  // number of lanes, and T is b, h, s or d.
  unsigned lane_size = vt.GetLaneSizeInBytes();
  VIXL_ASSERT(lane_size > 0);
  VIXL_ASSERT(lane < (kQRegSizeInBytes / lane_size));

  // Lane size is encoded in the opcode field. Lane index is encoded in the Q,
  // S and size fields.
  lane *= lane_size;
  if (lane_size == 8) lane++;

  Instr size = (lane << NEONLSSize_offset) & NEONLSSize_mask;
  Instr s = (lane << (NEONS_offset - 2)) & NEONS_mask;
  Instr q = (lane << (NEONQ_offset - 3)) & NEONQ_mask;

  Instr instr = op;
  switch (lane_size) {
    case 1:
      instr |= NEONLoadStoreSingle_b;
      break;
    case 2:
      instr |= NEONLoadStoreSingle_h;
      break;
    case 4:
      instr |= NEONLoadStoreSingle_s;
      break;
    default:
      VIXL_ASSERT(lane_size == 8);
      instr |= NEONLoadStoreSingle_d;
  }

  Emit(instr | LoadStoreStructAddrModeField(addr) | q | size | s | Rt(vt));
}


void Assembler::ld1(const VRegister& vt, int lane, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  LoadStoreStructSingle(vt, lane, src, NEONLoadStoreSingleStructLoad1);
}


void Assembler::ld1r(const VRegister& vt, const MemOperand& src) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  LoadStoreStructSingleAllLanes(vt, src, NEON_LD1R);
}


void Assembler::st1(const VRegister& vt, int lane, const MemOperand& dst) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  LoadStoreStructSingle(vt, lane, dst, NEONLoadStoreSingleStructStore1);
}

void Assembler::pmull(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vn.Is8B() && vd.Is8H()) || (vn.Is1D() && vd.Is1Q()));
  VIXL_ASSERT(CPUHas(CPUFeatures::kPmull1Q) || vd.Is8H());
  Emit(VFormat(vn) | NEON_PMULL | Rm(vm) | Rn(vn) | Rd(vd));
}

void Assembler::pmull2(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vn.Is16B() && vd.Is8H()) || (vn.Is2D() && vd.Is1Q()));
  VIXL_ASSERT(CPUHas(CPUFeatures::kPmull1Q) || vd.Is8H());
  Emit(VFormat(vn) | NEON_PMULL2 | Rm(vm) | Rn(vn) | Rd(vd));
}

void Assembler::NEON3DifferentL(const VRegister& vd,
                                const VRegister& vn,
                                const VRegister& vm,
                                NEON3DifferentOp vop) {
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vn.Is1H() && vd.Is1S()) || (vn.Is1S() && vd.Is1D()) ||
              (vn.Is8B() && vd.Is8H()) || (vn.Is4H() && vd.Is4S()) ||
              (vn.Is2S() && vd.Is2D()) || (vn.Is16B() && vd.Is8H()) ||
              (vn.Is8H() && vd.Is4S()) || (vn.Is4S() && vd.Is2D()));
  Instr format, op = vop;
  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
    format = SFormat(vn);
  } else {
    format = VFormat(vn);
  }
  Emit(format | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::NEON3DifferentW(const VRegister& vd,
                                const VRegister& vn,
                                const VRegister& vm,
                                NEON3DifferentOp vop) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT((vm.Is8B() && vd.Is8H()) || (vm.Is4H() && vd.Is4S()) ||
              (vm.Is2S() && vd.Is2D()) || (vm.Is16B() && vd.Is8H()) ||
              (vm.Is8H() && vd.Is4S()) || (vm.Is4S() && vd.Is2D()));
  Emit(VFormat(vm) | vop | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::NEON3DifferentHN(const VRegister& vd,
                                 const VRegister& vn,
                                 const VRegister& vm,
                                 NEON3DifferentOp vop) {
  VIXL_ASSERT(AreSameFormat(vm, vn));
  VIXL_ASSERT((vd.Is8B() && vn.Is8H()) || (vd.Is4H() && vn.Is4S()) ||
              (vd.Is2S() && vn.Is2D()) || (vd.Is16B() && vn.Is8H()) ||
              (vd.Is8H() && vn.Is4S()) || (vd.Is4S() && vn.Is2D()));
  Emit(VFormat(vd) | vop | Rm(vm) | Rn(vn) | Rd(vd));
}


// clang-format off
#define NEON_3DIFF_LONG_LIST(V) \
  V(saddl,  NEON_SADDL,  vn.IsVector() && vn.IsD())                            \
  V(saddl2, NEON_SADDL2, vn.IsVector() && vn.IsQ())                            \
  V(sabal,  NEON_SABAL,  vn.IsVector() && vn.IsD())                            \
  V(sabal2, NEON_SABAL2, vn.IsVector() && vn.IsQ())                            \
  V(uabal,  NEON_UABAL,  vn.IsVector() && vn.IsD())                            \
  V(uabal2, NEON_UABAL2, vn.IsVector() && vn.IsQ())                            \
  V(sabdl,  NEON_SABDL,  vn.IsVector() && vn.IsD())                            \
  V(sabdl2, NEON_SABDL2, vn.IsVector() && vn.IsQ())                            \
  V(uabdl,  NEON_UABDL,  vn.IsVector() && vn.IsD())                            \
  V(uabdl2, NEON_UABDL2, vn.IsVector() && vn.IsQ())                            \
  V(smlal,  NEON_SMLAL,  vn.IsVector() && vn.IsD())                            \
  V(smlal2, NEON_SMLAL2, vn.IsVector() && vn.IsQ())                            \
  V(umlal,  NEON_UMLAL,  vn.IsVector() && vn.IsD())                            \
  V(umlal2, NEON_UMLAL2, vn.IsVector() && vn.IsQ())                            \
  V(smlsl,  NEON_SMLSL,  vn.IsVector() && vn.IsD())                            \
  V(smlsl2, NEON_SMLSL2, vn.IsVector() && vn.IsQ())                            \
  V(umlsl,  NEON_UMLSL,  vn.IsVector() && vn.IsD())                            \
  V(umlsl2, NEON_UMLSL2, vn.IsVector() && vn.IsQ())                            \
  V(smull,  NEON_SMULL,  vn.IsVector() && vn.IsD())                            \
  V(smull2, NEON_SMULL2, vn.IsVector() && vn.IsQ())                            \
  V(umull,  NEON_UMULL,  vn.IsVector() && vn.IsD())                            \
  V(umull2, NEON_UMULL2, vn.IsVector() && vn.IsQ())                            \
  V(ssubl,  NEON_SSUBL,  vn.IsVector() && vn.IsD())                            \
  V(ssubl2, NEON_SSUBL2, vn.IsVector() && vn.IsQ())                            \
  V(uaddl,  NEON_UADDL,  vn.IsVector() && vn.IsD())                            \
  V(uaddl2, NEON_UADDL2, vn.IsVector() && vn.IsQ())                            \
  V(usubl,  NEON_USUBL,  vn.IsVector() && vn.IsD())                            \
  V(usubl2, NEON_USUBL2, vn.IsVector() && vn.IsQ())                            \
  V(sqdmlal,  NEON_SQDMLAL,  vn.Is1H() || vn.Is1S() || vn.Is4H() || vn.Is2S()) \
  V(sqdmlal2, NEON_SQDMLAL2, vn.Is1H() || vn.Is1S() || vn.Is8H() || vn.Is4S()) \
  V(sqdmlsl,  NEON_SQDMLSL,  vn.Is1H() || vn.Is1S() || vn.Is4H() || vn.Is2S()) \
  V(sqdmlsl2, NEON_SQDMLSL2, vn.Is1H() || vn.Is1S() || vn.Is8H() || vn.Is4S()) \
  V(sqdmull,  NEON_SQDMULL,  vn.Is1H() || vn.Is1S() || vn.Is4H() || vn.Is2S()) \
  V(sqdmull2, NEON_SQDMULL2, vn.Is1H() || vn.Is1S() || vn.Is8H() || vn.Is4S()) \
// clang-format on


#define VIXL_DEFINE_ASM_FUNC(FN, OP, AS)                   \
void Assembler::FN(const VRegister& vd,               \
                   const VRegister& vn,               \
                   const VRegister& vm) {             \
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));            \
  VIXL_ASSERT(AS);                                    \
  NEON3DifferentL(vd, vn, vm, OP);                    \
}
NEON_3DIFF_LONG_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// clang-format off
#define NEON_3DIFF_HN_LIST(V)         \
  V(addhn,   NEON_ADDHN,   vd.IsD())  \
  V(addhn2,  NEON_ADDHN2,  vd.IsQ())  \
  V(raddhn,  NEON_RADDHN,  vd.IsD())  \
  V(raddhn2, NEON_RADDHN2, vd.IsQ())  \
  V(subhn,   NEON_SUBHN,   vd.IsD())  \
  V(subhn2,  NEON_SUBHN2,  vd.IsQ())  \
  V(rsubhn,  NEON_RSUBHN,  vd.IsD())  \
  V(rsubhn2, NEON_RSUBHN2, vd.IsQ())
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP, AS)     \
  void Assembler::FN(const VRegister& vd,    \
                     const VRegister& vn,    \
                     const VRegister& vm) {  \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON)); \
    VIXL_ASSERT(AS);                         \
    NEON3DifferentHN(vd, vn, vm, OP);        \
  }
NEON_3DIFF_HN_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

void Assembler::uaddw(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsD());
  NEON3DifferentW(vd, vn, vm, NEON_UADDW);
}


void Assembler::uaddw2(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsQ());
  NEON3DifferentW(vd, vn, vm, NEON_UADDW2);
}


void Assembler::saddw(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsD());
  NEON3DifferentW(vd, vn, vm, NEON_SADDW);
}


void Assembler::saddw2(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsQ());
  NEON3DifferentW(vd, vn, vm, NEON_SADDW2);
}


void Assembler::usubw(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsD());
  NEON3DifferentW(vd, vn, vm, NEON_USUBW);
}


void Assembler::usubw2(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsQ());
  NEON3DifferentW(vd, vn, vm, NEON_USUBW2);
}


void Assembler::ssubw(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsD());
  NEON3DifferentW(vd, vn, vm, NEON_SSUBW);
}


void Assembler::ssubw2(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vm.IsQ());
  NEON3DifferentW(vd, vn, vm, NEON_SSUBW2);
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

void Assembler::xpaclri() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(XPACLRI);
}

void Assembler::pacia1716() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIA1716);
}

void Assembler::pacib1716() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIB1716);
}

void Assembler::autia1716() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIA1716);
}

void Assembler::autib1716() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIB1716);
}

void Assembler::paciaz() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIAZ);
}

void Assembler::pacibz() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIBZ);
}

void Assembler::autiaz() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIAZ);
}

void Assembler::autibz() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIBZ);
}

void Assembler::paciasp() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIASP);
}

void Assembler::pacibsp() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(PACIBSP);
}

void Assembler::autiasp() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIASP);
}

void Assembler::autibsp() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kPAuth));
  Emit(AUTIBSP);
}

void Assembler::bti(BranchTargetIdentifier id) {
  VIXL_ASSERT((id != EmitPACIASP) && (id != EmitPACIBSP));  // Not modes of Bti.
  VIXL_ASSERT(id != EmitBTI_none);  // Always generate an instruction.
  VIXL_ASSERT(CPUHas(CPUFeatures::kBTI));
  hint(static_cast<SystemHint>(id));
}

void Assembler::mvn(const Register& rd, const Operand& operand) {
  orn(rd, AppropriateZeroRegFor(rd), operand);
}


void Assembler::mrs(const Register& xt, SystemRegister sysreg) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(CPUHas(sysreg));
  Emit(MRS | ImmSystemRegister(sysreg) | Rt(xt));
}


void Assembler::msr(SystemRegister sysreg, const Register& xt) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(CPUHas(sysreg));
  Emit(MSR | Rt(xt) | ImmSystemRegister(sysreg));
}


void Assembler::cfinv() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFlagM));
  Emit(CFINV);
}


void Assembler::axflag() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kAXFlag));
  Emit(AXFLAG);
}


void Assembler::xaflag() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kAXFlag));
  Emit(XAFLAG);
}


void Assembler::clrex(int imm4) { Emit(CLREX | CRm(imm4)); }


void Assembler::dmb(BarrierDomain domain, BarrierType type) {
  Emit(DMB | ImmBarrierDomain(domain) | ImmBarrierType(type));
}


void Assembler::dsb(BarrierDomain domain, BarrierType type) {
  Emit(DSB | ImmBarrierDomain(domain) | ImmBarrierType(type));
}


void Assembler::isb() {
  Emit(ISB | ImmBarrierDomain(FullSystem) | ImmBarrierType(BarrierAll));
}

void Assembler::esb() {
  VIXL_ASSERT(CPUHas(CPUFeatures::kRAS));
  hint(ESB);
}

void Assembler::csdb() { hint(CSDB); }

void Assembler::fmov(const VRegister& vd, double imm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.IsScalar()) {
    VIXL_ASSERT(vd.Is1D());
    Emit(FMOV_d_imm | Rd(vd) | ImmFP64(imm));
  } else {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
    VIXL_ASSERT(vd.Is2D());
    Instr op = NEONModifiedImmediate_MOVI | NEONModifiedImmediateOpBit;
    Instr q = NEON_Q;
    uint32_t encoded_imm = FP64ToImm8(imm);
    Emit(q | op | ImmNEONabcdefgh(encoded_imm) | NEONCmode(0xf) | Rd(vd));
  }
}


void Assembler::fmov(const VRegister& vd, float imm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.IsScalar()) {
    VIXL_ASSERT(vd.Is1S());
    Emit(FMOV_s_imm | Rd(vd) | ImmFP32(imm));
  } else {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
    VIXL_ASSERT(vd.Is2S() || vd.Is4S());
    Instr op = NEONModifiedImmediate_MOVI;
    Instr q = vd.Is4S() ? NEON_Q : 0;
    uint32_t encoded_imm = FP32ToImm8(imm);
    Emit(q | op | ImmNEONabcdefgh(encoded_imm) | NEONCmode(0xf) | Rd(vd));
  }
}


void Assembler::fmov(const VRegister& vd, Float16 imm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.IsScalar()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    VIXL_ASSERT(vd.Is1H());
    Emit(FMOV_h_imm | Rd(vd) | ImmFP16(imm));
  } else {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kNEONHalf));
    VIXL_ASSERT(vd.Is4H() || vd.Is8H());
    Instr q = vd.Is8H() ? NEON_Q : 0;
    uint32_t encoded_imm = FP16ToImm8(imm);
    Emit(q | NEONModifiedImmediate_FMOV | ImmNEONabcdefgh(encoded_imm) |
         NEONCmode(0xf) | Rd(vd));
  }
}


void Assembler::fmov(const Register& rd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  VIXL_ASSERT((rd.GetSizeInBits() == vn.GetSizeInBits()) || vn.Is1H());
  FPIntegerConvertOp op;
  switch (vn.GetSizeInBits()) {
    case 16:
      VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
      op = rd.Is64Bits() ? FMOV_xh : FMOV_wh;
      break;
    case 32:
      op = FMOV_ws;
      break;
    default:
      op = FMOV_xd;
  }
  Emit(op | Rd(rd) | Rn(vn));
}


void Assembler::fmov(const VRegister& vd, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP) ||
              (vd.Is1D() && CPUHas(CPUFeatures::kNEON)));
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT((vd.GetSizeInBits() == rn.GetSizeInBits()) || vd.Is1H());
  FPIntegerConvertOp op;
  switch (vd.GetSizeInBits()) {
    case 16:
      VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
      op = rn.Is64Bits() ? FMOV_hx : FMOV_hw;
      break;
    case 32:
      op = FMOV_sw;
      break;
    default:
      op = FMOV_dx;
  }
  Emit(op | Rd(vd) | Rn(rn));
}


void Assembler::fmov(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  }
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT(vd.IsSameFormat(vn));
  Emit(FPType(vd) | FMOV | Rd(vd) | Rn(vn));
}


void Assembler::fmov(const VRegister& vd, int index, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kFP));
  VIXL_ASSERT((index == 1) && vd.Is1D() && rn.IsX());
  USE(index);
  Emit(FMOV_d1_x | Rd(vd) | Rn(rn));
}


void Assembler::fmov(const Register& rd, const VRegister& vn, int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kFP));
  VIXL_ASSERT((index == 1) && vn.Is1D() && rd.IsX());
  USE(index);
  Emit(FMOV_x_d1 | Rd(rd) | Rn(vn));
}


void Assembler::fmadd(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  FPDataProcessing3SourceOp op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    op = FMADD_h;
  } else if (vd.Is1S()) {
    op = FMADD_s;
  } else {
    VIXL_ASSERT(vd.Is1D());
    op = FMADD_d;
  }
  FPDataProcessing3Source(vd, vn, vm, va, op);
}


void Assembler::fmsub(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  FPDataProcessing3SourceOp op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    op = FMSUB_h;
  } else if (vd.Is1S()) {
    op = FMSUB_s;
  } else {
    VIXL_ASSERT(vd.Is1D());
    op = FMSUB_d;
  }
  FPDataProcessing3Source(vd, vn, vm, va, op);
}


void Assembler::fnmadd(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm,
                       const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  FPDataProcessing3SourceOp op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    op = FNMADD_h;
  } else if (vd.Is1S()) {
    op = FNMADD_s;
  } else {
    VIXL_ASSERT(vd.Is1D());
    op = FNMADD_d;
  }
  FPDataProcessing3Source(vd, vn, vm, va, op);
}


void Assembler::fnmsub(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm,
                       const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  FPDataProcessing3SourceOp op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    op = FNMSUB_h;
  } else if (vd.Is1S()) {
    op = FNMSUB_s;
  } else {
    VIXL_ASSERT(vd.Is1D());
    op = FNMSUB_d;
  }
  FPDataProcessing3Source(vd, vn, vm, va, op);
}


void Assembler::fnmul(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  VIXL_ASSERT(AreSameSizeAndType(vd, vn, vm));
  Instr op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
    op = FNMUL_h;
  } else if (vd.Is1S()) {
    op = FNMUL_s;
  } else {
    VIXL_ASSERT(vd.Is1D());
    op = FNMUL_d;
  }
  Emit(FPType(vd) | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::FPCompareMacro(const VRegister& vn,
                               double value,
                               FPTrapFlags trap) {
  USE(value);
  // Although the fcmp{e} instructions can strictly only take an immediate
  // value of +0.0, we don't need to check for -0.0 because the sign of 0.0
  // doesn't affect the result of the comparison.
  VIXL_ASSERT(value == 0.0);
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  Instr op = (trap == EnableTrap) ? FCMPE_zero : FCMP_zero;
  Emit(FPType(vn) | op | Rn(vn));
}


void Assembler::FPCompareMacro(const VRegister& vn,
                               const VRegister& vm,
                               FPTrapFlags trap) {
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  VIXL_ASSERT(vn.IsSameSizeAndType(vm));
  Instr op = (trap == EnableTrap) ? FCMPE : FCMP;
  Emit(FPType(vn) | op | Rm(vm) | Rn(vn));
}


void Assembler::fcmp(const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCompareMacro(vn, vm, DisableTrap);
}


void Assembler::fcmpe(const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCompareMacro(vn, vm, EnableTrap);
}


void Assembler::fcmp(const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCompareMacro(vn, value, DisableTrap);
}


void Assembler::fcmpe(const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCompareMacro(vn, value, EnableTrap);
}


void Assembler::FPCCompareMacro(const VRegister& vn,
                                const VRegister& vm,
                                StatusFlags nzcv,
                                Condition cond,
                                FPTrapFlags trap) {
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  VIXL_ASSERT(vn.IsSameSizeAndType(vm));
  Instr op = (trap == EnableTrap) ? FCCMPE : FCCMP;
  Emit(FPType(vn) | op | Rm(vm) | Cond(cond) | Rn(vn) | Nzcv(nzcv));
}

void Assembler::fccmp(const VRegister& vn,
                      const VRegister& vm,
                      StatusFlags nzcv,
                      Condition cond) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCCompareMacro(vn, vm, nzcv, cond, DisableTrap);
}


void Assembler::fccmpe(const VRegister& vn,
                       const VRegister& vm,
                       StatusFlags nzcv,
                       Condition cond) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  FPCCompareMacro(vn, vm, nzcv, cond, EnableTrap);
}


void Assembler::fcsel(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      Condition cond) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  Emit(FPType(vd) | FCSEL | Rm(vm) | Cond(cond) | Rn(vn) | Rd(vd));
}


void Assembler::fcvt(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  FPDataProcessing1SourceOp op;
  // The half-precision variants belong to base FP, and do not require kFPHalf.
  if (vd.Is1D()) {
    VIXL_ASSERT(vn.Is1S() || vn.Is1H());
    op = vn.Is1S() ? FCVT_ds : FCVT_dh;
  } else if (vd.Is1S()) {
    VIXL_ASSERT(vn.Is1D() || vn.Is1H());
    op = vn.Is1D() ? FCVT_sd : FCVT_sh;
  } else {
    VIXL_ASSERT(vd.Is1H());
    VIXL_ASSERT(vn.Is1D() || vn.Is1S());
    op = vn.Is1D() ? FCVT_hd : FCVT_hs;
  }
  FPDataProcessing1Source(vd, vn, op);
}


void Assembler::fcvtl(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is4S() && vn.Is4H()) || (vd.Is2D() && vn.Is2S()));
  // The half-precision variants belong to base FP, and do not require kFPHalf.
  Instr format = vd.Is2D() ? (1 << NEONSize_offset) : 0;
  Emit(format | NEON_FCVTL | Rn(vn) | Rd(vd));
}


void Assembler::fcvtl2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is4S() && vn.Is8H()) || (vd.Is2D() && vn.Is4S()));
  // The half-precision variants belong to base FP, and do not require kFPHalf.
  Instr format = vd.Is2D() ? (1 << NEONSize_offset) : 0;
  Emit(NEON_Q | format | NEON_FCVTL | Rn(vn) | Rd(vd));
}


void Assembler::fcvtn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vn.Is4S() && vd.Is4H()) || (vn.Is2D() && vd.Is2S()));
  // The half-precision variants belong to base FP, and do not require kFPHalf.
  Instr format = vn.Is2D() ? (1 << NEONSize_offset) : 0;
  Emit(format | NEON_FCVTN | Rn(vn) | Rd(vd));
}


void Assembler::fcvtn2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vn.Is4S() && vd.Is8H()) || (vn.Is2D() && vd.Is4S()));
  // The half-precision variants belong to base FP, and do not require kFPHalf.
  Instr format = vn.Is2D() ? (1 << NEONSize_offset) : 0;
  Emit(NEON_Q | format | NEON_FCVTN | Rn(vn) | Rd(vd));
}


void Assembler::fcvtxn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  Instr format = 1 << NEONSize_offset;
  if (vd.IsScalar()) {
    VIXL_ASSERT(vd.Is1S() && vn.Is1D());
    Emit(format | NEON_FCVTXN_scalar | Rn(vn) | Rd(vd));
  } else {
    VIXL_ASSERT(vd.Is2S() && vn.Is2D());
    Emit(format | NEON_FCVTXN | Rn(vn) | Rd(vd));
  }
}


void Assembler::fcvtxn2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT(vd.Is4S() && vn.Is2D());
  Instr format = 1 << NEONSize_offset;
  Emit(NEON_Q | format | NEON_FCVTXN | Rn(vn) | Rd(vd));
}

void Assembler::fjcvtzs(const Register& rd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kJSCVT));
  VIXL_ASSERT(rd.IsW() && vn.Is1D());
  Emit(FJCVTZS | Rn(vn) | Rd(rd));
}


void Assembler::NEONFPConvertToInt(const Register& rd,
                                   const VRegister& vn,
                                   Instr op) {
  Emit(SF(rd) | FPType(vn) | op | Rn(vn) | Rd(rd));
}


void Assembler::NEONFPConvertToInt(const VRegister& vd,
                                   const VRegister& vn,
                                   Instr op) {
  if (vn.IsScalar()) {
    VIXL_ASSERT((vd.Is1S() && vn.Is1S()) || (vd.Is1D() && vn.Is1D()));
    op |= NEON_Q | NEONScalar;
  }
  Emit(FPFormat(vn) | op | Rn(vn) | Rd(vd));
}


void Assembler::NEONFP16ConvertToInt(const VRegister& vd,
                                     const VRegister& vn,
                                     Instr op) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vn.IsLaneSizeH());
  if (vn.IsScalar()) {
    op |= NEON_Q | NEONScalar;
  } else if (vn.Is8H()) {
    op |= NEON_Q;
  }
  Emit(op | Rn(vn) | Rd(vd));
}


#define NEON_FP2REGMISC_FCVT_LIST(V) \
  V(fcvtnu, NEON_FCVTNU, FCVTNU)     \
  V(fcvtns, NEON_FCVTNS, FCVTNS)     \
  V(fcvtpu, NEON_FCVTPU, FCVTPU)     \
  V(fcvtps, NEON_FCVTPS, FCVTPS)     \
  V(fcvtmu, NEON_FCVTMU, FCVTMU)     \
  V(fcvtms, NEON_FCVTMS, FCVTMS)     \
  V(fcvtau, NEON_FCVTAU, FCVTAU)     \
  V(fcvtas, NEON_FCVTAS, FCVTAS)

#define VIXL_DEFINE_ASM_FUNC(FN, VEC_OP, SCA_OP)                 \
  void Assembler::FN(const Register& rd, const VRegister& vn) {  \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP));                       \
    if (vn.IsH()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));     \
    NEONFPConvertToInt(rd, vn, SCA_OP);                          \
  }                                                              \
  void Assembler::FN(const VRegister& vd, const VRegister& vn) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));   \
    if (vd.IsLaneSizeH()) {                                      \
      VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));               \
      NEONFP16ConvertToInt(vd, vn, VEC_OP##_H);                  \
    } else {                                                     \
      NEONFPConvertToInt(vd, vn, VEC_OP);                        \
    }                                                            \
  }
NEON_FP2REGMISC_FCVT_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


void Assembler::fcvtzs(const Register& rd, const VRegister& vn, int fbits) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  VIXL_ASSERT((fbits >= 0) && (fbits <= rd.GetSizeInBits()));
  if (fbits == 0) {
    Emit(SF(rd) | FPType(vn) | FCVTZS | Rn(vn) | Rd(rd));
  } else {
    Emit(SF(rd) | FPType(vn) | FCVTZS_fixed | FPScale(64 - fbits) | Rn(vn) |
         Rd(rd));
  }
}


void Assembler::fcvtzs(const VRegister& vd, const VRegister& vn, int fbits) {
  // This form is a NEON scalar FP instruction.
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vn.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    if (vd.IsLaneSizeH()) {
      NEONFP2RegMiscFP16(vd, vn, NEON_FCVTZS_H);
    } else {
      NEONFP2RegMisc(vd, vn, NEON_FCVTZS);
    }
  } else {
    VIXL_ASSERT(vd.Is1D() || vd.Is1S() || vd.Is2D() || vd.Is2S() || vd.Is4S() ||
                vd.Is1H() || vd.Is4H() || vd.Is8H());
    NEONShiftRightImmediate(vd, vn, fbits, NEON_FCVTZS_imm);
  }
}


void Assembler::fcvtzu(const Register& rd, const VRegister& vn, int fbits) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vn.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  VIXL_ASSERT(vn.Is1H() || vn.Is1S() || vn.Is1D());
  VIXL_ASSERT((fbits >= 0) && (fbits <= rd.GetSizeInBits()));
  if (fbits == 0) {
    Emit(SF(rd) | FPType(vn) | FCVTZU | Rn(vn) | Rd(rd));
  } else {
    Emit(SF(rd) | FPType(vn) | FCVTZU_fixed | FPScale(64 - fbits) | Rn(vn) |
         Rd(rd));
  }
}


void Assembler::fcvtzu(const VRegister& vd, const VRegister& vn, int fbits) {
  // This form is a NEON scalar FP instruction.
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vn.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    if (vd.IsLaneSizeH()) {
      NEONFP2RegMiscFP16(vd, vn, NEON_FCVTZU_H);
    } else {
      NEONFP2RegMisc(vd, vn, NEON_FCVTZU);
    }
  } else {
    VIXL_ASSERT(vd.Is1D() || vd.Is1S() || vd.Is2D() || vd.Is2S() || vd.Is4S() ||
                vd.Is1H() || vd.Is4H() || vd.Is8H());
    NEONShiftRightImmediate(vd, vn, fbits, NEON_FCVTZU_imm);
  }
}

void Assembler::ucvtf(const VRegister& vd, const VRegister& vn, int fbits) {
  // This form is a NEON scalar FP instruction.
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vn.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    if (vd.IsLaneSizeH()) {
      NEONFP2RegMiscFP16(vd, vn, NEON_UCVTF_H);
    } else {
      NEONFP2RegMisc(vd, vn, NEON_UCVTF);
    }
  } else {
    VIXL_ASSERT(vd.Is1D() || vd.Is1S() || vd.Is2D() || vd.Is2S() || vd.Is4S() ||
                vd.Is1H() || vd.Is4H() || vd.Is8H());
    NEONShiftRightImmediate(vd, vn, fbits, NEON_UCVTF_imm);
  }
}

void Assembler::scvtf(const VRegister& vd, const VRegister& vn, int fbits) {
  // This form is a NEON scalar FP instruction.
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vn.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    if (vd.IsLaneSizeH()) {
      NEONFP2RegMiscFP16(vd, vn, NEON_SCVTF_H);
    } else {
      NEONFP2RegMisc(vd, vn, NEON_SCVTF);
    }
  } else {
    VIXL_ASSERT(vd.Is1D() || vd.Is1S() || vd.Is2D() || vd.Is2S() || vd.Is4S() ||
                vd.Is1H() || vd.Is4H() || vd.Is8H());
    NEONShiftRightImmediate(vd, vn, fbits, NEON_SCVTF_imm);
  }
}


void Assembler::scvtf(const VRegister& vd, const Register& rn, int fbits) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    Emit(SF(rn) | FPType(vd) | SCVTF | Rn(rn) | Rd(vd));
  } else {
    Emit(SF(rn) | FPType(vd) | SCVTF_fixed | FPScale(64 - fbits) | Rn(rn) |
         Rd(vd));
  }
}


void Assembler::ucvtf(const VRegister& vd, const Register& rn, int fbits) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP));
  if (vd.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT(fbits >= 0);
  if (fbits == 0) {
    Emit(SF(rn) | FPType(vd) | UCVTF | Rn(rn) | Rd(vd));
  } else {
    Emit(SF(rn) | FPType(vd) | UCVTF_fixed | FPScale(64 - fbits) | Rn(rn) |
         Rd(vd));
  }
}


void Assembler::NEON3Same(const VRegister& vd,
                          const VRegister& vn,
                          const VRegister& vm,
                          NEON3SameOp vop) {
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.IsVector() || !vd.IsQ());

  Instr format, op = vop;
  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
    format = SFormat(vd);
  } else {
    format = VFormat(vd);
  }

  Emit(format | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::NEONFP3Same(const VRegister& vd,
                            const VRegister& vn,
                            const VRegister& vm,
                            Instr op) {
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  Emit(FPFormat(vd) | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::NEON3SameFP16(const VRegister& vd,
                              const VRegister& vn,
                              const VRegister& vm,
                              Instr op) {
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.GetLaneSizeInBytes() == kHRegSizeInBytes);
  if (vd.Is8H()) op |= NEON_Q;
  Emit(op | Rm(vm) | Rn(vn) | Rd(vd));
}


// clang-format off
#define NEON_FP2REGMISC_LIST(V)                                        \
  V(fabs,    NEON_FABS,    FABS,                FABS_h)                \
  V(fneg,    NEON_FNEG,    FNEG,                FNEG_h)                \
  V(fsqrt,   NEON_FSQRT,   FSQRT,               FSQRT_h)               \
  V(frintn,  NEON_FRINTN,  FRINTN,              FRINTN_h)              \
  V(frinta,  NEON_FRINTA,  FRINTA,              FRINTA_h)              \
  V(frintp,  NEON_FRINTP,  FRINTP,              FRINTP_h)              \
  V(frintm,  NEON_FRINTM,  FRINTM,              FRINTM_h)              \
  V(frintx,  NEON_FRINTX,  FRINTX,              FRINTX_h)              \
  V(frintz,  NEON_FRINTZ,  FRINTZ,              FRINTZ_h)              \
  V(frinti,  NEON_FRINTI,  FRINTI,              FRINTI_h)              \
  V(frsqrte, NEON_FRSQRTE, NEON_FRSQRTE_scalar, NEON_FRSQRTE_H_scalar) \
  V(frecpe,  NEON_FRECPE,  NEON_FRECPE_scalar,  NEON_FRECPE_H_scalar)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, VEC_OP, SCA_OP, SCA_OP_H)                   \
  void Assembler::FN(const VRegister& vd, const VRegister& vn) {             \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP));                                   \
    Instr op;                                                                \
    if (vd.IsScalar()) {                                                     \
      if (vd.Is1H()) {                                                       \
        if ((SCA_OP_H & NEONScalar2RegMiscFP16FMask) ==                      \
            NEONScalar2RegMiscFP16Fixed) {                                   \
          VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kNEONHalf));   \
        } else {                                                             \
          VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));                         \
        }                                                                    \
        op = SCA_OP_H;                                                       \
      } else {                                                               \
        if ((SCA_OP & NEONScalar2RegMiscFMask) == NEONScalar2RegMiscFixed) { \
          VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                           \
        }                                                                    \
        VIXL_ASSERT(vd.Is1S() || vd.Is1D());                                 \
        op = SCA_OP;                                                         \
      }                                                                      \
    } else {                                                                 \
      VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                               \
      VIXL_ASSERT(vd.Is4H() || vd.Is8H() || vd.Is2S() || vd.Is2D() ||        \
                  vd.Is4S());                                                \
      if (vd.IsLaneSizeH()) {                                                \
        VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));                         \
        op = VEC_OP##_H;                                                     \
        if (vd.Is8H()) {                                                     \
          op |= NEON_Q;                                                      \
        }                                                                    \
      } else {                                                               \
        op = VEC_OP;                                                         \
      }                                                                      \
    }                                                                        \
    if (vd.IsLaneSizeH()) {                                                  \
      NEONFP2RegMiscFP16(vd, vn, op);                                        \
    } else {                                                                 \
      NEONFP2RegMisc(vd, vn, op);                                            \
    }                                                                        \
  }
NEON_FP2REGMISC_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// clang-format off
#define NEON_FP2REGMISC_V85_LIST(V)       \
  V(frint32x,  NEON_FRINT32X,  FRINT32X)  \
  V(frint32z,  NEON_FRINT32Z,  FRINT32Z)  \
  V(frint64x,  NEON_FRINT64X,  FRINT64X)  \
  V(frint64z,  NEON_FRINT64Z,  FRINT64Z)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, VEC_OP, SCA_OP)                               \
  void Assembler::FN(const VRegister& vd, const VRegister& vn) {               \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kFrintToFixedSizedInt)); \
    Instr op;                                                                  \
    if (vd.IsScalar()) {                                                       \
      VIXL_ASSERT(vd.Is1S() || vd.Is1D());                                     \
      op = SCA_OP;                                                             \
    } else {                                                                   \
      VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                                 \
      VIXL_ASSERT(vd.Is2S() || vd.Is2D() || vd.Is4S());                        \
      op = VEC_OP;                                                             \
    }                                                                          \
    NEONFP2RegMisc(vd, vn, op);                                                \
  }
NEON_FP2REGMISC_V85_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

void Assembler::NEONFP2RegMiscFP16(const VRegister& vd,
                                   const VRegister& vn,
                                   Instr op) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  Emit(op | Rn(vn) | Rd(vd));
}


void Assembler::NEONFP2RegMisc(const VRegister& vd,
                               const VRegister& vn,
                               Instr op) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  Emit(FPFormat(vd) | op | Rn(vn) | Rd(vd));
}


void Assembler::NEON2RegMisc(const VRegister& vd,
                             const VRegister& vn,
                             NEON2RegMiscOp vop,
                             int value) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(value == 0);
  USE(value);

  Instr format, op = vop;
  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
    format = SFormat(vd);
  } else {
    format = VFormat(vd);
  }

  Emit(format | op | Rn(vn) | Rd(vd));
}


void Assembler::cmeq(const VRegister& vd, const VRegister& vn, int value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_CMEQ_zero, value);
}


void Assembler::cmge(const VRegister& vd, const VRegister& vn, int value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_CMGE_zero, value);
}


void Assembler::cmgt(const VRegister& vd, const VRegister& vn, int value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_CMGT_zero, value);
}


void Assembler::cmle(const VRegister& vd, const VRegister& vn, int value) {
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEON2RegMisc(vd, vn, NEON_CMLE_zero, value);
}


void Assembler::cmlt(const VRegister& vd, const VRegister& vn, int value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_CMLT_zero, value);
}


void Assembler::shll(const VRegister& vd, const VRegister& vn, int shift) {
  USE(shift);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is8H() && vn.Is8B() && shift == 8) ||
              (vd.Is4S() && vn.Is4H() && shift == 16) ||
              (vd.Is2D() && vn.Is2S() && shift == 32));
  Emit(VFormat(vn) | NEON_SHLL | Rn(vn) | Rd(vd));
}


void Assembler::shll2(const VRegister& vd, const VRegister& vn, int shift) {
  USE(shift);
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is8H() && vn.Is16B() && shift == 8) ||
              (vd.Is4S() && vn.Is8H() && shift == 16) ||
              (vd.Is2D() && vn.Is4S() && shift == 32));
  Emit(VFormat(vn) | NEON_SHLL | Rn(vn) | Rd(vd));
}


void Assembler::NEONFP2RegMisc(const VRegister& vd,
                               const VRegister& vn,
                               NEON2RegMiscOp vop,
                               double value) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(value == 0.0);
  USE(value);

  Instr op = vop;
  if (vd.IsScalar()) {
    VIXL_ASSERT(vd.Is1S() || vd.Is1D());
    op |= NEON_Q | NEONScalar;
  } else {
    VIXL_ASSERT(vd.Is2S() || vd.Is2D() || vd.Is4S());
  }

  Emit(FPFormat(vd) | op | Rn(vn) | Rd(vd));
}


void Assembler::NEONFP2RegMiscFP16(const VRegister& vd,
                                   const VRegister& vn,
                                   NEON2RegMiscFP16Op vop,
                                   double value) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(value == 0.0);
  USE(value);

  Instr op = vop;
  if (vd.IsScalar()) {
    VIXL_ASSERT(vd.Is1H());
    op |= NEON_Q | NEONScalar;
  } else {
    VIXL_ASSERT(vd.Is4H() || vd.Is8H());
    if (vd.Is8H()) {
      op |= NEON_Q;
    }
  }

  Emit(op | Rn(vn) | Rd(vd));
}


void Assembler::fcmeq(const VRegister& vd, const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vd.IsLaneSizeH()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    NEONFP2RegMiscFP16(vd, vn, NEON_FCMEQ_H_zero, value);
  } else {
    NEONFP2RegMisc(vd, vn, NEON_FCMEQ_zero, value);
  }
}


void Assembler::fcmge(const VRegister& vd, const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vd.IsLaneSizeH()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    NEONFP2RegMiscFP16(vd, vn, NEON_FCMGE_H_zero, value);
  } else {
    NEONFP2RegMisc(vd, vn, NEON_FCMGE_zero, value);
  }
}


void Assembler::fcmgt(const VRegister& vd, const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vd.IsLaneSizeH()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    NEONFP2RegMiscFP16(vd, vn, NEON_FCMGT_H_zero, value);
  } else {
    NEONFP2RegMisc(vd, vn, NEON_FCMGT_zero, value);
  }
}


void Assembler::fcmle(const VRegister& vd, const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vd.IsLaneSizeH()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    NEONFP2RegMiscFP16(vd, vn, NEON_FCMLE_H_zero, value);
  } else {
    NEONFP2RegMisc(vd, vn, NEON_FCMLE_zero, value);
  }
}


void Assembler::fcmlt(const VRegister& vd, const VRegister& vn, double value) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  if (vd.IsLaneSizeH()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    NEONFP2RegMiscFP16(vd, vn, NEON_FCMLT_H_zero, value);
  } else {
    NEONFP2RegMisc(vd, vn, NEON_FCMLT_zero, value);
  }
}


void Assembler::frecpx(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsScalar());
  VIXL_ASSERT(AreSameFormat(vd, vn));
  Instr op;
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    op = NEON_FRECPX_H_scalar;
  } else {
    VIXL_ASSERT(vd.Is1S() || vd.Is1D());
    op = NEON_FRECPX_scalar;
  }
  Emit(FPFormat(vd) | op | Rn(vn) | Rd(vd));
}


// clang-format off
#define NEON_3SAME_LIST(V) \
  V(add,      NEON_ADD,      vd.IsVector() || vd.Is1D())            \
  V(addp,     NEON_ADDP,     vd.IsVector() || vd.Is1D())            \
  V(sub,      NEON_SUB,      vd.IsVector() || vd.Is1D())            \
  V(cmeq,     NEON_CMEQ,     vd.IsVector() || vd.Is1D())            \
  V(cmge,     NEON_CMGE,     vd.IsVector() || vd.Is1D())            \
  V(cmgt,     NEON_CMGT,     vd.IsVector() || vd.Is1D())            \
  V(cmhi,     NEON_CMHI,     vd.IsVector() || vd.Is1D())            \
  V(cmhs,     NEON_CMHS,     vd.IsVector() || vd.Is1D())            \
  V(cmtst,    NEON_CMTST,    vd.IsVector() || vd.Is1D())            \
  V(sshl,     NEON_SSHL,     vd.IsVector() || vd.Is1D())            \
  V(ushl,     NEON_USHL,     vd.IsVector() || vd.Is1D())            \
  V(srshl,    NEON_SRSHL,    vd.IsVector() || vd.Is1D())            \
  V(urshl,    NEON_URSHL,    vd.IsVector() || vd.Is1D())            \
  V(sqdmulh,  NEON_SQDMULH,  vd.IsLaneSizeH() || vd.IsLaneSizeS())  \
  V(sqrdmulh, NEON_SQRDMULH, vd.IsLaneSizeH() || vd.IsLaneSizeS())  \
  V(shadd,    NEON_SHADD,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(uhadd,    NEON_UHADD,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(srhadd,   NEON_SRHADD,   vd.IsVector() && !vd.IsLaneSizeD())    \
  V(urhadd,   NEON_URHADD,   vd.IsVector() && !vd.IsLaneSizeD())    \
  V(shsub,    NEON_SHSUB,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(uhsub,    NEON_UHSUB,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(smax,     NEON_SMAX,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(smaxp,    NEON_SMAXP,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(smin,     NEON_SMIN,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(sminp,    NEON_SMINP,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(umax,     NEON_UMAX,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(umaxp,    NEON_UMAXP,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(umin,     NEON_UMIN,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(uminp,    NEON_UMINP,    vd.IsVector() && !vd.IsLaneSizeD())    \
  V(saba,     NEON_SABA,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(sabd,     NEON_SABD,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(uaba,     NEON_UABA,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(uabd,     NEON_UABD,     vd.IsVector() && !vd.IsLaneSizeD())    \
  V(mla,      NEON_MLA,      vd.IsVector() && !vd.IsLaneSizeD())    \
  V(mls,      NEON_MLS,      vd.IsVector() && !vd.IsLaneSizeD())    \
  V(mul,      NEON_MUL,      vd.IsVector() && !vd.IsLaneSizeD())    \
  V(and_,     NEON_AND,      vd.Is8B() || vd.Is16B())               \
  V(orr,      NEON_ORR,      vd.Is8B() || vd.Is16B())               \
  V(orn,      NEON_ORN,      vd.Is8B() || vd.Is16B())               \
  V(eor,      NEON_EOR,      vd.Is8B() || vd.Is16B())               \
  V(bic,      NEON_BIC,      vd.Is8B() || vd.Is16B())               \
  V(bit,      NEON_BIT,      vd.Is8B() || vd.Is16B())               \
  V(bif,      NEON_BIF,      vd.Is8B() || vd.Is16B())               \
  V(bsl,      NEON_BSL,      vd.Is8B() || vd.Is16B())               \
  V(pmul,     NEON_PMUL,     vd.Is8B() || vd.Is16B())               \
  V(uqadd,    NEON_UQADD,    true)                                  \
  V(sqadd,    NEON_SQADD,    true)                                  \
  V(uqsub,    NEON_UQSUB,    true)                                  \
  V(sqsub,    NEON_SQSUB,    true)                                  \
  V(sqshl,    NEON_SQSHL,    true)                                  \
  V(uqshl,    NEON_UQSHL,    true)                                  \
  V(sqrshl,   NEON_SQRSHL,   true)                                  \
  V(uqrshl,   NEON_UQRSHL,   true)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP, AS)     \
  void Assembler::FN(const VRegister& vd,    \
                     const VRegister& vn,    \
                     const VRegister& vm) {  \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON)); \
    VIXL_ASSERT(AS);                         \
    NEON3Same(vd, vn, vm, OP);               \
  }
NEON_3SAME_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

// clang-format off
#define NEON_FP3SAME_OP_LIST(V)                                        \
  V(fmulx,   NEON_FMULX,   NEON_FMULX_scalar,   NEON_FMULX_H_scalar)   \
  V(frecps,  NEON_FRECPS,  NEON_FRECPS_scalar,  NEON_FRECPS_H_scalar)  \
  V(frsqrts, NEON_FRSQRTS, NEON_FRSQRTS_scalar, NEON_FRSQRTS_H_scalar) \
  V(fabd,    NEON_FABD,    NEON_FABD_scalar,    NEON_FABD_H_scalar)    \
  V(fmla,    NEON_FMLA,    0,                   0)                     \
  V(fmls,    NEON_FMLS,    0,                   0)                     \
  V(facge,   NEON_FACGE,   NEON_FACGE_scalar,   NEON_FACGE_H_scalar)   \
  V(facgt,   NEON_FACGT,   NEON_FACGT_scalar,   NEON_FACGT_H_scalar)   \
  V(fcmeq,   NEON_FCMEQ,   NEON_FCMEQ_scalar,   NEON_FCMEQ_H_scalar)   \
  V(fcmge,   NEON_FCMGE,   NEON_FCMGE_scalar,   NEON_FCMGE_H_scalar)   \
  V(fcmgt,   NEON_FCMGT,   NEON_FCMGT_scalar,   NEON_FCMGT_H_scalar)   \
  V(faddp,   NEON_FADDP,   0,                   0)                     \
  V(fmaxp,   NEON_FMAXP,   0,                   0)                     \
  V(fminp,   NEON_FMINP,   0,                   0)                     \
  V(fmaxnmp, NEON_FMAXNMP, 0,                   0)                     \
  V(fadd,    NEON_FADD,    FADD,                0)                     \
  V(fsub,    NEON_FSUB,    FSUB,                0)                     \
  V(fmul,    NEON_FMUL,    FMUL,                0)                     \
  V(fdiv,    NEON_FDIV,    FDIV,                0)                     \
  V(fmax,    NEON_FMAX,    FMAX,                0)                     \
  V(fmin,    NEON_FMIN,    FMIN,                0)                     \
  V(fmaxnm,  NEON_FMAXNM,  FMAXNM,              0)                     \
  V(fminnm,  NEON_FMINNM,  FMINNM,              0)                     \
  V(fminnmp, NEON_FMINNMP, 0,                   0)
// clang-format on

// TODO: This macro is complicated because it classifies the instructions in the
// macro list above, and treats each case differently. It could be somewhat
// simpler if we were to split the macro, at the cost of some duplication.
#define VIXL_DEFINE_ASM_FUNC(FN, VEC_OP, SCA_OP, SCA_OP_H)               \
  void Assembler::FN(const VRegister& vd,                                \
                     const VRegister& vn,                                \
                     const VRegister& vm) {                              \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP));                               \
    Instr op;                                                            \
    bool is_fp16 = false;                                                \
    if ((SCA_OP != 0) && vd.IsScalar()) {                                \
      if ((SCA_OP_H != 0) && vd.Is1H()) {                                \
        VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kNEONHalf)); \
        is_fp16 = true;                                                  \
        op = SCA_OP_H;                                                   \
      } else {                                                           \
        VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());                \
        if ((SCA_OP & NEONScalar3SameFMask) == NEONScalar3SameFixed) {   \
          VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                       \
          if (vd.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));    \
        } else if (vd.Is1H()) {                                          \
          VIXL_ASSERT(CPUHas(CPUFeatures::kFPHalf));                     \
        }                                                                \
        op = SCA_OP;                                                     \
      }                                                                  \
    } else {                                                             \
      VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                           \
      VIXL_ASSERT(vd.IsVector());                                        \
      if (vd.Is4H() || vd.Is8H()) {                                      \
        VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));                     \
        is_fp16 = true;                                                  \
        op = VEC_OP##_H;                                                 \
      } else {                                                           \
        VIXL_ASSERT(vd.Is2S() || vd.Is2D() || vd.Is4S());                \
        op = VEC_OP;                                                     \
      }                                                                  \
    }                                                                    \
    if (is_fp16) {                                                       \
      NEON3SameFP16(vd, vn, vm, op);                                     \
    } else {                                                             \
      NEONFP3Same(vd, vn, vm, op);                                       \
    }                                                                    \
  }
NEON_FP3SAME_OP_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_FHM_LIST(V) \
  V(fmlal,   NEON_FMLAL)   \
  V(fmlal2,  NEON_FMLAL2)  \
  V(fmlsl,   NEON_FMLSL)   \
  V(fmlsl2,  NEON_FMLSL2)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, VEC_OP)                    \
  void Assembler::FN(const VRegister& vd,                   \
                     const VRegister& vn,                   \
                     const VRegister& vm) {                 \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON,                  \
                       CPUFeatures::kFP,                    \
                       CPUFeatures::kNEONHalf,              \
                       CPUFeatures::kFHM));                 \
    VIXL_ASSERT((vd.Is2S() && vn.Is2H() && vm.Is2H()) ||    \
                (vd.Is4S() && vn.Is4H() && vm.Is4H()));     \
    Emit(FPFormat(vd) | VEC_OP | Rm(vm) | Rn(vn) | Rd(vd)); \
  }
NEON_FHM_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


void Assembler::addp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1D() && vn.Is2D()));
  Emit(SFormat(vd) | NEON_ADDP_scalar | Rn(vn) | Rd(vd));
}


void Assembler::sqrdmlah(const VRegister& vd,
                         const VRegister& vn,
                         const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kRDM));
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.IsLaneSizeH() || vd.IsLaneSizeS());

  Instr format, op = NEON_SQRDMLAH;
  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
    format = SFormat(vd);
  } else {
    format = VFormat(vd);
  }

  Emit(format | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::sqrdmlsh(const VRegister& vd,
                         const VRegister& vn,
                         const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kRDM));
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.IsLaneSizeH() || vd.IsLaneSizeS());

  Instr format, op = NEON_SQRDMLSH;
  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
    format = SFormat(vd);
  } else {
    format = VFormat(vd);
  }

  Emit(format | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::sdot(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kDotProduct));
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B()) || (vd.Is4S() && vn.Is16B()));

  Emit(VFormat(vd) | NEON_SDOT | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::udot(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kDotProduct));
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B()) || (vd.Is4S() && vn.Is16B()));

  Emit(VFormat(vd) | NEON_UDOT | Rm(vm) | Rn(vn) | Rd(vd));
}

void Assembler::usdot(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kI8MM));
  VIXL_ASSERT(AreSameFormat(vn, vm));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B()) || (vd.Is4S() && vn.Is16B()));

  Emit(VFormat(vd) | 0x0e809c00 | Rm(vm) | Rn(vn) | Rd(vd));
}

void Assembler::faddp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1S() && vn.Is2S()) || (vd.Is1D() && vn.Is2D()) ||
              (vd.Is1H() && vn.Is2H()));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    Emit(NEON_FADDP_h_scalar | Rn(vn) | Rd(vd));
  } else {
    Emit(FPFormat(vd) | NEON_FADDP_scalar | Rn(vn) | Rd(vd));
  }
}


void Assembler::fmaxp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1S() && vn.Is2S()) || (vd.Is1D() && vn.Is2D()) ||
              (vd.Is1H() && vn.Is2H()));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    Emit(NEON_FMAXP_h_scalar | Rn(vn) | Rd(vd));
  } else {
    Emit(FPFormat(vd) | NEON_FMAXP_scalar | Rn(vn) | Rd(vd));
  }
}


void Assembler::fminp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1S() && vn.Is2S()) || (vd.Is1D() && vn.Is2D()) ||
              (vd.Is1H() && vn.Is2H()));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    Emit(NEON_FMINP_h_scalar | Rn(vn) | Rd(vd));
  } else {
    Emit(FPFormat(vd) | NEON_FMINP_scalar | Rn(vn) | Rd(vd));
  }
}


void Assembler::fmaxnmp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1S() && vn.Is2S()) || (vd.Is1D() && vn.Is2D()) ||
              (vd.Is1H() && vn.Is2H()));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    Emit(NEON_FMAXNMP_h_scalar | Rn(vn) | Rd(vd));
  } else {
    Emit(FPFormat(vd) | NEON_FMAXNMP_scalar | Rn(vn) | Rd(vd));
  }
}


void Assembler::fminnmp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));
  VIXL_ASSERT((vd.Is1S() && vn.Is2S()) || (vd.Is1D() && vn.Is2D()) ||
              (vd.Is1H() && vn.Is2H()));
  if (vd.Is1H()) {
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
    Emit(NEON_FMINNMP_h_scalar | Rn(vn) | Rd(vd));
  } else {
    Emit(FPFormat(vd) | NEON_FMINNMP_scalar | Rn(vn) | Rd(vd));
  }
}


// v8.3 complex numbers - floating-point complex multiply accumulate.
void Assembler::fcmla(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int vm_index,
                      int rot) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kFcma));
  VIXL_ASSERT(vd.IsVector() && AreSameFormat(vd, vn));
  VIXL_ASSERT((vm.IsH() && (vd.Is8H() || vd.Is4H())) ||
              (vm.IsS() && vd.Is4S()));
  if (vd.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  int index_num_bits = vd.Is4S() ? 1 : 2;
  Emit(VFormat(vd) | Rm(vm) | NEON_FCMLA_byelement |
       ImmNEONHLM(vm_index, index_num_bits) | ImmRotFcmlaSca(rot) | Rn(vn) |
       Rd(vd));
}


void Assembler::fcmla(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int rot) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kFcma));
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.IsVector() && !vd.IsLaneSizeB());
  if (vd.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  Emit(VFormat(vd) | Rm(vm) | NEON_FCMLA | ImmRotFcmlaVec(rot) | Rn(vn) |
       Rd(vd));
}


// v8.3 complex numbers - floating-point complex add.
void Assembler::fcadd(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int rot) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kFcma));
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.IsVector() && !vd.IsLaneSizeB());
  if (vd.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));
  Emit(VFormat(vd) | Rm(vm) | NEON_FCADD | ImmRotFcadd(rot) | Rn(vn) | Rd(vd));
}


void Assembler::orr(const VRegister& vd, const int imm8, const int left_shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONModifiedImmShiftLsl(vd, imm8, left_shift, NEONModifiedImmediate_ORR);
}


void Assembler::mov(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  if (vd.IsD()) {
    orr(vd.V8B(), vn.V8B(), vn.V8B());
  } else {
    VIXL_ASSERT(vd.IsQ());
    orr(vd.V16B(), vn.V16B(), vn.V16B());
  }
}


void Assembler::bic(const VRegister& vd, const int imm8, const int left_shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONModifiedImmShiftLsl(vd, imm8, left_shift, NEONModifiedImmediate_BIC);
}


void Assembler::movi(const VRegister& vd,
                     const uint64_t imm,
                     Shift shift,
                     const int shift_amount) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((shift == LSL) || (shift == MSL));
  if (vd.Is2D() || vd.Is1D()) {
    VIXL_ASSERT(shift_amount == 0);
    int imm8 = 0;
    for (int i = 0; i < 8; ++i) {
      int byte = (imm >> (i * 8)) & 0xff;
      VIXL_ASSERT((byte == 0) || (byte == 0xff));
      if (byte == 0xff) {
        imm8 |= (1 << i);
      }
    }
    int q = vd.Is2D() ? NEON_Q : 0;
    Emit(q | NEONModImmOp(1) | NEONModifiedImmediate_MOVI |
         ImmNEONabcdefgh(imm8) | NEONCmode(0xe) | Rd(vd));
  } else if (shift == LSL) {
    VIXL_ASSERT(IsUint8(imm));
    NEONModifiedImmShiftLsl(vd,
                            static_cast<int>(imm),
                            shift_amount,
                            NEONModifiedImmediate_MOVI);
  } else {
    VIXL_ASSERT(IsUint8(imm));
    NEONModifiedImmShiftMsl(vd,
                            static_cast<int>(imm),
                            shift_amount,
                            NEONModifiedImmediate_MOVI);
  }
}


void Assembler::mvn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  if (vd.IsD()) {
    not_(vd.V8B(), vn.V8B());
  } else {
    VIXL_ASSERT(vd.IsQ());
    not_(vd.V16B(), vn.V16B());
  }
}


void Assembler::mvni(const VRegister& vd,
                     const int imm8,
                     Shift shift,
                     const int shift_amount) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((shift == LSL) || (shift == MSL));
  if (shift == LSL) {
    NEONModifiedImmShiftLsl(vd, imm8, shift_amount, NEONModifiedImmediate_MVNI);
  } else {
    NEONModifiedImmShiftMsl(vd, imm8, shift_amount, NEONModifiedImmediate_MVNI);
  }
}


void Assembler::NEONFPByElement(const VRegister& vd,
                                const VRegister& vn,
                                const VRegister& vm,
                                int vm_index,
                                NEONByIndexedElementOp vop,
                                NEONByIndexedElementOp vop_half) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT((vd.Is2S() && vm.Is1S()) || (vd.Is4S() && vm.Is1S()) ||
              (vd.Is1S() && vm.Is1S()) || (vd.Is2D() && vm.Is1D()) ||
              (vd.Is1D() && vm.Is1D()) || (vd.Is4H() && vm.Is1H()) ||
              (vd.Is8H() && vm.Is1H()) || (vd.Is1H() && vm.Is1H()));
  VIXL_ASSERT((vm.Is1S() && (vm_index < 4)) || (vm.Is1D() && (vm_index < 2)) ||
              (vm.Is1H() && (vm.GetCode() < 16) && (vm_index < 8)));

  Instr op = vop;
  int index_num_bits;
  if (vm.Is1D()) {
    index_num_bits = 1;
  } else if (vm.Is1S()) {
    index_num_bits = 2;
  } else {
    index_num_bits = 3;
    op = vop_half;
  }

  if (vd.IsScalar()) {
    op |= NEON_Q | NEONScalar;
  }

  if (!vm.Is1H()) {
    op |= FPFormat(vd);
  } else if (vd.Is8H()) {
    op |= NEON_Q;
  }

  Emit(op | ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::NEONByElement(const VRegister& vd,
                              const VRegister& vn,
                              const VRegister& vm,
                              int vm_index,
                              NEONByIndexedElementOp vop) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT((vd.Is4H() && vm.Is1H()) || (vd.Is8H() && vm.Is1H()) ||
              (vd.Is1H() && vm.Is1H()) || (vd.Is2S() && vm.Is1S()) ||
              (vd.Is4S() && vm.Is1S()) || (vd.Is1S() && vm.Is1S()));
  VIXL_ASSERT((vm.Is1H() && (vm.GetCode() < 16) && (vm_index < 8)) ||
              (vm.Is1S() && (vm_index < 4)));

  Instr format, op = vop;
  int index_num_bits = vm.Is1H() ? 3 : 2;
  if (vd.IsScalar()) {
    op |= NEONScalar | NEON_Q;
    format = SFormat(vn);
  } else {
    format = VFormat(vn);
  }
  Emit(format | op | ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) |
       Rd(vd));
}


void Assembler::NEONByElementL(const VRegister& vd,
                               const VRegister& vn,
                               const VRegister& vm,
                               int vm_index,
                               NEONByIndexedElementOp vop) {
  VIXL_ASSERT((vd.Is4S() && vn.Is4H() && vm.Is1H()) ||
              (vd.Is4S() && vn.Is8H() && vm.Is1H()) ||
              (vd.Is1S() && vn.Is1H() && vm.Is1H()) ||
              (vd.Is2D() && vn.Is2S() && vm.Is1S()) ||
              (vd.Is2D() && vn.Is4S() && vm.Is1S()) ||
              (vd.Is1D() && vn.Is1S() && vm.Is1S()));

  VIXL_ASSERT((vm.Is1H() && (vm.GetCode() < 16) && (vm_index < 8)) ||
              (vm.Is1S() && (vm_index < 4)));

  Instr format, op = vop;
  int index_num_bits = vm.Is1H() ? 3 : 2;
  if (vd.IsScalar()) {
    op |= NEONScalar | NEON_Q;
    format = SFormat(vn);
  } else {
    format = VFormat(vn);
  }
  Emit(format | op | ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) |
       Rd(vd));
}


void Assembler::sdot(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm,
                     int vm_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kDotProduct));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B() && vm.Is1S4B()) ||
              (vd.Is4S() && vn.Is16B() && vm.Is1S4B()));

  int index_num_bits = 2;
  Emit(VFormat(vd) | NEON_SDOT_byelement |
       ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::udot(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm,
                     int vm_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kDotProduct));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B() && vm.Is1S4B()) ||
              (vd.Is4S() && vn.Is16B() && vm.Is1S4B()));

  int index_num_bits = 2;
  Emit(VFormat(vd) | NEON_UDOT_byelement |
       ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) | Rd(vd));
}

void Assembler::sudot(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int vm_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kI8MM));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B() && vm.Is1S4B()) ||
              (vd.Is4S() && vn.Is16B() && vm.Is1S4B()));
  int q = vd.Is4S() ? (1U << NEONQ_offset) : 0;
  int index_num_bits = 2;
  Emit(q | 0x0f00f000 | ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) |
       Rd(vd));
}


void Assembler::usdot(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int vm_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kI8MM));
  VIXL_ASSERT((vd.Is2S() && vn.Is8B() && vm.Is1S4B()) ||
              (vd.Is4S() && vn.Is16B() && vm.Is1S4B()));
  int q = vd.Is4S() ? (1U << NEONQ_offset) : 0;
  int index_num_bits = 2;
  Emit(q | 0x0f80f000 | ImmNEONHLM(vm_index, index_num_bits) | Rm(vm) | Rn(vn) |
       Rd(vd));
}

// clang-format off
#define NEON_BYELEMENT_LIST(V)                        \
  V(mul,      NEON_MUL_byelement,      vn.IsVector()) \
  V(mla,      NEON_MLA_byelement,      vn.IsVector()) \
  V(mls,      NEON_MLS_byelement,      vn.IsVector()) \
  V(sqdmulh,  NEON_SQDMULH_byelement,  true)          \
  V(sqrdmulh, NEON_SQRDMULH_byelement, true)          \
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP, AS)                     \
  void Assembler::FN(const VRegister& vd,               \
                     const VRegister& vn,               \
                     const VRegister& vm,               \
                     int vm_index) {                    \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));            \
    VIXL_ASSERT(AS);                                    \
    NEONByElement(vd, vn, vm, vm_index, OP);            \
  }
NEON_BYELEMENT_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_BYELEMENT_RDM_LIST(V)     \
  V(sqrdmlah, NEON_SQRDMLAH_byelement) \
  V(sqrdmlsh, NEON_SQRDMLSH_byelement)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                            \
  void Assembler::FN(const VRegister& vd,                       \
                     const VRegister& vn,                       \
                     const VRegister& vm,                       \
                     int vm_index) {                            \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON, CPUFeatures::kRDM)); \
    NEONByElement(vd, vn, vm, vm_index, OP);                    \
  }
NEON_BYELEMENT_RDM_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_FPBYELEMENT_LIST(V) \
  V(fmul,  NEON_FMUL_byelement,  NEON_FMUL_H_byelement)  \
  V(fmla,  NEON_FMLA_byelement,  NEON_FMLA_H_byelement)  \
  V(fmls,  NEON_FMLS_byelement,  NEON_FMLS_H_byelement)  \
  V(fmulx, NEON_FMULX_byelement, NEON_FMULX_H_byelement)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP, OP_H)                             \
  void Assembler::FN(const VRegister& vd,                              \
                     const VRegister& vn,                              \
                     const VRegister& vm,                              \
                     int vm_index) {                                   \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));         \
    if (vd.IsLaneSizeH()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf)); \
    NEONFPByElement(vd, vn, vm, vm_index, OP, OP_H);                   \
  }
NEON_FPBYELEMENT_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_BYELEMENT_LONG_LIST(V)                               \
  V(sqdmull,  NEON_SQDMULL_byelement, vn.IsScalar() || vn.IsD())  \
  V(sqdmull2, NEON_SQDMULL_byelement, vn.IsVector() && vn.IsQ())  \
  V(sqdmlal,  NEON_SQDMLAL_byelement, vn.IsScalar() || vn.IsD())  \
  V(sqdmlal2, NEON_SQDMLAL_byelement, vn.IsVector() && vn.IsQ())  \
  V(sqdmlsl,  NEON_SQDMLSL_byelement, vn.IsScalar() || vn.IsD())  \
  V(sqdmlsl2, NEON_SQDMLSL_byelement, vn.IsVector() && vn.IsQ())  \
  V(smull,    NEON_SMULL_byelement,   vn.IsVector() && vn.IsD())  \
  V(smull2,   NEON_SMULL_byelement,   vn.IsVector() && vn.IsQ())  \
  V(umull,    NEON_UMULL_byelement,   vn.IsVector() && vn.IsD())  \
  V(umull2,   NEON_UMULL_byelement,   vn.IsVector() && vn.IsQ())  \
  V(smlal,    NEON_SMLAL_byelement,   vn.IsVector() && vn.IsD())  \
  V(smlal2,   NEON_SMLAL_byelement,   vn.IsVector() && vn.IsQ())  \
  V(umlal,    NEON_UMLAL_byelement,   vn.IsVector() && vn.IsD())  \
  V(umlal2,   NEON_UMLAL_byelement,   vn.IsVector() && vn.IsQ())  \
  V(smlsl,    NEON_SMLSL_byelement,   vn.IsVector() && vn.IsD())  \
  V(smlsl2,   NEON_SMLSL_byelement,   vn.IsVector() && vn.IsQ())  \
  V(umlsl,    NEON_UMLSL_byelement,   vn.IsVector() && vn.IsD())  \
  V(umlsl2,   NEON_UMLSL_byelement,   vn.IsVector() && vn.IsQ())
// clang-format on


#define VIXL_DEFINE_ASM_FUNC(FN, OP, AS)      \
  void Assembler::FN(const VRegister& vd,     \
                     const VRegister& vn,     \
                     const VRegister& vm,     \
                     int vm_index) {          \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));  \
    VIXL_ASSERT(AS);                          \
    NEONByElementL(vd, vn, vm, vm_index, OP); \
  }
NEON_BYELEMENT_LONG_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_BYELEMENT_FHM_LIST(V)    \
  V(fmlal, NEON_FMLAL_H_byelement)    \
  V(fmlal2, NEON_FMLAL2_H_byelement)  \
  V(fmlsl, NEON_FMLSL_H_byelement)    \
  V(fmlsl2, NEON_FMLSL2_H_byelement)
// clang-format on


#define VIXL_DEFINE_ASM_FUNC(FN, OP)                                   \
  void Assembler::FN(const VRegister& vd,                              \
                     const VRegister& vn,                              \
                     const VRegister& vm,                              \
                     int vm_index) {                                   \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON,                             \
                       CPUFeatures::kFP,                               \
                       CPUFeatures::kNEONHalf,                         \
                       CPUFeatures::kFHM));                            \
    VIXL_ASSERT((vd.Is2S() && vn.Is2H()) || (vd.Is4S() && vn.Is4H())); \
    VIXL_ASSERT(vm.IsH());                                             \
    VIXL_ASSERT((vm_index >= 0) && (vm_index < 8));                    \
    /* Vm itself can only be in the bottom 16 registers. */            \
    VIXL_ASSERT(vm.GetCode() < 16);                                    \
    Emit(FPFormat(vd) | OP | Rd(vd) | Rn(vn) | Rm(vm) |                \
         ImmNEONHLM(vm_index, 3));                                     \
  }
NEON_BYELEMENT_FHM_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC

void Assembler::suqadd(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEON2RegMisc(vd, vn, NEON_SUQADD);
}


void Assembler::usqadd(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEON2RegMisc(vd, vn, NEON_USQADD);
}


void Assembler::abs(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_ABS);
}


void Assembler::sqabs(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEON2RegMisc(vd, vn, NEON_SQABS);
}


void Assembler::neg(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEON2RegMisc(vd, vn, NEON_NEG);
}


void Assembler::sqneg(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEON2RegMisc(vd, vn, NEON_SQNEG);
}


void Assembler::NEONXtn(const VRegister& vd,
                        const VRegister& vn,
                        NEON2RegMiscOp vop) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  Instr format, op = vop;
  if (vd.IsScalar()) {
    VIXL_ASSERT((vd.Is1B() && vn.Is1H()) || (vd.Is1H() && vn.Is1S()) ||
                (vd.Is1S() && vn.Is1D()));
    op |= NEON_Q | NEONScalar;
    format = SFormat(vd);
  } else {
    VIXL_ASSERT((vd.Is8B() && vn.Is8H()) || (vd.Is4H() && vn.Is4S()) ||
                (vd.Is2S() && vn.Is2D()) || (vd.Is16B() && vn.Is8H()) ||
                (vd.Is8H() && vn.Is4S()) || (vd.Is4S() && vn.Is2D()));
    format = VFormat(vd);
  }
  Emit(format | op | Rn(vn) | Rd(vd));
}


void Assembler::xtn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() && vd.IsD());
  NEONXtn(vd, vn, NEON_XTN);
}


void Assembler::xtn2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() && vd.IsQ());
  NEONXtn(vd, vn, NEON_XTN);
}


void Assembler::sqxtn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsScalar() || vd.IsD());
  NEONXtn(vd, vn, NEON_SQXTN);
}


void Assembler::sqxtn2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() && vd.IsQ());
  NEONXtn(vd, vn, NEON_SQXTN);
}


void Assembler::sqxtun(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsScalar() || vd.IsD());
  NEONXtn(vd, vn, NEON_SQXTUN);
}


void Assembler::sqxtun2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() && vd.IsQ());
  NEONXtn(vd, vn, NEON_SQXTUN);
}


void Assembler::uqxtn(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsScalar() || vd.IsD());
  NEONXtn(vd, vn, NEON_UQXTN);
}


void Assembler::uqxtn2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() && vd.IsQ());
  NEONXtn(vd, vn, NEON_UQXTN);
}


// NEON NOT and RBIT are distinguised by bit 22, the bottom bit of "size".
void Assembler::not_(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B());
  Emit(VFormat(vd) | NEON_RBIT_NOT | Rn(vn) | Rd(vd));
}


void Assembler::rbit(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B());
  Emit(VFormat(vn) | (1 << NEONSize_offset) | NEON_RBIT_NOT | Rn(vn) | Rd(vd));
}


void Assembler::ext(const VRegister& vd,
                    const VRegister& vn,
                    const VRegister& vm,
                    int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B());
  VIXL_ASSERT((0 <= index) && (index < vd.GetLanes()));
  Emit(VFormat(vd) | NEON_EXT | Rm(vm) | ImmNEONExt(index) | Rn(vn) | Rd(vd));
}


void Assembler::dup(const VRegister& vd, const VRegister& vn, int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  Instr q, scalar;

  // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
  // number of lanes, and T is b, h, s or d.
  int lane_size = vn.GetLaneSizeInBytes();
  NEONFormatField format;
  switch (lane_size) {
    case 1:
      format = NEON_16B;
      break;
    case 2:
      format = NEON_8H;
      break;
    case 4:
      format = NEON_4S;
      break;
    default:
      VIXL_ASSERT(lane_size == 8);
      format = NEON_2D;
      break;
  }

  if (vd.IsScalar()) {
    q = NEON_Q;
    scalar = NEONScalar;
  } else {
    VIXL_ASSERT(!vd.Is1D());
    q = vd.IsD() ? 0 : NEON_Q;
    scalar = 0;
  }
  Emit(q | scalar | NEON_DUP_ELEMENT | ImmNEON5(format, vn_index) | Rn(vn) |
       Rd(vd));
}


void Assembler::mov(const VRegister& vd, const VRegister& vn, int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsScalar());
  dup(vd, vn, vn_index);
}


void Assembler::dup(const VRegister& vd, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(!vd.Is1D());
  VIXL_ASSERT(vd.Is2D() == rn.IsX());
  int q = vd.IsD() ? 0 : NEON_Q;
  Emit(q | NEON_DUP_GENERAL | ImmNEON5(VFormat(vd), 0) | Rn(rn) | Rd(vd));
}


void Assembler::ins(const VRegister& vd,
                    int vd_index,
                    const VRegister& vn,
                    int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  // We support vd arguments of the form vd.VxT() or vd.T(), where x is the
  // number of lanes, and T is b, h, s or d.
  int lane_size = vd.GetLaneSizeInBytes();
  NEONFormatField format;
  switch (lane_size) {
    case 1:
      format = NEON_16B;
      break;
    case 2:
      format = NEON_8H;
      break;
    case 4:
      format = NEON_4S;
      break;
    default:
      VIXL_ASSERT(lane_size == 8);
      format = NEON_2D;
      break;
  }

  VIXL_ASSERT(
      (0 <= vd_index) &&
      (vd_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
  VIXL_ASSERT(
      (0 <= vn_index) &&
      (vn_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
  Emit(NEON_INS_ELEMENT | ImmNEON5(format, vd_index) |
       ImmNEON4(format, vn_index) | Rn(vn) | Rd(vd));
}


void Assembler::mov(const VRegister& vd,
                    int vd_index,
                    const VRegister& vn,
                    int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  ins(vd, vd_index, vn, vn_index);
}


void Assembler::ins(const VRegister& vd, int vd_index, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  // We support vd arguments of the form vd.VxT() or vd.T(), where x is the
  // number of lanes, and T is b, h, s or d.
  int lane_size = vd.GetLaneSizeInBytes();
  NEONFormatField format;
  switch (lane_size) {
    case 1:
      format = NEON_16B;
      VIXL_ASSERT(rn.IsW());
      break;
    case 2:
      format = NEON_8H;
      VIXL_ASSERT(rn.IsW());
      break;
    case 4:
      format = NEON_4S;
      VIXL_ASSERT(rn.IsW());
      break;
    default:
      VIXL_ASSERT(lane_size == 8);
      VIXL_ASSERT(rn.IsX());
      format = NEON_2D;
      break;
  }

  VIXL_ASSERT(
      (0 <= vd_index) &&
      (vd_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
  Emit(NEON_INS_GENERAL | ImmNEON5(format, vd_index) | Rn(rn) | Rd(vd));
}


void Assembler::mov(const VRegister& vd, int vd_index, const Register& rn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  ins(vd, vd_index, rn);
}


void Assembler::umov(const Register& rd, const VRegister& vn, int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
  // number of lanes, and T is b, h, s or d.
  int lane_size = vn.GetLaneSizeInBytes();
  NEONFormatField format;
  Instr q = 0;
  switch (lane_size) {
    case 1:
      format = NEON_16B;
      VIXL_ASSERT(rd.IsW());
      break;
    case 2:
      format = NEON_8H;
      VIXL_ASSERT(rd.IsW());
      break;
    case 4:
      format = NEON_4S;
      VIXL_ASSERT(rd.IsW());
      break;
    default:
      VIXL_ASSERT(lane_size == 8);
      VIXL_ASSERT(rd.IsX());
      format = NEON_2D;
      q = NEON_Q;
      break;
  }

  VIXL_ASSERT(
      (0 <= vn_index) &&
      (vn_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
  Emit(q | NEON_UMOV | ImmNEON5(format, vn_index) | Rn(vn) | Rd(rd));
}


void Assembler::mov(const Register& rd, const VRegister& vn, int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.GetSizeInBytes() >= 4);
  umov(rd, vn, vn_index);
}


void Assembler::smov(const Register& rd, const VRegister& vn, int vn_index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
  // number of lanes, and T is b, h, s.
  int lane_size = vn.GetLaneSizeInBytes();
  NEONFormatField format;
  Instr q = 0;
  VIXL_ASSERT(lane_size != 8);
  switch (lane_size) {
    case 1:
      format = NEON_16B;
      break;
    case 2:
      format = NEON_8H;
      break;
    default:
      VIXL_ASSERT(lane_size == 4);
      VIXL_ASSERT(rd.IsX());
      format = NEON_4S;
      break;
  }
  q = rd.IsW() ? 0 : NEON_Q;
  VIXL_ASSERT(
      (0 <= vn_index) &&
      (vn_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
  Emit(q | NEON_SMOV | ImmNEON5(format, vn_index) | Rn(vn) | Rd(rd));
}


void Assembler::cls(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(!vd.Is1D() && !vd.Is2D());
  Emit(VFormat(vn) | NEON_CLS | Rn(vn) | Rd(vd));
}


void Assembler::clz(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(!vd.Is1D() && !vd.Is2D());
  Emit(VFormat(vn) | NEON_CLZ | Rn(vn) | Rd(vd));
}


void Assembler::cnt(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B());
  Emit(VFormat(vn) | NEON_CNT | Rn(vn) | Rd(vd));
}


void Assembler::rev16(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B());
  Emit(VFormat(vn) | NEON_REV16 | Rn(vn) | Rd(vd));
}


void Assembler::rev32(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is8B() || vd.Is16B() || vd.Is4H() || vd.Is8H());
  Emit(VFormat(vn) | NEON_REV32 | Rn(vn) | Rd(vd));
}


void Assembler::rev64(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(!vd.Is1D() && !vd.Is2D());
  Emit(VFormat(vn) | NEON_REV64 | Rn(vn) | Rd(vd));
}


void Assembler::ursqrte(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is2S() || vd.Is4S());
  Emit(VFormat(vn) | NEON_URSQRTE | Rn(vn) | Rd(vd));
}


void Assembler::urecpe(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(AreSameFormat(vd, vn));
  VIXL_ASSERT(vd.Is2S() || vd.Is4S());
  Emit(VFormat(vn) | NEON_URECPE | Rn(vn) | Rd(vd));
}


void Assembler::NEONAddlp(const VRegister& vd,
                          const VRegister& vn,
                          NEON2RegMiscOp op) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT((op == NEON_SADDLP) || (op == NEON_UADDLP) ||
              (op == NEON_SADALP) || (op == NEON_UADALP));

  VIXL_ASSERT((vn.Is8B() && vd.Is4H()) || (vn.Is4H() && vd.Is2S()) ||
              (vn.Is2S() && vd.Is1D()) || (vn.Is16B() && vd.Is8H()) ||
              (vn.Is8H() && vd.Is4S()) || (vn.Is4S() && vd.Is2D()));
  Emit(VFormat(vn) | op | Rn(vn) | Rd(vd));
}


void Assembler::saddlp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAddlp(vd, vn, NEON_SADDLP);
}


void Assembler::uaddlp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAddlp(vd, vn, NEON_UADDLP);
}


void Assembler::sadalp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAddlp(vd, vn, NEON_SADALP);
}


void Assembler::uadalp(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAddlp(vd, vn, NEON_UADALP);
}


void Assembler::NEONAcrossLanesL(const VRegister& vd,
                                 const VRegister& vn,
                                 NEONAcrossLanesOp op) {
  VIXL_ASSERT((vn.Is8B() && vd.Is1H()) || (vn.Is16B() && vd.Is1H()) ||
              (vn.Is4H() && vd.Is1S()) || (vn.Is8H() && vd.Is1S()) ||
              (vn.Is4S() && vd.Is1D()));
  Emit(VFormat(vn) | op | Rn(vn) | Rd(vd));
}


void Assembler::saddlv(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAcrossLanesL(vd, vn, NEON_SADDLV);
}


void Assembler::uaddlv(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONAcrossLanesL(vd, vn, NEON_UADDLV);
}


void Assembler::NEONAcrossLanes(const VRegister& vd,
                                const VRegister& vn,
                                NEONAcrossLanesOp op,
                                Instr op_half) {
  VIXL_ASSERT((vn.Is8B() && vd.Is1B()) || (vn.Is16B() && vd.Is1B()) ||
              (vn.Is4H() && vd.Is1H()) || (vn.Is8H() && vd.Is1H()) ||
              (vn.Is4S() && vd.Is1S()));
  if ((op & NEONAcrossLanesFPFMask) == NEONAcrossLanesFPFixed) {
    if (vd.Is1H()) {
      VIXL_ASSERT(op_half != 0);
      Instr vop = op_half;
      if (vn.Is8H()) {
        vop |= NEON_Q;
      }
      Emit(vop | Rn(vn) | Rd(vd));
    } else {
      Emit(FPFormat(vn) | op | Rn(vn) | Rd(vd));
    }
  } else {
    Emit(VFormat(vn) | op | Rn(vn) | Rd(vd));
  }
}

// clang-format off
#define NEON_ACROSSLANES_LIST(V)           \
  V(addv,    NEON_ADDV)                    \
  V(smaxv,   NEON_SMAXV)                   \
  V(sminv,   NEON_SMINV)                   \
  V(umaxv,   NEON_UMAXV)                   \
  V(uminv,   NEON_UMINV)
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP)                             \
  void Assembler::FN(const VRegister& vd, const VRegister& vn) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));                     \
    NEONAcrossLanes(vd, vn, OP, 0);                              \
  }
NEON_ACROSSLANES_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


// clang-format off
#define NEON_ACROSSLANES_FP_LIST(V)   \
  V(fmaxv,   NEON_FMAXV,   NEON_FMAXV_H) \
  V(fminv,   NEON_FMINV,   NEON_FMINV_H) \
  V(fmaxnmv, NEON_FMAXNMV, NEON_FMAXNMV_H) \
  V(fminnmv, NEON_FMINNMV, NEON_FMINNMV_H) \
// clang-format on

#define VIXL_DEFINE_ASM_FUNC(FN, OP, OP_H)                            \
  void Assembler::FN(const VRegister& vd, const VRegister& vn) { \
    VIXL_ASSERT(CPUHas(CPUFeatures::kFP, CPUFeatures::kNEON));   \
    if (vd.Is1H()) VIXL_ASSERT(CPUHas(CPUFeatures::kNEONHalf));  \
    VIXL_ASSERT(vd.Is1S() || vd.Is1H());                         \
    NEONAcrossLanes(vd, vn, OP, OP_H);                           \
  }
NEON_ACROSSLANES_FP_LIST(VIXL_DEFINE_ASM_FUNC)
#undef VIXL_DEFINE_ASM_FUNC


void Assembler::NEONPerm(const VRegister& vd,
                         const VRegister& vn,
                         const VRegister& vm,
                         NEONPermOp op) {
  VIXL_ASSERT(AreSameFormat(vd, vn, vm));
  VIXL_ASSERT(!vd.Is1D());
  Emit(VFormat(vd) | op | Rm(vm) | Rn(vn) | Rd(vd));
}


void Assembler::trn1(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_TRN1);
}


void Assembler::trn2(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_TRN2);
}


void Assembler::uzp1(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_UZP1);
}


void Assembler::uzp2(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_UZP2);
}


void Assembler::zip1(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_ZIP1);
}


void Assembler::zip2(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONPerm(vd, vn, vm, NEON_ZIP2);
}


void Assembler::NEONShiftImmediate(const VRegister& vd,
                                   const VRegister& vn,
                                   NEONShiftImmediateOp op,
                                   int immh_immb) {
  VIXL_ASSERT(AreSameFormat(vd, vn));
  Instr q, scalar;
  if (vn.IsScalar()) {
    q = NEON_Q;
    scalar = NEONScalar;
  } else {
    q = vd.IsD() ? 0 : NEON_Q;
    scalar = 0;
  }
  Emit(q | op | scalar | immh_immb | Rn(vn) | Rd(vd));
}


void Assembler::NEONShiftLeftImmediate(const VRegister& vd,
                                       const VRegister& vn,
                                       int shift,
                                       NEONShiftImmediateOp op) {
  int lane_size_in_bits = vn.GetLaneSizeInBits();
  VIXL_ASSERT((shift >= 0) && (shift < lane_size_in_bits));
  NEONShiftImmediate(vd, vn, op, (lane_size_in_bits + shift) << 16);
}


void Assembler::NEONShiftRightImmediate(const VRegister& vd,
                                        const VRegister& vn,
                                        int shift,
                                        NEONShiftImmediateOp op) {
  int lane_size_in_bits = vn.GetLaneSizeInBits();
  VIXL_ASSERT((shift >= 1) && (shift <= lane_size_in_bits));
  NEONShiftImmediate(vd, vn, op, ((2 * lane_size_in_bits) - shift) << 16);
}


void Assembler::NEONShiftImmediateL(const VRegister& vd,
                                    const VRegister& vn,
                                    int shift,
                                    NEONShiftImmediateOp op) {
  int lane_size_in_bits = vn.GetLaneSizeInBits();
  VIXL_ASSERT((shift >= 0) && (shift < lane_size_in_bits));
  int immh_immb = (lane_size_in_bits + shift) << 16;

  VIXL_ASSERT((vn.Is8B() && vd.Is8H()) || (vn.Is4H() && vd.Is4S()) ||
              (vn.Is2S() && vd.Is2D()) || (vn.Is16B() && vd.Is8H()) ||
              (vn.Is8H() && vd.Is4S()) || (vn.Is4S() && vd.Is2D()));
  Instr q;
  q = vn.IsD() ? 0 : NEON_Q;
  Emit(q | op | immh_immb | Rn(vn) | Rd(vd));
}


void Assembler::NEONShiftImmediateN(const VRegister& vd,
                                    const VRegister& vn,
                                    int shift,
                                    NEONShiftImmediateOp op) {
  Instr q, scalar;
  int lane_size_in_bits = vd.GetLaneSizeInBits();
  VIXL_ASSERT((shift >= 1) && (shift <= lane_size_in_bits));
  int immh_immb = (2 * lane_size_in_bits - shift) << 16;

  if (vn.IsScalar()) {
    VIXL_ASSERT((vd.Is1B() && vn.Is1H()) || (vd.Is1H() && vn.Is1S()) ||
                (vd.Is1S() && vn.Is1D()));
    q = NEON_Q;
    scalar = NEONScalar;
  } else {
    VIXL_ASSERT((vd.Is8B() && vn.Is8H()) || (vd.Is4H() && vn.Is4S()) ||
                (vd.Is2S() && vn.Is2D()) || (vd.Is16B() && vn.Is8H()) ||
                (vd.Is8H() && vn.Is4S()) || (vd.Is4S() && vn.Is2D()));
    scalar = 0;
    q = vd.IsD() ? 0 : NEON_Q;
  }
  Emit(q | op | scalar | immh_immb | Rn(vn) | Rd(vd));
}


void Assembler::shl(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftLeftImmediate(vd, vn, shift, NEON_SHL);
}


void Assembler::sli(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftLeftImmediate(vd, vn, shift, NEON_SLI);
}


void Assembler::sqshl(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONShiftLeftImmediate(vd, vn, shift, NEON_SQSHL_imm);
}


void Assembler::sqshlu(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONShiftLeftImmediate(vd, vn, shift, NEON_SQSHLU);
}


void Assembler::uqshl(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  NEONShiftLeftImmediate(vd, vn, shift, NEON_UQSHL_imm);
}


void Assembler::sshll(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsD());
  NEONShiftImmediateL(vd, vn, shift, NEON_SSHLL);
}


void Assembler::sshll2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsQ());
  NEONShiftImmediateL(vd, vn, shift, NEON_SSHLL);
}


void Assembler::sxtl(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  sshll(vd, vn, 0);
}


void Assembler::sxtl2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  sshll2(vd, vn, 0);
}


void Assembler::ushll(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsD());
  NEONShiftImmediateL(vd, vn, shift, NEON_USHLL);
}


void Assembler::ushll2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsQ());
  NEONShiftImmediateL(vd, vn, shift, NEON_USHLL);
}


void Assembler::uxtl(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  ushll(vd, vn, 0);
}


void Assembler::uxtl2(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  ushll2(vd, vn, 0);
}


void Assembler::sri(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_SRI);
}


void Assembler::sshr(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_SSHR);
}


void Assembler::ushr(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_USHR);
}


void Assembler::srshr(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_SRSHR);
}


void Assembler::urshr(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_URSHR);
}


void Assembler::ssra(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_SSRA);
}


void Assembler::usra(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_USRA);
}


void Assembler::srsra(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_SRSRA);
}


void Assembler::ursra(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsVector() || vd.Is1D());
  NEONShiftRightImmediate(vd, vn, shift, NEON_URSRA);
}


void Assembler::shrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsD());
  NEONShiftImmediateN(vd, vn, shift, NEON_SHRN);
}


void Assembler::shrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_SHRN);
}


void Assembler::rshrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsD());
  NEONShiftImmediateN(vd, vn, shift, NEON_RSHRN);
}


void Assembler::rshrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_RSHRN);
}


void Assembler::sqshrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_SQSHRN);
}


void Assembler::sqshrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_SQSHRN);
}


void Assembler::sqrshrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_SQRSHRN);
}


void Assembler::sqrshrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_SQRSHRN);
}


void Assembler::sqshrun(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_SQSHRUN);
}


void Assembler::sqshrun2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_SQSHRUN);
}


void Assembler::sqrshrun(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_SQRSHRUN);
}


void Assembler::sqrshrun2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_SQRSHRUN);
}


void Assembler::uqshrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_UQSHRN);
}


void Assembler::uqshrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_UQSHRN);
}


void Assembler::uqrshrn(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vd.IsD() || (vn.IsScalar() && vd.IsScalar()));
  NEONShiftImmediateN(vd, vn, shift, NEON_UQRSHRN);
}


void Assembler::uqrshrn2(const VRegister& vd, const VRegister& vn, int shift) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(vn.IsVector() && vd.IsQ());
  NEONShiftImmediateN(vd, vn, shift, NEON_UQRSHRN);
}

void Assembler::smmla(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kI8MM));
  VIXL_ASSERT(vd.IsLaneSizeS());
  VIXL_ASSERT(vn.IsLaneSizeB() && vm.IsLaneSizeB());

  Emit(0x4e80a400 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::usmmla(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kI8MM));
  VIXL_ASSERT(vd.IsLaneSizeS());
  VIXL_ASSERT(vn.IsLaneSizeB() && vm.IsLaneSizeB());

  Emit(0x4e80ac00 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::ummla(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kI8MM));
  VIXL_ASSERT(vd.IsLaneSizeS());
  VIXL_ASSERT(vn.IsLaneSizeB() && vm.IsLaneSizeB());

  Emit(0x6e80a400 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::bcax(const VRegister& vd, const VRegister& vn, const VRegister& vm, const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA3));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B() && vm.Is16B());

  Emit(0xce200000 | Rd(vd) | Rn(vn) | Rm(vm) | Ra(va));
}

void Assembler::eor3(const VRegister& vd, const VRegister& vn, const VRegister& vm, const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA3));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B() && vm.Is16B() && va.Is16B());

  Emit(0xce000000 | Rd(vd) | Rn(vn) | Rm(vm) | Ra(va));
}

void Assembler::xar(const VRegister& vd, const VRegister& vn, const VRegister& vm, int rotate) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA3));
  VIXL_ASSERT(vd.Is2D() && vn.Is2D() && vm.Is2D());
  VIXL_ASSERT(IsUint6(rotate));

  Emit(0xce800000 | Rd(vd) | Rn(vn) | Rm(vm) | rotate << 10);
}

void Assembler::rax1(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA3));
  VIXL_ASSERT(vd.Is2D() && vn.Is2D() && vm.Is2D());

  Emit(0xce608c00 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha1c(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(vd.IsQ() && vn.IsS() && vm.Is4S());

  Emit(0x5e000000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha1h(const VRegister& sd, const VRegister& sn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(sd.IsS() && sn.IsS());

  Emit(0x5e280800 | Rd(sd) | Rn(sn));
}

void Assembler::sha1m(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(vd.IsQ() && vn.IsS() && vm.Is4S());

  Emit(0x5e002000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha1p(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(vd.IsQ() && vn.IsS() && vm.Is4S());

  Emit(0x5e001000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha1su0(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());

  Emit(0x5e003000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha1su1(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA1));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S());

  Emit(0x5e281800 | Rd(vd) | Rn(vn));
}

void Assembler::sha256h(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA2));
  VIXL_ASSERT(vd.IsQ() && vn.IsQ() && vm.Is4S());

  Emit(0x5e004000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha256h2(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA2));
  VIXL_ASSERT(vd.IsQ() && vn.IsQ() && vm.Is4S());

  Emit(0x5e005000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha256su0(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA2));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S());

  Emit(0x5e282800 | Rd(vd) | Rn(vn));
}

void Assembler::sha256su1(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA2));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());

  Emit(0x5e006000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha512h(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA512));
  VIXL_ASSERT(vd.IsQ() && vn.IsQ() && vm.Is2D());

  Emit(0xce608000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha512h2(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA512));
  VIXL_ASSERT(vd.IsQ() && vn.IsQ() && vm.Is2D());

  Emit(0xce608400 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sha512su0(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA512));
  VIXL_ASSERT(vd.Is2D() && vn.Is2D());

  Emit(0xcec08000 | Rd(vd) | Rn(vn));
}

void Assembler::sha512su1(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSHA512));
  VIXL_ASSERT(vd.Is2D() && vn.Is2D() && vm.Is2D());

  Emit(0xce608800 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::aesd(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kAES));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B());

  Emit(0x4e285800 | Rd(vd) | Rn(vn));
}

void Assembler::aese(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kAES));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B());

  Emit(0x4e284800 | Rd(vd) | Rn(vn));
}

void Assembler::aesimc(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kAES));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B());

  Emit(0x4e287800 | Rd(vd) | Rn(vn));
}

void Assembler::aesmc(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kAES));
  VIXL_ASSERT(vd.Is16B() && vn.Is16B());

  Emit(0x4e286800 | Rd(vd) | Rn(vn));
}

void Assembler::sm3partw1(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());

  Emit(0xce60c000 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sm3partw2(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());

  Emit(0xce60c400 | Rd(vd) | Rn(vn) | Rm(vm));
}

void Assembler::sm3ss1(const VRegister& vd, const VRegister& vn, const VRegister& vm, const VRegister& va) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S() && va.Is4S());

  Emit(0xce400000 | Rd(vd) | Rn(vn) | Rm(vm) | Ra(va));
}

void Assembler::sm3tt1a(const VRegister& vd, const VRegister& vn, const VRegister& vm, int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());
  VIXL_ASSERT(IsUint2(index));

  Instr i = static_cast<uint32_t>(index) << 12;
  Emit(0xce408000 | Rd(vd) | Rn(vn) | Rm(vm) | i);
}

void Assembler::sm3tt1b(const VRegister& vd, const VRegister& vn, const VRegister& vm, int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());
  VIXL_ASSERT(IsUint2(index));

  Instr i = static_cast<uint32_t>(index) << 12;
  Emit(0xce408400 | Rd(vd) | Rn(vn) | Rm(vm) | i);
}

void Assembler::sm3tt2a(const VRegister& vd, const VRegister& vn, const VRegister& vm, int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());
  VIXL_ASSERT(IsUint2(index));

  Instr i = static_cast<uint32_t>(index) << 12;
  Emit(0xce408800 | Rd(vd) | Rn(vn) | Rm(vm) | i);
}

void Assembler::sm3tt2b(const VRegister& vd, const VRegister& vn, const VRegister& vm, int index) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM3));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());
  VIXL_ASSERT(IsUint2(index));

  Instr i = static_cast<uint32_t>(index) << 12;
  Emit(0xce408c00 | Rd(vd) | Rn(vn) | Rm(vm) | i);
}

void Assembler::sm4e(const VRegister& vd, const VRegister& vn) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM4));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S());

  Emit(0xcec08400 | Rd(vd) | Rn(vn));
}

void Assembler::sm4ekey(const VRegister& vd, const VRegister& vn, const VRegister& vm) {
  VIXL_ASSERT(CPUHas(CPUFeatures::kNEON));
  VIXL_ASSERT(CPUHas(CPUFeatures::kSM4));
  VIXL_ASSERT(vd.Is4S() && vn.Is4S() && vm.Is4S());

  Emit(0xce60c800 | Rd(vd) | Rn(vn) | Rm(vm));
}

// Note:
// For all ToImm instructions below, a difference in case
// for the same letter indicates a negated bit.
// If b is 1, then B is 0.
uint32_t Assembler::FP16ToImm8(Float16 imm) {
  VIXL_ASSERT(IsImmFP16(imm));
  // Half: aBbb.cdef.gh00.0000 (16 bits)
  uint16_t bits = Float16ToRawbits(imm);
  // bit7: a000.0000
  uint16_t bit7 = ((bits >> 15) & 0x1) << 7;
  // bit6: 0b00.0000
  uint16_t bit6 = ((bits >> 13) & 0x1) << 6;
  // bit5_to_0: 00cd.efgh
  uint16_t bit5_to_0 = (bits >> 6) & 0x3f;
  uint32_t result = static_cast<uint32_t>(bit7 | bit6 | bit5_to_0);
  return result;
}


Instr Assembler::ImmFP16(Float16 imm) {
  return FP16ToImm8(imm) << ImmFP_offset;
}


uint32_t Assembler::FP32ToImm8(float imm) {
  // bits: aBbb.bbbc.defg.h000.0000.0000.0000.0000
  uint32_t bits = FloatToRawbits(imm);
  VIXL_ASSERT(IsImmFP32(bits));
  // bit7: a000.0000
  uint32_t bit7 = ((bits >> 31) & 0x1) << 7;
  // bit6: 0b00.0000
  uint32_t bit6 = ((bits >> 29) & 0x1) << 6;
  // bit5_to_0: 00cd.efgh
  uint32_t bit5_to_0 = (bits >> 19) & 0x3f;

  return bit7 | bit6 | bit5_to_0;
}


Instr Assembler::ImmFP32(float imm) { return FP32ToImm8(imm) << ImmFP_offset; }


uint32_t Assembler::FP64ToImm8(double imm) {
  // bits: aBbb.bbbb.bbcd.efgh.0000.0000.0000.0000
  //       0000.0000.0000.0000.0000.0000.0000.0000
  uint64_t bits = DoubleToRawbits(imm);
  VIXL_ASSERT(IsImmFP64(bits));
  // bit7: a000.0000
  uint64_t bit7 = ((bits >> 63) & 0x1) << 7;
  // bit6: 0b00.0000
  uint64_t bit6 = ((bits >> 61) & 0x1) << 6;
  // bit5_to_0: 00cd.efgh
  uint64_t bit5_to_0 = (bits >> 48) & 0x3f;

  return static_cast<uint32_t>(bit7 | bit6 | bit5_to_0);
}


Instr Assembler::ImmFP64(double imm) { return FP64ToImm8(imm) << ImmFP_offset; }


// Code generation helpers.
bool Assembler::OneInstrMoveImmediateHelper(Assembler* assm,
                                            const Register& dst,
                                            uint64_t imm) {
  bool emit_code = assm != NULL;
  unsigned n, imm_s, imm_r;
  int reg_size = dst.GetSizeInBits();

  if (IsImmMovz(imm, reg_size) && !dst.IsSP()) {
    // Immediate can be represented in a move zero instruction. Movz can't write
    // to the stack pointer.
    if (emit_code) {
      assm->movz(dst, imm);
    }
    return true;
  } else if (IsImmMovn(imm, reg_size) && !dst.IsSP()) {
    // Immediate can be represented in a move negative instruction. Movn can't
    // write to the stack pointer.
    if (emit_code) {
      assm->movn(dst, dst.Is64Bits() ? ~imm : (~imm & kWRegMask));
    }
    return true;
  } else if (IsImmLogical(imm, reg_size, &n, &imm_s, &imm_r)) {
    // Immediate can be represented in a logical orr instruction.
    VIXL_ASSERT(!dst.IsZero());
    if (emit_code) {
      assm->LogicalImmediate(dst,
                             AppropriateZeroRegFor(dst),
                             n,
                             imm_s,
                             imm_r,
                             ORR);
    }
    return true;
  }
  return false;
}


void Assembler::MoveWide(const Register& rd,
                         uint64_t imm,
                         int shift,
                         MoveWideImmediateOp mov_op) {
  // Ignore the top 32 bits of an immediate if we're moving to a W register.
  if (rd.Is32Bits()) {
    // Check that the top 32 bits are zero (a positive 32-bit number) or top
    // 33 bits are one (a negative 32-bit number, sign extended to 64 bits).
    VIXL_ASSERT(((imm >> kWRegSize) == 0) ||
                ((imm >> (kWRegSize - 1)) == 0x1ffffffff));
    imm &= kWRegMask;
  }

  if (shift >= 0) {
    // Explicit shift specified.
    VIXL_ASSERT((shift == 0) || (shift == 16) || (shift == 32) ||
                (shift == 48));
    VIXL_ASSERT(rd.Is64Bits() || (shift == 0) || (shift == 16));
    shift /= 16;
  } else {
    // Calculate a new immediate and shift combination to encode the immediate
    // argument.
    VIXL_ASSERT(shift == -1);
    shift = 0;
    if ((imm & 0xffffffffffff0000) == 0) {
      // Nothing to do.
    } else if ((imm & 0xffffffff0000ffff) == 0) {
      imm >>= 16;
      shift = 1;
    } else if ((imm & 0xffff0000ffffffff) == 0) {
      VIXL_ASSERT(rd.Is64Bits());
      imm >>= 32;
      shift = 2;
    } else if ((imm & 0x0000ffffffffffff) == 0) {
      VIXL_ASSERT(rd.Is64Bits());
      imm >>= 48;
      shift = 3;
    }
  }

  VIXL_ASSERT(IsUint16(imm));

  Emit(SF(rd) | MoveWideImmediateFixed | mov_op | Rd(rd) | ImmMoveWide(imm) |
       ShiftMoveWide(shift));
}


void Assembler::AddSub(const Register& rd,
                       const Register& rn,
                       const Operand& operand,
                       FlagsUpdate S,
                       AddSubOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  if (operand.IsImmediate()) {
    int64_t immediate = operand.GetImmediate();
    VIXL_ASSERT(IsImmAddSub(immediate));
    Instr dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
    Emit(SF(rd) | AddSubImmediateFixed | op | Flags(S) |
         ImmAddSub(static_cast<int>(immediate)) | dest_reg | RnSP(rn));
  } else if (operand.IsShiftedRegister()) {
    VIXL_ASSERT(operand.GetRegister().GetSizeInBits() == rd.GetSizeInBits());
    VIXL_ASSERT(operand.GetShift() != ROR);

    // For instructions of the form:
    //   add/sub   wsp, <Wn>, <Wm> [, LSL #0-3 ]
    //   add/sub   <Wd>, wsp, <Wm> [, LSL #0-3 ]
    //   add/sub   wsp, wsp, <Wm> [, LSL #0-3 ]
    //   adds/subs <Wd>, wsp, <Wm> [, LSL #0-3 ]
    // or their 64-bit register equivalents, convert the operand from shifted to
    // extended register mode, and emit an add/sub extended instruction.
    if (rn.IsSP() || rd.IsSP()) {
      VIXL_ASSERT(!(rd.IsSP() && (S == SetFlags)));
      DataProcExtendedRegister(rd,
                               rn,
                               operand.ToExtendedRegister(),
                               S,
                               AddSubExtendedFixed | op);
    } else {
      DataProcShiftedRegister(rd, rn, operand, S, AddSubShiftedFixed | op);
    }
  } else {
    VIXL_ASSERT(operand.IsExtendedRegister());
    DataProcExtendedRegister(rd, rn, operand, S, AddSubExtendedFixed | op);
  }
}


void Assembler::AddSubWithCarry(const Register& rd,
                                const Register& rn,
                                const Operand& operand,
                                FlagsUpdate S,
                                AddSubWithCarryOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  VIXL_ASSERT(rd.GetSizeInBits() == operand.GetRegister().GetSizeInBits());
  VIXL_ASSERT(operand.IsShiftedRegister() && (operand.GetShiftAmount() == 0));
  Emit(SF(rd) | op | Flags(S) | Rm(operand.GetRegister()) | Rn(rn) | Rd(rd));
}


void Assembler::hlt(int code) {
  VIXL_ASSERT(IsUint16(code));
  Emit(HLT | ImmException(code));
}


void Assembler::brk(int code) {
  VIXL_ASSERT(IsUint16(code));
  Emit(BRK | ImmException(code));
}


void Assembler::svc(int code) { Emit(SVC | ImmException(code)); }

void Assembler::udf(int code) { Emit(UDF | ImmUdf(code)); }


// TODO(all): The third parameter should be passed by reference but gcc 4.8.2
// reports a bogus uninitialised warning then.
void Assembler::Logical(const Register& rd,
                        const Register& rn,
                        const Operand operand,
                        LogicalOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  if (operand.IsImmediate()) {
    int64_t immediate = operand.GetImmediate();
    unsigned reg_size = rd.GetSizeInBits();

    VIXL_ASSERT(immediate != 0);
    VIXL_ASSERT(immediate != -1);
    VIXL_ASSERT(rd.Is64Bits() || IsUint32(immediate));

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
      VIXL_UNREACHABLE();
    }
  } else {
    VIXL_ASSERT(operand.IsShiftedRegister());
    VIXL_ASSERT(operand.GetRegister().GetSizeInBits() == rd.GetSizeInBits());
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
  unsigned reg_size = rd.GetSizeInBits();
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
    int64_t immediate = operand.GetImmediate();
    VIXL_ASSERT(IsImmConditionalCompare(immediate));
    ccmpop = ConditionalCompareImmediateFixed | op |
             ImmCondCmp(static_cast<unsigned>(immediate));
  } else {
    VIXL_ASSERT(operand.IsShiftedRegister() && (operand.GetShiftAmount() == 0));
    ccmpop = ConditionalCompareRegisterFixed | op | Rm(operand.GetRegister());
  }
  Emit(SF(rn) | ccmpop | Cond(cond) | Rn(rn) | Nzcv(nzcv));
}


void Assembler::DataProcessing1Source(const Register& rd,
                                      const Register& rn,
                                      DataProcessing1SourceOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  Emit(SF(rn) | op | Rn(rn) | Rd(rd));
}


void Assembler::FPDataProcessing1Source(const VRegister& vd,
                                        const VRegister& vn,
                                        FPDataProcessing1SourceOp op) {
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  Emit(FPType(vn) | op | Rn(vn) | Rd(vd));
}


void Assembler::FPDataProcessing3Source(const VRegister& vd,
                                        const VRegister& vn,
                                        const VRegister& vm,
                                        const VRegister& va,
                                        FPDataProcessing3SourceOp op) {
  VIXL_ASSERT(vd.Is1H() || vd.Is1S() || vd.Is1D());
  VIXL_ASSERT(AreSameSizeAndType(vd, vn, vm, va));
  Emit(FPType(vd) | op | Rm(vm) | Rn(vn) | Rd(vd) | Ra(va));
}


void Assembler::NEONModifiedImmShiftLsl(const VRegister& vd,
                                        const int imm8,
                                        const int left_shift,
                                        NEONModifiedImmediateOp op) {
  VIXL_ASSERT(vd.Is8B() || vd.Is16B() || vd.Is4H() || vd.Is8H() || vd.Is2S() ||
              vd.Is4S());
  VIXL_ASSERT((left_shift == 0) || (left_shift == 8) || (left_shift == 16) ||
              (left_shift == 24));
  VIXL_ASSERT(IsUint8(imm8));

  int cmode_1, cmode_2, cmode_3;
  if (vd.Is8B() || vd.Is16B()) {
    VIXL_ASSERT(op == NEONModifiedImmediate_MOVI);
    cmode_1 = 1;
    cmode_2 = 1;
    cmode_3 = 1;
  } else {
    cmode_1 = (left_shift >> 3) & 1;
    cmode_2 = left_shift >> 4;
    cmode_3 = 0;
    if (vd.Is4H() || vd.Is8H()) {
      VIXL_ASSERT((left_shift == 0) || (left_shift == 8));
      cmode_3 = 1;
    }
  }
  int cmode = (cmode_3 << 3) | (cmode_2 << 2) | (cmode_1 << 1);

  int q = vd.IsQ() ? NEON_Q : 0;

  Emit(q | op | ImmNEONabcdefgh(imm8) | NEONCmode(cmode) | Rd(vd));
}


void Assembler::NEONModifiedImmShiftMsl(const VRegister& vd,
                                        const int imm8,
                                        const int shift_amount,
                                        NEONModifiedImmediateOp op) {
  VIXL_ASSERT(vd.Is2S() || vd.Is4S());
  VIXL_ASSERT((shift_amount == 8) || (shift_amount == 16));
  VIXL_ASSERT(IsUint8(imm8));

  int cmode_0 = (shift_amount >> 4) & 1;
  int cmode = 0xc | cmode_0;

  int q = vd.IsQ() ? NEON_Q : 0;

  Emit(q | op | ImmNEONabcdefgh(imm8) | NEONCmode(cmode) | Rd(vd));
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
      VIXL_UNREACHABLE();
  }
}


void Assembler::EmitExtendShift(const Register& rd,
                                const Register& rn,
                                Extend extend,
                                unsigned left_shift) {
  VIXL_ASSERT(rd.GetSizeInBits() >= rn.GetSizeInBits());
  unsigned reg_size = rd.GetSizeInBits();
  // Use the correct size of register.
  Register rn_ = Register(rn.GetCode(), rd.GetSizeInBits());
  // Bits extracted are high_bit:0.
  unsigned high_bit = (8 << (extend & 0x3)) - 1;
  // Number of bits left in the result that are not introduced by the shift.
  unsigned non_shift_bits = (reg_size - left_shift) & (reg_size - 1);

  if ((non_shift_bits > high_bit) || (non_shift_bits == 0)) {
    switch (extend) {
      case UXTB:
      case UXTH:
      case UXTW:
        ubfm(rd, rn_, non_shift_bits, high_bit);
        break;
      case SXTB:
      case SXTH:
      case SXTW:
        sbfm(rd, rn_, non_shift_bits, high_bit);
        break;
      case UXTX:
      case SXTX: {
        VIXL_ASSERT(rn.GetSizeInBits() == kXRegSize);
        // Nothing to extend. Just shift.
        lsl(rd, rn_, left_shift);
        break;
      }
      default:
        VIXL_UNREACHABLE();
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
  VIXL_ASSERT(operand.IsShiftedRegister());
  VIXL_ASSERT(rn.Is64Bits() ||
              (rn.Is32Bits() && IsUint5(operand.GetShiftAmount())));
  Emit(SF(rd) | op | Flags(S) | ShiftDP(operand.GetShift()) |
       ImmDPShift(operand.GetShiftAmount()) | Rm(operand.GetRegister()) |
       Rn(rn) | Rd(rd));
}


void Assembler::DataProcExtendedRegister(const Register& rd,
                                         const Register& rn,
                                         const Operand& operand,
                                         FlagsUpdate S,
                                         Instr op) {
  Instr dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
  Emit(SF(rd) | op | Flags(S) | Rm(operand.GetRegister()) |
       ExtendMode(operand.GetExtend()) |
       ImmExtendShift(operand.GetShiftAmount()) | dest_reg | RnSP(rn));
}


Instr Assembler::LoadStoreMemOperand(const MemOperand& addr,
                                     unsigned access_size_in_bytes_log2,
                                     LoadStoreScalingOption option) {
  Instr base = RnSP(addr.GetBaseRegister());
  int64_t offset = addr.GetOffset();

  if (addr.IsImmediateOffset()) {
    bool prefer_unscaled =
        (option == PreferUnscaledOffset) || (option == RequireUnscaledOffset);
    if (prefer_unscaled && IsImmLSUnscaled(offset)) {
      // Use the unscaled addressing mode.
      return base | LoadStoreUnscaledOffsetFixed | ImmLS(offset);
    }

    if ((option != RequireUnscaledOffset) &&
        IsImmLSScaled(offset, access_size_in_bytes_log2)) {
      // We need `offset` to be positive for the shift to be well-defined.
      // IsImmLSScaled should check this.
      VIXL_ASSERT(offset >= 0);
      // Use the scaled addressing mode.
      return base | LoadStoreUnsignedOffsetFixed |
             ImmLSUnsigned(offset >> access_size_in_bytes_log2);
    }

    if ((option != RequireScaledOffset) && IsImmLSUnscaled(offset)) {
      // Use the unscaled addressing mode.
      return base | LoadStoreUnscaledOffsetFixed | ImmLS(offset);
    }
  }

  // All remaining addressing modes are register-offset, pre-indexed or
  // post-indexed modes.
  VIXL_ASSERT((option != RequireUnscaledOffset) &&
              (option != RequireScaledOffset));

  if (addr.IsRegisterOffset()) {
    Extend ext = addr.GetExtend();
    Shift shift = addr.GetShift();
    unsigned shift_amount = addr.GetShiftAmount();

    // LSL is encoded in the option field as UXTX.
    if (shift == LSL) {
      ext = UXTX;
    }

    // Shifts are encoded in one bit, indicating a left shift by the memory
    // access size.
    VIXL_ASSERT((shift_amount == 0) || (shift_amount == access_size_in_bytes_log2));
    return base | LoadStoreRegisterOffsetFixed | Rm(addr.GetRegisterOffset()) |
           ExtendMode(ext) | ImmShiftLS((shift_amount > 0) ? 1 : 0);
  }

  if (addr.IsImmediatePreIndex() && IsImmLSUnscaled(offset)) {
    return base | LoadStorePreIndexFixed | ImmLS(offset);
  }

  if (addr.IsImmediatePostIndex() && IsImmLSUnscaled(offset)) {
    return base | LoadStorePostIndexFixed | ImmLS(offset);
  }

  // If this point is reached, the MemOperand (addr) cannot be encoded.
  VIXL_UNREACHABLE();
  return 0;
}


void Assembler::LoadStore(const CPURegister& rt,
                          const MemOperand& addr,
                          LoadStoreOp op,
                          LoadStoreScalingOption option) {
  VIXL_ASSERT(CPUHas(rt));
  Emit(op | Rt(rt) | LoadStoreMemOperand(addr, CalcLSDataSize(op), option));
}

void Assembler::LoadStorePAC(const Register& xt,
                             const MemOperand& addr,
                             LoadStorePACOp op) {
  VIXL_ASSERT(xt.Is64Bits());
  VIXL_ASSERT(addr.IsImmediateOffset() || addr.IsImmediatePreIndex());

  Instr pac_op = op;
  if (addr.IsImmediatePreIndex()) {
    pac_op |= LoadStorePACPreBit;
  }

  Instr base = RnSP(addr.GetBaseRegister());
  int64_t offset = addr.GetOffset();

  Emit(pac_op | Rt(xt) | base | ImmLSPAC(static_cast<int>(offset)));
}


void Assembler::Prefetch(int op,
                         const MemOperand& addr,
                         LoadStoreScalingOption option) {
  VIXL_ASSERT(addr.IsRegisterOffset() || addr.IsImmediateOffset());

  Instr prfop = ImmPrefetchOperation(op);
  Emit(PRFM | prfop | LoadStoreMemOperand(addr, kXRegSizeInBytesLog2, option));
}

void Assembler::Prefetch(PrefetchOperation op,
                         const MemOperand& addr,
                         LoadStoreScalingOption option) {
  // Passing unnamed values in 'op' is undefined behaviour in C++.
  VIXL_ASSERT(IsNamedPrefetchOperation(op));
  Prefetch(static_cast<int>(op), addr, option);
}


bool Assembler::IsImmAddSub(int64_t immediate) {
  return IsUint12(immediate) ||
         (IsUint12(immediate >> 12) && ((immediate & 0xfff) == 0));
}


bool Assembler::IsImmConditionalCompare(int64_t immediate) {
  return IsUint5(immediate);
}


bool Assembler::IsImmFP16(Float16 imm) {
  // Valid values will have the form:
  // aBbb.cdef.gh00.000
  uint16_t bits = Float16ToRawbits(imm);
  // bits[6..0] are cleared.
  if ((bits & 0x3f) != 0) {
    return false;
  }

  // bits[13..12] are all set or all cleared.
  uint16_t b_pattern = (bits >> 12) & 0x03;
  if (b_pattern != 0 && b_pattern != 0x03) {
    return false;
  }

  // bit[15] and bit[14] are opposite.
  if (((bits ^ (bits << 1)) & 0x4000) == 0) {
    return false;
  }

  return true;
}


bool Assembler::IsImmFP32(uint32_t bits) {
  // Valid values will have the form:
  // aBbb.bbbc.defg.h000.0000.0000.0000.0000
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


bool Assembler::IsImmFP64(uint64_t bits) {
  // Valid values will have the form:
  // aBbb.bbbb.bbcd.efgh.0000.0000.0000.0000
  // 0000.0000.0000.0000.0000.0000.0000.0000
  // bits[47..0] are cleared.
  if ((bits & 0x0000ffffffffffff) != 0) {
    return false;
  }

  // bits[61..54] are all set or all cleared.
  uint32_t b_pattern = (bits >> 48) & 0x3fc0;
  if ((b_pattern != 0) && (b_pattern != 0x3fc0)) {
    return false;
  }

  // bit[62] and bit[61] are opposite.
  if (((bits ^ (bits << 1)) & (UINT64_C(1) << 62)) == 0) {
    return false;
  }

  return true;
}


bool Assembler::IsImmLSPair(int64_t offset, unsigned access_size_in_bytes_log2) {
  const auto access_size_in_bytes = 1U << access_size_in_bytes_log2;
  VIXL_ASSERT(access_size_in_bytes_log2 <= kQRegSizeInBytesLog2);
  return IsMultiple(offset, access_size_in_bytes) &&
         IsInt7(offset / access_size_in_bytes);
}


bool Assembler::IsImmLSScaled(int64_t offset, unsigned access_size_in_bytes_log2) {
  const auto access_size_in_bytes = 1U << access_size_in_bytes_log2;
  VIXL_ASSERT(access_size_in_bytes_log2 <= kQRegSizeInBytesLog2);
  return IsMultiple(offset, access_size_in_bytes) &&
         IsUint12(offset / access_size_in_bytes);
}


bool Assembler::IsImmLSUnscaled(int64_t offset) { return IsInt9(offset); }


// The movn instruction can generate immediates containing an arbitrary 16-bit
// value, with remaining bits set, eg. 0xffff1234, 0xffff1234ffffffff.
bool Assembler::IsImmMovn(uint64_t imm, unsigned reg_size) {
  return IsImmMovz(~imm, reg_size);
}


// The movz instruction can generate immediates containing an arbitrary 16-bit
// value, with remaining bits clear, eg. 0x00001234, 0x0000123400000000.
bool Assembler::IsImmMovz(uint64_t imm, unsigned reg_size) {
  VIXL_ASSERT((reg_size == kXRegSize) || (reg_size == kWRegSize));
  return CountClearHalfWords(imm, reg_size) >= ((reg_size / 16) - 1);
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
  VIXL_ASSERT((width == kBRegSize) || (width == kHRegSize) ||
              (width == kSRegSize) || (width == kDRegSize));

  bool negate = false;

  // Logical immediates are encoded using parameters n, imm_s and imm_r using
  // the following table:
  //
  //    N   imms    immr    size        S             R
  //    1  ssssss  rrrrrr    64    UInt(ssssss)  UInt(rrrrrr)
  //    0  0sssss  xrrrrr    32    UInt(sssss)   UInt(rrrrr)
  //    0  10ssss  xxrrrr    16    UInt(ssss)    UInt(rrrr)
  //    0  110sss  xxxrrr     8    UInt(sss)     UInt(rrr)
  //    0  1110ss  xxxxrr     4    UInt(ss)      UInt(rr)
  //    0  11110s  xxxxxr     2    UInt(s)       UInt(r)
  // (s bits must not be all set)
  //
  // A pattern is constructed of size bits, where the least significant S+1 bits
  // are set. The pattern is rotated right by R, and repeated across a 32 or
  // 64-bit value, depending on destination register width.
  //
  // Put another way: the basic format of a logical immediate is a single
  // contiguous stretch of 1 bits, repeated across the whole word at intervals
  // given by a power of 2. To identify them quickly, we first locate the
  // lowest stretch of 1 bits, then the next 1 bit above that; that combination
  // is different for every logical immediate, so it gives us all the
  // information we need to identify the only logical immediate that our input
  // could be, and then we simply check if that's the value we actually have.
  //
  // (The rotation parameter does give the possibility of the stretch of 1 bits
  // going 'round the end' of the word. To deal with that, we observe that in
  // any situation where that happens the bitwise NOT of the value is also a
  // valid logical immediate. So we simply invert the input whenever its low bit
  // is set, and then we know that the rotated case can't arise.)

  if (value & 1) {
    // If the low bit is 1, negate the value, and set a flag to remember that we
    // did (so that we can adjust the return values appropriately).
    negate = true;
    value = ~value;
  }

  if (width <= kWRegSize) {
    // To handle 8/16/32-bit logical immediates, the very easiest thing is to repeat
    // the input value to fill a 64-bit word. The correct encoding of that as a
    // logical immediate will also be the correct encoding of the value.

    // Avoid making the assumption that the most-significant 56/48/32 bits are zero by
    // shifting the value left and duplicating it.
    for (unsigned bits = width; bits <= kWRegSize; bits *= 2) {
      value <<= bits;
      uint64_t mask = (UINT64_C(1) << bits) - 1;
      value |= ((value >> bits) & mask);
    }
  }

  // The basic analysis idea: imagine our input word looks like this.
  //
  //    0011111000111110001111100011111000111110001111100011111000111110
  //                                                          c  b    a
  //                                                          |<--d-->|
  //
  // We find the lowest set bit (as an actual power-of-2 value, not its index)
  // and call it a. Then we add a to our original number, which wipes out the
  // bottommost stretch of set bits and replaces it with a 1 carried into the
  // next zero bit. Then we look for the new lowest set bit, which is in
  // position b, and subtract it, so now our number is just like the original
  // but with the lowest stretch of set bits completely gone. Now we find the
  // lowest set bit again, which is position c in the diagram above. Then we'll
  // measure the distance d between bit positions a and c (using CLZ), and that
  // tells us that the only valid logical immediate that could possibly be equal
  // to this number is the one in which a stretch of bits running from a to just
  // below b is replicated every d bits.
  uint64_t a = LowestSetBit(value);
  uint64_t value_plus_a = value + a;
  uint64_t b = LowestSetBit(value_plus_a);
  uint64_t value_plus_a_minus_b = value_plus_a - b;
  uint64_t c = LowestSetBit(value_plus_a_minus_b);

  int d, clz_a, out_n;
  uint64_t mask;

  if (c != 0) {
    // The general case, in which there is more than one stretch of set bits.
    // Compute the repeat distance d, and set up a bitmask covering the basic
    // unit of repetition (i.e. a word with the bottom d bits set). Also, in all
    // of these cases the N bit of the output will be zero.
    clz_a = CountLeadingZeros(a, kXRegSize);
    int clz_c = CountLeadingZeros(c, kXRegSize);
    d = clz_a - clz_c;
    mask = ((UINT64_C(1) << d) - 1);
    out_n = 0;
  } else {
    // Handle degenerate cases.
    //
    // If any of those 'find lowest set bit' operations didn't find a set bit at
    // all, then the word will have been zero thereafter, so in particular the
    // last lowest_set_bit operation will have returned zero. So we can test for
    // all the special case conditions in one go by seeing if c is zero.
    if (a == 0) {
      // The input was zero (or all 1 bits, which will come to here too after we
      // inverted it at the start of the function), for which we just return
      // false.
      return false;
    } else {
      // Otherwise, if c was zero but a was not, then there's just one stretch
      // of set bits in our word, meaning that we have the trivial case of
      // d == 64 and only one 'repetition'. Set up all the same variables as in
      // the general case above, and set the N bit in the output.
      clz_a = CountLeadingZeros(a, kXRegSize);
      d = 64;
      mask = ~UINT64_C(0);
      out_n = 1;
    }
  }

  // If the repeat period d is not a power of two, it can't be encoded.
  if (!IsPowerOf2(d)) {
    return false;
  }

  if (((b - a) & ~mask) != 0) {
    // If the bit stretch (b - a) does not fit within the mask derived from the
    // repeat period, then fail.
    return false;
  }

  // The only possible option is b - a repeated every d bits. Now we're going to
  // actually construct the valid logical immediate derived from that
  // specification, and see if it equals our original input.
  //
  // To repeat a value every d bits, we multiply it by a number of the form
  // (1 + 2^d + 2^(2d) + ...), i.e. 0x0001000100010001 or similar. These can
  // be derived using a table lookup on CLZ(d).
  static const uint64_t multipliers[] = {
      0x0000000000000001UL,
      0x0000000100000001UL,
      0x0001000100010001UL,
      0x0101010101010101UL,
      0x1111111111111111UL,
      0x5555555555555555UL,
  };
  uint64_t multiplier = multipliers[CountLeadingZeros(d, kXRegSize) - 57];
  uint64_t candidate = (b - a) * multiplier;

  if (value != candidate) {
    // The candidate pattern doesn't match our input value, so fail.
    return false;
  }

  // We have a match! This is a valid logical immediate, so now we have to
  // construct the bits and pieces of the instruction encoding that generates
  // it.

  // Count the set bits in our basic stretch. The special case of clz(0) == -1
  // makes the answer come out right for stretches that reach the very top of
  // the word (e.g. numbers like 0xffffc00000000000).
  int clz_b = (b == 0) ? -1 : CountLeadingZeros(b, kXRegSize);
  int s = clz_a - clz_b;

  // Decide how many bits to rotate right by, to put the low bit of that basic
  // stretch in position a.
  int r;
  if (negate) {
    // If we inverted the input right at the start of this function, here's
    // where we compensate: the number of set bits becomes the number of clear
    // bits, and the rotation count is based on position b rather than position
    // a (since b is the location of the 'lowest' 1 bit after inversion).
    s = d - s;
    r = (clz_b + 1) & (d - 1);
  } else {
    r = (clz_a + 1) & (d - 1);
  }

  // Now we're done, except for having to encode the S output in such a way that
  // it gives both the number of set bits and the length of the repeated
  // segment. The s field is encoded like this:
  //
  //     imms    size        S
  //    ssssss    64    UInt(ssssss)
  //    0sssss    32    UInt(sssss)
  //    10ssss    16    UInt(ssss)
  //    110sss     8    UInt(sss)
  //    1110ss     4    UInt(ss)
  //    11110s     2    UInt(s)
  //
  // So we 'or' (2 * -d) with our computed s to form imms.
  if ((n != NULL) || (imm_s != NULL) || (imm_r != NULL)) {
    *n = out_n;
    *imm_s = ((2 * -d) | (s - 1)) & 0x3f;
    *imm_r = r;
  }

  return true;
}


LoadStoreOp Assembler::LoadOpFor(const CPURegister& rt) {
  VIXL_ASSERT(rt.IsValid());
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? LDR_x : LDR_w;
  } else {
    VIXL_ASSERT(rt.IsVRegister());
    switch (rt.GetSizeInBits()) {
      case kBRegSize:
        return LDR_b;
      case kHRegSize:
        return LDR_h;
      case kSRegSize:
        return LDR_s;
      case kDRegSize:
        return LDR_d;
      default:
        VIXL_ASSERT(rt.IsQ());
        return LDR_q;
    }
  }
}


LoadStoreOp Assembler::StoreOpFor(const CPURegister& rt) {
  VIXL_ASSERT(rt.IsValid());
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STR_x : STR_w;
  } else {
    VIXL_ASSERT(rt.IsVRegister());
    switch (rt.GetSizeInBits()) {
      case kBRegSize:
        return STR_b;
      case kHRegSize:
        return STR_h;
      case kSRegSize:
        return STR_s;
      case kDRegSize:
        return STR_d;
      default:
        VIXL_ASSERT(rt.IsQ());
        return STR_q;
    }
  }
}


LoadStorePairOp Assembler::StorePairOpFor(const CPURegister& rt,
                                          const CPURegister& rt2) {
  VIXL_ASSERT(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STP_x : STP_w;
  } else {
    VIXL_ASSERT(rt.IsVRegister());
    switch (rt.GetSizeInBytes()) {
      case kSRegSizeInBytes:
        return STP_s;
      case kDRegSizeInBytes:
        return STP_d;
      default:
        VIXL_ASSERT(rt.IsQ());
        return STP_q;
    }
  }
}


LoadStorePairOp Assembler::LoadPairOpFor(const CPURegister& rt,
                                         const CPURegister& rt2) {
  VIXL_ASSERT((STP_w | LoadStorePairLBit) == LDP_w);
  return static_cast<LoadStorePairOp>(StorePairOpFor(rt, rt2) |
                                      LoadStorePairLBit);
}


LoadStorePairNonTemporalOp Assembler::StorePairNonTemporalOpFor(
    const CPURegister& rt, const CPURegister& rt2) {
  VIXL_ASSERT(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  if (rt.IsRegister()) {
    return rt.Is64Bits() ? STNP_x : STNP_w;
  } else {
    VIXL_ASSERT(rt.IsVRegister());
    switch (rt.GetSizeInBytes()) {
      case kSRegSizeInBytes:
        return STNP_s;
      case kDRegSizeInBytes:
        return STNP_d;
      default:
        VIXL_ASSERT(rt.IsQ());
        return STNP_q;
    }
  }
}


LoadStorePairNonTemporalOp Assembler::LoadPairNonTemporalOpFor(
    const CPURegister& rt, const CPURegister& rt2) {
  VIXL_ASSERT((STNP_w | LoadStorePairNonTemporalLBit) == LDNP_w);
  return static_cast<LoadStorePairNonTemporalOp>(
      StorePairNonTemporalOpFor(rt, rt2) | LoadStorePairNonTemporalLBit);
}


LoadLiteralOp Assembler::LoadLiteralOpFor(const CPURegister& rt) {
  if (rt.IsRegister()) {
    return rt.IsX() ? LDR_x_lit : LDR_w_lit;
  } else {
    VIXL_ASSERT(rt.IsVRegister());
    switch (rt.GetSizeInBytes()) {
      case kSRegSizeInBytes:
        return LDR_s_lit;
      case kDRegSizeInBytes:
        return LDR_d_lit;
      default:
        VIXL_ASSERT(rt.IsQ());
        return LDR_q_lit;
    }
  }
}


bool Assembler::CPUHas(const CPURegister& rt) const {
  // Core registers are available without any particular CPU features.
  if (rt.IsRegister()) return true;
  VIXL_ASSERT(rt.IsVRegister());
  // The architecture does not allow FP and NEON to be implemented separately,
  // but we can crudely categorise them based on register size, since FP only
  // uses D, S and (occasionally) H registers.
  if (rt.IsH() || rt.IsS() || rt.IsD()) {
    return CPUHas(CPUFeatures::kFP) || CPUHas(CPUFeatures::kNEON);
  }
  VIXL_ASSERT(rt.IsB() || rt.IsQ());
  return CPUHas(CPUFeatures::kNEON);
}


bool Assembler::CPUHas(const CPURegister& rt, const CPURegister& rt2) const {
  // This is currently only used for loads and stores, where rt and rt2 must
  // have the same size and type. We could extend this to cover other cases if
  // necessary, but for now we can avoid checking both registers.
  VIXL_ASSERT(AreSameSizeAndType(rt, rt2));
  USE(rt2);
  return CPUHas(rt);
}


bool Assembler::CPUHas(SystemRegister sysreg) const {
  switch (sysreg) {
    case RNDR:
    case RNDRRS:
      return CPUHas(CPUFeatures::kRNG);
    case FPCR:
    case NZCV:
    case DCZID_EL0:
      break;
  }
  return true;
}


}  // namespace aarch64
}  // namespace vixl
