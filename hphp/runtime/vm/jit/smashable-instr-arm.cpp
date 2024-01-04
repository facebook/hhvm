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
#include "hphp/runtime/vm/jit/service-requests.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP::jit::arm {

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

  auto const the_start = cb.frontier();
  meta.smashableLocations.insert(the_start);

  poolLiteral(cb, meta, (uint64_t)imm, 64, true);
  a.    bind (&imm_data);
  a.    Ldr  (x2a(d), &imm_data);

  cb.sync(the_start);
  return the_start;
}

// SmashableCalls don't embed the target at the end, because the
// BLR must be the last instruction of the sequence.
TCA emitSmashableCall(CodeBlock& cb, CGMeta& meta, TCA target) {
  align(cb, &meta, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };

  auto const the_start = cb.frontier();
  meta.smashableLocations.insert(the_start);

  assertx((makeTarget32(target) & 3) == 0);
  addVeneer(meta, the_start, target);
  vixl::Label veneer_addr;
  a.bind(&veneer_addr);
  a.bl(&veneer_addr);

  cb.sync(the_start);
  return the_start;
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& meta, TCA target) {
  align(cb, &meta, Alignment::SmashJmp, AlignContext::Live);

  vixl::MacroAssembler a { cb };

  auto const the_start = cb.frontier();
  meta.smashableLocations.insert(the_start);

  assertx((makeTarget32(target) & 3) == 0);
  addVeneer(meta, the_start, target);
  vixl::Label veneer_addr;
  a.bind(&veneer_addr);
  a.b(&veneer_addr);

  cb.sync(the_start);
  return the_start;
}

