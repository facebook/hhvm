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

constexpr uint8_t kSmashJccFlipOff = 2;
constexpr uint8_t kSmashJccFlopOff = 6;

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
  // FIXME: This is used in func-guard*
  not_implemented();
}

TCA emitSmashableCall(CodeBlock& cb, TCA target) {
  align(cb, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  // Load the target address and branch to it. After return skip the data
  a.    Ldr  (rAsm, &target_data);
  a.    Blr  (rAsm);
  a.    B    (&after_data);

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
  using namespace vixl;
  align(cb, Alignment::SmashJcc, AlignContext::Live);

  // Emit the following sequence. During smashing,
  //
  // 1.  If condition doesn't change and the current instruction is
  //     'b.<cc> flip', overwrite 'flipdata' and viceversa
  // 2.  If the condition changes and the current instruction is
  //     'b.<cc> flip', overwrite 'flopdata' first, then overwrite the
  //     instruction to 'b.<cc> flop' and viceversa
  //
  //           b.<cc> flip
  //           b next
  // flip:     ldr x18, flipdata
  //           br x18
  // flipdata: target
  // flop:     ldr x18, flopdata
  //           br x18
  // flopdata: zeroes
  // next:

  vixl::MacroAssembler a { cb };
  vixl::Label flip, flop, flipdata, flopdata, next;

  auto const start = cb.frontier();

  // Emit the conditional branch to flip
  a.    B    (&flip, arm::convertCC(cc));
  a.    B    (&next);

  // Emit the flip portion
  a.    bind (&flip);
  a.    Ldr  (rAsm, &flipdata);
  a.    Br   (rAsm);
  a.    bind (&flipdata);
  a.    dc64 (reinterpret_cast<int64_t>(target));

  // Emit the flop portion
  a.    bind (&flop);
  a.    Ldr  (rAsm, &flopdata);
  a.    Br   (rAsm);
  a.    bind (&flopdata);
  a.    dc64 (0ULL);
  a.    bind (&next);

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
  using namespace vixl;
  always_assert(is_aligned(inst, Alignment::SmashJcc));

  Instruction* b = Instruction::Cast(inst);
  always_assert(b->IsCondBranchImm());

  int offset = b->ImmPCRel();
  always_assert(offset == kSmashJccFlipOff || offset == kSmashJccFlopOff);

  // If condition has changed, switch flip<->flop
  if (cc != CC_None)
    offset = (offset == kSmashJccFlipOff) ? kSmashJccFlopOff:kSmashJccFlipOff;

  // Overwrite the target address
  *reinterpret_cast<TCA*>(inst + offset * 4 + 8) = target;

  // If condition has changed, overwrite the branch instruction
  if (cc != CC_None) {
    int cond = arm::convertCC(cc);
    uint32_t newinst = b->InstructionBits();

    // Create new branch instruction with new condition code and target address
    // FIXME: Use vixl routines
    newinst = (newinst & 0xfffffff0) | cond | ((offset & 0x7ffff) << 5);
    *reinterpret_cast<uint32_t*>(inst) = newinst;
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + smashableMovqLen() - 8);
}

uint32_t smashableCmpqImm(TCA inst) {
  not_implemented();
}

TCA smashableCallTarget(TCA call) {
  return *reinterpret_cast<TCA*>(call + smashableCallLen() - 8);
}

TCA smashableJmpTarget(TCA jmp) {
  return *reinterpret_cast<TCA*>(jmp + smashableJmpLen() - 8);
}

TCA smashableJccTarget(TCA jmp) {
  using namespace vixl;
  Instruction* b = Instruction::Cast(jmp);
  always_assert(b->IsCondBranchImm());

  int offset = b->ImmPCRel();
  always_assert(offset == kSmashJccFlipOff || offset == kSmashJccFlopOff);
  return *reinterpret_cast<TCA*>(jmp + offset * 4 + 8);
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;
  struct JccDecoder : public Decoder {
    void VisitConditionalBranch(Instruction* inst) override {
      cc = (Condition)inst->ConditionBranch();
    }
    folly::Optional<Condition> cc;
  };
  JccDecoder decoder;
  decoder.Decode(Instruction::Cast(inst));
  always_assert(decoder.cc);
  return convertCC(*decoder.cc);
}

///////////////////////////////////////////////////////////////////////////////

}}}
