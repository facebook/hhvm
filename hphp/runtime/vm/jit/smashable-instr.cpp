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

#include "hphp/runtime/vm/jit/smashable-instr.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"
#include "hphp/runtime/vm/jit/smashable-instr-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

size_t sizeof_smashable_movq() {
  ARCH_SWITCH_CALL(sizeof_smashable_movq);
}
size_t sizeof_smashable_cmpq() {
  ARCH_SWITCH_CALL(sizeof_smashable_cmpq);
}
size_t sizeof_smashable_call() {
  ARCH_SWITCH_CALL(sizeof_smashable_call);
}
size_t sizeof_smashable_jmp() {
  ARCH_SWITCH_CALL(sizeof_smashable_jmp);
}
size_t sizeof_smashable_jcc() {
  ARCH_SWITCH_CALL(sizeof_smashable_jcc);
}

TCA emit_smashable_movq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  ARCH_SWITCH_CALL(emit_smashable_movq, cb, imm, d);
}
TCA emit_smashable_cmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  ARCH_SWITCH_CALL(emit_smashable_cmpq, cb, imm, r, disp);
}
TCA emit_smashable_call(CodeBlock& cb, TCA target) {
  ARCH_SWITCH_CALL(emit_smashable_call, cb, target);
}
TCA emit_smashable_jmp(CodeBlock& cb, TCA target) {
  ARCH_SWITCH_CALL(emit_smashable_jmp, cb, target);
}
TCA emit_smashable_jcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  ARCH_SWITCH_CALL(emit_smashable_jcc, cb, target, cc);
}
std::pair<TCA,TCA>
emit_smashable_jcc_and_jmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  ARCH_SWITCH_CALL(emit_smashable_jcc_and_jmp, cb, target, cc);
}

void smash_movq(TCA inst, uint64_t imm) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smash_movq, inst, imm);
}
void smash_call(TCA inst, TCA target) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smash_call, inst, target);
}
void smash_jmp(TCA inst, TCA target) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smash_jmp, inst, target);
}
void smash_jcc(TCA inst, TCA target) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smash_jcc, inst, target);
}

uint64_t smashable_movq_imm(TCA inst) {
  ARCH_SWITCH_CALL(smashable_movq_imm, inst);
}
TCA smashable_call_target(TCA inst) {
  ARCH_SWITCH_CALL(smashable_call_target, inst);
}
TCA smashable_jmp_target(TCA inst) {
  ARCH_SWITCH_CALL(smashable_jmp_target, inst);
}
TCA smashable_jcc_target(TCA inst) {
  ARCH_SWITCH_CALL(smashable_jcc_target, inst);
}
ConditionCode smashable_jcc_cond(TCA inst) {
  ARCH_SWITCH_CALL(smashable_jcc_cond, inst);
}

///////////////////////////////////////////////////////////////////////////////

}}
