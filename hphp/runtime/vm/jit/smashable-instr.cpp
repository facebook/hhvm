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

#include "hphp/runtime/vm/jit/smashable-instr.h"

#include "hphp/runtime/vm/jit/smashable-instr-arm.h"
#include "hphp/runtime/vm/jit/smashable-instr-x64.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"
#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

size_t smashableMovqLen() {
  return ARCH_SWITCH_CALL(smashableMovqLen);
}
size_t smashableCmpqLen() {
  return ARCH_SWITCH_CALL(smashableCmpqLen);
}
size_t smashableCallLen() {
  return ARCH_SWITCH_CALL(smashableCallLen);
}
size_t smashableJmpLen() {
  return ARCH_SWITCH_CALL(smashableJmpLen);
}
size_t smashableJccLen() {
  return ARCH_SWITCH_CALL(smashableJccLen);
}

size_t smashableAlignTo() {
  return ARCH_SWITCH_CALL(smashableAlignTo);
}

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d) {
  return ARCH_SWITCH_CALL(emitSmashableMovq, cb, fixups, imm, d);
}
TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& fixups, int32_t imm,
                      PhysReg r, int8_t disp) {
  return ARCH_SWITCH_CALL(emitSmashableCmpq, cb, fixups, imm, r, disp);
}
TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target) {
  return ARCH_SWITCH_CALL(emitSmashableCall, cb, fixups, target);
}
TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target) {
  return ARCH_SWITCH_CALL(emitSmashableJmp, cb, fixups, target);
}
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc) {
  return ARCH_SWITCH_CALL(emitSmashableJcc, cb, fixups, target, cc);
}

void smashMovq(TCA inst, uint64_t imm) {
  return ARCH_SWITCH_CALL(smashMovq, inst, imm);
}
void smashCmpq(TCA inst, uint32_t imm) {
  return ARCH_SWITCH_CALL(smashCmpq, inst, imm);
}
void smashCall(TCA inst, TCA target) {
  return ARCH_SWITCH_CALL(smashCall, inst, target);
}
void smashJmp(TCA inst, TCA target) {
  return ARCH_SWITCH_CALL(smashJmp, inst, target);
}
void smashJcc(TCA inst, TCA target) {
  return ARCH_SWITCH_CALL(smashJcc, inst, target);
}

uint64_t smashableMovqImm(TCA inst) {
  return ARCH_SWITCH_CALL(smashableMovqImm, inst);
}
uint32_t smashableCmpqImm(TCA inst) {
  return ARCH_SWITCH_CALL(smashableCmpqImm, inst);
}
TCA smashableCallTarget(TCA inst) {
  return ARCH_SWITCH_CALL(smashableCallTarget, inst);
}
TCA smashableJmpTarget(TCA inst) {
  return ARCH_SWITCH_CALL(smashableJmpTarget, inst);
}
TCA smashableJccTarget(TCA inst) {
  return ARCH_SWITCH_CALL(smashableJccTarget, inst);
}
ConditionCode smashableJccCond(TCA inst) {
  return ARCH_SWITCH_CALL(smashableJccCond, inst);
}

/*
 * Right now, our smashable calls are always a known size.  If this ever
 * changes, this implementation will need to be architecture-dependent (and
 * the sizeof* routine will probably need to take a TCA).
 */
TCA smashableCallFromRet(TCA ret) {
  return ret - smashableCallLen();
}

bool optimizeSmashedCall(TCA inst) {
  return ARCH_SWITCH_CALL(optimizeSmashedCall, inst);
}

bool optimizeSmashedJmp(TCA inst) {
  return ARCH_SWITCH_CALL(optimizeSmashedJmp, inst);
}

bool optimizeSmashedJcc(TCA inst) {
  return ARCH_SWITCH_CALL(optimizeSmashedJcc, inst);
}

///////////////////////////////////////////////////////////////////////////////

}}
