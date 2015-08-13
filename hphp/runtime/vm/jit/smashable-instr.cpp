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

size_t smashableMovqLen() {
  ARCH_SWITCH_CALL(smashableMovqLen);
}
size_t smashableCmpqLen() {
  ARCH_SWITCH_CALL(smashableCmpqLen);
}
size_t smashableCallLen() {
  ARCH_SWITCH_CALL(smashableCallLen);
}
size_t smashableJmpLen() {
  ARCH_SWITCH_CALL(smashableJmpLen);
}
size_t smashableJccLen() {
  ARCH_SWITCH_CALL(smashableJccLen);
}

TCA emitSmashableMovq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  ARCH_SWITCH_CALL(emitSmashableMovq, cb, imm, d);
}
TCA emitSmashableCmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  ARCH_SWITCH_CALL(emitSmashableCmpq, cb, imm, r, disp);
}
TCA emitSmashableCall(CodeBlock& cb, TCA target) {
  ARCH_SWITCH_CALL(emitSmashableCall, cb, target);
}
TCA emitSmashableJmp(CodeBlock& cb, TCA target) {
  ARCH_SWITCH_CALL(emitSmashableJmp, cb, target);
}
TCA emitSmashableJcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  ARCH_SWITCH_CALL(emitSmashableJcc, cb, target, cc);
}
std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  ARCH_SWITCH_CALL(emitSmashableJccAndJmp, cb, target, cc);
}

void smashMovq(TCA inst, uint64_t imm) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smashMovq, inst, imm);
}
void smashCall(TCA inst, TCA target) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smashCall, inst, target);
}
void smashJmp(TCA inst, TCA target) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smashJmp, inst, target);
}
void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  assertx(MCGenerator::canWrite());
  ARCH_SWITCH_CALL(smashJcc, inst, target, cc);
}

uint64_t smashableMovqImm(TCA inst) {
  ARCH_SWITCH_CALL(smashableMovqImm, inst);
}
TCA smashableCallTarget(TCA inst) {
  ARCH_SWITCH_CALL(smashableCallTarget, inst);
}
TCA smashableJmpTarget(TCA inst) {
  ARCH_SWITCH_CALL(smashableJmpTarget, inst);
}
TCA smashableJccTarget(TCA inst) {
  ARCH_SWITCH_CALL(smashableJccTarget, inst);
}
ConditionCode smashableJccCond(TCA inst) {
  ARCH_SWITCH_CALL(smashableJccCond, inst);
}

/*
 * Right now, our smashable calls are always a known size.  If this ever
 * changes, this implementation will need to be architecture-dependent (and
 * the sizeof* routine will probably need to take a TCA).
 */
TCA smashableCallFromRet(TCA ret) {
  return ret - smashableCallLen();
}

///////////////////////////////////////////////////////////////////////////////

}}
