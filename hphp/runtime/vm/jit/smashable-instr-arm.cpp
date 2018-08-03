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

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  poolLiteral(cb, meta, (uint64_t)imm, 64, true);
  a.    bind (&imm_data);
  a.    Ldr  (x2a(d), &imm_data);

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

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  assertx((makeTarget32(target) & 3) == 0);
  meta.addressImmediates.insert(cb.frontier());
  // Load the target address and call it
  poolLiteral(cb, meta, (uint64_t)makeTarget32(target), 32, true);
  a.    bind (&target_data);
  a.    Ldr  (rAsm_w, &target_data);
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

  assertx((makeTarget32(target) & 3) == 0);
  meta.addressImmediates.insert(cb.frontier());
  poolLiteral(cb, meta, (uint64_t)makeTarget32(target), 32, true);
  a.    bind (&target_data);
  a.    Ldr  (rAsm_w, &target_data);
  a.    Br   (rAsm);

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
  vixl::Label after;

  meta.smashableLocations.insert(cb.frontier());
  auto const the_start = cb.frontier();

  // Emit the conditional branch
  a.    B    (&after, InvertCondition(arm::convertCC(cc)));

  assertx((makeTarget32(target) & 3) == 0);
  meta.addressImmediates.insert(cb.frontier());
  // Emit the smashable jump
  poolLiteral(cb, meta, (uint64_t)makeTarget32(target), 32, true);
  a.    bind (&target_data);
  a.    Ldr  (rAsm_w, &target_data);
  a.    Br   (rAsm);

  a.    bind (&after);

  cb.sync(the_start);
  return the_start;
}

///////////////////////////////////////////////////////////////////////////////

namespace {
// The vasm instruction after a smashable may have had nops inserted before it.
// In that case the branch jumping over its immediate will point after the nop
// sequence rather than immediately after the smashable.  This helper is used
// to check the branch is pointing to an appropriate address.
//
// Returns true if target points to inst or a chain of Nops leading to inst.
// Otherwise it returns false.
bool targetsInst(vixl::Instruction* target, vixl::Instruction* inst) {
  while (target != inst && inst->IsNop()) {
    inst = inst->NextInstruction();
  }
  return target == inst;
}
}

bool possiblySmashableMovq(TCA inst) {
  using namespace vixl;
  Instruction* ldr = Instruction::Cast(inst);
  return (ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_x_lit);
}

bool possiblySmashableCall(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);
  Instruction* blr = ldr->NextInstruction();
  const auto rd = ldr->Rd();

  return (ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          blr->Mask(UnconditionalBranchToRegisterMask) == BLR &&
          blr->Rn() == rd);
}

bool possiblySmashableJmp(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);
  Instruction* br = ldr->NextInstruction();
  const auto rd = ldr->Rd();

  return (ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          br->Mask(UnconditionalBranchToRegisterMask) == BR &&
          br->Rn() == rd);
}

bool possiblySmashableJcc(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* ldr = b->NextInstruction();;
  Instruction* br = ldr->NextInstruction();
  Instruction* after = br->NextInstruction();
  const auto rd = ldr->Rd();

  return (b->IsCondBranchImm() &&
          targetsInst(b->ImmPCOffsetTarget(), after) &&
          ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          br->Mask(UnconditionalBranchToRegisterMask) == BR &&
          br->Rn() == rd);
}

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t target) {
  using namespace vixl;
  assertx(possiblySmashableMovq(inst));

  Instruction* ldr = Instruction::Cast(inst);
  patchTarget64(ldr->LiteralAddress(), reinterpret_cast<TCA>(target));
}

void smashCmpq(TCA /*inst*/, uint32_t /*target*/) {
  not_implemented();
}

void smashCall(TCA inst, TCA target) {
  using namespace vixl;
  assertx(possiblySmashableCall(inst));
  Instruction* ldr = Instruction::Cast(inst);
  patchTarget32(ldr->LiteralAddress(), target);
}

void smashJmp(TCA inst, TCA target) {
  using namespace vixl;
  assertx(possiblySmashableJmp(inst));
  // If the target is within the smashable jmp, then set the target to the
  // end. This mirrors logic in x86_64 with the exception that ARM cannot
  // replace the entire smashable jmp with nops.
  if (target > inst && target - inst <= smashableJmpLen()) {
    target = inst + smashableJmpLen();
  }
  Instruction* ldr = Instruction::Cast(inst);
  patchTarget32(ldr->LiteralAddress(), target);
}

