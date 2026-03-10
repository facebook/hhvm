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

#include "macro-assembler-aarch64.h"

#include <cctype>

namespace vixl {
namespace aarch64 {


void Pool::Release() {
  if (--monitor_ == 0) {
    // Ensure the pool has not been blocked for too long.
    VIXL_ASSERT(masm_->GetCursorOffset() < checkpoint_);
  }
}


void Pool::SetNextCheckpoint(ptrdiff_t checkpoint) {
  masm_->checkpoint_ = std::min(masm_->checkpoint_, checkpoint);
  checkpoint_ = checkpoint;
}


LiteralPool::LiteralPool(MacroAssembler* masm)
    : Pool(masm),
      size_(0),
      first_use_(-1),
      recommended_checkpoint_(kNoCheckpointRequired) {}


LiteralPool::~LiteralPool() VIXL_NEGATIVE_TESTING_ALLOW_EXCEPTION {
  VIXL_ASSERT(IsEmpty());
  VIXL_ASSERT(!IsBlocked());
  for (std::vector<RawLiteral*>::iterator it = deleted_on_destruction_.begin();
       it != deleted_on_destruction_.end();
       it++) {
    delete *it;
  }
}


void LiteralPool::Reset() {
  std::vector<RawLiteral*>::iterator it, end;
  for (it = entries_.begin(), end = entries_.end(); it != end; ++it) {
    RawLiteral* literal = *it;
    if (literal->deletion_policy_ == RawLiteral::kDeletedOnPlacementByPool) {
      delete literal;
    }
  }
  entries_.clear();
  size_ = 0;
  first_use_ = -1;
  Pool::Reset();
  recommended_checkpoint_ = kNoCheckpointRequired;
}


void LiteralPool::CheckEmitFor(size_t amount, EmitOption option) {
  if (IsEmpty() || IsBlocked()) return;

  ptrdiff_t distance = masm_->GetCursorOffset() + amount - first_use_;
  if (distance >= kRecommendedLiteralPoolRange) {
    Emit(option);
  }
}


void LiteralPool::CheckEmitForBranch(size_t range) {
  if (IsEmpty() || IsBlocked()) return;
  if (GetMaxSize() >= range) Emit();
}

// We use a subclass to access the protected `ExactAssemblyScope` constructor
// giving us control over the pools. This allows us to use this scope within
// code emitting pools without creating a circular dependency.
// We keep the constructor private to restrict usage of this helper class.
class ExactAssemblyScopeWithoutPoolsCheck : public ExactAssemblyScope {
 private:
  ExactAssemblyScopeWithoutPoolsCheck(MacroAssembler* masm, size_t size)
      : ExactAssemblyScope(masm,
                           size,
                           ExactAssemblyScope::kExactSize,
                           ExactAssemblyScope::kIgnorePools) {}

  friend void LiteralPool::Emit(LiteralPool::EmitOption);
  friend void VeneerPool::Emit(VeneerPool::EmitOption, size_t);
};


void LiteralPool::Emit(EmitOption option) {
  // There is an issue if we are asked to emit a blocked or empty pool.
  VIXL_ASSERT(!IsBlocked());
  VIXL_ASSERT(!IsEmpty());

  size_t pool_size = GetSize();
  size_t emit_size = pool_size;
  if (option == kBranchRequired) emit_size += kInstructionSize;
  Label end_of_pool;

  VIXL_ASSERT(emit_size % kInstructionSize == 0);
  {
    CodeBufferCheckScope guard(masm_,
                               emit_size,
                               CodeBufferCheckScope::kCheck,
                               CodeBufferCheckScope::kExactSize);
#ifdef VIXL_DEBUG
    // Also explicitly disallow usage of the `MacroAssembler` here.
    masm_->SetAllowMacroInstructions(false);
#endif
    if (option == kBranchRequired) {
      ExactAssemblyScopeWithoutPoolsCheck eas_guard(masm_, kInstructionSize);
      masm_->b(&end_of_pool);
    }

    {
      // Marker indicating the size of the literal pool in 32-bit words.
      VIXL_ASSERT((pool_size % kWRegSizeInBytes) == 0);
      ExactAssemblyScopeWithoutPoolsCheck eas_guard(masm_, kInstructionSize);
      masm_->ldr(xzr, static_cast<int>(pool_size / kWRegSizeInBytes));
    }

    // Now populate the literal pool.
    std::vector<RawLiteral*>::iterator it, end;
    for (it = entries_.begin(), end = entries_.end(); it != end; ++it) {
      VIXL_ASSERT((*it)->IsUsed());
      masm_->place(*it);
    }

    if (option == kBranchRequired) masm_->bind(&end_of_pool);
#ifdef VIXL_DEBUG
    masm_->SetAllowMacroInstructions(true);
#endif
  }

  Reset();
}


void LiteralPool::AddEntry(RawLiteral* literal) {
  // A literal must be registered immediately before its first use. Here we
  // cannot control that it is its first use, but we check no code has been
  // emitted since its last use.
  VIXL_ASSERT(masm_->GetCursorOffset() == literal->GetLastUse());

  UpdateFirstUse(masm_->GetCursorOffset());
  VIXL_ASSERT(masm_->GetCursorOffset() >= first_use_);
  entries_.push_back(literal);
  size_ += literal->GetSize();
}


void LiteralPool::UpdateFirstUse(ptrdiff_t use_position) {
  first_use_ = std::min(first_use_, use_position);
  if (first_use_ == -1) {
    first_use_ = use_position;
    SetNextRecommendedCheckpoint(GetNextRecommendedCheckpoint());
    SetNextCheckpoint(first_use_ + Instruction::kLoadLiteralRange);
  } else {
    VIXL_ASSERT(use_position > first_use_);
  }
}


void VeneerPool::Reset() {
  Pool::Reset();
  unresolved_branches_.Reset();
}


void VeneerPool::Release() {
  if (--monitor_ == 0) {
    VIXL_ASSERT(IsEmpty() || masm_->GetCursorOffset() <
                                 unresolved_branches_.GetFirstLimit());
  }
}


void VeneerPool::RegisterUnresolvedBranch(ptrdiff_t branch_pos,
                                          Label* label,
                                          ImmBranchType branch_type) {
  VIXL_ASSERT(!label->IsBound());
  BranchInfo branch_info = BranchInfo(branch_pos, label, branch_type);
  unresolved_branches_.insert(branch_info);
  UpdateNextCheckPoint();
  // TODO: In debug mode register the label with the assembler to make sure it
  // is bound with masm Bind and not asm bind.
}


void VeneerPool::DeleteUnresolvedBranchInfoForLabel(Label* label) {
  if (IsEmpty()) {
    VIXL_ASSERT(checkpoint_ == kNoCheckpointRequired);
    return;
  }

  if (label->IsLinked()) {
    Label::LabelLinksIterator links_it(label);
    for (; !links_it.Done(); links_it.Advance()) {
      ptrdiff_t link_offset = *links_it.Current();
      Instruction* link = masm_->GetInstructionAt(link_offset);

      // ADR instructions are not handled.
      if (BranchTypeUsesVeneers(link->GetBranchType())) {
        BranchInfo branch_info(link_offset, label, link->GetBranchType());
        unresolved_branches_.erase(branch_info);
      }
    }
  }

  UpdateNextCheckPoint();
}


bool VeneerPool::ShouldEmitVeneer(int64_t first_unreacheable_pc,
                                  size_t amount) {
  ptrdiff_t offset =
      kPoolNonVeneerCodeSize + amount + GetMaxSize() + GetOtherPoolsMaxSize();
  return (masm_->GetCursorOffset() + offset) > first_unreacheable_pc;
}


void VeneerPool::CheckEmitFor(size_t amount, EmitOption option) {
  if (IsEmpty()) return;

  VIXL_ASSERT(masm_->GetCursorOffset() + kPoolNonVeneerCodeSize <
              unresolved_branches_.GetFirstLimit());

  if (IsBlocked()) return;

  if (ShouldEmitVeneers(amount)) {
    Emit(option, amount);
  } else {
    UpdateNextCheckPoint();
  }
}


void VeneerPool::Emit(EmitOption option, size_t amount) {
  // There is an issue if we are asked to emit a blocked or empty pool.
  VIXL_ASSERT(!IsBlocked());
  VIXL_ASSERT(!IsEmpty());

  Label end;
  if (option == kBranchRequired) {
    ExactAssemblyScopeWithoutPoolsCheck guard(masm_, kInstructionSize);
    masm_->b(&end);
  }

  // We want to avoid generating veneer pools too often, so generate veneers for
  // branches that don't immediately require a veneer but will soon go out of
  // range.
  static const size_t kVeneerEmissionMargin = 1 * KBytes;

  for (BranchInfoSetIterator it(&unresolved_branches_); !it.Done();) {
    BranchInfo* branch_info = it.Current();
    if (ShouldEmitVeneer(branch_info->first_unreacheable_pc_,
                         amount + kVeneerEmissionMargin)) {
      CodeBufferCheckScope scope(masm_,
                                 kVeneerCodeSize,
                                 CodeBufferCheckScope::kCheck,
                                 CodeBufferCheckScope::kExactSize);
      ptrdiff_t branch_pos = branch_info->pc_offset_;
      Instruction* branch = masm_->GetInstructionAt(branch_pos);
      Label* label = branch_info->label_;

      // Patch the branch to point to the current position, and emit a branch
      // to the label.
      Instruction* veneer = masm_->GetCursorAddress<Instruction*>();
      branch->SetImmPCOffsetTarget(veneer);
      {
        ExactAssemblyScopeWithoutPoolsCheck guard(masm_, kInstructionSize);
        masm_->b(label);
      }

      // Update the label. The branch patched does not point to it any longer.
      label->DeleteLink(branch_pos);

      it.DeleteCurrentAndAdvance();
    } else {
      it.AdvanceToNextType();
    }
  }

  UpdateNextCheckPoint();

  masm_->bind(&end);
}


MacroAssembler::MacroAssembler(PositionIndependentCodeOption pic)
    : Assembler(pic),
#ifdef VIXL_DEBUG
      allow_macro_instructions_(true),
#endif
      generate_simulator_code_(VIXL_AARCH64_GENERATE_SIMULATOR_CODE),
      sp_(sp),
      tmp_list_(ip0, ip1),
      v_tmp_list_(d31),
      p_tmp_list_(CPURegList::Empty(CPURegister::kPRegister)),
      current_scratch_scope_(NULL),
      literal_pool_(this),
      veneer_pool_(this),
      recommended_checkpoint_(Pool::kNoCheckpointRequired),
      fp_nan_propagation_(NoFPMacroNaNPropagationSelected) {
  checkpoint_ = GetNextCheckPoint();
#ifndef VIXL_DEBUG
  USE(allow_macro_instructions_);
#endif
}


MacroAssembler::MacroAssembler(size_t capacity,
                               PositionIndependentCodeOption pic)
    : Assembler(capacity, pic),
#ifdef VIXL_DEBUG
      allow_macro_instructions_(true),
#endif
      generate_simulator_code_(VIXL_AARCH64_GENERATE_SIMULATOR_CODE),
      sp_(sp),
      tmp_list_(ip0, ip1),
      v_tmp_list_(d31),
      p_tmp_list_(CPURegList::Empty(CPURegister::kPRegister)),
      current_scratch_scope_(NULL),
      literal_pool_(this),
      veneer_pool_(this),
      recommended_checkpoint_(Pool::kNoCheckpointRequired),
      fp_nan_propagation_(NoFPMacroNaNPropagationSelected) {
  checkpoint_ = GetNextCheckPoint();
}


MacroAssembler::MacroAssembler(byte* buffer,
                               size_t capacity,
                               PositionIndependentCodeOption pic)
    : Assembler(buffer, capacity, pic),
#ifdef VIXL_DEBUG
      allow_macro_instructions_(true),
#endif
      generate_simulator_code_(VIXL_AARCH64_GENERATE_SIMULATOR_CODE),
      sp_(sp),
      tmp_list_(ip0, ip1),
      v_tmp_list_(d31),
      p_tmp_list_(CPURegList::Empty(CPURegister::kPRegister)),
      current_scratch_scope_(NULL),
      literal_pool_(this),
      veneer_pool_(this),
      recommended_checkpoint_(Pool::kNoCheckpointRequired),
      fp_nan_propagation_(NoFPMacroNaNPropagationSelected) {
  checkpoint_ = GetNextCheckPoint();
}


MacroAssembler::~MacroAssembler() {}


void MacroAssembler::Reset() {
  Assembler::Reset();

  VIXL_ASSERT(!literal_pool_.IsBlocked());
  literal_pool_.Reset();
  veneer_pool_.Reset();

  checkpoint_ = GetNextCheckPoint();
}


void MacroAssembler::FinalizeCode(FinalizeOption option) {
  if (!literal_pool_.IsEmpty()) {
    // The user may decide to emit more code after Finalize, emit a branch if
    // that's the case.
    literal_pool_.Emit(option == kUnreachable ? Pool::kNoBranchRequired
                                              : Pool::kBranchRequired);
  }
  VIXL_ASSERT(veneer_pool_.IsEmpty());

  Assembler::FinalizeCode();
}


void MacroAssembler::CheckEmitFor(size_t amount) {
  CheckEmitPoolsFor(amount);
  GetBuffer()->EnsureSpaceFor(amount);
}


void MacroAssembler::CheckEmitPoolsFor(size_t amount) {
  literal_pool_.CheckEmitFor(amount);
  veneer_pool_.CheckEmitFor(amount);
  checkpoint_ = GetNextCheckPoint();
}


int MacroAssembler::MoveImmediateHelper(MacroAssembler* masm,
                                        const Register& rd,
                                        uint64_t imm) {
  bool emit_code = (masm != NULL);
  VIXL_ASSERT(IsUint32(imm) || IsInt32(imm) || rd.Is64Bits());
  // The worst case for size is mov 64-bit immediate to sp:
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction to move to sp
  MacroEmissionCheckScope guard(masm);

  // Immediates on Aarch64 can be produced using an initial value, and zero to
  // three move keep operations.
  //
  // Initial values can be generated with:
  //  1. 64-bit move zero (movz).
  //  2. 32-bit move inverted (movn).
  //  3. 64-bit move inverted.
  //  4. 32-bit orr immediate.
  //  5. 64-bit orr immediate.
  // Move-keep may then be used to modify each of the 16-bit half words.
  //
  // The code below supports all five initial value generators, and
  // applying move-keep operations to move-zero and move-inverted initial
  // values.

  // Try to move the immediate in one instruction, and if that fails, switch to
  // using multiple instructions.
  if (OneInstrMoveImmediateHelper(masm, rd, imm)) {
    return 1;
  } else {
    int instruction_count = 0;
    unsigned reg_size = rd.GetSizeInBits();

    // Generic immediate case. Imm will be represented by
    //   [imm3, imm2, imm1, imm0], where each imm is 16 bits.
    // A move-zero or move-inverted is generated for the first non-zero or
    // non-0xffff immX, and a move-keep for subsequent non-zero immX.

    uint64_t ignored_halfword = 0;
    bool invert_move = false;
    // If the number of 0xffff halfwords is greater than the number of 0x0000
    // halfwords, it's more efficient to use move-inverted.
    if (CountClearHalfWords(~imm, reg_size) >
        CountClearHalfWords(imm, reg_size)) {
      ignored_halfword = 0xffff;
      invert_move = true;
    }

    // Mov instructions can't move values into the stack pointer, so set up a
    // temporary register, if needed.
    UseScratchRegisterScope temps;
    Register temp;
    if (emit_code) {
      temps.Open(masm);
      temp = rd.IsSP() ? temps.AcquireSameSizeAs(rd) : rd;
    }

    // Iterate through the halfwords. Use movn/movz for the first non-ignored
    // halfword, and movk for subsequent halfwords.
    VIXL_ASSERT((reg_size % 16) == 0);
    bool first_mov_done = false;
    for (unsigned i = 0; i < (reg_size / 16); i++) {
      uint64_t imm16 = (imm >> (16 * i)) & 0xffff;
      if (imm16 != ignored_halfword) {
        if (!first_mov_done) {
          if (invert_move) {
            if (emit_code) masm->movn(temp, ~imm16 & 0xffff, 16 * i);
            instruction_count++;
          } else {
            if (emit_code) masm->movz(temp, imm16, 16 * i);
            instruction_count++;
          }
          first_mov_done = true;
        } else {
          // Construct a wider constant.
          if (emit_code) masm->movk(temp, imm16, 16 * i);
          instruction_count++;
        }
      }
    }

    VIXL_ASSERT(first_mov_done);

    // Move the temporary if the original destination register was the stack
    // pointer.
    if (rd.IsSP()) {
      if (emit_code) masm->mov(rd, temp);
      instruction_count++;
    }
    return instruction_count;
  }
}


void MacroAssembler::B(Label* label, BranchType type, Register reg, int bit) {
  VIXL_ASSERT((reg.Is(NoReg) || (type >= kBranchTypeFirstUsingReg)) &&
              ((bit == -1) || (type >= kBranchTypeFirstUsingBit)));
  if (kBranchTypeFirstCondition <= type && type <= kBranchTypeLastCondition) {
    B(static_cast<Condition>(type), label);
  } else {
    switch (type) {
      case always:
        B(label);
        break;
      case never:
        break;
      case reg_zero:
        Cbz(reg, label);
        break;
      case reg_not_zero:
        Cbnz(reg, label);
        break;
      case reg_bit_clear:
        Tbz(reg, bit, label);
        break;
      case reg_bit_set:
        Tbnz(reg, bit, label);
        break;
      default:
        VIXL_UNREACHABLE();
    }
  }
}


void MacroAssembler::B(Label* label) {
  // We don't need to check the size of the literal pool, because the size of
  // the literal pool is already bounded by the literal range, which is smaller
  // than the range of this branch.
  VIXL_ASSERT(Instruction::GetImmBranchForwardRange(UncondBranchType) >
              Instruction::kLoadLiteralRange);
  SingleEmissionCheckScope guard(this);
  b(label);
}


void MacroAssembler::B(Label* label, Condition cond) {
  // We don't need to check the size of the literal pool, because the size of
  // the literal pool is already bounded by the literal range, which is smaller
  // than the range of this branch.
  VIXL_ASSERT(Instruction::GetImmBranchForwardRange(CondBranchType) >
              Instruction::kLoadLiteralRange);
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT((cond != al) && (cond != nv));
  EmissionCheckScope guard(this, 2 * kInstructionSize);

  if (label->IsBound() && LabelIsOutOfRange(label, CondBranchType)) {
    Label done;
    b(&done, InvertCondition(cond));
    b(label);
    bind(&done);
  } else {
    if (!label->IsBound()) {
      veneer_pool_.RegisterUnresolvedBranch(GetCursorOffset(),
                                            label,
                                            CondBranchType);
    }
    b(label, cond);
  }
}


void MacroAssembler::Cbnz(const Register& rt, Label* label) {
  // We don't need to check the size of the literal pool, because the size of
  // the literal pool is already bounded by the literal range, which is smaller
  // than the range of this branch.
  VIXL_ASSERT(Instruction::GetImmBranchForwardRange(CompareBranchType) >
              Instruction::kLoadLiteralRange);
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!rt.IsZero());
  EmissionCheckScope guard(this, 2 * kInstructionSize);

