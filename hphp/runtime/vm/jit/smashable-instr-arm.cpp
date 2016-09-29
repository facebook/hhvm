/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d) {
  align(cb, &fixups, Alignment::SmashMovq, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label imm_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  a.    Ldr  (x2a(d), &imm_data);
  a.    B    (&after_data);

  // Emit the immediate into the instruction stream.
  a.    bind (&imm_data);
  a.    dc64 (imm);
  a.    bind (&after_data);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& fixups, int32_t imm,
                      PhysReg r, int8_t disp) {
  // FIXME: This is used in func-guard*
  not_implemented();
}

TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target) {
  align(cb, &fixups, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  // Jump over the data
  a.    B    (&after_data);

  // Emit the call target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (target);
  a.    bind (&after_data);

  // Load the target address and call it
  a.    Ldr  (rAsm, &target_data);
  a.    Blr  (rAsm);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target) {
  align(cb, &fixups, Alignment::SmashJmp, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (rAsm, &target_data);
  a.    Br   (rAsm);

  // Emit the jmp target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (target);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

// While a b.cc can be overwritten on ARM, if the cc and the target
// are both changed then the behavior can cause old cc to jump to new
// target or new cc to jump to old target. Therefore we'll keep
// the branch as an indirect branch to a target stored in the
// instruction stream. This we we can at least guarantee that old cc
// won't jump to new target. We can still have an issue where new cc
// jumps to old target, but that old target is *likely* a stub.
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc) {
  align(cb, &fixups, Alignment::SmashJcc, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label after_data;

  auto const start = cb.frontier();

  // Emit the conditional branch
  a.    B    (&after_data, InvertCondition(arm::convertCC(cc)));

  // Emit the smashable jump
  emitSmashableJmp(cb, fixups, target);
  a.    bind (&after_data);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static void smashInstr(TCA inst, T target, size_t sz) {
  *reinterpret_cast<T*>(inst + sz - 8) = target;
  auto const end = reinterpret_cast<char*>(inst + sz);
  auto const begin = end - 8;
  __builtin___clear_cache(begin, end);
}

void smashMovq(TCA inst, uint64_t target) {
  smashInstr(inst, target, smashableMovqLen());
}

void smashCmpq(TCA inst, uint32_t target) {
  not_implemented();
}

void smashCall(TCA inst, TCA target) {
  smashInstr(inst, target, (1 * 4) + 8);
}

void smashJmp(TCA inst, TCA target) {

  // If the target is within the smashable jmp, then set the target to the
  // end. This mirrors logic in x86_64 with the exception that ARM cannot
  // replace the entire smashable jmp with nops.
  if (target > inst && target - inst <= smashableJmpLen()) {
    target = inst + smashableJmpLen();
  }
  smashInstr(inst, target, smashableJmpLen());
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  using namespace vixl;

  // Smash the conditional branch if required
  if (cc != CC_None) {
    Instruction* bcc = Instruction::Cast(inst);
    Instr bits = bcc->InstructionBits();
    bits = (bits >> 4 << 4) | InvertCondition(arm::convertCC(cc));
    bcc->SetInstructionBits(bits);
    auto const begin = reinterpret_cast<char*>(inst);
    auto const end = begin + 4;
    __builtin___clear_cache(begin, end);
  }

  // Then smash the target if changed
  if (smashableJccTarget(inst) != target) {
    smashInstr(inst, target, smashableJccLen());
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + smashableMovqLen() - 8);
}

uint32_t smashableCmpqImm(TCA inst) {
  not_implemented();
}

TCA smashableCallTarget(TCA inst) {
  using namespace vixl;

  Instruction* ldr = Instruction::Cast(inst + (1 * 4) + 8);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* blr = Instruction::Cast(inst + (2 * 4) + 8);
  if (blr->Bits(31, 10) != 0x358FC0 || blr->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(inst + (1 * 4));
  assertx((dest & 3) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

TCA smashableJmpTarget(TCA inst) {
  using namespace vixl;

  // This doesn't verify that each of the two or three instructions that make
  // up this sequence matches; just the first one and the indirect jump.
  Instruction* ldr = Instruction::Cast(inst);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* br = Instruction::Cast(inst + 4);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(inst + 8);
  assertx((dest & 3) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

TCA smashableJccTarget(TCA inst) {
  using namespace vixl;

  Instruction* b = Instruction::Cast(inst);
  if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return nullptr;

  Instruction* br = Instruction::Cast(inst + 8);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(inst + 12);
  assertx((dest & 3) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;
  Instruction* b = Instruction::Cast(inst);
  if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return CC_NP;

  Instruction* br = Instruction::Cast(inst + 8);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return CC_NP;

  return arm::convertCC(InvertCondition(static_cast<Condition>(b->Bits(3, 0))));

}

///////////////////////////////////////////////////////////////////////////////

}}}
