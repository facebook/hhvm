/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/jit/smashable-instr-arm.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/alignment.h"
#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/cg-meta.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * For smashable jmps and calls in ARM, we emit the target address straight
 * into the instruction stream, and then do a pc-relative load to read it.
 *
 * This neatly sidesteps the problem of concurrent modification and execution,
 * as well as the problem of 19- and 26-bit jump offsets (not big enough).  It
 * does, however, entail an indirect jump.
 */

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& meta, uint64_t imm,
                      PhysReg d) {
  align(cb, &meta, Alignment::SmashMovq, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label imm_data;
  vixl::Label after_data;

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  a.    Ldr  (x2a(d), &imm_data);
  a.    B    (&after_data);

  // Emit the immediate into the instruction stream.
  a.    bind (&imm_data);
  a.    dc64 (imm);
  a.    bind (&after_data);

  cb.sync(the_start);
  return the_start;
}

TCA emitSmashableCmpq(CodeBlock& /*cb*/, CGMeta& /*meta*/, int32_t /*imm*/,
                      PhysReg /*r*/, int8_t /*disp*/) {
  // FIXME: This is used in func-guard*
  not_implemented();
}

// SmashableCalls don't embed the target at the end, because the
// BLR must be the last instruction of the sequence.
TCA emitSmashableCall(CodeBlock& cb, CGMeta& meta, TCA target) {
  align(cb, &meta, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  vixl::Label after_data;

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  // Jump over the data
  a.    B    (&after_data);

  // Emit the call target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (target);
  a.    bind (&after_data);

  // Load the target address and call it
  a.    Ldr  (rAsm, &target_data);
  a.    Blr  (rAsm);

  cb.sync(the_start);
  return the_start;
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& meta, TCA target) {
  align(cb, &meta, Alignment::SmashJmp, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  a.    Ldr  (rAsm_w, &target_data);
  a.    Br   (rAsm);

  // Emit the jmp target into the instruction stream.
  a.    bind (&target_data);
  a.    dc32 (makeTarget32(target));

  cb.sync(the_start);
  return the_start;
}

// While a b.cc can be overwritten on ARM, if the cc and the target
// are both changed then the behavior can cause old cc to jump to new
// target or new cc to jump to old target. Therefore we'll keep
// the branch as an indirect branch to a target stored in the
// instruction stream. This way we can at least guarantee that old cc
// won't jump to new target. We can still have an issue where new cc
// jumps to old target, but that old target is *likely* a stub.
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& meta, TCA target,
                     ConditionCode cc) {
  align(cb, &meta, Alignment::SmashJcc, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  vixl::Label after_data;

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  // Emit the conditional branch
  a.    B    (&after_data, InvertCondition(arm::convertCC(cc)));

  // Emit the smashable jump
  a.    Ldr  (rAsm_w, &target_data);
  a.    Br   (rAsm);

  // Emit the jmp target into the instruction stream.
  a.    bind (&target_data);
  a.    dc32 (makeTarget32(target));

  a.    bind (&after_data);

  cb.sync(the_start);
  return the_start;
}

///////////////////////////////////////////////////////////////////////////////

bool isSmashableMovq(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);
  Instruction* b = ldr->NextInstruction();
  Instruction* target = b->NextInstruction();
  Instruction* after = target->NextInstruction()->NextInstruction();

  return (ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_x_lit &&
          ldr->ImmPCOffsetTarget() == target &&
          b->Mask(UnconditionalBranchMask) == B &&
          b->ImmPCOffsetTarget() == after);
}

bool isSmashableCall(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* target = b->NextInstruction();
  Instruction* ldr = target->NextInstruction()->NextInstruction();
  Instruction* blr = ldr->NextInstruction();
  const auto rd = ldr->Rd();

  return (b->Mask(UnconditionalBranchMask) == B &&
          b->ImmPCOffsetTarget() == ldr &&
          ldr->Mask(LoadLiteralMask) == LDR_x_lit &&
          ldr->ImmPCOffsetTarget() == target &&
          blr->Mask(UnconditionalBranchToRegisterMask) == BLR &&
          blr->Rn() == rd);
}

bool isSmashableJmp(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);
  Instruction* br = ldr->NextInstruction();
  Instruction* target = br->NextInstruction();
  const auto rd = ldr->Rd();

  return (ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          ldr->ImmPCOffsetTarget() == target &&
          br->Mask(UnconditionalBranchToRegisterMask) == BR &&
          br->Rn() == rd);
}

bool isSmashableJcc(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* ldr = b->NextInstruction();;
  Instruction* br = ldr->NextInstruction();
  Instruction* target = br->NextInstruction();
  Instruction* after = target->NextInstruction();
  const auto rd = ldr->Rd();

  return (b->IsCondBranchImm() &&
          b->ImmPCOffsetTarget() == after &&
          ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          ldr->ImmPCOffsetTarget() == target &&
          br->Mask(UnconditionalBranchToRegisterMask) == BR &&
          br->Rn() == rd);
}

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t target) {
  assertx(isSmashableMovq(inst));
  patchTarget64(inst + smashableMovqLen() - 8, reinterpret_cast<TCA>(target));
}