  if (label->IsBound() && LabelIsOutOfRange(label, CondBranchType)) {
    Label done;
    cbz(rt, &done);
    b(label);
    bind(&done);
  } else {
    if (!label->IsBound()) {
      veneer_pool_.RegisterUnresolvedBranch(GetCursorOffset(),
                                            label,
                                            CompareBranchType);
    }
    cbnz(rt, label);
  }
}


void MacroAssembler::Cbz(const Register& rt, Label* label) {
  // We don't need to check the size of the literal pool, because the size of
  // the literal pool is already bounded by the literal range, which is smaller
  // than the range of this branch.
  VIXL_ASSERT(Instruction::GetImmBranchForwardRange(CompareBranchType) >
              Instruction::kLoadLiteralRange);
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!rt.IsZero());
  EmissionCheckScope guard(this, 2 * kInstructionSize);

  if (label->IsBound() && LabelIsOutOfRange(label, CondBranchType)) {
    Label done;
    cbnz(rt, &done);
    b(label);
    bind(&done);
  } else {
    if (!label->IsBound()) {
      veneer_pool_.RegisterUnresolvedBranch(GetCursorOffset(),
                                            label,
                                            CompareBranchType);
    }
    cbz(rt, label);
  }
}


void MacroAssembler::Tbnz(const Register& rt, unsigned bit_pos, Label* label) {
  // This is to avoid a situation where emitting a veneer for a TBZ/TBNZ branch
  // can become impossible because we emit the literal pool first.
  literal_pool_.CheckEmitForBranch(
      Instruction::GetImmBranchForwardRange(TestBranchType));
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!rt.IsZero());
  EmissionCheckScope guard(this, 2 * kInstructionSize);

  if (label->IsBound() && LabelIsOutOfRange(label, TestBranchType)) {
    Label done;
    tbz(rt, bit_pos, &done);
    b(label);
    bind(&done);
  } else {
    if (!label->IsBound()) {
      veneer_pool_.RegisterUnresolvedBranch(GetCursorOffset(),
                                            label,
                                            TestBranchType);
    }
    tbnz(rt, bit_pos, label);
  }
}


void MacroAssembler::Tbz(const Register& rt, unsigned bit_pos, Label* label) {
  // This is to avoid a situation where emitting a veneer for a TBZ/TBNZ branch
  // can become impossible because we emit the literal pool first.
  literal_pool_.CheckEmitForBranch(
      Instruction::GetImmBranchForwardRange(TestBranchType));
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!rt.IsZero());
  EmissionCheckScope guard(this, 2 * kInstructionSize);

  if (label->IsBound() && LabelIsOutOfRange(label, TestBranchType)) {
    Label done;
    tbnz(rt, bit_pos, &done);
    b(label);
    bind(&done);
  } else {
    if (!label->IsBound()) {
      veneer_pool_.RegisterUnresolvedBranch(GetCursorOffset(),
                                            label,
                                            TestBranchType);
    }
    tbz(rt, bit_pos, label);
  }
}

void MacroAssembler::Bind(Label* label, BranchTargetIdentifier id) {
  VIXL_ASSERT(allow_macro_instructions_);
  veneer_pool_.DeleteUnresolvedBranchInfoForLabel(label);
  if (id == EmitBTI_none) {
    bind(label);
  } else {
    // Emit this inside an ExactAssemblyScope to ensure there are no extra
    // instructions between the bind and the target identifier instruction.
    ExactAssemblyScope scope(this, kInstructionSize);
    bind(label);
    if (id == EmitPACIASP) {
      paciasp();
    } else if (id == EmitPACIBSP) {
      pacibsp();
    } else {
      bti(id);
    }
  }
}

// Bind a label to a specified offset from the start of the buffer.
void MacroAssembler::BindToOffset(Label* label, ptrdiff_t offset) {
  VIXL_ASSERT(allow_macro_instructions_);
  veneer_pool_.DeleteUnresolvedBranchInfoForLabel(label);
  Assembler::BindToOffset(label, offset);
}


void MacroAssembler::And(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, AND);
}


void MacroAssembler::Ands(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, ANDS);
}


void MacroAssembler::Tst(const Register& rn, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Ands(AppropriateZeroRegFor(rn), rn, operand);
}


void MacroAssembler::Bic(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, BIC);
}


void MacroAssembler::Bics(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, BICS);
}


void MacroAssembler::Orr(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, ORR);
}


void MacroAssembler::Orn(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, ORN);
}


void MacroAssembler::Eor(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, EOR);
}


void MacroAssembler::Eon(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  LogicalMacro(rd, rn, operand, EON);
}


void MacroAssembler::LogicalMacro(const Register& rd,
                                  const Register& rn,
                                  const Operand& operand,
                                  LogicalOp op) {
  // The worst case for size is logical immediate to sp:
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction to do the operation
  //  * 1 instruction to move to sp
  MacroEmissionCheckScope guard(this);
  UseScratchRegisterScope temps(this);
  // Use `rd` as a temp, if we can.
  temps.Include(rd);
  // We read `rn` after evaluating `operand`.
  temps.Exclude(rn);
  // It doesn't matter if `operand` is in `temps` (e.g. because it alises `rd`)
  // because we don't need it after it is evaluated.

  if (operand.IsImmediate()) {
    uint64_t immediate = operand.GetImmediate();
    unsigned reg_size = rd.GetSizeInBits();

    // If the operation is NOT, invert the operation and immediate.
    if ((op & NOT) == NOT) {
      op = static_cast<LogicalOp>(op & ~NOT);
      immediate = ~immediate;
    }

    // Ignore the top 32 bits of an immediate if we're moving to a W register.
    if (rd.Is32Bits()) {
      // Check that the top 32 bits are consistent.
      VIXL_ASSERT(((immediate >> kWRegSize) == 0) ||
                  ((immediate >> kWRegSize) == 0xffffffff));
      immediate &= kWRegMask;
    }

    VIXL_ASSERT(rd.Is64Bits() || IsUint32(immediate));

    // Special cases for all set or all clear immediates.
    if (immediate == 0) {
      switch (op) {
        case AND:
          Mov(rd, 0);
          return;
        case ORR:
          VIXL_FALLTHROUGH();
        case EOR:
          Mov(rd, rn);
          return;
        case ANDS:
          VIXL_FALLTHROUGH();
        case BICS:
          break;
        default:
          VIXL_UNREACHABLE();
      }
    } else if ((rd.Is64Bits() && (immediate == UINT64_C(0xffffffffffffffff))) ||
               (rd.Is32Bits() && (immediate == UINT64_C(0x00000000ffffffff)))) {
      switch (op) {
        case AND:
          Mov(rd, rn);
          return;
        case ORR:
          Mov(rd, immediate);
          return;
        case EOR:
          Mvn(rd, rn);
          return;
        case ANDS:
          VIXL_FALLTHROUGH();
        case BICS:
          break;
        default:
          VIXL_UNREACHABLE();
      }
    }

    unsigned n, imm_s, imm_r;
    if (IsImmLogical(immediate, reg_size, &n, &imm_s, &imm_r)) {
      // Immediate can be encoded in the instruction.
      LogicalImmediate(rd, rn, n, imm_s, imm_r, op);
    } else {
      // Immediate can't be encoded: synthesize using move immediate.
      Register temp = temps.AcquireSameSizeAs(rn);
      VIXL_ASSERT(!temp.Aliases(rn));

      // If the left-hand input is the stack pointer, we can't pre-shift the
      // immediate, as the encoding won't allow the subsequent post shift.
      PreShiftImmMode mode = rn.IsSP() ? kNoShift : kAnyShift;
      Operand imm_operand = MoveImmediateForShiftedOp(temp, immediate, mode);

      if (rd.Is(sp) || rd.Is(wsp)) {
        // If rd is the stack pointer we cannot use it as the destination
        // register so we use the temp register as an intermediate again.
        Logical(temp, rn, imm_operand, op);
        Mov(rd, temp);
      } else {
        Logical(rd, rn, imm_operand, op);
      }
    }
  } else if (operand.IsExtendedRegister()) {
    VIXL_ASSERT(operand.GetRegister().GetSizeInBits() <= rd.GetSizeInBits());
    // Add/sub extended supports shift <= 4. We want to support exactly the
    // same modes here.
    VIXL_ASSERT(operand.GetShiftAmount() <= 4);
    VIXL_ASSERT(
        operand.GetRegister().Is64Bits() ||
        ((operand.GetExtend() != UXTX) && (operand.GetExtend() != SXTX)));

    Register temp = temps.AcquireSameSizeAs(rn);
    VIXL_ASSERT(!temp.Aliases(rn));
    EmitExtendShift(temp,
                    operand.GetRegister(),
                    operand.GetExtend(),
                    operand.GetShiftAmount());
    Logical(rd, rn, Operand(temp), op);
  } else {
    // The operand can be encoded in the instruction.
    VIXL_ASSERT(operand.IsShiftedRegister());
    Logical(rd, rn, operand, op);
  }
}


