/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

TCA emit_smashable_movq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  not_implemented();
}

TCA emit_smashable_cmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  not_implemented();
}

TCA emit_smashable_call(CodeBlock& cb, TCA target) {
  vixl::MacroAssembler a { cb };
  vixl::Label after_data;
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (arm::rAsm, &target_data);
  a.    Blr  (arm::rAsm);
  // When the call returns, jump over the data.
  a.    B    (&after_data);
  if (!cb.isFrontierAligned(8)) {
    a.  Nop  ();
    assertx(cb.isFrontierAligned(8));
  }
  a.    bind (&target_data);
  a.    dc64 (reinterpret_cast<int64_t>(target));
  a.    bind (&after_data);

  // Keep in sync with smash_ctrl_flow().
  assertx(target_data.target() == start + 12 ||
          target_data.target() == start + 16);

  return start;
}

TCA emit_smashable_jmp(CodeBlock& cb, TCA target) {
  vixl::MacroAssembler a { cb };
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (arm::rAsm, &target_data);
  a.    Br   (arm::rAsm);
  if (!cb.isFrontierAligned(8)) {
    a.  Nop  ();
    assertx(cb.isFrontierAligned(8));
  }
  a.    bind (&target_data);
  a.    dc64 (reinterpret_cast<int64_t>(target));

  // Keep in sync with smash_ctrl_flow().
  assertx(target_data.target() == start + 8 ||
          target_data.target() == start + 12);

  return start;
}

TCA emit_smashable_jcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  vixl::MacroAssembler a { cb };
  vixl::Label after_data;

  auto const start = cb.frontier();

  a.    B    (&after_data, InvertCondition(arm::convertCC(cc)));
  emit_smashable_jmp(cb, target);
  a.    bind (&after_data);

  return start;
}

std::pair<TCA,TCA>
emit_smashable_jcc_and_jmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  auto const jcc = emit_smashable_jcc(cb, target, cc);
  auto const jmp = emit_smashable_jmp(cb, target);
  return std::make_pair(jcc, jmp);
}

///////////////////////////////////////////////////////////////////////////////

void smash_ctrl_flow(TCA inst, TCA target, bool is_jmp) {
  // We wrote two instructions for a jmp, or three for a jcc or call, and then
  // the target was written at the next 8-byte boundary.
  auto data_ptr = inst + (is_jmp ? 8 : 12);
  if ((uintptr_t(data_ptr) & 7) != 0) {
    data_ptr += 4;
    assertx((uintptr_t(data_ptr) & 7) == 0);
  }
  *reinterpret_cast<TCA*>(data_ptr) = target;
}

void smash_movq(TCA inst, uint64_t target) {
  not_implemented();
}

void smash_call(TCA inst, TCA target) {
  smash_ctrl_flow(inst, target, false);
}

void smash_jmp(TCA inst, TCA target) {
  smash_ctrl_flow(inst, target, true);
}

void smash_jcc(TCA inst, TCA target, ConditionCode cc) {
  assertx(cc == CC_None);
  smash_ctrl_flow(inst, target, false);
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashable_movq_imm(TCA inst) {
  not_implemented();
}

TCA smashable_call_target(TCA call) {
  using namespace vixl;
  Instruction* ldr = Instruction::Cast(call);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* blr = Instruction::Cast(call + 4);
  if (blr->Bits(31, 10) != 0x358FC0 || blr->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(blr + 8);
  if ((dest & 7) != 0) {
    dest += 4;
    assertx((dest & 7) == 0);
  }
  return *reinterpret_cast<TCA*>(dest);
}

TCA smashable_jmp_target(TCA jmp) {
  // This doesn't verify that each of the two or three instructions that make
  // up this sequence matches; just the first one and the indirect jump.
  using namespace vixl;
  Instruction* ldr = Instruction::Cast(jmp);
  if (ldr->Bits(31, 24) != 0x58) return nullptr;

  Instruction* br = Instruction::Cast(jmp + 4);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 8);
  if ((dest & 7) != 0) {
    dest += 4;
    assertx((dest & 7) == 0);
  }
  return *reinterpret_cast<TCA*>(dest);
}

TCA smashable_jcc_target(TCA jmp) {
  using namespace vixl;
  Instruction* b = Instruction::Cast(jmp);
  if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return nullptr;

  Instruction* br = Instruction::Cast(jmp + 8);
  if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

  uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 12);
  if ((dest & 7) != 0) {
    dest += 4;
    assertx((dest & 7) == 0);
  }
  return *reinterpret_cast<TCA*>(dest);
}

ConditionCode smashable_jcc_cond(TCA inst) {
  not_implemented();
}

///////////////////////////////////////////////////////////////////////////////

}}}
