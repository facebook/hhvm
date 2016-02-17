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

TCA emitSmashableMovq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  align(cb, Alignment::SmashMovq, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label imm_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  a.    Ldr  (x2a(d), &imm_data);
  a.    B    (&after_data);
  assertx(cb.isFrontierAligned(8));

  // Emit the immediate into the instruction stream.
  a.    bind (&imm_data);
  a.    dc64 (imm);
  a.    bind (&after_data);

  return start;
}

TCA emitSmashableCmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  not_implemented();
}

TCA emitSmashableCall(CodeBlock& cb, TCA target) {
  align(cb, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label after_data;
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (rAsm, &target_data);
  a.    Blr  (rAsm);
  // When the call returns, jump over the data.
  a.    B    (&after_data);
  assertx(cb.isFrontierAligned(8));

  // Emit the call target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (reinterpret_cast<int64_t>(target));
  a.    bind (&after_data);

  return start;
}

TCA emitSmashableJmp(CodeBlock& cb, TCA target) {
  align(cb, Alignment::SmashJmp, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (rAsm, &target_data);
  a.    Br   (rAsm);
  assertx(cb.isFrontierAligned(8));

  // Emit the jmp target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (reinterpret_cast<int64_t>(target));

  return start;
}

TCA emitSmashableJcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  align(cb, Alignment::SmashJcc, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label after_data;

  auto const start = cb.frontier();

  a.    B    (&after_data, InvertCondition(arm::convertCC(cc)));
  emitSmashableJmp(cb, target);
  a.    bind (&after_data);

  return start;
}

std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  auto const jcc = emitSmashableJcc(cb, target, cc);
  auto const jmp = emitSmashableJmp(cb, target);
  return std::make_pair(jcc, jmp);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static void smashInstr(TCA inst, T target, size_t sz) {
  *reinterpret_cast<T*>(inst + sz - 8) = target;
}

void smashMovq(TCA inst, uint64_t target) {
  smashInstr(inst, target, smashableMovqLen());
}

void smashCmpq(TCA inst, uint32_t target) {
  not_implemented();
}

void smashCall(TCA inst, TCA target) {
  smashInstr(inst, target, smashableCallLen());
}

void smashJmp(TCA inst, TCA target) {
  smashInstr(inst, target, smashableJmpLen());
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  assertx(cc == CC_None);
  smashInstr(inst, target, smashableJccLen());
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + smashableMovqLen() - 8);
}

uint32_t smashableCmpqImm(TCA inst) {
  not_implemented();
}

TCA smashableCallTarget(TCA call) {
  using namespace vixl;
  Instruction* ldr = Instruction::Cast(call);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* blr = Instruction::Cast(call + 4);
  if (blr->Bits(31, 10) != 0x358FC0 || blr->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(blr + 8);
  assertx((dest & 7) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

TCA smashableJmpTarget(TCA jmp) {
  // This doesn't verify that each of the two or three instructions that make
  // up this sequence matches; just the first one and the indirect jump.
  using namespace vixl;
  Instruction* ldr = Instruction::Cast(jmp);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* br = Instruction::Cast(jmp + 4);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 8);
  assertx((dest & 7) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

TCA smashableJccTarget(TCA jmp) {
  using namespace vixl;
  Instruction* b = Instruction::Cast(jmp);
  if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return nullptr;

  Instruction* br = Instruction::Cast(jmp + 8);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 12);
  assertx((dest & 7) == 0);

  return *reinterpret_cast<TCA*>(dest);
}

ConditionCode smashableJccCond(TCA inst) {
  not_implemented();
}

///////////////////////////////////////////////////////////////////////////////

}}}