void MacroAssembler::Mov(const Register& rd,
                         const Operand& operand,
                         DiscardMoveMode discard_mode) {
  VIXL_ASSERT(allow_macro_instructions_);
  // The worst case for size is mov immediate with up to 4 instructions.
  MacroEmissionCheckScope guard(this);

  if (operand.IsImmediate()) {
    // Call the macro assembler for generic immediates.
    Mov(rd, operand.GetImmediate());
  } else if (operand.IsShiftedRegister() && (operand.GetShiftAmount() != 0)) {
    // Emit a shift instruction if moving a shifted register. This operation
    // could also be achieved using an orr instruction (like orn used by Mvn),
    // but using a shift instruction makes the disassembly clearer.
    EmitShift(rd,
              operand.GetRegister(),
              operand.GetShift(),
              operand.GetShiftAmount());
  } else if (operand.IsExtendedRegister()) {
    // Emit an extend instruction if moving an extended register. This handles
    // extend with post-shift operations, too.
    EmitExtendShift(rd,
                    operand.GetRegister(),
                    operand.GetExtend(),
                    operand.GetShiftAmount());
  } else {
    Mov(rd, operand.GetRegister(), discard_mode);
  }
}


void MacroAssembler::Movi16bitHelper(const VRegister& vd, uint64_t imm) {
  VIXL_ASSERT(IsUint16(imm));
  int byte1 = (imm & 0xff);
  int byte2 = ((imm >> 8) & 0xff);
  if (byte1 == byte2) {
    movi(vd.Is64Bits() ? vd.V8B() : vd.V16B(), byte1);
  } else if (byte1 == 0) {
    movi(vd, byte2, LSL, 8);
  } else if (byte2 == 0) {
    movi(vd, byte1);
  } else if (byte1 == 0xff) {
    mvni(vd, ~byte2 & 0xff, LSL, 8);
  } else if (byte2 == 0xff) {
    mvni(vd, ~byte1 & 0xff);
  } else {
    UseScratchRegisterScope temps(this);
    Register temp = temps.AcquireW();
    movz(temp, imm);
    dup(vd, temp);
  }
}


void MacroAssembler::Movi32bitHelper(const VRegister& vd, uint64_t imm) {
  VIXL_ASSERT(IsUint32(imm));

  uint8_t bytes[sizeof(imm)];
  memcpy(bytes, &imm, sizeof(imm));

  // All bytes are either 0x00 or 0xff.
  {
    bool all0orff = true;
    for (int i = 0; i < 4; ++i) {
      if ((bytes[i] != 0) && (bytes[i] != 0xff)) {
        all0orff = false;
        break;
      }
    }

    if (all0orff == true) {
      movi(vd.Is64Bits() ? vd.V1D() : vd.V2D(), ((imm << 32) | imm));
      return;
    }
  }

  // Of the 4 bytes, only one byte is non-zero.
  for (int i = 0; i < 4; i++) {
    if ((imm & (0xff << (i * 8))) == imm) {
      movi(vd, bytes[i], LSL, i * 8);
      return;
    }
  }

  // Of the 4 bytes, only one byte is not 0xff.
  for (int i = 0; i < 4; i++) {
    uint32_t mask = ~(0xff << (i * 8));
    if ((imm & mask) == mask) {
      mvni(vd, ~bytes[i] & 0xff, LSL, i * 8);
      return;
    }
  }

  // Immediate is of the form 0x00MMFFFF.
  if ((imm & 0xff00ffff) == 0x0000ffff) {
    movi(vd, bytes[2], MSL, 16);
    return;
  }

  // Immediate is of the form 0x0000MMFF.
  if ((imm & 0xffff00ff) == 0x000000ff) {
    movi(vd, bytes[1], MSL, 8);
    return;
  }

  // Immediate is of the form 0xFFMM0000.
  if ((imm & 0xff00ffff) == 0xff000000) {
    mvni(vd, ~bytes[2] & 0xff, MSL, 16);
    return;
  }
  // Immediate is of the form 0xFFFFMM00.
  if ((imm & 0xffff00ff) == 0xffff0000) {
    mvni(vd, ~bytes[1] & 0xff, MSL, 8);
    return;
  }

  // Top and bottom 16-bits are equal.
  if (((imm >> 16) & 0xffff) == (imm & 0xffff)) {
    Movi16bitHelper(vd.Is64Bits() ? vd.V4H() : vd.V8H(), imm & 0xffff);
    return;
  }

  // Default case.
  {
    UseScratchRegisterScope temps(this);
    Register temp = temps.AcquireW();
    Mov(temp, imm);
    dup(vd, temp);
  }
}


void MacroAssembler::Movi64bitHelper(const VRegister& vd, uint64_t imm) {
  // All bytes are either 0x00 or 0xff.
  {
    bool all0orff = true;
    for (int i = 0; i < 8; ++i) {
      int byteval = (imm >> (i * 8)) & 0xff;
      if (byteval != 0 && byteval != 0xff) {
        all0orff = false;
        break;
      }
    }
    if (all0orff == true) {
      movi(vd, imm);
      return;
    }
  }

  // Top and bottom 32-bits are equal.
  if (((imm >> 32) & 0xffffffff) == (imm & 0xffffffff)) {
    Movi32bitHelper(vd.Is64Bits() ? vd.V2S() : vd.V4S(), imm & 0xffffffff);
    return;
  }

  // Default case.
  {
    UseScratchRegisterScope temps(this);
    Register temp = temps.AcquireX();
    Mov(temp, imm);
    if (vd.Is1D()) {
      fmov(vd.D(), temp);
    } else {
      dup(vd.V2D(), temp);
    }
  }
}


void MacroAssembler::Movi(const VRegister& vd,
                          uint64_t imm,
                          Shift shift,
                          int shift_amount) {
  VIXL_ASSERT(allow_macro_instructions_);
  MacroEmissionCheckScope guard(this);
  if (shift_amount != 0 || shift != LSL) {
    movi(vd, imm, shift, shift_amount);
  } else if (vd.Is8B() || vd.Is16B()) {
    // 8-bit immediate.
    VIXL_ASSERT(IsUint8(imm));
    movi(vd, imm);
  } else if (vd.Is4H() || vd.Is8H()) {
    // 16-bit immediate.
    Movi16bitHelper(vd, imm);
  } else if (vd.Is2S() || vd.Is4S()) {
    // 32-bit immediate.
    Movi32bitHelper(vd, imm);
  } else {
    // 64-bit immediate.
    Movi64bitHelper(vd, imm);
  }
}


void MacroAssembler::Movi(const VRegister& vd, uint64_t hi, uint64_t lo) {
  // TODO: Move 128-bit values in a more efficient way.
  VIXL_ASSERT(vd.Is128Bits());
  if (hi == lo) {
    Movi(vd.V2D(), lo);
    return;
  }

  Movi(vd.V1D(), lo);

  if (hi != 0) {
    UseScratchRegisterScope temps(this);
    // TODO: Figure out if using a temporary V register to materialise the
    // immediate is better.
    Register temp = temps.AcquireX();
    Mov(temp, hi);
    Ins(vd.V2D(), 1, temp);
  }
}


void MacroAssembler::Mvn(const Register& rd, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  // The worst case for size is mvn immediate with up to 4 instructions.
  MacroEmissionCheckScope guard(this);

  if (operand.IsImmediate()) {
    // Call the macro assembler for generic immediates.
    Mvn(rd, operand.GetImmediate());
  } else if (operand.IsExtendedRegister()) {
    // Emit two instructions for the extend case. This differs from Mov, as
    // the extend and invert can't be achieved in one instruction.
    EmitExtendShift(rd,
                    operand.GetRegister(),
                    operand.GetExtend(),
                    operand.GetShiftAmount());
    mvn(rd, rd);
  } else {
    // Otherwise, register and shifted register cases can be handled by the
    // assembler directly, using orn.
    mvn(rd, operand);
  }
}


void MacroAssembler::Mov(const Register& rd, uint64_t imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  MoveImmediateHelper(this, rd, imm);
}


void MacroAssembler::Ccmp(const Register& rn,
                          const Operand& operand,
                          StatusFlags nzcv,
                          Condition cond) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (operand.IsImmediate()) {
    int64_t imm = operand.GetImmediate();
    if ((imm < 0) && CanBeNegated(imm)) {
      ConditionalCompareMacro(rn, -imm, nzcv, cond, CCMN);
      return;
    }
  }
  ConditionalCompareMacro(rn, operand, nzcv, cond, CCMP);
}


void MacroAssembler::Ccmn(const Register& rn,
                          const Operand& operand,
                          StatusFlags nzcv,
                          Condition cond) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (operand.IsImmediate()) {
    int64_t imm = operand.GetImmediate();
    if ((imm < 0) && CanBeNegated(imm)) {
      ConditionalCompareMacro(rn, -imm, nzcv, cond, CCMP);
      return;
    }
  }
  ConditionalCompareMacro(rn, operand, nzcv, cond, CCMN);
}


void MacroAssembler::ConditionalCompareMacro(const Register& rn,
                                             const Operand& operand,
                                             StatusFlags nzcv,
                                             Condition cond,
                                             ConditionalCompareOp op) {
  VIXL_ASSERT((cond != al) && (cond != nv));
  // The worst case for size is ccmp immediate:
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction for ccmp
  MacroEmissionCheckScope guard(this);

  if ((operand.IsShiftedRegister() && (operand.GetShiftAmount() == 0)) ||
      (operand.IsImmediate() &&
       IsImmConditionalCompare(operand.GetImmediate()))) {
    // The immediate can be encoded in the instruction, or the operand is an
    // unshifted register: call the assembler.
    ConditionalCompare(rn, operand, nzcv, cond, op);
  } else {
    UseScratchRegisterScope temps(this);
    // The operand isn't directly supported by the instruction: perform the
    // operation on a temporary register.
    Register temp = temps.AcquireSameSizeAs(rn);
    Mov(temp, operand);
    ConditionalCompare(rn, temp, nzcv, cond, op);
  }
}


void MacroAssembler::CselHelper(MacroAssembler* masm,
                                const Register& rd,
                                Operand left,
                                Operand right,
                                Condition cond,
                                bool* should_synthesise_left,
                                bool* should_synthesise_right) {
  bool emit_code = (masm != NULL);

  VIXL_ASSERT(!emit_code || masm->allow_macro_instructions_);
  VIXL_ASSERT((cond != al) && (cond != nv));
  VIXL_ASSERT(!rd.IsZero() && !rd.IsSP());
  VIXL_ASSERT(left.IsImmediate() || !left.GetRegister().IsSP());
  VIXL_ASSERT(right.IsImmediate() || !right.GetRegister().IsSP());

  if (should_synthesise_left != NULL) *should_synthesise_left = false;
  if (should_synthesise_right != NULL) *should_synthesise_right = false;

  // The worst case for size occurs when the inputs are two non encodable
  // constants:
  //  * up to 4 instructions to materialise the left constant
  //  * up to 4 instructions to materialise the right constant
  //  * 1 instruction for csel
  EmissionCheckScope guard(masm, 9 * kInstructionSize);
  UseScratchRegisterScope temps;
  if (masm != NULL) {
    temps.Open(masm);
  }

  // Try to handle cases where both inputs are immediates.
  bool left_is_immediate = left.IsImmediate() || left.IsZero();
  bool right_is_immediate = right.IsImmediate() || right.IsZero();
  if (left_is_immediate && right_is_immediate &&
      CselSubHelperTwoImmediates(masm,
                                 rd,
                                 left.GetEquivalentImmediate(),
                                 right.GetEquivalentImmediate(),
                                 cond,
                                 should_synthesise_left,
                                 should_synthesise_right)) {
    return;
  }

  // Handle cases where one of the two inputs is -1, 0, or 1.
  bool left_is_small_immediate =
      left_is_immediate && ((-1 <= left.GetEquivalentImmediate()) &&
                            (left.GetEquivalentImmediate() <= 1));
  bool right_is_small_immediate =
      right_is_immediate && ((-1 <= right.GetEquivalentImmediate()) &&
                             (right.GetEquivalentImmediate() <= 1));
  if (right_is_small_immediate || left_is_small_immediate) {
    bool swapped_inputs = false;
    if (!right_is_small_immediate) {
      std::swap(left, right);
      cond = InvertCondition(cond);
      swapped_inputs = true;
    }
    CselSubHelperRightSmallImmediate(masm,
                                     &temps,
                                     rd,
                                     left,
                                     right,
                                     cond,
                                     swapped_inputs ? should_synthesise_right
                                                    : should_synthesise_left);
    return;
  }

  // Otherwise both inputs need to be available in registers. Synthesise them
  // if necessary and emit the `csel`.
  if (!left.IsPlainRegister()) {
    if (emit_code) {
      Register temp = temps.AcquireSameSizeAs(rd);
      masm->Mov(temp, left);
      left = temp;
    }
    if (should_synthesise_left != NULL) *should_synthesise_left = true;
  }
  if (!right.IsPlainRegister()) {
    if (emit_code) {
      Register temp = temps.AcquireSameSizeAs(rd);
      masm->Mov(temp, right);
      right = temp;
    }
    if (should_synthesise_right != NULL) *should_synthesise_right = true;
  }
  if (emit_code) {
    VIXL_ASSERT(left.IsPlainRegister() && right.IsPlainRegister());
    if (left.GetRegister().Is(right.GetRegister())) {
      masm->Mov(rd, left.GetRegister());
    } else {
      masm->csel(rd, left.GetRegister(), right.GetRegister(), cond);
    }
  }
}