TCA emitSmashableJcc(CodeBlock& cb, CGMeta& meta, TCA target,
                     ConditionCode cc) {
  align(cb, &meta, Alignment::SmashJcc, AlignContext::Live);

  vixl::MacroAssembler a { cb };

  auto const the_start = cb.frontier();
  meta.smashableLocations.insert(the_start);

  assertx((makeTarget32(target) & 3) == 0);
  addVeneer(meta, the_start, target);
  vixl::Label veneer_addr;
  a.bind(&veneer_addr);
  a.b(&veneer_addr, arm::convertCC(cc));

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

bool isVeneer(vixl::Instruction* ldr) {
  using namespace vixl;

  auto const br = ldr->NextInstruction();
  auto const rd = ldr->Rd();

  return (ldr->IsLoadLiteral() &&
          ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
          br->Mask(UnconditionalBranchToRegisterMask) == BR &&
          br->Rn() == rd);
}

bool possiblySmashableCall(TCA inst) {
  using namespace vixl;

  auto const bl = Instruction::Cast(inst);

  return (bl->Mask(UnconditionalBranchMask) == BL &&
          isVeneer(bl->ImmPCOffsetTarget()));
}

bool possiblySmashableJmp(TCA inst) {
  using namespace vixl;

  auto const b = Instruction::Cast(inst);

  return (b->Mask(UnconditionalBranchMask) == B &&
          isVeneer(b->ImmPCOffsetTarget()));
}

bool possiblySmashableJcc(TCA inst) {
  using namespace vixl;

  auto const b = Instruction::Cast(inst);

  return b->IsCondBranchImm() && isVeneer(b->ImmPCOffsetTarget());
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

  if (!possiblySmashableCall(inst)) {
    // In repo authoritative mode only, the optimization performed at the end of
    // this function may have turned a smashable jmp into a direct one, for
    // which possiblySmashableJmp returns false.
    assertx(RuntimeOption::RepoAuthoritative);
    return;
  }

  auto const bl = Instruction::Cast(inst);
  auto const ldr = bl->ImmPCOffsetTarget();
  patchTarget32(ldr->LiteralAddress(), target);

  // If the target can be reached through a direct call, then patch the original
  // call.  Notice that this optimization prevents a debugger guard from being
  // installed later, so we only perform it in repo authoritative mode.  Also,
  // once this optimization is performed, the smashable call can't be smashed
  // again, because possiblySmashableCall() can't detect the code pattern
  // anymore.  For this reason, we don't perform this optimization when the
  // target is a stub, since they're just a temporary target that we later want
  // to replace with a permanent one.
  int64_t offset = target - inst;
  if (RuntimeOption::RepoAuthoritative && !svcreq::isStub(target) &&
      is_int28(offset)) {
    CodeBlock cb;
    uint32_t newInst;
    cb.init((TCA)&newInst, kInstructionSize, "smashCall");
    MacroAssembler a{cb};
    a.bl(offset >> kInstructionSizeLog2);
    smashInst(inst, newInst);
  }
}

void smashJmp(TCA inst, TCA target) {
  using namespace vixl;
  if (!possiblySmashableJmp(inst)) {
    // In repo authoritative mode only, the optimization performed at the end of
    // this function may have turned a smashable jmp into a direct one, for
    // which possiblySmashableJmp returns false.
    assertx(RuntimeOption::RepoAuthoritative);
    return;
  }

  // If the target is within the smashable jmp, then set the target to the
  // end. This mirrors logic in x86_64 with the exception that ARM cannot
  // replace the entire smashable jmp with nops.
  if (target > inst && target - inst <= smashableJmpLen()) {
    target = inst + smashableJmpLen();
  }

  auto const b = Instruction::Cast(inst);
  auto const ldr = b->ImmPCOffsetTarget();
  patchTarget32(ldr->LiteralAddress(), target);

  // If the target can be reached through a direct jump, then patch the original
  // jump.  Notice that this optimization prevents a debugger guard from being
  // installed later, so we only perform it in repo authoritative mode.  Also,
  // once this optimization is performed, the smashable jump can't be smashed
  // again, because possiblySmashableJmp() can't detect the code pattern
  // anymore.  For this reason, we don't perform this optimization when the
  // target is a stub, since they're just a temporary target that we later want
  // to replace with a permanent one.
  int64_t offset = target - inst;
  if (RuntimeOption::RepoAuthoritative && !svcreq::isStub(target) &&
      is_int28(offset)) {
    CodeBlock cb;
    uint32_t newInst;
    cb.init((TCA)&newInst, kInstructionSize, "smashJmp");
    MacroAssembler a{cb};
    if (offset == kInstructionSize) {
      a.nop();
    } else {
      a.b(offset >> kInstructionSizeLog2);
    }
    smashInst(inst, newInst);
  }
}

void smashJcc(TCA inst, TCA target) {
  using namespace vixl;

  if (!possiblySmashableJcc(inst)) {
    // In repo authoritative mode only, the optimization performed at the end of
    // this function may have turned a smashable jcc into a direct one, for
    // which possiblySmashableJcc returns false.
    assertx(RuntimeOption::RepoAuthoritative);
    return;
  }

  auto const b = Instruction::Cast(inst);
  auto const ldr = b->ImmPCOffsetTarget();
  patchTarget32(ldr->LiteralAddress(), target);

  // If the target can be reached through a direct branch, then patch the
  // original branch.  Notice that this optimization prevents a debugger guard
  // from being installed later, so we only perform it in repo authoritative
  // mode.  Also, once this optimization is performed, the smashable Jcc can't
  // be smashed again, because possiblySmashableJcc() can't detect the code
  // pattern anymore.  For this reason, we don't perform this optimization when
  // the target is a stub, since they're just a temporary target that we later
  // want to replace with a permanent one.
  int64_t offset = target - inst;
  if (RuntimeOption::RepoAuthoritative && !svcreq::isStub(target) &&
      is_int21(offset)) {
    CodeBlock cb;
    uint32_t newInst;
    cb.init((TCA)&newInst, kInstructionSize, "smashJcc");
    MacroAssembler a{cb};
    auto const cond = static_cast<Condition>(b->ConditionBranch());
    a.b(offset >> kInstructionSizeLog2, cond);
    smashInst(inst, newInst);
  }
}

void smashInterceptJcc(TCA inst) {
  not_implemented();
}

void smashInterceptJmp(TCA inst) {
  not_implemented();
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

/*
 * Given an instruction `inst' that calls or jumps/branches to a veneer, return
 * the target address that the corresponding veneer jumps to.
 */
static TCA smashableTarget(TCA inst) {
  using namespace vixl;

  auto const b = Instruction::Cast(inst);
  auto const ldr = b->ImmPCOffsetTarget();
  assertx(isVeneer(ldr));
  auto const target32 = *reinterpret_cast<uint32_t*>(ldr->LiteralAddress());
  assertx((target32 & 3) == 0);
  return reinterpret_cast<TCA>(target32);
}

TCA smashableCallTarget(TCA inst) {
  return possiblySmashableCall(inst) ? smashableTarget(inst) : nullptr;
}

TCA smashableJmpTarget(TCA inst) {
  return possiblySmashableJmp(inst) ? smashableTarget(inst) : nullptr;
}

TCA smashableJccTarget(TCA inst) {
  return possiblySmashableJcc(inst) ? smashableTarget(inst) : nullptr;
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);

  assertx(possiblySmashableJcc(inst));

  return arm::convertCC(static_cast<Condition>(b->Bits(3, 0)));
}

///////////////////////////////////////////////////////////////////////////////

bool optimizeSmashedCall(TCA inst) {
  return false;
}

bool optimizeSmashedJmp(TCA inst) {
  return false;
}

bool optimizeSmashedJcc(TCA inst) {
  using namespace vixl;

  if (!possiblySmashableJcc(inst)) return false;

  // Notice that this optimization prevents a debugger guard from being
  // installed later, so we only perform it in repo authoritative mode.
  if (!RuntimeOption::RepoAuthoritative) return false;

  auto const b = Instruction::Cast(inst);
  auto const target = smashableJccTarget(inst);
  assertx(target);
  auto offset = (intptr_t)target - (intptr_t)b;
  assertx(!is_int21(offset)); // otherwise smashJcc would have optimized it

  auto const ldr = b->ImmPCOffsetTarget();
  assertx(ldr->IsLoadLiteral() && ldr->Mask(LoadLiteralMask) == LDR_w_lit);
  offset = (intptr_t)target - (intptr_t)ldr;

  if (is_int28(offset)) {
    CodeBlock tmpBlock;
    tmpBlock.init((TCA)ldr, 2 * kInstructionSize, "optimizeSmashedJcc");
    MacroAssembler a{tmpBlock};
    a.b(offset >> kInstructionSizeLog2); // replace the ldr
    a.nop(); // replace the br
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

}