void smashCmpq(TCA /*inst*/, uint32_t /*target*/) {
  not_implemented();
}

void smashCall(TCA inst, TCA target) {
  assertx(isSmashableCall(inst));
  // Note: The target is not at the end of the smashableCall.
  patchTarget64(inst + (1 * 4), target);
}

void smashJmp(TCA inst, TCA target) {
  assertx(isSmashableJmp(inst));
  // If the target is within the smashable jmp, then set the target to the
  // end. This mirrors logic in x86_64 with the exception that ARM cannot
  // replace the entire smashable jmp with nops.
  if (target > inst && target - inst <= smashableJmpLen()) {
    target = inst + smashableJmpLen();
  }
  patchTarget32(inst + smashableJmpLen() - 4, target);
}

void smashJcc(TCA inst, TCA target) {
  assertx(isSmashableJcc(inst));
  if (smashableJccTarget(inst) != target) {
    patchTarget32(inst + smashableJccLen() - 4, target);
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  using namespace vixl;

  assertx(isSmashableMovq(inst));
  Instruction* target =
    Instruction::Cast(inst)->NextInstruction()->NextInstruction();
  return *reinterpret_cast<uint64_t*>(target);
}

uint32_t smashableCmpqImm(TCA /*inst*/) {
  not_implemented();
}

TCA smashableCallTarget(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* target = b->NextInstruction();

  if (isSmashableCall(inst)) {
    assertx((reinterpret_cast<uintptr_t>(target) & 3) == 0);
    return *reinterpret_cast<TCA*>(target);
  }
  return nullptr;
}

TCA smashableJmpTarget(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);
  Instruction* br = ldr->NextInstruction();
  Instruction* target = br->NextInstruction();

  if (isSmashableJmp(inst)) {
    assertx((reinterpret_cast<uintptr_t>(target) & 3) == 0);
    const uint32_t target32 = *reinterpret_cast<uint32_t*>(target);
    return reinterpret_cast<TCA>(target32);
  }
  return nullptr;
}

TCA smashableJccTarget(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* ldr = b->NextInstruction();;
  Instruction* br = ldr->NextInstruction();
  Instruction* target = br->NextInstruction();

  if (isSmashableJcc(inst)) {
    assertx((reinterpret_cast<uintptr_t>(target) & 3) == 0);
    const uint32_t target32 = *reinterpret_cast<uint32_t*>(target);
    return reinterpret_cast<TCA>(target32);
  }
  return nullptr;
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);

  assertx(isSmashableJcc(inst));

  return arm::convertCC(InvertCondition(static_cast<Condition>(b->Bits(3, 0))));
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Helper function which determines if a target literal belongs to a smashable
 * sequence. Takes the address of the target literal and analyzes the
 * instructions around the literal to determine if the sequence is a smashable
 * jcc, jmp, or a call. If it is, then the TCA at the start of the sequence is
 * returned. Otherwise a nullptr is returned.
 *
 * Note: The analysis is performed such that a jmp is not returned when the
 *       sequence is a full jcc even though a jcc actually uses the same
 *       sequence as a jmp in its implementation.
 */
TCA getSmashableFromTargetAddr(TCA addr) {
  using namespace vixl;

  const uint32_t target32 = *reinterpret_cast<uint32_t*>(addr);
  auto target = reinterpret_cast<TCA>(target32);

  auto inst = addr - 3 * kInstructionSize;
  if (smashableJccTarget(inst) == target) return inst;

  inst = addr - 2 * kInstructionSize;
  if (smashableJmpTarget(inst) == target) return inst;

  inst = addr - 1 * kInstructionSize;
  if (smashableCallTarget(inst) == target) return inst;

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}}}