bool MacroAssembler::CselSubHelperTwoImmediates(MacroAssembler* masm,
                                                const Register& rd,
                                                int64_t left,
                                                int64_t right,
                                                Condition cond,
                                                bool* should_synthesise_left,
                                                bool* should_synthesise_right) {
  bool emit_code = (masm != NULL);
  if (should_synthesise_left != NULL) *should_synthesise_left = false;
  if (should_synthesise_right != NULL) *should_synthesise_right = false;

  if (left == right) {
    if (emit_code) masm->Mov(rd, left);
    return true;
  } else if (left == -right) {
    if (should_synthesise_right != NULL) *should_synthesise_right = true;
    if (emit_code) {
      masm->Mov(rd, right);
      masm->Cneg(rd, rd, cond);
    }
    return true;
  }

  if (CselSubHelperTwoOrderedImmediates(masm, rd, left, right, cond)) {
    return true;
  } else {
    std::swap(left, right);
    if (CselSubHelperTwoOrderedImmediates(masm,
                                          rd,
                                          left,
                                          right,
                                          InvertCondition(cond))) {
      return true;
    }
  }

  // TODO: Handle more situations. For example handle `csel rd, #5, #6, cond`
  // with `cinc`.
  return false;
}


bool MacroAssembler::CselSubHelperTwoOrderedImmediates(MacroAssembler* masm,
                                                       const Register& rd,
                                                       int64_t left,
                                                       int64_t right,
                                                       Condition cond) {
  bool emit_code = (masm != NULL);

  if ((left == 1) && (right == 0)) {
    if (emit_code) masm->cset(rd, cond);
    return true;
  } else if ((left == -1) && (right == 0)) {
    if (emit_code) masm->csetm(rd, cond);
    return true;
  }
  return false;
}


void MacroAssembler::CselSubHelperRightSmallImmediate(
    MacroAssembler* masm,
    UseScratchRegisterScope* temps,
    const Register& rd,
    const Operand& left,
    const Operand& right,
    Condition cond,
    bool* should_synthesise_left) {
  bool emit_code = (masm != NULL);
  VIXL_ASSERT((right.IsImmediate() || right.IsZero()) &&
              (-1 <= right.GetEquivalentImmediate()) &&
              (right.GetEquivalentImmediate() <= 1));
  Register left_register;

  if (left.IsPlainRegister()) {
    left_register = left.GetRegister();
  } else {
    if (emit_code) {
      left_register = temps->AcquireSameSizeAs(rd);
      masm->Mov(left_register, left);
    }
    if (should_synthesise_left != NULL) *should_synthesise_left = true;
  }
  if (emit_code) {
    int64_t imm = right.GetEquivalentImmediate();
    Register zr = AppropriateZeroRegFor(rd);
    if (imm == 0) {
      masm->csel(rd, left_register, zr, cond);
    } else if (imm == 1) {
      masm->csinc(rd, left_register, zr, cond);
    } else {
      VIXL_ASSERT(imm == -1);
      masm->csinv(rd, left_register, zr, cond);
    }
  }
}


void MacroAssembler::Add(const Register& rd,
                         const Register& rn,
                         const Operand& operand,
                         FlagsUpdate S) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (operand.IsImmediate()) {
    int64_t imm = operand.GetImmediate();
    if ((imm < 0) && CanBeNegated(imm) && IsImmAddSub(-imm)) {
      AddSubMacro(rd, rn, -imm, S, SUB);
      return;
    }
  }
  AddSubMacro(rd, rn, operand, S, ADD);
}


void MacroAssembler::Adds(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  Add(rd, rn, operand, SetFlags);
}

#define MINMAX(V)        \
  V(Smax, smax, IsInt8)  \
  V(Smin, smin, IsInt8)  \
  V(Umax, umax, IsUint8) \
  V(Umin, umin, IsUint8)

#define VIXL_DEFINE_MASM_FUNC(MASM, ASM, RANGE)      \
  void MacroAssembler::MASM(const Register& rd,      \
                            const Register& rn,      \
                            const Operand& op) {     \
    VIXL_ASSERT(allow_macro_instructions_);          \
    if (op.IsImmediate()) {                          \
      int64_t imm = op.GetImmediate();               \
      if (!RANGE(imm)) {                             \
        UseScratchRegisterScope temps(this);         \
        Register temp = temps.AcquireSameSizeAs(rd); \
        Mov(temp, imm);                              \
        MASM(rd, rn, temp);                          \
        return;                                      \
      }                                              \
    }                                                \
    SingleEmissionCheckScope guard(this);            \
    ASM(rd, rn, op);                                 \
  }
MINMAX(VIXL_DEFINE_MASM_FUNC)
#undef VIXL_DEFINE_MASM_FUNC

void MacroAssembler::St2g(const Register& rt, const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  st2g(rt, addr);
}

void MacroAssembler::Stg(const Register& rt, const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  stg(rt, addr);
}

void MacroAssembler::Stgp(const Register& rt1,
                          const Register& rt2,
                          const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  stgp(rt1, rt2, addr);
}

void MacroAssembler::Stz2g(const Register& rt, const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  stz2g(rt, addr);
}

void MacroAssembler::Stzg(const Register& rt, const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  stzg(rt, addr);
}

void MacroAssembler::Ldg(const Register& rt, const MemOperand& addr) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  ldg(rt, addr);
}

void MacroAssembler::Sub(const Register& rd,
                         const Register& rn,
                         const Operand& operand,
                         FlagsUpdate S) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (operand.IsImmediate()) {
    int64_t imm = operand.GetImmediate();
    if ((imm < 0) && CanBeNegated(imm) && IsImmAddSub(-imm)) {
      AddSubMacro(rd, rn, -imm, S, ADD);
      return;
    }
  }
  AddSubMacro(rd, rn, operand, S, SUB);
}


void MacroAssembler::Subs(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  Sub(rd, rn, operand, SetFlags);
}


void MacroAssembler::Cmn(const Register& rn, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Adds(AppropriateZeroRegFor(rn), rn, operand);
}


void MacroAssembler::Cmp(const Register& rn, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Subs(AppropriateZeroRegFor(rn), rn, operand);
}


void MacroAssembler::Fcmp(const VRegister& fn, double value, FPTrapFlags trap) {
  VIXL_ASSERT(allow_macro_instructions_);
  // The worst case for size is:
  //  * 1 to materialise the constant, using literal pool if necessary
  //  * 1 instruction for fcmp{e}
  MacroEmissionCheckScope guard(this);
  if (value != 0.0) {
    UseScratchRegisterScope temps(this);
    VRegister tmp = temps.AcquireSameSizeAs(fn);
    Fmov(tmp, value);
    FPCompareMacro(fn, tmp, trap);
  } else {
    FPCompareMacro(fn, value, trap);
  }
}


void MacroAssembler::Fcmpe(const VRegister& fn, double value) {
  Fcmp(fn, value, EnableTrap);
}


void MacroAssembler::Fmov(VRegister vd, double imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  // Floating point immediates are loaded through the literal pool.
  MacroEmissionCheckScope guard(this);
  uint64_t rawbits = DoubleToRawbits(imm);

  if (rawbits == 0) {
    fmov(vd.D(), xzr);
    return;
  }

  if (vd.Is1H() || vd.Is4H() || vd.Is8H()) {
    Fmov(vd, Float16(imm));
    return;
  }

  if (vd.Is1S() || vd.Is2S() || vd.Is4S()) {
    Fmov(vd, static_cast<float>(imm));
    return;
  }

  VIXL_ASSERT(vd.Is1D() || vd.Is2D());
  if (IsImmFP64(rawbits)) {
    fmov(vd, imm);
  } else if (vd.IsScalar()) {
    ldr(vd,
        new Literal<double>(imm,
                            &literal_pool_,
                            RawLiteral::kDeletedOnPlacementByPool));
  } else {
    // TODO: consider NEON support for load literal.
    Movi(vd, rawbits);
  }
}


void MacroAssembler::Fmov(VRegister vd, float imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  // Floating point immediates are loaded through the literal pool.
  MacroEmissionCheckScope guard(this);
  uint32_t rawbits = FloatToRawbits(imm);

  if (rawbits == 0) {
    fmov(vd.S(), wzr);
    return;
  }

  if (vd.Is1H() || vd.Is4H() || vd.Is8H()) {
    Fmov(vd, Float16(imm));
    return;
  }

  if (vd.Is1D() || vd.Is2D()) {
    Fmov(vd, static_cast<double>(imm));
    return;
  }

  VIXL_ASSERT(vd.Is1S() || vd.Is2S() || vd.Is4S());
  if (IsImmFP32(rawbits)) {
    fmov(vd, imm);
  } else if (vd.IsScalar()) {
    ldr(vd,
        new Literal<float>(imm,
                           &literal_pool_,
                           RawLiteral::kDeletedOnPlacementByPool));
  } else {
    // TODO: consider NEON support for load literal.
    Movi(vd, rawbits);
  }
}


void MacroAssembler::Fmov(VRegister vd, Float16 imm) {
  VIXL_ASSERT(allow_macro_instructions_);
  MacroEmissionCheckScope guard(this);

  if (vd.Is1S() || vd.Is2S() || vd.Is4S()) {
    Fmov(vd, FPToFloat(imm, kIgnoreDefaultNaN));
    return;
  }

  if (vd.Is1D() || vd.Is2D()) {
    Fmov(vd, FPToDouble(imm, kIgnoreDefaultNaN));
    return;
  }

  VIXL_ASSERT(vd.Is1H() || vd.Is4H() || vd.Is8H());
  uint16_t rawbits = Float16ToRawbits(imm);
  if (IsImmFP16(imm)) {
    fmov(vd, imm);
  } else {
    if (vd.IsScalar()) {
      if (rawbits == 0x0) {
        fmov(vd, wzr);
      } else {
        // We can use movz instead of the literal pool.
        UseScratchRegisterScope temps(this);
        Register temp = temps.AcquireW();
        Mov(temp, rawbits);
        Fmov(vd, temp);
      }
    } else {
      // TODO: consider NEON support for load literal.
      Movi(vd, static_cast<uint64_t>(rawbits));
    }
  }
}


void MacroAssembler::Neg(const Register& rd, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (operand.IsImmediate() && CanBeNegated(operand.GetImmediate())) {
    Mov(rd, -operand.GetImmediate());
  } else {
    Sub(rd, AppropriateZeroRegFor(rd), operand);
  }
}


void MacroAssembler::Negs(const Register& rd, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Subs(rd, AppropriateZeroRegFor(rd), operand);
}


bool MacroAssembler::TryOneInstrMoveImmediate(const Register& dst,
                                              uint64_t imm) {
  return OneInstrMoveImmediateHelper(this, dst, imm);
}


Operand MacroAssembler::MoveImmediateForShiftedOp(const Register& dst,
                                                  uint64_t imm,
                                                  PreShiftImmMode mode) {
  int reg_size = dst.GetSizeInBits();

  // Encode the immediate in a single move instruction, if possible.
  if (TryOneInstrMoveImmediate(dst, imm)) {
    // The move was successful; nothing to do here.
  } else {
    // Pre-shift the immediate to the least-significant bits of the register.
    int shift_low = CountTrailingZeros(imm, reg_size);
    if (mode == kLimitShiftForSP) {
      // When applied to the stack pointer, the subsequent arithmetic operation
      // can use the extend form to shift left by a maximum of four bits. Right
      // shifts are not allowed, so we filter them out later before the new
      // immediate is tested.
      shift_low = std::min(shift_low, 4);
    }
    // TryOneInstrMoveImmediate handles `imm` with a value of zero, so shift_low
    // must lie in the range [0, 63], and the shifts below are well-defined.
    VIXL_ASSERT((shift_low >= 0) && (shift_low < 64));
    // imm_low = imm >> shift_low (with sign extension)
    uint64_t imm_low = ExtractSignedBitfield64(63, shift_low, imm);

    // Pre-shift the immediate to the most-significant bits of the register,
    // inserting set bits in the least-significant bits.
    int shift_high = CountLeadingZeros(imm, reg_size);
    VIXL_ASSERT((shift_high >= 0) && (shift_high < 64));
    uint64_t imm_high = (imm << shift_high) | GetUintMask(shift_high);

    if ((mode != kNoShift) && TryOneInstrMoveImmediate(dst, imm_low)) {
      // The new immediate has been moved into the destination's low bits:
      // return a new leftward-shifting operand.
      return Operand(dst, LSL, shift_low);
    } else if ((mode == kAnyShift) && TryOneInstrMoveImmediate(dst, imm_high)) {
      // The new immediate has been moved into the destination's high bits:
      // return a new rightward-shifting operand.
      return Operand(dst, LSR, shift_high);
    } else {
      Mov(dst, imm);
    }
  }
  return Operand(dst);
}