void smashJcc(TCA inst, TCA target) {
  using namespace vixl;
  assertx(possiblySmashableJcc(inst));
  if (smashableJccTarget(inst) != target) {
    Instruction* b = Instruction::Cast(inst);
    Instruction* ldr = b->NextInstruction();
    patchTarget32(ldr->LiteralAddress(), target);
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  using namespace vixl;

  assertx(possiblySmashableMovq(inst));
  Instruction* ldr = Instruction::Cast(inst);
  return *reinterpret_cast<uint64_t*>(ldr->LiteralAddress());
}

uint32_t smashableCmpqImm(TCA /*inst*/) {
  not_implemented();
}

TCA smashableCallTarget(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);

  if (possiblySmashableCall(inst)) {
    auto const target32 = *reinterpret_cast<uint32_t*>(ldr->LiteralAddress());
    assertx((target32 & 3) == 0);
    return reinterpret_cast<TCA>(target32);
  }
  return nullptr;
}

TCA smashableJmpTarget(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst);

  if (possiblySmashableJmp(inst)) {
    const uint32_t target32 =
      *reinterpret_cast<uint32_t*>(ldr->LiteralAddress());
    assertx((target32 & 3) == 0);
    return reinterpret_cast<TCA>(target32);
  }
  return nullptr;
}

TCA smashableJccTarget(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  Instruction* ldr = b->NextInstruction();;

  if (possiblySmashableJcc(inst)) {
    const uint32_t target32 =
      *reinterpret_cast<uint32_t*>(ldr->LiteralAddress());
    assertx((target32 & 3) == 0);
    return reinterpret_cast<TCA>(target32);
  }
  return nullptr;
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);

  assertx(possiblySmashableJcc(inst));

  return arm::convertCC(InvertCondition(static_cast<Condition>(b->Bits(3, 0))));
}
///////////////////////////////////////////////////////////////////////////////

bool optimizeSmashedCall(TCA inst) {
  using namespace vixl;

  assertx(possiblySmashableCall(inst));

  auto const ldr = Instruction::Cast(inst);
  auto const blr = ldr->NextInstruction();
  auto const callee = smashableCallTarget(inst);
  auto const offset = (intptr_t)callee - (intptr_t)blr;

  if (is_int28(offset)) {
    CodeBlock callBlock;
    callBlock.init(inst, 8 /* bytes */, "optimizeSmashedCall");
    MacroAssembler a{callBlock};
    a.nop();
    a.bl(offset >> kInstructionSizeLog2);
    return true;
  }

  return false;
}

bool optimizeSmashedJmp(TCA inst) {
  using namespace vixl;

  assertx(possiblySmashableJmp(inst));

  auto const ldr = Instruction::Cast(inst);
  auto const  br = ldr->NextInstruction();
  auto const target = smashableJmpTarget(inst);
  auto const offset = (intptr_t)target - (intptr_t)br;

  if (is_int28(offset)) {
    CodeBlock callBlock;
    callBlock.init(inst, 8 /* bytes */, "optimizeSmashedJmp");
    MacroAssembler a{callBlock};
    a.nop();
    a.b(offset >> kInstructionSizeLog2);
    return true;
  }

  return false;
}

bool optimizeSmashedJcc(TCA inst) {
  using namespace vixl;

  assertx(possiblySmashableJcc(inst));

  auto const b = Instruction::Cast(inst);
  auto const target = smashableJccTarget(inst);
  auto const offset = (intptr_t)target - (intptr_t)b;

  if (is_int21(offset)) {
    CodeBlock callBlock;
    callBlock.init(inst, 12 /* bytes */, "optimizeSmashedJcc");
    MacroAssembler a{callBlock};
    auto const cond = static_cast<Condition>(b->ConditionBranch());
    auto const invCond = InvertCondition(cond);
    a.b(offset >> kInstructionSizeLog2, invCond);
    a.nop();
    a.nop();
    return true;
  }

  return optimizeSmashedJmp((TCA)(b->NextInstruction()));
}

///////////////////////////////////////////////////////////////////////////////

}}}