void MacroAssembler::Move(const GenericOperand& dst,
                          const GenericOperand& src) {
  if (dst.Equals(src)) {
    return;
  }

  VIXL_ASSERT(dst.IsValid() && src.IsValid());

  // The sizes of the operands must match exactly.
  VIXL_ASSERT(dst.GetSizeInBits() == src.GetSizeInBits());
  VIXL_ASSERT(dst.GetSizeInBits() <= kXRegSize);
  int operand_size = static_cast<int>(dst.GetSizeInBits());

  if (dst.IsCPURegister() && src.IsCPURegister()) {
    CPURegister dst_reg = dst.GetCPURegister();
    CPURegister src_reg = src.GetCPURegister();
    if (dst_reg.IsRegister() && src_reg.IsRegister()) {
      Mov(Register(dst_reg), Register(src_reg));
    } else if (dst_reg.IsVRegister() && src_reg.IsVRegister()) {
      Fmov(VRegister(dst_reg), VRegister(src_reg));
    } else {
      if (dst_reg.IsRegister()) {
        Fmov(Register(dst_reg), VRegister(src_reg));
      } else {
        Fmov(VRegister(dst_reg), Register(src_reg));
      }
    }
    return;
  }

  if (dst.IsMemOperand() && src.IsMemOperand()) {
    UseScratchRegisterScope temps(this);
    CPURegister temp = temps.AcquireCPURegisterOfSize(operand_size);
    Ldr(temp, src.GetMemOperand());
    Str(temp, dst.GetMemOperand());
    return;
  }

  if (dst.IsCPURegister()) {
    Ldr(dst.GetCPURegister(), src.GetMemOperand());
  } else {
    Str(src.GetCPURegister(), dst.GetMemOperand());
  }
}


void MacroAssembler::ComputeAddress(const Register& dst,
                                    const MemOperand& mem_op) {
  // We cannot handle pre-indexing or post-indexing.
  VIXL_ASSERT(mem_op.GetAddrMode() == Offset);
  Register base = mem_op.GetBaseRegister();
  if (mem_op.IsImmediateOffset()) {
    Add(dst, base, mem_op.GetOffset());
  } else {
    VIXL_ASSERT(mem_op.IsRegisterOffset());
    Register reg_offset = mem_op.GetRegisterOffset();
    Shift shift = mem_op.GetShift();
    Extend extend = mem_op.GetExtend();
    if (shift == NO_SHIFT) {
      VIXL_ASSERT(extend != NO_EXTEND);
      Add(dst, base, Operand(reg_offset, extend, mem_op.GetShiftAmount()));
    } else {
      VIXL_ASSERT(extend == NO_EXTEND);
      Add(dst, base, Operand(reg_offset, shift, mem_op.GetShiftAmount()));
    }
  }
}


void MacroAssembler::AddSubMacro(const Register& rd,
                                 const Register& rn,
                                 const Operand& operand,
                                 FlagsUpdate S,
                                 AddSubOp op) {
  // Worst case is add/sub immediate:
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction for add/sub
  MacroEmissionCheckScope guard(this);

  if (operand.IsZero() && rd.Is(rn) && rd.Is64Bits() && rn.Is64Bits() &&
      (S == LeaveFlags)) {
    // The instruction would be a nop. Avoid generating useless code.
    return;
  }

  if ((operand.IsImmediate() && !IsImmAddSub(operand.GetImmediate())) ||
      (rn.IsZero() && !operand.IsShiftedRegister()) ||
      (operand.IsShiftedRegister() && (operand.GetShift() == ROR))) {
    UseScratchRegisterScope temps(this);
    // Use `rd` as a temp, if we can.
    temps.Include(rd);
    // We read `rn` after evaluating `operand`.
    temps.Exclude(rn);
    // It doesn't matter if `operand` is in `temps` (e.g. because it alises
    // `rd`) because we don't need it after it is evaluated.
    Register temp = temps.AcquireSameSizeAs(rn);
    if (operand.IsImmediate()) {
      PreShiftImmMode mode = kAnyShift;

      // If the destination or source register is the stack pointer, we can
      // only pre-shift the immediate right by values supported in the add/sub
      // extend encoding.
      if (rd.IsSP()) {
        // If the destination is SP and flags will be set, we can't pre-shift
        // the immediate at all.
        mode = (S == SetFlags) ? kNoShift : kLimitShiftForSP;
      } else if (rn.IsSP()) {
        mode = kLimitShiftForSP;
      }

      Operand imm_operand =
          MoveImmediateForShiftedOp(temp, operand.GetImmediate(), mode);
      AddSub(rd, rn, imm_operand, S, op);
    } else {
      Mov(temp, operand);
      AddSub(rd, rn, temp, S, op);
    }
  } else {
    AddSub(rd, rn, operand, S, op);
  }
}


void MacroAssembler::Adc(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  AddSubWithCarryMacro(rd, rn, operand, LeaveFlags, ADC);
}


void MacroAssembler::Adcs(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  AddSubWithCarryMacro(rd, rn, operand, SetFlags, ADC);
}


void MacroAssembler::Sbc(const Register& rd,
                         const Register& rn,
                         const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  AddSubWithCarryMacro(rd, rn, operand, LeaveFlags, SBC);
}


void MacroAssembler::Sbcs(const Register& rd,
                          const Register& rn,
                          const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  AddSubWithCarryMacro(rd, rn, operand, SetFlags, SBC);
}


void MacroAssembler::Ngc(const Register& rd, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Register zr = AppropriateZeroRegFor(rd);
  Sbc(rd, zr, operand);
}


void MacroAssembler::Ngcs(const Register& rd, const Operand& operand) {
  VIXL_ASSERT(allow_macro_instructions_);
  Register zr = AppropriateZeroRegFor(rd);
  Sbcs(rd, zr, operand);
}


void MacroAssembler::AddSubWithCarryMacro(const Register& rd,
                                          const Register& rn,
                                          const Operand& operand,
                                          FlagsUpdate S,
                                          AddSubWithCarryOp op) {
  VIXL_ASSERT(rd.GetSizeInBits() == rn.GetSizeInBits());
  // Worst case is addc/subc immediate:
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction for add/sub
  MacroEmissionCheckScope guard(this);
  UseScratchRegisterScope temps(this);
  // Use `rd` as a temp, if we can.
  temps.Include(rd);
  // We read `rn` after evaluating `operand`.
  temps.Exclude(rn);
  // It doesn't matter if `operand` is in `temps` (e.g. because it alises `rd`)
  // because we don't need it after it is evaluated.

  if (operand.IsImmediate() ||
      (operand.IsShiftedRegister() && (operand.GetShift() == ROR))) {
    // Add/sub with carry (immediate or ROR shifted register.)
    Register temp = temps.AcquireSameSizeAs(rn);
    Mov(temp, operand);
    AddSubWithCarry(rd, rn, Operand(temp), S, op);
  } else if (operand.IsShiftedRegister() && (operand.GetShiftAmount() != 0)) {
    // Add/sub with carry (shifted register).
    VIXL_ASSERT(operand.GetRegister().GetSizeInBits() == rd.GetSizeInBits());
    VIXL_ASSERT(operand.GetShift() != ROR);
    VIXL_ASSERT(
        IsUintN(rd.GetSizeInBits() == kXRegSize ? kXRegSizeLog2 : kWRegSizeLog2,
                operand.GetShiftAmount()));
    Register temp = temps.AcquireSameSizeAs(rn);
    EmitShift(temp,
              operand.GetRegister(),
              operand.GetShift(),
              operand.GetShiftAmount());
    AddSubWithCarry(rd, rn, Operand(temp), S, op);
  } else if (operand.IsExtendedRegister()) {
    // Add/sub with carry (extended register).
    VIXL_ASSERT(operand.GetRegister().GetSizeInBits() <= rd.GetSizeInBits());
    // Add/sub extended supports a shift <= 4. We want to support exactly the
    // same modes.
    VIXL_ASSERT(operand.GetShiftAmount() <= 4);
    VIXL_ASSERT(
        operand.GetRegister().Is64Bits() ||
        ((operand.GetExtend() != UXTX) && (operand.GetExtend() != SXTX)));
    Register temp = temps.AcquireSameSizeAs(rn);
    EmitExtendShift(temp,
                    operand.GetRegister(),
                    operand.GetExtend(),
                    operand.GetShiftAmount());
    AddSubWithCarry(rd, rn, Operand(temp), S, op);
  } else {
    // The addressing mode is directly supported by the instruction.
    AddSubWithCarry(rd, rn, operand, S, op);
  }
}


void MacroAssembler::Rmif(const Register& xn,
                          unsigned shift,
                          StatusFlags flags) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  rmif(xn, shift, flags);
}


void MacroAssembler::Setf8(const Register& wn) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  setf8(wn);
}


void MacroAssembler::Setf16(const Register& wn) {
  VIXL_ASSERT(allow_macro_instructions_);
  SingleEmissionCheckScope guard(this);
  setf16(wn);
}

void MacroAssembler::Chkfeat(const Register& xdn) {
  VIXL_ASSERT(allow_macro_instructions_);
  MacroEmissionCheckScope guard(this);
  if (xdn.Is(x16)) {
    chkfeat(xdn);
  } else {
    UseScratchRegisterScope temps(this);
    if (temps.TryAcquire(x16)) {
      Mov(x16, xdn);
      chkfeat(x16);
      Mov(xdn, x16);
    } else {
      VIXL_ABORT();
    }
  }
}

#define DEFINE_FUNCTION(FN, REGTYPE, REG, OP)                          \
  void MacroAssembler::FN(const REGTYPE REG, const MemOperand& addr) { \
    VIXL_ASSERT(allow_macro_instructions_);                            \
    LoadStoreMacro(REG, addr, OP);                                     \
  }
LS_MACRO_LIST(DEFINE_FUNCTION)
#undef DEFINE_FUNCTION


void MacroAssembler::LoadStoreMacro(const CPURegister& rt,
                                    const MemOperand& addr,
                                    LoadStoreOp op) {
  VIXL_ASSERT(addr.IsImmediateOffset() || addr.IsImmediatePostIndex() ||
              addr.IsImmediatePreIndex() || addr.IsRegisterOffset());

  // Worst case is ldr/str pre/post index:
  //  * 1 instruction for ldr/str
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction to update the base
  MacroEmissionCheckScope guard(this);

  int64_t offset = addr.GetOffset();
  unsigned access_size = CalcLSDataSize(op);

  // Check if an immediate offset fits in the immediate field of the
  // appropriate instruction. If not, emit two instructions to perform
  // the operation.
  if (addr.IsImmediateOffset() && !IsImmLSScaled(offset, access_size) &&
      !IsImmLSUnscaled(offset)) {
    // Immediate offset that can't be encoded using unsigned or unscaled
    // addressing modes.
    UseScratchRegisterScope temps(this);
    Register temp = temps.AcquireSameSizeAs(addr.GetBaseRegister());
    Mov(temp, addr.GetOffset());
    LoadStore(rt, MemOperand(addr.GetBaseRegister(), temp), op);
  } else if (addr.IsImmediatePostIndex() && !IsImmLSUnscaled(offset)) {
    // Post-index beyond unscaled addressing range.
    LoadStore(rt, MemOperand(addr.GetBaseRegister()), op);
    Add(addr.GetBaseRegister(), addr.GetBaseRegister(), Operand(offset));
  } else if (addr.IsImmediatePreIndex() && !IsImmLSUnscaled(offset)) {
    // Pre-index beyond unscaled addressing range.
    Add(addr.GetBaseRegister(), addr.GetBaseRegister(), Operand(offset));
    LoadStore(rt, MemOperand(addr.GetBaseRegister()), op);
  } else {
    // Encodable in one load/store instruction.
    LoadStore(rt, addr, op);
  }
}


#define DEFINE_FUNCTION(FN, REGTYPE, REG, REG2, OP) \
  void MacroAssembler::FN(const REGTYPE REG,        \
                          const REGTYPE REG2,       \
                          const MemOperand& addr) { \
    VIXL_ASSERT(allow_macro_instructions_);         \
    LoadStorePairMacro(REG, REG2, addr, OP);        \
  }
LSPAIR_MACRO_LIST(DEFINE_FUNCTION)
#undef DEFINE_FUNCTION

void MacroAssembler::LoadStorePairMacro(const CPURegister& rt,
                                        const CPURegister& rt2,
                                        const MemOperand& addr,
                                        LoadStorePairOp op) {
  // TODO(all): Should we support register offset for load-store-pair?
  VIXL_ASSERT(!addr.IsRegisterOffset());
  // Worst case is ldp/stp immediate:
  //  * 1 instruction for ldp/stp
  //  * up to 4 instructions to materialise the constant
  //  * 1 instruction to update the base
  MacroEmissionCheckScope guard(this);

  int64_t offset = addr.GetOffset();
  unsigned access_size = CalcLSPairDataSize(op);

  // Check if the offset fits in the immediate field of the appropriate
  // instruction. If not, emit two instructions to perform the operation.
  if (IsImmLSPair(offset, access_size)) {
    // Encodable in one load/store pair instruction.
    LoadStorePair(rt, rt2, addr, op);
  } else {
    Register base = addr.GetBaseRegister();
    if (addr.IsImmediateOffset()) {
      UseScratchRegisterScope temps(this);
      Register temp = temps.AcquireSameSizeAs(base);
      Add(temp, base, offset);
      LoadStorePair(rt, rt2, MemOperand(temp), op);
    } else if (addr.IsImmediatePostIndex()) {
      LoadStorePair(rt, rt2, MemOperand(base), op);
      Add(base, base, offset);
    } else {
      VIXL_ASSERT(addr.IsImmediatePreIndex());
      Add(base, base, offset);
      LoadStorePair(rt, rt2, MemOperand(base), op);
    }
  }
}


void MacroAssembler::Prfm(PrefetchOperation op, const MemOperand& addr) {
  MacroEmissionCheckScope guard(this);

  // There are no pre- or post-index modes for prfm.
  VIXL_ASSERT(addr.IsImmediateOffset() || addr.IsRegisterOffset());

  // The access size is implicitly 8 bytes for all prefetch operations.
  unsigned size = kXRegSizeInBytesLog2;

  // Check if an immediate offset fits in the immediate field of the
  // appropriate instruction. If not, emit two instructions to perform
  // the operation.
  if (addr.IsImmediateOffset() && !IsImmLSScaled(addr.GetOffset(), size) &&
      !IsImmLSUnscaled(addr.GetOffset())) {
    // Immediate offset that can't be encoded using unsigned or unscaled
    // addressing modes.
    UseScratchRegisterScope temps(this);
    Register temp = temps.AcquireSameSizeAs(addr.GetBaseRegister());
    Mov(temp, addr.GetOffset());
    Prefetch(op, MemOperand(addr.GetBaseRegister(), temp));
  } else {
    // Simple register-offsets are encodable in one instruction.
    Prefetch(op, addr);
  }
}


void MacroAssembler::Push(const CPURegister& src0,
                          const CPURegister& src1,
                          const CPURegister& src2,
                          const CPURegister& src3) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(AreSameSizeAndType(src0, src1, src2, src3));
  VIXL_ASSERT(src0.IsValid());

  int count = 1 + src1.IsValid() + src2.IsValid() + src3.IsValid();
  int size = src0.GetSizeInBytes();

  PrepareForPush(count, size);
  PushHelper(count, size, src0, src1, src2, src3);
}


void MacroAssembler::Pop(const CPURegister& dst0,
                         const CPURegister& dst1,
                         const CPURegister& dst2,
                         const CPURegister& dst3) {
  // It is not valid to pop into the same register more than once in one
  // instruction, not even into the zero register.
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(!AreAliased(dst0, dst1, dst2, dst3));
  VIXL_ASSERT(AreSameSizeAndType(dst0, dst1, dst2, dst3));
  VIXL_ASSERT(dst0.IsValid());

  int count = 1 + dst1.IsValid() + dst2.IsValid() + dst3.IsValid();
  int size = dst0.GetSizeInBytes();

  PrepareForPop(count, size);
  PopHelper(count, size, dst0, dst1, dst2, dst3);
}


void MacroAssembler::PushCPURegList(CPURegList registers) {
  VIXL_ASSERT(!registers.Overlaps(*GetScratchRegisterList()));
  VIXL_ASSERT(!registers.Overlaps(*GetScratchVRegisterList()));
  VIXL_ASSERT(allow_macro_instructions_);

  int reg_size = registers.GetRegisterSizeInBytes();
  PrepareForPush(registers.GetCount(), reg_size);

  // Bump the stack pointer and store two registers at the bottom.
  int size = registers.GetTotalSizeInBytes();
  const CPURegister& bottom_0 = registers.PopLowestIndex();
  const CPURegister& bottom_1 = registers.PopLowestIndex();
  if (bottom_0.IsValid() && bottom_1.IsValid()) {
    Stp(bottom_0, bottom_1, MemOperand(StackPointer(), -size, PreIndex));
  } else if (bottom_0.IsValid()) {
    Str(bottom_0, MemOperand(StackPointer(), -size, PreIndex));
  }

  int offset = 2 * reg_size;
  while (!registers.IsEmpty()) {
    const CPURegister& src0 = registers.PopLowestIndex();
    const CPURegister& src1 = registers.PopLowestIndex();
    if (src1.IsValid()) {
      Stp(src0, src1, MemOperand(StackPointer(), offset));
    } else {
      Str(src0, MemOperand(StackPointer(), offset));
    }
    offset += 2 * reg_size;
  }
}


void MacroAssembler::PopCPURegList(CPURegList registers) {
  VIXL_ASSERT(!registers.Overlaps(*GetScratchRegisterList()));
  VIXL_ASSERT(!registers.Overlaps(*GetScratchVRegisterList()));
  VIXL_ASSERT(allow_macro_instructions_);

  int reg_size = registers.GetRegisterSizeInBytes();
  PrepareForPop(registers.GetCount(), reg_size);


  int size = registers.GetTotalSizeInBytes();
  const CPURegister& bottom_0 = registers.PopLowestIndex();
  const CPURegister& bottom_1 = registers.PopLowestIndex();

  int offset = 2 * reg_size;
  while (!registers.IsEmpty()) {
    const CPURegister& dst0 = registers.PopLowestIndex();
    const CPURegister& dst1 = registers.PopLowestIndex();
    if (dst1.IsValid()) {
      Ldp(dst0, dst1, MemOperand(StackPointer(), offset));
    } else {
      Ldr(dst0, MemOperand(StackPointer(), offset));
    }
    offset += 2 * reg_size;
  }

  // Load the two registers at the bottom and drop the stack pointer.
  if (bottom_0.IsValid() && bottom_1.IsValid()) {
    Ldp(bottom_0, bottom_1, MemOperand(StackPointer(), size, PostIndex));
  } else if (bottom_0.IsValid()) {
    Ldr(bottom_0, MemOperand(StackPointer(), size, PostIndex));
  }
}


void MacroAssembler::PushMultipleTimes(int count, Register src) {
  VIXL_ASSERT(allow_macro_instructions_);
  int size = src.GetSizeInBytes();

  PrepareForPush(count, size);
  // Push up to four registers at a time if possible because if the current
  // stack pointer is sp and the register size is 32, registers must be pushed
  // in blocks of four in order to maintain the 16-byte alignment for sp.
  while (count >= 4) {
    PushHelper(4, size, src, src, src, src);
    count -= 4;
  }
  if (count >= 2) {
    PushHelper(2, size, src, src, NoReg, NoReg);
    count -= 2;
  }
  if (count == 1) {
    PushHelper(1, size, src, NoReg, NoReg, NoReg);
    count -= 1;
  }
  VIXL_ASSERT(count == 0);
}


void MacroAssembler::PushHelper(int count,
                                int size,
                                const CPURegister& src0,
                                const CPURegister& src1,
                                const CPURegister& src2,
                                const CPURegister& src3) {
  // Ensure that we don't unintentionally modify scratch or debug registers.
  // Worst case for size is 2 stp.
  ExactAssemblyScope scope(this,
                           2 * kInstructionSize,
                           ExactAssemblyScope::kMaximumSize);

  VIXL_ASSERT(AreSameSizeAndType(src0, src1, src2, src3));
  VIXL_ASSERT(size == src0.GetSizeInBytes());

  // When pushing multiple registers, the store order is chosen such that
  // Push(a, b) is equivalent to Push(a) followed by Push(b).
  switch (count) {
    case 1:
      VIXL_ASSERT(src1.IsNone() && src2.IsNone() && src3.IsNone());
      str(src0, MemOperand(StackPointer(), -1 * size, PreIndex));
      break;
    case 2:
      VIXL_ASSERT(src2.IsNone() && src3.IsNone());
      stp(src1, src0, MemOperand(StackPointer(), -2 * size, PreIndex));
      break;
    case 3:
      VIXL_ASSERT(src3.IsNone());
      stp(src2, src1, MemOperand(StackPointer(), -3 * size, PreIndex));
      str(src0, MemOperand(StackPointer(), 2 * size));
      break;
    case 4:
      // Skip over 4 * size, then fill in the gap. This allows four W registers
      // to be pushed using sp, whilst maintaining 16-byte alignment for sp at
      // all times.
      stp(src3, src2, MemOperand(StackPointer(), -4 * size, PreIndex));
      stp(src1, src0, MemOperand(StackPointer(), 2 * size));
      break;
    default:
      VIXL_UNREACHABLE();
  }
}


void MacroAssembler::PopHelper(int count,
                               int size,
                               const CPURegister& dst0,
                               const CPURegister& dst1,
                               const CPURegister& dst2,
                               const CPURegister& dst3) {
  // Ensure that we don't unintentionally modify scratch or debug registers.
  // Worst case for size is 2 ldp.
  ExactAssemblyScope scope(this,
                           2 * kInstructionSize,
                           ExactAssemblyScope::kMaximumSize);

  VIXL_ASSERT(AreSameSizeAndType(dst0, dst1, dst2, dst3));
  VIXL_ASSERT(size == dst0.GetSizeInBytes());

  // When popping multiple registers, the load order is chosen such that
  // Pop(a, b) is equivalent to Pop(a) followed by Pop(b).
  switch (count) {
    case 1:
      VIXL_ASSERT(dst1.IsNone() && dst2.IsNone() && dst3.IsNone());
      ldr(dst0, MemOperand(StackPointer(), 1 * size, PostIndex));
      break;
    case 2:
      VIXL_ASSERT(dst2.IsNone() && dst3.IsNone());
      ldp(dst0, dst1, MemOperand(StackPointer(), 2 * size, PostIndex));
      break;
    case 3:
      VIXL_ASSERT(dst3.IsNone());
      ldr(dst2, MemOperand(StackPointer(), 2 * size));
      ldp(dst0, dst1, MemOperand(StackPointer(), 3 * size, PostIndex));
      break;
    case 4:
      // Load the higher addresses first, then load the lower addresses and skip
      // the whole block in the second instruction. This allows four W registers
      // to be popped using sp, whilst maintaining 16-byte alignment for sp at
      // all times.
      ldp(dst2, dst3, MemOperand(StackPointer(), 2 * size));
      ldp(dst0, dst1, MemOperand(StackPointer(), 4 * size, PostIndex));
      break;
    default:
      VIXL_UNREACHABLE();
  }
}


void MacroAssembler::PrepareForPush(int count, int size) {
  if (sp.Is(StackPointer())) {
    // If the current stack pointer is sp, then it must be aligned to 16 bytes
    // on entry and the total size of the specified registers must also be a
    // multiple of 16 bytes.
    VIXL_ASSERT((count * size) % 16 == 0);
  } else {
    // Even if the current stack pointer is not the system stack pointer (sp),
    // the system stack pointer will still be modified in order to comply with
    // ABI rules about accessing memory below the system stack pointer.
    BumpSystemStackPointer(count * size);
  }
}


void MacroAssembler::PrepareForPop(int count, int size) {
  USE(count, size);
  if (sp.Is(StackPointer())) {
    // If the current stack pointer is sp, then it must be aligned to 16 bytes
    // on entry and the total size of the specified registers must also be a
    // multiple of 16 bytes.
    VIXL_ASSERT((count * size) % 16 == 0);
  }
}

void MacroAssembler::Poke(const Register& src, const Operand& offset) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (offset.IsImmediate()) {
    VIXL_ASSERT(offset.GetImmediate() >= 0);
  }

  Str(src, MemOperand(StackPointer(), offset));
}


void MacroAssembler::Peek(const Register& dst, const Operand& offset) {
  VIXL_ASSERT(allow_macro_instructions_);
  if (offset.IsImmediate()) {
    VIXL_ASSERT(offset.GetImmediate() >= 0);
  }

  Ldr(dst, MemOperand(StackPointer(), offset));
}


void MacroAssembler::Claim(const Operand& size) {
  VIXL_ASSERT(allow_macro_instructions_);

  if (size.IsZero()) {
    return;
  }

  if (size.IsImmediate()) {
    VIXL_ASSERT(size.GetImmediate() > 0);
    if (sp.Is(StackPointer())) {
      VIXL_ASSERT((size.GetImmediate() % 16) == 0);
    }
  }

  if (!sp.Is(StackPointer())) {
    BumpSystemStackPointer(size);
  }

  Sub(StackPointer(), StackPointer(), size);
}


void MacroAssembler::Drop(const Operand& size) {
  VIXL_ASSERT(allow_macro_instructions_);

  if (size.IsZero()) {
    return;
  }

  if (size.IsImmediate()) {
    VIXL_ASSERT(size.GetImmediate() > 0);
    if (sp.Is(StackPointer())) {
      VIXL_ASSERT((size.GetImmediate() % 16) == 0);
    }
  }

  Add(StackPointer(), StackPointer(), size);
}


void MacroAssembler::PushCalleeSavedRegisters() {
  // Ensure that the macro-assembler doesn't use any scratch registers.
  // 10 stp will be emitted.
  // TODO(all): Should we use GetCalleeSaved and SavedFP.
  ExactAssemblyScope scope(this, 10 * kInstructionSize);

  // This method must not be called unless the current stack pointer is sp.
  VIXL_ASSERT(sp.Is(StackPointer()));

  MemOperand tos(sp, -2 * static_cast<int>(kXRegSizeInBytes), PreIndex);

  stp(x29, x30, tos);
  stp(x27, x28, tos);
  stp(x25, x26, tos);
  stp(x23, x24, tos);
  stp(x21, x22, tos);
  stp(x19, x20, tos);

  stp(d14, d15, tos);
  stp(d12, d13, tos);
  stp(d10, d11, tos);
  stp(d8, d9, tos);
}


void MacroAssembler::PopCalleeSavedRegisters() {
  // Ensure that the macro-assembler doesn't use any scratch registers.
  // 10 ldp will be emitted.
  // TODO(all): Should we use GetCalleeSaved and SavedFP.
  ExactAssemblyScope scope(this, 10 * kInstructionSize);

  // This method must not be called unless the current stack pointer is sp.
  VIXL_ASSERT(sp.Is(StackPointer()));

  MemOperand tos(sp, 2 * kXRegSizeInBytes, PostIndex);

  ldp(d8, d9, tos);
  ldp(d10, d11, tos);
  ldp(d12, d13, tos);
  ldp(d14, d15, tos);

  ldp(x19, x20, tos);
  ldp(x21, x22, tos);
  ldp(x23, x24, tos);
  ldp(x25, x26, tos);
  ldp(x27, x28, tos);
  ldp(x29, x30, tos);
}

void MacroAssembler::LoadCPURegList(CPURegList registers,
                                    const MemOperand& src) {
  LoadStoreCPURegListHelper(kLoad, registers, src);
}

void MacroAssembler::StoreCPURegList(CPURegList registers,
                                     const MemOperand& dst) {
  LoadStoreCPURegListHelper(kStore, registers, dst);
}


void MacroAssembler::LoadStoreCPURegListHelper(LoadStoreCPURegListAction op,
                                               CPURegList registers,
                                               const MemOperand& mem) {
  // We do not handle pre-indexing or post-indexing.
  VIXL_ASSERT(!(mem.IsPreIndex() || mem.IsPostIndex()));
  VIXL_ASSERT(!registers.Overlaps(tmp_list_));
  VIXL_ASSERT(!registers.Overlaps(v_tmp_list_));
  VIXL_ASSERT(!registers.Overlaps(p_tmp_list_));
  VIXL_ASSERT(!registers.IncludesAliasOf(sp));

  UseScratchRegisterScope temps(this);

  MemOperand loc = BaseMemOperandForLoadStoreCPURegList(registers, mem, &temps);
  const int reg_size = registers.GetRegisterSizeInBytes();

  VIXL_ASSERT(IsPowerOf2(reg_size));

  // Since we are operating on register pairs, we would like to align on double
  // the standard size; on the other hand, we don't want to insert an extra
  // operation, which will happen if the number of registers is even. Note that
  // the alignment of the base pointer is unknown here, but we assume that it
  // is more likely to be aligned.
  if (((loc.GetOffset() & (2 * reg_size - 1)) != 0) &&
      ((registers.GetCount() % 2) != 0)) {
    if (op == kStore) {
      Str(registers.PopLowestIndex(), loc);
    } else {
      VIXL_ASSERT(op == kLoad);
      Ldr(registers.PopLowestIndex(), loc);
    }
    loc.AddOffset(reg_size);
  }
  while (registers.GetCount() >= 2) {
    const CPURegister& dst0 = registers.PopLowestIndex();
    const CPURegister& dst1 = registers.PopLowestIndex();
    if (op == kStore) {
      Stp(dst0, dst1, loc);
    } else {
      VIXL_ASSERT(op == kLoad);
      Ldp(dst0, dst1, loc);
    }
    loc.AddOffset(2 * reg_size);
  }
  if (!registers.IsEmpty()) {
    if (op == kStore) {
      Str(registers.PopLowestIndex(), loc);
    } else {
      VIXL_ASSERT(op == kLoad);
      Ldr(registers.PopLowestIndex(), loc);
    }
  }
}

MemOperand MacroAssembler::BaseMemOperandForLoadStoreCPURegList(
    const CPURegList& registers,
    const MemOperand& mem,
    UseScratchRegisterScope* scratch_scope) {
  // If necessary, pre-compute the base address for the accesses.
  if (mem.IsRegisterOffset()) {
    Register reg_base = scratch_scope->AcquireX();
    ComputeAddress(reg_base, mem);
    return MemOperand(reg_base);

  } else if (mem.IsImmediateOffset()) {
    int reg_size = registers.GetRegisterSizeInBytes();
    int total_size = registers.GetTotalSizeInBytes();
    int64_t min_offset = mem.GetOffset();
    int64_t max_offset =
        mem.GetOffset() + std::max(0, total_size - 2 * reg_size);
    if ((registers.GetCount() >= 2) &&
        (!Assembler::IsImmLSPair(min_offset, WhichPowerOf2(reg_size)) ||
         !Assembler::IsImmLSPair(max_offset, WhichPowerOf2(reg_size)))) {
      Register reg_base = scratch_scope->AcquireX();
      ComputeAddress(reg_base, mem);
      return MemOperand(reg_base);
    }
  }

  return mem;
}

void MacroAssembler::BumpSystemStackPointer(const Operand& space) {
  VIXL_ASSERT(!sp.Is(StackPointer()));
  // TODO: Several callers rely on this not using scratch registers, so we use
  // the assembler directly here. However, this means that large immediate
  // values of 'space' cannot be handled.
  ExactAssemblyScope scope(this, kInstructionSize);
  sub(sp, StackPointer(), space);
}


// TODO(all): Fix printf for NEON and SVE registers.

// This is the main Printf implementation. All callee-saved registers are
// preserved, but NZCV and the caller-saved registers may be clobbered.
void MacroAssembler::PrintfNoPreserve(const char* format,
                                      const CPURegister& arg0,
                                      const CPURegister& arg1,
                                      const CPURegister& arg2,
                                      const CPURegister& arg3) {
  // We cannot handle a caller-saved stack pointer. It doesn't make much sense
  // in most cases anyway, so this restriction shouldn't be too serious.
  VIXL_ASSERT(!kCallerSaved.IncludesAliasOf(StackPointer()));

  // The provided arguments, and their proper PCS registers.
  CPURegister args[kPrintfMaxArgCount] = {arg0, arg1, arg2, arg3};
  CPURegister pcs[kPrintfMaxArgCount];

  int arg_count = kPrintfMaxArgCount;

  // The PCS varargs registers for printf. Note that x0 is used for the printf
  // format string.
  static const CPURegList kPCSVarargs =
      CPURegList(CPURegister::kRegister, kXRegSize, 1, arg_count);
  static const CPURegList kPCSVarargsV =
      CPURegList(CPURegister::kVRegister, kDRegSize, 0, arg_count - 1);

  // We can use caller-saved registers as scratch values, except for the
  // arguments and the PCS registers where they might need to go.
  UseScratchRegisterScope temps(this);
  temps.Include(kCallerSaved);
  temps.Include(kCallerSavedV);
  temps.Exclude(kPCSVarargs);
  temps.Exclude(kPCSVarargsV);
  temps.Exclude(arg0, arg1, arg2, arg3);

  // Copies of the arg lists that we can iterate through.
  CPURegList pcs_varargs = kPCSVarargs;
  CPURegList pcs_varargs_fp = kPCSVarargsV;

  // Place the arguments. There are lots of clever tricks and optimizations we
  // could use here, but Printf is a debug tool so instead we just try to keep
  // it simple: Move each input that isn't already in the right place to a
  // scratch register, then move everything back.
  for (unsigned i = 0; i < kPrintfMaxArgCount; i++) {
    // Work out the proper PCS register for this argument.
    if (args[i].IsRegister()) {
      pcs[i] = pcs_varargs.PopLowestIndex().X();
      // We might only need a W register here. We need to know the size of the
      // argument so we can properly encode it for the simulator call.
      if (args[i].Is32Bits()) pcs[i] = pcs[i].W();
    } else if (args[i].IsVRegister()) {
      // In C, floats are always cast to doubles for varargs calls.
      pcs[i] = pcs_varargs_fp.PopLowestIndex().D();
    } else {
      VIXL_ASSERT(args[i].IsNone());
      arg_count = i;
      break;
    }

    // If the argument is already in the right place, leave it where it is.
    if (args[i].Aliases(pcs[i])) continue;

    // Otherwise, if the argument is in a PCS argument register, allocate an
    // appropriate scratch register and then move it out of the way.
    if (kPCSVarargs.IncludesAliasOf(args[i]) ||
        kPCSVarargsV.IncludesAliasOf(args[i])) {
      if (args[i].IsRegister()) {
        Register old_arg = Register(args[i]);
        Register new_arg = temps.AcquireSameSizeAs(old_arg);
        Mov(new_arg, old_arg);
        args[i] = new_arg;
      } else {
        VRegister old_arg(args[i]);
        VRegister new_arg = temps.AcquireSameSizeAs(old_arg);
        Fmov(new_arg, old_arg);
        args[i] = new_arg;
      }
    }
  }

  // Do a second pass to move values into their final positions and perform any
  // conversions that may be required.
  for (int i = 0; i < arg_count; i++) {
    VIXL_ASSERT(pcs[i].GetType() == args[i].GetType());
    if (pcs[i].IsRegister()) {
      Mov(Register(pcs[i]), Register(args[i]), kDiscardForSameWReg);
    } else {
      VIXL_ASSERT(pcs[i].IsVRegister());
      if (pcs[i].GetSizeInBits() == args[i].GetSizeInBits()) {
        Fmov(VRegister(pcs[i]), VRegister(args[i]));
      } else {
        Fcvt(VRegister(pcs[i]), VRegister(args[i]));
      }
    }
  }

  // Load the format string into x0, as per the procedure-call standard.
  //
  // To make the code as portable as possible, the format string is encoded
  // directly in the instruction stream. It might be cleaner to encode it in a
  // literal pool, but since Printf is usually used for debugging, it is
  // beneficial for it to be minimally dependent on other features.
  temps.Exclude(x0);
  Label format_address;
  Adr(x0, &format_address);

  // Emit the format string directly in the instruction stream.
  {
    BlockPoolsScope scope(this);
    // Data emitted:
    //   branch
    //   strlen(format) + 1 (includes null termination)
    //   padding to next instruction
    //   unreachable
    EmissionCheckScope guard(this,
                             AlignUp(strlen(format) + 1, kInstructionSize) +
                                 2 * kInstructionSize);
    Label after_data;
    B(&after_data);
    Bind(&format_address);
    EmitString(format);
    Unreachable();
    Bind(&after_data);
  }

  // We don't pass any arguments on the stack, but we still need to align the C
  // stack pointer to a 16-byte boundary for PCS compliance.
  if (!sp.Is(StackPointer())) {
    Bic(sp, StackPointer(), 0xf);
  }

  // Actually call printf. This part needs special handling for the simulator,
  // since the system printf function will use a different instruction set and
  // the procedure-call standard will not be compatible.
  if (generate_simulator_code_) {
    ExactAssemblyScope scope(this, kPrintfLength);
    hlt(kPrintfOpcode);
    dc32(arg_count);  // kPrintfArgCountOffset

    // Determine the argument pattern.
    uint32_t arg_pattern_list = 0;
    for (int i = 0; i < arg_count; i++) {
      uint32_t arg_pattern;
      if (pcs[i].IsRegister()) {
        arg_pattern = pcs[i].Is32Bits() ? kPrintfArgW : kPrintfArgX;
      } else {
        VIXL_ASSERT(pcs[i].Is64Bits());
        arg_pattern = kPrintfArgD;
      }
      VIXL_ASSERT(arg_pattern < (1 << kPrintfArgPatternBits));
      arg_pattern_list |= (arg_pattern << (kPrintfArgPatternBits * i));
    }
    dc32(arg_pattern_list);  // kPrintfArgPatternListOffset
  } else {
    Register tmp = temps.AcquireX();
    Mov(tmp, reinterpret_cast<uintptr_t>(printf));
    Blr(tmp);
  }
}


void MacroAssembler::Printf(const char* format,
                            CPURegister arg0,
                            CPURegister arg1,
                            CPURegister arg2,
                            CPURegister arg3) {
  // We can only print sp if it is the current stack pointer.
  if (!sp.Is(StackPointer())) {
    VIXL_ASSERT(!sp.Aliases(arg0));
    VIXL_ASSERT(!sp.Aliases(arg1));
    VIXL_ASSERT(!sp.Aliases(arg2));
    VIXL_ASSERT(!sp.Aliases(arg3));
  }

  // Make sure that the macro assembler doesn't try to use any of our arguments
  // as scratch registers.
  UseScratchRegisterScope exclude_all(this);
  exclude_all.ExcludeAll();

  // Preserve all caller-saved registers as well as NZCV.
  // If sp is the stack pointer, PushCPURegList asserts that the size of each
  // list is a multiple of 16 bytes.
  PushCPURegList(kCallerSaved);
  PushCPURegList(kCallerSavedV);

  {
    UseScratchRegisterScope temps(this);
    // We can use caller-saved registers as scratch values (except for argN).
    temps.Include(kCallerSaved);
    temps.Include(kCallerSavedV);
    temps.Exclude(arg0, arg1, arg2, arg3);

    // If any of the arguments are the current stack pointer, allocate a new
    // register for them, and adjust the value to compensate for pushing the
    // caller-saved registers.
    bool arg0_sp = StackPointer().Aliases(arg0);
    bool arg1_sp = StackPointer().Aliases(arg1);
    bool arg2_sp = StackPointer().Aliases(arg2);
    bool arg3_sp = StackPointer().Aliases(arg3);
    if (arg0_sp || arg1_sp || arg2_sp || arg3_sp) {
      // Allocate a register to hold the original stack pointer value, to pass
      // to PrintfNoPreserve as an argument.
      Register arg_sp = temps.AcquireX();
      Add(arg_sp,
          StackPointer(),
          kCallerSaved.GetTotalSizeInBytes() +
              kCallerSavedV.GetTotalSizeInBytes());
      if (arg0_sp) arg0 = Register(arg_sp.GetCode(), arg0.GetSizeInBits());
      if (arg1_sp) arg1 = Register(arg_sp.GetCode(), arg1.GetSizeInBits());
      if (arg2_sp) arg2 = Register(arg_sp.GetCode(), arg2.GetSizeInBits());
      if (arg3_sp) arg3 = Register(arg_sp.GetCode(), arg3.GetSizeInBits());
    }

    // Preserve NZCV.
    Register tmp = temps.AcquireX();
    Mrs(tmp, NZCV);
    Push(tmp, xzr);
    temps.Release(tmp);

    PrintfNoPreserve(format, arg0, arg1, arg2, arg3);

    // Restore NZCV.
    tmp = temps.AcquireX();
    Pop(xzr, tmp);
    Msr(NZCV, tmp);
    temps.Release(tmp);
  }

  PopCPURegList(kCallerSavedV);
  PopCPURegList(kCallerSaved);
}

void MacroAssembler::Trace(TraceParameters parameters, TraceCommand command) {
  VIXL_ASSERT(allow_macro_instructions_);

  if (generate_simulator_code_) {
    // The arguments to the trace pseudo instruction need to be contiguous in
    // memory, so make sure we don't try to emit a literal pool.
    ExactAssemblyScope scope(this, kTraceLength);

    Label start;
    bind(&start);

    // Refer to simulator-aarch64.h for a description of the marker and its
    // arguments.
    hlt(kTraceOpcode);

    VIXL_ASSERT(GetSizeOfCodeGeneratedSince(&start) == kTraceParamsOffset);
    dc32(parameters);

    VIXL_ASSERT(GetSizeOfCodeGeneratedSince(&start) == kTraceCommandOffset);
    dc32(command);
  } else {
    // Emit nothing on real hardware.
    USE(parameters, command);
  }
}


void MacroAssembler::Log(TraceParameters parameters) {
  VIXL_ASSERT(allow_macro_instructions_);

  if (generate_simulator_code_) {
    // The arguments to the log pseudo instruction need to be contiguous in
    // memory, so make sure we don't try to emit a literal pool.
    ExactAssemblyScope scope(this, kLogLength);

    Label start;
    bind(&start);

    // Refer to simulator-aarch64.h for a description of the marker and its
    // arguments.
    hlt(kLogOpcode);

    VIXL_ASSERT(GetSizeOfCodeGeneratedSince(&start) == kLogParamsOffset);
    dc32(parameters);
  } else {
    // Emit nothing on real hardware.
    USE(parameters);
  }
}


void MacroAssembler::SetSimulatorCPUFeatures(const CPUFeatures& features) {
  ConfigureSimulatorCPUFeaturesHelper(features, kSetCPUFeaturesOpcode);
}


void MacroAssembler::EnableSimulatorCPUFeatures(const CPUFeatures& features) {
  ConfigureSimulatorCPUFeaturesHelper(features, kEnableCPUFeaturesOpcode);
}


void MacroAssembler::DisableSimulatorCPUFeatures(const CPUFeatures& features) {
  ConfigureSimulatorCPUFeaturesHelper(features, kDisableCPUFeaturesOpcode);
}


void MacroAssembler::ConfigureSimulatorCPUFeaturesHelper(
    const CPUFeatures& features, DebugHltOpcode action) {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(generate_simulator_code_);

  typedef ConfigureCPUFeaturesElementType ElementType;
  VIXL_ASSERT(CPUFeatures::kNumberOfFeatures <=
              std::numeric_limits<ElementType>::max());

  size_t count = features.Count();

  size_t preamble_length = kConfigureCPUFeaturesListOffset;
  size_t list_length = (count + 1) * sizeof(ElementType);
  size_t padding_length = AlignUp(list_length, kInstructionSize) - list_length;

  size_t total_length = preamble_length + list_length + padding_length;

  // Check the overall code size as well as the size of each component.
  ExactAssemblyScope guard_total(this, total_length);

  {  // Preamble: the opcode itself.
    ExactAssemblyScope guard_preamble(this, preamble_length);
    hlt(action);
  }
  {  // A kNone-terminated list of features.
    ExactAssemblyScope guard_list(this, list_length);
    for (CPUFeatures::const_iterator it = features.begin();
         it != features.end();
         ++it) {
      dc(static_cast<ElementType>(*it));
    }
    dc(static_cast<ElementType>(CPUFeatures::kNone));
  }
  {  // Padding for instruction alignment.
    ExactAssemblyScope guard_padding(this, padding_length);
    for (size_t size = 0; size < padding_length; size += sizeof(ElementType)) {
      // The exact value is arbitrary.
      dc(static_cast<ElementType>(CPUFeatures::kNone));
    }
  }
}

void MacroAssembler::SaveSimulatorCPUFeatures() {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(generate_simulator_code_);
  SingleEmissionCheckScope guard(this);
  hlt(kSaveCPUFeaturesOpcode);
}


void MacroAssembler::RestoreSimulatorCPUFeatures() {
  VIXL_ASSERT(allow_macro_instructions_);
  VIXL_ASSERT(generate_simulator_code_);
  SingleEmissionCheckScope guard(this);
  hlt(kRestoreCPUFeaturesOpcode);
}


void UseScratchRegisterScope::Open(MacroAssembler* masm) {
  VIXL_ASSERT(masm_ == NULL);
  VIXL_ASSERT(masm != NULL);
  masm_ = masm;

  CPURegList* available = masm->GetScratchRegisterList();
  CPURegList* available_v = masm->GetScratchVRegisterList();
  CPURegList* available_p = masm->GetScratchPRegisterList();
  old_available_ = available->GetList();
  old_available_v_ = available_v->GetList();
  old_available_p_ = available_p->GetList();
  VIXL_ASSERT(available->GetType() == CPURegister::kRegister);
  VIXL_ASSERT(available_v->GetType() == CPURegister::kVRegister);
  VIXL_ASSERT(available_p->GetType() == CPURegister::kPRegister);

  parent_ = masm->GetCurrentScratchRegisterScope();
  masm->SetCurrentScratchRegisterScope(this);
}


void UseScratchRegisterScope::Close() {
  if (masm_ != NULL) {
    // Ensure that scopes nest perfectly, and do not outlive their parents.
    // This is a run-time check because the order of destruction of objects in
    // the _same_ scope is implementation-defined, and is likely to change in
    // optimised builds.
    VIXL_CHECK(masm_->GetCurrentScratchRegisterScope() == this);
    masm_->SetCurrentScratchRegisterScope(parent_);

    masm_->GetScratchRegisterList()->SetList(old_available_);
    masm_->GetScratchVRegisterList()->SetList(old_available_v_);
    masm_->GetScratchPRegisterList()->SetList(old_available_p_);

    masm_ = NULL;
  }
}


bool UseScratchRegisterScope::IsAvailable(const CPURegister& reg) const {
  return masm_->GetScratchRegisterList()->IncludesAliasOf(reg) ||
         masm_->GetScratchVRegisterList()->IncludesAliasOf(reg) ||
         masm_->GetScratchPRegisterList()->IncludesAliasOf(reg);
}

Register UseScratchRegisterScope::AcquireRegisterOfSize(int size_in_bits) {
  int code = AcquireFrom(masm_->GetScratchRegisterList()).GetCode();
  return Register(code, size_in_bits);
}


VRegister UseScratchRegisterScope::AcquireVRegisterOfSize(int size_in_bits) {
  int code = AcquireFrom(masm_->GetScratchVRegisterList()).GetCode();
  return VRegister(code, size_in_bits);
}


void UseScratchRegisterScope::Release(const CPURegister& reg) {
  VIXL_ASSERT(masm_ != NULL);

  // Release(NoReg) has no effect.
  if (reg.IsNone()) return;

  ReleaseByCode(GetAvailableListFor(reg.GetBank()), reg.GetCode());
}


void UseScratchRegisterScope::Include(const CPURegList& list) {
  VIXL_ASSERT(masm_ != NULL);

  // Including an empty list has no effect.
  if (list.IsEmpty()) return;
  VIXL_ASSERT(list.GetType() != CPURegister::kNoRegister);

  RegList reg_list = list.GetList();
  if (list.GetType() == CPURegister::kRegister) {
    // Make sure that neither sp nor xzr are included the list.
    reg_list &= ~(xzr.GetBit() | sp.GetBit());
  }

  IncludeByRegList(GetAvailableListFor(list.GetBank()), reg_list);
}


void UseScratchRegisterScope::Include(const Register& reg1,
                                      const Register& reg2,
                                      const Register& reg3,
                                      const Register& reg4) {
  VIXL_ASSERT(masm_ != NULL);
  RegList include =
      reg1.GetBit() | reg2.GetBit() | reg3.GetBit() | reg4.GetBit();
  // Make sure that neither sp nor xzr are included the list.
  include &= ~(xzr.GetBit() | sp.GetBit());

  IncludeByRegList(masm_->GetScratchRegisterList(), include);
}


void UseScratchRegisterScope::Include(const VRegister& reg1,
                                      const VRegister& reg2,
                                      const VRegister& reg3,
                                      const VRegister& reg4) {
  RegList include =
      reg1.GetBit() | reg2.GetBit() | reg3.GetBit() | reg4.GetBit();
  IncludeByRegList(masm_->GetScratchVRegisterList(), include);
}


void UseScratchRegisterScope::Include(const CPURegister& reg1,
                                      const CPURegister& reg2,
                                      const CPURegister& reg3,
                                      const CPURegister& reg4) {
  RegList include = 0;
  RegList include_v = 0;
  RegList include_p = 0;

  const CPURegister regs[] = {reg1, reg2, reg3, reg4};

  for (size_t i = 0; i < ArrayLength(regs); i++) {
    RegList bit = regs[i].GetBit();
    switch (regs[i].GetBank()) {
      case CPURegister::kNoRegisterBank:
        // Include(NoReg) has no effect.
        VIXL_ASSERT(regs[i].IsNone());
        break;
      case CPURegister::kRRegisterBank:
        include |= bit;
        break;
      case CPURegister::kVRegisterBank:
        include_v |= bit;
        break;
      case CPURegister::kPRegisterBank:
        include_p |= bit;
        break;
    }
  }

  IncludeByRegList(masm_->GetScratchRegisterList(), include);
  IncludeByRegList(masm_->GetScratchVRegisterList(), include_v);
  IncludeByRegList(masm_->GetScratchPRegisterList(), include_p);
}


void UseScratchRegisterScope::Exclude(const CPURegList& list) {
  ExcludeByRegList(GetAvailableListFor(list.GetBank()), list.GetList());
}


void UseScratchRegisterScope::Exclude(const Register& reg1,
                                      const Register& reg2,
                                      const Register& reg3,
                                      const Register& reg4) {
  RegList exclude =
      reg1.GetBit() | reg2.GetBit() | reg3.GetBit() | reg4.GetBit();
  ExcludeByRegList(masm_->GetScratchRegisterList(), exclude);
}


void UseScratchRegisterScope::Exclude(const VRegister& reg1,
                                      const VRegister& reg2,
                                      const VRegister& reg3,
                                      const VRegister& reg4) {
  RegList exclude_v =
      reg1.GetBit() | reg2.GetBit() | reg3.GetBit() | reg4.GetBit();
  ExcludeByRegList(masm_->GetScratchVRegisterList(), exclude_v);
}


void UseScratchRegisterScope::Exclude(const CPURegister& reg1,
                                      const CPURegister& reg2,
                                      const CPURegister& reg3,
                                      const CPURegister& reg4) {
  RegList exclude = 0;
  RegList exclude_v = 0;
  RegList exclude_p = 0;

  const CPURegister regs[] = {reg1, reg2, reg3, reg4};

  for (size_t i = 0; i < ArrayLength(regs); i++) {
    RegList bit = regs[i].GetBit();
    switch (regs[i].GetBank()) {
      case CPURegister::kNoRegisterBank:
        // Exclude(NoReg) has no effect.
        VIXL_ASSERT(regs[i].IsNone());
        break;
      case CPURegister::kRRegisterBank:
        exclude |= bit;
        break;
      case CPURegister::kVRegisterBank:
        exclude_v |= bit;
        break;
      case CPURegister::kPRegisterBank:
        exclude_p |= bit;
        break;
    }
  }

  ExcludeByRegList(masm_->GetScratchRegisterList(), exclude);
  ExcludeByRegList(masm_->GetScratchVRegisterList(), exclude_v);
  ExcludeByRegList(masm_->GetScratchPRegisterList(), exclude_p);
}


void UseScratchRegisterScope::ExcludeAll() {
  ExcludeByRegList(masm_->GetScratchRegisterList(),
                   masm_->GetScratchRegisterList()->GetList());
  ExcludeByRegList(masm_->GetScratchVRegisterList(),
                   masm_->GetScratchVRegisterList()->GetList());
  ExcludeByRegList(masm_->GetScratchPRegisterList(),
                   masm_->GetScratchPRegisterList()->GetList());
}


CPURegister UseScratchRegisterScope::AcquireFrom(CPURegList* available,
                                                 RegList mask) {
  VIXL_CHECK((available->GetList() & mask) != 0);
  CPURegister result = available->PopLowestIndex(mask);
  VIXL_ASSERT(!AreAliased(result, xzr, sp));
  return result;
}


void UseScratchRegisterScope::ReleaseByCode(CPURegList* available, int code) {
  ReleaseByRegList(available, static_cast<RegList>(1) << code);
}


void UseScratchRegisterScope::ReleaseByRegList(CPURegList* available,
                                               RegList regs) {
  available->SetList(available->GetList() | regs);
}


void UseScratchRegisterScope::IncludeByRegList(CPURegList* available,
                                               RegList regs) {
  available->SetList(available->GetList() | regs);
}


void UseScratchRegisterScope::ExcludeByRegList(CPURegList* available,
                                               RegList exclude) {
  available->SetList(available->GetList() & ~exclude);
}

CPURegList* UseScratchRegisterScope::GetAvailableListFor(
    CPURegister::RegisterBank bank) {
  switch (bank) {
    case CPURegister::kNoRegisterBank:
      return NULL;
    case CPURegister::kRRegisterBank:
      return masm_->GetScratchRegisterList();
    case CPURegister::kVRegisterBank:
      return masm_->GetScratchVRegisterList();
    case CPURegister::kPRegisterBank:
      return masm_->GetScratchPRegisterList();
  }
  VIXL_UNREACHABLE();
  return NULL;
}

}  // namespace aarch64
}  // namespace vixl
